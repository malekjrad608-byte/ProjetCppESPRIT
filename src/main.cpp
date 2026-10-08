#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QLocale>
#include "appcontroller.h"
#include "database/databasemanager.h"
#include "ui/design.h"

int main(int argc,char *argv[]) {
    QApplication app(argc,argv);
    app.setApplicationName("ANIWORLD"); app.setOrganizationName("ANIWORLD"); app.setApplicationVersion("2.4.1");
    app.setFont(QFont("Segoe UI",10));
    QLocale::setDefault(QLocale(QLocale::French,QLocale::Tunisia));
    Design::applyTheme();
    app.setWindowIcon(QIcon(Design::brand("logo",QSize(128,128))));
    const bool demo=app.arguments().contains("--demo");
    const QString directory=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if(!demo && !QDir().mkpath(directory)) {
        QMessageBox::critical(nullptr,"Stockage indisponible","Impossible de créer le dossier de sauvegarde :\n"+directory); return 1;
    }
    auto &db=DatabaseManager::instance();
    if(!db.initialize(demo?":memory:":directory+"/services.sqlite") || (demo && !db.addExamples())) {
        QMessageBox::critical(nullptr,"Stockage indisponible","Impossible d'ouvrir les services.\n"+db.lastError()); return 1;
    }
    if(!db.addDefaultUsers()) {
        QMessageBox::warning(nullptr,"Utilisateurs","Impossible d'initialiser les utilisateurs par défaut.\n"+db.lastError());
    }
    AppController controller(demo); controller.start();
    return app.exec();
}
