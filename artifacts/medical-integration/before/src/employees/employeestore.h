#pragma once
#include <QSqlDatabase>
#include <QString>
namespace EmployeeStore {
bool initialize(QSqlDatabase db,QString *error);
bool importInitial(QSqlDatabase db,QString *error);
}
