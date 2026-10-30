#pragma once
#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QPushButton>
#include "../models/schedule.h"
#include "../models/employee.h"

class ScheduleDialog : public QDialog {
    Q_OBJECT
public:
    explicit ScheduleDialog(const QList<Employee> &employees, const QDate &preselectedDate = QDate(), QWidget *parent = nullptr);
    explicit ScheduleDialog(const Schedule &s, const QList<Employee> &employees, QWidget *parent = nullptr);
    Schedule getSchedule() const;

private:
    void setupUi();
    void applyStyle();
    void populateEmployees(const QList<Employee> &employees);
    void loadSchedule(const Schedule &s);

    QComboBox   *m_employeeBox;
    QDateEdit   *m_dateEdit;
    QComboBox   *m_shiftBox;
    QTextEdit   *m_noteEdit;
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    int          m_scheduleId = 0;
    QList<Employee> m_employees;
};
