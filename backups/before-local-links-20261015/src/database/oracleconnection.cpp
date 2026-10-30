#include "oracleconnection.h"
#include <QSettings>
#include <QStandardPaths>
#include <QFileInfo>
#include <QFile>
#include <QSqlError>
#include <QSqlRecord>
#include <QSet>
#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#endif

namespace OracleConnection {
QString defaultConfigPath() { return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/oracle.ini"; }
bool configured() { QSettings s(defaultConfigPath(),QSettings::IniFormat); return s.value("oracle/enabled",false).toBool(); }
bool isOracle() { return QSqlDatabase::database("aniworld-services",false).driverName()=="QODBC"; }
bool open(QSqlDatabase &db,QString *error,const QString &configPath) {
    const QString path=configPath.isEmpty()?defaultConfigPath():configPath;
    QSettings s(path,QSettings::IniFormat);
    QFile file(QFileInfo(path).absolutePath()+"/"+s.value("oracle/passwordFile").toString());
    if(!file.open(QIODevice::ReadOnly)) { *error="Configuration Oracle incomplète : mot de passe Windows introuvable."; return false; }
    const QByteArray encrypted=file.readAll(); QByteArray password;
#ifdef Q_OS_WIN
    DATA_BLOB input{DWORD(encrypted.size()),reinterpret_cast<BYTE *>(const_cast<char *>(encrypted.data()))},output{};
    if(!CryptUnprotectData(&input,nullptr,nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&output)) {
        *error="Le mot de passe Oracle appartient à un autre compte Windows ou est endommagé."; return false;
    }
    password=QByteArray(reinterpret_cast<char *>(output.pbData),output.cbData);
    SecureZeroMemory(output.pbData,output.cbData); LocalFree(output.pbData);
#else
    *error="Cette configuration utilise la protection Windows DPAPI."; return false;
#endif
    const auto bin=s.value("oracle/home").toString()+"/bin";
    qputenv("PATH",bin.toLocal8Bit()+";"+qgetenv("PATH"));
    db=QSqlDatabase::addDatabase("QODBC","aniworld-services");
    db.setDatabaseName(s.value("oracle/dsn").toString());
    db.setConnectOptions("SQL_ATTR_LOGIN_TIMEOUT=5");
    const bool opened=db.open(s.value("oracle/username").toString(),QString::fromUtf8(password));
    password.fill('\0');
    if(!opened) { *error=db.lastError().text(); return false; }
    QSqlQuery q(db);
    if(!q.exec("SELECT version FROM schema_version WHERE id=1") || !q.next() || q.value(0).toInt()!=1) {
        *error="Schéma Oracle absent ou incompatible. Exécutez tools/install-oracle-schema.ps1.";db.close();return false;
    }
    error->clear();return true;
}
qint64 insertedId(const QSqlQuery &query,const QString &table) {
    if(!isOracle()) return query.lastInsertId().toLongLong();
    // Whitelist identifiers; CURRVAL is private to this session, unlike MAX(id).
    if(!QStringList{"services","medical_records"}.contains(table)) return 0;
    QSqlQuery id(QSqlDatabase::database("aniworld-services"));
    return id.exec("SELECT "+table+"_seq.CURRVAL FROM dual") && id.next()?id.value(0).toLongLong():0;
}
bool migrateLocal(const QString &path,QString *error) {
    auto db=QSqlDatabase::database("aniworld-services"); QSqlQuery q(db);
    if(!q.exec("SELECT 1 FROM integration_migrations WHERE name='sqlite-to-oracle-v1'")) { *error=q.lastError().text();return false; }
    if(q.next()) return true;
    q.finish();
    const QStringList tables={"accounts","services","employes","planning_affectations","medical_records","integration_migrations"};
    for(const auto &table:tables) {
        if(!q.exec("SELECT COUNT(*) FROM "+table) || !q.next()) { *error=q.lastError().text();return false; }
        if(q.value(0).toInt()!=0) { *error="Migration arrêtée : les tables Oracle doivent être vides. Aucune donnée remplacée.";return false; }
    }
    q.finish();
    if(!QFile::exists(path)) { *error="Base locale à importer introuvable.";return false; }
    bool ok=true;
    {
        auto source=QSqlDatabase::addDatabase("QSQLITE","oracle-migration-source");
        source.setDatabaseName(path);source.setConnectOptions("QSQLITE_OPEN_READONLY");
        if(!source.open()) { *error=source.lastError().text();ok=false; }
        if(ok && !db.transaction()) { *error=db.lastError().text();ok=false; }
        if(ok) {
            for(const auto &table:tables) {
                QSqlQuery read(source),write(db);
                if(!read.exec("SELECT * FROM "+table)) { *error=read.lastError().text();ok=false;break; }
                QStringList columns,marks; const auto record=read.record();
                for(int i=0;i<record.count();++i) {
                    const auto name=record.fieldName(i);columns<<name;
                    marks<<(table=="accounts" && (name=="salt" || name=="password_hash")?"HEXTORAW(?)":"?");
                }
                write.prepare("INSERT INTO "+table+"("+columns.join(',')+") VALUES("+marks.join(',')+")");
                while(read.next()) {
                    for(int i=0;i<record.count();++i) {
                        const auto name=record.fieldName(i);
                        QVariant value=read.value(i);
                        if(table=="accounts" && (name=="salt" || name=="password_hash")) value=QString::fromLatin1(value.toByteArray().toHex());
                        // Oracle's Windows ODBC driver rejects SQL_C_SBIGINT parameters.
                        // Decimal text preserves the exact integer without a lossy double.
                        else if(value.metaType().id()==QMetaType::LongLong || value.metaType().id()==QMetaType::ULongLong) value=value.toString();
                        write.bindValue(i,value);
                    }
                    if(!write.exec()) { *error=table+": "+write.lastError().text();ok=false;break; }
                }
                if(!ok) break;
            }
            for(const QString &table:{QString("services"),QString("employes"),QString("planning_affectations"),QString("medical_records")}) {
                if(!ok) break;
                if(!q.exec("SELECT COALESCE(MAX(id),0) FROM "+table) || !q.next()) { *error=q.lastError().text();ok=false;break; }
                const qint64 maximum=q.value(0).toLongLong();
                do {
                    if(!q.exec("SELECT "+table+"_seq.NEXTVAL FROM dual") || !q.next()) { *error=q.lastError().text();ok=false;break; }
                } while(q.value(0).toLongLong()<=maximum);
            }
            if(ok && !q.exec("INSERT INTO integration_migrations VALUES('sqlite-to-oracle-v1')")) { *error=q.lastError().text();ok=false; }
            if(ok && !db.commit()) { *error=db.lastError().text();ok=false; }
            if(!ok) db.rollback();
        }
        source.close();
    }
    QSqlDatabase::removeDatabase("oracle-migration-source");return ok;
}
}
