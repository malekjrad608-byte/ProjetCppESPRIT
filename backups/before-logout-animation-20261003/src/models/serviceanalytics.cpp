#include "serviceanalytics.h"

ServiceMetrics ServiceMetrics::from(const QList<Service> &services, const QDateTime &now) {
    ServiceMetrics metrics;
    metrics.total = services.size();
    for (const auto &service : services) {
        ++metrics.categories[service.category];
        if (service.status == "Planifié") {
            ++metrics.planned;
            metrics.scheduledMinutes += service.duration;
            if (service.dateTime < now) ++metrics.overdue;
        } else if (service.status == "En cours") {
            ++metrics.inProgress;
            metrics.scheduledMinutes += service.duration;
        } else if (service.status == "Terminé") {
            ++metrics.completed;
            metrics.completedAmount += service.price;
        } else if (service.status == "Annulé") {
            ++metrics.cancelled;
        }
    }
    return metrics;
}
