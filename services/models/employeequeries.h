#pragma once
#include "modelrows.h"
// Lecture des responsables pour le planning de Services uniquement.
class EmployeeQueries {
public: static ModelRows planningEmployees();
private: static ModelRows read(const QString &sql,const QVariantList &values={});
};
