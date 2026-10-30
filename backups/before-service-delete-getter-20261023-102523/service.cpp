#include "service.h"
#include "../database/oracleconnection.h"
#include "../employees/employe.h"
#include <QScopedPointer>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <cmath>
namespace {
QVariant nullableId(int id) {
  return id > 0 ? QVariant(id) : QVariant(QMetaType(QMetaType::Int));
}
} // namespace
// Initialisation des attributs prives par les setters declares dans service.h.
Service::Service(int id, const QString &name, const QString &clientName,
                 const QString &category, const QDateTime &dateTime,
                 const QString &status, int duration, double price) {
  setId(id);
  setName(name);
  setClientName(clientName);
  setCategory(category);
  setDateTime(dateTime);
  setStatus(status);
  setDuration(duration);
  setPrice(price);
}

bool Service::ajouter() {
  int inserted = 0;
  // INSERT prepare/bindValue/exec dans enregistrer(), puis commit en mode
  // Oracle.
  if (!enregistrer(false, &inserted, &m_error))
    return false;
  setId(inserted);
  return true;
}

QSqlQueryModel *Service::afficher(QObject *parent,
                                  const QList<int> *orderedIds) const {
  auto *model = new QSqlQueryModel(parent);
  QSqlQuery query(OracleConnection::database());
  QString sql = OracleConnection::isOracle()
                    ? "SELECT "
                      "id,name,description,responsible,category,dateTime,"
                      "status,duration,price,clientName,"
                      "invoiceNumber,invoiceDate,0,id_employe,0 FROM services"
                    : "SELECT "
                      "id,name,description,responsible,category,dateTime,"
                      "status,duration,price,clientName,"
                      "invoiceNumber,invoiceDate,id_local,id_employe,id_client "
                      "FROM services";
  // La vue transmet uniquement les identifiants filtres, jamais du SQL.
  if (orderedIds) {
    if (orderedIds->isEmpty())
      sql += " WHERE 1=0";
    else {
      // Oracle accepte au plus 1000 valeurs dans chaque expression IN.
      QStringList groups;
      for (int first = 0; first < orderedIds->size(); first += 900) {
        QStringList placeholders;
        for (int i = first; i < qMin(first + 900, int(orderedIds->size())); ++i)
          placeholders << "?";
        groups << "id IN (" + placeholders.join(',') + ")";
      }
      sql += " WHERE (" + groups.join(" OR ") + ") ORDER BY CASE id";
      for (int i = 0; i < orderedIds->size(); ++i)
        sql += " WHEN ? THEN " + QString::number(i);
      sql += " END";
    }
  } else
    sql += " ORDER BY dateTime DESC,id DESC";
  query.prepare(sql);
  if (orderedIds) {
    for (int value : *orderedIds)
      query.addBindValue(value);
    for (int value : *orderedIds)
      query.addBindValue(value);
  }
  query.exec();
  model->setQuery(std::move(query));
  const QStringList titles = {"ID",       "Service",   "Description",
                              "Référent", "Catégorie", "Date / heure",
                              "Statut",   "Durée",     "Tarif",
                              "Client",   "Facture",   "Date facture",
                              "Local",    "Employé",   "Client enregistré"};
  for (int i = 0; i < titles.size(); ++i)
    model->setHeaderData(i, Qt::Horizontal, titles[i]);
  return model;
}

bool Service::supprimer(int id) {
  QSqlQuery query(OracleConnection::database());
  query.prepare("DELETE FROM services WHERE id=:id");
  query.bindValue(":id", id);
  if (!query.exec()) {
    m_error = OracleConnection::isOracle()
                  ? OracleConnection::friendlyError(query.lastError().text())
                  : query.lastError().text();
    return false;
  }
  if (query.numRowsAffected() != 1) {
    m_error = "Ce service n'existe plus.";
    return false;
  }
  m_error.clear();
  return true;
}

bool Service::modifier(QString *error) const {
  int inserted = 0;
  return enregistrer(true, &inserted, error);
}

