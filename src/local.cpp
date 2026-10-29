#include "local.h"
#include <QSqlError>
#include <QVariant>

RoomState Local::state() const
{
    if (outOfService)
        return RoomState::OutOfService;
    return occupancy > 0 ? RoomState::Occupied : RoomState::Available;
}

QString Local::stateText() const
{
    switch (state()) {
    case RoomState::Available:    return QStringLiteral("Disponible");
    case RoomState::Occupied:     return QStringLiteral("Occupé");
    case RoomState::OutOfService: return QStringLiteral("Hors service");
    }
    return QString();
}

QString Local::cleaningText() const
{
    switch (cleaning) {
    case CleaningState::Clean:      return QStringLiteral("Propre");
    case CleaningState::ToClean:    return QStringLiteral("À nettoyer");
    case CleaningState::InProgress: return QStringLiteral("En cours");
    }
    return QString();
}

QString Local::floorText() const
{
    return floor == 0 ? QStringLiteral("Rez-de-chaussée")
                      : QStringLiteral("Étage %1").arg(floor);
}

QString Local::floorShort() const
{
    return floor == 0 ? QStringLiteral("RDC") : QStringLiteral("Étage %1").arg(floor);
}

QString Local::iconName() const
{
    if (type == "Consultation")    return QStringLiteral("consult");
    if (type == "Chirurgie")       return QStringLiteral("surgery");
    if (type == "Hospitalisation") return QStringLiteral("bed");
    if (type == "Pension")         return QStringLiteral("home");
    if (type == "Laboratoire")     return QStringLiteral("flask");
    if (type == "Imagerie")        return QStringLiteral("scan");
    if (type == "Isolement")       return QStringLiteral("isolation");
    return QStringLiteral("rooms");
}

bool Local::isUrgentType() const
{
    return type == "Chirurgie" || type == "Isolement";
}

void Local::markToClean()
{
    cleaning = CleaningState::ToClean;
    dirtySince = QDateTime::currentDateTime();
}

void Local::startCleaning()
{
    cleaning = CleaningState::InProgress;
}

void Local::markClean()
{
    cleaning = CleaningState::Clean;
    lastCleaned = QDateTime::currentDateTime();
    dirtySince = QDateTime();
}

qint64 Local::dirtyMinutes() const
{
    if (cleaning != CleaningState::ToClean)
        return 0;
    const QDateTime since = dirtySince.isValid() ? dirtySince : lastCleaned;
    if (!since.isValid())
        return 0;
    return qMax<qint64>(0, since.secsTo(QDateTime::currentDateTime()) / 60);
}

QStringList Local::types()
{
    return { "Consultation", "Chirurgie", "Hospitalisation", "Pension",
             "Laboratoire", "Imagerie", "Isolement" };
}

// =====================================================================
//  CRUD - table LOCAUX (meme architecture que l'atelier CRUD)
//  Requetes preparees : prepare() + bindValue() -> protection contre l'injection SQL
// =====================================================================

static QString s_erreur;   // dernier message d'erreur SQL (affiche dans l'interface)

QString Local::derniereErreur()
{
    return s_erreur;
}

// Controle de saisie (fait en C++ avant d'envoyer la requete)
QString Local::controleSaisie() const
{
    if (name.trimmed().isEmpty())
        return QStringLiteral("Le nom du local est obligatoire.");
    if (name.trimmed().size() > 150)
        return QStringLiteral("Le nom ne doit pas dépasser 150 caractères.");
    if (type.trimmed().isEmpty())
        return QStringLiteral("Le type du local est obligatoire.");
    if (capacity < 1 || capacity > 50)
        return QStringLiteral("La capacité doit être comprise entre 1 et 50.");
    if (occupancy < 0 || occupancy > capacity)
        return QStringLiteral("L'occupation doit être comprise entre 0 et la capacité.");
    if (idService <= 0)
        return QStringLiteral("Choisissez le service du local.");
    return QString();
}

// Les dates sont enregistrees en texte (format ISO).
// Date vide -> chaine vide (Oracle la stocke comme NULL ; evite l'erreur ODBC S1010)
static QString dateVersBd(const QDateTime &d)
{
    return d.isValid() ? d.toString(Qt::ISODate) : QString("");
}

