#pragma once
#include <QString>
#include <QDate>
#include <QDateTime>

struct MedicalRecord {
    int id = 0;
    int animalId = 0;
    QString type;         // Consultation, Vaccination, Chirurgie, Examen, etc.
    QString description;
    QString veterinarian;
    QDateTime date;
    QString status;       // En cours, Terminé, Annulé
    QString notes;
    QString diagnosis;
    QString treatment;
    double cost = 0.0;
    QDateTime createdAt;
    QDateTime updatedAt;

    bool isEmpty() const { return id == 0 && animalId == 0; }
};

struct Vaccination {
    int id = 0;
    int animalId = 0;
    QString vaccineName;
    QString vaccineType;
    QDate administeredDate;
    QDate nextDueDate;
    QString veterinarian;
    QString batchNumber;
    QString notes;
    bool isReminderSet = false;
    QDateTime createdAt;
    QDateTime updatedAt;

    bool isOverdue() const {
        return nextDueDate.isValid() && nextDueDate < QDate::currentDate();
    }

    bool isDueSoon(int days = 7) const {
        if (!nextDueDate.isValid()) return false;
        const int daysUntil = QDate::currentDate().daysTo(nextDueDate);
        return daysUntil >= 0 && daysUntil <= days;
    }
};
