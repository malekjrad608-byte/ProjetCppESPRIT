#pragma once
#include <QString>
#include <QDateTime>

struct Service {
    int id = 0;
    QString name;
    QString description;
    QString responsible;
    QString category = QStringLiteral("Soins & bien-être");
    QDateTime dateTime = QDateTime::currentDateTime();
    QString status = QStringLiteral("Planifié");
    int duration = 30;
    double price = 0;
};
