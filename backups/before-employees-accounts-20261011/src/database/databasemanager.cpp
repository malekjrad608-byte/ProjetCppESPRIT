#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QSqlRecord>
#include <cmath>

DatabaseManager &DatabaseManager::instance() { static DatabaseManager manager; return manager; }
bool DatabaseManager::initialize(const QString &path) {
    m_lastInsertedId = 0;
    if (m_db.isValid()) m_db.close();
    else m_db = QSqlDatabase::addDatabase("QSQLITE", "aniworld-services");
    m_db.setDatabaseName(path);
    if (!m_db.open()) { m_error = m_db.lastError().text(); return false; }
    QSqlQuery query(m_db);
    if (!query.exec(R"(CREATE TABLE IF NOT EXISTS services (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL CHECK(length(trim(name)) > 0),
        description TEXT NOT NULL DEFAULT '', responsible TEXT NOT NULL DEFAULT '',
        category TEXT NOT NULL, dateTime TEXT NOT NULL,
        status TEXT NOT NULL CHECK(status IN ('Planifié','En cours','Terminé','Annulé')),
        duration INTEGER NOT NULL CHECK(duration BETWEEN 5 AND 1440),
        price REAL NOT NULL CHECK(price >= 0)
    ))")) { m_error = query.lastError().text(); return false; }
    // Additive migration: preserve all existing consultations and their IDs.
    const auto columns=m_db.record("services");
    if (!m_db.transaction()) { m_error=m_db.lastError().text(); return false; }
    const QStringList additions={"clientName TEXT NOT NULL DEFAULT ''", "invoiceNumber TEXT NOT NULL DEFAULT ''", "invoiceDate TEXT NOT NULL DEFAULT ''"};
    for (const auto &column:additions) {
        if (!columns.contains(column.section(' ',0,0)) && !query.exec("ALTER TABLE services ADD COLUMN "+column)) {
            m_error=query.lastError().text(); m_db.rollback(); return false;
        }
    }
    // Identity is allocated once; editing the consultation never renumbers it.
    const QStringList migration={
        "UPDATE services SET invoiceNumber=printf('FAC-%s-%06d',substr(dateTime,1,4),id), invoiceDate=strftime('%Y-%m-%dT%H:%M:%S','now','localtime') WHERE invoiceNumber=''",
        "CREATE UNIQUE INDEX IF NOT EXISTS service_invoice_number ON services(invoiceNumber)",
        "CREATE TRIGGER IF NOT EXISTS service_invoice_created AFTER INSERT ON services BEGIN UPDATE services SET invoiceNumber=printf('FAC-%s-%06d',substr(NEW.dateTime,1,4),NEW.id), invoiceDate=strftime('%Y-%m-%dT%H:%M:%S','now','localtime') WHERE id=NEW.id; END"
    };
    for (const auto &sql:migration) if (!query.exec(sql)) { m_error=query.lastError().text(); m_db.rollback(); return false; }
    if (!m_db.commit()) { m_error=m_db.lastError().text(); return false; }
    m_error.clear(); return true;
}
QList<Service> DatabaseManager::getAllServices() {
    QList<Service> result;
    QSqlQuery query(m_db);
    if (!query.exec("SELECT id,name,description,responsible,category,dateTime,status,duration,price,clientName,invoiceNumber,invoiceDate FROM services ORDER BY dateTime DESC,id DESC")) {
        m_error = query.lastError().text(); return result;
    }
    m_error.clear();
    while (query.next()) {
        Service s;
        s.id=query.value(0).toInt(); s.name=query.value(1).toString();
        s.description=query.value(2).toString(); s.responsible=query.value(3).toString();
        s.category=query.value(4).toString();
        s.dateTime=QDateTime::fromString(query.value(5).toString(),Qt::ISODate);
        s.status=query.value(6).toString(); s.duration=query.value(7).toInt(); s.price=query.value(8).toDouble();
        s.clientName=query.value(9).toString(); s.invoiceNumber=query.value(10).toString();
        s.invoiceDate=QDateTime::fromString(query.value(11).toString(),Qt::ISODate);
        result.append(s);
    }
    return result;
}
bool DatabaseManager::save(const Service &s, bool update) {
    if (s.name.trimmed().isEmpty() || s.name.trimmed().size()>120 || !s.dateTime.isValid()) {
        m_error="Vérifiez le nom et la date du service."; return false;
    }
    if (s.clientName.size()>120 || s.description.size()>5000 || !std::isfinite(s.price) || s.price<0 || s.price>99999.99) {
        m_error="Vérifiez le client, le détail et le tarif de la prestation."; return false;
    }
    QSqlQuery query(m_db);
    query.prepare(update
        ? "UPDATE services SET name=?,description=?,responsible=?,category=?,dateTime=?,status=?,duration=?,price=?,clientName=? WHERE id=?"
        : "INSERT INTO services(name,description,responsible,category,dateTime,status,duration,price,clientName) VALUES(?,?,?,?,?,?,?,?,?)");
    query.addBindValue(s.name.trimmed());
    query.addBindValue(s.description.isNull()?QStringLiteral(""):s.description.trimmed());
    query.addBindValue(s.responsible.isNull()?QStringLiteral(""):s.responsible.trimmed());
    query.addBindValue(s.category); query.addBindValue(s.dateTime.toString(Qt::ISODate));
    query.addBindValue(s.status); query.addBindValue(s.duration); query.addBindValue(s.price);
    query.addBindValue(s.clientName.isNull()?QStringLiteral(""):s.clientName.trimmed());
    if(update) query.addBindValue(s.id);
    if(!query.exec()) { m_error=query.lastError().text(); return false; }
    if(update && query.numRowsAffected()!=1) { m_error="Ce service n'existe plus."; return false; }
    if (!update) m_lastInsertedId = query.lastInsertId().toInt();
    m_error.clear(); return true;
}
bool DatabaseManager::addService(const Service &s) { return save(s,false); }
bool DatabaseManager::updateService(const Service &s) { return save(s,true); }
bool DatabaseManager::deleteService(int id) {
    QSqlQuery query(m_db); query.prepare("DELETE FROM services WHERE id=?"); query.addBindValue(id);
    if(!query.exec()) { m_error=query.lastError().text(); return false; }
    if(query.numRowsAffected()!=1) { m_error="Ce service n'existe plus."; return false; }
    m_error.clear(); return true;
}
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
        Service s; s.name=QString::fromUtf8(e.name); s.category=QString::fromUtf8(e.category);
        s.responsible=QString::fromUtf8(e.person); s.status=QString::fromUtf8(e.status);
        s.dateTime=QDateTime(QDate::currentDate().addDays(e.day),QTime(e.hour,0));
        s.duration=e.duration; s.price=e.price; s.description=QString::fromUtf8(e.description);
        s.clientName=QStringList{"Nour Ben Salah","Ahmed Trabelsi","Inès Mansouri"}.at((e.hour+e.day+3)%3);
        if(!addService(s)) { m_db.rollback(); return false; }
    }
    if(!m_db.commit()) { m_error=m_db.lastError().text(); return false; }
    return true;
}
