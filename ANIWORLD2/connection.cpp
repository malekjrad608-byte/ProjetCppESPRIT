#include "connection.h"

#include <QStringList>
#include <QtGlobal>

// ======================================================================
// PARAMETRES ORACLE
// DSN ODBC 64 bits : Source_Projet2A -> service Oracle localhost:1521/XEPDB1
// Modifiez ces trois valeurs si votre configuration locale est differente.
// ======================================================================
static const char *ODBC_DSN = "Source_Projet2A18";
static const char *DB_USER  = "rawand";
static const char *DB_PASS  = "rawand123123";

Connection::Connection()
{
    // Une seule connexion par defaut pour tout le projet Qt SQL.
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection()
{
    if (db.isOpen())
        db.close();
}

Connection& Connection::createInstance()
{
    static Connection instance;
    return instance;
}

QString Connection::lastError() const
{
    return m_error;
}

bool Connection::isConnected()
{
    if (!db.isOpen())
        return false;

    QSqlQuery q(db);
    if (q.exec("SELECT 1 FROM DUAL") && q.next()) {
        q.finish();
        return true;
    }

    m_error = q.lastError().text();
    q.finish();
    db.close();
    return false;
}

bool Connection::tableExiste(const QString &nom)
{
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) FROM USER_TABLES WHERE TABLE_NAME = :nom");
    q.bindValue(":nom", nom.toUpper());
    bool existe = false;
    if (q.exec() && q.next())
        existe = q.value(0).toInt() > 0;
    else
        m_error = q.lastError().text();
    q.finish();
    return existe;
}

bool Connection::colonneExiste(const QString &table, const QString &colonne)
{
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) FROM USER_TAB_COLUMNS "
              "WHERE TABLE_NAME = :table AND COLUMN_NAME = :colonne");
    q.bindValue(":table", table.toUpper());
    q.bindValue(":colonne", colonne.toUpper());
    bool existe = false;
    if (q.exec() && q.next())
        existe = q.value(0).toInt() > 0;
    else
        m_error = q.lastError().text();
    q.finish();
    return existe;
}

bool Connection::sequenceExiste(const QString &nom)
{
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) FROM USER_SEQUENCES WHERE SEQUENCE_NAME = :nom");
    q.bindValue(":nom", nom.toUpper());
    bool existe = false;
    if (q.exec() && q.next())
        existe = q.value(0).toInt() > 0;
    else
        m_error = q.lastError().text();
    q.finish();
    return existe;
}

bool Connection::idEstIdentity(const QString &table)
{
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) FROM USER_TAB_IDENTITY_COLS "
              "WHERE TABLE_NAME = :table AND COLUMN_NAME = 'ID'");
    q.bindValue(":table", table.toUpper());
    bool identity = false;
    if (q.exec() && q.next())
        identity = q.value(0).toInt() > 0;
    q.finish();
    // Sur Oracle moderne USER_TAB_IDENTITY_COLS existe. En cas d'erreur,
    // on continue avec le mecanisme sequence/trigger classique.
    return identity;
}

bool Connection::triggerInsertionExiste(const QString &table)
{
    // Detection du trigger qui genere vraiment :NEW.ID, pas seulement d'un
    // trigger INSERT de journalisation ou d'une autre regle metier. USER_SOURCE
    // fournit le corps ligne par ligne sans dependance au type LONG de USER_TRIGGERS.
    QSqlQuery q(db);
    q.prepare("SELECT T.TRIGGER_NAME, S.TEXT FROM USER_TRIGGERS T "
              "JOIN USER_SOURCE S ON S.NAME = T.TRIGGER_NAME AND S.TYPE = 'TRIGGER' "
              "WHERE T.TABLE_NAME = :table AND T.STATUS = 'ENABLED' "
              "AND INSTR(UPPER(T.TRIGGERING_EVENT), 'INSERT') > 0 "
              "AND T.TRIGGER_TYPE LIKE '%EACH ROW%' ORDER BY T.TRIGGER_NAME, S.LINE");
    q.bindValue(":table", table.toUpper());
    bool existe = false;
    QString nomCourant;
    QString corpsCourant;
    if (q.exec()) {
        while (q.next()) {
            const QString nom = q.value(0).toString();
            if (!nomCourant.isEmpty() && nom != nomCourant) {
                const QString corps = corpsCourant.toUpper();
                if (corps.contains(":NEW.ID") && corps.contains("NEXTVAL")) {
                    existe = true;
                    break;
                }
                corpsCourant.clear();
            }
            nomCourant = nom;
            corpsCourant += q.value(1).toString() + QLatin1Char(' ');
        }
        if (!existe) {
            const QString corps = corpsCourant.toUpper();
            existe = corps.contains(":NEW.ID") && corps.contains("NEXTVAL");
        }
    } else {
        m_error = q.lastError().text();
    }
    q.finish();
    return existe;
}

