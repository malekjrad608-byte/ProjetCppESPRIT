#ifndef MEDICALRECORD_H
#define MEDICALRECORD_H

#include <QString>
#include <QDate>

struct AnimalInfo {
    QString animalId;
    QString name;
    QString species;
    QString breed;
    int age = 0;
    QString sex;
    QString owner;
};

struct MedicalInfo {
    QString recordId;
    int localId = 0;
    QDate consultationDate;
    QString reason;
    QString diagnosis;
    QString treatment;
    QString vaccinations;
    QString allergies;
    double weight = 0;
    QString observations;
    QString vaccinationStatus; // "À jour" or "À renouveler"
};

class MedicalRecord
{
public:
    MedicalRecord();
    MedicalRecord(const AnimalInfo& animal, const MedicalInfo& medical);

    qint64 storageId = 0;
    AnimalInfo animal;
    MedicalInfo medical;

    QString getFullName() const;
    QString getVaccinationStatus() const;

    bool operator==(const MedicalRecord& other) const;
};

#endif // MEDICALRECORD_H
