#include "localrepository.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QDebug>
#include <QVariant>

#include "connection.h"

// Nom de la table Oracle (si votre equipe utilise "LOCAL", changez ici)
static const QString kTable = QStringLiteral("LOCAUX");

LocalRepository::LocalRepository(QObject *parent) : QObject(parent)
{
    // 1) on essaie Oracle ; 2) sinon on garde l'ancien fichier JSON (secours)
    m_useDb = Connection::createInstance().createConnection() && loadFromDb();
    if (m_useDb) {
        if (m_items.isEmpty()) {          // table vide -> donnees de demonstration
            seed();
            for (const Local &l : m_items)
                dbInsert(l);
        }
    } else {
        qWarning() << "Oracle indisponible, utilisation du fichier JSON.";
        load();
    }
}

// ---------------------------------------------------------------- Oracle
// Colonnes : id_local, nom_local, type_local, emplacement, capacite, statut, description
// Les infos propres a l'application (specialite, occupation, nettoyage...) sont
// rangees en JSON dans la colonne "description" (CLOB) : le schema commun ne change pas.

QString LocalRepository::statutOf(const Local &l)
{
    if (l.outOfService)   return QStringLiteral("MAINTENANCE");
    if (l.occupancy > 0)  return QStringLiteral("OCCUPIED");
    return QStringLiteral("AVAILABLE");
}

QString LocalRepository::extraToJson(const Local &l)
{
    QJsonObject o;
    o["specialty"] = l.specialty;
    o["occupancy"] = l.occupancy;
    o["cleaning"] = static_cast<int>(l.cleaning);
    o["lastCleaned"] = l.lastCleaned.toString(Qt::ISODate);
    if (l.dirtySince.isValid())
        o["dirtySince"] = l.dirtySince.toString(Qt::ISODate);
    return QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact));
}

Local LocalRepository::fromRow(QSqlQuery &q)
{
    Local l;
    l.id = q.value(0).toInt();
    l.name = q.value(1).toString();
    l.type = q.value(2).toString();

    const QString emp = q.value(3).toString();
    const QRegularExpressionMatch m = QRegularExpression("(\\d+)").match(emp);
    l.floor = (emp.contains("RDC", Qt::CaseInsensitive) || !m.hasMatch()) ? 0 : m.captured(1).toInt();

    l.capacity = q.value(4).isNull() ? 1 : qMax(1, q.value(4).toInt());
    l.outOfService = (q.value(5).toString() == QLatin1String("MAINTENANCE"));

    l.lastCleaned = QDateTime::currentDateTime();
    const QString desc = q.value(6).toString();
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(desc.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        const QJsonObject o = doc.object();
        l.specialty = o["specialty"].toString();
        l.occupancy = o["occupancy"].toInt();
        l.cleaning = static_cast<CleaningState>(o["cleaning"].toInt());
        const QDateTime lc = QDateTime::fromString(o["lastCleaned"].toString(), Qt::ISODate);
        if (lc.isValid())
            l.lastCleaned = lc;
        l.dirtySince = QDateTime::fromString(o["dirtySince"].toString(), Qt::ISODate);
    } else {
        l.specialty = desc;   // texte libre saisi par un autre module
    }
    return l;
}

bool LocalRepository::loadFromDb()
{
    m_items.clear();
    QSqlQuery q(Connection::createInstance().database());
    if (!q.exec(QStringLiteral("SELECT id_local, nom_local, type_local, emplacement, capacite, "
                               "statut, description FROM %1 ORDER BY id_local").arg(kTable))) {
        qWarning() << "SELECT impossible :" << q.lastError().text();
        return false;
    }
    while (q.next())
        m_items.append(fromRow(q));
    return true;
}

int LocalRepository::dbNextId() const
{
    QSqlQuery q(Connection::createInstance().database());
    if (q.exec(QStringLiteral("SELECT NVL(MAX(id_local), 0) + 1 FROM %1").arg(kTable)) && q.next())
        return qMax(101, q.value(0).toInt());
    return 101;
}

bool LocalRepository::dbInsert(const Local &l)
{
    QSqlQuery q(Connection::createInstance().database());
    q.prepare(QStringLiteral("INSERT INTO %1 (id_local, nom_local, type_local, emplacement, capacite, "
                             "statut, description) VALUES (?, ?, ?, ?, ?, ?, ?)").arg(kTable));
    q.addBindValue(l.id);
    q.addBindValue(l.name);
    q.addBindValue(l.type);
    q.addBindValue(l.floorShort());
    q.addBindValue(l.capacity);
    q.addBindValue(statutOf(l));
    q.addBindValue(extraToJson(l));
    if (!q.exec()) {
        qWarning() << "INSERT impossible :" << q.lastError().text();
        return false;
    }
    return true;
}

bool LocalRepository::dbUpdate(const Local &l)
{
    QSqlQuery q(Connection::createInstance().database());
    q.prepare(QStringLiteral("UPDATE %1 SET nom_local = ?, type_local = ?, emplacement = ?, "
                             "capacite = ?, statut = ?, description = ? WHERE id_local = ?").arg(kTable));
    q.addBindValue(l.name);
    q.addBindValue(l.type);
    q.addBindValue(l.floorShort());
    q.addBindValue(l.capacity);
    q.addBindValue(statutOf(l));
    q.addBindValue(extraToJson(l));
    q.addBindValue(l.id);
    if (!q.exec()) {
        qWarning() << "UPDATE impossible :" << q.lastError().text();
        return false;
    }
    return true;
}

