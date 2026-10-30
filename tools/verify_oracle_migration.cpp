#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>
#include <QSqlRecord>
#include <QSqlError>
#include <cmath>
#include "../src/database/oracleconnection.h"

// Read-only comparison. Never logs account hashes, credentials or patient data.
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("ANIWORLD"); app.setOrganizationName("ANIWORLD");
    QSqlDatabase oracle; QString error;
    if (!OracleConnection::open(oracle, &error)) { qCritical() << error; return 1; }
    auto local = QSqlDatabase::addDatabase("QSQLITE", "migration-verification");
    local.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/services.sqlite");
    local.setConnectOptions("QSQLITE_OPEN_READONLY");
    if (!local.open()) { qCritical() << local.lastError().text(); return 1; }
    for (const QString &table : {QString("accounts"), QString("services"), QString("employes"), QString("planning_affectations"), QString("medical_records")}) {
        const QString key = table == "accounts" ? "username" : "id";
        QSqlQuery source(local), target(oracle);
        if (!source.exec("SELECT * FROM "+table+" ORDER BY "+key)) return 2;
        QStringList columns;
        for (int i=0; i<source.record().count(); ++i) {
            const QString column=source.record().fieldName(i);
            columns << (table=="accounts" && (column=="salt" || column=="password_hash") ? "RAWTOHEX("+column+")" : column);
        }
        if (!target.exec("SELECT "+columns.join(',')+" FROM "+table+" ORDER BY "+key)) { qCritical()<<target.lastError().text(); return 2; }
        int rows=0, cells=0;
        while (source.next()) {
            if (!target.next()) { qCritical()<<table<<"missing row"; return 3; }
            ++rows;
            for (int i=0; i<source.record().count(); ++i) {
                auto a=source.value(i), b=target.value(i); bool same;
                if (a.metaType().id()==QMetaType::QByteArray)
                    same=a.toByteArray()==QByteArray::fromHex(b.toString().toLatin1());
                else if (a.metaType().id()==QMetaType::LongLong || a.metaType().id()==QMetaType::Int)
                    same=a.toLongLong()==b.toLongLong();
                else if (a.metaType().id()==QMetaType::Double)
                    same=std::abs(a.toDouble()-b.toDouble())<1e-8;
                else same=a.toString()==b.toString(); // Oracle treats empty strings as NULL.
                if (!same) { qCritical()<<table<<"row"<<rows<<"column"<<source.record().fieldName(i)<<"differs"; return 3; }
                ++cells;
            }
        }
        if (target.next()) { qCritical()<<table<<"unexpected additional row"; return 3; }
        qInfo().noquote()<<table<<":"<<rows<<"rows,"<<cells<<"values identical";
    }
    QSqlQuery q(oracle);
    if (!q.exec("SELECT COUNT(*) FROM user_objects WHERE status<>'VALID'") || !q.next() || q.value(0).toInt()!=0) return 4;
    qInfo()<<"All schema objects valid. Original SQLite data and account credentials preserved.";
    return 0;
}
