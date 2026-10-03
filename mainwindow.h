#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>

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

private:
    QFrame *creerTopBar();
    QFrame *creerSidebar();
    QPushButton *creerBoutonMenu(const QString &icone, const QString &texte);
    QWidget *creerPageEmployes();
    QWidget *creerPagePlaceholder(const QString &titre, const QString &icone,
                                  const QString &description);
    void fadeInPage(QWidget *page);

    QStackedWidget *m_stack;
    QPushButton *m_btnEmployes;
    QPushButton *m_btnTableau;
    QPushButton *m_btnServices;
    QPushButton *m_btnStock;
    QPushButton *m_btnLocaux;
    QPushButton *m_btnDossiers;

    QWidget *m_logoWidget;
    QPropertyAnimation *m_logoAnimation;
};

#endif