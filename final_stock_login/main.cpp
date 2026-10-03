#include <QApplication>
#include <QStyleFactory>
#include "loginwindow.h"
#include "stockwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyle(QStyleFactory::create("Fusion"));
    app.setApplicationName("ANIWORLD");
    app.setOrganizationName("devCore");

    StockWidget stockWindow;
    stockWindow.setWindowTitle("ANIWORLD - Gestion du Stock");
    stockWindow.resize(1024, 768);

    LoginWindow loginWindow;

    QObject::connect(&loginWindow, &LoginWindow::loginSuccessful,
                     &app, [&]() {
        loginWindow.hide();
        stockWindow.show();
        stockWindow.raise();
        stockWindow.activateWindow();
    });

    loginWindow.show();
    return app.exec();
}
