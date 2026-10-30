#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>

class OracleConnection {
public:
  static QString defaultConfigPath();
  static bool configured();
  // Active application handle: teacher Singleton in Oracle, isolated SQLite in
  // demo/tests.
  static QSqlDatabase database();
  static bool open(QSqlDatabase &db, QString *error,
                   const QString &configPath = {});
  static bool isOracle();
  static QString diagnostic(QString *error);
  static qint64 insertedId(const QSqlQuery &query, const QString &table);
  static bool migrateLocal(const QString &path, QString *error);
  static int nextId(const QString &sequence);
  static QString friendlyError(const QString &message);
};