QList<Service> Service::afficherListe(QString *error) {
  QList<Service> result;
  QScopedPointer<QSqlQueryModel> model(Service().afficher());
  if (model->lastError().isValid()) {
    *error = model->lastError().text();
    return result;
  }
  while (model->canFetchMore())
    model->fetchMore();
  error->clear();
  for (int row = 0; row < model->rowCount(); ++row) {
    const auto query = model->record(row);
    Service s;
    s.setId(query.value(0).toInt());
    s.setName(query.value(1).toString());
    s.setDescription(query.value(2).toString());
    s.setResponsible(query.value(3).toString());
    s.setCategory(query.value(4).toString());
    s.setDateTime(
        QDateTime::fromString(query.value(5).toString(), Qt::ISODate));
    s.setStatus(query.value(6).toString());
    s.setDuration(query.value(7).toInt());
    s.setPrice(query.value(8).toDouble());
    s.setClientName(query.value(9).toString());
    s.setInvoiceNumber(query.value(10).toString());
    s.setInvoiceDate(
        QDateTime::fromString(query.value(11).toString(), Qt::ISODate));
    s.setLocalId(query.value(12).toInt());
    s.setEmployeeId(query.value(13).toInt());
    s.setClientId(query.value(14).toInt());
    result.append(s);
  }
  return result;
}
bool Service::enregistrer(bool update, int *insertedId, QString *error) const {
  const Service &s = *this;
  if (s.getName().trimmed().isEmpty() || s.getName().trimmed().size() > 120 ||
      !s.getDateTime().isValid()) {
    *error = "Vérifiez le nom et la date du service.";
    return false;
  }
  if (s.getClientName().size() > 120 || s.getDescription().size() > 5000 ||
      !std::isfinite(s.getPrice()) || s.getPrice() < 0 ||
      s.getPrice() > 99999.99) {
    *error = "Vérifiez le client, le détail et le tarif de la prestation.";
    return false;
  }
  if (s.getDuration() < 5 || s.getDuration() > 1440 ||
      !QStringList{"Planifié", "En cours", "Terminé", "Annulé"}.contains(
          s.getStatus()) ||
      s.getCategory().trimmed().isEmpty()) {
    *error = "Vérifiez la catégorie, le statut et la durée (5 à 1440 minutes).";
    return false;
  }
  if (s.getStatus() != "Annulé" &&
      !Service::available(s.getEmployeeId(), s.getDateTime(), Service::end(s),
                          update ? s.getId() : 0, error))
    return false;
  if (OracleConnection::isOracle())
    return enregistrerOracle(update, insertedId, error);
  QSqlQuery query(OracleConnection::database());
  query.prepare(
      update
          ? "UPDATE services SET "
            "name=:name,description=:description,responsible=:responsible,"
            "category=:category,dateTime=:"
            "datetime,status=:status,duration=:duration,price=:price,"
            "clientName=:clientName,id_local=:"
            "id_local,id_employe=:id_employe,id_client=:id_client WHERE id=:id"
          : "INSERT INTO "
            "services(name,description,responsible,category,dateTime,status,"
            "duration,price,clientName,"
            "id_local,id_employe,id_client) "
            "VALUES(:name,:description,:responsible,:category,:datetime,:"
            "status,:duration,:price,:"
            "clientName,:id_local,:id_employe,:id_client)");
  query.bindValue(":name", s.getName().trimmed());
  query.bindValue(":description", s.getDescription().isNull()
                                      ? QStringLiteral("")
                                      : s.getDescription().trimmed());
  query.bindValue(":responsible", s.getResponsible().isNull()
                                      ? QStringLiteral("")
                                      : s.getResponsible().trimmed());
  query.bindValue(":category", s.getCategory());
  query.bindValue(":datetime", s.getDateTime().toString(Qt::ISODate));
  query.bindValue(":status", s.getStatus());
  query.bindValue(":duration", s.getDuration());
  query.bindValue(":price", s.getPrice());
  query.bindValue(":clientName", s.getClientName().isNull()
                                     ? QStringLiteral("")
                                     : s.getClientName().trimmed());
  query.bindValue(":id_local", nullableId(s.getLocalId()));
  query.bindValue(":id_employe", nullableId(s.getEmployeeId()));
  query.bindValue(":id_client", nullableId(s.getClientId()));
  if (update)
    query.bindValue(":id", s.getId());
  if (!query.exec()) {
    const auto code = query.lastError().nativeErrorCode();
    *error = code.contains("20041")
                 ? QStringLiteral("Cette facture a déjà été émise ou annulée. "
                                  "Ses informations sont "
                                  "conservées ; créez une nouvelle prestation "
                                  "pour corriger la facturation.")
                 : query.lastError().text();
    if (code.contains("20051"))
      *error = "Affectation refusée : cet employé a déjà un service ou une "
               "tâche sur ce créneau. "
               "Actualisez le planning et choisissez un autre horaire ou un "
               "autre employé.";
    return false;
  }
  if (update && query.numRowsAffected() != 1) {
    *error = "Ce service n'existe plus.";
    return false;
  }
  if (!update)
    *insertedId = int(OracleConnection::insertedId(query, "services"));
  error->clear();
  return true;
}

bool Service::ajouter(QString *error) {
  const bool test = ajouter();
  *error = m_error;
  return test;
}

