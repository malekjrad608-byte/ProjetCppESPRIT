#include <QApplication>
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
    a.setApplicationName("ANIWORLD");        // nécessaire aux réglages enregistrés (SMS, alertes de veille sanitaire)
    a.setOrganizationName("ANIWORLD");

    QApplication::setStyle("Fusion");
    chargerStyle();

    // Ne jamais ouvrir Oracle pendant le démarrage : un pilote ODBC absent,
    // un DSN incorrect ou un serveur arrêté peut bloquer l'interface avant
    // son premier affichage. L'utilisateur peut lancer la connexion depuis
    // le bouton de statut Oracle dans la barre supérieure.
    MainWindow mainWindow;
    mainWindow.show();

    return a.exec();
}