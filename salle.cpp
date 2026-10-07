#include "salle.h"

// ============================================================
// Salle (classe mère)
// ============================================================
Salle::Salle(int id, const QString& nom, int capacite)
    : id(id), nom(nom), capacite(capacite),
    occupationActuelle(0), etat("Libre") {}

bool Salle::estDisponible() const {
    return etat == "Libre" && occupationActuelle < capacite;
}

bool Salle::affecterAnimal() {
    if (!estDisponible()) return false;
    occupationActuelle++;
    if (occupationActuelle >= capacite) etat = "Occupée";
    return true;
}

void Salle::liberer() {
    if (occupationActuelle > 0) occupationActuelle--;
    if (occupationActuelle == 0 && etat == "Occupée") etat = "Libre";
}

QString Salle::toCSV() const {
    return QString("%1;%2;%3;%4;%5;%6")
    .arg(id).arg(nom).arg(getType())
        .arg(capacite).arg(occupationActuelle).arg(etat);
}

// ============================================================
// SalleConsultation
// ============================================================
SalleConsultation::SalleConsultation(int id, const QString& nom,
                                     int capacite, const QString& specialite)
    : Salle(id, nom, capacite), specialite(specialite) {}

QString SalleConsultation::afficherDetails() const {
    return QString("[Consultation] %1 — Spécialité : %2")
        .arg(nom).arg(specialite);
}

QString SalleConsultation::toCSV() const {
    return Salle::toCSV() + ";" + specialite;
}

// ============================================================
// SalleChirurgie
// ============================================================
SalleChirurgie::SalleChirurgie(int id, const QString& nom,
                               int capacite, const QString& typeBloc)
    : Salle(id, nom, capacite), typeBloc(typeBloc) {}

QString SalleChirurgie::afficherDetails() const {
    return QString("[Chirurgie] %1 — Bloc : %2")
        .arg(nom).arg(typeBloc);
}

QString SalleChirurgie::toCSV() const {
    return Salle::toCSV() + ";" + typeBloc;
}

// ============================================================
// SalleToilettage
// ============================================================
SalleToilettage::SalleToilettage(int id, const QString& nom,
                                 int capacite, const QString& typeBain)
    : Salle(id, nom, capacite), typeBain(typeBain) {}

QString SalleToilettage::afficherDetails() const {
    return QString("[Toilettage] %1 — Bain : %2")
        .arg(nom).arg(typeBain);
}

QString SalleToilettage::toCSV() const {
    return Salle::toCSV() + ";" + typeBain;
}

// ============================================================
// SallePension
// ============================================================
SallePension::SallePension(int id, const QString& nom,
                           int capacite, const QString& typeAnimal)
    : Salle(id, nom, capacite), typeAnimal(typeAnimal) {}

QString SallePension::afficherDetails() const {
    return QString("[Pension] %1 — Animal : %2 — %3/%4 cages")
        .arg(nom).arg(typeAnimal)
        .arg(occupationActuelle).arg(capacite);
}

QString SallePension::toCSV() const {
    return Salle::toCSV() + ";" + typeAnimal;
}

// ============================================================
// SalleAdoption
// ============================================================
SalleAdoption::SalleAdoption(int id, const QString& nom,
                             int capacite, const QString& description)
    : Salle(id, nom, capacite), description(description) {}

QString SalleAdoption::afficherDetails() const {
    return QString("[Adoption] %1 — %2 — %3/%4 cages")
        .arg(nom).arg(description)
        .arg(occupationActuelle).arg(capacite);
}

QString SalleAdoption::toCSV() const {
    return Salle::toCSV() + ";" + description;
}

// ============================================================
// SalleStorage
// ============================================================
SalleStorage::SalleStorage(int id, const QString& nom,
                           int capacite, const QString& contenu)
    : Salle(id, nom, capacite), contenu(contenu) {}

QString SalleStorage::afficherDetails() const {
    return QString("[Storage] %1 — Contenu : %2")
        .arg(nom).arg(contenu);
}

QString SalleStorage::toCSV() const {
    return Salle::toCSV() + ";" + contenu;
}