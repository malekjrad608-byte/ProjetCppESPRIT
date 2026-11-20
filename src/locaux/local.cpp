#include "local.h"
#include "../models/relationmodel.h"
#include "../models/modulesupplements.h"
#include "../database/oracleconnection.h"
#include <QCryptographicHash>
#include <QMap>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>

bool Local::afficher(QList<Local> *items, qint64 *revision, QString *error) {
  Rows rows;
  if (!readRows(&rows, revision, error))
    return false;
  items->clear();
  for (const auto &r : rows) {
    Local l;
    l.id = r[0].toInt();
    l.name = r[1].toString();
    l.type = r[2].toString();
    l.floor = r[3].toInt();
    l.capacity = r[4].toInt();
    l.occupancy = r[5].toInt();
    l.specialty = r[6].toString();
    l.outOfService = r[7].toInt() != 0;
    l.cleaning = CleaningState(r[8].toInt());
    l.lastCleaned = QDateTime::fromString(r[9].toString(), Qt::ISODate);
    l.dirtySince = QDateTime::fromString(r[10].toString(), Qt::ISODate);
    l.serviceId = r[11].toInt();
    if(OracleConnection::isOracle())for(const auto &v:ModuleSupplements::read("locals")) {
      const auto o=v.toObject();
      if(o["id"].toInt()==l.id && o["name"].toString()==l.name && o["oracleState"].toString()==l.stateText()) {
        l.occupancy=qBound(0,o["occupancy"].toInt(),l.capacity);
        l.cleaning=CleaningState(qBound(0,o["cleaning"].toInt(),2));
        l.lastCleaned=QDateTime::fromString(o["lastCleaned"].toString(),Qt::ISODate);
        l.dirtySince=QDateTime::fromString(o["dirtySince"].toString(),Qt::ISODate);break;
      }
    }
    items->append(l);
  }
  return true;
}
bool Local::enregistrer(const QList<Local> &items, qint64 *revision,
                        QString *error) {
  Rows rows;
  QSet<QString> names;
  for (const auto &l : items) {
    if (l.name.trimmed().isEmpty() || l.name.size() > (OracleConnection::isOracle()?150:120) || l.capacity < 1 ||
        l.occupancy < 0 || l.occupancy > l.capacity || l.serviceId <= 0 ||
        int(l.cleaning) < 0 || int(l.cleaning) > 2 ||
        names.contains(l.name.trimmed().toCaseFolded())) {
      *error = "Vérifiez le nom unique, la capacité, l’occupation, le "
               "nettoyage et le service obligatoire.";
      return false;
    }
    names.insert(l.name.trimmed().toCaseFolded());
    rows.append({l.id, l.name, l.type, l.floor, l.capacity, l.occupancy,
                 l.specialty, int(l.outOfService), int(l.cleaning),
                 l.lastCleaned.toString(Qt::ISODate),
                 l.dirtySince.toString(Qt::ISODate), l.serviceId});
  }
  if(!saveRows(rows,revision,error))return false;
  if(OracleConnection::isOracle()) {
    QString issue;
    if(!ModuleSupplements::update("locals",[&](QJsonArray &saved){
      saved=QJsonArray();for(const auto &l:items) {
        auto o=l.toJson();o["oracleState"]=l.outOfService?"Hors service":l.occupancy>0?"Occupé":"Disponible";
        saved.append(o);
      }return true;
    },&issue)){*error="Oracle enregistré ; complément local non sauvegardé : "+issue;return false;}
  }
  return true;
}

RoomState Local::state() const {
  if (outOfService)
    return RoomState::OutOfService;
  if(occupancy>0)return RoomState::Occupied;
  return cleaning==CleaningState::Clean?RoomState::Available:RoomState::ToClean;
}

QString Local::stateText() const {
  switch (state()) {
  case RoomState::Available:
    return QStringLiteral("Disponible");
  case RoomState::Occupied:
    return QStringLiteral("Occupé");
  case RoomState::OutOfService:
    return QStringLiteral("Hors service");
  case RoomState::ToClean:return QStringLiteral("À nettoyer");
  }
  return QString();
}

QString Local::cleaningText() const {
  switch (cleaning) {
  case CleaningState::Clean:
    return QStringLiteral("Propre");
  case CleaningState::ToClean:
    return QStringLiteral("À nettoyer");
  case CleaningState::InProgress:
    return QStringLiteral("En cours");
  }
  return QString();
}

QString Local::floorText() const {
  return floor == 0 ? QStringLiteral("Rez-de-chaussée")
                    : QStringLiteral("Étage %1").arg(floor);
}

