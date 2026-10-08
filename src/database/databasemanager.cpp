#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDate>
#include <QPair>
#include <algorithm>

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

    // Animals table
    if (!query.exec(R"(CREATE TABLE IF NOT EXISTS animals (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL CHECK(length(trim(name)) > 0),
        species TEXT NOT NULL,
        breed TEXT,
        gender TEXT,
        birthDate TEXT,
        color TEXT,
        ownerId TEXT,
        ownerPhone TEXT,
        ownerEmail TEXT,
        notes TEXT,
        createdAt TEXT NOT NULL,
        updatedAt TEXT NOT NULL
    ))")) { m_error = query.lastError().text(); return false; }

    // Medical records table
    if (!query.exec(R"(CREATE TABLE IF NOT EXISTS medical_records (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        animalId INTEGER NOT NULL,
        type TEXT NOT NULL,
        description TEXT,
        veterinarian TEXT,
        date TEXT NOT NULL,
        status TEXT NOT NULL CHECK(status IN ('En cours','Terminé','Annulé')),
        notes TEXT,
        diagnosis TEXT,
        treatment TEXT,
        cost REAL DEFAULT 0.0,
        createdAt TEXT NOT NULL,
        updatedAt TEXT NOT NULL,
        FOREIGN KEY (animalId) REFERENCES animals(id) ON DELETE CASCADE
    ))")) { m_error = query.lastError().text(); return false; }

    // Vaccinations table
    if (!query.exec(R"(CREATE TABLE IF NOT EXISTS vaccinations (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        animalId INTEGER NOT NULL,
        vaccineName TEXT NOT NULL,
        vaccineType TEXT,
        administeredDate TEXT NOT NULL,
        nextDueDate TEXT,
        veterinarian TEXT,
        batchNumber TEXT,
        notes TEXT,
        isReminderSet INTEGER DEFAULT 0,
        createdAt TEXT NOT NULL,
        updatedAt TEXT NOT NULL,
        FOREIGN KEY (animalId) REFERENCES animals(id) ON DELETE CASCADE
    ))")) { m_error = query.lastError().text(); return false; }

    // Users table for medical records access
    if (!query.exec(R"(CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        userId TEXT NOT NULL UNIQUE CHECK(length(trim(userId)) > 0),
        username TEXT NOT NULL,
        role TEXT NOT NULL,
        createdAt TEXT NOT NULL
    ))")) { m_error = query.lastError().text(); return false; }

    m_error.clear(); return true;
}
QList<Service> DatabaseManager::getAllServices() {
    QList<Service> result;
    QSqlQuery query(m_db);
    if (!query.exec("SELECT id,name,description,responsible,category,dateTime,status,duration,price FROM services ORDER BY dateTime DESC,id DESC")) {
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
        result.append(s);
    }
    return result;
}
bool DatabaseManager::save(const Service &s, bool update) {
    if (s.name.trimmed().isEmpty() || s.name.trimmed().size()>120 || !s.dateTime.isValid()) {
        m_error="Vérifiez le nom et la date du service."; return false;
    }
    QSqlQuery query(m_db);
    query.prepare(update
        ? "UPDATE services SET name=?,description=?,responsible=?,category=?,dateTime=?,status=?,duration=?,price=? WHERE id=?"
        : "INSERT INTO services(name,description,responsible,category,dateTime,status,duration,price) VALUES(?,?,?,?,?,?,?,?)");
    query.addBindValue(s.name.trimmed());
    query.addBindValue(s.description.isNull()?QStringLiteral(""):s.description.trimmed());
    query.addBindValue(s.responsible.isNull()?QStringLiteral(""):s.responsible.trimmed());
    query.addBindValue(s.category); query.addBindValue(s.dateTime.toString(Qt::ISODate));
    query.addBindValue(s.status); query.addBindValue(s.duration); query.addBindValue(s.price);
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
        if(!addService(s)) { m_db.rollback(); return false; }
    }
    if(!m_db.commit()) { m_error=m_db.lastError().text(); return false; }
    return true;
}

