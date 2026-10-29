#include <QApplication>
#include <QFile>
#include <QFont>
#include <QIcon>
#include <QMessageBox>
#include <QPalette>

#include "connection.h"
#include "mainwindow.h"
#include "theme.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ANIWORLD");
    app.setOrganizationName("ANIWORLD");
    app.setWindowIcon(QIcon(":/images/logo.png"));

    // Style identique sur tous les PC (meme si Windows est en mode sombre)
    app.setStyle("Fusion");
    QPalette pal;
    pal.setColor(QPalette::Window, Theme::Background);
    pal.setColor(QPalette::WindowText, Theme::Ink);
    pal.setColor(QPalette::Base, Qt::white);
    pal.setColor(QPalette::AlternateBase, Theme::TealXL);
    pal.setColor(QPalette::Text, Theme::Ink);
    pal.setColor(QPalette::Button, Qt::white);
    pal.setColor(QPalette::ButtonText, Theme::Ink);
    pal.setColor(QPalette::ToolTipBase, Theme::TealDark);
    pal.setColor(QPalette::ToolTipText, Qt::white);
    pal.setColor(QPalette::Highlight, Theme::Teal);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::PlaceholderText, Theme::Muted);
    app.setPalette(pal);
    app.setFont(Theme::font(9.5));

    QFile qss(":/resources/style.qss");
    if (qss.open(QIODevice::ReadOnly | QIODevice::Text))
        app.setStyleSheet(QString::fromUtf8(qss.readAll()));

    // Connexion a la base (comme l'atelier) : ouverte une seule fois, avant l'interface
    Connection &c = Connection::createInstance();
    bool test = c.createConnection();
    if (!test) {
        QMessageBox::critical(nullptr, QObject::tr("database is not open"),
                              QObject::tr("connection failed.\n") + c.lastError());
        return 1;
    }

    MainWindow w;
    w.resize(1360, 820);
    w.show();
    QMessageBox::information(nullptr, QObject::tr("database is open"),
                             QObject::tr("connection successful."));
    return app.exec();
}
