// Explicit, repeatable demo-data operation. Oracle rows are only read.
#include "src/database/databasemanager.h"
#include "src/employees/employe.h"
#include "src/employees/planning.h"
#include "src/models/modulesupplements.h"
#include "src/models/service.h"
#include "src/dialogs/serviceplanningdialog.h"
#include "src/ui/design.h"
#include <QApplication>
#include <QFile>
#include <QSaveFile>
#include <QDir>
#include <QJsonObject>
#include <QTextStream>
#include <QTimer>
#include <QTableWidget>

int main(int argc,char **argv) {
    QApplication app(argc,argv);
    app.setApplicationName("ANIWORLD");app.setOrganizationName("ANIWORLD");
    Design::applyTheme();
    QTextStream out(stdout),err(stderr);
    if(!DatabaseManager::instance().initializeOracle()) {err<<DatabaseManager::instance().lastError();return 1;}
    QString error;
    const auto original=ModuleSupplements::read("planning",&error);
    if(!error.isEmpty()){err<<error;return 1;}
    const auto employees=Employe::afficher();
    if(employees.isEmpty()){err<<"Aucun employe disponible.";return 1;}
    QList<Planning> proposal;
    const QDate monday(2026,11,9);
    int day=0;
    for(const auto &employee:employees) {
        if(!employee.isDisponible())continue;
        for(int slot=0;slot<2;++slot) {
            Planning p;
            p.setResponsableId(employee.getId());
            p.setTitre(slot==0 ? "Démo · Préparation et suivi des soins" : "Démo · Contrôle du matériel et bilan");
            p.setCompetence(employee.getPoste().isEmpty()?"Organisation des soins":employee.getPoste());
            p.setDebut(QDateTime(monday.addDays(day%5),QTime(slot==0?9:14,0)));
            p.setFin(p.getDebut().addSecs(3600));p.setHeures(1);
            bool exists=false;
            for(const auto &v:original){const auto o=v.toObject();if(o["employee"].toInt()==employee.getId() && o["title"].toString()==p.getTitre() && o["start"].toString()==p.getDebut().toString(Qt::ISODate))exists=true;}
            if(exists)continue;
            if(!Service::available(employee.getId(),p.getDebut(),p.getFin(),0,&error)){err<<error;return 1;}
            proposal<<p;
        }
        ++day;
    }
    out<<"Proposed demo tasks: "<<proposal.size()<<Qt::endl;
    if(!app.arguments().contains("--apply"))return 0;
    const QString path=ModuleSupplements::path("planning");
    const bool existed=QFile::exists(path);
    QFile previous(path);QByteArray bytes;
    if(existed){if(!previous.open(QIODevice::ReadOnly))return 1;bytes=previous.readAll();previous.close();}
    QDir().mkpath("artifacts/planning-demo-20261109");
    if(existed && !proposal.isEmpty() && !QFile::copy(path,"artifacts/planning-demo-20261109/planning-before-"+QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz")+".json"))return 1;
    int added=0;
    for(const auto &p:proposal) {
        if(!p.ajouter(&error)) {
            if(existed){QSaveFile restore(path);if(!restore.open(QIODevice::WriteOnly)||restore.write(bytes)!=bytes.size()||!restore.commit()){err<<"Restore failed: "<<path;return 2;}}
            else if(!QFile::remove(path)){err<<"Restore failed: "<<path;return 2;}
            err<<error;return 1;
        }
        ++added;
    }
    const auto appointments=Service::appointments(&error);
    if(!error.isEmpty()){err<<error;return 1;}
    int demos=0;for(const auto &a:appointments)if(a.start.date()>=monday && a.start.date()<monday.addDays(7) && a.title.startsWith("Démo ·"))++demos;
    out<<"Added: "<<added<<"; demo tasks readable: "<<demos<<"; employee count: "<<day<<Qt::endl;
    out<<"Persistence: "<<path<<Qt::endl;
    ServicePlanningDialog dialog(monday);dialog.show();
    QTimer::singleShot(700,&app,[&]{
        out<<"Visible weekly rows: "<<dialog.findChild<QTableWidget *>("servicePlanningTable")->rowCount()<<Qt::endl;
        dialog.grab().save("artifacts/planning-demo-20261109/planning-populated.png");app.quit();
    });
    return app.exec();
}