// Animals
QList<Animal> DatabaseManager::getAllAnimals() {
    QList<Animal> result;
    QSqlQuery query(m_db);
    if (!query.exec("SELECT id,name,species,breed,gender,birthDate,color,ownerId,ownerPhone,ownerEmail,notes,createdAt,updatedAt FROM animals ORDER BY name ASC")) {
        m_error = query.lastError().text(); return result;
    }
    m_error.clear();
    while (query.next()) {
        Animal a;
        a.id = query.value(0).toInt();
        a.name = query.value(1).toString();
        a.species = query.value(2).toString();
        a.breed = query.value(3).toString();
        a.gender = query.value(4).toString();
        a.birthDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        a.color = query.value(6).toString();
        a.ownerId = query.value(7).toString();
        a.ownerPhone = query.value(8).toString();
        a.ownerEmail = query.value(9).toString();
        a.notes = query.value(10).toString();
        a.createdAt = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
        a.updatedAt = QDateTime::fromString(query.value(12).toString(), Qt::ISODate);
        result.append(a);
    }
    return result;
}

Animal DatabaseManager::getAnimalById(int id) {
    Animal a;
    QSqlQuery query(m_db);
    query.prepare("SELECT id,name,species,breed,gender,birthDate,color,ownerId,ownerPhone,ownerEmail,notes,createdAt,updatedAt FROM animals WHERE id=?");
    query.addBindValue(id);
    if (!query.exec()) { m_error = query.lastError().text(); return a; }
    if (query.next()) {
        a.id = query.value(0).toInt();
        a.name = query.value(1).toString();
        a.species = query.value(2).toString();
        a.breed = query.value(3).toString();
        a.gender = query.value(4).toString();
        a.birthDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        a.color = query.value(6).toString();
        a.ownerId = query.value(7).toString();
        a.ownerPhone = query.value(8).toString();
        a.ownerEmail = query.value(9).toString();
        a.notes = query.value(10).toString();
        a.createdAt = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
        a.updatedAt = QDateTime::fromString(query.value(12).toString(), Qt::ISODate);
    }
    m_error.clear();
    return a;
}

