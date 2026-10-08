#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QTextEdit>
#include <QCheckBox>
#include "../models/medicalrecord.h"

class VaccinationDialog : public QDialog {
    Q_OBJECT
public:
    explicit VaccinationDialog(int animalId, QWidget *parent = nullptr);
    VaccinationDialog(const Vaccination &vaccination, bool readOnly, QWidget *parent = nullptr);
    Vaccination getVaccination() const;
protected:
    void showEvent(QShowEvent *event) override;
private:
    void setup(const Vaccination &vaccination, bool editing, bool readOnly);
    void validateAndAccept();
    int m_animalId;
    Vaccination m_vaccination;
    QLineEdit *m_vaccineName;
    QLineEdit *m_vaccineType;
    QDateEdit *m_administeredDate;
    QDateEdit *m_nextDueDate;
    QLineEdit *m_veterinarian;
    QLineEdit *m_batchNumber;
    QTextEdit *m_notes;
    QCheckBox *m_reminderSet;
    QLabel *m_error;
};
