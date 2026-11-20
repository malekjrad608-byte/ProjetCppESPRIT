#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QLocale>
#include "appcontroller.h"
#include "database/databasemanager.h"
#include "ui/design.h"
#include "employees/employe.h"
#include "medical/medicalrecord.h"
#include "database/oracleconnection.h"
#include <QTextStream>


int main(int argc,char *argv[]) {
    QApplication app(argc,argv);
    app.setApplicationName("ANIWORLD"); app.setOrganizationName("ANIWORLD"); app.setApplicationVersion("3.0.0");
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
    const bool migrate=app.arguments().contains("--migrate-oracle");
    const bool check=app.arguments().contains("--check-oracle");
    const bool oracle=!demo && !app.arguments().contains("--local") && (migrate || check || app.arguments().contains("--oracle") || OracleConnection::configured());
    const bool opened=oracle?db.initializeOracle():db.initialize(demo?":memory:":directory+"/services.sqlite");
    if(!opened || (demo && !db.addExamples())) {
        if(migrate || check) { QTextStream(stderr)<<db.lastError()<<Qt::endl;return 1; }
        QMessageBox::critical(nullptr,"Stockage indisponible","Impossible d'ouvrir les services.\n"+db.lastError()); return 1;
    }
    QString error;
    if(migrate) {
        const bool ok=OracleConnection::migrateLocal(directory+"/services.sqlite",&error);
        QTextStream(ok?stdout:stderr)<<(ok?"Migration Oracle terminée; données locales conservées.":error)<<Qt::endl;
        return ok?0:1;
    }
    if(check) {
        const auto report=OracleConnection::diagnostic(&error);
        if(!error.isEmpty()){QTextStream(stderr)<<error<<Qt::endl;return 1;}
        QTextStream(stdout)<<report;
        return 0;
    }
    if(!oracle && !Employe::importInitial(OracleConnection::database(),&error)) {
        QMessageBox::critical(nullptr,"Import des employés",error); return 1;
    }
    if(demo && !MedicalRecord::seedExamples(&error)) { QMessageBox::critical(nullptr,"Dossiers médicaux",error); return 1; }
    if(!oracle && !demo && !MedicalRecord::importInitial(&error)) { QMessageBox::critical(nullptr,"Import des dossiers médicaux",error); return 1; }
    AppController controller(demo); controller.start();
    return app.exec();
}
