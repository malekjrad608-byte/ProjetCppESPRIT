#include "connection.h"

#include <QDebug>
#include <QStringList>
#include <QtGlobal>

// ======================================================================
// PARAMETRES ORACLE
// DSN ODBC 64 bits -> service Oracle de votre installation.
// Modifiez DSN / utilisateur / mot de passe si votre configuration locale est differente.
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
    q.prepare("SELECT 1 FROM DUAL");
    if (q.exec() && q.next()) {
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

// Type Oracle de la colonne (NUMBER, VARCHAR2, DATE...) ou chaine vide si la colonne n'existe pas.
QString Connection::typeColonne(const QString &table, const QString &colonne)
{
    QSqlQuery q(db);
    q.prepare("SELECT DATA_TYPE FROM USER_TAB_COLUMNS "
              "WHERE TABLE_NAME = :table AND COLUMN_NAME = :colonne");
    q.bindValue(":table", table.toUpper());
    q.bindValue(":colonne", colonne.toUpper());
    QString type;
    if (q.exec() && q.next())
        type = q.value(0).toString().toUpper();
    q.finish();
    return type;
}

bool Connection::colonneEstIdentity(const QString &table, const QString &colonne)
{
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(*) FROM USER_TAB_IDENTITY_COLS "
              "WHERE TABLE_NAME = :table AND COLUMN_NAME = :colonne");
    q.bindValue(":table", table.toUpper());
    q.bindValue(":colonne", colonne.toUpper());
    bool identity = false;
    if (q.exec() && q.next())
        identity = q.value(0).toInt() > 0;
    q.finish();
    return identity;
}

// Vrai si un trigger actif BEFORE INSERT FOR EACH ROW de la table remplit
// :NEW.<colonne> avec une sequence (NEXTVAL), quel que soit le nom du trigger.
bool Connection::triggerAutoIncrementExiste(const QString &table, const QString &colonne)
{
    QSqlQuery q(db);
    q.prepare("SELECT T.TRIGGER_NAME, S.TEXT FROM USER_TRIGGERS T "
              "JOIN USER_SOURCE S ON S.NAME = T.TRIGGER_NAME AND S.TYPE = 'TRIGGER' "
              "WHERE T.TABLE_NAME = :table AND T.STATUS = 'ENABLED' "
              "AND INSTR(UPPER(T.TRIGGERING_EVENT), 'INSERT') > 0 "
              "AND T.TRIGGER_TYPE LIKE '%EACH ROW%' ORDER BY T.TRIGGER_NAME, S.LINE");
    q.bindValue(":table", table.toUpper());

    const QString cible = ":NEW." + colonne.toUpper();
    bool existe = false;
    QString nomCourant;
    QString corpsCourant;

    auto corpsValide = [&](const QString &corps) {
        const QString haut = corps.toUpper();
        int pos = haut.indexOf(cible);
        while (pos >= 0) {
            // ":NEW.ID_EMP" ne doit pas etre confondu avec ":NEW.ID_EMP_X"
            const int suite = pos + cible.size();
            const QChar c = suite < haut.size() ? haut.at(suite) : QChar(' ');
            if (!(c.isLetterOrNumber() || c == QLatin1Char('_'))) {
                return haut.contains("NEXTVAL");
            }
            pos = haut.indexOf(cible, suite);
        }
        return false;
    };

    if (q.exec()) {
        while (q.next()) {
            const QString nom = q.value(0).toString();
            if (!nomCourant.isEmpty() && nom != nomCourant) {
                if (corpsValide(corpsCourant)) {
                    existe = true;
                    break;
                }
                corpsCourant.clear();
            }
            nomCourant = nom;
            corpsCourant += q.value(1).toString() + QLatin1Char(' ');
        }
        if (!existe)
            existe = corpsValide(corpsCourant);
    }
    q.finish();
    return existe;
}

// Controle de conformite (lecture seule) de la table EMPLOYER.
// Retourne "" si les 17 colonnes existent, sinon la description de l'ecart.
QString Connection::verifierSchemaEmployes()
{
    if (!tableExiste("EMPLOYER"))
        return "La table EMPLOYER est absente du schéma Oracle de l'utilisateur connecté.";

    const QStringList attendues = {
        "ID_EMP", "NOM", "PRENOM", "CIN", "DATE_NAISS", "ADRESSE", "TEL", "POSTE",
        "COURRIEL", "COMPETENCES", "DATE_EMBAUCHE", "TYPE_CONTRAT", "SALAIRE", "CHARGE",
        "DISPONIBLE", "DISPO_REPOS", "ID_SERV"
    };
    QStringList absentes;
    for (const QString &colonne : attendues) {
        if (typeColonne("EMPLOYER", colonne).isEmpty())
            absentes << colonne;
    }
    if (!absentes.isEmpty())
        return "Colonnes absentes de la table EMPLOYER : " + absentes.join(", ") + ".";

    // ID_EMP : genere par Oracle (IDENTITY ou trigger + sequence) ou calcule par l'application.
    m_idAuto = colonneEstIdentity("EMPLOYER", "ID_EMP")
               || triggerAutoIncrementExiste("EMPLOYER", "ID_EMP");
    qDebug() << (m_idAuto ? "ID_EMP genere par Oracle" : "ID_EMP calcule par l'application (MAX+1)");
    return QString();
}

bool Connection::createConnection()
{
    if (db.isOpen())
        db.close();
    m_erreurSchemaEmployes.clear();
    m_idAuto = false;

    db.setDatabaseName(ODBC_DSN);
    db.setUserName(DB_USER);
    db.setPassword(DB_PASS);

    if (!db.open()) {
        m_error = "Ouverture de la connexion ODBC/Oracle impossible : " + db.lastError().text();
        qDebug() << "Echec connexion Oracle :" << m_error;
        return false;
    }

    qDebug() << "Connexion Oracle ouverte :" << ODBC_DSN << "utilisateur" << DB_USER;
    m_error.clear();

    // La connexion est valide a partir d'ici. Un probleme de schema Employes ne
    // ferme pas la connexion : Locaux, Dossiers medicaux, etc. continuent de fonctionner.
    m_erreurSchemaEmployes = verifierSchemaEmployes();
    if (!m_erreurSchemaEmployes.isEmpty())
        qDebug() << "Schema Employes non conforme (connexion Oracle conservee) :"
                 << m_erreurSchemaEmployes;
    else
        qDebug() << "Schema Employes conforme";

    return true;
}
