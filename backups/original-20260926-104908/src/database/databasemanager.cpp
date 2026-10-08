#include "databasemanager.h"
#include <QSqlQuery>
#include <QVariant>
#include <QDate>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

bool DatabaseManager::connect(const QString &host, int port,
                               const QString &dbName,
                               const QString &user, const QString &password) {
    m_db = QSqlDatabase::addDatabase("QMYSQL");
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);
    if (!m_db.open()) {
        qWarning() << "DB Error:" << m_db.lastError().text();
        return false;
    }
    createTables();
    return true;
}

bool DatabaseManager::isConnected() const {
    return m_db.isOpen();
}

void DatabaseManager::createTables() {
    QSqlQuery q;
    q.exec(R"(CREATE TABLE IF NOT EXISTS employees (
        id        INT AUTO_INCREMENT PRIMARY KEY,
        firstName VARCHAR(100) NOT NULL,
        lastName  VARCHAR(100) NOT NULL,
        poste     VARCHAR(100),
        phone     VARCHAR(30),
        email     VARCHAR(150)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)");

    q.exec(R"(CREATE TABLE IF NOT EXISTS services (
        id          INT AUTO_INCREMENT PRIMARY KEY,
        name        VARCHAR(200) NOT NULL,
        description TEXT,
        employeeId  INT,
        dateTime    DATETIME,
        status      VARCHAR(50) DEFAULT 'Planifie',
        FOREIGN KEY (employeeId) REFERENCES employees(id) ON DELETE SET NULL
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)");

    q.exec(R"(CREATE TABLE IF NOT EXISTS schedules (
        id          INT AUTO_INCREMENT PRIMARY KEY,
        employeeId  INT NOT NULL,
        date        DATE NOT NULL,
        shift       VARCHAR(30) NOT NULL,
        note        TEXT,
        FOREIGN KEY (employeeId) REFERENCES employees(id) ON DELETE CASCADE
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4)");
}

// ─────────── SERVICES ───────────
QList<Service> DatabaseManager::getAllServices() {
    QList<Service> list;
    QSqlQuery q;
    q.exec(R"(SELECT s.id, s.name, s.description,
                     CONCAT(e.firstName,' ',e.lastName) AS responsible,
                     s.employeeId, s.dateTime, s.status
              FROM services s
              LEFT JOIN employees e ON s.employeeId = e.id
              ORDER BY s.dateTime DESC)");
    while (q.next()) {
        list.append(Service(
            q.value(0).toInt(),
            q.value(1).toString(),
            q.value(2).toString(),
            q.value(3).toString(),
            q.value(4).toInt(),
            q.value(5).toDateTime(),
            q.value(6).toString()
        ));
    }
    return list;
}

bool DatabaseManager::addService(const Service &s) {
    QSqlQuery q;
    q.prepare("INSERT INTO services (name,description,employeeId,dateTime,status) VALUES(?,?,?,?,?)");
    q.addBindValue(s.name);
    q.addBindValue(s.description);
    q.addBindValue(s.employeeId > 0 ? QVariant(s.employeeId) : QVariant(QMetaType(QMetaType::Int)));
    q.addBindValue(s.dateTime);
    q.addBindValue(s.status);
    return q.exec();
}

bool DatabaseManager::updateService(const Service &s) {
    QSqlQuery q;
    q.prepare("UPDATE services SET name=?,description=?,employeeId=?,dateTime=?,status=? WHERE id=?");
    q.addBindValue(s.name);
    q.addBindValue(s.description);
    q.addBindValue(s.employeeId > 0 ? QVariant(s.employeeId) : QVariant(QMetaType(QMetaType::Int)));
    q.addBindValue(s.dateTime);
    q.addBindValue(s.status);
    q.addBindValue(s.id);
    return q.exec();
}

bool DatabaseManager::deleteService(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM services WHERE id=?");
    q.addBindValue(id);
    return q.exec();
}

