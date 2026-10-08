#include <QApplication>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application-wide font
    QFont appFont("Segoe UI", 10);
    app.setFont(appFont);

    app.setApplicationName("Gestion de Service");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("GestionService");

    MainWindow w;
    w.show();

    return app.exec();
}
