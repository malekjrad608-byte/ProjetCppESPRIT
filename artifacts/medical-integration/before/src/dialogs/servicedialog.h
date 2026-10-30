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
protected:
    void showEvent(QShowEvent *event) override;
private:
    void setup(const Service &service,bool editing,bool readOnly);
    void validateAndAccept();
    int m_id=0;
    QLineEdit *m_name;
    QLineEdit *m_responsible;
    QLineEdit *m_client;
    QTextEdit *m_description;
    QComboBox *m_category;
    QComboBox *m_status;
    QDateTimeEdit *m_date;
    QSpinBox *m_duration;
    QDoubleSpinBox *m_price;
    QLabel *m_error;
};