// ─────────── EMPLOYEES ───────────
QList<Employee> DatabaseManager::getAllEmployees() {
    QList<Employee> list;
    QSqlQuery q("SELECT id,firstName,lastName,poste,phone,email FROM employees ORDER BY lastName");
    while (q.next()) {
        list.append(Employee(
            q.value(0).toInt(),
            q.value(1).toString(),
            q.value(2).toString(),
            q.value(3).toString(),
            q.value(4).toString(),
            q.value(5).toString()
        ));
    }
    return list;
}

bool DatabaseManager::addEmployee(const Employee &e) {
    QSqlQuery q;
    q.prepare("INSERT INTO employees (firstName,lastName,poste,phone,email) VALUES(?,?,?,?,?)");
    q.addBindValue(e.firstName);
    q.addBindValue(e.lastName);
    q.addBindValue(e.poste);
    q.addBindValue(e.phone);
    q.addBindValue(e.email);
    return q.exec();
}

bool DatabaseManager::updateEmployee(const Employee &e) {
    QSqlQuery q;
    q.prepare("UPDATE employees SET firstName=?,lastName=?,poste=?,phone=?,email=? WHERE id=?");
    q.addBindValue(e.firstName);
    q.addBindValue(e.lastName);
    q.addBindValue(e.poste);
    q.addBindValue(e.phone);
    q.addBindValue(e.email);
    q.addBindValue(e.id);
    return q.exec();
}

bool DatabaseManager::deleteEmployee(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM employees WHERE id=?");
    q.addBindValue(id);
    return q.exec();
}

// ─────────── SCHEDULES ───────────
QList<Schedule> DatabaseManager::getSchedulesForWeek(const QDate &weekStart) {
    QList<Schedule> list;
    QSqlQuery q;
    q.prepare(R"(SELECT sc.id,
                        sc.employeeId,
                        CONCAT(e.firstName,' ',e.lastName),
                        sc.date,
                        sc.shift,
                        sc.note
                 FROM schedules sc
                 JOIN employees e ON sc.employeeId = e.id
                 WHERE sc.date BETWEEN ? AND ?
                 ORDER BY sc.date, e.lastName)");
    q.addBindValue(weekStart);
    q.addBindValue(weekStart.addDays(6));
    q.exec();
    while (q.next()) {
        list.append(Schedule(
            q.value(0).toInt(),
            q.value(1).toInt(),
            q.value(2).toString(),
            q.value(3).toDate(),
            q.value(4).toString(),
            q.value(5).toString()
        ));
    }
    return list;
}

QList<Schedule> DatabaseManager::getSchedulesForEmployee(int employeeId) {
    QList<Schedule> list;
    QSqlQuery q;
    q.prepare(R"(SELECT sc.id, sc.employeeId,
                        CONCAT(e.firstName,' ',e.lastName),
                        sc.date, sc.shift, sc.note
                 FROM schedules sc
                 JOIN employees e ON sc.employeeId=e.id
                 WHERE sc.employeeId=?
                 ORDER BY sc.date)");
    q.addBindValue(employeeId);
    q.exec();
    while (q.next()) {
        list.append(Schedule(
            q.value(0).toInt(), q.value(1).toInt(),
            q.value(2).toString(), q.value(3).toDate(),
            q.value(4).toString(), q.value(5).toString()
        ));
    }
    return list;
}

bool DatabaseManager::addSchedule(const Schedule &s) {
    QSqlQuery q;
    q.prepare("INSERT INTO schedules (employeeId,date,shift,note) VALUES(?,?,?,?)");
    q.addBindValue(s.employeeId);
    q.addBindValue(s.date);
    q.addBindValue(s.shift);
    q.addBindValue(s.note);
    return q.exec();
}

bool DatabaseManager::updateSchedule(const Schedule &s) {
    QSqlQuery q;
    q.prepare("UPDATE schedules SET employeeId=?,date=?,shift=?,note=? WHERE id=?");
    q.addBindValue(s.employeeId);
    q.addBindValue(s.date);
    q.addBindValue(s.shift);
    q.addBindValue(s.note);
    q.addBindValue(s.id);
    return q.exec();
}

bool DatabaseManager::deleteSchedule(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM schedules WHERE id=?");
    q.addBindValue(id);
    return q.exec();
}
