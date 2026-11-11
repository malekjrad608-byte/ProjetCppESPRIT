#include "oracleconnection.h"
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QSettings>
#include <QSqlError>
#include <QSqlRecord>
#include <QStandardPaths>

// Adapter only: Connection in the host owns the one Qt SQL connection.
// This module never opens, closes, replaces or configures that connection.
QSqlDatabase OracleConnection::database() {
  return QSqlDatabase::database(QSqlDatabase::defaultConnection, false);
}
bool OracleConnection::isOracle() { return database().driverName() == "QODBC"; }
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
