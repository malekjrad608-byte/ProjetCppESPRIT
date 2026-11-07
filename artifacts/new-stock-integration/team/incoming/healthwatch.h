#ifndef HEALTHWATCH_H
#define HEALTHWATCH_H

#include <QString>
#include <QStringList>
#include <QDate>
#include <QList>
#include <QVector>

#include "medicalrecord.h"

// ---------------------------------------------------------------------------
//  Veille sanitaire : analyse automatique des diagnostics des dossiers médicaux.
//   - Récidive : le même animal revient avec le même diagnostic (>= 2 passages dans les 6 derniers mois).
//   - Foyer possible : le même diagnostic chez plusieurs animaux différents sur une courte
//     période (>= 3 animaux en 14 jours) : risque de maladie contagieuse.
//  Aucune table supplémentaire : les alertes sont recalculées à partir des dossiers.
//  Les alertes déjà consultées par le médecin sont mémorisées (QSettings) pour ne pas
//  réapparaître ; une nouvelle consultation les fait revenir (nouvelle clé).
//  Il s'agit d'une aide à la décision : le vétérinaire valide toujours.
// ---------------------------------------------------------------------------
struct HealthAlert
{
    enum Type  { Recidive, Foyer };
    enum Level { Attention, Eleve };

    Type type = Recidive;
    Level level = Attention;
    QString cle;                 // identifiant stable (pour "déjà vu")
    QString diagnostic;          // libellé du diagnostic (famille regroupée)
    QString resume;              // titre court
    QStringList animaux;         // "Rex (ANI-001)"
    QStringList idAnimaux;
    QStringList especes;
    QList<QDate> dates;          // dates des consultations concernées (triées)
    QString conseil;             // pistes à valider par le vétérinaire
};

class HealthWatch
{
public:
    // Réglages (modifiables)
    static constexpr int kRecidiveJours = 180;     // fenêtre de récidive : les 180 derniers jours jusqu'à aujourd'hui
    static constexpr int kRecidiveMin = 2;         // nombre de passages pour une récidive
    static constexpr int kFoyerJours = 14;         // fenêtre de foyer
    static constexpr int kFoyerMin = 3;            // nombre d'animaux différents
    static constexpr int kFoyerEleve = 5;          // à partir de ce nombre : niveau élevé
    static constexpr int kActifJours = 30;         // foyer : actif tant que le dernier cas date de moins de 30 jours

    // Alertes actives, les plus graves d'abord
    static QList<HealthAlert> analyser(const QVector<MedicalRecord> &records, const QDate &today = QDate::currentDate());

    static QList<HealthAlert> nonVues(const QList<HealthAlert> &alertes);
    static void marquerVues(const QList<HealthAlert> &alertes);
};

#endif // HEALTHWATCH_H
