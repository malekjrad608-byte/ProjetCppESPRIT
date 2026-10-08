#pragma once
#include <QSqlDatabase>
#include <QList>
#include "../models/service.h"
#include "../models/animal.h"
#include "../models/medicalrecord.h"
#include "../models/medicalanalytics.h"

// Local service storage for the interface phase. No server or credentials required.
class DatabaseManager {
public:
    static DatabaseManager &instance();
    bool initialize(const QString &path);
    bool isConnected() const { return m_db.isOpen(); }
    QString lastError() const { return m_error; }
    QString storagePath() const { return m_db.databaseName(); }
    int lastInsertedId() const { return m_lastInsertedId; }

    // Services
    QList<Service> getAllServices();
    bool addService(const Service &service);
    bool updateService(const Service &service);
    bool deleteService(int id);
    bool addExamples();

    // Animals
    QList<Animal> getAllAnimals();
    Animal getAnimalById(int id);
    bool addAnimal(const Animal &animal);
    bool updateAnimal(const Animal &animal);
    bool deleteAnimal(int id);
    QList<Animal> searchAnimals(const QString &query);

    // Medical Records
    QList<MedicalRecord> getMedicalRecords(int animalId);
    QList<MedicalRecord> getAllMedicalRecords();
    bool addMedicalRecord(const MedicalRecord &record);
    bool updateMedicalRecord(const MedicalRecord &record);
    bool deleteMedicalRecord(int id);

    // Vaccinations
    QList<Vaccination> getVaccinations(int animalId);
    QList<Vaccination> getAllVaccinations();
    QList<Vaccination> getVaccinationsDueSoon(int days = 7);
    QList<Vaccination> getVaccinationsOverdue();
    bool addVaccination(const Vaccination &vaccination);
    bool updateVaccination(const Vaccination &vaccination);
    bool deleteVaccination(int id);

    // Analytics
    MedicalAnalytics getMedicalAnalytics();

    // Users
    bool verifyUserId(const QString &userId);
    bool addDefaultUsers();

private:
    DatabaseManager() = default;
    bool save(const Service &service, bool update);
    bool saveAnimal(const Animal &animal, bool update);
    bool saveMedicalRecord(const MedicalRecord &record, bool update);
    bool saveVaccination(const Vaccination &vaccination, bool update);
    QSqlDatabase m_db;
    QString m_error;
    int m_lastInsertedId = 0;
};
