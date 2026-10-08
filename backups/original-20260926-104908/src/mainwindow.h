#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QLabel>
#include "widgets/dashboardwidget.h"
#include "widgets/servicewidget.h"
#include "widgets/schedulewidget.h"
#include "widgets/calendarwidget.h"

// Forward declaration for employee management
class EmployeeManagementWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onNavChanged(int row);
    void showConnectionDialog();

private:
    void setupUi();
    void setupSidebar();
    void setupContent();
    void applyGlobalStyle();

    QWidget          *m_sidebar;
    QListWidget      *m_navList;
    QStackedWidget   *m_stack;

    DashboardWidget  *m_dashboard;
    ServiceWidget    *m_serviceWidget;
    ScheduleWidget   *m_scheduleWidget;
    CalendarWidget   *m_calendarWidget;
    QWidget          *m_employeeWidget;

    QLabel           *m_statusLabel;
};
