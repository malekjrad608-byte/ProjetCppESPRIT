#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QPropertyAnimation>
#include "widgets/dashboardwidget.h"
#include "widgets/servicewidget.h"
#include "widgets/medicalrecordswidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(bool demo=false,QWidget *parent=nullptr);
    void setSignedInUser(const QString &username);
signals:
    void logoutRequested();
public slots:
    void navigate(int page);
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    void positionNavigationMarker(bool animated);
    void requestMedicalAccess();
    DashboardWidget *m_dashboard;
    ServiceWidget *m_services;
    MedicalRecordsWidget *m_medicalRecords;
    QStackedWidget *m_stack;
    QList<QPushButton *> m_navigation;
    QLabel *m_breadcrumb;
    QFrame *m_navigationMarker;
    QPropertyAnimation *m_markerMotion;
    QLabel *m_sessionUser;
    QPushButton *m_logoutButton;
    QLabel *m_headerDate = nullptr;
};
