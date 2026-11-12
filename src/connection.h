#pragma once
#include <QSqlDatabase>
#include <QString>

// Connexion Oracle (ODBC) - patron Singleton (Meyers) comme dans l'atelier.
class Connection
{
public:
    static Connection &createInstance();
    bool createConnection();                 // ouvre la connexion (true si OK)
    bool isOpen() const { return db.isOpen(); }
    QSqlDatabase database() const { return db; }
    QString lastError() const { return m_error; }

private:
    Connection();
    ~Connection();
    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;

    QSqlDatabase db;
    QString m_error;
};
