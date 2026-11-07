#include "local.h"
#include <QSqlError>
#include <QVariant>

// =====================================================================
//  Les 4 etats d'un local (colonne ETAT, texte)
// =====================================================================

const QString Local::Disponible  = QStringLiteral("Disponible");
const QString Local::Occupe      = QStringLiteral("Occupé");
const QString Local::ANettoyer   = QStringLiteral("À nettoyer");
const QString Local::HorsService = QStringLiteral("Hors service");

QStringList Local::etats()
{
    return { Disponible, Occupe, ANettoyer, HorsService };
}

// =====================================================================
//  Methodes metier : la SEULE facon de changer ETAT / OCCUPATION
// =====================================================================

bool Local::peutRecevoirAnimal() const
{
    return (etat == Disponible || etat == Occupe) && occupancy < capacity;
}

bool Local::ajouterAnimal()
{
    if (!peutRecevoirAnimal())
        return false;
    ++occupancy;
    etat = Occupe;
    return true;
}

bool Local::retirerAnimal()
{
    if (occupancy <= 0)
        return false;
    --occupancy;
    if (occupancy == 0)
        etat = ANettoyer;          // le dernier animal est parti : le local doit etre nettoye
    return true;
}

bool Local::signalerANettoyer()
{
    if (etat != Disponible)        // un local occupe ou hors service ne se nettoie pas
        return false;
    etat = ANettoyer;
    return true;
}

bool Local::terminerNettoyage()
{
    if (etat != ANettoyer)
        return false;
    etat = Disponible;
    return true;
}

bool Local::setOutOfService(bool on)
{
    if (on) {
        if (occupancy > 0)         // il faut d'abord liberer les animaux
            return false;
        etat = HorsService;
    } else if (etat == HorsService) {
        etat = Disponible;         // remise en service
    }
    return true;
}

// =====================================================================
//  Affichage
// =====================================================================

RoomState Local::state() const
{
    if (etat == HorsService) return RoomState::OutOfService;
    if (etat == Occupe)      return RoomState::Occupied;
    if (etat == ANettoyer)   return RoomState::ToClean;
    return RoomState::Available;
}

CleaningState Local::getCleaning() const
{
    return etat == ANettoyer ? CleaningState::ToClean : CleaningState::Clean;
}

QString Local::cleaningText() const
{
    return etat == ANettoyer ? QStringLiteral("À nettoyer") : QStringLiteral("Propre");
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

// Message d'erreur lisible : seulement la premiere ligne du message Oracle
// (le pilote ODBC ajoute parfois des caracteres illisibles apres)
static QString texteErreur(const QSqlError &e)
{
    QString t = e.databaseText().trimmed();
    if (t.isEmpty())
        t = e.text().trimmed();
    return t.section('\n', 0, 0).trimmed();
}

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
    if (floor < 0)
        return QStringLiteral("L'étage ne peut pas être négatif.");
    if (capacity < 1 || capacity > 50)
        return QStringLiteral("La capacité doit être comprise entre 1 et 50.");
    if (occupancy < 0 || occupancy > capacity)
        return QStringLiteral("L'occupation doit être comprise entre 0 et la capacité.");
    if (specialty.trimmed().size() > 250)
        return QStringLiteral("La spécialité ne doit pas dépasser 250 caractères.");
    if (!etats().contains(etat))
        return QStringLiteral("État du local invalide.");
    if (etat == HorsService && occupancy > 0)
        return QStringLiteral("Un local hors service ne peut pas contenir d'animaux.");
    if (idService <= 0)
        return QStringLiteral("Choisissez le service du local.");
    return QString();
}

// ---------- Create ----------
bool Local::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO LOCAUX (ID_LOCAL, NOM_LOCAL, TYPE_LOCAL, ETAGE, CAPACITE, "
                  "SPECIALITE, ETAT, OCCUPATION, ID_SERV) "
                  "VALUES (:id, :name, :type, :floor, :capacity, :specialty, :etat, :occupancy, :service)");
    query.bindValue(":id", id);
    query.bindValue(":name", name.trimmed());
    query.bindValue(":type", type);
    query.bindValue(":floor", floor);
    query.bindValue(":capacity", capacity);
    query.bindValue(":specialty", specialty.trimmed());
    query.bindValue(":etat", etat);
    query.bindValue(":occupancy", occupancy);
    query.bindValue(":service", idService);

    bool ok = query.exec();
    s_erreur = ok ? QString() : texteErreur(query.lastError());
    return ok;
}

// ---------- Read ----------
QSqlQueryModel *Local::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_LOCAL, NOM_LOCAL, TYPE_LOCAL, ETAGE, CAPACITE, SPECIALITE, "
                    "ETAT, OCCUPATION, ID_SERV FROM LOCAUX ORDER BY ID_LOCAL");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Étage"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Capacité"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Spécialité"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("État"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Occupation"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Service"));
    s_erreur = model->lastError().isValid() ? texteErreur(model->lastError()) : QString();
    return model;
}

// ---------- Update ----------
bool Local::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE LOCAUX SET NOM_LOCAL = :name, TYPE_LOCAL = :type, ETAGE = :floor, "
                  "CAPACITE = :capacity, SPECIALITE = :specialty, ETAT = :etat, "
                  "OCCUPATION = :occupancy, ID_SERV = :service WHERE ID_LOCAL = :id");
    query.bindValue(":id", id);
    query.bindValue(":name", name.trimmed());
    query.bindValue(":type", type);
    query.bindValue(":floor", floor);
    query.bindValue(":capacity", capacity);
    query.bindValue(":specialty", specialty.trimmed());
    query.bindValue(":etat", etat);
    query.bindValue(":occupancy", occupancy);
    query.bindValue(":service", idService);

    bool ok = query.exec();
    s_erreur = ok ? QString() : texteErreur(query.lastError());
    return ok;
}

// ---------- Delete ----------
bool Local::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM LOCAUX WHERE ID_LOCAL = :id");
    query.bindValue(":id", id);

    bool ok = query.exec();
    s_erreur = ok ? QString() : texteErreur(query.lastError());
    return ok;
}

// Prochain identifiant libre, calcule par Oracle (fiable meme si la liste en memoire est vide)
int Local::prochainId()
{
    QSqlQuery query;
    if (query.exec("SELECT NVL(MAX(ID_LOCAL), 100) + 1 FROM LOCAUX") && query.next())
        return query.value(0).toInt();
    return 0;
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
    l.specialty = r.value(5).toString();
    l.etat = r.value(6).toString();
    l.occupancy = qBound(0, r.value(7).toInt(), l.capacity);
    l.idService = r.value(8).toInt();

    // securite : on garde ETAT et OCCUPATION coherents meme si la base a ete modifiee a la main
    if (!etats().contains(l.etat))
        l.etat = Disponible;
    if (l.occupancy > 0 && l.etat != HorsService)
        l.etat = Occupe;
    else if (l.occupancy == 0 && l.etat == Occupe)
        l.etat = Disponible;
    return l;
}
