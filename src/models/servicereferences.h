#pragma once
#include <QList>
#include <QPair>
#include <QString>
// Lecture des propriétaires existants pour la référence Service ; aucun CRUD Médical.
class ServiceReferences {
public:
 static QList<QPair<qint64,QString>> proprietaires(QString *error);
 static QString proprietaire(qint64 id,QString *error);
};
