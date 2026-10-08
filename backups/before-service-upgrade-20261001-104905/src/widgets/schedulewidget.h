#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QDate>
#include <QColor>
#include "../models/schedule.h"
#include "../models/employee.h"

class ScheduleWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScheduleWidget(QWidget *parent = nullptr);
    void refresh();
    void goToWeek(const QDate &date);

private slots:
    void onPrevWeek();
    void onNextWeek();
    void onAdd();
    void onDelete();
    void onCellDoubleClicked(int row, int col);

private:
    void setupUi();
    void buildWeekTable();
    void applyStyle();
    QString shiftBadge(const QString &shift, int id);
    QColor  shiftBgColor(const QString &shift);

    QDate         m_weekStart;
    QTableWidget *m_table;
    QPushButton  *m_prevBtn;
    QPushButton  *m_nextBtn;
    QPushButton  *m_addBtn;
    QPushButton  *m_deleteBtn;
    QLabel       *m_weekLabel;
    QList<Schedule> m_schedules;
    QList<Employee> m_employees;
};
