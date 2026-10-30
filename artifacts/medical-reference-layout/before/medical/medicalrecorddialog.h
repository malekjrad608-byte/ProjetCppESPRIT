#ifndef MEDICALRECORDDIALOG_H
#define MEDICALRECORDDIALOG_H

#include <QDialog>
class QPushButton;
#include "medicalrecord.h"

class QLineEdit;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QDateEdit;
class QTextEdit;

class MedicalRecordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MedicalRecordDialog(QWidget *parent = nullptr);
    void setMedicalRecord(const MedicalRecord& record);
    MedicalRecord getMedicalRecord() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();
    bool validateForm();

    // Animal info widgets
    QLineEdit *m_editAnimalId;
    QLineEdit *m_editAnimalName;
    QComboBox *m_comboSpecies;
    QLineEdit *m_editBreed;
    QSpinBox *m_spinAge;
    QComboBox *m_comboSex;
    QLineEdit *m_editOwner;

    // Medical info widgets
    QLineEdit *m_editRecordId;
    QDateEdit *m_dateConsultation;
    QTextEdit *m_editReason;
    QTextEdit *m_editDiagnosis;
    QTextEdit *m_editTreatment;

    // Health widgets
    QTextEdit *m_editVaccinations;
    QTextEdit *m_editAllergies;
    QDoubleSpinBox *m_spinWeight;
    QTextEdit *m_editObservations;
    QComboBox *m_comboVaccinationStatus;

    QPushButton *m_btnSave;
    QPushButton *m_btnCancel;
};

#endif // MEDICALRECORDDIALOG_H
