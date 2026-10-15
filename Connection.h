#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>

/* Classe Connection
 * Singleton gérant une connexion unique à la base de données (QODBC).
 * Accès : Connection::createInstance().createConnection();
 */
class Connection {
public:
    static Connection& createInstance();   // Retourne l'instance unique
    bool createConnection();               // Configure et ouvre la base
    bool isOpen() const { return db.isOpen(); }

private:
    QSqlDatabase db;

    Connection();
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
};

#endif // CONNECTION_H
