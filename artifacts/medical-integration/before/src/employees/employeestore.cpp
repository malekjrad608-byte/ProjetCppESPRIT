#include "employeestore.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QTemporaryDir>
#include <QFile>
#include <QVariant>

namespace EmployeeStore {
bool initialize(QSqlDatabase db,QString *error) {
    const QStringList statements={
        "PRAGMA foreign_keys=ON",
        "CREATE TABLE IF NOT EXISTS employes(id INTEGER PRIMARY KEY AUTOINCREMENT,nom TEXT NOT NULL,prenom TEXT NOT NULL,courriel TEXT,telephone TEXT,poste TEXT,date_embauche TEXT,disponible INTEGER DEFAULT 1,charge_hebdo REAL DEFAULT 0,dispo_hebdo REAL DEFAULT 35,competences TEXT)",
        "CREATE TABLE IF NOT EXISTS planning_affectations(id INTEGER PRIMARY KEY AUTOINCREMENT,titre TEXT NOT NULL,competence_requise TEXT NOT NULL,debut TEXT NOT NULL,fin TEXT NOT NULL,charge_heures REAL NOT NULL DEFAULT 1,responsable_id INTEGER NOT NULL,created_at TEXT DEFAULT CURRENT_TIMESTAMP,FOREIGN KEY(responsable_id) REFERENCES employes(id) ON DELETE CASCADE)",
        "CREATE INDEX IF NOT EXISTS idx_planning_responsable_dates ON planning_affectations(responsable_id,debut,fin)",
        "CREATE TABLE IF NOT EXISTS integration_migrations(name TEXT PRIMARY KEY)"};
    QSqlQuery q(db);
    for(const auto &sql:statements) if(!q.exec(sql)) { *error=q.lastError().text(); return false; }
    return true;
}
bool importInitial(QSqlDatabase db,QString *error) {
    QSqlQuery check(db);
    if(!check.exec("SELECT 1 FROM integration_migrations WHERE name='employes-aniworld2-v1'")) { *error=check.lastError().text(); return false; }
    if(check.next()) return true;
    if(!check.exec("SELECT COUNT(*) FROM employes") || !check.next()) { *error=check.lastError().text(); return false; }
    if(check.value(0).toInt()>0) { *error="L'import initial ne peut pas remplacer un annuaire déjà rempli."; return false; }
    QTemporaryDir temporary;
    const QString path=temporary.filePath("initial.sqlite");
    if(!temporary.isValid() || !QFile::copy(":/data/employees-initial.sqlite",path)) { *error="La base initiale des employés est introuvable."; return false; }
    bool ok=true;
    {
        QSqlDatabase source=QSqlDatabase::addDatabase("QSQLITE","employees-import");
        source.setDatabaseName(path); source.setConnectOptions("QSQLITE_OPEN_READONLY");
        if(!source.open()) { *error=source.lastError().text(); ok=false; }
        if(ok && !db.transaction()) { *error=db.lastError().text(); ok=false; }
        if(ok) {
            for(const QString &table:{QString("employes"),QString("planning_affectations")}) {
                QSqlQuery read(source),write(db);
                if(!read.exec("SELECT * FROM "+table)) { *error=read.lastError().text(); ok=false; break; }
                QStringList columns,marks; const auto record=read.record();
                for(int i=0;i<record.count();++i) { columns<<record.fieldName(i); marks<<"?"; }
                write.prepare("INSERT INTO "+table+"("+columns.join(',')+") VALUES("+marks.join(',')+")");
                while(read.next()) {
                    for(int i=0;i<record.count();++i) write.bindValue(i,read.value(i));
                    if(!write.exec()) { *error=write.lastError().text(); ok=false; break; }
                }
                if(!ok) break;
            }
            QSqlQuery mark(db);
            if(ok && !mark.exec("INSERT INTO integration_migrations VALUES('employes-aniworld2-v1')")) { *error=mark.lastError().text(); ok=false; }
            if(ok && !db.commit()) { *error=db.lastError().text(); ok=false; }
            if(!ok) db.rollback();
        }
        source.close();
    }
    QSqlDatabase::removeDatabase("employees-import");
    return ok;
}
}
