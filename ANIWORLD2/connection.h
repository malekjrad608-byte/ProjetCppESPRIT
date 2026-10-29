#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

// Connexion Oracle unique pour toute l'application (Singleton Meyers).
// Toutes les QSqlQuery sans connexion explicite utilisent cette connexion Qt par defaut.
class Connection {
public:
    static Connection& createInstance();
    bool createConnection();
    QString lastError() const;
    bool isConnected();
    QString databaseName() const { return db.databaseName(); }
    QString userName() const { return db.userName(); }

private:
    QSqlDatabase db;
    QString m_error;

    Connection();
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    bool tableExiste(const QString &nom);
    bool colonneExiste(const QString &table, const QString &colonne);
    bool sequenceExiste(const QString &nom);
    bool idEstIdentity(const QString &table);
    bool triggerInsertionExiste(const QString &table);
    bool ajouterColonneSiAbsente(const QString &table, const QString &colonne,
                                 const QString &definition);
    bool assurerGenerateurId(const QString &table, const QString &sequence,
                             const QString &trigger);
    bool contrainteExiste(const QString &nom);
    void ajouterContrainte(const QString &table, const QString &nom,
                           const QString &definition);
    bool creerTables();
    bool preparerTableEmployes();
    bool preparerTablePlanning();
    void verifierContraintes();
};

#endif // CONNECTION_H
