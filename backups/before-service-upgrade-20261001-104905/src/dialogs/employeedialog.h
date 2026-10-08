#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include "../models/employee.h"

class EmployeeDialog : public QDialog {
    Q_OBJECT
public:
    explicit EmployeeDialog(QWidget *parent = nullptr);
    explicit EmployeeDialog(const Employee &emp, QWidget *parent = nullptr);
    Employee getEmployee() const;

private:
    void setupUi();
    void loadEmployee(const Employee &e);
    void applyStyle();

    QLineEdit *m_firstNameEdit;
    QLineEdit *m_lastNameEdit;
    QLineEdit *m_posteEdit;
    QLineEdit *m_phoneEdit;
    QLineEdit *m_emailEdit;
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    int m_employeeId = 0;
};
