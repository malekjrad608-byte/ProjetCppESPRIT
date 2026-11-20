#include "modulesupplements.h"
#include "../database/oracleconnection.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QLockFile>
#include <QSaveFile>
#include <QStandardPaths>

QString ModuleSupplements::path(const QString &module) {
    const auto db=OracleConnection::database();
    const auto identity=(db.driverName()+"|"+db.databaseName()+"|"+db.userName()).toUtf8();
    const auto key=QCryptographicHash::hash(identity,QCryptographicHash::Sha256).toHex().left(16);
    const QString dir=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/modules/"+key;
    QDir().mkpath(dir);
    return dir+"/"+module+".json";
}

QJsonArray ModuleSupplements::read(const QString &module, QString *error) {
    if(error)error->clear();
    QFile f(path(module));
    if(!f.exists())return {};
    if(!f.open(QIODevice::ReadOnly)){if(error)*error=f.errorString();return {};}
    QJsonParseError parse;
    const auto doc=QJsonDocument::fromJson(f.readAll(),&parse);
    if(parse.error!=QJsonParseError::NoError || !doc.isArray()){
        if (error) {
            *error = "Fichier complementaire illisible : " + f.fileName();
        }
        return {};
    }
    return doc.array();
}

bool ModuleSupplements::update(const QString &module,
                              const std::function<bool(QJsonArray &)> &change,
                              QString *error) {
    const QString filename=path(module);
    QLockFile lock(filename+".lock");
    if(!lock.tryLock(2000)){if(error)*error="Donnees complementaires utilisees par une autre instance.";return false;}
    QString issue;
    auto rows=read(module,&issue);
    if(!issue.isEmpty()){if(error)*error=issue;return false;}
    if(!change(rows)){if(error && error->isEmpty())*error="Modification refusee ou element introuvable.";return false;}
    QSaveFile file(filename);
    const auto data=QJsonDocument(rows).toJson();
    if(!file.open(QIODevice::WriteOnly) || file.write(data)!=data.size() || !file.commit()) {
        if (error) {
            *error = file.errorString();
        }
        return false;
    }
    if(error)error->clear();
    return true;
}
