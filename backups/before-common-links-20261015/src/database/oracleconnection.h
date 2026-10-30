#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>

namespace OracleConnection {
QString defaultConfigPath();
bool configured();
bool open(QSqlDatabase &db, QString *error, const QString &configPath = {});
bool isOracle();
qint64 insertedId(const QSqlQuery &query, const QString &table);
bool migrateLocal(const QString &path, QString *error);
}
