#ifndef CAGE_H
#define CAGE_H

#include <QString>
#include <QDate>

// ============================================================
// Classe Cage
// ------------------------------------------------------------
// Représente une cage individuelle dans une salle Pension
// ou Adoption. Chaque cage peut être libre ou occupée.
// ============================================================
class Cage {
private:
    int numero;              // Numéro de la cage (1, 2, 3...)
    bool occupee;            // Occupée ou libre
    QString nomAnimal;       // Nom de l'animal (ex: "Max")
    QString typeAnimal;      // "Chien", "Chat", "Oiseau"...
    QDate dateArrivee;       // Date d'arrivée
    int nombreJours;         // Nombre de jours de séjour

public:
    // Constructeur : cage vide par défaut
    Cage(int numero);

    // --- Getters ---
    int getNumero() const { return numero; }
    bool estOccupee() const { return occupee; }
    QString getNomAnimal() const { return nomAnimal; }
    QString getTypeAnimal() const { return typeAnimal; }
    QDate getDateArrivee() const { return dateArrivee; }
    int getNombreJours() const { return nombreJours; }

    // --- Séjour : date de sortie, jours restants, dépassement ---
    QDate dateSortie() const { return occupee ? dateArrivee.addDays(nombreJours) : QDate(); }
    int joursRestants() const { return occupee ? QDate::currentDate().daysTo(dateSortie()) : 0; }
    bool sejourDepasse() const { return occupee && joursRestants() < 0; }

    // --- Méthodes métier ---
    void occuper(const QString& nom, const QString& type,
                 const QDate& date, int jours);
    void liberer();

    // --- Sérialisation ---
    QString toCSV() const;
};

#endif // CAGE_H
