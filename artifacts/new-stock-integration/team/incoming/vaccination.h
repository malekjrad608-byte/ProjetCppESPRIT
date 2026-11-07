#ifndef VACCINATION_H
#define VACCINATION_H

#include <QString>
#include <QStringList>
#include <QDate>
#include <QList>
#include <QVector>

#include "medicalrecord.h"

// ---------------------------------------------------------------------------
//  Fonctionnalité DM-I1 : Rappels vaccinaux
//  Une ligne de la table VACCINATION = un vaccin fait à un animal + sa date de rappel.
//  Pour un même (animal, vaccin) seule la ligne la plus récente compte.
// ---------------------------------------------------------------------------
struct Vaccination
{
    enum Etat { AJour, Bientot, EnRetard };

    QString idVaccin;
    QString idAnimal, nomAnimal, espece, proprietaire;
    QString telephone;          // numéro du propriétaire (SMS)
    QString nomVaccin;
    QDate dateVaccin;
    QDate dateRappel;
    QDate dateSms;              // dernier SMS réellement envoyé (invalide = jamais)

    int joursRestants(const QDate &today = QDate::currentDate()) const { return today.daysTo(dateRappel); }
    Etat etat(int joursAvant = 30, const QDate &today = QDate::currentDate()) const
    {
        const int d = joursRestants(today);
        return d < 0 ? EnRetard : (d <= joursAvant ? Bientot : AJour);
    }
};

struct VaccineType
{
    QString nom;
    int intervalleMois;
};

// Couche Modèle : toutes les requêtes SQL du module "rappels" sont ici.
// Hors ligne (pas de connexion) : données de démonstration gardées en mémoire.
class VaccinationManager
{
public:
    // Vaccins usuels par espèce (intervalle de rappel par défaut, modifiable à la saisie)
    static QList<VaccineType> catalogue(const QString &espece);
    static int intervalleParDefaut(const QString &espece, const QString &nomVaccin);

    // Dernière ligne de chaque (animal, vaccin). 'records' sert à créer la démo hors ligne.
    static QList<Vaccination> derniers(const QVector<MedicalRecord> &records);

    // Seulement ceux qui sont en retard ou à échéance dans 'joursAvant' jours, triés par urgence.
    static QList<Vaccination> aRappeler(const QVector<MedicalRecord> &records, int joursAvant = 30);

    static bool ajouter(Vaccination &v);                         // génère idVaccin
    static bool enregistrerTelephone(const QString &idVaccin, const QString &tel);
    static bool marquerSmsEnvoye(const QString &idVaccin, const QDate &date);

    // Statut "À jour" / "À renouveler" d'un animal d'après ses rappels (vide = aucun vaccin suivi)
    static QString statutAnimal(const QString &idAnimal, const QVector<MedicalRecord> &records);

private:
    static QList<Vaccination> tousLesVaccins();
    static QList<Vaccination> &demo(const QVector<MedicalRecord> &records);
    static QString prochainId(const QList<Vaccination> &existants);
};

#endif // VACCINATION_H
