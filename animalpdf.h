#ifndef ANIMALPDF_H
#define ANIMALPDF_H

#include <QString>
#include <QVector>
#include "medicalrecord.h"

// Fiche PDF d'un animal : identité, résumé (nombre de visites, dernière visite, prochain RDV),
// historique complet des consultations, vaccinations et rendez-vous.
class AnimalPdf
{
public:
    // Retourne false si le fichier ne peut pas être écrit
    static bool exporter(const QString &idAnimal, const QVector<MedicalRecord> &records, const QString &path);
    static QString nomFichierPropose(const QString &idAnimal, const QVector<MedicalRecord> &records);
};

#endif // ANIMALPDF_H
