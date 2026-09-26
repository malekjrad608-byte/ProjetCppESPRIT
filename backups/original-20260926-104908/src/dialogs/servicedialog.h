#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include "../models/service.h"
#include "../models/employee.h"

class ServiceDialog : public QDialog {
    Q_OBJECT
public:
    explicit ServiceDialog(const QList<Employee> &employees, QWidget *parent = nullptr);
    explicit ServiceDialog(const Service &service, const QList<Employee> &employees, QWidget *parent = nullptr);

    Service getService() const;

private:
    void setupUi();
    void populateEmployees(const QList<Employee> &employees);
    void loadService(const Service &s);
    void applyStyle();

    QLineEdit     *m_nameEdit;
    QTextEdit     *m_descEdit;
    QComboBox     *m_employeeBox;
    QDateTimeEdit *m_dateEdit;
    QComboBox     *m_statusBox;
    QPushButton   *m_saveBtn;
    QPushButton   *m_cancelBtn;

    int m_serviceId = 0;
    QList<Employee> m_employees;
};
