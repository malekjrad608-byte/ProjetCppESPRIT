#ifndef PLANNING_H
#define PLANNING_H

#include <QString>
#include <QDate>
#include <QDateTime>
#include <QList>

// =======================================================================
//  MODELE (architecture Modele-Vue) : entite PLANNING_AFFECTATIONS
//  Toutes les requetes SQL (preparees) liees au planning sont ici.
// =======================================================================
class Planning
{
public:
    Planning();

    // ---- Structures de resultat (donnees pour la Vue) ----
    struct Conflit {
        QString employe;
        QString titre1, debut1, fin1;
        QString titre2, debut2, fin2;
    };
    struct Plage {
        int responsableId = 0;
        QString prenom;
        QString nom;
        QDateTime debut;
        QDateTime fin;
    };
    enum class Resultat { Ok, AucunResponsable, ErreurBase };

    // ---- Getters / Setters ----
    QString getTitre() const { return m_titre; }
    QString getCompetence() const { return m_competence; }
    QDateTime getDebut() const { return m_debut; }
    QDateTime getFin() const { return m_fin; }
    double getHeures() const { return m_heures; }
    int getResponsableId() const { return m_responsableId; }

    void setTitre(const QString &t) { m_titre = t; }
    void setCompetence(const QString &c) { m_competence = c; }
    void setDebut(const QDateTime &d) { m_debut = d; }
    void setFin(const QDateTime &f) { m_fin = f; }
    void setHeures(double h) { m_heures = h; }
    void setResponsableId(int id) { m_responsableId = id; }

    // ---- Controle de saisie (C++) : "" si valide, sinon message d'erreur ----
    QString validerSaisie() const;

    // ---- CRUD / metier ----
    bool ajouter(QString *erreur = nullptr) const;
    // Choisit l'employe disponible le moins charge (competence, sans conflit,
    // capacite suffisante) puis enregistre l'affectation.
    Resultat affecterAutomatiquement(QString &nomResponsable, double &tauxPrevu,
                                     QString &erreur);

    // ---- Utilitaires de semaine ----
    static QDate semaineDebut(const QDate &date = QDate::currentDate());
    static QString borneSemaine(const QDate &date);

    // ---- Requetes de lecture ----
    static QList<Conflit> conflits();
    static QList<Plage> plagesAPartirDe(const QDate &date);
    static double chargePlanifieeSemaine(const QDate &debutSemaine);
    static bool chargePlanifieeEmploye(int id, const QDate &debutSemaine, double &heures);
    static bool aConflit(int id, const QDateTime &debut, const QDateTime &fin);

private:
    QString m_titre;
    QString m_competence;
    QDateTime m_debut;
    QDateTime m_fin;
    double m_heures = 1.0;
    int m_responsableId = 0;
};

#endif // PLANNING_H
