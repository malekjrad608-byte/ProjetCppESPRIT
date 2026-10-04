#ifndef SALLE_H
#define SALLE_H

#include <QString>

// ============================================================
// Classe ABSTRAITE Salle (mère)
// ------------------------------------------------------------
// Toutes les salles héritent de cette classe.
// Elle contient les attributs communs à toutes les salles.
// ============================================================
class Salle {
protected:
    int id;                  // Identifiant unique
    QString nom;             // Nom de la salle
    int capacite;            // Capacité max (nb cages ou places)
    int occupationActuelle;  // Nombre d'animaux actuellement
    QString etat;            // "Libre", "Occupée", "Hors service"

public:
    // Constructeur
    Salle(int id, const QString& nom, int capacite);
    virtual ~Salle() {}

    // --- Getters ---
    int getId() const { return id; }
    QString getNom() const { return nom; }
    int getCapacite() const { return capacite; }
    int getOccupation() const { return occupationActuelle; }
    QString getEtat() const { return etat; }

    // --- Setters ---
    void setNom(const QString& n) { nom = n; }
    void setCapacite(int c) { capacite = c; }
    void setEtat(const QString& e) { etat = e; }

    // --- Méthodes métier ---
    bool estDisponible() const;
    bool affecterAnimal();
    void liberer();

    // --- Méthodes virtuelles pures (chaque fille DOIT les coder) ---
    virtual QString getType() const = 0;
    virtual QString afficherDetails() const = 0;

    // --- Sérialisation CSV ---
    virtual QString toCSV() const;
};

// ============================================================
// Classe FILLE 1 : SalleConsultation
// ============================================================
class SalleConsultation : public Salle {
private:
    QString specialite;  // "Généraliste", "Dentiste"...

public:
    SalleConsultation(int id, const QString& nom, int capacite,
                      const QString& specialite);

    QString getSpecialite() const { return specialite; }

    QString getType() const override { return "Consultation"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

// ============================================================
// Classe FILLE 2 : SalleChirurgie
// ============================================================
class SalleChirurgie : public Salle {
private:
    QString typeBloc;  // "Aseptique", "Standard"

public:
    SalleChirurgie(int id, const QString& nom, int capacite,
                   const QString& typeBloc);

    QString getTypeBloc() const { return typeBloc; }

    QString getType() const override { return "Chirurgie"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

// ============================================================
// Classe FILLE 3 : SalleToilettage
// ============================================================
class SalleToilettage : public Salle {
private:
    QString typeBain;  // "Classique", "Hydro-massage"

public:
    SalleToilettage(int id, const QString& nom, int capacite,
                    const QString& typeBain);

    QString getTypeBain() const { return typeBain; }

    QString getType() const override { return "Toilettage"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

// ============================================================
// Classe FILLE 4 : SallePension (avec cages)
// ============================================================
class SallePension : public Salle {
private:
    QString typeAnimal;  // "Chien", "Chat", "Mixte"

public:
    SallePension(int id, const QString& nom, int capacite,
                 const QString& typeAnimal);

    QString getTypeAnimal() const { return typeAnimal; }

    QString getType() const override { return "Pension"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

// ============================================================
// Classe FILLE 5 : SalleAdoption (avec cages)
// ============================================================
class SalleAdoption : public Salle {
private:
    QString description;  // Description de la salle

public:
    SalleAdoption(int id, const QString& nom, int capacite,
                  const QString& description);

    QString getDescription() const { return description; }

    QString getType() const override { return "Adoption"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

// ============================================================
// Classe FILLE 6 : SalleStorage (LECTURE SEULE)
// ------------------------------------------------------------
// Pas de gestion d'animaux, juste un espace de stockage
// ============================================================
class SalleStorage : public Salle {
private:
    QString contenu;  // "Médicaments", "Matériel"...

public:
    SalleStorage(int id, const QString& nom, int capacite,
                 const QString& contenu);

    QString getContenu() const { return contenu; }

    QString getType() const override { return "Storage"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

#endif // SALLE_H