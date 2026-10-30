#pragma once
#include <QList>
#include <QString>
#include "local.h"

// Une alerte automatique (local sale trop longtemps, local sature, ...)
struct Alert
{
    enum Level { Info = 0, Warning = 1, Critical = 2 };

    Level level = Info;
    QString kind;        // "dirty", "full", "almost-full", "global", "none-available"
    QString title;       // phrase complete  (liste des alertes)
    QString shortText;   // version courte    (carte / panneau de detail)
    QString detail;      // explication
    int localId = 0;     // 0 = alerte globale (toute la clinique)
    QString key;         // identifiant stable, sert a detecter les NOUVELLES alertes
};

namespace AlertEngine {

// Seuil "trop longtemps sale" en minutes (reglable, sauvegarde automatiquement).
// Les blocs et l'isolement ont un seuil deux fois plus court.
int dirtyThresholdMinutes();
void setDirtyThresholdMinutes(int minutes);
int thresholdFor(const Local &l);

QString formatMinutes(qint64 minutes);          // 95 -> "1 h 35 min"
QString priorityText(const Local &l);           // "URGENT", "Haute", "Normale", "En cours", "—"

// Analyse tous les locaux et retourne les alertes, les plus graves d'abord
QList<Alert> evaluate(const QList<Local> &locaux);

} // namespace AlertEngine
