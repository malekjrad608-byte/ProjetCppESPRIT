#include <QApplication>
#include <QMessageBox>
#include <QTextStream>
#include <QLocale>
#include "database/databasemanager.h"
#include "database/oracleconnection.h"
#include "ui/design.h"
#include "appcontroller.h"
int main(int argc,char **argv) {
 QApplication app(argc,argv);app.setApplicationName("ANIWORLD");app.setOrganizationName("ANIWORLD");
 app.setApplicationDisplayName("ANIWORLD — Services");app.setFont(QFont("Segoe UI",10));
 QLocale::setDefault(QLocale(QLocale::French,QLocale::Tunisia));Design::applyTheme();
 auto &db=DatabaseManager::instance();const bool check=app.arguments().contains("--check-oracle");
 if(!db.initializeOracle()) {if(check)QTextStream(stderr)<<db.lastError();else QMessageBox::critical(nullptr,"Connexion Oracle",db.lastError());return 1;}
 if(check){QString error;auto report=OracleConnection::diagnostic(&error);QTextStream(stdout)<<report;return error.isEmpty()?0:1;}
 AppController controller(false);controller.start();return app.exec();
}
