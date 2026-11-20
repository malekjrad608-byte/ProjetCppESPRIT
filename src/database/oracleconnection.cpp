#include "oracleconnection.h"
#include "connection.h"
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QSettings>
#include <QSqlError>
#include <QSqlRecord>
#include <QStandardPaths>

QString OracleConnection::defaultConfigPath() {
  return QStandardPaths::writableLocation(
             QStandardPaths::AppLocalDataLocation) +
         "/oracle.ini";
}
bool OracleConnection::configured() {
  QSettings s(defaultConfigPath(), QSettings::IniFormat);
  return s.value("oracle/enabled", false).toBool();
}
QSqlDatabase OracleConnection::database() {
  // The original Connection class owns Qt's default ODBC connection.
  // A closed Oracle handle may remain registered while a demo/test uses SQLite.
  return QSqlDatabase::contains("aniworld-services")
             ? QSqlDatabase::database("aniworld-services", false)
             : QSqlDatabase::database(QSqlDatabase::defaultConnection, false);
}
bool OracleConnection::isOracle() { return database().driverName() == "QODBC"; }
bool OracleConnection::open(QSqlDatabase &db, QString *error,
                            const QString &configPath) {
  const QString path = configPath.isEmpty() ? defaultConfigPath() : configPath;
  QSettings s(path, QSettings::IniFormat);
  const auto bin = s.value("oracle/home").toString() + "/bin";
  qputenv("PATH", bin.toLocal8Bit() + ";" + qgetenv("PATH"));
  // Keep the teacher's connection.cpp/.h unchanged: only its three DB
  // parameters differ.
  Connection &connection = Connection::createInstance();
  const bool opened = connection.createConnection();
  db = QSqlDatabase::database(QSqlDatabase::defaultConnection, false);
  if (!opened) {
    *error = db.lastError().text();
    return false;
  }
  QSqlQuery q(db);
  if (!(q.prepare("SELECT COUNT(*) FROM user_tables") && q.exec()) ||
      !q.next() || q.value(0).toInt() != 5) {
    *error =
        "Le schéma Oracle doit contenir exactement les cinq tables métier.";
    db.close();
    return false;
  }
  if (!(q.prepare(
            "SELECT COUNT(*) FROM user_tab_columns WHERE (table_name IN "
            "('EMPLOYER','LOCAL') AND column_name='ID_SERV') OR "
            "(table_name='MEDICAL' AND column_name='ID_EMP')") &&
        q.exec()) ||
      !q.next() || q.value(0).toInt() != 3) {
    *error = "Migration vers le modèle à cinq tables nécessaire.";
    db.close();
    return false;
  }
  error->clear();
  return true;
}
qint64 OracleConnection::insertedId(const QSqlQuery &query,
                                    const QString &table) {
  if (!isOracle())
    return query.lastInsertId().toLongLong();
  // Whitelist identifiers; CURRVAL is private to this session, unlike MAX(id).
  if (!QStringList{"services", "medical_records"}.contains(table))
    return 0;
  QSqlQuery id(OracleConnection::database());
  return (id.prepare("SELECT " + table + "_seq.CURRVAL FROM dual") &&
          id.exec()) &&
                 id.next()
             ? id.value(0).toLongLong()
             : 0;
}
bool OracleConnection::migrateLocal(const QString &, QString *error) {
  *error =
      "L’import de l’ancien modèle est désactivé dans le schéma à cinq tables.";
  return false;
}

QString OracleConnection::diagnostic(QString *error) {
  QSqlQuery query(OracleConnection::database());
  query.prepare("SELECT USER,SYS_CONTEXT('USERENV','CON_NAME') FROM dual");
  if (!query.exec() || !query.next()) {
    *error = query.lastError().text();
    return {};
  }
  QString report = "Connexion Qt QODBC : " + query.value(0).toString() + " / " +
                   query.value(1).toString() + "\n";
  for (const QString &table :
       {QString("services"), QString("employes"), QString("medical_records"),
        QString("locaux"), QString("stock_lots")}) {
    query.prepare("SELECT COUNT(*) FROM " + table);
    if (!query.exec() || !query.next()) {
      *error = query.lastError().text();
      return {};
    }
    report += table + " : " + query.value(0).toString() + "\n";
  }
  error->clear();
  return report;
}

int OracleConnection::nextId(const QString &sequence) {
  QSqlQuery q(database());
  if (!(q.prepare("SELECT " + sequence + ".NEXTVAL FROM dual") && q.exec()) ||
      !q.next())
    return 0;
  return q.value(0).toInt();
}

QString OracleConnection::friendlyError(const QString &s) {
  if (s.contains("UQ_EMPLOYER_CIN"))return "Ce CIN appartient déjà à un autre employé.";
  if (s.contains("UQ_MEDICAL_ANIMAL"))return "Cet identifiant animal existe déjà.";
  if (s.contains("UQ_STOCK_ARTICLE"))return "Cette référence article existe déjà.";
  if (s.contains("20051"))
    return "Cet employé est déjà occupé sur ce créneau. Choisissez un autre "
           "horaire.";
  if (s.contains("20041"))
    return "La facture a déjà été émise. Ses informations sont conservées.";
  if (s.contains("FK_SERVICE_TEAM") || s.contains("FK_EMPLOYE_SERVICE"))
    return "Ce service est lié à des employés. Réaffectez-les à un autre service avant de le supprimer.";
  if (s.contains("02292"))
    return "Cet élément est encore utilisé. Réaffectez ses éléments liés avant "
           "suppression.";
  return s.section(QChar(0), 0, 0);
}