bool DatabaseManager::saveAnimal(const Animal &a, bool update) {
    if (a.name.trimmed().isEmpty() || a.name.trimmed().size() > 100) {
        m_error = "Vérifiez le nom de l'animal."; return false;
    }
    QSqlQuery query(m_db);
    query.prepare(update
        ? "UPDATE animals SET name=?,species=?,breed=?,gender=?,birthDate=?,color=?,ownerId=?,ownerPhone=?,ownerEmail=?,notes=?,updatedAt=? WHERE id=?"
        : "INSERT INTO animals(name,species,breed,gender,birthDate,color,ownerId,ownerPhone,ownerEmail,notes,createdAt,updatedAt) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.addBindValue(a.name.trimmed());
    query.addBindValue(a.species);
    query.addBindValue(a.breed);
    query.addBindValue(a.gender);
    query.addBindValue(a.birthDate.isValid() ? a.birthDate.toString(Qt::ISODate) : QString());
    query.addBindValue(a.color);
    query.addBindValue(a.ownerId);
    query.addBindValue(a.ownerPhone);
    query.addBindValue(a.ownerEmail);
    query.addBindValue(a.notes);
    const QDateTime now = QDateTime::currentDateTime();
    query.addBindValue(now.toString(Qt::ISODate));
    if (update) {
        query.addBindValue(a.id);
    } else {
        query.addBindValue(now.toString(Qt::ISODate));
    }
    if (!query.exec()) { m_error = query.lastError().text(); return false; }
    if (update && query.numRowsAffected() != 1) { m_error = "Cet animal n'existe plus."; return false; }
    if (!update) m_lastInsertedId = query.lastInsertId().toInt();
    m_error.clear(); return true;
}

bool DatabaseManager::addAnimal(const Animal &a) { return saveAnimal(a, false); }
bool DatabaseManager::updateAnimal(const Animal &a) { return saveAnimal(a, true); }

bool DatabaseManager::deleteAnimal(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM animals WHERE id=?");
    query.addBindValue(id);
    if (!query.exec()) { m_error = query.lastError().text(); return false; }
    if (query.numRowsAffected() != 1) { m_error = "Cet animal n'existe plus."; return false; }
    m_error.clear(); return true;
}

QList<Animal> DatabaseManager::searchAnimals(const QString &queryStr) {
    QList<Animal> result;
    QSqlQuery query(m_db);
    query.prepare("SELECT id,name,species,breed,gender,birthDate,color,ownerId,ownerPhone,ownerEmail,notes,createdAt,updatedAt FROM animals WHERE name LIKE ? OR species LIKE ? OR breed LIKE ? OR ownerId LIKE ? OR notes LIKE ? ORDER BY name ASC");
    QString pattern = "%" + queryStr + "%";
    query.addBindValue(pattern);
    query.addBindValue(pattern);
    query.addBindValue(pattern);
    query.addBindValue(pattern);
    query.addBindValue(pattern);
    if (!query.exec()) { m_error = query.lastError().text(); return result; }
    m_error.clear();
    while (query.next()) {
        Animal a;
        a.id = query.value(0).toInt();
        a.name = query.value(1).toString();
        a.species = query.value(2).toString();
        a.breed = query.value(3).toString();
        a.gender = query.value(4).toString();
        a.birthDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        a.color = query.value(6).toString();
        a.ownerId = query.value(7).toString();
        a.ownerPhone = query.value(8).toString();
        a.ownerEmail = query.value(9).toString();
        a.notes = query.value(10).toString();
        a.createdAt = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
        a.updatedAt = QDateTime::fromString(query.value(12).toString(), Qt::ISODate);
        result.append(a);
    }
    return result;
}

// Medical Records
QList<MedicalRecord> DatabaseManager::getMedicalRecords(int animalId) {
    QList<MedicalRecord> result;
    QSqlQuery query(m_db);
    query.prepare("SELECT id,animalId,type,description,veterinarian,date,status,notes,diagnosis,treatment,cost,createdAt,updatedAt FROM medical_records WHERE animalId=? ORDER BY date DESC");
    query.addBindValue(animalId);
    if (!query.exec()) { m_error = query.lastError().text(); return result; }
    m_error.clear();
    while (query.next()) {
        MedicalRecord r;
        r.id = query.value(0).toInt();
        r.animalId = query.value(1).toInt();
        r.type = query.value(2).toString();
        r.description = query.value(3).toString();
        r.veterinarian = query.value(4).toString();
        r.date = QDateTime::fromString(query.value(5).toString(), Qt::ISODate);
        r.status = query.value(6).toString();
        r.notes = query.value(7).toString();
        r.diagnosis = query.value(8).toString();
        r.treatment = query.value(9).toString();
        r.cost = query.value(10).toDouble();
        r.createdAt = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
        r.updatedAt = QDateTime::fromString(query.value(12).toString(), Qt::ISODate);
        result.append(r);
    }
    return result;
}

QList<MedicalRecord> DatabaseManager::getAllMedicalRecords() {
    QList<MedicalRecord> result;
    QSqlQuery query(m_db);
    if (!query.exec("SELECT id,animalId,type,description,veterinarian,date,status,notes,diagnosis,treatment,cost,createdAt,updatedAt FROM medical_records ORDER BY date DESC")) {
        m_error = query.lastError().text(); return result;
    }
    m_error.clear();
    while (query.next()) {
        MedicalRecord r;
        r.id = query.value(0).toInt();
        r.animalId = query.value(1).toInt();
        r.type = query.value(2).toString();
        r.description = query.value(3).toString();
        r.veterinarian = query.value(4).toString();
        r.date = QDateTime::fromString(query.value(5).toString(), Qt::ISODate);
        r.status = query.value(6).toString();
        r.notes = query.value(7).toString();
        r.diagnosis = query.value(8).toString();
        r.treatment = query.value(9).toString();
        r.cost = query.value(10).toDouble();
        r.createdAt = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
        r.updatedAt = QDateTime::fromString(query.value(12).toString(), Qt::ISODate);
        result.append(r);
    }
    return result;
}

bool DatabaseManager::saveMedicalRecord(const MedicalRecord &r, bool update) {
    if (r.animalId == 0 || r.type.isEmpty() || !r.date.isValid()) {
        m_error = "Vérifiez l'animal, le type et la date."; return false;
    }
    QSqlQuery query(m_db);
    query.prepare(update
        ? "UPDATE medical_records SET animalId=?,type=?,description=?,veterinarian=?,date=?,status=?,notes=?,diagnosis=?,treatment=?,cost=?,updatedAt=? WHERE id=?"
        : "INSERT INTO medical_records(animalId,type,description,veterinarian,date,status,notes,diagnosis,treatment,cost,createdAt,updatedAt) VALUES(?,?,?,?,?,?,?,?,?,?,?,?)");
    query.addBindValue(r.animalId);
    query.addBindValue(r.type);
    query.addBindValue(r.description);
    query.addBindValue(r.veterinarian);
    query.addBindValue(r.date.toString(Qt::ISODate));
    query.addBindValue(r.status);
    query.addBindValue(r.notes);
    query.addBindValue(r.diagnosis);
    query.addBindValue(r.treatment);
    query.addBindValue(r.cost);
    const QDateTime now = QDateTime::currentDateTime();
    query.addBindValue(now.toString(Qt::ISODate));
    if (update) {
        query.addBindValue(r.id);
    } else {
        query.addBindValue(now.toString(Qt::ISODate));
    }
    if (!query.exec()) { m_error = query.lastError().text(); return false; }
    if (update && query.numRowsAffected() != 1) { m_error = "Ce dossier n'existe plus."; return false; }
    if (!update) m_lastInsertedId = query.lastInsertId().toInt();
    m_error.clear(); return true;
}

bool DatabaseManager::addMedicalRecord(const MedicalRecord &r) { return saveMedicalRecord(r, false); }
bool DatabaseManager::updateMedicalRecord(const MedicalRecord &r) { return saveMedicalRecord(r, true); }

bool DatabaseManager::deleteMedicalRecord(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM medical_records WHERE id=?");
    query.addBindValue(id);
    if (!query.exec()) { m_error = query.lastError().text(); return false; }
    if (query.numRowsAffected() != 1) { m_error = "Ce dossier n'existe plus."; return false; }
    m_error.clear(); return true;
}

// Vaccinations
QList<Vaccination> DatabaseManager::getVaccinations(int animalId) {
    QList<Vaccination> result;
    QSqlQuery query(m_db);
    query.prepare("SELECT id,animalId,vaccineName,vaccineType,administeredDate,nextDueDate,veterinarian,batchNumber,notes,isReminderSet,createdAt,updatedAt FROM vaccinations WHERE animalId=? ORDER BY administeredDate DESC");
    query.addBindValue(animalId);
    if (!query.exec()) { m_error = query.lastError().text(); return result; }
    m_error.clear();
    while (query.next()) {
        Vaccination v;
        v.id = query.value(0).toInt();
        v.animalId = query.value(1).toInt();
        v.vaccineName = query.value(2).toString();
        v.vaccineType = query.value(3).toString();
        v.administeredDate = QDate::fromString(query.value(4).toString(), Qt::ISODate);
        v.nextDueDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        v.veterinarian = query.value(6).toString();
        v.batchNumber = query.value(7).toString();
        v.notes = query.value(8).toString();
        v.isReminderSet = query.value(9).toBool();
        v.createdAt = QDateTime::fromString(query.value(10).toString(), Qt::ISODate);
        v.updatedAt = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
        result.append(v);
    }
    return result;
}

QList<Vaccination> DatabaseManager::getAllVaccinations() {
    QList<Vaccination> result;
    QSqlQuery query(m_db);
    if (!query.exec("SELECT id,animalId,vaccineName,vaccineType,administeredDate,nextDueDate,veterinarian,batchNumber,notes,isReminderSet,createdAt,updatedAt FROM vaccinations ORDER BY nextDueDate ASC")) {
        m_error = query.lastError().text(); return result;
    }
    m_error.clear();
    while (query.next()) {
        Vaccination v;
        v.id = query.value(0).toInt();
        v.animalId = query.value(1).toInt();
        v.vaccineName = query.value(2).toString();
        v.vaccineType = query.value(3).toString();
        v.administeredDate = QDate::fromString(query.value(4).toString(), Qt::ISODate);
        v.nextDueDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        v.veterinarian = query.value(6).toString();
        v.batchNumber = query.value(7).toString();
        v.notes = query.value(8).toString();
        v.isReminderSet = query.value(9).toBool();
        v.createdAt = QDateTime::fromString(query.value(10).toString(), Qt::ISODate);
        v.updatedAt = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
        result.append(v);
    }
    return result;
}

QList<Vaccination> DatabaseManager::getVaccinationsDueSoon(int days) {
    QList<Vaccination> result;
    QSqlQuery query(m_db);
    const QDate today = QDate::currentDate();
    const QDate dueDate = today.addDays(days);
    query.prepare("SELECT id,animalId,vaccineName,vaccineType,administeredDate,nextDueDate,veterinarian,batchNumber,notes,isReminderSet,createdAt,updatedAt FROM vaccinations WHERE nextDueDate BETWEEN ? AND ? ORDER BY nextDueDate ASC");
    query.addBindValue(today.toString(Qt::ISODate));
    query.addBindValue(dueDate.toString(Qt::ISODate));
    if (!query.exec()) { m_error = query.lastError().text(); return result; }
    m_error.clear();
    while (query.next()) {
        Vaccination v;
        v.id = query.value(0).toInt();
        v.animalId = query.value(1).toInt();
        v.vaccineName = query.value(2).toString();
        v.vaccineType = query.value(3).toString();
        v.administeredDate = QDate::fromString(query.value(4).toString(), Qt::ISODate);
        v.nextDueDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        v.veterinarian = query.value(6).toString();
        v.batchNumber = query.value(7).toString();
        v.notes = query.value(8).toString();
        v.isReminderSet = query.value(9).toBool();
        v.createdAt = QDateTime::fromString(query.value(10).toString(), Qt::ISODate);
        v.updatedAt = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
        result.append(v);
    }
    return result;
}

QList<Vaccination> DatabaseManager::getVaccinationsOverdue() {
    QList<Vaccination> result;
    QSqlQuery query(m_db);
    const QDate today = QDate::currentDate();
    query.prepare("SELECT id,animalId,vaccineName,vaccineType,administeredDate,nextDueDate,veterinarian,batchNumber,notes,isReminderSet,createdAt,updatedAt FROM vaccinations WHERE nextDueDate < ? ORDER BY nextDueDate ASC");
    query.addBindValue(today.toString(Qt::ISODate));
    if (!query.exec()) { m_error = query.lastError().text(); return result; }
    m_error.clear();
    while (query.next()) {
        Vaccination v;
        v.id = query.value(0).toInt();
        v.animalId = query.value(1).toInt();
        v.vaccineName = query.value(2).toString();
        v.vaccineType = query.value(3).toString();
        v.administeredDate = QDate::fromString(query.value(4).toString(), Qt::ISODate);
        v.nextDueDate = QDate::fromString(query.value(5).toString(), Qt::ISODate);
        v.veterinarian = query.value(6).toString();
        v.batchNumber = query.value(7).toString();
        v.notes = query.value(8).toString();
        v.isReminderSet = query.value(9).toBool();
        v.createdAt = QDateTime::fromString(query.value(10).toString(), Qt::ISODate);
        v.updatedAt = QDateTime::fromString(query.value(11).toString(), Qt::ISODate);
        result.append(v);
    }
    return result;
}

bool DatabaseManager::saveVaccination(const Vaccination &v, bool update) {
    if (v.animalId == 0 || v.vaccineName.isEmpty() || !v.administeredDate.isValid()) {
        m_error = "Vérifiez l'animal, le nom du vaccin et la date d'administration."; return false;
    }
    QSqlQuery query(m_db);
    query.prepare(update
        ? "UPDATE vaccinations SET animalId=?,vaccineName=?,vaccineType=?,administeredDate=?,nextDueDate=?,veterinarian=?,batchNumber=?,notes=?,isReminderSet=?,updatedAt=? WHERE id=?"
        : "INSERT INTO vaccinations(animalId,vaccineName,vaccineType,administeredDate,nextDueDate,veterinarian,batchNumber,notes,isReminderSet,createdAt,updatedAt) VALUES(?,?,?,?,?,?,?,?,?,?,?)");
    query.addBindValue(v.animalId);
    query.addBindValue(v.vaccineName);
    query.addBindValue(v.vaccineType);
    query.addBindValue(v.administeredDate.toString(Qt::ISODate));
    query.addBindValue(v.nextDueDate.isValid() ? v.nextDueDate.toString(Qt::ISODate) : QString());
    query.addBindValue(v.veterinarian);
    query.addBindValue(v.batchNumber);
    query.addBindValue(v.notes);
    query.addBindValue(v.isReminderSet ? 1 : 0);
    const QDateTime now = QDateTime::currentDateTime();
    query.addBindValue(now.toString(Qt::ISODate));
    if (update) {
        query.addBindValue(v.id);
    } else {
        query.addBindValue(now.toString(Qt::ISODate));
    }
    if (!query.exec()) { m_error = query.lastError().text(); return false; }
    if (update && query.numRowsAffected() != 1) { m_error = "Ce vaccin n'existe plus."; return false; }
    if (!update) m_lastInsertedId = query.lastInsertId().toInt();
    m_error.clear(); return true;
}

bool DatabaseManager::addVaccination(const Vaccination &v) { return saveVaccination(v, false); }
bool DatabaseManager::updateVaccination(const Vaccination &v) { return saveVaccination(v, true); }

bool DatabaseManager::deleteVaccination(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM vaccinations WHERE id=?");
    query.addBindValue(id);
    if (!query.exec()) { m_error = query.lastError().text(); return false; }
    if (query.numRowsAffected() != 1) { m_error = "Ce vaccin n'existe plus."; return false; }
    m_error.clear(); return true;
}

// Analytics
MedicalAnalytics DatabaseManager::getMedicalAnalytics() {
    MedicalAnalytics analytics;
    analytics.clear();

    QList<Animal> animals = getAllAnimals();
    analytics.totalAnimals = animals.size();

    for (const auto &a : animals) {
        if (!a.species.isEmpty()) {
            analytics.animalsBySpecies[a.species]++;
        }
    }

    QList<MedicalRecord> records = getAllMedicalRecords();
    analytics.totalRecords = records.size();

    const QDate today = QDate::currentDate();
    const QDate thisMonthStart = today.addDays(1 - today.day());
    const QDate thisYearStart = QDate(today.year(), 1, 1);

    for (const auto &r : records) {
        if (!r.type.isEmpty()) {
            analytics.recordsByType[r.type]++;
        }
        if (!r.status.isEmpty()) {
            analytics.recordsByStatus[r.status]++;
        }
        analytics.totalCost += r.cost;

        if (r.date.date() >= thisMonthStart) {
            analytics.recordsThisMonth++;
        }
        if (r.date.date() >= thisYearStart) {
            analytics.recordsThisYear++;
        }
    }

    QList<Vaccination> vaccinations = getAllVaccinations();
    analytics.totalVaccinations = vaccinations.size();

    for (const auto &v : vaccinations) {
        if (v.isDueSoon(7)) {
            analytics.vaccinationsDueSoon++;
        }
        if (v.isOverdue()) {
            analytics.vaccinationsOverdue++;
        }
    }

    // Top species
    QList<QPair<QString, int>> speciesList;
    for (auto it = analytics.animalsBySpecies.constBegin(); it != analytics.animalsBySpecies.constEnd(); ++it) {
        speciesList.append(qMakePair(it.key(), it.value()));
    }
    std::sort(speciesList.begin(), speciesList.end(), [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
        return a.second > b.second;
    });
    for (int i = 0; i < qMin(5, speciesList.size()); ++i) {
        analytics.topSpecies.append(speciesList[i].first);
    }

    // Recent animals (last 5 added)
    QList<QPair<QDateTime, QString>> recentList;
    for (const auto &a : animals) {
        recentList.append(qMakePair(a.createdAt, a.name));
    }
    std::sort(recentList.begin(), recentList.end(), [](const QPair<QDateTime, QString> &a, const QPair<QDateTime, QString> &b) {
        return a.first > b.first;
    });
    for (int i = 0; i < qMin(5, recentList.size()); ++i) {
        analytics.recentAnimals.append(recentList[i].second);
    }

    m_error.clear();
    return analytics;
}

// Users
bool DatabaseManager::verifyUserId(const QString &userId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM users WHERE userId=? COLLATE NOCASE");
    query.addBindValue(userId.trimmed());
    if (!query.exec()) {
        m_error = query.lastError().text();
        return false;
    }
    if (query.next()) {
        int count = query.value(0).toInt();
        m_error.clear();
        return count > 0;
    }
    m_error.clear();
    return false;
}

bool DatabaseManager::addDefaultUsers() {
    QSqlQuery query(m_db);
    if (!query.exec("SELECT COUNT(*) FROM users")) {
        m_error = query.lastError().text();
        return false;
    }
    if (query.next() && query.value(0).toInt() > 0) {
        m_error.clear();
        return true; // Users already exist
    }

    const QDateTime now = QDateTime::currentDateTime();
    const QStringList defaultUsers = {
        "admin", "veterinaire", "infirmier", "assistant"
    };

    if (!m_db.transaction()) {
        m_error = m_db.lastError().text();
        return false;
    }

    for (const QString &uid : defaultUsers) {
        query.prepare("INSERT INTO users(userId, username, role, createdAt) VALUES(?, ?, ?, ?)");
        query.addBindValue(uid);
        query.addBindValue(uid);
        query.addBindValue(uid);
        query.addBindValue(now.toString(Qt::ISODate));
        if (!query.exec()) {
            m_db.rollback();
            m_error = query.lastError().text();
            return false;
        }
    }

    if (!m_db.commit()) {
        m_error = m_db.lastError().text();
        return false;
    }

    m_error.clear();
    return true;
}