QString Local::floorShort() const {
  return floor == 0 ? QStringLiteral("RDC")
                    : QStringLiteral("Étage %1").arg(floor);
}

QString Local::iconName() const {
  if (type == "Consultation")
    return QStringLiteral("consult");
  if (type == "Chirurgie")
    return QStringLiteral("surgery");
  if (type == "Hospitalisation")
    return QStringLiteral("bed");
  if (type == "Pension")
    return QStringLiteral("home");
  if (type == "Laboratoire")
    return QStringLiteral("flask");
  if (type == "Imagerie")
    return QStringLiteral("scan");
  if (type == "Isolement")
    return QStringLiteral("isolation");
  return QStringLiteral("rooms");
}

bool Local::isUrgentType() const {
  return type == "Chirurgie" || type == "Isolement";
}

void Local::markToClean() {
  cleaning = CleaningState::ToClean;
  dirtySince = QDateTime::currentDateTime();
}

void Local::startCleaning() { cleaning = CleaningState::InProgress; }

void Local::markClean() {
  cleaning = CleaningState::Clean;
  lastCleaned = QDateTime::currentDateTime();
  dirtySince = QDateTime();
}

qint64 Local::dirtyMinutes() const {
  if (cleaning != CleaningState::ToClean)
    return 0;
  const QDateTime since = dirtySince.isValid() ? dirtySince : lastCleaned;
  if (!since.isValid())
    return 0;
  return qMax<qint64>(0, since.secsTo(QDateTime::currentDateTime()) / 60);
}

QStringList Local::types() {
  return {"Consultation", "Chirurgie", "Hospitalisation", "Pension",
          "Laboratoire",  "Imagerie",  "Isolement"};
}

QJsonObject Local::toJson() const {
  QJsonObject o;
  o["id"] = id;
  o["name"] = name;
  o["type"] = type;
  o["floor"] = floor;
  o["capacity"] = capacity;
  o["occupancy"] = occupancy;
  o["specialty"] = specialty;
  o["outOfService"] = outOfService;
  o["cleaning"] = static_cast<int>(cleaning);
  o["lastCleaned"] = lastCleaned.toString(Qt::ISODate);
  if (dirtySince.isValid())
    o["dirtySince"] = dirtySince.toString(Qt::ISODate);
  return o;
}

Local Local::fromJson(const QJsonObject &o) {
  Local l;
  l.id = o["id"].toInt();
  l.name = o["name"].toString();
  l.type = o["type"].toString();
  l.floor = o["floor"].toInt();
  l.capacity = o["capacity"].toInt(1);
  l.occupancy = o["occupancy"].toInt();
  l.specialty = o["specialty"].toString();
  l.outOfService = o["outOfService"].toBool();
  l.cleaning = static_cast<CleaningState>(o["cleaning"].toInt());
  l.lastCleaned =
      QDateTime::fromString(o["lastCleaned"].toString(), Qt::ISODate);
  if (!l.lastCleaned.isValid())
    l.lastCleaned = QDateTime::currentDateTime();
  l.dirtySince = QDateTime::fromString(o["dirtySince"].toString(), Qt::ISODate);
  return l;
}

