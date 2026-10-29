#include "employe.h"
#include "planning.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QObject>
#include <QRegularExpression>
#include <QVariant>
#include <QLocale>
#include <QtGlobal>
#include <algorithm>

namespace {
const QString COLONNES_EMPLOYE = QStringLiteral(
    "id, nom, prenom, courriel, telephone, poste, date_embauche, "
    "disponible, charge_hebdo, dispo_hebdo, competences");

// Construit un Employe a partir de la ligne courante d'un QSqlQuery
// (colonnes dans l'ordre de COLONNES_EMPLOYE).
QDate lireDateOracle(const QVariant &valeur)
{
    // Oracle DATE peut arriver via ODBC comme QDate, QDateTime ou texte selon
    // le type de colonne et la version du pilote.
    QDate date = valeur.toDate();
    if (date.isValid())
        return date;

    const QString texte = valeur.toString().trimmed();
    date = QDate::fromString(texte.left(10), "yyyy-MM-dd");
    if (date.isValid()) return date;
    date = QDate::fromString(texte.left(10), "dd/MM/yyyy");
    if (date.isValid()) return date;
    date = QDate::fromString(texte.left(10), "dd-MM-yyyy");
    if (date.isValid()) return date;

    // Anciennes tables : date parfois en VARCHAR2 selon le format NLS Oracle.
    const QLocale anglais(QLocale::English, QLocale::UnitedStates);
    date = anglais.toDate(texte, "dd-MMM-yy");
    if (date.isValid()) return date;
    return anglais.toDate(texte, "dd-MMM-yyyy");
}

bool colonneDateEmbaucheEstDate()
{
    QSqlQuery q;
    q.prepare("SELECT DATA_TYPE FROM USER_TAB_COLUMNS "
              "WHERE TABLE_NAME = 'EMPLOYES' AND COLUMN_NAME = 'DATE_EMBAUCHE'");
    bool estDate = false;
    if (q.exec() && q.next()) {
        const QString type = q.value(0).toString().toUpper();
        estDate = (type == "DATE" || type.startsWith("TIMESTAMP"));
    }
    q.finish();
    return estDate;
}

QString expressionDatePourEcriture()
{
    return colonneDateEmbaucheEstDate()
        ? QStringLiteral("TO_DATE(:date, 'YYYY-MM-DD')")
        : QStringLiteral(":date");
}

// Construit un Employe a partir de la ligne courante d'un QSqlQuery.
Employe depuisRequete(const QSqlQuery &q)
{
    Employe e;
    e.setId(q.value(0).toInt());
    e.setNom(q.value(1).toString());
    e.setPrenom(q.value(2).toString());
    e.setCourriel(q.value(3).toString());
    e.setTelephone(q.value(4).toString());
    e.setPoste(q.value(5).toString());
    e.setDateEmbauche(lireDateOracle(q.value(6)));
    e.setDisponible(q.value(7).toInt() == 1);
    e.setChargeHebdo(q.value(8).toDouble());
    e.setDispoHebdo(q.value(9).toDouble());
    e.setCompetences(q.value(10).toString());
    return e;
}

// Message lisible pour une erreur Oracle (doublon de courriel, etc.)
QString messageErreur(const QSqlQuery &q)
{
    const QString t = q.lastError().text();
    if (t.contains("ORA-00001") || t.contains("UQ_EMPLOYES_COURRIEL"))
        return "Ce courriel est déjà utilisé par un autre employé.";
    return t;
}

// Lie les attributs de l'employe aux parametres nommes de la requete preparee.
void lierValeurs(QSqlQuery &q, const Employe &e)
{
    q.bindValue(":nom", e.getNom().trimmed());
    q.bindValue(":prenom", e.getPrenom().trimmed());
    // courriel vide -> NULL (la contrainte UNIQUE autorise plusieurs NULL)
    q.bindValue(":courriel", e.getCourriel().trimmed().isEmpty()
                                 ? QVariant(QMetaType(QMetaType::QString))
                                 : QVariant(e.getCourriel().trimmed()));
    q.bindValue(":tel", e.getTelephone().trimmed());
    q.bindValue(":poste", e.getPoste());
    q.bindValue(":date", e.getDateEmbauche().toString("yyyy-MM-dd"));
    q.bindValue(":dispo", e.isDisponible() ? 1 : 0);
    q.bindValue(":ch", e.getChargeHebdo());
    q.bindValue(":dh", e.getDispoHebdo());
    q.bindValue(":comp", e.getCompetences().trimmed());
}
} // namespace

