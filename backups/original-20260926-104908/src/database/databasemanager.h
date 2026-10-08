#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QList>
#include "../models/service.h"
#include "../models/employee.h"
#include "../models/schedule.h"

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool connect(const QString &host, int port,
                 const QString &dbName,
                 const QString &user, const QString &password);
    bool isConnected() const;
    void createTables();

    // ── Services ──
    QList<Service>  getAllServices();
    bool            addService(const Service &s);
    bool            updateService(const Service &s);
    bool            deleteService(int id);

    // ── Employees ──
    QList<Employee> getAllEmployees();
    bool            addEmployee(const Employee &e);
    bool            updateEmployee(const Employee &e);
    bool            deleteEmployee(int id);

    // ── Schedules ──
    QList<Schedule> getSchedulesForWeek(const QDate &weekStart);
    QList<Schedule> getSchedulesForEmployee(int employeeId);
    bool            addSchedule(const Schedule &s);
    bool            updateSchedule(const Schedule &s);
    bool            deleteSchedule(int id);

private:
    DatabaseManager() = default;
    QSqlDatabase m_db;
};
