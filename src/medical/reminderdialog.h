#ifndef REMINDERDIALOG_H
#define REMINDERDIALOG_H

#include <QDialog>
#include <QVector>
#include <QList>
#include "medicalrecord.h"
#include "vaccination.h"

class QTableWidget;
class QLabel;
class QPushButton;
class QCheckBox;
class SmsService;

// DM-I1 : fenêtre d'alerte "Rappels vaccinaux".
// Affiche les animaux dont un vaccin est en retard ou à renouveler (30 jours),
// permet d'envoyer le SMS au propriétaire, de renouveler un vaccin et d'en ajouter un.
class ReminderDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReminderDialog(const QVector<MedicalRecord> &records, QWidget *parent = nullptr);

    // Envoi automatique au démarrage (option cochée dans la fenêtre) : renvoie le nombre de SMS lancés
    static int envoyerAutomatiquement(const QVector<MedicalRecord> &records, SmsService *sms);
    static bool autoSendEnabled();

private slots:
    void onSendSelected();
    void onSendAll();
    void onRenew();
    void onAddVaccine();
    void onItemChanged();

private:
    void reload();
    void sendRows(const QList<int> &rows);

    QVector<MedicalRecord> m_records;
    QList<Vaccination> m_rows;
    bool m_loading = false;

    QTableWidget *m_table;
    QLabel *m_summary;
    QLabel *m_status;
    QCheckBox *m_autoSend;
    SmsService *m_sms;
};

#endif // REMINDERDIALOG_H
