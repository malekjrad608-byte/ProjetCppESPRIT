#pragma once
#include <QString>
#include <QDate>
#include <QVariant>

struct Animal {
    int id = 0;
    QString name;
    QString species;      // Chien, Chat, etc.
    QString breed;
    QString gender;      // Mâle, Femelle
    QDate birthDate;
    QString color;
    QString ownerId;     // Propriétaire
    QString ownerPhone;
    QString ownerEmail;
    QString notes;
    QDateTime createdAt;
    QDateTime updatedAt;

    bool isEmpty() const { return id == 0 && name.isEmpty(); }
    int getAge() const {
        if (!birthDate.isValid()) return 0;
        return QDate::currentDate().year() - birthDate.year();
    }
};
