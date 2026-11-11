#include "employe.h"
#include "connection.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QObject>
#include <QStringList>
#include <QRegularExpression>
#include <QVariant>
#include <QHash>
#include <QSet>
#include <QLocale>
#include <QtGlobal>
#include <algorithm>
#include <utility>

namespace {
// Colonnes de la table EMPLOYER, dans l'ordre lu par depuisRequete().
const QString COLONNES_EMPLOYE = QStringLiteral(
    "id_emp, nom, prenom, cin, date_naiss, adresse, tel, poste, courriel, "
    "competences, date_embauche, type_contrat, salaire, charge, disponible, "
    "dispo_repos, id_serv");

QDate lireDateOracle(const QVariant &valeur)
{
    // Oracle DATE peut arriver via ODBC comme QDate, QDateTime ou texte selon
    // la version du pilote.
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

    const QLocale anglais(QLocale::English, QLocale::UnitedStates);
    date = anglais.toDate(texte, "dd-MMM-yy");
    if (date.isValid()) return date;
    return anglais.toDate(texte, "dd-MMM-yyyy");
}

// CIN : colonne VARCHAR2(20) dans la table EMPLOYER.
QString lireCin(const QVariant &valeur)
{
    return valeur.toString().trimmed();
}

// SALAIRE, CHARGE et DISPO_REPOS sont des colonnes VARCHAR2(20) dans EMPLOYER.
// SALAIRE/CHARGE contiennent des nombres ; DISPO_REPOS contient le jour de repos
// ou une ancienne valeur numerique d'heures disponibles.
double lireNombre(const QVariant &valeur, double defaut)
{
    QString t = valeur.toString().trimmed();
    if (t.isEmpty())
        return defaut;
    t.replace(QLatin1Char(','), QLatin1Char('.'));
    bool ok = false;
    const double d = t.toDouble(&ok);
    if (ok)
        return d;
    static const QRegularExpression reNombre(QStringLiteral("[0-9]+(\\.[0-9]+)?"));
    const QRegularExpressionMatch m = reNombre.match(t);
    return m.hasMatch() ? m.captured(0).toDouble() : defaut;
}

QString ecrireNombre(double valeur)
{
    QString t = QString::number(valeur, 'f', 2);
    while (t.endsWith(QLatin1Char('0')))
        t.chop(1);
    if (t.endsWith(QLatin1Char('.')))
        t.chop(1);
    return t.isEmpty() ? QStringLiteral("0") : t;
}

// Construit un Employe a partir de la ligne courante d'un QSqlQuery
// (colonnes dans l'ordre de COLONNES_EMPLOYE).
Employe depuisRequete(const QSqlQuery &q)
{
    Employe e;
    e.setId(q.value(0).toInt());
    e.setNom(q.value(1).toString());
    e.setPrenom(q.value(2).toString());
    e.setCin(lireCin(q.value(3)));
    e.setDateNaiss(lireDateOracle(q.value(4)));
    e.setAdresse(q.value(5).toString());
    e.setTel(q.value(6).toString());
    e.setPoste(q.value(7).toString());
    e.setCourriel(q.value(8).toString());
    e.setCompetences(q.value(9).toString());
    e.setDateEmbauche(lireDateOracle(q.value(10)));
    e.setTypeContrat(q.value(11).toString());
    e.setSalaire(lireNombre(q.value(12), 0.0));
    e.setCharge(lireNombre(q.value(13), 0.0));
    e.setDisponible(q.value(14).toInt() == 1);
    e.setDispoRepos(q.value(15).toString().trimmed());
    e.setIdServ(q.value(16).isNull() ? 0 : q.value(16).toInt());
    return e;
}

// Message lisible pour une erreur Oracle.
QString messageErreur(const QSqlQuery &q)
{
    const QString t = q.lastError().text();
    if (t.contains("ORA-00001")) {
        if (t.toUpper().contains("CIN"))
            return "Ce CIN est déjà utilisé par un autre employé.";
        if (t.toUpper().contains("COURRIEL"))
            return "Ce courriel est déjà utilisé par un autre employé.";
        return "Une valeur unique (CIN ou courriel) est déjà utilisée par un autre employé.";
    }
    if (t.contains("ORA-02291"))
        return "Le service choisi n'existe pas dans la table SERVICE.";
    if (t.contains("ORA-01400"))
        return "Une colonne obligatoire de la table EMPLOYER est vide.\n\n" + t;
    return t;
}

bool courrielDejaUtilise(const QString &courriel, int idExclu = 0)
{
    const QString email = courriel.trimmed();
    if (email.isEmpty())
        return false;

    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT COUNT(*) FROM employer "
              "WHERE UPPER(TRIM(courriel)) = UPPER(:email) AND id_emp <> :id");
    q.bindValue(":email", email);
    q.bindValue(":id", idExclu);
    const bool trouve = q.exec() && q.next() && q.value(0).toInt() > 0;
    q.finish();
    return trouve;
}