bool Connection::ajouterColonneSiAbsente(const QString &table, const QString &colonne,
                                         const QString &definition)
{
    if (colonneExiste(table, colonne))
        return true;

    QSqlQuery q(db);
    const QString sql = "ALTER TABLE " + table + " ADD (" + colonne + " " + definition + ")";
    if (!q.exec(sql)) {
        m_error = "Ajout de la colonne " + table + "." + colonne + " impossible : "
                  + q.lastError().text();
        q.finish();
        return false;
    }
    q.finish();
    return true;
}

bool Connection::assurerGenerateurId(const QString &table, const QString &sequence,
                                     const QString &trigger)
{
    if (!colonneExiste(table, "ID")) {
        m_error = "La table " + table + " existe, mais la colonne ID est absente. "
                  "Verifiez sa structure dans Oracle SQL Developer.";
        return false;
    }

    // Une colonne IDENTITY Oracle genere deja ses propres identifiants.
    if (idEstIdentity(table))
        return true;

    // Si un trigger d'insertion est deja actif, on ne cree pas un deuxieme
    // trigger susceptible d'ecraser l'ID genere par le trigger historique.
    if (triggerInsertionExiste(table))
        return true;

    if (!sequenceExiste(sequence)) {
        // Une sequence manquante doit commencer apres le plus grand ID deja
        // present pour ne pas entrer en conflit avec les donnees existantes.
        QSqlQuery maxQuery(db);
        if (!maxQuery.exec("SELECT NVL(MAX(ID), 0) + 1 FROM " + table) || !maxQuery.next()) {
            m_error = "Impossible de calculer le prochain ID de " + table + " : "
                      + maxQuery.lastError().text();
            maxQuery.finish();
            return false;
        }
        const qulonglong prochainId = qMax<qulonglong>(1, maxQuery.value(0).toULongLong());
        maxQuery.finish();

        QSqlQuery createSequence(db);
        const QString sqlSequence = "CREATE SEQUENCE " + sequence + " START WITH "
                                    + QString::number(prochainId)
                                    + " INCREMENT BY 1 NOCACHE";
        if (!createSequence.exec(sqlSequence)) {
            m_error = "Creation de la sequence " + sequence + " impossible : "
                      + createSequence.lastError().text();
            createSequence.finish();
            return false;
        }
        createSequence.finish();
    }

    QSqlQuery createTrigger(db);
    const QString sqlTrigger =
        "CREATE OR REPLACE TRIGGER " + trigger + " "
        "BEFORE INSERT ON " + table + " FOR EACH ROW "
        "WHEN (NEW.ID IS NULL) "
        "BEGIN SELECT " + sequence + ".NEXTVAL INTO :NEW.ID FROM DUAL; END;";
    if (!createTrigger.exec(sqlTrigger)) {
        m_error = "Creation du trigger " + trigger + " impossible : "
                  + createTrigger.lastError().text();
        createTrigger.finish();
        return false;
    }
    createTrigger.finish();
    return true;
}

