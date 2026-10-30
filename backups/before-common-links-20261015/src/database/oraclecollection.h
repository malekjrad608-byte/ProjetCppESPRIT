#pragma once
#include <QString>
#include <QVector>
#include <QVariant>
namespace OracleCollection {
using Rows = QVector<QVariantList>;
bool read(const QString &name,Rows *rows,qint64 *revision,QString *error);
bool save(const QString &name,const Rows &rows,qint64 *revision,QString *error);
}
