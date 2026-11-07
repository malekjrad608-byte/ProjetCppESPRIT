#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QStyleFactory>
#include <QDebug>
#include <QMessageBox>
#include "mainwindow.h"
#include "connection.h"

void chargerStyle()
{
    QFile file(":/style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
        file.close();
        qDebug() << "✅ Style QSS chargé";
    } else {
        qDebug() << "⚠️ Impossible de charger style.qss";
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("ANIWORLD");
    QCoreApplication::setApplicationName("ANIWORLD");

    QApplication::setStyle("Fusion");
    chargerStyle();

    // Meyers Singleton : une seule connexion Oracle pour toute l'application.
    // La connexion doit etre ouverte AVANT de construire MainWindow, car
    // son constructeur charge les donnees du modele Employe.
    Connection& c = Connection::createInstance();
    if (!c.createConnection()) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Connexion à Oracle"),
            QObject::tr("Impossible d'établir la connexion à la base de données.\n\n%1")
                .arg(c.lastError())
        );
        return 1;
    }

    // La Vue est instanciee uniquement apres l'ouverture reussie de la BD.
    MainWindow mainWindow;
    mainWindow.show();

    return a.exec();
}