// ---------- Create ----------
bool Local::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO LOCAUX (ID_LOCAL, NOM_LOCAL, TYPE_LOCAL, ETAGE, CAPACITE, OCCUPATION, "
                  "SPECIALITE, HORS_SERVICE, NETTOYAGE, DERNIER_NETTOYAGE, SALE_DEPUIS, ID_SERV) "
                  "VALUES (:id, :name, :type, :floor, :capacity, :occupancy, :specialty, "
                  ":oos, :cleaning, :last, :dirty, :service)");
    query.bindValue(":id", id);
    query.bindValue(":name", name.trimmed());
    query.bindValue(":type", type);
    query.bindValue(":floor", floor);
    query.bindValue(":capacity", capacity);
    query.bindValue(":occupancy", occupancy);
    query.bindValue(":specialty", specialty.trimmed());
    query.bindValue(":oos", outOfService ? 1 : 0);
    query.bindValue(":cleaning", static_cast<int>(cleaning));
    query.bindValue(":last", dateVersBd(lastCleaned));
    query.bindValue(":dirty", dateVersBd(dirtySince));
    query.bindValue(":service", idService);

    bool ok = query.exec();
    s_erreur = ok ? QString() : query.lastError().text();
    return ok;
}

// ---------- Read ----------
QSqlQueryModel *Local::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_LOCAL, NOM_LOCAL, TYPE_LOCAL, ETAGE, CAPACITE, OCCUPATION, SPECIALITE, "
                    "HORS_SERVICE, NETTOYAGE, DERNIER_NETTOYAGE, SALE_DEPUIS, ID_SERV "
                    "FROM LOCAUX ORDER BY ID_LOCAL");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Étage"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Capacité"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Occupation"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Spécialité"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Hors service"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Nettoyage"));
    model->setHeaderData(9, Qt::Horizontal, QObject::tr("Dernier nettoyage"));
    model->setHeaderData(10, Qt::Horizontal, QObject::tr("Sale depuis"));
    model->setHeaderData(11, Qt::Horizontal, QObject::tr("Service"));
    s_erreur = model->lastError().isValid() ? model->lastError().text() : QString();
    return model;
}

// ---------- Update ----------
bool Local::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE LOCAUX SET NOM_LOCAL = :name, TYPE_LOCAL = :type, ETAGE = :floor, "
                  "CAPACITE = :capacity, OCCUPATION = :occupancy, SPECIALITE = :specialty, "
                  "HORS_SERVICE = :oos, NETTOYAGE = :cleaning, "
                  "DERNIER_NETTOYAGE = :last, SALE_DEPUIS = :dirty, ID_SERV = :service "
                  "WHERE ID_LOCAL = :id");
    query.bindValue(":id", id);
    query.bindValue(":name", name.trimmed());
    query.bindValue(":type", type);
    query.bindValue(":floor", floor);
    query.bindValue(":capacity", capacity);
    query.bindValue(":occupancy", occupancy);
    query.bindValue(":specialty", specialty.trimmed());
    query.bindValue(":oos", outOfService ? 1 : 0);
    query.bindValue(":cleaning", static_cast<int>(cleaning));
    query.bindValue(":last", dateVersBd(lastCleaned));
    query.bindValue(":dirty", dateVersBd(dirtySince));
    query.bindValue(":service", idService);

    bool ok = query.exec();
    s_erreur = ok ? QString() : query.lastError().text();
    return ok;
}

// ---------- Delete ----------
bool Local::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM LOCAUX WHERE ID_LOCAL = :id");
    query.bindValue(":id", id);

    bool ok = query.exec();
    s_erreur = ok ? QString() : query.lastError().text();
    return ok;
}

// Liste des services (pour choisir le service d'un local dans le formulaire)
QList<QPair<int, QString>> Local::services()
{
    QList<QPair<int, QString>> liste;
    QSqlQuery query;
    if (query.exec("SELECT ID_SERV, CATEGORIE, PRESTATION FROM SERVICES ORDER BY ID_SERV")) {
        while (query.next())
            liste.append({ query.value(0).toInt(),
                           query.value(1).toString() + " - " + query.value(2).toString() });
    }
    return liste;
}

// Transforme une ligne du SELECT (meme ordre de colonnes que afficher()) en objet Local
Local Local::fromRecord(const QSqlRecord &r)
{
    Local l;
    l.id = r.value(0).toInt();
    l.name = r.value(1).toString();
    l.type = r.value(2).toString();
    l.floor = r.value(3).toInt();
    l.capacity = qMax(1, r.value(4).toInt());
    l.occupancy = qBound(0, r.value(5).toInt(), l.capacity);
    l.specialty = r.value(6).toString();
    l.outOfService = r.value(7).toInt() != 0;
    l.cleaning = static_cast<CleaningState>(qBound(0, r.value(8).toInt(), 2));
    l.lastCleaned = QDateTime::fromString(r.value(9).toString(), Qt::ISODate);
    if (!l.lastCleaned.isValid())
        l.lastCleaned = QDateTime::currentDateTime();
    l.dirtySince = QDateTime::fromString(r.value(10).toString(), Qt::ISODate);
    if (l.cleaning == CleaningState::ToClean && !l.dirtySince.isValid())
        l.dirtySince = l.lastCleaned;
    l.idService = r.value(11).toInt();
    return l;
}
