#include "connection.h"

Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection() {
    if (db.isOpen())
        db.close();
}

Connection& Connection::createInstance() {
    static Connection instance; // Instance statique (Meyers Singleton)
    return instance;
}

bool Connection::createConnection() {
    // Configuration de la connexion ODBC pour Oracle 21c (Utilisateur c##rawand)
    db.setDatabaseName("Source_Projet2A"); // Esm l'ODBC elli 3maltou
    db.setUserName("c##rawand");
    db.setPassword("rawand123123");

    if (db.open()) {
        qDebug() << "Connexion établie avec succès (c##rawand)";
        return true;
    }
    qDebug() << "Échec de la connexion :" << db.lastError().text();
    return false;
}