bool cinDejaUtilise(const QString &cin, int idExclu = 0)
{
    const QString chiffres = cin.trimmed();
    if (chiffres.isEmpty())
        return false;

    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT COUNT(*) FROM employer WHERE cin = :cin AND id_emp <> :id");
    q.bindValue(":cin", chiffres);
    q.bindValue(":id", idExclu);
    const bool trouve = q.exec() && q.next() && q.value(0).toInt() > 0;
    q.finish();
    return trouve;
}

// Lie les attributs de l'employe aux parametres nommes de la requete preparee.
void lierValeurs(QSqlQuery &q, const Employe &e)
{
    q.bindValue(":nom", e.getNom().trimmed());
    q.bindValue(":prenom", e.getPrenom().trimmed());
    q.bindValue(":cin", e.getCin().trimmed());
    q.bindValue(":naiss", e.getDateNaiss().toString("yyyy-MM-dd"));
    q.bindValue(":adresse", e.getAdresse().trimmed());
    q.bindValue(":tel", e.getTel().trimmed());
    q.bindValue(":poste", e.getPoste());
    // courriel vide -> NULL
    q.bindValue(":courriel", e.getCourriel().trimmed().isEmpty()
                                 ? QVariant(QMetaType(QMetaType::QString))
                                 : QVariant(e.getCourriel().trimmed()));
    q.bindValue(":comp", e.getCompetences().trimmed());
    q.bindValue(":emb", e.getDateEmbauche().toString("yyyy-MM-dd"));
    q.bindValue(":contrat", e.getTypeContrat().trimmed());
    q.bindValue(":salaire", ecrireNombre(e.getSalaire()));
    q.bindValue(":charge", ecrireNombre(e.getCharge()));
    q.bindValue(":actif", e.isDisponible() ? 1 : 0);
    q.bindValue(":repos", e.getDispoReposText().trimmed().isEmpty()
                                 ? QVariant(QStringLiteral("Non précisé"))
                                 : QVariant(e.getDispoReposText().trimmed()));
    // pas de service -> NULL (cle etrangere facultative)
    q.bindValue(":serv", e.getIdServ() > 0 ? QVariant(e.getIdServ())
                                           : QVariant(QMetaType(QMetaType::Int)));
}
} // namespace

Employe::Employe() {}

Employe::Employe(int id, const QString &nom, const QString &prenom, const QString &cin,
                 const QDate &dateNaiss, const QString &adresse, const QString &tel,
                 const QString &poste, const QString &courriel, const QString &competences,
                 const QDate &dateEmbauche, const QString &typeContrat, double salaire,
                 double charge, bool disponible, double dispoRepos, int idServ)
    : m_id(id), m_nom(nom), m_prenom(prenom), m_cin(cin), m_dateNaiss(dateNaiss),
      m_adresse(adresse), m_tel(tel), m_poste(poste), m_courriel(courriel),
      m_competences(competences), m_dateEmbauche(dateEmbauche), m_typeContrat(typeContrat),
      m_salaire(salaire), m_charge(charge), m_disponible(disponible),
      m_dispoRepos(ecrireNombre(dispoRepos)), m_idServ(idServ)
{
}

// ======================================================================
//  Logique metier
// ======================================================================
double Employe::tauxCharge() const
{
    const double capacite = getDispoRepos();
    if (capacite <= 0) return 0.0;
    return m_charge / capacite;
}

bool Employe::depasseSeuil(double seuil) const
{
    return tauxCharge() > seuil;
}

