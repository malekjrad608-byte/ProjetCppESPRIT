#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QString>
#include <QDate>
#include <QList>
#include <QSqlQuery>
#include <QSqlQueryModel>

// =======================================================================
//  MODELE (architecture Modele-Vue) : entite EMPLOYER (table Oracle existante)
//
//  Relation officielle :
//  EMPLOYER(ID_EMP pk, NOM, PRENOM, CIN unique, DATE_NAISS, ADRESSE, TEL,
//           POSTE, COURRIEL, COMPETENCES, DATE_EMBAUCHE, TYPE_CONTRAT,
//           SALAIRE, CHARGE, DISPONIBLE, DISPO_REPOS, #ID_SERV)
//  CIN, SALAIRE, CHARGE et DISPO_REPOS sont des VARCHAR2(20) dans Oracle : le
//  modele convertit SALAIRE et CHARGE en nombres. DISPO_REPOS conserve le jour
//  de repos saisi (ou une ancienne valeur numerique d'heures disponibles).
//
//  Toutes les requetes SQL (preparees : prepare() + bindValue()) liees aux
//  employes sont ici. Aucune requete ne doit etre ecrite derriere un bouton.
// =======================================================================
class Employe
{
public:
    // ---- Constructeurs ----
    Employe();
    Employe(int id, const QString &nom, const QString &prenom, const QString &cin,
            const QDate &dateNaiss, const QString &adresse, const QString &tel,
            const QString &poste, const QString &courriel, const QString &competences,
            const QDate &dateEmbauche, const QString &typeContrat, double salaire,
            double charge, bool disponible, double dispoRepos, int idServ);

    // ---- Structures de resultat (donnees pour la Vue) ----
    struct ChargeEmploye {
        int id = 0;
        QString nom;
        QString prenom;
        QString poste;
        double charge = 0.0;   // colonne CHARGE (heures / semaine)
        double dispo = 35.0;   // capacite de reference (heures / semaine)
        bool disponible = true;
    };
    struct StatsCharge {
        int total = 0;
        double somme = 0.0;
        double moyenne = 0.0;
        double maximum = 0.0;
        double minimum = 0.0;
    };
    struct ServiceInfo {       // categorie de SERVICE + ID_SERV correspondant
        int id = 0;
        QString libelle;
    };

    // ---- Getters ----
    int getId() const { return m_id; }
    QString getNom() const { return m_nom; }
    QString getPrenom() const { return m_prenom; }
    QString getCin() const { return m_cin; }
    QDate getDateNaiss() const { return m_dateNaiss; }
    QString getAdresse() const { return m_adresse; }
    QString getTel() const { return m_tel; }
    QString getPoste() const { return m_poste; }
    QString getCourriel() const { return m_courriel; }
    QString getCompetences() const { return m_competences; }
    QDate getDateEmbauche() const { return m_dateEmbauche; }
    QString getTypeContrat() const { return m_typeContrat; }
    double getSalaire() const { return m_salaire; }
    double getCharge() const { return m_charge; }
    bool isDisponible() const { return m_disponible; }
    // DISPO_REPOS accepte un jour de repos (ex. Dimanche) ou, pour les
    // anciennes fiches, un nombre d'heures disponibles. Les statistiques
    // utilisent 35 h comme capacite de reference si la valeur est textuelle.
    double getDispoRepos() const {
        bool ok = false;
        QString texte = m_dispoRepos;
        texte.replace(QLatin1Char(','), QLatin1Char('.'));
        const double valeur = texte.toDouble(&ok);
        return ok && valeur > 0.0 ? valeur : 35.0;
    }
    QString getDispoReposText() const { return m_dispoRepos; }
    int getIdServ() const { return m_idServ; }          // 0 = aucun service

    // ---- Setters ----
    void setId(int id) { m_id = id; }
    void setNom(const QString &n) { m_nom = n; }
    void setPrenom(const QString &p) { m_prenom = p; }
    void setCin(const QString &c) { m_cin = c; }
    void setDateNaiss(const QDate &d) { m_dateNaiss = d; }
    void setAdresse(const QString &a) { m_adresse = a; }
    void setTel(const QString &t) { m_tel = t; }
    void setPoste(const QString &p) { m_poste = p; }
    void setCourriel(const QString &c) { m_courriel = c; }
    void setCompetences(const QString &c) { m_competences = c; }
    void setDateEmbauche(const QDate &d) { m_dateEmbauche = d; }
    void setTypeContrat(const QString &t) { m_typeContrat = t; }
    void setSalaire(double s) { m_salaire = s; }
    void setCharge(double c) { m_charge = c; }
    void setDisponible(bool d) { m_disponible = d; }
    void setDispoRepos(double d) { m_dispoRepos = QString::number(d); }
    void setDispoRepos(const QString &d) { m_dispoRepos = d.trimmed(); }
    void setIdServ(int s) { m_idServ = s; }

    // ---- Logique metier ----
    double tauxCharge() const;
    bool depasseSeuil(double seuil = 0.8) const;

    // ---- Controle de saisie (C++) : retourne "" si tout est valide,
    //      sinon le message d'erreur a afficher a l'utilisateur ----
    QString validerSaisie(bool courrielObligatoire = false,
                           bool telephoneEtSalaireObligatoires = false) const;

    // ---- CRUD ----
    bool ajouter(QString *erreur = nullptr) const;               // Create
    bool modifier(QString *erreur = nullptr) const;              // Update
    static bool supprimer(int id, QString *erreur = nullptr);    // Delete
    static bool charger(int id, Employe &resultat);              // Read (un employe)
    static QSqlQueryModel *afficher();                           // Read (modele pour un QTableView)
    static QList<Employe> rechercher(const QString &filtre = QString(),
                                     const QString &tri = "date",
                                     const QString &poste = "Tous les postes",
                                     const QString &statut = "Tous");   // Read (liste filtree / cards)
    static QList<Employe> afficherTous();                        // Read (tous, tries par prenom/nom)

    // ---- Autres requetes liees a l'entite ----
    static bool basculerDisponibilite(int id, bool *nouveauStatut = nullptr, QString *erreur = nullptr);
    static bool existeCourriel(const QString &courriel);
    static int compterParDisponibilite(bool disponible);
    static double chargeTotaleDisponibles();
    static QList<Employe> listerDisponibles();
    static QList<Employe> rechercherParCompetence(const QString &competence);
    static QList<Employe> listerDisponiblesParCharge(bool decroissant);
    static QList<ChargeEmploye> charges();
    static StatsCharge statistiques(const QList<ChargeEmploye> &charges);

    // ---- Lien avec la table SERVICE (cle etrangere ID_SERV) ----
    static QList<ServiceInfo> listerServices();
    static QString libelleService(int idServ);

private:
    int m_id = 0;
    QString m_nom;
    QString m_prenom;
    QString m_cin;
    QDate m_dateNaiss;
    QString m_adresse;
    QString m_tel;
    QString m_poste;
    QString m_courriel;
    QString m_competences;
    QDate m_dateEmbauche;
    QString m_typeContrat;
    double m_salaire = 0.0;
    double m_charge = 0.0;
    bool m_disponible = true;
    QString m_dispoRepos;
    int m_idServ = 0;
};

#endif // EMPLOYE_H