bool Local::loadRows(Rows *rows, QString *error) {
  QSqlQuery q(OracleConnection::database());
  const auto fields =
      QStringList{"id",       "name",         "type",        "floor",
                  "capacity", "occupancy",    "specialty",   "out_of_service",
                  "cleaning", "last_cleaned", "dirty_since", "id_service"};
  if (!(q.prepare("SELECT " + fields.join(',') + " FROM " +
                  QStringLiteral("locaux") + " ORDER BY " + fields[0]) &&
        q.exec())) {
    *error = q.lastError().text();
    return false;
  }
  rows->clear();
  while (q.next()) {
    QVariantList row;
    for (int i = 0; i < fields.size(); ++i)
      row << q.value(i);
    *rows << row;
  }
  return true;
}
qint64 Local::rowsVersion(const Rows &rows) {
  QMap<QString, QVariantList> ordered;
  for (const auto &row : rows)
    ordered.insert(row[0].toString(), row);
  QByteArray bytes;
  for (const auto &row : ordered)
    for (const auto &value : row) {
      const auto s = value.toString().toUtf8();
      bytes += QByteArray::number(s.size()) + ':' + s;
    }
  const auto hash = QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
  quint64 result = 0;
  for (int i = 0; i < 8; ++i)
    result = (result << 8) | quint8(hash[i]);
  return qint64((result & 0x7fffffffffffffffULL) | 1);
}
bool Local::readRows(Rows *rows, qint64 *revision, QString *error) {
  if (!loadRows(rows, error))
    return false;
  *revision = rowsVersion(*rows);
  error->clear();
  return true;
}
bool Local::saveRows(const Rows &rows, qint64 *revision, QString *error) {
  const auto fields =
      QStringList{"id",       "name",         "type",        "floor",
                  "capacity", "occupancy",    "specialty",   "out_of_service",
                  "cleaning", "last_cleaned", "dirty_since", "id_service"};
  QSet<QString> keys;
  for (const auto &row : rows) {
    if (row.size() != fields.size() || row[0].toString().trimmed().isEmpty() ||
        keys.contains(row[0].toString())) {
      *error = "Identifiant vide ou en double.";
      return false;
    }
    if (row.last().toInt() <= 0) {
      *error = "Choisissez le service de chaque local.";
      return false;
    }
    keys.insert(row[0].toString());
  }
  auto db = OracleConnection::database();
  if (!db.transaction()) {
    *error = db.lastError().text();
    return false;
  }
  auto fail = [&](QString message) {
    db.rollback();
    *error = message.section(QChar(0), 0, 0);
    return false;
  };
  QSqlQuery q(db);
  if (!(q.prepare("LOCK TABLE " + QStringLiteral("local") +
                  " IN EXCLUSIVE MODE WAIT 5") &&
        q.exec()))
    return fail(q.lastError().text());
  Rows beforeRows;
  if (!loadRows(&beforeRows, error))
    return fail(*error);
  if (rowsVersion(beforeRows) != *revision)
    return fail("Ces données ont été modifiées par une autre session. "
                "Actualisez avant de réessayer.");
  QMap<QString, QVariantList> old;
  for (const auto &row : beforeRows)
    old.insert(row[0].toString(), row);
  for (auto it = old.cbegin(); it != old.cend(); ++it)
    if (!keys.contains(it.key())) {
      q.prepare("DELETE FROM " + QStringLiteral("local") + " WHERE " +
                QStringLiteral("id_local") + "=?");
      q.addBindValue(it.value()[0]);
      if (!q.exec())
        return fail(q.lastError().text().contains("02292")
                        ? "Ce local contient encore du stock. Réaffectez ses "
                          "lots avant suppression."
                        : q.lastError().text());
    }
  for (const auto &row : rows) {
    const auto before = old.value(row[0].toString());
    bool changed = before.isEmpty();
    for (int i = 0; !changed && i < fields.size(); ++i)
      changed = before[i].toString() != row[i].toString();
    if (!changed)
      continue;
    const QStringList writeFields={"id_local","nom_local","type","etage","capacite","specialite","etat","id_serv"};
    const QVariantList values={row[0],row[1],row[2],row[3],row[4],row[6],
       row[7].toBool()?QString("Hors service"):row[5].toInt()>0?QString("Occupé"):QString("Disponible"),row[11]};
    if (before.isEmpty()) {
      QStringList marks;
      for (int i = 0; i < writeFields.size(); ++i)
        marks << "?";
      q.prepare("INSERT INTO " + QStringLiteral("local") + "(" +
                writeFields.join(',') + ") VALUES(" + marks.join(',') + ")");
      for (const auto &v : values)
        q.addBindValue(v);
    } else {
      QStringList assignments;
      for (int i = 1; i < writeFields.size(); ++i)
        assignments << writeFields[i] + "=?";
      q.prepare("UPDATE " + QStringLiteral("local") + " SET " +
                assignments.join(',') + " WHERE " + QStringLiteral("id_local") + "=?");
      for (int i = 1; i < values.size(); ++i)
        q.addBindValue(values[i]);
      q.addBindValue(row[0]);
    }
    if (!q.exec())
      return fail(q.lastError().text());
  }
  Rows saved;
  if (!loadRows(&saved, error))
    return fail(*error);
  if (!db.commit())
    return fail(db.lastError().text());
  *revision = rowsVersion(saved);
  error->clear();
  return true;
}

const QString Local::Disponible="Disponible";
const QString Local::Occupe="Occupé";
const QString Local::ANettoyer="À nettoyer";
const QString Local::HorsService="Hors service";
QString Local::controleSaisie() const {
 if(name.trimmed().isEmpty() || name.size()>150)return "Renseignez un nom (150 caractères maximum).";
 if(capacity<1 || capacity>10000 || occupancy>capacity || occupancy<0)return "Vérifiez la capacité et l'occupation.";
 if(serviceId<=0)return "Choisissez le service associé.";
 return {};
}
QList<QPair<int,QString>> Local::services() {
 QList<QPair<int,QString>> out;auto rows=RelationModel::afficher(RelationModel::Entity::Services);
 while(rows.next())out.append({rows.value(0).toInt(),rows.value(1).toString()});return out;
}