// ======================================================================
//  Controle de saisie (cote C++)
// ======================================================================
QString Employe::validerSaisie(bool courrielObligatoire,
                              bool telephoneEtSalaireObligatoires) const
{
    static const QRegularExpression reNom(
        QStringLiteral("^[\\p{L}][\\p{L} '\\-]*$"));
    static const QRegularExpression reMail(
        QStringLiteral("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$"));
    static const QRegularExpression reTel(
        QStringLiteral("^\\+?[0-9 ]{8,20}$"));
    static const QRegularExpression reCin(
        QStringLiteral("^[0-9]{8}$"));

    const QString nom = m_nom.trimmed();
    const QString prenom = m_prenom.trimmed();
    const QString mail = m_courriel.trimmed();
    const QString tel = m_tel.trimmed();
    const QString cin = m_cin.trimmed();

    if (nom.isEmpty() || prenom.isEmpty())
        return "Nom et prénom sont obligatoires.";
    if (nom.size() > 100 || prenom.size() > 100)
        return "Nom et prénom : 100 caractères maximum.";
    if (!reNom.match(nom).hasMatch())
        return "Le nom ne doit contenir que des lettres, espaces, apostrophes ou tirets.";
    if (!reNom.match(prenom).hasMatch())
        return "Le prénom ne doit contenir que des lettres, espaces, apostrophes ou tirets.";
    if (!reCin.match(cin).hasMatch())
        return "Le CIN est obligatoire : exactement 8 chiffres.";
    if (!m_dateNaiss.isValid())
        return "La date de naissance est invalide.";
    if (m_dateNaiss.addYears(16) > QDate::currentDate())
        return "L'employé doit avoir au moins 16 ans.";
    if (m_dateNaiss.year() < 1930)
        return "La date de naissance est trop ancienne (avant 1930).";
    if (m_adresse.trimmed().size() > 250)
        return "Adresse : 250 caractères maximum.";
    if (courrielObligatoire && mail.isEmpty())
        return "Le courriel est obligatoire pour ajouter un nouvel employé.";
    if (!mail.isEmpty() && (mail.size() > 150 || !reMail.match(mail).hasMatch()))
        return "Le courriel est invalide (exemple : nom@domaine.tn).";
    if (telephoneEtSalaireObligatoires && tel.isEmpty())
        return "Le téléphone est obligatoire pour ajouter un nouvel employé.";
    if (!tel.isEmpty() && !reTel.match(tel).hasMatch())
        return "Le téléphone est invalide (chiffres et espaces, « + » autorisé au début).";
    if (m_poste.trimmed().isEmpty())
        return "Le poste est obligatoire.";
    if (m_typeContrat.trimmed().isEmpty() || m_typeContrat.trimmed().size() > 30)
        return "Le type de contrat est obligatoire (30 caractères maximum).";
    if (!m_dateEmbauche.isValid())
        return "La date d'embauche est invalide.";
    if (m_dateEmbauche < m_dateNaiss.addYears(16))
        return "La date d'embauche est incohérente avec la date de naissance (16 ans minimum).";
    if (telephoneEtSalaireObligatoires && m_salaire < 0)
        return "Le salaire est obligatoire pour ajouter un nouvel employé.";
    if (m_salaire < 0 || m_salaire > 1000000.0)
        return "Le salaire doit être compris entre 0 et 1 000 000 DT.";
    if (m_charge < 0 || m_charge > 60)
        return "La charge doit être comprise entre 0 et 60 h par semaine.";
    if (m_dispoRepos.trimmed().size() > 20)
        return "Le jour de repos doit contenir 20 caractères maximum.";
    QString reposSaisi = m_dispoRepos;
    reposSaisi.replace(QLatin1Char(','), QLatin1Char('.'));
    bool reposNumerique = false;
    const double heuresRepos = reposSaisi.toDouble(&reposNumerique);
    if (reposNumerique && (heuresRepos <= 0 || heuresRepos > 60))
        return "Si vous saisissez des heures de disponibilité, la valeur doit être comprise entre 1 et 60.";
    if (m_competences.trimmed().size() > 300)
        return "Compétences : 300 caractères maximum.";
    return QString();
}

