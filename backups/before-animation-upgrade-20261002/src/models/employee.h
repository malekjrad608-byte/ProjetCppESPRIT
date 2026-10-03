#pragma once
#include <QString>

class Employee {
public:
    int     id         = 0;
    QString firstName;
    QString lastName;
    QString poste;
    QString phone;
    QString email;

    Employee() = default;
    Employee(int id, const QString &firstName, const QString &lastName,
             const QString &poste, const QString &phone, const QString &email)
        : id(id), firstName(firstName), lastName(lastName),
          poste(poste), phone(phone), email(email) {}

    QString fullName() const { return firstName + " " + lastName; }
};
