#include "servicereferences.h"
#include "../database/oracleconnection.h"
#include <QSqlQuery>
#include <QSqlError>
QList<QPair<qint64, QString>> ServiceReferences::proprietaires(QString *error) {
  QList<QPair<qint64, QString>> result;
  QSqlQuery q(OracleConnection::database());
  q.prepare(OracleConnection::isOracle()
      ? "SELECT MIN(id_doss),TRIM(proprietaire) FROM medical WHERE TRIM(proprietaire) IS NOT NULL GROUP BY TRIM(proprietaire) ORDER BY TRIM(proprietaire)"
      : "SELECT MIN(id),TRIM(owner) FROM medical_records WHERE TRIM(owner)<>'' GROUP BY TRIM(owner) ORDER BY TRIM(owner)");
  if (!q.exec()) { if(error)*error=q.lastError().text(); return result; }
  while(q.next()) result.append({q.value(0).toLongLong(),q.value(1).toString()});
  if(error)error->clear();
  return result;
}
QString ServiceReferences::proprietaire(qint64 id, QString *error) {
  QSqlQuery query(OracleConnection::database());
  query.prepare(OracleConnection::isOracle()
                    ? "SELECT proprietaire FROM medical WHERE id_doss=:id"
                    : "SELECT owner FROM medical_records WHERE id=:id");
  query.bindValue(":id", QString::number(id));
  if (!query.exec()) {
    *error = query.lastError().text();
    return {};
  }
  if (!query.next() || query.value(0).toString().trimmed().isEmpty()) {
    *error = "Le dossier médical source n'existe plus ou son propriétaire est vide. Actualisez les dossiers.";
    return {};
  }
  error->clear();
  return query.value(0).toString().trimmed();
}
