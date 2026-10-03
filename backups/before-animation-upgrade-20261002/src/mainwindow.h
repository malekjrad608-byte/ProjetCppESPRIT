#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include "widgets/dashboardwidget.h"
#include "widgets/servicewidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(bool demo=false,QWidget *parent=nullptr);
public slots:
    void navigate(int page);
private:
    DashboardWidget *m_dashboard;
    ServiceWidget *m_services;
    QStackedWidget *m_stack;
    QList<QPushButton *> m_navigation;
    QLabel *m_breadcrumb;
};
