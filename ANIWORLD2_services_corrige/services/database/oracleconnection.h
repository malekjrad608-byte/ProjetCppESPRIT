#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>

// Accessor for the shared Connection; owns no connection or credentials.
class OracleConnection {
public:
    static QSqlDatabase database();
    static bool isOracle();
    static qint64 insertedId(const QSqlQuery &query, const QString &table);
    static int nextId(const QString &sequence);
    static QString friendlyError(const QString &message);
};
