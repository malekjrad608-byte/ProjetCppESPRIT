#include "employeequeries.h"
#include "../database/oracleconnection.h"
#include <QSqlError>
#include <QSqlRecord>
ModelRows EmployeeQueries::read(const QString &sql,
                                const QVariantList &values) {
  ModelRows result;
  QSqlQuery query(OracleConnection::database());
  if (!query.prepare(sql)) {
    result.error = {query.lastError().text(),
                    query.lastError().nativeErrorCode()};
    return result;
  }
  for (const auto &value : values)
    query.addBindValue(value);
  if (!query.exec()) {
    result.error = {query.lastError().text(),
                    query.lastError().nativeErrorCode()};
    return result;
  }
  while (query.next()) {
    QVariantList row;
    for (int i = 0; i < query.record().count(); ++i)
      row.append(query.value(i));
    result.rows.append(row);
  }
  if (query.lastError().isValid())
    result.error = {query.lastError().text(),
                    query.lastError().nativeErrorCode()};
  return result;
}
ModelRows EmployeeQueries::planningEmployees() {
  return read(
      "SELECT id,nom,prenom,disponible FROM employes ORDER BY nom,prenom,id");
}