Employe::Employe() {}

Employe::Employe(int id, const QString &nom, const QString &prenom, const QString &courriel,
                 const QString &telephone, const QString &poste, const QDate &dateEmbauche,
                 bool disponible, double chargeHebdo, double dispoHebdo,
                 const QString &competences)
    : m_id(id), m_nom(nom), m_prenom(prenom), m_courriel(courriel), m_telephone(telephone),
      m_poste(poste), m_dateEmbauche(dateEmbauche), m_disponible(disponible),
      m_chargeHebdo(chargeHebdo), m_dispoHebdo(dispoHebdo), m_competences(competences)
{
}

// ======================================================================
//  Logique metier
// ======================================================================
double Employe::tauxCharge() const
{
    if (m_dispoHebdo <= 0) return 0.0;
    return m_chargeHebdo / m_dispoHebdo;
}

bool Employe::depasseSeuil(double seuil) const
{
    return tauxCharge() > seuil;
}

// ======================================================================
//  Controle de saisie (cote C++)
// ======================================================================
QString Employe::validerSaisie() const
{
    static const QRegularExpression reNom(
        QStringLiteral("^[\\p{L}][\\p{L} '\\-]*$"));
    static const QRegularExpression reMail(
        QStringLiteral("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$"));
    static const QRegularExpression reTel(
        QStringLiteral("^\\+?[0-9 ]{8,20}$"));

    const QString nom = m_nom.trimmed();
    const QString prenom = m_prenom.trimmed();
    const QString mail = m_courriel.trimmed();
    const QString tel = m_telephone.trimmed();

    if (nom.isEmpty() || prenom.isEmpty())
        return "Nom et prénom sont obligatoires.";
    if (nom.size() > 100 || prenom.size() > 100)
        return "Nom et prénom : 100 caractères maximum.";
    if (!reNom.match(nom).hasMatch())
        return "Le nom ne doit contenir que des lettres, espaces, apostrophes ou tirets.";
    if (!reNom.match(prenom).hasMatch())
        return "Le prénom ne doit contenir que des lettres, espaces, apostrophes ou tirets.";
    if (!mail.isEmpty() && (mail.size() > 150 || !reMail.match(mail).hasMatch()))
        return "Le courriel est invalide (exemple : nom@domaine.tn).";
    if (!tel.isEmpty() && !reTel.match(tel).hasMatch())
        return "Le téléphone est invalide (chiffres et espaces, « + » autorisé au début).";
    if (m_poste.trimmed().isEmpty())
        return "Le poste est obligatoire.";
    if (m_chargeHebdo < 0 || m_chargeHebdo > 60)
        return "La charge hebdomadaire doit être comprise entre 0 et 60 h.";
    if (m_dispoHebdo <= 0 || m_dispoHebdo > 60)
        return "La disponibilité hebdomadaire doit être comprise entre 1 et 60 h.";
    if (m_competences.trimmed().size() > 300)
        return "Compétences : 300 caractères maximum.";
    return QString();
}

// ======================================================================
//  CRUD
// ======================================================================
bool Employe::ajouter(QString *erreur) const
{
    QSqlQuery q;
    q.setForwardOnly(true);
    const QString dateExpr = expressionDatePourEcriture();
    q.prepare(QString("INSERT INTO employes (nom, prenom, courriel, telephone, poste, "
                      "date_embauche, disponible, charge_hebdo, dispo_hebdo, competences) "
                      "VALUES (:nom, :prenom, :courriel, :tel, :poste, %1, :dispo, "
                      ":ch, :dh, :comp)").arg(dateExpr));
    lierValeurs(q, *this);
    const bool ok = q.exec();
    if (!ok && erreur) *erreur = messageErreur(q);
    q.finish();
    return ok;
}

