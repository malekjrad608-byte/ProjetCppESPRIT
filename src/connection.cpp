#include "connection.h"
#include <QSqlError>
#include <QDebug>

// Ctor : initialise l'attribut `db` avec le driver ODBC (Singleton => une seule connexion partagee)
Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}

// Dtor : ferme la connexion si elle est encore ouverte
Connection::~Connection() {
    if (db.isOpen())
        db.close();
}

// Meyers Singleton : instance statique locale, creee au premier appel
Connection& Connection::createInstance() {
    static Connection instance;
    return instance;
}

bool Connection::createConnection() {
    if (db.isOpen())
        return true;

    db.setDatabaseName("Source_Projet2A18");   // nom de la source de donnees ODBC
    db.setUserName("rawand");                  // utilisateur commun de l'equipe
    db.setPassword("rawand123123");            // mot de passe

    if (db.open()) {
        qDebug() << "Connexion établie";
        m_error.clear();
        return true;
    }

    m_error = db.lastError().text();
    qDebug() << "Échec de la connexion :" << m_error;
    return false;
}
