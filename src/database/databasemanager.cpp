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
#include "../medical/medicalrecord.h"
#include "../employees/employe.h"

DatabaseManager &DatabaseManager::instance() { static DatabaseManager manager; return manager; }
bool DatabaseManager::initializeOracle(const QString &configPath) {
    if(m_db.isValid()) m_db.close();
    m_db=QSqlDatabase();
    // Remove only the demo connection; the teacher Singleton owns the Oracle handle.
    if(QSqlDatabase::contains("aniworld-services")) QSqlDatabase::removeDatabase("aniworld-services");
    m_localPath=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/services.sqlite";
    return OracleConnection::open(m_db,&m_error,configPath);
}
bool DatabaseManager::initialize(const QString &path) {
    m_localPath=path;
    m_lastInsertedId = 0;
    if (m_db.isValid()) m_db.close();
    if(m_db.driverName()!="QSQLITE") {
        m_db=QSqlDatabase(); QSqlDatabase::removeDatabase("aniworld-services");
        m_db = QSqlDatabase::addDatabase("QSQLITE", "aniworld-services");
    }
    m_db.setDatabaseName(path);
    if (!m_db.open()) { m_error = m_db.lastError().text(); return false; }
    QSqlQuery query(m_db);
    if (!(query.prepare(R"(CREATE TABLE IF NOT EXISTS services (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL CHECK(length(trim(name)) > 0),
        description TEXT NOT NULL DEFAULT '', responsible TEXT NOT NULL DEFAULT '',
        category TEXT NOT NULL, dateTime TEXT NOT NULL,
        status TEXT NOT NULL CHECK(status IN ('Planifié','En cours','Terminé','Annulé')),
        duration INTEGER NOT NULL CHECK(duration BETWEEN 5 AND 1440),
        price REAL NOT NULL CHECK(price >= 0)
    ))") && query.exec())) { m_error = query.lastError().text(); return false; }
    // Additive migration: preserve all existing consultations and their IDs.
    const auto columns=m_db.record("services");
    if (!m_db.transaction()) { m_error=m_db.lastError().text(); return false; }
    const QStringList additions={"id_employe INTEGER", "id_client INTEGER", "id_local INTEGER", "clientName TEXT NOT NULL DEFAULT ''", "invoiceNumber TEXT NOT NULL DEFAULT ''", "invoiceDate TEXT NOT NULL DEFAULT ''"};
    for (const auto &column:additions) {
        if (!columns.contains(column.section(' ',0,0)) && !(query.prepare("ALTER TABLE services ADD COLUMN "+column) && query.exec())) {
            m_error=query.lastError().text(); m_db.rollback(); return false;
        }
    }
    // Identity is allocated once; editing the consultation never renumbers it.
    const QStringList migration={
        "UPDATE services SET invoiceNumber=printf('FAC-%s-%06d',substr(dateTime,1,4),id), invoiceDate=strftime('%Y-%m-%dT%H:%M:%S','now','localtime') WHERE invoiceNumber=''",
        "CREATE UNIQUE INDEX IF NOT EXISTS service_invoice_number ON services(invoiceNumber)",
        "CREATE TRIGGER IF NOT EXISTS service_invoice_created AFTER INSERT ON services BEGIN UPDATE services SET invoiceNumber=printf('FAC-%s-%06d',substr(NEW.dateTime,1,4),NEW.id), invoiceDate=strftime('%Y-%m-%dT%H:%M:%S','now','localtime') WHERE id=NEW.id; END"
    };
    for (const auto &sql:migration) if (!(query.prepare(sql) && query.exec())) { m_error=query.lastError().text(); m_db.rollback(); return false; }
    if (!m_db.commit()) { m_error=m_db.lastError().text(); return false; }
    if(!Employe::initialize(m_db,&m_error) || !Accounts::initialize(m_db,&m_error) || !MedicalRecord::initialize(m_db,&m_error)) return false;
    m_error.clear(); return true;
}
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
bool DatabaseManager::addExamples() {
    if(!getAllServices().isEmpty()) return true;
    if(!m_error.isEmpty()) return false;
    struct Example { const char *name; const char *category; const char *person; int hour; int day; const char *status; int duration; double price; const char *description; };
    const Example examples[] = {
        {"Toilettage complet","Toilettage","Sarra Ben Ali",9,0,"Terminé",60,65,"Bain, brossage et coupe adaptés au pelage. Produits doux pour peaux sensibles."},
        {"Consultation de bien-être","Soins & bien-être","Dr. Yasmine Amara",10,0,"En cours",30,50,"Bilan général et conseils de prévention."},
        {"Séance d'éducation","Éducation","Amine Trabelsi",14,0,"Planifié",45,40,"Séance individuelle : rappel, marche en laisse et socialisation."},
        {"Bain & brossage","Toilettage","Sarra Ben Ali",15,0,"Planifié",30,35,"Entretien du pelage et nettoyage en douceur."},
        {"Garde à la journée","Pension & garde","Amine Trabelsi",8,1,"Planifié",480,45,"Accueil de jour avec promenades, repas et temps de jeu."},
        {"Soin des griffes","Soins & bien-être","Dr. Yasmine Amara",11,-1,"Terminé",15,20,"Coupe des griffes et contrôle des coussinets."},
        {"Éducation découverte","Éducation","Amine Trabelsi",16,-1,"Annulé",30,25,"Première rencontre et évaluation des besoins."}
    };
    if(!m_db.transaction()) { m_error=m_db.lastError().text(); return false; }
    for(const auto &e:examples) {
        Service s; s.setName(QString::fromUtf8(e.name)); s.setCategory(QString::fromUtf8(e.category));
        s.setResponsible(QString::fromUtf8(e.person)); s.setStatus(QString::fromUtf8(e.status));
        s.setDateTime(QDateTime(QDate::currentDate().addDays(e.day),QTime(e.hour,0)));
        s.setDuration(e.duration); s.setPrice(e.price); s.setDescription(QString::fromUtf8(e.description));
        s.setClientName(QStringList{"Nour Ben Salah","Ahmed Trabelsi","Inès Mansouri"}.at((e.hour+e.day+3)%3));
        if(!addService(s)) { m_db.rollback(); return false; }
    }
    if(!m_db.commit()) { m_error=m_db.lastError().text(); return false; }
    return true;
}