bool Employe::modifier(QString *erreur) const
{
    QSqlQuery q;
    q.setForwardOnly(true);
    const QString dateExpr = expressionDatePourEcriture();
    q.prepare(QString("UPDATE employes SET nom=:nom, prenom=:prenom, courriel=:courriel, "
                      "telephone=:tel, poste=:poste, date_embauche=%1, disponible=:dispo, "
                      "charge_hebdo=:ch, dispo_hebdo=:dh, competences=:comp WHERE id=:id")
                  .arg(dateExpr));
    lierValeurs(q, *this);
    q.bindValue(":id", m_id);
    const bool ok = q.exec();
    if (!ok && erreur) *erreur = messageErreur(q);
    q.finish();
    return ok;
}

bool Employe::supprimer(int id, QString *erreur)
{
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("DELETE FROM employes WHERE id=:id");
    q.bindValue(":id", id);
    const bool ok = q.exec();
    if (!ok && erreur) *erreur = q.lastError().text();
    q.finish();
    return ok;
}

bool Employe::charger(int id, Employe &resultat)
{
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(QString("SELECT %1 FROM employes WHERE id=:id").arg(COLONNES_EMPLOYE));
    q.bindValue(":id", id);
    bool trouve = false;
    if (q.exec() && q.next()) {
        resultat = depuisRequete(q);
        trouve = true;
    }
    q.finish();
    return trouve;
}

// Read : retourne un QSqlQueryModel a afficher dans un QTableView (setModel), comme dans l'atelier.
QSqlQueryModel *Employe::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT id, nom, prenom, courriel, telephone, poste, date_embauche, "
                    "disponible, charge_hebdo, dispo_hebdo, competences FROM employes ORDER BY id");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prénom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Courriel"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Téléphone"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Poste"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Embauche"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Disponible"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Charge (h)"));
    model->setHeaderData(9, Qt::Horizontal, QObject::tr("Dispo (h)"));
    model->setHeaderData(10, Qt::Horizontal, QObject::tr("Compétences"));
    return model;
}

QList<Employe> Employe::rechercher(const QString &filtre, const QString &tri,
                                   const QString &poste, const QString &statut)
{
    QString orderBy = "date_embauche DESC, prenom ASC";
    if (tri == "charge") orderBy = "charge_hebdo DESC, prenom ASC";

    QString sql = QString("SELECT %1 FROM employes WHERE 1=1").arg(COLONNES_EMPLOYE);

    if (!filtre.isEmpty())
        sql += " AND (UPPER(nom) LIKE UPPER(:f1) OR UPPER(prenom) LIKE UPPER(:f2) "
               "OR UPPER(courriel) LIKE UPPER(:f3) OR UPPER(poste) LIKE UPPER(:f4) "
               "OR TO_CHAR(id) LIKE :f5)";
    if (poste != "Tous les postes")
        sql += " AND poste = :poste";
    if (statut == "Disponible")
        sql += " AND disponible = 1";
    else if (statut == "Non disponible")
        sql += " AND disponible = 0";

    sql += " ORDER BY " + orderBy;

    QList<Employe> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(sql);
    if (!filtre.isEmpty()) {
        const QString motif = "%" + filtre + "%";
        for (const char *nom : {":f1", ":f2", ":f3", ":f4", ":f5"})
            q.bindValue(QString::fromLatin1(nom), motif);
    }
    if (poste != "Tous les postes") q.bindValue(":poste", poste);

    if (q.exec()) {
        while (q.next())
            liste.append(depuisRequete(q));
    }
    q.finish();
    return liste;
}

QList<Employe> Employe::afficherTous()
{
    QList<Employe> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(QString("SELECT %1 FROM employes ORDER BY prenom, nom").arg(COLONNES_EMPLOYE));
    if (q.exec()) {
        while (q.next())
            liste.append(depuisRequete(q));
    }
    q.finish();
    return liste;
}

// ======================================================================
//  Autres requetes
// ======================================================================
bool Employe::basculerDisponibilite(int id, bool *nouveauStatut, QString *erreur)
{
    Employe e;
    if (!charger(id, e)) {
        if (erreur) *erreur = "Employé introuvable.";
        return false;
    }
    const bool nouveau = !e.isDisponible();

    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("UPDATE employes SET disponible=:d WHERE id=:id");
    q.bindValue(":d", nouveau ? 1 : 0);
    q.bindValue(":id", id);
    const bool ok = q.exec();
    if (!ok && erreur) *erreur = q.lastError().text();
    q.finish();
    if (ok && nouveauStatut) *nouveauStatut = nouveau;
    return ok;
}

