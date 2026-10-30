#pragma once
#include <QString>
#include <QSqlDatabase>

namespace Accounts {
enum class Role { None, Admin, Services, Stock, Employees, Locaux };
bool initialize(QSqlDatabase db, QString *error);
Role authenticate(const QString &username, const QString &password);
bool changePassword(const QString &username, const QString &current, const QString &replacement, QString *error);
bool allows(Role role, int page);
int homePage(Role role);
QString label(Role role);
}
