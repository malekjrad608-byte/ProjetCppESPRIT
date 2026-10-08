#pragma once
#include <QString>
#include <QMap>
#include <QVector>

struct MedicalAnalytics {
    int totalAnimals = 0;
    int totalRecords = 0;
    int totalVaccinations = 0;
    int vaccinationsDueSoon = 0;
    int vaccinationsOverdue = 0;

    QMap<QString, int> animalsBySpecies;
    QMap<QString, int> recordsByType;
    QMap<QString, int> recordsByStatus;

    QVector<QString> topSpecies;
    QVector<QString> recentAnimals;

    double totalCost = 0.0;
    int recordsThisMonth = 0;
    int recordsThisYear = 0;

    void clear() {
        totalAnimals = 0;
        totalRecords = 0;
        totalVaccinations = 0;
        vaccinationsDueSoon = 0;
        vaccinationsOverdue = 0;
        animalsBySpecies.clear();
        recordsByType.clear();
        recordsByStatus.clear();
        topSpecies.clear();
        recentAnimals.clear();
        totalCost = 0.0;
        recordsThisMonth = 0;
        recordsThisYear = 0;
    }
};
