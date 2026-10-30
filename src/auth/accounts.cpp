#include "accounts.h"
#include "../database/oracleconnection.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

namespace {
constexpr int iterations = 100000;
QString localAccountsPath() {
  return QStandardPaths::writableLocation(
             QStandardPaths::AppLocalDataLocation) +
         "/accounts-local.json";
}
QJsonArray localAccounts() {
  QFile f(localAccountsPath());
  if (!f.open(QIODevice::ReadOnly))
    return {};
  return QJsonDocument::fromJson(f.readAll()).array();
}
QByteArray salt() {
  QByteArray value(24, Qt::Uninitialized);
  for (auto &c : value)
    c = char(QRandomGenerator::system()->generate() & 0xff);
  return value;
}
QByteArray digest(const QString &password, const QByteArray &salt, int count) {
  return QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256,
                                            password.toUtf8(), salt, count, 32);
}
Accounts::Role parse(const QString &role) {
  if (role == "admin")
    return Accounts::Role::Admin;
  if (role == "services")
    return Accounts::Role::Services;
  if (role == "stock")
    return Accounts::Role::Stock;
  if (role == "medical")
    return Accounts::Role::Medical;
  if (role == "locaux")
    return Accounts::Role::Locaux;
  if (role == "employes")
    return Accounts::Role::Employees;
  return Accounts::Role::None;
}
} // namespace

