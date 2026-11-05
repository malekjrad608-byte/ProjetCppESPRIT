#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>

class GestionEmployes;
class MedicalWindow;
class QLabel;
class QVBoxLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void afficherEmployes();
    void afficherServices();
    void afficherStock();
    void afficherLocaux();
    void afficherDossiers();
    void rafraichirStatutOracle(bool retenter = false);

private:
    QFrame *creerTopBar();
    QFrame *creerSidebar();
    QPushButton *creerBoutonMenu(const QString &icone, const QString &texte);
    QWidget *creerPageEmployes();
    QWidget *creerPagePlaceholder(const QString &titre, const QString &icone,
                                  const QString &description);
    void fadeInPage(QWidget *page);

    QStackedWidget *m_stack;
    GestionEmployes *m_gestionEmployes = nullptr;
    QWidget *m_dossiersPage = nullptr;        // conteneur de la page Dossiers médicaux
    QLabel *m_breadcrumb = nullptr;
    MedicalWindow *m_medical = nullptr;       // créée à la première ouverture
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