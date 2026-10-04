#include "mainwindow.h"
#include "loginwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Charger le style QSS
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        a.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
        styleFile.close();
    }

    LoginWindow loginWindow;
    MainWindow *mainWindow = nullptr;

    QObject::connect(&loginWindow, &LoginWindow::loginSuccessful,
                     &a, [&]() {
        if (!mainWindow)
            mainWindow = new MainWindow;

        loginWindow.hide();
        mainWindow->show();
        mainWindow->raise();
        mainWindow->activateWindow();
    });

    loginWindow.show();

    const int result = a.exec();
    delete mainWindow;
    return result;
}
