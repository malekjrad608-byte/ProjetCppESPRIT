#include "connection.h"
#include <QCoreApplication>
#include <QSqlQuery>
#include <QTextStream>
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    auto &connection = Connection::createInstance();
    out << "Same Singleton instance: " << (&connection == &Connection::createInstance()) << Qt::endl;
    if (!connection.createConnection()) return 1;
    auto db = QSqlDatabase::database();
    out << "Driver: " << db.driverName() << "; DSN: " << db.databaseName() << Qt::endl;
    QSqlQuery q(db);
    q.prepare("SELECT USER, SYS_CONTEXT('USERENV','CON_NAME'), (SELECT COUNT(*) FROM user_tables) FROM dual");
    if (!q.exec() || !q.next()) { out << q.lastError().text() << Qt::endl; return 2; }
    out << "User: " << q.value(0).toString() << "; Container: " << q.value(1).toString() << "; Tables: " << q.value(2).toInt() << Qt::endl;
    return 0;
}
