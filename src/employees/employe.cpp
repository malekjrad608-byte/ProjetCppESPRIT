#include "employe.h"
#include "../database/oracleconnection.h"
#include "../models/employeequeries.h"
#include "../models/service.h"
#include <QFile>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTemporaryDir>
#include <cmath>

Employe::Employe() {}

bool Employe::ajouter(QString *error) {
  if (!enregistrer(false, error))
    return false;
  QSqlQuery query(OracleConnection::database());
  query.prepare(OracleConnection::isOracle()
                    ? "SELECT employes_seq.CURRVAL FROM dual"
                    : "SELECT last_insert_rowid()");
  if (query.exec() && query.next())
    m_id = query.value(0).toInt();
  return true;
}

QList<Employe> Employe::afficher(const QString &filter, const QString &sort,
                                 const QString &job, const QString &status,
                                 QString *error) {
  auto rows = EmployeeQueries::list(filter, sort, job, status);
  if (error)
    *error = rows.lastError().text();
  QList<Employe> result;
  while (rows.next()) {
    Employe e;
    e.m_id = rows.value(0).toInt();
    e.m_nom = rows.value(1).toString();
    e.m_prenom = rows.value(2).toString();
    e.m_courriel = rows.value(3).toString();
    e.m_telephone = rows.value(4).toString();
    e.m_poste = rows.value(5).toString();
    e.m_dateEmbauche = QDate::fromString(rows.value(6).toString(), Qt::ISODate);
    e.m_disponible = rows.value(7).toInt() != 0;
    e.m_chargeHebdo = rows.value(8).toDouble();
    e.m_dispoHebdo = rows.value(9).toDouble();
    e.m_competences = rows.value(10).toString();
    if(OracleConnection::isOracle()) {
      e.setCin(rows.value(11).toString());e.setDateNaissance(QDate::fromString(rows.value(12).toString(),Qt::ISODate));
      e.setAdresse(rows.value(13).toString());e.setTypeContrat(rows.value(14).toString());
      e.setSalaire(rows.value(15).isNull()?-1:rows.value(15).toDouble());
    }
    result.append(e);
  }
  return result;
}

bool Employe::supprimer(int id, QString *error) {
  QSqlQuery query(OracleConnection::database());
  query.prepare(OracleConnection::isOracle()
                    ? "DELETE FROM employes WHERE id=? AND EXISTS "
                      "(SELECT 1 FROM employes other WHERE "
                      "other.id_service=employes.id_service AND other.id<>employes.id)"
                    : "DELETE FROM employes WHERE id=?");
  query.addBindValue(id);
  if (!query.exec()) {
    *error = OracleConnection::friendlyError(query.lastError().text());
    return false;
  }
  if (query.numRowsAffected() != 1) {
    *error = "Employé introuvable ou dernier employé du service. "
             "Le service doit garder au moins un employé.";
    return false;
  }
  error->clear();
  return true;
}

bool Employe::modifier(QString *error) const {
  return enregistrer(true, error);
}

bool Employe::enregistrer(bool update, QString *error) const {
  error->clear();
  if (m_nom.trimmed().isEmpty() || m_prenom.trimmed().isEmpty() ||
      m_nom.size() > 120 || m_prenom.size() > 120 ||
      !m_dateEmbauche.isValid()) {
    *error = "Nom et prénom obligatoires (120 caractères maximum), avec une "
             "date d’embauche valide.";
    return false;
  }
  if (!std::isfinite(m_chargeHebdo) || !std::isfinite(m_dispoHebdo) ||
      m_chargeHebdo < 0 || m_chargeHebdo > 60 || m_dispoHebdo < 0 ||
      m_dispoHebdo > 60) {
    *error = "Les heures doivent être comprises entre 0 et 60.";
    return false;
  }
  if (!m_courriel.isEmpty() &&
      !QRegularExpression(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)")
           .match(m_courriel)
           .hasMatch()) {
    *error = "Saisissez une adresse courriel valide.";
    return false;
  }
  QVariantList values = {m_nom.trimmed(),
                               m_prenom.trimmed(),
                               m_courriel.trimmed(),
                               m_telephone.trimmed(),
                               m_poste,
                               m_dateEmbauche.toString(Qt::ISODate),
                               m_disponible ? 1 : 0,
                               m_chargeHebdo,
                               m_dispoHebdo,
                               m_competences.trimmed()};
  if (OracleConnection::isOracle()) {
    if(getCin().size()>30 || getAdresse().size()>500 || getTypeContrat().size()>80 || !std::isfinite(getSalaire()) || getSalaire() < -1) {
      *error="Vérifiez le CIN, l'adresse, le contrat et le salaire.";return false;
    }
    values<<getCin().trimmed()<<getDateNaissance().toString(Qt::ISODate)<<getAdresse().trimmed()<<getTypeContrat().trimmed()
          <<(getSalaire()<0?QVariant(QMetaType(QMetaType::Double)):QVariant(getSalaire()));
    return Employe::saveEmployee(update ? m_id : 0, m_serviceId, values, error);
  }
  QSqlQuery query(OracleConnection::database());
  query.prepare(
      update
          ? "UPDATE employes SET "
            "nom=?,prenom=?,courriel=?,telephone=?,poste=?,date_embauche=?,"
            "disponible=?,charge_hebdo=?,dispo_hebdo=?,competences=? WHERE id=?"
          : "INSERT INTO "
            "employes(nom,prenom,courriel,telephone,poste,date_embauche,"
            "disponible,charge_hebdo,dispo_hebdo,competences) "
            "VALUES(?,?,?,?,?,?,?,?,?,?)");
  for (const auto &value : values)
    query.addBindValue(value);
  if (update)
    query.addBindValue(m_id);
  if (!query.exec()) {
    *error = query.lastError().text();
    return false;
  }
  if (query.numRowsAffected() != 1) {
    *error = "Employé introuvable. Actualisez la liste.";
    return false;
  }
  return true;
}

