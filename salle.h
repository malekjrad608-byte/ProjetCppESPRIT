#ifndef SALLE_H
#define SALLE_H

#include <QString>
#include <QtGlobal>

// ============================================================
// Classe ABSTRAITE Salle (mère)
// ============================================================
class Salle {
protected:
    int id;                  // Identifiant unique
    QString nom;             // Nom de la salle
    int capacite;            // Capacité max (nb cages ou places)
    int occupationActuelle;  // Nombre d'animaux actuellement
    QString etat;            // "Libre", "Occupée", "Hors service"

public:
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

    // Fixe l'occupation (bornée à la capacité) et garde l'état cohérent :
    // plein => "Occupée", sinon "Occupée" repasse "Libre". "Hors service" est conservé.
    void setOccupation(int o) {
        occupationActuelle = qBound(0, o, capacite);
        if (capacite > 0 && occupationActuelle >= capacite && etat == "Libre") etat = "Occupée";
        else if (occupationActuelle < capacite && etat == "Occupée") etat = "Libre";
    }

    // --- Méthodes métier ---
    bool estDisponible() const;
    bool affecterAnimal();
    void liberer();

    // --- Info spécifique (spécialité, bloc, bain, ...) modifiable ---
    virtual QString getInfo() const { return QString(); }
    virtual void setInfo(const QString&) {}

    // --- Méthodes virtuelles pures ---
    virtual QString getType() const = 0;
    virtual QString afficherDetails() const = 0;

    // --- Sérialisation CSV ---
    virtual QString toCSV() const;
};

// ---------------- SalleConsultation ----------------
class SalleConsultation : public Salle {
private:
    QString specialite;
public:
    SalleConsultation(int id, const QString& nom, int capacite, const QString& specialite);
    QString getSpecialite() const { return specialite; }
    QString getInfo() const override { return specialite; }
    void setInfo(const QString& v) override { specialite = v; }
    QString getType() const override { return "Consultation"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

// ---------------- SalleChirurgie ----------------
class SalleChirurgie : public Salle {
private:
    QString typeBloc;
public:
    SalleChirurgie(int id, const QString& nom, int capacite, const QString& typeBloc);
    QString getTypeBloc() const { return typeBloc; }
    QString getInfo() const override { return typeBloc; }
    void setInfo(const QString& v) override { typeBloc = v; }
    QString getType() const override { return "Chirurgie"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

// ---------------- SalleToilettage ----------------
class SalleToilettage : public Salle {
private:
    QString typeBain;
public:
    SalleToilettage(int id, const QString& nom, int capacite, const QString& typeBain);
    QString getTypeBain() const { return typeBain; }
    QString getInfo() const override { return typeBain; }
    void setInfo(const QString& v) override { typeBain = v; }
    QString getType() const override { return "Toilettage"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

// ---------------- SallePension (avec cages) ----------------
class SallePension : public Salle {
private:
    QString typeAnimal;
public:
    SallePension(int id, const QString& nom, int capacite, const QString& typeAnimal);
    QString getTypeAnimal() const { return typeAnimal; }
    QString getInfo() const override { return typeAnimal; }
    void setInfo(const QString& v) override { typeAnimal = v; }
    QString getType() const override { return "Pension"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

// ---------------- SalleAdoption (avec cages) ----------------
class SalleAdoption : public Salle {
private:
    QString description;
public:
    SalleAdoption(int id, const QString& nom, int capacite, const QString& description);
    QString getDescription() const { return description; }
    QString getInfo() const override { return description; }
    void setInfo(const QString& v) override { description = v; }
    QString getType() const override { return "Adoption"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

// ---------------- SalleStorage (LECTURE SEULE) ----------------
class SalleStorage : public Salle {
private:
    QString contenu;
public:
    SalleStorage(int id, const QString& nom, int capacite, const QString& contenu);
    QString getContenu() const { return contenu; }
    QString getInfo() const override { return contenu; }
    void setInfo(const QString& v) override { contenu = v; }
    QString getType() const override { return "Storage"; }
    QString afficherDetails() const override;
    QString toCSV() const override;
};

#endif // SALLE_H
