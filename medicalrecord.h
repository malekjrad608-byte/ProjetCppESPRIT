#ifndef MEDICALRECORD_H
#define MEDICALRECORD_H

#include <QString>
#include <QDate>
#include <QVector>

struct AnimalInfo {
    QString animalId;
    QString name;
    QString species;
    QString breed;
    int age;
    QString sex;
    QString owner;
};

struct MedicalInfo {
    QString recordId;
    QDate consultationDate;
    QString reason;
    QString diagnosis;
    QString treatment;
    QString vaccinations;
    QString allergies;
    double weight;
    QString observations;
    QString vaccinationStatus; // "À jour" or "À renouveler"
};

class MedicalRecord
{
public:
    MedicalRecord();
    MedicalRecord(const AnimalInfo& animal, const MedicalInfo& medical);

    AnimalInfo animal;
    MedicalInfo medical;

    QString getFullName() const;
    QString getVaccinationStatus() const;

    // --- Persistance base de données (table MEDICAL_RECORD) ---
    static QVector<MedicalRecord> afficher();      // SELECT *
    bool ajouter() const;                          // INSERT
    bool modifier() const;                         // UPDATE (clé : RECORD_ID)
    static bool supprimer(const QString& recordId); // DELETE

    bool operator==(const MedicalRecord& other) const;
};

#endif // MEDICALRECORD_H