bool Service::supprimer(int id, QString *error) {
  Service service;
  const bool test = service.supprimer(id);
  *error = service.lastError();
  return test;
}

bool Service::enregistrerOracle(bool update, int *insertedId,
                                QString *error) const {
  const Service &s = *this;
  auto db = OracleConnection::database();
  const bool createEmployee = s.getEmployeeId() == -1;
  if (s.getEmployeeId() <= 0 && !createEmployee) {
    *error = "Choisissez un employé ou créez le premier employé du service.";
    return false;
  }
  if (createEmployee && (s.getNewEmployeeNom().trimmed().isEmpty() ||
                         s.getNewEmployeePrenom().trimmed().isEmpty())) {
    *error = "Nom et prénom du nouvel employé obligatoires.";
    return false;
  }
  if (!db.transaction()) {
    *error = db.lastError().text();
    return false;
  }
  auto fail = [&](QString message) {
    db.rollback();
    *error = OracleConnection::friendlyError(message);
    return false;
  };
  const int id = update ? s.getId() : OracleConnection::nextId("services_seq");
  const int employee = createEmployee ? OracleConnection::nextId("employes_seq")
                                      : s.getEmployeeId();
  if (id <= 0 || employee <= 0)
    return fail("Impossible de générer les identifiants.");
  QSqlQuery q(db);
  q.prepare(update ? "UPDATE services SET "
                     "name=:name,description=:description,responsible=:"
                     "responsible,category=:category,"
                     "datetime=:datetime,status=:status,duration=:duration,"
                     "price=:price,clientName=:"
                     "clientName,id_employe=:id_employe WHERE id=:id"
                   : "INSERT INTO "
                     "services(name,description,responsible,category,datetime,"
                     "status,duration,price,"
                     "clientName,id_employe,id) "
                     "VALUES(:name,:description,:responsible,:category,:"
                     "datetime,:status,:duration,:price,:"
                     "clientName,:id_employe,:id)");
  q.bindValue(":name", s.getName().trimmed());
  q.bindValue(":description", s.getDescription());
  q.bindValue(":responsible", createEmployee ? s.getNewEmployeeNom() + " " +
                                                   s.getNewEmployeePrenom()
                                             : s.getResponsible());
  q.bindValue(":category", s.getCategory());
  q.bindValue(":datetime", s.getDateTime().toString(Qt::ISODate));
  q.bindValue(":status", s.getStatus());
  q.bindValue(":duration", s.getDuration());
  q.bindValue(":price", s.getPrice());
  q.bindValue(":clientName", s.getClientName());
  q.bindValue(":id_employe", employee);
  q.bindValue(":id", id);
  if (!q.exec())
    return fail(q.lastError().text());
  if (q.numRowsAffected() != 1)
    return fail("Service introuvable.");
  if (createEmployee) {
    q.prepare("INSERT INTO employes(id,nom,prenom,id_service,date_embauche) "
              "VALUES(?,?,?,?,?)");
    q.addBindValue(employee);
    q.addBindValue(s.getNewEmployeeNom().trimmed());
    q.addBindValue(s.getNewEmployeePrenom().trimmed());
    q.addBindValue(id);
    q.addBindValue(QDate::currentDate().toString(Qt::ISODate));
    if (!q.exec())
      return fail(q.lastError().text());
  } else if (!Employe::moveEmployee(employee, id, error))
    return fail(*error);
  if (!db.commit())
    return fail(db.lastError().text());
  *insertedId = id;
  error->clear();
  return true;
}

bool Service::modifier() { return modifier(&m_error); }

