#include <QApplication>
#include <QFont>
#include <QLocale>
#include <QStyleFactory>
#include <QMessageBox> // Ajouté pour afficher l'état de la connexion

#include "loginwindow.h"
#include "stockwidget.h"
#include "connection.h" // Ajouté pour appeler le Singleton

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setFont(QFont("Segoe UI", 9));
    QLocale::setDefault(QLocale(QLocale::French, QLocale::France));
    app.setApplicationName("ANIWORLD");
    app.setOrganizationName("devCore");

    // -----------------------------------------------------------
    // Test de la connexion à la base de données Oracle (Singleton)
    // -----------------------------------------------------------
    Connection& c = Connection::createInstance();
    bool test = c.createConnection();

    if (test) {
        QMessageBox::information(nullptr, QObject::tr("Base de données"),
                                 QObject::tr("Connexion réussie.\nBienvenue sur ANIWORLD."), QMessageBox::Ok);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Base de données"),
                              QObject::tr("Échec de la connexion.\nVeuillez vérifier vos paramètres."), QMessageBox::Cancel);
    }
    // -----------------------------------------------------------

    auto *window = new StockWidget;
    window->setWindowTitle("ANIWORLD — Gestion du centre");
    window->resize(1400, 900);

    auto *login = new LoginWindow;

    QObject::connect(login, &LoginWindow::loginSuccessful, &app, [login, window]() {
        login->hide();
        window->show();
        window->raise();
        window->activateWindow();
    });

    QObject::connect(window, &StockWidget::logoutRequested, &app, [login, window]() {
        window->hide();
        login->show();
        login->raise();
        login->activateWindow();
    });

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [login, window]() {
        login->deleteLater();
        window->deleteLater();
    });

    // On affiche la fenêtre de connexion en premier
    login->show();

    return app.exec();
}