#include "medicalrecord.h"

MedicalRecord::MedicalRecord()
{
    animal.age = 0;
    medical.weight = 0.0;
}

MedicalRecord::MedicalRecord(const AnimalInfo& animalInfo, const MedicalInfo& medicalInfo)
    : animal(animalInfo), medical(medicalInfo)
{
}

QString MedicalRecord::getFullName() const
{
    return animal.name;
}

QString MedicalRecord::getVaccinationStatus() const
{
    return medical.vaccinationStatus;
}

bool MedicalRecord::operator==(const MedicalRecord& other) const
{
    return medical.recordId == other.medical.recordId;
}
