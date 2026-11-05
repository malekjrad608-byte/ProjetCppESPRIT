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

    QApplication::setStyle("Fusion");
    chargerStyle();

    // Si Oracle n'est pas disponible, l'application demarre quand meme :
    // le badge en haut a droite affichera "Oracle non connecte" (clic = reessayer).
    Connection& c = Connection::createInstance();
    c.createConnection();

    MainWindow mainWindow;
    mainWindow.show();

    return a.exec();
}