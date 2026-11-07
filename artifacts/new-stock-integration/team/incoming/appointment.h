#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <QString>
#include <QDate>
#include <QDateTime>
#include <QList>
#include <QVector>

#include "medicalrecord.h"

// ---------------------------------------------------------------------------
//  Rendez-vous (RDV) d'un animal + rappel automatique par SMS au propriétaire.
//  Une ligne de la table RENDEZVOUS = un rendez-vous.
//  Statuts : "Prévu", "Venu" (une consultation a eu lieu), "Manqué" (date dépassée sans consultation),
//            "Annulé" (remplacé par un nouveau rendez-vous).
//  SMS (un seul de chaque type par rendez-vous) :
//    - SMS de rappel : la veille ou le jour du rendez-vous
//    - SMS "rendez-vous manqué" : le lendemain si l'animal n'est pas venu
// ---------------------------------------------------------------------------
struct RendezVous
{
    QString idRdv;
    QString idAnimal, nomAnimal, proprietaire, telephone;
    QString motif;
    QDateTime dateRdv;
    QString statut = "Prévu";
    QDate smsRappel;     // date d'envoi du SMS de rappel (invalide = pas encore envoyé)
    QDate smsManque;     // date d'envoi du SMS "manqué"
};

struct RdvSmsAction
{
    RendezVous rdv;
    bool manque;         // false = SMS de rappel, true = SMS "rendez-vous manqué"
};

// Couche Modèle : toutes les requêtes SQL des rendez-vous sont ici.
// Hors ligne (pas de connexion) : données de démonstration gardées en mémoire.
class RdvManager
{
public:
    static QList<RendezVous> tous(const QVector<MedicalRecord> &records);
    static QList<RendezVous> pourAnimal(const QString &idAnimal, const QVector<MedicalRecord> &records);
    // Prochain rendez-vous "Prévu" de l'animal
    static bool prochain(const QString &idAnimal, const QVector<MedicalRecord> &records, RendezVous &out);

    // Planifie un rendez-vous : l'ancien rendez-vous "Prévu" du même animal est annulé
    static bool planifier(RendezVous &r, const QVector<MedicalRecord> &records);
    static bool changerStatut(const QString &idRdv, const QString &statut);
    static bool marquerSms(const QString &idRdv, bool manque, const QDate &date);

    // Dernier numéro connu du propriétaire (rendez-vous, puis vaccinations)
    static QString dernierTelephone(const QString &idAnimal, const QVector<MedicalRecord> &records);

    // Met à jour les statuts (Venu / Manqué) d'après les consultations enregistrées
    static void majStatuts(const QVector<MedicalRecord> &records, const QDateTime &now = QDateTime::currentDateTime());
    // SMS à envoyer maintenant (appelle majStatuts) ; chaque SMS ne part qu'une fois
    static QList<RdvSmsAction> aEnvoyer(const QVector<MedicalRecord> &records, const QDateTime &now = QDateTime::currentDateTime());

private:
    static QList<RendezVous> lireBase();
    static QList<RendezVous> &demo(const QVector<MedicalRecord> &records);
    static QString prochainId(const QList<RendezVous> &existants);
};

#endif // APPOINTMENT_H
