#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QString>
#include <QDate>
#include <QList>
#include <QSqlQuery>
#include <QSqlQueryModel>

// =======================================================================
//  MODELE (architecture Modele-Vue) : entite EMPLOYES
//  Toutes les requetes SQL (preparees : prepare() + bindValue()) liees aux
//  employes sont ici. Aucune requete ne doit etre ecrite derriere un bouton.
// =======================================================================
class Employe
{
public:
    // ---- Constructeurs (comme Etudiant() et Etudiant(int,QString,QString) de l'atelier) ----
    Employe();
    Employe(int id, const QString &nom, const QString &prenom, const QString &courriel,
            const QString &telephone, const QString &poste, const QDate &dateEmbauche,
            bool disponible, double chargeHebdo, double dispoHebdo, const QString &competences);

    // ---- Structures de resultat (donnees pour la Vue) ----
    struct ChargeEmploye {
        QString nom;
        QString prenom;
        QString poste;
        double charge = 0.0;   // charge_hebdo + heures planifiees de la semaine
        double dispo = 0.0;    // disponibilite hebdomadaire
    };
    struct StatsCharge {
        int total = 0;
        double somme = 0.0;
        double moyenne = 0.0;
        double maximum = 0.0;
        double minimum = 0.0;
    };

    // ---- Getters ----
    int getId() const { return m_id; }
    QString getNom() const { return m_nom; }
    QString getPrenom() const { return m_prenom; }
    QString getCourriel() const { return m_courriel; }
    QString getTelephone() const { return m_telephone; }
    QString getPoste() const { return m_poste; }
    QDate getDateEmbauche() const { return m_dateEmbauche; }
    bool isDisponible() const { return m_disponible; }
    double getChargeHebdo() const { return m_chargeHebdo; }
    double getDispoHebdo() const { return m_dispoHebdo; }
    QString getCompetences() const { return m_competences; }

    // ---- Setters ----
    void setId(int id) { m_id = id; }
    void setNom(const QString &n) { m_nom = n; }
    void setPrenom(const QString &p) { m_prenom = p; }
    void setCourriel(const QString &c) { m_courriel = c; }
    void setTelephone(const QString &t) { m_telephone = t; }
    void setPoste(const QString &p) { m_poste = p; }
    void setDateEmbauche(const QDate &d) { m_dateEmbauche = d; }
    void setDisponible(bool d) { m_disponible = d; }
    void setChargeHebdo(double c) { m_chargeHebdo = c; }
    void setDispoHebdo(double d) { m_dispoHebdo = d; }
    void setCompetences(const QString &c) { m_competences = c; }

    // ---- Logique metier ----
    double tauxCharge() const;
    bool depasseSeuil(double seuil = 0.8) const;

    // ---- Controle de saisie (C++) : retourne "" si tout est valide,
    //      sinon le message d'erreur a afficher a l'utilisateur ----
    QString validerSaisie() const;

    // ---- CRUD ----
    bool ajouter(QString *erreur = nullptr) const;               // Create
    bool modifier(QString *erreur = nullptr) const;              // Update
    static bool supprimer(int id, QString *erreur = nullptr);    // Delete
    static bool charger(int id, Employe &resultat);              // Read (un employe)
    static QSqlQueryModel *afficher();                           // Read (modele pour un QTableView, comme l'atelier)
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
    static QList<ChargeEmploye> chargesSemaine(const QDate &debutSemaine);
    static StatsCharge statistiques(const QList<ChargeEmploye> &charges);

private:
    int m_id = 0;
    QString m_nom;
    QString m_prenom;
    QString m_courriel;
    QString m_telephone;
    QString m_poste;
    QDate m_dateEmbauche;
    bool m_disponible = true;
    double m_chargeHebdo = 0.0;
    double m_dispoHebdo = 35.0;
    QString m_competences;
};

#endif // EMPLOYE_H
