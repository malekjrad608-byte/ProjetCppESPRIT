#include "localrepository.h"
#include "../database/oraclecollection.h"
#include "../database/databasemanager.h"

#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QSet>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>

LocalRepository::LocalRepository(QObject *parent, const QString &storagePath)
    : QObject(parent), m_path(storagePath) { load(); }

QString LocalRepository::filePath() const { return m_path; }

void LocalRepository::load()
{
    m_items.clear();
    m_loadFailed=false;
    if(m_path==":memory:") { seed(); return; }
    if(m_path=="oracle:") {
        OracleCollection::Rows rows;
        if(!OracleCollection::read("locaux",&rows,&m_revision,&m_error)) {m_loadFailed=true;return;}
        if(m_revision>0) {
            for(const auto &r:rows) {
                Local l;l.id=r[0].toInt();l.name=r[1].toString();l.type=r[2].toString();l.floor=r[3].toInt();
                l.capacity=r[4].toInt();l.occupancy=r[5].toInt();l.specialty=r[6].toString();l.outOfService=r[7].toInt()!=0;
                l.cleaning=CleaningState(r[8].toInt());l.lastCleaned=QDateTime::fromString(r[9].toString(),Qt::ISODate);
                l.dirtySince=QDateTime::fromString(r[10].toString(),Qt::ISODate);m_items<<l;
            }
            return;
        }
    }
    const QString sourcePath=m_path=="oracle:"?QFileInfo(DatabaseManager::instance().storagePath()).absolutePath()+"/locaux_v2.json":m_path;
    const bool existing=QFile::exists(sourcePath);
    QFile f(existing?sourcePath:QString(":/data/locaux-initial.json"));
    if(!f.open(QIODevice::ReadOnly)) {
        m_loadFailed=true; m_error="Impossible de lire les locaux : "+f.errorString(); return;
    }
    QJsonParseError error;
    const auto doc=QJsonDocument::fromJson(f.readAll(),&error);
    if(error.error!=QJsonParseError::NoError || !doc.isArray()) {
        m_loadFailed=true; m_error="Le fichier des locaux est invalide. Il a été conservé sans modification."; return;
    }
    QSet<int> ids; QSet<QString> names;
    for(const auto &value:doc.array()) {
        const Local l=Local::fromJson(value.toObject());
        if(!value.isObject() || l.id<=0 || ids.contains(l.id) || l.name.trimmed().isEmpty()
                || names.contains(l.name.trimmed().toCaseFolded()) || l.capacity<1 || l.occupancy<0 || l.occupancy>l.capacity) {
            m_items.clear(); m_loadFailed=true; m_error="Le fichier des locaux contient des données invalides. L'original a été conservé."; return;
        }
        ids.insert(l.id); names.insert(l.name.trimmed().toCaseFolded()); m_items.append(l);
    }
    if((!existing || m_path=="oracle:") && !save()) m_loadFailed=true;
}

bool LocalRepository::save()
{
    if(m_loadFailed) { emit storageError(m_error); return false; }
    if(m_path==":memory:") return true;
    if(m_path=="oracle:") {
        OracleCollection::Rows rows;
        for(const auto &l:m_items) rows<<QVariantList{l.id,l.name,l.type,l.floor,l.capacity,l.occupancy,l.specialty,int(l.outOfService),int(l.cleaning),l.lastCleaned.toString(Qt::ISODate),l.dirtySince.toString(Qt::ISODate)};
        if(!OracleCollection::save("locaux",rows,&m_revision,&m_error)) {emit storageError(m_error);return false;}
        return true;
    }
    QDir().mkpath(QFileInfo(m_path).absolutePath());
    QSaveFile f(m_path);
    QJsonArray arr; for(const auto &l:m_items) arr.append(l.toJson());
    const QByteArray data=QJsonDocument(arr).toJson(QJsonDocument::Indented);
    if(!f.open(QIODevice::WriteOnly) || f.write(data)!=data.size() || !f.commit()) {
        m_error="Les locaux n'ont pas pu être enregistrés : "+f.errorString();
        emit storageError(m_error); return false;
    }
    m_error.clear(); return true;
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
    const auto before=m_items;
    seed();
    if(!save()) { m_items=before; return; }
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
    if(l.name.trimmed().isEmpty() || nameExists(l.name) || l.capacity<1 || l.occupancy<0 || l.occupancy>l.capacity) return 0;
    l.id = nextId();
    m_items.append(l);
    if(!save()) { m_items.removeLast(); return 0; }
    emit changed();
    return l.id;
}

bool LocalRepository::update(const Local &l)
{
    if(l.name.trimmed().isEmpty() || nameExists(l.name,l.id) || l.capacity<1 || l.occupancy<0 || l.occupancy>l.capacity) return false;
    for (Local &x : m_items) {
        if (x.id == l.id) {
            const Local before=x;
            x = l;
            if(!save()) { x=before; return false; }
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
            const Local before=m_items.takeAt(i);
            if(!save()) { m_items.insert(i,before); return false; }
            emit changed();
            return true;
        }
    }
    return false;
}