bool Employe::existeCourriel(const QString &courriel)
{
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT COUNT(*) FROM employes WHERE courriel=:email");
    q.bindValue(":email", courriel);
    bool existe = false;
    if (q.exec() && q.next())
        existe = q.value(0).toInt() > 0;
    q.finish();
    return existe;
}

int Employe::compterParDisponibilite(bool disponible)
{
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT COUNT(*) FROM employes WHERE disponible=:d");
    q.bindValue(":d", disponible ? 1 : 0);
    int n = 0;
    if (q.exec() && q.next())
        n = q.value(0).toInt();
    q.finish();
    return n;
}

double Employe::chargeTotaleDisponibles()
{
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT COALESCE(SUM(charge_hebdo),0) FROM employes WHERE disponible=:d");
    q.bindValue(":d", 1);
    double total = 0.0;
    if (q.exec() && q.next())
        total = q.value(0).toDouble();
    q.finish();
    return total;
}

QList<Employe> Employe::listerDisponibles()
{
    QList<Employe> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(QString("SELECT %1 FROM employes WHERE disponible=:d").arg(COLONNES_EMPLOYE));
    q.bindValue(":d", 1);
    if (q.exec()) {
        while (q.next())
            liste.append(depuisRequete(q));
    }
    q.finish();
    return liste;
}

QList<Employe> Employe::rechercherParCompetence(const QString &competence)
{
    QList<Employe> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(QString("SELECT %1 FROM employes WHERE disponible=1 "
                      "AND UPPER(competences) LIKE UPPER(:c) "
                      "ORDER BY (charge_hebdo * 1.0 / NULLIF(dispo_hebdo, 0)) ASC")
                  .arg(COLONNES_EMPLOYE));
    q.bindValue(":c", "%" + competence.trimmed() + "%");
    if (q.exec()) {
        while (q.next())
            liste.append(depuisRequete(q));
    }
    q.finish();
    return liste;
}

QList<Employe> Employe::listerDisponiblesParCharge(bool decroissant)
{
    QList<Employe> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(QString("SELECT %1 FROM employes WHERE disponible=1 AND dispo_hebdo > 0 "
                      "ORDER BY (charge_hebdo * 1.0 / NULLIF(dispo_hebdo, 0)) %2")
                  .arg(COLONNES_EMPLOYE, QString(decroissant ? "DESC" : "ASC")));
    if (q.exec()) {
        while (q.next())
            liste.append(depuisRequete(q));
    }
    q.finish();
    return liste;
}

QList<Employe::ChargeEmploye> Employe::chargesSemaine(const QDate &debutSemaine)
{
    QList<ChargeEmploye> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT e.nom, e.prenom, e.poste, "
              "(COALESCE(e.charge_hebdo,0)+COALESCE(p.plan,0)) AS charge_eff, e.dispo_hebdo "
              "FROM employes e "
              "LEFT JOIN (SELECT responsable_id, SUM(charge_heures) AS plan "
              "FROM planning_affectations WHERE debut >= :debutSemaine AND debut < :finSemaine "
              "GROUP BY responsable_id) p ON p.responsable_id=e.id "
              "ORDER BY (COALESCE(e.charge_hebdo,0)+COALESCE(p.plan,0)) DESC");
    q.bindValue(":debutSemaine", Planning::borneSemaine(debutSemaine));
    q.bindValue(":finSemaine", Planning::borneSemaine(debutSemaine.addDays(7)));
    if (q.exec()) {
        while (q.next()) {
            ChargeEmploye c;
            c.nom = q.value(0).toString();
            c.prenom = q.value(1).toString();
            c.poste = q.value(2).toString();
            c.charge = q.value(3).toDouble();
            c.dispo = q.value(4).toDouble();
            liste.append(c);
        }
    }
    q.finish();
    return liste;
}

Employe::StatsCharge Employe::statistiques(const QList<ChargeEmploye> &charges)
{
    StatsCharge s;
    s.total = charges.size();
    if (charges.isEmpty()) return s;
    s.minimum = charges.first().charge;
    s.maximum = charges.first().charge;
    for (const ChargeEmploye &c : charges) {
        s.somme += c.charge;
        s.maximum = std::max(s.maximum, c.charge);
        s.minimum = std::min(s.minimum, c.charge);
    }
    s.moyenne = s.somme / s.total;
    return s;
}
