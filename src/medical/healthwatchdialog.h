#ifndef HEALTHWATCHDIALOG_H
#define HEALTHWATCHDIALOG_H

#include <QDialog>
#include <QList>
#include <QVector>
#include "healthwatch.h"

class QTableWidget;
class QTextBrowser;
class QLabel;
class SmsService;

// Veille sanitaire : liste des alertes (récidives, foyers possibles) avec conseils
// et envoi d'un SMS de prévention aux propriétaires concernés.
class HealthWatchDialog : public QDialog
{
    Q_OBJECT
public:
    HealthWatchDialog(const QVector<MedicalRecord> &records, QWidget *parent = nullptr);

private slots:
    void onSelectionChanged();
    void onSendSms();

private:
    QVector<MedicalRecord> m_records;
    QList<HealthAlert> m_alerts;
    QTableWidget *m_table;
    QTextBrowser *m_details;
    QLabel *m_status;
    SmsService *m_sms;
    int m_smsOk = 0, m_smsFail = 0, m_smsPending = 0;
    QString m_smsError;
};

#endif // HEALTHWATCHDIALOG_H
