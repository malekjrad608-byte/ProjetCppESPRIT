#include "serviceanalytics.h"

ServiceMetrics ServiceMetrics::from(const QList<Service> &services, const QDateTime &now) {
    ServiceMetrics metrics;
    metrics.total = services.size();
    for (const auto &service : services) {
        ++metrics.categories[service.getCategory()];
        if (service.getStatus() == "Planifié") {
            ++metrics.planned;
            metrics.scheduledMinutes += service.getDuration();
            if (service.getDateTime() < now) ++metrics.overdue;
        } else if (service.getStatus() == "En cours") {
            ++metrics.inProgress;
            metrics.scheduledMinutes += service.getDuration();
        } else if (service.getStatus() == "Terminé") {
            ++metrics.completed;
            metrics.completedAmount += service.getPrice();
        } else if (service.getStatus() == "Annulé") {
            ++metrics.cancelled;
        }
    }
    return metrics;
}
