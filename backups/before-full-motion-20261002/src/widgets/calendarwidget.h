#pragma once
#include <QWidget>
#include <QCalendarWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include "../models/schedule.h"
#include "../models/employee.h"

class CalendarWidget : public QWidget {
    Q_OBJECT
public:
    explicit CalendarWidget(QWidget *parent = nullptr);
    void refresh();

private slots:
    void onDateSelected(const QDate &date);
    void onAddShift();

private:
    void setupUi();
    void loadShiftsForDate(const QDate &date);

    QCalendarWidget *m_calendar;
    QListWidget     *m_shiftList;
    QLabel          *m_dateLabel;
    QPushButton     *m_addBtn;
    QList<Employee>  m_employees;
};
