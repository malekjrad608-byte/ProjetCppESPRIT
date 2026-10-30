#include "accounts.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QVariant>

namespace {
constexpr int iterations=100000;
QByteArray salt() {
    QByteArray value(24,Qt::Uninitialized);
    for(auto &c:value) c=char(QRandomGenerator::system()->generate() & 0xff);
    return value;
}
QByteArray digest(const QString &password,const QByteArray &salt,int count) {
    return QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256,password.toUtf8(),salt,count,32);
}
Accounts::Role parse(const QString &role) {
    if(role=="admin") return Accounts::Role::Admin;
    if(role=="services") return Accounts::Role::Services;
    if(role=="stock") return Accounts::Role::Stock;
    if(role=="locaux") return Accounts::Role::Locaux;
    if(role=="employes") return Accounts::Role::Employees;
    return Accounts::Role::None;
}
}
namespace Accounts {
bool initialize(QSqlDatabase db,QString *error) {
    QSqlQuery q(db);
    auto fail=[&] { if(error) *error=q.lastError().text(); q.finish(); db.rollback(); return false; };
    if(!db.transaction()) { if(error) *error=db.lastError().text(); return false; }
    if(!q.exec("SELECT sql FROM sqlite_master WHERE type='table' AND name='accounts'")) return fail();
    const bool migrate=q.next() && !q.value(0).toString().contains("'locaux'");
    q.finish();
    if(migrate && !q.exec("ALTER TABLE accounts RENAME TO accounts_before_locaux")) return fail();
    if(!q.exec("CREATE TABLE IF NOT EXISTS accounts(username TEXT PRIMARY KEY COLLATE NOCASE, role TEXT NOT NULL CHECK(role IN ('admin','services','stock','employes','locaux')), salt BLOB NOT NULL, password_hash BLOB NOT NULL, iterations INTEGER NOT NULL)")) return fail();
    if(migrate) {
        if(!q.exec("INSERT INTO accounts SELECT * FROM accounts_before_locaux")) return fail();
        if(!q.exec("DROP TABLE accounts_before_locaux")) return fail();
    }
    for(const QString &name:{QString("admin"),QString("services"),QString("stock"),QString("employes"),QString("locaux")}) {
        q.prepare("SELECT 1 FROM accounts WHERE username=?"); q.addBindValue(name);
        if(!q.exec()) { *error=q.lastError().text(); db.rollback(); return false; }
        if(q.next()) continue;
        const QByteArray nonce=salt();
        q.prepare("INSERT INTO accounts VALUES(?,?,?,?,?)"); q.addBindValue(name); q.addBindValue(name);
        q.addBindValue(nonce); q.addBindValue(digest(name+"123",nonce,iterations)); q.addBindValue(iterations);
        if(!q.exec()) { *error=q.lastError().text(); db.rollback(); return false; }
    }
    if(!db.commit()) { *error=db.lastError().text(); return false; }
    return true;
}
Role authenticate(const QString &username,const QString &password) {
    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
    q.prepare("SELECT role,salt,password_hash,iterations FROM accounts WHERE username=?"); q.addBindValue(username.trimmed());
    if(!q.exec() || !q.next()) return Role::None;
    const int count=q.value(3).toInt(); if(count<10000 || count>1000000) return Role::None;
    const QByteArray expected=q.value(2).toByteArray(),actual=digest(password,q.value(1).toByteArray(),count);
    if(expected.size()!=actual.size()) return Role::None;
    unsigned difference=0; for(int i=0;i<actual.size();++i) difference|=quint8(actual[i]) ^ quint8(expected[i]);
    return difference==0?parse(q.value(0).toString()):Role::None;
}
bool changePassword(const QString &username,const QString &current,const QString &replacement,QString *error) {
    if(authenticate(username,current)==Role::None) { *error="Le mot de passe actuel est incorrect."; return false; }
    if(replacement.size()<8 || replacement.size()>128) { *error="Utilisez entre 8 et 128 caractères."; return false; }
    const QByteArray nonce=salt(); QSqlQuery q(QSqlDatabase::database("aniworld-services"));
    q.prepare("UPDATE accounts SET salt=?,password_hash=?,iterations=? WHERE username=?");
    q.addBindValue(nonce); q.addBindValue(digest(replacement,nonce,iterations)); q.addBindValue(iterations); q.addBindValue(username);
    if(!q.exec() || q.numRowsAffected()!=1) { *error="Le changement n'a pas pu être enregistré."; return false; }
    return true;
}
bool allows(Role role,int page) {
    if(role==Role::Admin) return page>=0 && page<=4;
    return (role==Role::Services && (page==0 || page==1)) || (role==Role::Stock && page==2) || (role==Role::Employees && page==3) || (role==Role::Locaux && page==4);
}
int homePage(Role role) { return role==Role::Locaux?4:role==Role::Stock?2:role==Role::Employees?3:1; }
QString label(Role role) { return role==Role::Locaux?"Locaux":role==Role::Admin?"Administration":role==Role::Services?"Services":role==Role::Stock?"Stock":role==Role::Employees?"Employés":"Déconnecté"; }
}
