#pragma once
#include "service.h"
#include <QList>
#include <QMap>

// Independent of widgets and storage so the service module can be integrated later.
struct ServiceMetrics {
    int total = 0;
    int planned = 0;
    int inProgress = 0;
    int completed = 0;
    int cancelled = 0;
    int overdue = 0;
    int scheduledMinutes = 0;
    double completedAmount = 0;
    QMap<QString, int> categories;
    static ServiceMetrics from(const QList<Service> &services,
                               const QDateTime &now = QDateTime::currentDateTime());
};
