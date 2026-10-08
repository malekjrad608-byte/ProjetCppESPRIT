#ifndef MEDICALRECORDDETAILSDIALOG_H
#define MEDICALRECORDDETAILSDIALOG_H

#include <QDialog>
#include "medicalrecord.h"

class QLabel;
class QTableWidget;

class MedicalRecordDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MedicalRecordDetailsDialog(QWidget *parent = nullptr);
    void setMedicalRecord(const MedicalRecord& record);

private slots:
    void onCloseClicked();

private:
    void setupUI();
    void displayRecord(const MedicalRecord& record);
    void displayMedicalHistory(const MedicalRecord& record);

    QLabel *m_labelAnimalId;
    QLabel *m_labelAnimalName;
    QLabel *m_labelSpecies;
    QLabel *m_labelBreed;
    QLabel *m_labelAge;
    QLabel *m_labelSex;
    QLabel *m_labelOwner;

    QLabel *m_labelRecordId;
    QLabel *m_labelConsultationDate;
    QLabel *m_labelReason;
    QLabel *m_labelDiagnosis;
    QLabel *m_labelTreatment;

    QLabel *m_labelVaccinations;
    QLabel *m_labelAllergies;
    QLabel *m_labelWeight;
    QLabel *m_labelObservations;
    QLabel *m_labelVaccinationStatus;

    QTableWidget *m_historyTable;

    QPushButton *m_btnClose;
};

#endif // MEDICALRECORDDETAILSDIALOG_H