// ======================================================================
//  CRUD
// ======================================================================
bool Employe::ajouter(QString *erreur) const
{
    const QString validation = validerSaisie(true, true);
    if (!validation.isEmpty()) {
        if (erreur) *erreur = validation;
        return false;
    }
    if (cinDejaUtilise(m_cin)) {
        if (erreur) *erreur = "Ce CIN est déjà utilisé par un autre employé.";
        return false;
    }
    if (courrielDejaUtilise(m_courriel)) {
        if (erreur) *erreur = "Ce courriel est déjà utilisé par un autre employé.";
        return false;
    }

    // ID_EMP : si la base le genere deja (trigger / IDENTITY) on ne le fournit pas ;
    // sinon on prend le plus grand ID + 1. Aucune modification de la base.
    const bool idParOracle = Connection::createInstance().idEmpAutomatique();
    const QString colId = idParOracle ? QString() : QStringLiteral("id_emp, ");
    const QString valId = idParOracle ? QString()
                                      : QStringLiteral("(SELECT NVL(MAX(id_emp), 0) + 1 FROM employer), ");

    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(QString("INSERT INTO employer (%1nom, prenom, cin, date_naiss, adresse, tel, poste, "
                      "courriel, competences, date_embauche, type_contrat, salaire, charge, "
                      "disponible, dispo_repos, id_serv) "
                      "VALUES (%2:nom, :prenom, :cin, TO_DATE(:naiss, 'YYYY-MM-DD'), :adresse, :tel, "
                      ":poste, :courriel, :comp, TO_DATE(:emb, 'YYYY-MM-DD'), :contrat, :salaire, "
                      ":charge, :actif, :repos, :serv)").arg(colId, valId));
    lierValeurs(q, *this);
    const bool ok = q.exec();
    if (!ok && erreur) *erreur = messageErreur(q);
    q.finish();
    return ok;
}

bool Employe::modifier(QString *erreur) const
{
    if (m_id <= 0) {
        if (erreur) *erreur = "Identifiant d'employé invalide pour la modification.";
        return false;
    }
    const QString validation = validerSaisie();
    if (!validation.isEmpty()) {
        if (erreur) *erreur = validation;
        return false;
    }
    if (cinDejaUtilise(m_cin, m_id)) {
        if (erreur) *erreur = "Ce CIN est déjà utilisé par un autre employé.";
        return false;
    }
    if (courrielDejaUtilise(m_courriel, m_id)) {
        if (erreur) *erreur = "Ce courriel est déjà utilisé par un autre employé.";
        return false;
    }

    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("UPDATE employer SET nom=:nom, prenom=:prenom, cin=:cin, "
              "date_naiss=TO_DATE(:naiss, 'YYYY-MM-DD'), adresse=:adresse, tel=:tel, "
              "poste=:poste, courriel=:courriel, competences=:comp, "
              "date_embauche=TO_DATE(:emb, 'YYYY-MM-DD'), type_contrat=:contrat, "
              "salaire=:salaire, charge=:charge, disponible=:actif, dispo_repos=:repos, "
              "id_serv=:serv WHERE id_emp=:id");
    lierValeurs(q, *this);
    q.bindValue(":id", m_id);
    const bool ok = q.exec();
    if (!ok && erreur) {
        *erreur = messageErreur(q);
    } else if (ok && q.numRowsAffected() == 0) {
        if (erreur) *erreur = "Aucun employé ne correspond à cet identifiant.";
        q.finish();
        return false;
    }
    q.finish();
    return ok;
}

bool Employe::supprimer(int id, QString *erreur)
{
    if (id <= 0) {
        if (erreur) *erreur = "Identifiant d'employé invalide pour la suppression.";
        return false;
    }

    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("DELETE FROM employer WHERE id_emp=:id");
    q.bindValue(":id", id);
    const bool ok = q.exec();
    if (!ok && erreur) {
        const QString t = q.lastError().text();
        if (t.contains("ORA-02292"))
            *erreur = "Suppression impossible : cet employé est encore référencé "
                      "(par exemple dans un dossier médical). Modifiez ou supprimez "
                      "d'abord les enregistrements liés.";
        else
            *erreur = t;
    } else if (ok && q.numRowsAffected() == 0) {
        if (erreur) *erreur = "Aucun employé ne correspond à cet identifiant.";
        q.finish();
        return false;
    }
    q.finish();
    return ok;
}

bool Employe::charger(int id, Employe &resultat)
{
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(QString("SELECT %1 FROM employer WHERE id_emp=:id").arg(COLONNES_EMPLOYE));
    q.bindValue(":id", id);
    bool trouve = false;
    if (q.exec() && q.next()) {
        resultat = depuisRequete(q);
        trouve = true;
    }
    q.finish();
    return trouve;
}

