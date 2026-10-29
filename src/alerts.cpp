#include "alerts.h"

#include <QMap>
#include <QSettings>
#include <algorithm>

namespace AlertEngine {

int dirtyThresholdMinutes()
{
    QSettings s;
    return qBound(1, s.value("alerts/dirtyMinutes", 60).toInt(), 600);
}

void setDirtyThresholdMinutes(int minutes)
{
    QSettings s;
    s.setValue("alerts/dirtyMinutes", qBound(1, minutes, 600));
}

int thresholdFor(const Local &l)
{
    const int base = dirtyThresholdMinutes();
    return l.isUrgentType() ? qMax(1, base / 2) : base;
}

QString formatMinutes(qint64 m)
{
    if (m < 60)
        return QStringLiteral("%1 min").arg(m);
    const qint64 h = m / 60;
    const qint64 r = m % 60;
    return r == 0 ? QStringLiteral("%1 h").arg(h) : QStringLiteral("%1 h %2 min").arg(h).arg(r);
}

QString priorityText(const Local &l)
{
    if (l.getCleaning() == CleaningState::InProgress)
        return QStringLiteral("En cours");
    if (l.getCleaning() != CleaningState::ToClean)
        return QStringLiteral("—");
    const qint64 m = l.dirtyMinutes();
    const int th = thresholdFor(l);
    if (l.isUrgentType() || m >= 2 * th)
        return QStringLiteral("URGENT");
    if (m >= th)
        return QStringLiteral("Haute");
    return QStringLiteral("Normale");
}

QList<Alert> evaluate(const QList<Local> &all)
{
    QList<Alert> out;
    int cap = 0, occ = 0;
    QMap<QString, int> roomsOfType;
    QMap<QString, int> freeOfType;

    for (const Local &l : all) {
        roomsOfType[l.getType()] += 1;
        if (!l.getOutOfService() && l.getOccupancy() < l.getCapacity())
            freeOfType[l.getType()] += 1;
        if (!l.getOutOfService()) {
            cap += l.getCapacity();
            occ += l.getOccupancy();
        }

        // 1) local sale depuis trop longtemps
        if (l.getCleaning() == CleaningState::ToClean) {
            const qint64 m = l.dirtyMinutes();
            const int th = thresholdFor(l);
            if (m >= th) {
                Alert a;
                a.level = (m >= 2 * th || l.isUrgentType()) ? Alert::Critical : Alert::Warning;
                a.kind = "dirty";
                a.localId = l.getId();
                a.title = QStringLiteral("%1 est sale depuis %2").arg(l.getName(), formatMinutes(m));
                a.shortText = QStringLiteral("Sale depuis %1").arg(formatMinutes(m));
                a.detail = QStringLiteral("Seuil : %1 min%2. À nettoyer en priorité.")
                               .arg(th)
                               .arg(l.isUrgentType() ? QStringLiteral(" (local stérile)") : QString());
                a.key = QStringLiteral("dirty:%1:%2").arg(l.getId()).arg(int(a.level));
                out.append(a);
            }
        }

        // 2) local sature (plusieurs places seulement)
        if (!l.getOutOfService() && l.getCapacity() > 1) {
            if (l.getOccupancy() >= l.getCapacity()) {
                Alert a;
                a.level = Alert::Warning;
                a.kind = "full";
                a.localId = l.getId();
                a.title = QStringLiteral("%1 est complet (%2/%3)").arg(l.getName()).arg(l.getOccupancy()).arg(l.getCapacity());
                a.shortText = QStringLiteral("Complet (%1/%2)").arg(l.getOccupancy()).arg(l.getCapacity());
                a.detail = QStringLiteral("Plus aucune place libre : orienter les nouveaux animaux ailleurs.");
                a.key = QStringLiteral("full:%1").arg(l.getId());
                out.append(a);
            } else if (l.getOccupancy() * 100 >= l.getCapacity() * 85) {
                Alert a;
                a.level = Alert::Info;
                a.kind = "almost-full";
                a.localId = l.getId();
                a.title = QStringLiteral("%1 est presque plein (%2/%3)").arg(l.getName()).arg(l.getOccupancy()).arg(l.getCapacity());
                a.shortText = QStringLiteral("Presque plein (%1/%2)").arg(l.getOccupancy()).arg(l.getCapacity());
                a.detail = QStringLiteral("Il ne reste que %1 place(s).").arg(l.getCapacity() - l.getOccupancy());
                a.key = QStringLiteral("almost:%1").arg(l.getId());
                out.append(a);
            }
        }
    }

    // 3) clinique entiere presque saturee
    if (cap > 0) {
        const int pct = qRound(100.0 * occ / cap);
        if (pct >= 70) {
            Alert a;
            a.level = pct >= 85 ? Alert::Critical : Alert::Warning;
            a.kind = "global";
            a.title = QStringLiteral("La clinique est occupée à %1 %").arg(pct);
            a.shortText = a.title;
            a.detail = QStringLiteral("%1 animaux pour %2 places disponibles.").arg(occ).arg(cap);
            a.key = QStringLiteral("global:%1").arg(int(a.level));
            out.append(a);
        }
    }

    // 4) plus aucun local disponible pour un type important
    const QStringList watched = { "Consultation", "Chirurgie", "Hospitalisation", "Isolement" };
    for (const QString &t : watched) {
        if (roomsOfType.value(t) > 0 && freeOfType.value(t) == 0) {
            Alert a;
            a.level = (t == "Chirurgie" || t == "Isolement") ? Alert::Critical : Alert::Warning;
            a.kind = "none-available";
            a.title = QStringLiteral("Plus aucun local « %1 » disponible").arg(t);
            a.shortText = a.title;
            a.detail = QStringLiteral("Tous les locaux de ce type sont occupés, complets ou hors service.");
            a.key = QStringLiteral("none:%1").arg(t);
            out.append(a);
        }
    }

    std::stable_sort(out.begin(), out.end(),
                     [](const Alert &a, const Alert &b) { return a.level > b.level; });
    return out;
}

} // namespace AlertEngine
