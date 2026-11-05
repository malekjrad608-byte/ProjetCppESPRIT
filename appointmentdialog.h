#ifndef APPOINTMENTDIALOG_H
#define APPOINTMENTDIALOG_H

#include <QDialog>
#include <QVector>
#include <QList>
#include "medicalrecord.h"
#include "appointment.h"

class QTableWidget;
class QLabel;
class SmsService;

// Liste des rendez-vous (prévus et manqués) avec l'état des SMS ; envoi manuel possible.
// Les SMS automatiques sont lancés par la fenêtre principale (voir MainWindow::checkAppointments).
class AppointmentDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AppointmentDialog(const QVector<MedicalRecord> &records, QWidget *parent = nullptr);

private slots:
    void onSendSelected();
    void onMarkCame();
    void onCancelRdv();

private:
    void reload();
    int currentRow() const;

    QVector<MedicalRecord> m_records;
    QList<RendezVous> m_rows;
    QTableWidget *m_table;
    QLabel *m_status;
    SmsService *m_sms;
};

#endif // APPOINTMENTDIALOG_H
