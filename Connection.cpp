#include "Connection.h"

// Ctor : enregistre le driver ODBC une seule fois (stocké dans le Singleton).
// QSqlDatabase::addDatabase() sans nom => connexion par défaut de l'application,
// donc QSqlQuery l'utilise automatiquement partout.
Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}

// Dtor : ferme la connexion si elle est encore ouverte
Connection::~Connection() {
    if (db.isOpen())
        db.close();
}

// Meyers Singleton (thread-safe en C++11+)
Connection& Connection::createInstance() {
    static Connection instance;
    return instance;
}

bool Connection::createConnection() {
    if (db.isOpen())
        return true;                          // déjà connecté

    db.setDatabaseName("Source_Projet2A18");  // nom de la source de données ODBC (DSN)
    db.setUserName("rawand");
    db.setPassword("rawand123123");

    if (db.open()) {
        qDebug() << "Connexion établie";
        return true;
    }
    qDebug() << "Échec de la connexion :" << db.lastError().text();
    return false;
}