bool Employe::basculerDisponibilite(int id, QString *error) {
  QSqlQuery query(OracleConnection::database());
  query.prepare("UPDATE employes SET disponible=CASE WHEN disponible=1 THEN 0 "
                "ELSE 1 END WHERE id=?");
  query.addBindValue(id);
  if (!query.exec()) {
    *error = query.lastError().text();
    return false;
  }
  if (query.numRowsAffected() != 1) {
    *error = "Employé introuvable. Actualisez la liste.";
    return false;
  }
  error->clear();
  return true;
}

double Employe::tauxCharge() const {
  if (m_dispoHebdo <= 0)
    return 0.0;
  return m_chargeHebdo / m_dispoHebdo;
}

bool Employe::depasseSeuil(double seuil) const { return tauxCharge() > seuil; }

bool Employe::moveEmployee(int employee, int target, QString *error) {
  QSqlQuery q(OracleConnection::database());
  q.prepare("SELECT id_service FROM employes WHERE id=? FOR UPDATE WAIT 5");
  q.addBindValue(employee);
  if (!q.exec() || !q.next()) {
    *error = "Employé introuvable ou occupé par une autre modification.";
    return false;
  }
  const int previous = q.value(0).toInt();
  q.finish();
  if (previous == target)
    return true;
  q.prepare("SELECT id FROM employes WHERE id_service=? AND id<>? ORDER BY id");
  q.addBindValue(previous);
  q.addBindValue(employee);
  if (!q.exec() || !q.next()) {
    *error = "Le service d’origine doit garder au moins un employé. "
             "Ajoutez-lui un autre employé avant cette réaffectation.";
    return false;
  }
  q.finish();
  q.prepare("UPDATE employes SET id_service=? WHERE id=?");
  q.addBindValue(target);
  q.addBindValue(employee);
  if (!q.exec()) {
    *error = q.lastError().text();
    return false;
  }
  return true;
}

int Employe::employeeService(int id) {
  QSqlQuery q(OracleConnection::database());
  q.prepare("SELECT id_service FROM employes WHERE id=?");
  q.addBindValue(id);
  return q.exec() && q.next() ? q.value(0).toInt() : 0;
}

bool Employe::saveEmployee(int id, int service, const QList<QVariant> &values,
                           QString *error) {
  if (service <= 0) {
    *error = "Choisissez le service de cet employé.";
    return false;
  }
  auto db = OracleConnection::database();
  if (!db.transaction()) {
    *error = db.lastError().text();
    return false;
  }
  auto fail = [&](QString message) {
    db.rollback();
    *error = OracleConnection::friendlyError(message);
    return false;
  };
  if (id > 0 && employeeService(id) != service) {
    QSqlQuery slot(db);
    slot.prepare("SELECT datetime,duration FROM services WHERE id=?");
    slot.addBindValue(service);
    if (!slot.exec() || !slot.next())
      return fail("Service introuvable.");
    auto start = QDateTime::fromString(slot.value(0).toString(), Qt::ISODate);
    auto end = start.addSecs(slot.value(1).toInt() * 60);
    slot.finish();
    if (!Service::available(id, start, end, service, error))
      return fail(*error);
    if (!moveEmployee(id, service, error))
      return fail(*error);
  }
  QSqlQuery q(db);
  const bool extended=values.size()==15;
  if(values.size()!=10 && !extended)return fail("Profil employé incomplet.");
  QString assignments="nom=?,prenom=?,courriel=?,telephone=?,poste=?,date_embauche=?,disponible=?,charge_hebdo=?,dispo_hebdo=?,competences=?";
  QString fields="nom,prenom,courriel,telephone,poste,date_embauche,disponible,charge_hebdo,dispo_hebdo,competences";
  if(extended){assignments+=",cin=?,date_naiss=?,adresse=?,type_contrat=?,salaire=?";fields+=",cin,date_naiss,adresse,type_contrat,salaire";}
  QStringList marks;for(int i=0;i<values.size()+1;++i)marks<<"?";
  q.prepare(id>0?"UPDATE employes SET "+assignments+" WHERE id=?":"INSERT INTO employes("+fields+",id_service) VALUES("+marks.join(',')+")");
  for (const auto &v : values)
    q.addBindValue(v);
  q.addBindValue(id > 0 ? id : service);
  if (!q.exec())
    return fail(q.lastError().text());
  if (!db.commit())
    return fail(db.lastError().text());
  error->clear();
  return true;
}

