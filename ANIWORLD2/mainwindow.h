#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>

class GestionEmployes;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void afficherEmployes();
    void afficherTableauDeBord();
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
    QPushButton *m_btnEmployes;
    QPushButton *m_btnTableau;
    QPushButton *m_btnServices;
    QPushButton *m_btnStock;
    QPushButton *m_btnLocaux;
    QPushButton *m_btnDossiers;

    QPushButton *m_btnOracle = nullptr;
    QWidget *m_logoWidget;
    QPropertyAnimation *m_logoAnimation;
};

#endif