#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>

namespace Ui { class MainWindow; }   // interface unique : mainwindow.ui (toutes les pages)
class ServiceWidget;
class GestionEmployes;
class MedicalWindow;
class StockWidget;
class QLabel;
class QVBoxLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // ======================================================================
    // === SLOTS GENERAUX (NAVIGATION ENTRE PAGES) ===
    // ======================================================================
    void afficherServices();
    void afficherStock();
    void rafraichirStatutOracle(bool retenter = false);

    // ======================================================================
    // === MODULE EMPLOYE ===
    // ======================================================================
    // Etudiante responsable du module Employe (Linda) : travaille ici uniquement
    void afficherEmployes();

    // ======================================================================
    // === MODULE LOCAUX ===
    // ======================================================================
    // Etudiante responsable du module Locaux (Emna) : travaille ici uniquement
    void afficherLocaux();

    // ======================================================================
    // === MODULE DOSSIERS MEDICAUX ===
    // ======================================================================
    // Etudiante responsable du module Dossiers medicaux (Rawand) : travaille ici uniquement
    void afficherDossiers();


private:
    QFrame *creerTopBar();
    QFrame *creerSidebar();
    QPushButton *creerBoutonMenu(const QString &icone, const QString &texte);
    QWidget *creerPageEmployes();
    QWidget *creerPagePlaceholder(const QString &titre, const QString &icone,
                                  const QString &description);
    void fadeInPage(QWidget *page);

    // ======================================================================
    // === MODULE SERVICES ===
    // ======================================================================
    void initialiserServices();
    ServiceWidget *m_services = nullptr;

    Ui::MainWindow *ui = nullptr;             // généré depuis mainwindow.ui
    QStackedWidget *m_stack = nullptr;        // = ui->stackedWidget
    QFrame *m_topBar = nullptr;               // bande du haut (cachee sur la page Locaux)
    GestionEmployes *m_gestionEmployes = nullptr;
    QWidget *m_dossiersPage = nullptr;        // conteneur de la page Dossiers médicaux
    QLabel *m_breadcrumb = nullptr;
    MedicalWindow *m_medical = nullptr;
    StockWidget *m_stock = nullptr;       // créée à la première ouverture
    QPushButton *m_btnEmployes;
    QPushButton *m_btnServices;
    QPushButton *m_btnStock;
    QPushButton *m_btnLocaux;
    QPushButton *m_btnDossiers;

    QPushButton *m_btnOracle = nullptr;
    QWidget *m_logoWidget;
    QPropertyAnimation *m_logoAnimation;
};

#endif