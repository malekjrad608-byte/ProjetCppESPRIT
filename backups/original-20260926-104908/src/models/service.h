#pragma once
#include <QString>
#include <QDateTime>

class Service {
public:
    int         id          = 0;
    QString     name;
    QString     description;
    QString     responsible;   // employee name
    int         employeeId  = 0;
    QDateTime   dateTime;
    QString     status;        // "Planifié","En cours","Terminé","Annulé"

    Service() = default;
    Service(int id, const QString &name, const QString &description,
            const QString &responsible, int employeeId,
            const QDateTime &dateTime, const QString &status)
        : id(id), name(name), description(description),
          responsible(responsible), employeeId(employeeId),
          dateTime(dateTime), status(status) {}
};
