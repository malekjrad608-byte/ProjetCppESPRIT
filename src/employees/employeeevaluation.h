#pragma once
#include "../models/modelrows.h"
#include <QDate>
#include <QJsonObject>
class EmployeeEvaluation {
public:
    static ModelRows latest(int employee);
    // date, evaluateur, six notes, points forts, axes, commentaires, objectifs
    static bool ajouter(int employee,const QVariantList &values,QString *error);
    static QJsonObject fiche(int employee,QString *error);
    static bool enregistrerFiche(int employee,const QJsonObject &fiche,QString *error);
};
