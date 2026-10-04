  #ifndef GESTIONNAIRESALLES_H
#define GESTIONNAIRESALLES_H

#include <QVector>
#include <QString>
#include <QMap>
#include "salle.h"
#include "cage.h"

// ============================================================
// Structure Statistiques
// ============================================================
struct Statistiques {
    int totalSalles = 0;
    int sallesLibres = 0;
    int sallesOccupees = 0;
    int sallesHorsService = 0;
    int capaciteTotale = 0;
    int occupationTotale = 0;
    double tauxOccupation = 0.0;
    QMap<QString, int> repartitionParType;
};

// ============================================================
// GestionnaireSalles
// ------------------------------------------------------------
// Gère toutes les salles + les cages de pension/adoption
// ============================================================
class GestionnaireSalles {
private:
    QVector<Salle*> salles;
    QVector<Cage> cagesPension;   // 20 cages
    QVector<Cage> cagesAdoption;  // 20 cages
    QString cheminSauvegarde;     // Fichier CSV par défaut

public:
    GestionnaireSalles();
    ~GestionnaireSalles();

    // --- CRUD ---
    bool ajouter(Salle* s);
    bool supprimer(int id);
    Salle* rechercherParId(int id) const;

    // --- Recherche & Tri ---
    QVector<Salle*> rechercher(const QString& critere) const;
    QVector<Salle*> trierParCapacite(bool ascendant = true) const;
    QVector<Salle*> trierParOccupation(bool ascendant = true) const;

    // --- Métier ---
    bool affecterAnimal(int idSalle);
    bool libererSalle(int idSalle);
    bool changerEtat(int idSalle, const QString& nouvelEtat);

    // --- Cages ---
    QVector<Cage>& getCagesPension() { return cagesPension; }
    QVector<Cage>& getCagesAdoption() { return cagesAdoption; }
    bool occuperCagePension(int numero, const QString& nom,
                            const QString& type, const QDate& date, int jours);
    bool occuperCageAdoption(int numero, const QString& nom,
                             const QString& type, const QDate& date, int jours);
    bool libererCagePension(int numero);
    bool libererCageAdoption(int numero);

    // --- Stats ---
    Statistiques calculerStatistiques() const;

    // --- Persistance ---
    void setCheminSauvegarde(const QString& chemin) { cheminSauvegarde = chemin; }
    QString getCheminSauvegarde() const { return cheminSauvegarde; }
    bool sauvegarderCSV(const QString& chemin) const;
    bool sauvegarderAuto() const;  // Sauvegarde automatique
    bool chargerCSV(const QString& chemin);
    bool exporterPDF(const QString& chemin) const;

    // --- Accès ---
    QVector<Salle*> getToutesLesSalles() const { return salles; }
    int taille() const { return salles.size(); }

private:
    void vider();
    void initialiserCages();  // Crée les 20 cages au démarrage
};

#endif // GESTIONNAIRESALLES_H