QList<Service::Appointment> Service::appointments(QString *error) {
  error->clear();
  QList<Appointment> result;
  QSqlQuery q(OracleConnection::database());
  if (!(q.prepare(OracleConnection::isOracle()
                      ? "SELECT "
                        "s.id,e.id,s.name,s.clientName,s.status,s.dateTime,s."
                        "duration FROM services s JOIN employes e ON "
                        "e.id_service=s.id"
                      : "SELECT "
                        "id,id_employe,name,clientName,status,dateTime,"
                        "duration FROM services") &&
        q.exec())) {
    *error = "Lecture du planning impossible : " + q.lastError().text();
    return {};
  }
  while (q.next()) {
    Appointment a;
    a.serviceId = q.value(0).toInt();
    a.employeeId = q.value(1).toInt();
    a.title = QString("SRV-%1 · ").arg(a.serviceId, 3, 10, QChar('0')) +
              q.value(2).toString();
    a.client = q.value(3).toString();
    a.status = q.value(4).toString();
    a.start = QDateTime::fromString(q.value(5).toString(), Qt::ISODate);
    a.finish = a.start.addSecs(q.value(6).toInt() * 60);
    if (!a.start.isValid() || a.finish <= a.start) {
      *error = "Un service possède un créneau invalide. Corrigez sa date et sa "
               "durée.";
      return {};
    }
    result << a;
  }
  if (!OracleConnection::isOracle()) {
    if (!(q.prepare("SELECT id,responsable_id,titre,debut,fin FROM "
                    "planning_affectations") &&
          q.exec())) {
      *error = "Lecture des tâches impossible : " + q.lastError().text();
      return {};
    }
    while (q.next()) {
      Appointment a;
      a.employeeId = q.value(1).toInt();
      a.title = QString("Tâche #%1 · ").arg(q.value(0).toInt()) +
                q.value(2).toString();
      a.status = "Tâche";
      a.start =
          QDateTime::fromString(q.value(3).toString(), "yyyy-MM-dd HH:mm:ss");
      a.finish =
          QDateTime::fromString(q.value(4).toString(), "yyyy-MM-dd HH:mm:ss");
      if (!a.start.isValid() || !a.finish.isValid() || a.finish <= a.start) {
        *error = "Une tâche possède un créneau invalide. Corrigez son planning "
                 "dans Employés.";
        return {};
      }
      result << a;
    }
  }
  std::sort(result.begin(), result.end(),
            [](const Appointment &a, const Appointment &b) {
              return a.start == b.start ? a.title < b.title : a.start < b.start;
            });
  return result;
}
bool Service::available(int employee, const QDateTime &start,
                        const QDateTime &finish, int excludedService,
                        QString *error) {
  error->clear();
  if (employee <= 0)
    return true;
  if (!start.isValid() || !finish.isValid() || finish <= start) {
    *error = "Créneau invalide.";
    return false;
  }
  const auto entries = appointments(error);
  if (!error->isEmpty())
    return false;
  for (const auto &a : entries) {
    if (a.employeeId != employee || a.status == "Annulé" ||
        (excludedService > 0 && a.serviceId == excludedService))
      continue;
    if (start < a.finish && a.start < finish) {
      *error =
          QString(
              "Affectation impossible : cet employé est déjà occupé par %1, du "
              "%2 au %3. Choisissez un autre horaire ou un autre employé.")
              .arg(a.title, a.start.toString("dd/MM/yyyy HH:mm"),
                   a.finish.toString("dd/MM/yyyy HH:mm"));
      return false;
    }
  }
  return true;
}
QDate Service::weekStart(const QDate &date) {
  return date.addDays(1 - date.dayOfWeek());
}
QDateTime Service::end(const Service &service) {
  return service.getDateTime().addSecs(qint64(service.getDuration()) * 60);
}
QSet<int> Service::conflictingServices(const QList<Service> &services) {
  QSet<int> result;
  for (int i = 0; i < services.size(); ++i) {
    const auto &a = services[i];
    if (a.getEmployeeId() <= 0 || a.getStatus() == "Annulé" ||
        !a.getDateTime().isValid() || a.getDuration() <= 0)
      continue;
    for (int j = i + 1; j < services.size(); ++j) {
      const auto &b = services[j];
      if (b.getEmployeeId() != a.getEmployeeId() || b.getStatus() == "Annulé" ||
          !b.getDateTime().isValid() || b.getDuration() <= 0)
        continue;
      if (a.getDateTime() < end(b) && b.getDateTime() < end(a)) {
        result.insert(a.getId());
        result.insert(b.getId());
      }
    }
  }
  return result;
}
QList<Service> Service::inWeek(const QList<Service> &services,
                               const QDate &date, int employee,
                               bool cancelled) {
  const auto monday = weekStart(date);
  const QDateTime start(monday, QTime(0, 0)),
      finish(monday.addDays(7), QTime(0, 0));
  QList<Service> result;
  for (const auto &s : services) {
    if (!s.getDateTime().isValid() || s.getDateTime() >= finish ||
        end(s) <= start)
      continue;
    if (!cancelled && s.getStatus() == "Annulé")
      continue;
    if (employee > 0 && s.getEmployeeId() != employee)
      continue;
    if (employee == -1 && s.getEmployeeId() > 0)
      continue;
    result << s;
  }
  std::sort(result.begin(), result.end(),
            [](const Service &a, const Service &b) {
              return a.getDateTime() == b.getDateTime()
                         ? a.getId() < b.getId()
                         : a.getDateTime() < b.getDateTime();
            });
  return result;
}

QString Service::invoiceStatus(int id) {
  QSqlQuery q(OracleConnection::database());
  q.prepare("SELECT invoice_status FROM services WHERE id=?");
  q.addBindValue(id);
  return q.exec() && q.next() ? q.value(0).toString() : QString();
}
