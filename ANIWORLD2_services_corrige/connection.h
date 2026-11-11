#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

// Connexion Oracle unique pour toute l'application (Singleton Meyers).
// Toutes les QSqlQuery sans connexion explicite utilisent cette connexion Qt par defaut.
//
// L'application NE MODIFIE JAMAIS la structure de la base (aucun CREATE / ALTER /
// DROP) : elle utilise la table EXISTANTE
//   EMPLOYER(ID_EMP, NOM, PRENOM, CIN, DATE_NAISS, ADRESSE, TEL, POSTE, COURRIEL,
//            COMPETENCES, DATE_EMBAUCHE, TYPE_CONTRAT, SALAIRE, CHARGE, DISPONIBLE,
//            DISPO_REPOS, ID_SERV)
// et verifie seulement (lecture seule) que ces 17 colonnes existent. Sinon le
// module Employes s'affiche en "mode degrade" avec la liste des colonnes absentes.
class Connection {
public:
    static Connection& createInstance();
    bool createConnection();
    QString lastError() const;

    // Etat du schema du module Employes. Une erreur ici n'empeche PAS la connexion
    // Oracle : seul le module Employes s'affiche en mode degrade.
    bool schemaEmployesOk() const { return m_erreurSchemaEmployes.isEmpty(); }
    QString erreurSchemaEmployes() const { return m_erreurSchemaEmployes; }

    // Vrai si Oracle genere deja ID_EMP (trigger + sequence, ou colonne IDENTITY).
    // Sinon l'application calcule elle-meme MAX(ID_EMP)+1 a l'insertion.
    bool idEmpAutomatique() const { return m_idAuto; }

    bool isConnected();
    bool isOpen() const { return db.isOpen(); }
    QString databaseName() const { return db.databaseName(); }
    QString userName() const { return db.userName(); }

private:
    QSqlDatabase db;
    QString m_error;
    QString m_erreurSchemaEmployes;   // vide = schema Employes conforme
    bool m_idAuto = false;

    Connection();
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    bool tableExiste(const QString &nom);
    QString typeColonne(const QString &table, const QString &colonne);   // "" si absente
    bool colonneEstIdentity(const QString &table, const QString &colonne);
    bool triggerAutoIncrementExiste(const QString &table, const QString &colonne);
    QString verifierSchemaEmployes();   // "" si conforme, sinon description des ecarts
};

#endif // CONNECTION_H
