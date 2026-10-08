#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include "../models/medicalrecord.h"

class MedicalRecordDialog : public QDialog {
    Q_OBJECT
public:
    explicit MedicalRecordDialog(int animalId, QWidget *parent = nullptr);
    MedicalRecordDialog(const MedicalRecord &record, bool readOnly, QWidget *parent = nullptr);
    MedicalRecord getRecord() const;
protected:
    void showEvent(QShowEvent *event) override;
private:
    void setup(const MedicalRecord &record, bool editing, bool readOnly);
    void validateAndAccept();
    int m_animalId;
    MedicalRecord m_record;
    QLineEdit *m_type;
    QLineEdit *m_veterinarian;
    QDateTimeEdit *m_date;
    QComboBox *m_status;
    QTextEdit *m_description;
    QTextEdit *m_diagnosis;
    QTextEdit *m_treatment;
    QDoubleSpinBox *m_cost;
    QTextEdit *m_notes;
    QLabel *m_error;
};
