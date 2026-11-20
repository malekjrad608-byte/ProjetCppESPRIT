#include "employeeevaluation.h"
#include "employe.h"
#include "../models/modulesupplements.h"
#include <QJsonObject>
#include <QDateTime>

QJsonObject EmployeeEvaluation::fiche(int employee,QString *error) {
    for(const auto &value:ModuleSupplements::read("employee-evaluation-drafts",error)) {
        const auto row=value.toObject();
        if(row["employee"].toInt()==employee)return row["fiche"].toObject();
    }
    if(error && !error->isEmpty())return {};
    auto old=latest(employee);
    if(!old.isOk()){if(error)*error=old.lastError().text();return {};}
    if(!old.next())return {};
    QJsonArray notes;for(int i=2;i<8;++i)notes.append(old.value(i).toInt());
    return {{"dateEvaluation",old.value(0).toString()},{"evaluateur",old.value(1).toString()},
            {"notes",notes},{"pointsForts",old.value(8).toString()},
            {"axesAmelioration",old.value(9).toString()},{"commentaires",old.value(10).toString()},
            {"objectifs",old.value(11).toString()}};
}

bool EmployeeEvaluation::enregistrerFiche(int employee,const QJsonObject &fiche,QString *error) {
    Employe employe;
    const auto notes=fiche["notes"].toArray();
    if(!Employe::charger(employee,employe) || notes.size()!=6 ||
       !QDate::fromString(fiche["dateEvaluation"].toString(),Qt::ISODate).isValid()) {
        if (error) {
            *error = "Employé, date ou notes invalides.";
        }
        return false;
    }
    for(const auto &note:notes)if(!note.isDouble() || note.toDouble()!=note.toInt() || note.toInt()<0 || note.toInt()>5) {
        if (error) {
            *error = "Les notes doivent être comprises entre 0 et 5 (0 = non noté).";
        }
        return false;
    }
    return ModuleSupplements::update("employee-evaluation-drafts",[&](QJsonArray &rows){
        const QJsonObject saved{{"employee",employee},{"fiche",fiche}};
        for(int i=0;i<rows.size();++i)if(rows[i].toObject()["employee"].toInt()==employee){rows[i]=saved;return true;}
        rows.append(saved);return true;
    },error);
}

ModelRows EmployeeEvaluation::latest(int employee) {
    ModelRows result;QString error;
    const auto all=ModuleSupplements::read("employee-evaluations",&error);
    result.error.message=error;
    QString date;QVariantList latest;
    for(const auto &v:all){auto o=v.toObject();if(o["employee"].toInt()!=employee)continue;
        auto values=o["values"].toArray().toVariantList();
        if(values.value(0).toString()>=date){date=values.value(0).toString();latest=values;}
    }
    if(!latest.isEmpty())result.rows.append(latest);
    return result;
}
bool EmployeeEvaluation::ajouter(int employee,const QVariantList &values,QString *error) {
    Employe e;
    if(!Employe::charger(employee,e) || values.size()!=12 || !QDate::fromString(values[0].toString(),Qt::ISODate).isValid()) {
        if (error) {
            *error = "Employé ou date invalide.";
        }
        return false;
    }
    for(int i=2;i<8;++i)if(values[i].toInt()<1 || values[i].toInt()>5){if(error)*error="Les six notes doivent être comprises entre 1 et 5.";return false;}
    return ModuleSupplements::update("employee-evaluations",[&](QJsonArray &rows){
        rows.append(QJsonObject{{"employee",employee},{"values",QJsonArray::fromVariantList(values)},
                               {"saved",QDateTime::currentDateTimeUtc().toString(Qt::ISODate)}});return true;
    },error);
}
