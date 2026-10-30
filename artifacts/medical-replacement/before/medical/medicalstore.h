#pragma once
#include "medicalrecord.h"
#include <QSqlDatabase>
#include <QVector>
namespace MedicalStore {
bool initialize(QSqlDatabase db,QString *error);
QVector<MedicalRecord> all(QString *error=nullptr);
bool save(MedicalRecord &record,QString *error=nullptr);
bool remove(qint64 id,QString *error=nullptr);
QVector<MedicalRecord> examples();
bool seedExamples(QString *error=nullptr);
QString reportHtml(const QVector<MedicalRecord> &records,bool demo=false);
}
