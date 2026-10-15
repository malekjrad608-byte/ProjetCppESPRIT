#include "connection.h"

#include <QDebug>
#include <QSqlError>

// ====== A MODIFIER SELON VOTRE PC ======
static const char *kDsn      = "Source_Projet2A";  // nom de la source de donnees ODBC
static const char *kUser     = "SYSTEM";         // utilisateur Oracle
static const char *kPassword = "1234";         // mot de passe de l'utilisateur
// =======================================

Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection()
{
    if (db.isOpen())
        db.close();
}

Connection &Connection::createInstance()
{
    static Connection instance;
    return instance;
}

bool Connection::createConnection()
{
    if (db.isOpen())
        return true;
    db.setDatabaseName(kDsn);
    db.setUserName(kUser);
    db.setPassword(kPassword);

    if (db.open()) {
        qDebug() << "Connexion etablie";
        m_error.clear();
        return true;
    }
    m_error = db.lastError().text();
    qDebug() << "Echec de la connexion :" << m_error;
    return false;
}
