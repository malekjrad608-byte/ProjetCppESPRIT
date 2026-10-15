#include "medicalrecord.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

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

// ------------------------------------------------------------ Base de données

namespace {
void bindRecord(QSqlQuery &q, const MedicalRecord &r)
{
    q.bindValue(":record_id", r.medical.recordId);
    q.bindValue(":animal_id", r.animal.animalId);
    q.bindValue(":name", r.animal.name);
    q.bindValue(":species", r.animal.species);
    q.bindValue(":breed", r.animal.breed);
    q.bindValue(":age", r.animal.age);
    q.bindValue(":sex", r.animal.sex);
    q.bindValue(":owner", r.animal.owner);
    q.bindValue(":consult_date", r.medical.consultationDate);
    q.bindValue(":reason", r.medical.reason);
    q.bindValue(":diagnosis", r.medical.diagnosis);
    q.bindValue(":treatment", r.medical.treatment);
    q.bindValue(":vaccinations", r.medical.vaccinations);
    q.bindValue(":allergies", r.medical.allergies);
    q.bindValue(":weight", r.medical.weight);
    q.bindValue(":observations", r.medical.observations);
    q.bindValue(":vacc_status", r.medical.vaccinationStatus);
}

bool run(QSqlQuery &q, const char *what)
{
    if (q.exec())
        return true;
    qDebug() << what << "a échoué :" << q.lastError().text();
    return false;
}
}

QVector<MedicalRecord> MedicalRecord::afficher()
{
    QVector<MedicalRecord> list;
    QSqlQuery q;
    q.prepare("SELECT RECORD_ID, ANIMAL_ID, NAME, SPECIES, BREED, AGE, SEX, OWNER, "
              "CONSULT_DATE, REASON, DIAGNOSIS, TREATMENT, VACCINATIONS, ALLERGIES, "
              "WEIGHT, OBSERVATIONS, VACC_STATUS FROM MEDICAL_RECORD ORDER BY CONSULT_DATE DESC");
    if (!run(q, "SELECT"))
        return list;
    while (q.next()) {
        MedicalRecord r;
        r.medical.recordId = q.value(0).toString();
        r.animal.animalId = q.value(1).toString();
        r.animal.name = q.value(2).toString();
        r.animal.species = q.value(3).toString();
        r.animal.breed = q.value(4).toString();
        r.animal.age = q.value(5).toInt();
        r.animal.sex = q.value(6).toString();
        r.animal.owner = q.value(7).toString();
        r.medical.consultationDate = q.value(8).toDate();
        r.medical.reason = q.value(9).toString();
        r.medical.diagnosis = q.value(10).toString();
        r.medical.treatment = q.value(11).toString();
        r.medical.vaccinations = q.value(12).toString();
        r.medical.allergies = q.value(13).toString();
        r.medical.weight = q.value(14).toDouble();
        r.medical.observations = q.value(15).toString();
        r.medical.vaccinationStatus = q.value(16).toString();
        list.append(r);
    }
    return list;
}

bool MedicalRecord::ajouter() const
{
    QSqlQuery q;
    q.prepare("INSERT INTO MEDICAL_RECORD (RECORD_ID, ANIMAL_ID, NAME, SPECIES, BREED, AGE, SEX, OWNER, "
              "CONSULT_DATE, REASON, DIAGNOSIS, TREATMENT, VACCINATIONS, ALLERGIES, WEIGHT, "
              "OBSERVATIONS, VACC_STATUS) VALUES (:record_id, :animal_id, :name, :species, :breed, "
              ":age, :sex, :owner, :consult_date, :reason, :diagnosis, :treatment, :vaccinations, "
              ":allergies, :weight, :observations, :vacc_status)");
    bindRecord(q, *this);
    return run(q, "INSERT");
}

bool MedicalRecord::modifier() const
{
    QSqlQuery q;
    q.prepare("UPDATE MEDICAL_RECORD SET ANIMAL_ID=:animal_id, NAME=:name, SPECIES=:species, "
              "BREED=:breed, AGE=:age, SEX=:sex, OWNER=:owner, CONSULT_DATE=:consult_date, "
              "REASON=:reason, DIAGNOSIS=:diagnosis, TREATMENT=:treatment, VACCINATIONS=:vaccinations, "
              "ALLERGIES=:allergies, WEIGHT=:weight, OBSERVATIONS=:observations, "
              "VACC_STATUS=:vacc_status WHERE RECORD_ID=:record_id");
    bindRecord(q, *this);
    return run(q, "UPDATE");
}

bool MedicalRecord::supprimer(const QString &recordId)
{
    QSqlQuery q;
    q.prepare("DELETE FROM MEDICAL_RECORD WHERE RECORD_ID=:record_id");
    q.bindValue(":record_id", recordId);
    return run(q, "DELETE");
}