bool Accounts::initialize(QSqlDatabase db, QString *error) {
  QSqlQuery q(db);
  auto fail = [&] {
    if (error)
      *error = q.lastError().text();
    q.finish();
    db.rollback();
    return false;
  };
  if (!db.transaction()) {
    if (error)
      *error = db.lastError().text();
    return false;
  }
  if (!(q.prepare("SELECT sql FROM sqlite_master WHERE type='table' AND "
                  "name='accounts'") &&
        q.exec()))
    return fail();
  const bool migrate = q.next() && !q.value(0).toString().contains("'medical'");
  q.finish();
  if (migrate &&
      !(q.prepare("ALTER TABLE accounts RENAME TO accounts_before_medical") &&
        q.exec()))
    return fail();
  if (!(q.prepare("CREATE TABLE IF NOT EXISTS accounts(username TEXT PRIMARY "
                  "KEY COLLATE NOCASE, role TEXT NOT NULL CHECK(role IN "
                  "('admin','services','stock','employes','locaux','medical')),"
                  " salt BLOB NOT NULL, password_hash BLOB NOT NULL, "
                  "iterations INTEGER NOT NULL)") &&
        q.exec()))
    return fail();
  if (migrate) {
    if (!(q.prepare(
              "INSERT INTO accounts SELECT * FROM accounts_before_medical") &&
          q.exec()))
      return fail();
    if (!(q.prepare("DROP TABLE accounts_before_medical") && q.exec()))
      return fail();
  }
  for (const QString &name :
       {QString("admin"), QString("services"), QString("stock"),
        QString("employes"), QString("locaux"), QString("medical")}) {
    q.prepare("SELECT 1 FROM accounts WHERE username=?");
    q.addBindValue(name);
    if (!q.exec()) {
      *error = q.lastError().text();
      db.rollback();
      return false;
    }
    if (q.next())
      continue;
    const QByteArray nonce = salt();
    q.prepare("INSERT INTO accounts VALUES(?,?,?,?,?)");
    q.addBindValue(name);
    q.addBindValue(name);
    q.addBindValue(nonce);
    q.addBindValue(digest(name + "123", nonce, iterations));
    q.addBindValue(iterations);
    if (!q.exec()) {
      *error = q.lastError().text();
      db.rollback();
      return false;
    }
  }
  if (!db.commit()) {
    *error = db.lastError().text();
    return false;
  }
  return true;
}
Accounts::Role Accounts::authenticate(const QString &username,
                                      const QString &password) {
  if (OracleConnection::isOracle()) {
    for (const auto &value : localAccounts()) {
      const auto a = value.toObject();
      if (a["username"].toString().compare(username.trimmed(),
                                           Qt::CaseInsensitive) != 0)
        continue;
      const int count = a["iterations"].toInt();
      if (count < 10000 || count > 1000000)
        return Role::None;
      const auto expected =
          QByteArray::fromHex(a["password_hash"].toString().toLatin1());
      const auto actual =
          digest(password, QByteArray::fromHex(a["salt"].toString().toLatin1()),
                 count);
      if (expected.size() != actual.size())
        return Role::None;
      unsigned diff = 0;
      for (int i = 0; i < actual.size(); ++i)
        diff |= quint8(actual[i]) ^ quint8(expected[i]);
      return diff == 0 ? parse(a["role"].toString()) : Role::None;
    }
    return Role::None;
  }
  QSqlQuery q(OracleConnection::database());
  const bool oracle = OracleConnection::isOracle();
  q.prepare(
      oracle ? "SELECT role,RAWTOHEX(salt),RAWTOHEX(password_hash),iterations "
               "FROM accounts WHERE LOWER(username)=LOWER(?)"
             : "SELECT role,salt,password_hash,iterations FROM accounts WHERE "
               "LOWER(username)=LOWER(?)");
  q.addBindValue(username.trimmed());
  if (!q.exec() || !q.next())
    return Role::None;
  const int count = q.value(3).toInt();
  if (count < 10000 || count > 1000000)
    return Role::None;
  const QByteArray expected =
      oracle ? QByteArray::fromHex(q.value(2).toByteArray())
             : q.value(2).toByteArray();
  const QByteArray actual =
      digest(password,
             oracle ? QByteArray::fromHex(q.value(1).toByteArray())
                    : q.value(1).toByteArray(),
             count);
  if (expected.size() != actual.size())
    return Role::None;
  unsigned difference = 0;
  for (int i = 0; i < actual.size(); ++i)
    difference |= quint8(actual[i]) ^ quint8(expected[i]);
  return difference == 0 ? parse(q.value(0).toString()) : Role::None;
}
bool Accounts::changePassword(const QString &username, const QString &current,
                              const QString &replacement, QString *error) {
  if (authenticate(username, current) == Role::None) {
    *error = "Le mot de passe actuel est incorrect.";
    return false;
  }
  if (replacement.size() < 8 || replacement.size() > 128) {
    *error = "Utilisez entre 8 et 128 caractères.";
    return false;
  }
  const QByteArray nonce = salt();
  QSqlQuery q(OracleConnection::database());
  const bool oracle = OracleConnection::isOracle();
  if (oracle) {
    auto accounts = localAccounts();
    bool found = false;
    for (int i = 0; i < accounts.size(); ++i) {
      auto a = accounts[i].toObject();
      if (a["username"].toString().compare(username.trimmed(),
                                           Qt::CaseInsensitive) != 0)
        continue;
      a["salt"] = QString::fromLatin1(nonce.toHex());
      a["password_hash"] =
          QString::fromLatin1(digest(replacement, nonce, iterations).toHex());
      a["iterations"] = iterations;
      accounts[i] = a;
      found = true;
      break;
    }
    QSaveFile file(localAccountsPath());
    const auto data = QJsonDocument(accounts).toJson();
    if (!found || !file.open(QIODevice::WriteOnly) ||
        file.write(data) != data.size() || !file.commit()) {
      *error = "Impossible d’enregistrer le mot de passe local.";
      return false;
    }
    error->clear();
    return true;
  }
  q.prepare(oracle ? "UPDATE accounts SET "
                     "salt=HEXTORAW(?),password_hash=HEXTORAW(?),iterations=? "
                     "WHERE LOWER(username)=LOWER(?)"
                   : "UPDATE accounts SET salt=?,password_hash=?,iterations=? "
                     "WHERE LOWER(username)=LOWER(?)");
  const auto hash = digest(replacement, nonce, iterations);
  q.addBindValue(oracle ? QVariant(QString::fromLatin1(nonce.toHex()))
                        : QVariant(nonce));
  q.addBindValue(oracle ? QVariant(QString::fromLatin1(hash.toHex()))
                        : QVariant(hash));
  q.addBindValue(iterations);
  q.addBindValue(username);
  if (!q.exec() || q.numRowsAffected() != 1) {
    *error = "Le changement n'a pas pu être enregistré.";
    return false;
  }
  return true;
}
bool Accounts::allows(Role role, int page) {
  if (role == Role::Admin)
    return page >= 0 && page <= 5;
  return (role == Role::Services && (page == 0 || page == 1)) ||
         (role == Role::Stock && page == 2) ||
         (role == Role::Employees && page == 3) ||
         (role == Role::Locaux && page == 4) ||
         (role == Role::Medical && page == 5);
}
int Accounts::homePage(Role role) {
  return role == Role::Medical     ? 5
         : role == Role::Locaux    ? 4
         : role == Role::Stock     ? 2
         : role == Role::Employees ? 3
                                   : 1;
}
QString Accounts::label(Role role) {
  return role == Role::Medical     ? "Dossiers médicaux"
         : role == Role::Locaux    ? "Locaux"
         : role == Role::Admin     ? "Administration"
         : role == Role::Services  ? "Services"
         : role == Role::Stock     ? "Stock"
         : role == Role::Employees ? "Employés"
                                   : "Déconnecté";
}
