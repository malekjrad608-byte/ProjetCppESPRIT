#include <QCoreApplication>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include "../src/database/oracleconnection.h"
int main(int argc,char **argv) {
    QCoreApplication a(argc,argv);a.setApplicationName("ANIWORLD");a.setOrganizationName("ANIWORLD");
    QSqlDatabase db;QString error;
    if(!OracleConnection::open(db,&error)) {qCritical()<<error;return 1;}
    QSqlQuery q(db);
    auto result=[&](const QString &label,bool ok){qInfo()<<label<<ok<<q.lastError().type()<<q.lastError().nativeErrorCode()<<q.lastError().text();};
    result("prepare SELECT",q.prepare("SELECT ? FROM dual"));q.addBindValue(QString("hello"));result("exec SELECT",q.exec());
    if(q.next()) qInfo()<<q.value(0);
    db.transaction();
    result("prepare INSERT",q.prepare("INSERT INTO accounts(username,role,salt,password_hash,iterations) VALUES(?,?,HEXTORAW(?),HEXTORAW(?),?)"));
    q.addBindValue(QString("oracle-probe"));q.addBindValue(QString("services"));q.addBindValue(QString(48,'a'));q.addBindValue(QString(64,'b'));q.addBindValue(100000);
    result("exec INSERT int",q.exec());q.finish();db.rollback();
    db.transaction();
    result("prepare INSERT2",q.prepare("INSERT INTO accounts(username,role,salt,password_hash,iterations) VALUES(?,?,HEXTORAW(?),HEXTORAW(?),?)"));
    q.addBindValue(QString("oracle-probe"));q.addBindValue(QString("services"));q.addBindValue(QString(48,'a'));q.addBindValue(QString(64,'b'));q.addBindValue(qlonglong(100000));
    result("exec INSERT longlong",q.exec());q.finish();db.rollback();
}
