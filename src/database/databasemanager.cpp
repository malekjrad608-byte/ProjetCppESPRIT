#include "databasemanager.h"
#include "oracleconnection.h"
#include "../models/service.h"

#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QSqlRecord>
#include <cmath>
#include "../auth/accounts.h"

DatabaseManager &DatabaseManager::instance() { static DatabaseManager manager; return manager; }
bool DatabaseManager::initializeOracle(const QString &configPath) {
    if(m_db.isValid()) m_db.close();
    m_db=QSqlDatabase();
    // Remove only the demo connection; the teacher Singleton owns the Oracle handle.
    if(QSqlDatabase::contains("aniworld-services")) QSqlDatabase::removeDatabase("aniworld-services");
    m_localPath=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/services.sqlite";
    return OracleConnection::open(m_db,&m_error,configPath);
}
bool DatabaseManager::initialize(const QString &) { m_error="Cette copie Services utilise la connexion Oracle existante."; return false; }
QList<Service> DatabaseManager::getAllServices() { return Service::afficherListe(&m_error); }
bool DatabaseManager::save(const Service &s,bool update) {
    Service entity=s;
    const bool ok=update?entity.modifier(&m_error):entity.ajouter(&m_error);
    if(ok)m_lastInsertedId=entity.getId();
    return ok;
}
bool DatabaseManager::addService(const Service &s) { return save(s,false); }
bool DatabaseManager::updateService(const Service &s) { return save(s,true); }
bool DatabaseManager::deleteService(int id) { return Service::supprimer(id,&m_error); }
bool DatabaseManager::addExamples() { return false; }
