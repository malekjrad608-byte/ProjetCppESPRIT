#include "oraclecollection.h"
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMap>
#include <QSet>
#include <QCoreApplication>

namespace {
QString table(const QString &name) { return name=="stock"?"stock_lots":"locaux"; }
QStringList columns(const QString &name) {
    return name=="stock"?QStringList{"lot","article","category","quantity","unit","threshold","expiry"}
        :QStringList{"id","name","type","floor","capacity","occupancy","specialty","out_of_service","cleaning","last_cleaned","dirty_since"};
}
bool allowed(const QString &name) { return name=="stock" || name=="locaux"; }
bool fail(QSqlDatabase db,QString *error,const QString &message) { db.rollback();*error=message;return false; }
}
namespace OracleCollection {
bool read(const QString &name,Rows *rows,qint64 *revision,QString *error) {
    if(!allowed(name)) { *error="Collection inconnue.";return false; }
    auto db=QSqlDatabase::database("aniworld-services");
    if(!db.transaction()) { *error=db.lastError().text();return false; }
    QSqlQuery q(db);q.prepare("SELECT revision FROM collection_versions WHERE name=? FOR UPDATE WAIT 5");q.addBindValue(name);
    if(!q.exec() || !q.next()) return fail(db,error,q.lastError().text());
    const qint64 version=q.value(0).toLongLong();q.finish();
    const auto fields=columns(name);
    if(!q.exec("SELECT "+fields.join(',')+" FROM "+table(name)+" ORDER BY "+fields[0])) return fail(db,error,q.lastError().text());
    Rows result;
    while(q.next()) { QVariantList row; for(int i=0;i<fields.size();++i) row<<q.value(i); result<<row; }
    q.finish();
    if(!db.commit()) return fail(db,error,db.lastError().text());
    *rows=result;*revision=version;error->clear();return true;
}
bool save(const QString &name,const Rows &rows,qint64 *revision,QString *error) {
    if(!allowed(name)) { *error="Collection inconnue.";return false; }
    const auto fields=columns(name);QSet<QString> keys;
    for(const auto &row:rows) {
        if(row.size()!=fields.size() || row[0].toString().trimmed().isEmpty() || keys.contains(row[0].toString())) {
            *error="Identifiant ou référence de lot vide ou en double.";return false;
        }
        keys.insert(row[0].toString());
    }
    auto db=QSqlDatabase::database("aniworld-services");
    if(!db.transaction()) { *error=db.lastError().text();return false; }
    QSqlQuery q(db);q.prepare("SELECT revision FROM collection_versions WHERE name=? FOR UPDATE WAIT 5");q.addBindValue(name);
    if(!q.exec() || !q.next()) return fail(db,error,q.lastError().text());
    if(q.value(0).toLongLong()!=*revision) return fail(db,error,"Ces données ont été modifiées par une autre session. Actualisez avant de réessayer.");
    q.finish();
    QMap<QString,QVariantList> old;
    if(!q.exec("SELECT "+fields.join(',')+" FROM "+table(name))) return fail(db,error,q.lastError().text());
    while(q.next()) { QVariantList row;for(int i=0;i<fields.size();++i)row<<q.value(i);old.insert(row[0].toString(),row); }q.finish();
    auto audit=[&](const QVariantList &before,const QVariantList &after,const QString &action) {
        if(name!="stock") return true;
        const auto &row=after.isEmpty()?before:after;
        QSqlQuery journal(db);journal.prepare("INSERT INTO stock_movements(lot,article,quantity_before,quantity_after,action,changed_by) VALUES(?,?,?,?,?,?)");
        journal.addBindValue(row[0]);journal.addBindValue(row[1]);journal.addBindValue(before.isEmpty()?0:before[3].toInt());
        journal.addBindValue(after.isEmpty()?0:after[3].toInt());journal.addBindValue(action);
        journal.addBindValue(QCoreApplication::instance()->property("sessionUsername").toString());
        if(!journal.exec()) { *error=journal.lastError().text();return false; }return true;
    };
    for(auto it=old.cbegin();it!=old.cend();++it) if(!keys.contains(it.key())) {
        q.prepare("DELETE FROM "+table(name)+" WHERE "+fields[0]+"=?");q.addBindValue(it.value()[0]);
        if(!q.exec()) return fail(db,error,q.lastError().text());
        if(!audit(it.value(),{},"Suppression")) {db.rollback();return false;}
    }
    for(const auto &row:rows) {
        const auto before=old.value(row[0].toString());bool changed=before.isEmpty();
        for(int i=0;!changed && i<fields.size();++i) changed=before[i].toString()!=row[i].toString();
        if(!changed) continue;
        if(before.isEmpty()) {
            QStringList marks;for(int i=0;i<fields.size();++i)marks<<"?";
            q.prepare("INSERT INTO "+table(name)+"("+fields.join(',')+") VALUES("+marks.join(',')+")");
            for(const auto &value:row) q.addBindValue(value);
        } else {
            QStringList assignments;for(int i=1;i<fields.size();++i)assignments<<fields[i]+"=?";
            q.prepare("UPDATE "+table(name)+" SET "+assignments.join(',')+" WHERE "+fields[0]+"=?");
            for(int i=1;i<fields.size();++i) q.addBindValue(row[i]);
            q.addBindValue(row[0]);
        }
        if(!q.exec()) return fail(db,error,q.lastError().text());
        if(!audit(before,row,before.isEmpty()?"Ajout":"Modification / mouvement")) {db.rollback();return false;}
    }
    q.prepare("UPDATE collection_versions SET revision=revision+1 WHERE name=?");q.addBindValue(name);
    if(!q.exec()) return fail(db,error,q.lastError().text());
    if(!db.commit()) return fail(db,error,db.lastError().text());
    ++*revision;error->clear();return true;
}
}