// Read : retourne un QSqlQueryModel a afficher dans un QTableView (setModel).
QSqlQueryModel *Employe::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery q;
    q.prepare(QString("SELECT %1 FROM employer ORDER BY id_emp").arg(COLONNES_EMPLOYE));
    q.exec();
    model->setQuery(std::move(q));
    const QStringList entetes = {
        QObject::tr("ID"), QObject::tr("Nom"), QObject::tr("Prénom"), QObject::tr("CIN"),
        QObject::tr("Naissance"), QObject::tr("Adresse"), QObject::tr("Tél"),
        QObject::tr("Poste"), QObject::tr("Courriel"), QObject::tr("Compétences"),
        QObject::tr("Embauche"), QObject::tr("Contrat"), QObject::tr("Salaire"),
        QObject::tr("Charge (h)"), QObject::tr("Disponible"), QObject::tr("Repos"),
        QObject::tr("Service")};
    for (int i = 0; i < entetes.size(); ++i)
        model->setHeaderData(i, Qt::Horizontal, entetes.at(i));
    return model;
}

QList<Employe> Employe::rechercher(const QString &filtre, const QString &tri,
                                   const QString &poste, const QString &statut)
{
    // Tri par charge fait en C++ (CHARGE est une colonne texte) ; tri par date en SQL.
    const QString orderBy = "date_embauche DESC, prenom ASC";

    QString sql = QString("SELECT %1 FROM employer WHERE 1=1").arg(COLONNES_EMPLOYE);

    if (!filtre.isEmpty())
        sql += " AND (UPPER(nom) LIKE UPPER(:f1) OR UPPER(prenom) LIKE UPPER(:f2) "
               "OR UPPER(courriel) LIKE UPPER(:f3) OR UPPER(poste) LIKE UPPER(:f4) "
               "OR TO_CHAR(id_emp) LIKE :f5 OR cin LIKE :f6)";
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
        for (const char *nom : {":f1", ":f2", ":f3", ":f4", ":f5", ":f6"})
            q.bindValue(QString::fromLatin1(nom), motif);
    }
    if (poste != "Tous les postes") q.bindValue(":poste", poste);

    if (q.exec()) {
        while (q.next())
            liste.append(depuisRequete(q));
    }
    q.finish();
    if (tri == "charge") {
        std::stable_sort(liste.begin(), liste.end(), [](const Employe &a, const Employe &b) {
            return a.getCharge() > b.getCharge();
        });
    }
    return liste;
}

QList<Employe> Employe::afficherTous()
{
    QList<Employe> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(QString("SELECT %1 FROM employer ORDER BY prenom, nom").arg(COLONNES_EMPLOYE));
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
    q.prepare("UPDATE employer SET disponible=:d WHERE id_emp=:id");
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
    if (courriel.trimmed().isEmpty())
        return false;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT COUNT(*) FROM employer WHERE UPPER(TRIM(courriel))=UPPER(:email)");
    q.bindValue(":email", courriel.trimmed());
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
    q.prepare("SELECT COUNT(*) FROM employer WHERE disponible=:d");
    q.bindValue(":d", disponible ? 1 : 0);
    int n = 0;
    if (q.exec() && q.next())
        n = q.value(0).toInt();
    q.finish();
    return n;
}

double Employe::chargeTotaleDisponibles()
{
    double total = 0.0;
    const QList<Employe> liste = listerDisponibles();
    for (const Employe &e : liste)
        total += e.getCharge();
    return total;
}

QList<Employe> Employe::listerDisponibles()
{
    QList<Employe> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(QString("SELECT %1 FROM employer WHERE disponible=:d").arg(COLONNES_EMPLOYE));
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
    q.prepare(QString("SELECT %1 FROM employer WHERE disponible=1 "
                      "AND UPPER(competences) LIKE UPPER(:c)")
                  .arg(COLONNES_EMPLOYE));
    q.bindValue(":c", "%" + competence.trimmed() + "%");
    if (q.exec()) {
        while (q.next())
            liste.append(depuisRequete(q));
    }
    q.finish();
    std::stable_sort(liste.begin(), liste.end(), [](const Employe &a, const Employe &b) {
        return a.tauxCharge() < b.tauxCharge();
    });
    return liste;
}

QList<Employe> Employe::listerDisponiblesParCharge(bool decroissant)
{
    QList<Employe> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare(QString("SELECT %1 FROM employer WHERE disponible=1").arg(COLONNES_EMPLOYE));
    if (q.exec()) {
        while (q.next()) {
            const Employe e = depuisRequete(q);
            if (e.getDispoRepos() > 0)
                liste.append(e);
        }
    }
    q.finish();
    std::stable_sort(liste.begin(), liste.end(), [decroissant](const Employe &a, const Employe &b) {
        return decroissant ? a.tauxCharge() > b.tauxCharge() : a.tauxCharge() < b.tauxCharge();
    });
    return liste;
}

