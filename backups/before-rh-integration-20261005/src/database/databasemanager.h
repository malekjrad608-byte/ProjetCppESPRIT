#pragma once
#include <QSqlDatabase>
#include <QList>
#include "../models/service.h"

// Local service storage for the interface phase. No server or credentials required.
class DatabaseManager {
public:
    static DatabaseManager &instance();
    bool initialize(const QString &path);
    bool isConnected() const { return m_db.isOpen(); }
    QString lastError() const { return m_error; }
    QString storagePath() const { return m_db.databaseName(); }
    int lastInsertedId() const { return m_lastInsertedId; }
    QList<Service> getAllServices();
    bool addService(const Service &service);
    bool updateService(const Service &service);
    bool deleteService(int id);
    bool addExamples();
private:
    DatabaseManager() = default;
    bool save(const Service &service, bool update);
    QSqlDatabase m_db;
    QString m_error;
    int m_lastInsertedId = 0;
};
