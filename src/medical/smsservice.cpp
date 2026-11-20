#include "smsservice.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>

SmsService::SmsService(QObject *parent) : QObject(parent)
{
    m_sid   = qEnvironmentVariable("ANIWORLD_SMS_SID");
    m_token = qEnvironmentVariable("ANIWORLD_SMS_TOKEN");
    m_from  = qEnvironmentVariable("ANIWORLD_SMS_FROM");
}

QString SmsService::normalizePhone(const QString &raw)
{
    QString digits;
    for (const QChar &c : raw)
        if (c.isDigit())
            digits.append(c);
    const bool international = raw.trimmed().startsWith('+') || raw.trimmed().startsWith("00");

    if (raw.trimmed().startsWith("00") && digits.startsWith("00"))
        digits.remove(0, 2);
    if (international)
        return digits.size() >= 8 ? "+" + digits : QString();
    if (digits.size() == 8)                                   // numéro tunisien local
        return "+216" + digits;
    if (digits.size() == 11 && digits.startsWith("216"))
        return "+" + digits;
    return QString();
}

QString SmsService::composeReminder(const Vaccination &v)
{
    const QString date = QLocale(QLocale::French).toString(v.dateRappel, "dd/MM/yyyy");
    const QString vaccin = v.nomVaccin.section('(', 0, 0).trimmed();   // nom court : le SMS reste dans 160 caractères
    const int d = v.joursRestants();
    if (d < 0)
        return QString("ANIWORLD : le rappel de vaccin %1 de %2 etait prevu le %3 (en retard). "
                       "Merci de nous contacter pour prendre rendez-vous.")
                .arg(vaccin, v.nomAnimal, date);
    return QString("ANIWORLD : le rappel de vaccin %1 de %2 est a faire avant le %3. "
                   "Merci de nous contacter pour prendre rendez-vous.")
            .arg(vaccin, v.nomAnimal, date);
}

QString SmsService::composeRdvRappel(const RendezVous &r)
{
    const QLocale fr(QLocale::French);
    return QString("ANIWORLD : rappel, %1 a rendez-vous le %2 a %3. "
                   "Merci de venir a l'heure ou de nous prevenir en cas d'empechement.")
            .arg(r.nomAnimal, fr.toString(r.dateRdv.date(), "dd/MM/yyyy"), fr.toString(r.dateRdv.time(), "HH:mm"));
}

QString SmsService::composeRdvManque(const RendezVous &r)
{
    const QLocale fr(QLocale::French);
    return QString("ANIWORLD : %1 avait rendez-vous le %2 mais nous ne l'avons pas vu. "
                   "Merci de nous contacter pour fixer un nouveau rendez-vous.")
            .arg(r.nomAnimal, fr.toString(r.dateRdv.date(), "dd/MM/yyyy"));
}

void SmsService::send(const QString &phone, const QString &text, const QString &reference)
{
    const QString to = normalizePhone(phone);
    if (to.isEmpty()) {
        QTimer::singleShot(0, this, [this, reference]() {
            emit finished(reference, false, false, "Numéro de téléphone invalide");
        });
        return;
    }

    if (!isConfigured()) {
        QTimer::singleShot(0, this, [this, reference, to, text]() {
            emit finished(reference, true, true,
                          QString("SIMULATION vers %1 : %2").arg(to, text));
        });
        return;
    }

    const QUrl url(QString("https://api.twilio.com/2010-04-01/Accounts/%1/Messages.json").arg(m_sid));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setRawHeader("Authorization", "Basic " + (m_sid + ":" + m_token).toUtf8().toBase64());

    auto enc = [](const QString &s) { return QUrl::toPercentEncoding(s); };
    const QByteArray body = "To=" + enc(to) + "&From=" + enc(m_from) + "&Body=" + enc(text);

    QNetworkReply *reply = m_nam.post(req, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply, reference]() {
        const QByteArray data = reply->readAll();
        const bool ok = reply->error() == QNetworkReply::NoError;
        QString info = ok ? QString("SMS envoyé") : reply->errorString();
        if (!ok) {
            const QJsonObject o = QJsonDocument::fromJson(data).object();
            if (o.contains("message"))
                info = o.value("message").toString();
        }
        reply->deleteLater();
        emit finished(reference, ok, false, info);
    });
}
