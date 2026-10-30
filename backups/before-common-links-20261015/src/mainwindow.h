#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QPropertyAnimation>
#include "widgets/dashboardwidget.h"
#include "widgets/servicewidget.h"
#include "auth/accounts.h"
class StockWidget;
class MedicalWidget;
class PageLocaux;
class GestionEmployes;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(bool demo=false,QWidget *parent=nullptr);
    void setSignedInUser(const QString &username);
    void setSession(const QString &username,Accounts::Role role);
    Accounts::Role sessionRole() const { return m_role; }
signals:
    void logoutRequested();
public slots:
    void navigate(int page);
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    void positionNavigationMarker(bool animated);
    DashboardWidget *m_dashboard;
    ServiceWidget *m_services;
    StockWidget *m_stock;
    MedicalWidget *m_medical;
    PageLocaux *m_locaux;
    GestionEmployes *m_employees;
    QStackedWidget *m_stack;
    QList<QPushButton *> m_navigation;
    QLabel *m_breadcrumb;
    QFrame *m_navigationMarker;
    QPropertyAnimation *m_markerMotion;
    QLabel *m_sessionUser;
    QPushButton *m_logoutButton;
    QLabel *m_headerDate = nullptr;
    QLabel *m_storageBadge;
    QPushButton *m_passwordButton;
    Accounts::Role m_role=Accounts::Role::Admin;
    QString m_username;
    bool m_demo;
};