bool Employe::initialize(QSqlDatabase db, QString *error) {
  const QStringList statements = {
      "PRAGMA foreign_keys=ON",
      "CREATE TABLE IF NOT EXISTS employes(id INTEGER PRIMARY KEY "
      "AUTOINCREMENT,nom TEXT NOT NULL,prenom TEXT NOT NULL,courriel "
      "TEXT,telephone TEXT,poste TEXT,date_embauche TEXT,disponible INTEGER "
      "DEFAULT 1,charge_hebdo REAL DEFAULT 0,dispo_hebdo REAL DEFAULT "
      "35,competences TEXT)",
      "CREATE TABLE IF NOT EXISTS planning_affectations(id INTEGER PRIMARY KEY "
      "AUTOINCREMENT,titre TEXT NOT NULL,competence_requise TEXT NOT "
      "NULL,debut TEXT NOT NULL,fin TEXT NOT NULL,charge_heures REAL NOT NULL "
      "DEFAULT 1,responsable_id INTEGER NOT NULL,created_at TEXT DEFAULT "
      "CURRENT_TIMESTAMP,FOREIGN KEY(responsable_id) REFERENCES employes(id) "
      "ON DELETE CASCADE)",
      "CREATE INDEX IF NOT EXISTS idx_planning_responsable_dates ON "
      "planning_affectations(responsable_id,debut,fin)",
      "CREATE TRIGGER IF NOT EXISTS protect_employee_planning BEFORE DELETE ON "
      "employes WHEN EXISTS(SELECT 1 FROM planning_affectations WHERE "
      "responsable_id=OLD.id) BEGIN SELECT RAISE(ABORT,'Employe reference : "
      "historique du planning conserve'); END",
      "CREATE TABLE IF NOT EXISTS integration_migrations(name TEXT PRIMARY "
      "KEY)"};
  QSqlQuery q(db);
  for (const auto &sql : statements)
    if (!(q.prepare(sql) && q.exec())) {
      *error = q.lastError().text();
      return false;
    }
  return true;
}

bool Employe::importInitial(QSqlDatabase db, QString *error) {
  QSqlQuery check(db);
  if (!(check.prepare("SELECT 1 FROM integration_migrations WHERE "
                      "name='employes-aniworld2-v1'") &&
        check.exec())) {
    *error = check.lastError().text();
    return false;
  }
  if (check.next())
    return true;
  if (!(check.prepare("SELECT COUNT(*) FROM employes") && check.exec()) ||
      !check.next()) {
    *error = check.lastError().text();
    return false;
  }
  if (check.value(0).toInt() > 0) {
    *error = "L'import initial ne peut pas remplacer un annuaire déjà rempli.";
    return false;
  }
  QTemporaryDir temporary;
  const QString path = temporary.filePath("initial.sqlite");
  if (!temporary.isValid() ||
      !QFile::copy(":/data/employees-initial.sqlite", path)) {
    *error = "La base initiale des employés est introuvable.";
    return false;
  }
  bool ok = true;
  {
    QSqlDatabase source =
        QSqlDatabase::addDatabase("QSQLITE", "employees-import");
    source.setDatabaseName(path);
    source.setConnectOptions("QSQLITE_OPEN_READONLY");
    if (!source.open()) {
      *error = source.lastError().text();
      ok = false;
    }
    if (ok && !db.transaction()) {
      *error = db.lastError().text();
      ok = false;
    }
    if (ok) {
      for (const QString &table :
           {QString("employes"), QString("planning_affectations")}) {
        QSqlQuery read(source), write(db);
        if (!(read.prepare("SELECT * FROM " + table) && read.exec())) {
          *error = read.lastError().text();
          ok = false;
          break;
        }
        QStringList columns, marks;
        const auto record = read.record();
        for (int i = 0; i < record.count(); ++i) {
          columns << record.fieldName(i);
          marks << "?";
        }
        write.prepare("INSERT INTO " + table + "(" + columns.join(',') +
                      ") VALUES(" + marks.join(',') + ")");
        while (read.next()) {
          for (int i = 0; i < record.count(); ++i)
            write.bindValue(i, read.value(i));
          if (!write.exec()) {
            *error = write.lastError().text();
            ok = false;
            break;
          }
        }
        if (!ok)
          break;
      }
      QSqlQuery mark(db);
      if (ok && !(mark.prepare("INSERT INTO integration_migrations "
                               "VALUES('employes-aniworld2-v1')") &&
                  mark.exec())) {
        *error = mark.lastError().text();
        ok = false;
      }
      if (ok && !db.commit()) {
        *error = db.lastError().text();
        ok = false;
      }
      if (!ok)
        db.rollback();
    }
    source.close();
  }
  QSqlDatabase::removeDatabase("employees-import");
  return ok;
}
