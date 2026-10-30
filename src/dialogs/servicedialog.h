#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include "../models/service.h"

class ServiceDialog : public QDialog {
    Q_OBJECT
public:
    explicit ServiceDialog(QWidget *parent=nullptr);
    ServiceDialog(const Service &service,bool readOnly,QWidget *parent=nullptr);
    Service getService() const;
    void showError(const QString &message) { m_error->setText(message); m_error->show(); }
protected:
    void showEvent(QShowEvent *event) override;
private:
    void setup(const Service &service,bool editing,bool readOnly);
    void validateAndAccept();
    int m_id=0;
    qint64 m_medicalRecordId=0;
    QLineEdit *m_name;
    QLineEdit *m_responsible;
    QLineEdit *m_client;
    QLineEdit *m_newEmployeeNom=nullptr;
    QLineEdit *m_newEmployeePrenom=nullptr;
    QTextEdit *m_description;
    QComboBox *m_category;
    QComboBox *m_status;
    QComboBox *m_local;
    QComboBox *m_employee;
    QComboBox *m_clientChoice;
    QDateTimeEdit *m_date;
    QSpinBox *m_duration;
    QDoubleSpinBox *m_price;
    QLabel *m_error;
};
