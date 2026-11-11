#ifndef TREATMENTADVISOR_H
#define TREATMENTADVISOR_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QVector>
#include "medicalrecord.h"
#include "lookupdata.h"

// ---------------------------------------------------------------------------
//  Diagnostic intelligent : dès que le vétérinaire saisit un diagnostic, on
//  analyse l'état de l'animal (espèce, âge, poids, allergies, historique) et on
//  propose directement les traitements les plus utilisés pour ce diagnostic.
//
//  Méthode : statistiques sur les dossiers déjà enregistrés (aucun service externe) :
//   - dossiers au diagnostic identique/proche  -> traitements prescrits, classés par fréquence
//   - même espèce  = poids x1, autre espèce = x0,35 ; même animal = x1,5
//   - traitements en conflit avec une allergie connue = signalés et classés en dernier
//   - disponibilité en stock affichée
//  Les suggestions sont une aide à la décision : le vétérinaire valide toujours.
// ---------------------------------------------------------------------------
struct TreatmentSuggestion
{
    QString nom;
    double score = 0;
    int percent = 0;                 // part des cas similaires où ce traitement a été prescrit
    int nbCas = 0;                   // nombre de dossiers similaires concernés
    bool dejaPourCetAnimal = false;
    bool allergie = false;           // en conflit avec une allergie de l'animal
    QString allergene;
    int stock = -1;                  // -1 = inconnu, 0 = rupture, >0 = quantité disponible
    bool baseConnaissance = false;   // proposition de base (pas assez d'historique)
    QString raison;                  // phrase affichée sous le nom
};

class TreatmentAdvisor
{
public:
    static QList<TreatmentSuggestion> suggerer(const QString &diagnostic, const QString &espece,
                                               const QString &idAnimal, const QString &allergies,
                                               const QVector<MedicalRecord> &records,
                                               const QList<LookupItem> &stock,
                                               const QString &dossierExclu = QString(),
                                               int maxResultats = 6);

    // Résumé de l'état de l'animal (lignes prêtes à afficher)
    static QStringList etatAnimal(const QString &idAnimal, const QString &espece, int age, double poids,
                                  const QString &allergies, const QVector<MedicalRecord> &records);

    static QString normaliser(const QString &texte);   // minuscules, sans accents
};

#endif // TREATMENTADVISOR_H
