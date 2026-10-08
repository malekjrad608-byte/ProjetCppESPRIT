#pragma once
#include <QString>
#include <QDate>

class Schedule {
public:
    int     id         = 0;
    int     employeeId = 0;
    QString employeeName;
    QDate   date;
    QString shift;     // "Matin","Après-midi","Nuit"
    QString note;

    Schedule() = default;
    Schedule(int id, int employeeId, const QString &employeeName,
             const QDate &date, const QString &shift, const QString &note)
        : id(id), employeeId(employeeId), employeeName(employeeName),
          date(date), shift(shift), note(note) {}
};