// Charge (CHARGE) et disponibilite (DISPO_REPOS) de chaque employe : donnees
// de la fenetre Statistiques et de Performance & Evaluation.
QList<Employe::ChargeEmploye> Employe::charges()
{
    QList<ChargeEmploye> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT id_emp, nom, prenom, poste, charge, dispo_repos, disponible "
              "FROM employer ORDER BY prenom, nom");
    if (q.exec()) {
        while (q.next()) {
            ChargeEmploye c;
            c.id = q.value(0).toInt();
            c.nom = q.value(1).toString();
            c.prenom = q.value(2).toString();
            c.poste = q.value(3).toString();
            c.charge = lireNombre(q.value(4), 0.0);
            QString capaciteTexte = q.value(5).toString().trimmed();
            capaciteTexte.replace(QLatin1Char(','), QLatin1Char('.'));
            bool capaciteValide = false;
            const double capacite = capaciteTexte.toDouble(&capaciteValide);
            c.dispo = capaciteValide && capacite > 0.0 ? capacite : 35.0;
            c.disponible = q.value(6).toInt() == 1;
            liste.append(c);
        }
    }
    q.finish();
    std::stable_sort(liste.begin(), liste.end(), [](const ChargeEmploye &a, const ChargeEmploye &b) {
        return a.charge > b.charge;
    });
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

// ======================================================================
//  Table SERVICE (cle etrangere ID_SERV)
// ======================================================================
QList<Employe::ServiceInfo> Employe::listerServices()
{
    // Les catégories affichées reprennent exactement celles de la maquette.
    // Chaque choix est relié à un ID_SERV existant dans la table SERVICE.
    // Aucune ligne ni table Oracle n'est créée ici.
    const QStringList categoriesSouhaitees = {
        QStringLiteral("Consultation vétérinaire"),
        QStringLiteral("Vaccination"),
        QStringLiteral("Analyses de laboratoire"),
        QStringLiteral("Imagerie médicale"),
        QStringLiteral("Chirurgie vétérinaire")
    };

    QHash<QString, int> idParCategorie;
    QSet<QString> categoriesSupplementaires;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT id_serv, categorie FROM service "
              "WHERE categorie IS NOT NULL ORDER BY id_serv");
    if (q.exec()) {
        while (q.next()) {
            const QString libelle = q.value(1).toString().trimmed();
            if (libelle.isEmpty())
                continue;
            const QString cle = libelle.toCaseFolded();
            if (!idParCategorie.contains(cle))
                idParCategorie.insert(cle, q.value(0).toInt());
            categoriesSupplementaires.insert(libelle);
        }
    }
    q.finish();

    QList<ServiceInfo> liste;
    QSet<QString> dejaAjoutees;
    for (const QString &categorie : categoriesSouhaitees) {
        ServiceInfo info;
        info.libelle = categorie;
        info.id = idParCategorie.value(categorie.toCaseFolded(), 0);
        liste.append(info);
        dejaAjoutees.insert(categorie.toCaseFolded());
    }

    // Conserve aussi les autres catégories déjà présentes dans SERVICE.
    QStringList autres = categoriesSupplementaires.values();
    std::sort(autres.begin(), autres.end(), [](const QString &a, const QString &b) {
        return a.toCaseFolded() < b.toCaseFolded();
    });
    for (const QString &categorie : autres) {
        const QString cle = categorie.toCaseFolded();
        if (dejaAjoutees.contains(cle))
            continue;
        ServiceInfo info;
        info.libelle = categorie;
        info.id = idParCategorie.value(cle, 0);
        liste.append(info);
        dejaAjoutees.insert(cle);
    }
    return liste;
}

QString Employe::libelleService(int idServ)
{
    if (idServ <= 0)
        return QString();

    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT categorie FROM service WHERE id_serv=:id");
    q.bindValue(":id", idServ);
    QString libelle;
    if (q.exec() && q.next())
        libelle = q.value(0).toString().trimmed();
    q.finish();
    return libelle.isEmpty() ? QString("Service #%1").arg(idServ) : libelle;
}
