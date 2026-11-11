#pragma once
#include <QSqlDatabase>
#include <QList>
#include "../models/service.h"

// Services facade: CRUD delegates to Service, connection belongs to the host.
class DatabaseManager {
public:
    static DatabaseManager &instance();
    QString lastError() const { return m_error; }
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
    QString m_error;
    int m_lastInsertedId = 0;
};
