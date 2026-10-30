#pragma once
#include <QSqlDatabase>
#include <QString>

class Accounts {
public:
  enum class Role { None, Admin, Services, Stock, Employees, Locaux, Medical };
  static bool initialize(QSqlDatabase db, QString *error);
  static Role authenticate(const QString &username, const QString &password);
  static bool changePassword(const QString &username, const QString &current,
                             const QString &replacement, QString *error);
  static bool allows(Role role, int page);
  static int homePage(Role role);
  static QString label(Role role);
};
