#pragma once
#include <QJsonArray>
#include <QString>
#include <functional>

// Donnees complementaires des modules importes, sans modifier le schema Oracle.
// Fichiers distincts par application/base ; ecriture atomique et verrou interprocessus.
class ModuleSupplements {
public:
    static QString path(const QString &module);
    static QJsonArray read(const QString &module, QString *error = nullptr);
    static bool update(const QString &module,
                       const std::function<bool(QJsonArray &)> &change,
                       QString *error = nullptr);
};