bool Connection::preparerTableEmployes()
{
    if (!tableExiste("EMPLOYES")) {
        QSqlQuery q(db);
        const QString sql =
            "CREATE TABLE EMPLOYES ("
            " ID NUMBER PRIMARY KEY,"
            " NOM VARCHAR2(100) NOT NULL,"
            " PRENOM VARCHAR2(100) NOT NULL,"
            " COURRIEL VARCHAR2(150),"
            " TELEPHONE VARCHAR2(30),"
            " POSTE VARCHAR2(100),"
            " DATE_EMBAUCHE DATE,"
            " DISPONIBLE NUMBER(1) DEFAULT 1 NOT NULL,"
            " CHARGE_HEBDO NUMBER DEFAULT 0 NOT NULL,"
            " DISPO_HEBDO NUMBER DEFAULT 35 NOT NULL,"
            " COMPETENCES VARCHAR2(300),"
            " CONSTRAINT CK_EMPLOYES_DISPO CHECK (DISPONIBLE IN (0,1)),"
            " CONSTRAINT CK_EMPLOYES_CHARGE CHECK (CHARGE_HEBDO >= 0),"
            " CONSTRAINT CK_EMPLOYES_DISPO_H CHECK (DISPO_HEBDO >= 0 AND DISPO_HEBDO <= 60),"
            " CONSTRAINT UQ_EMPLOYES_COURRIEL UNIQUE (COURRIEL))";
        if (!q.exec(sql)) {
            m_error = "Creation de la table EMPLOYES impossible : " + q.lastError().text();
            q.finish();
            return false;
        }
        q.finish();
    }

    // Mise a niveau non destructive des tables deja existantes. Si NOM/PRENOM
    // manquent, on s'arrete plutot que de creer des colonnes qui masqueraient
    // une structure Oracle differente de celle attendue par l'application.
    if (!colonneExiste("EMPLOYES", "NOM") || !colonneExiste("EMPLOYES", "PRENOM")) {
        m_error = "La table EMPLOYES existe, mais les colonnes NOM et/ou PRENOM sont absentes. "
                  "Verifiez la structure de la table dans Oracle SQL Developer.";
        return false;
    }

    if (!ajouterColonneSiAbsente("EMPLOYES", "COURRIEL", "VARCHAR2(150)") ||
        !ajouterColonneSiAbsente("EMPLOYES", "TELEPHONE", "VARCHAR2(30)") ||
        !ajouterColonneSiAbsente("EMPLOYES", "POSTE", "VARCHAR2(100)") ||
        !ajouterColonneSiAbsente("EMPLOYES", "DATE_EMBAUCHE", "DATE") ||
        !ajouterColonneSiAbsente("EMPLOYES", "DISPONIBLE", "NUMBER(1) DEFAULT 1 NOT NULL") ||
        !ajouterColonneSiAbsente("EMPLOYES", "CHARGE_HEBDO", "NUMBER DEFAULT 0 NOT NULL") ||
        !ajouterColonneSiAbsente("EMPLOYES", "DISPO_HEBDO", "NUMBER DEFAULT 35 NOT NULL") ||
        !ajouterColonneSiAbsente("EMPLOYES", "COMPETENCES", "VARCHAR2(300)"))
        return false;

    if (!assurerGenerateurId("EMPLOYES", "EMPLOYES_SEQ", "ANIWORLD_EMPLOYES_ID_BI"))
        return false;

    return true;
}

bool Connection::preparerTablePlanning()
{
    if (!tableExiste("PLANNING_AFFECTATIONS")) {
        QSqlQuery q(db);
        const QString sql =
            "CREATE TABLE PLANNING_AFFECTATIONS ("
            " ID NUMBER PRIMARY KEY,"
            " TITRE VARCHAR2(200) NOT NULL,"
            " COMPETENCE_REQUISE VARCHAR2(200),"
            " DEBUT VARCHAR2(19) NOT NULL,"
            " FIN VARCHAR2(19) NOT NULL,"
            " CHARGE_HEURES NUMBER DEFAULT 1 NOT NULL,"
            " RESPONSABLE_ID NUMBER NOT NULL,"
            " CREATED_AT TIMESTAMP DEFAULT SYSTIMESTAMP,"
            " CONSTRAINT CK_PLANNING_CHARGE CHECK (CHARGE_HEURES > 0 AND CHARGE_HEURES <= 24),"
            " CONSTRAINT CK_PLANNING_DATES CHECK (FIN > DEBUT),"
            " CONSTRAINT FK_PLANNING_EMPLOYE FOREIGN KEY (RESPONSABLE_ID) "
            " REFERENCES EMPLOYES(ID) ON DELETE CASCADE)";
        if (!q.exec(sql)) {
            m_error = "Creation de la table PLANNING_AFFECTATIONS impossible : "
                      + q.lastError().text();
            q.finish();
            return false;
        }
        q.finish();
    }

    const QStringList obligatoires = {"ID", "TITRE", "DEBUT", "FIN", "RESPONSABLE_ID"};
    for (const QString &colonne : obligatoires) {
        if (!colonneExiste("PLANNING_AFFECTATIONS", colonne)) {
            m_error = "La table PLANNING_AFFECTATIONS ne contient pas la colonne " + colonne + ".";
            return false;
        }
    }

    if (!ajouterColonneSiAbsente("PLANNING_AFFECTATIONS", "COMPETENCE_REQUISE", "VARCHAR2(200)") ||
        !ajouterColonneSiAbsente("PLANNING_AFFECTATIONS", "CHARGE_HEURES", "NUMBER DEFAULT 1 NOT NULL") ||
        !ajouterColonneSiAbsente("PLANNING_AFFECTATIONS", "CREATED_AT", "TIMESTAMP DEFAULT SYSTIMESTAMP"))
        return false;

    if (!assurerGenerateurId("PLANNING_AFFECTATIONS", "PLANNING_SEQ", "ANIWORLD_PLANNING_ID_BI"))
        return false;

    return true;
}

