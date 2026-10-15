#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"
#include "Connection.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("ANIWORLD");
    a.setOrganizationName("ANIWORLD");

    // Connexion à la base (Singleton). En cas d'échec, l'application démarre
    // avec les données d'exemple en mémoire (rien n'est enregistré).
    const bool connected = Connection::createInstance().createConnection();
    if (!connected)
        QMessageBox::warning(nullptr, "Base de données",
            "Connexion à la base impossible.\nMode hors ligne : les modifications ne seront pas enregistrées.");

    MainWindow w;
    w.show();
    return a.exec();
}
