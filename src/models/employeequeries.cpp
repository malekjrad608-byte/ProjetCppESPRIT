#include "employeequeries.h"
#include "../database/oracleconnection.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

const QString EmployeeQueries::columns =
    "id,nom,prenom,courriel,telephone,poste,date_embauche,disponible,charge_"
    "hebdo,dispo_hebdo,competences";
const QString EmployeeQueries::joinWeek =
    " LEFT JOIN (SELECT responsable_id,SUM(charge_heures) AS plan FROM "
    "planning_affectations WHERE debut>=? AND debut<? GROUP BY responsable_id) "
    "p ON p.responsable_id=e.id ";
const QString EmployeeQueries::charge =
    "(COALESCE(e.charge_hebdo,0)+COALESCE(p.plan,0))";

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
ModelRows EmployeeQueries::conflicts() {
  return read("SELECT e.prenom || ' ' || "
              "e.nom,p1.titre,p1.debut,p1.fin,p2.titre,p2.debut,p2.fin FROM "
              "planning_affectations p1 JOIN planning_affectations p2 ON "
              "p1.responsable_id=p2.responsable_id AND p1.id<p2.id AND "
              "p1.debut<p2.fin AND p2.debut<p1.fin JOIN employes e ON "
              "e.id=p1.responsable_id ORDER BY p1.debut" +
              QString(OracleConnection::isOracle() ? " FETCH FIRST 20 ROWS ONLY"
                                                   : " LIMIT 20"));
}
ModelRows EmployeeQueries::consecutive(const QString &today) {
  return read(
      "SELECT p.responsable_id,e.prenom,e.nom,p.debut,p.fin FROM "
      "planning_affectations p JOIN employes e ON e.id=p.responsable_id WHERE "
      "p.fin>=? ORDER BY p.responsable_id,p.debut",
      {today});
}
ModelRows EmployeeQueries::statistics(const QString &start,
                                      const QString &finish) {
  return read("SELECT COUNT(*),COALESCE(SUM(" + charge + "),0),COALESCE(AVG(" +
                  charge + "),0),COALESCE(MAX(" + charge +
                  "),0),COALESCE(MIN(" + charge + "),0) FROM employes e" +
                  joinWeek,
              {start, finish});
}
ModelRows EmployeeQueries::workloads(const QString &start,
                                     const QString &finish) {
  return read(
      "SELECT e.prenom,e.nom,e.charge_hebdo,e.dispo_hebdo,COALESCE(p.plan,0) "
      "FROM employes e" +
          joinWeek,
      {start, finish});
}
ModelRows EmployeeQueries::workloadRanking(const QString &start,
                                           const QString &finish) {
  return read("SELECT e.nom,e.prenom,e.poste," + charge +
                  ",e.dispo_hebdo FROM employes e" + joinWeek + "ORDER BY " +
                  charge + " DESC",
              {start, finish});
}
ModelRows EmployeeQueries::list(const QString &filter, const QString &sort,
                                const QString &job, const QString &status) {
  QString sql = "SELECT " + columns + (OracleConnection::isOracle()?",cin,date_naiss,adresse,type_contrat,salaire":"") + " FROM employes WHERE 1=1";
  QVariantList values;
  if (!filter.isEmpty()) {
    sql += " AND (nom LIKE ? OR prenom LIKE ? OR courriel LIKE ? OR poste LIKE "
           "? OR " +
           QString(OracleConnection::isOracle() ? "TO_CHAR(id)"
                                                : "CAST(id AS TEXT)") +
           " LIKE ?)";
    for (int i = 0; i < 5; ++i)
      values.append("%" + filter + "%");
  }
  if (job != "Tous les postes") {
    sql += " AND poste=?";
    values.append(job);
  }
  if (status == "Disponible")
    sql += " AND disponible=1";
  else if (status == "Non disponible")
    sql += " AND disponible=0";
  sql += sort == "charge" ? " ORDER BY charge_hebdo DESC,prenom ASC"
                          : " ORDER BY date_embauche DESC,prenom ASC";
  return read(sql, values);
}
ModelRows EmployeeQueries::find(int id) {
  return read("SELECT " + columns + (OracleConnection::isOracle()?",cin,date_naiss,adresse,type_contrat,salaire":"") + " FROM employes WHERE id=?", {id});
}
ModelRows EmployeeQueries::countAvailable(bool available) {
  return read("SELECT COUNT(*) FROM employes WHERE disponible=?",
              {available ? 1 : 0});
}
ModelRows EmployeeQueries::weeklyCharge(const QString &start,
                                        const QString &finish) {
  return read(
      OracleConnection::isOracle()
          ? "SELECT (SELECT COALESCE(SUM(charge_hebdo),0) FROM "
            "employes)+(SELECT COALESCE(SUM(charge_heures),0) FROM "
            "planning_affectations WHERE debut>=? AND debut<?) FROM dual"
          : "SELECT COALESCE(SUM(e.charge_hebdo),0)+COALESCE((SELECT "
            "SUM(p.charge_heures) FROM planning_affectations p WHERE "
            "p.debut>=? AND p.debut<?),0) FROM employes e",
      {start, finish});
}
ModelRows EmployeeQueries::skilled(const QString &skill) {
  return read("SELECT nom,prenom,poste,charge_hebdo,dispo_hebdo FROM employes "
              "WHERE disponible=1 AND competences LIKE ? ORDER BY "
              "(charge_hebdo*1.0/NULLIF(dispo_hebdo,0)) ASC",
              {"%" + skill + "%"});
}
ModelRows EmployeeQueries::balance() {
  return read("SELECT nom,prenom,poste,charge_hebdo,dispo_hebdo FROM employes "
              "WHERE disponible=1 AND dispo_hebdo>0 ORDER BY "
              "(charge_hebdo*1.0/dispo_hebdo) DESC");
}
ModelRows EmployeeQueries::candidates() {
  return read("SELECT id,nom,prenom,poste,competences,charge_hebdo,dispo_hebdo "
              "FROM employes WHERE disponible=1");
}
ModelRows EmployeeQueries::plannedHours(int id, const QString &start,
                                        const QString &finish) {
  return read(
      "SELECT COALESCE(SUM(charge_heures),0) FROM planning_affectations WHERE "
      "responsable_id=? AND debut>=? AND debut<?",
      {id, start, finish});
}
ModelRows EmployeeQueries::addTask(const QString &title, const QString &skill,
                                   const QString &start, const QString &finish,
                                   double hours, int employee) {
  if (OracleConnection::isOracle()) {
    ModelRows result;
    result.error.message =
        "Les affectations Oracle sont gérées depuis le service.";
    return result;
  }
  return read("INSERT INTO "
              "planning_affectations(titre,competence_requise,debut,fin,charge_"
              "heures,responsable_id) VALUES(?,?,?,?,?,?)",
              {title, skill, start, finish, hours, employee});
}
ModelRows EmployeeQueries::certificateList() {
  return read("SELECT id,nom,prenom,poste FROM employes ORDER BY prenom,nom");
}
ModelRows EmployeeQueries::certificate(int id) {
  return read("SELECT nom,prenom,poste,date_embauche FROM employes WHERE id=?",
              {id});
}
ModelRows EmployeeQueries::planningEmployees() {
  return read(
      "SELECT id,nom,prenom,disponible FROM employes ORDER BY nom,prenom,id");
}
