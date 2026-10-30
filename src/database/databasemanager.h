#pragma once
#include <QSqlDatabase>
#include <QList>
#include "../models/service.h"

// Application facade. Oracle is owned by the unchanged teacher Connection Singleton.
class DatabaseManager {
public:
    static DatabaseManager &instance();
    bool initialize(const QString &path);
    bool initializeOracle(const QString &configPath = {});
    bool isConnected() const { return m_db.isOpen(); }
    QString lastError() const { return m_error; }
    QString storagePath() const { return m_localPath; }
    bool isOracle() const { return m_db.driverName()=="QODBC"; }
    int lastInsertedId() const { return m_lastInsertedId; }
    QList<Service> getAllServices();
    bool addService(const Service &service);
    bool updateService(const Service &service);
    bool deleteService(int id);
    bool addExamples();
private:
    DatabaseManager() = default;
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;
    bool save(const Service &service, bool update);
    QSqlDatabase m_db;
    QString m_error;
    QString m_localPath;
    int m_lastInsertedId = 0;
};
