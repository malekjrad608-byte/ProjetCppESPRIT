#ifndef SMSSERVICE_H
#define SMSSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>

#include "vaccination.h"
#include "appointment.h"

// Envoi de SMS aux propriétaires (rappels vaccinaux).
//
// Passerelle : API REST Twilio. Les identifiants ne sont JAMAIS écrits dans le code,
// ils sont lus dans des variables d'environnement :
//     ANIWORLD_SMS_SID    (Account SID)
//     ANIWORLD_SMS_TOKEN  (Auth Token)
//     ANIWORLD_SMS_FROM   (numéro expéditeur Twilio, ex: +1415...)
// Si elles sont absentes : mode SIMULATION (rien n'est envoyé, le message est seulement affiché).
class SmsService : public QObject
{
    Q_OBJECT
public:
    explicit SmsService(QObject *parent = nullptr);

    bool isConfigured() const { return !m_sid.isEmpty() && !m_token.isEmpty() && !m_from.isEmpty(); }

    // reference = identifiant renvoyé tel quel dans le signal (ici l'ID du vaccin)
    void send(const QString &phone, const QString &text, const QString &reference);

    // "20 123 456" -> "+21620123456" ; renvoie "" si le numéro est invalide
    static QString normalizePhone(const QString &raw);
    static QString composeReminder(const Vaccination &v);
    static QString composeRdvRappel(const RendezVous &r);   // SMS la veille / le jour du rendez-vous
    static QString composeRdvManque(const RendezVous &r);   // SMS si l'animal n'est pas venu

signals:
    // simulated = true : aucun SMS réel n'est parti
    void finished(const QString &reference, bool ok, bool simulated, const QString &info);

private:
    QNetworkAccessManager m_nam;
    QString m_sid, m_token, m_from;
};

#endif // SMSSERVICE_H
