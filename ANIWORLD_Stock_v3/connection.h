#ifndef CONNECTION_H
#define CONNECTION_H
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>

class Connection {
public:
    static Connection& createInstance();
    bool createConnection();
private:
    QSqlDatabase db;
    Connection();
    ~Connection();
    // Interdire la copie w l'affectation
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
};
#endif // CONNECTION_H