bool LocalRepository::dbDelete(int id)
{
    QSqlQuery q(Connection::createInstance().database());
    q.prepare(QStringLiteral("DELETE FROM %1 WHERE id_local = ?").arg(kTable));
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "DELETE impossible :" << q.lastError().text();
        return false;
    }
    return true;
}

bool LocalRepository::dbClear()
{
    QSqlQuery q(Connection::createInstance().database());
    return q.exec(QStringLiteral("DELETE FROM %1").arg(kTable));
}

QString LocalRepository::filePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + "/locaux_v2.json";
}

void LocalRepository::load()
{
    m_items.clear();
    QFile f(filePath());
    if (f.open(QIODevice::ReadOnly)) {
        QJsonParseError err;
        const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
        if (err.error == QJsonParseError::NoError && doc.isArray()) {
            const QJsonArray arr = doc.array();
            for (const QJsonValue &v : arr)
                m_items.append(Local::fromJson(v.toObject()));
        }
    }
    if (m_items.isEmpty()) {
        seed();
        save();
    }
}

void LocalRepository::save() const
{
    if (m_useDb)
        return;   // avec Oracle, chaque operation est deja enregistree
    QDir().mkpath(QFileInfo(filePath()).absolutePath());
    QFile f(filePath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    QJsonArray arr;
    for (const Local &l : m_items)
        arr.append(l.toJson());
    f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

void LocalRepository::seed()
{
    m_items.clear();
    int id = 100;
    auto add = [&](const QString &name, const QString &type, int cap,
                   const QString &spec, int floor) {
        Local l;
        l.id = ++id;
        l.name = name;
        l.type = type;
        l.capacity = cap;
        l.specialty = spec;
        l.floor = floor;
        l.lastCleaned = QDateTime::currentDateTime();
        m_items.append(l);
    };
    add("Consultation 1", "Consultation", 1, "Généraliste", 0);
    add("Consultation 2", "Consultation", 1, "Généraliste", 0);
    add("Consultation 3", "Consultation", 1, "Dentiste", 0);
    add("Consultation 4", "Consultation", 1, "Ophtalmo", 0);
    add("Consultation 5", "Consultation", 1, "Généraliste", 0);
    add("Bloc A", "Chirurgie", 1, "Aseptique", 1);
    add("Bloc B", "Chirurgie", 1, "Aseptique", 1);
    add("Hospitalisation 1", "Hospitalisation", 8, "Soins post-op", 1);
    add("Hospitalisation 2", "Hospitalisation", 8, "Soins intensifs", 1);
    add("Laboratoire", "Laboratoire", 1, "Analyses", 0);
    add("Radiologie", "Imagerie", 1, "Radio / écho", 0);
    add("Pension A", "Pension", 20, "Chiens", 0);
    add("Pension B", "Pension", 12, "Chats", 0);
    add("Isolement", "Isolement", 4, "Maladies contagieuses", 1);

    // Etats de demonstration (pour voir les alertes et les couleurs du plan)
    const QDateTime now = QDateTime::currentDateTime();
    for (Local &l : m_items) {
        if (l.name == "Bloc A") {             // sale depuis 1 h 35 -> alerte critique
            l.cleaning = CleaningState::ToClean;
            l.dirtySince = now.addSecs(-95 * 60);
        } else if (l.name == "Consultation 3") {   // sale depuis 3 h 20 -> avertissement
            l.cleaning = CleaningState::ToClean;
            l.dirtySince = now.addSecs(-200 * 60);
        } else if (l.name == "Consultation 2") {   // sale depuis 25 min -> pas encore d'alerte
            l.cleaning = CleaningState::ToClean;
            l.dirtySince = now.addSecs(-25 * 60);
        } else if (l.name == "Consultation 1") {
            l.occupancy = 1;
        } else if (l.name == "Pension A") {        // 19/20 -> presque sature
            l.occupancy = 19;
        } else if (l.name == "Pension B") {
            l.occupancy = 5;
        } else if (l.name == "Hospitalisation 1") { // 8/8 -> sature
            l.occupancy = 8;
        } else if (l.name == "Radiologie") {
            l.outOfService = true;
        }
    }
}

void LocalRepository::resetDemo()
{
    seed();
    if (m_useDb) {
        dbClear();
        for (const Local &l : m_items)
            dbInsert(l);
    }
    save();
    emit changed();
}

int LocalRepository::nextId() const
{
    int m = 100;
    for (const Local &l : m_items)
        m = qMax(m, l.id);
    return m + 1;
}

const Local *LocalRepository::find(int id) const
{
    for (const Local &l : m_items)
        if (l.id == id)
            return &l;
    return nullptr;
}

bool LocalRepository::nameExists(const QString &name, int ignoreId) const
{
    for (const Local &l : m_items)
        if (l.id != ignoreId && l.name.compare(name, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}

int LocalRepository::add(Local l)
{
    l.id = m_useDb ? dbNextId() : nextId();
    if (m_useDb && !dbInsert(l))
        return 0;
    m_items.append(l);
    save();
    emit changed();
    return l.id;
}

bool LocalRepository::update(const Local &l)
{
    for (Local &x : m_items) {
        if (x.id == l.id) {
            if (m_useDb && !dbUpdate(l))
                return false;
            x = l;
            save();
            emit changed();
            return true;
        }
    }
    return false;
}

bool LocalRepository::remove(int id)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) {
            if (m_useDb && !dbDelete(id))
                return false;
            m_items.removeAt(i);
            save();
            emit changed();
            return true;
        }
    }
    return false;
}
