#include "databasemanager.h"
#include "oracleconnection.h"
#include "../models/service.h"

#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QSqlRecord>
#include <cmath>

DatabaseManager &DatabaseManager::instance() { static DatabaseManager manager; return manager; }
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