bool Connection::creerTables()
{
    if (!preparerTableEmployes())
        return false;
    if (!preparerTablePlanning())
        return false;
    return true;
}

bool Connection::contrainteExiste(const QString &nom)
{
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) FROM USER_CONSTRAINTS WHERE CONSTRAINT_NAME = :nom");
    q.bindValue(":nom", nom.toUpper());
    bool existe = false;
    if (q.exec() && q.next())
        existe = q.value(0).toInt() > 0;
    q.finish();
    return existe;
}

// Ajoute une contrainte aux tables deja existantes sans revalider les anciennes
// lignes ; Oracle controle les nouvelles operations apres l'activation.
void Connection::ajouterContrainte(const QString &table, const QString &nom,
                                   const QString &definition)
{
    if (contrainteExiste(nom))
        return;

    QSqlQuery q(db);
    const QString sql = "ALTER TABLE " + table + " ADD CONSTRAINT " + nom + " "
                        + definition + " ENABLE NOVALIDATE";
    if (!q.exec(sql))
        qDebug() << "Contrainte ignoree" << nom << ":" << q.lastError().text();
    else
        qDebug() << "Contrainte ajoutee :" << nom;
    q.finish();
}

void Connection::verifierContraintes()
{
    ajouterContrainte("EMPLOYES", "CK_EMPLOYES_DISPO", "CHECK (DISPONIBLE IN (0,1))");
    ajouterContrainte("EMPLOYES", "CK_EMPLOYES_CHARGE", "CHECK (CHARGE_HEBDO >= 0)");
    ajouterContrainte("EMPLOYES", "CK_EMPLOYES_DISPO_H",
                      "CHECK (DISPO_HEBDO >= 0 AND DISPO_HEBDO <= 60)");
    ajouterContrainte("PLANNING_AFFECTATIONS", "CK_PLANNING_CHARGE",
                      "CHECK (CHARGE_HEURES > 0 AND CHARGE_HEURES <= 24)");
    ajouterContrainte("PLANNING_AFFECTATIONS", "CK_PLANNING_DATES", "CHECK (FIN > DEBUT)");
}

bool Connection::createConnection()
{
    if (db.isOpen())
        db.close();

    db.setDatabaseName(ODBC_DSN);
    db.setUserName(DB_USER);
    db.setPassword(DB_PASS);

    if (!db.open()) {
        m_error = "Ouverture de la connexion ODBC/Oracle impossible : " + db.lastError().text();
        qDebug() << "Echec connexion Oracle :" << m_error;
        return false;
    }

    qDebug() << "Connexion Oracle ouverte :" << ODBC_DSN << "utilisateur" << DB_USER;
    if (!creerTables()) {
        // Ne pas laisser l'interface annoncer une connexion utilisable si la
        // preparation du schema a echoue.
        const QString erreurSchema = m_error;
        db.close();
        m_error = erreurSchema;
        qDebug() << "Preparation du schema Oracle impossible :" << m_error;
        return false;
    }

    verifierContraintes();
    m_error.clear();
    qDebug() << "Tables Oracle verifiees";
    return true;
}
