#include <QApplication>
#include "mainwindow.h"
#include "logindialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Show login dialog first
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        // Login successful, show main window
        MainWindow w;
        w.show();
        return a.exec();
    }

    // Login cancelled or failed
    return 0;
}
