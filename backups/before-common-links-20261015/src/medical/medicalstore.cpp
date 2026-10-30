#include "medicalstore.h"
#include "../ui/localchoice.h"
#include <QSqlRecord>
#include "../database/oracleconnection.h"
#include "../ui/invoice.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <cmath>

namespace {
QSqlDatabase database() { return QSqlDatabase::database("aniworld-services"); }
bool fail(QString *error,const QString &message) { if(error) *error=message; return false; }
QString number(qint64 id) { return QString("MED-%1").arg(id,6,10,QChar('0')); }
QString clean(const QString &text) { return text.isNull()?QStringLiteral(""):text.trimmed(); }
}
namespace MedicalStore {
bool initialize(QSqlDatabase db,QString *error) {
    QSqlQuery q(db);
    if(!q.exec(R"(CREATE TABLE IF NOT EXISTS medical_records (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        animal_id TEXT NOT NULL, name TEXT NOT NULL, species TEXT NOT NULL, breed TEXT NOT NULL,
        age INTEGER NOT NULL CHECK(age BETWEEN 0 AND 30), sex TEXT NOT NULL, owner TEXT NOT NULL,
        consultation_date TEXT NOT NULL, reason TEXT NOT NULL, diagnosis TEXT NOT NULL,
        treatment TEXT NOT NULL, vaccinations TEXT NOT NULL, allergies TEXT NOT NULL,
        weight REAL NOT NULL CHECK(weight BETWEEN 0 AND 100), observations TEXT NOT NULL,
        vaccination_status TEXT NOT NULL CHECK(vaccination_status IN ('À jour','À renouveler'))
    ))")) return fail(error,q.lastError().text());
    if(!db.record("medical_records").contains("id_local") && !q.exec("ALTER TABLE medical_records ADD COLUMN id_local INTEGER")) return fail(error,q.lastError().text());
    if(error) error->clear();
    return true;
}
QVector<MedicalRecord> all(QString *error) {
    QVector<MedicalRecord> result; QSqlQuery q(database());
    if(!q.exec("SELECT id,animal_id,name,species,breed,age,sex,owner,consultation_date,reason,diagnosis,treatment,vaccinations,allergies,weight,observations,vaccination_status,id_local FROM medical_records ORDER BY consultation_date DESC,id DESC")) {
        fail(error,q.lastError().text()); return result;
    }
    while(q.next()) {
        MedicalRecord r; r.storageId=q.value(0).toLongLong(); r.medical.recordId=number(r.storageId);
        r.animal.animalId=q.value(1).toString(); r.animal.name=q.value(2).toString(); r.animal.species=q.value(3).toString();
        r.animal.breed=q.value(4).toString(); r.animal.age=q.value(5).toInt(); r.animal.sex=q.value(6).toString(); r.animal.owner=q.value(7).toString();
        r.medical.consultationDate=QDate::fromString(q.value(8).toString(),Qt::ISODate);
        r.medical.reason=q.value(9).toString(); r.medical.diagnosis=q.value(10).toString(); r.medical.treatment=q.value(11).toString();
        r.medical.vaccinations=q.value(12).toString(); r.medical.allergies=q.value(13).toString(); r.medical.weight=q.value(14).toDouble();
        r.medical.observations=q.value(15).toString(); r.medical.vaccinationStatus=q.value(16).toString(); r.medical.localId=q.value(17).toInt(); result.append(r);
    }
    if(error) error->clear();
    return result;
}
bool save(MedicalRecord &r,QString *error) {
    if(r.animal.animalId.trimmed().isEmpty() || r.animal.name.trimmed().isEmpty() || r.animal.owner.trimmed().isEmpty() || !r.medical.consultationDate.isValid())
        return fail(error,"Renseignez l'identifiant de l'animal, son nom, son propriétaire et une date valide.");
    for(const auto &value:{r.animal.animalId,r.animal.name,r.animal.owner,r.animal.breed})
        if(value.size()>120) return fail(error,"Les informations de l'animal sont limitées à 120 caractères par champ.");
    for(const auto &value:{r.medical.reason,r.medical.diagnosis,r.medical.treatment,r.medical.vaccinations,r.medical.allergies,r.medical.observations})
        if(value.size()>5000) return fail(error,"Chaque note médicale est limitée à 5 000 caractères.");
    if(!QStringList{"Chien","Chat","Lapin","Oiseau"}.contains(r.animal.species) || !QStringList{"Mâle","Femelle"}.contains(r.animal.sex)
        || r.animal.age<0 || r.animal.age>30 || !std::isfinite(r.medical.weight) || r.medical.weight<0 || r.medical.weight>100
        || !QStringList{"À jour","À renouveler"}.contains(r.medical.vaccinationStatus)) return fail(error,"Vérifiez l'espèce, le sexe, l'âge, le poids et le statut vaccinal.");
    QSqlQuery q(database());
    q.prepare(r.storageId>0
        ? "UPDATE medical_records SET animal_id=?,name=?,species=?,breed=?,age=?,sex=?,owner=?,consultation_date=?,reason=?,diagnosis=?,treatment=?,vaccinations=?,allergies=?,weight=?,observations=?,vaccination_status=?,id_local=? WHERE id=?"
        : "INSERT INTO medical_records(animal_id,name,species,breed,age,sex,owner,consultation_date,reason,diagnosis,treatment,vaccinations,allergies,weight,observations,vaccination_status,id_local) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    for(const auto &s:{r.animal.animalId,r.animal.name,r.animal.species,r.animal.breed}) q.addBindValue(clean(s));
    q.addBindValue(r.animal.age); q.addBindValue(clean(r.animal.sex)); q.addBindValue(clean(r.animal.owner));
    q.addBindValue(r.medical.consultationDate.toString(Qt::ISODate));
    for(const auto &s:{r.medical.reason,r.medical.diagnosis,r.medical.treatment,r.medical.vaccinations,r.medical.allergies}) q.addBindValue(clean(s));
    q.addBindValue(r.medical.weight); q.addBindValue(clean(r.medical.observations)); q.addBindValue(r.medical.vaccinationStatus);
    q.addBindValue(LocalChoice::sqlValue(r.medical.localId));
    if(r.storageId>0) q.addBindValue(QString::number(r.storageId));
    if(!q.exec()) return fail(error,q.lastError().text());
    if(q.numRowsAffected()!=1) return fail(error,"Ce dossier n'existe plus. Actualisez la liste.");
    if(r.storageId==0) r.storageId=OracleConnection::insertedId(q,"medical_records");
    r.medical.recordId=number(r.storageId); if(error) error->clear(); return true;
}
bool remove(qint64 id,QString *error) {
    QSqlQuery q(database()); q.prepare("DELETE FROM medical_records WHERE id=?"); q.addBindValue(QString::number(id));
    if(!q.exec()) return fail(error,q.lastError().text());
    if(q.numRowsAffected()!=1) return fail(error,"Ce dossier n'existe plus.");
    if(error) error->clear();
    return true;
}
bool seedExamples(QString *error) {
    QString loadError; const auto records=all(&loadError);
    if(!loadError.isEmpty()) return fail(error,loadError);
    if(!records.isEmpty()) return true;
    auto db=database(); if(!db.transaction()) return fail(error,db.lastError().text());
    for(auto r:examples()) if(!save(r,error)) { db.rollback(); return false; }
    if(!db.commit()) { db.rollback(); return fail(error,db.lastError().text()); }
    return true;
}
bool importInitial(QString *error) {
    auto db=database();
    if(!db.transaction()) return fail(error,db.lastError().text());
    QSqlQuery q(db);
    auto abort=[&]{const auto message=q.lastError().text();q.finish();db.rollback();return fail(error,message);};
    if(!q.exec("SELECT 1 FROM integration_migrations WHERE name='medical-reference-ui-v1'")) return abort();
    const bool imported=q.next();q.finish();
    if(imported) {db.rollback();return true;}
    if(!q.exec("SELECT COUNT(*) FROM medical_records") || !q.next()) return abort();
    const bool empty=q.value(0).toInt()==0;q.finish();
    if(empty) for(auto record:examples()) {
        if(!save(record,error)){db.rollback();return false;}
    }
    if(!q.exec("INSERT INTO integration_migrations(name) VALUES('medical-reference-ui-v1')")) return abort();
    if(!db.commit()) {db.rollback();return fail(error,db.lastError().text());}
    if(error) error->clear();
    return true;
}
QString reportHtml(const QVector<MedicalRecord> &records,bool demo) {
    auto escape=[](const QString &s) { return Invoice::escape(s).replace("\n","<br>"); };
    QString html="<html><head><meta charset='utf-8'><style>body{font-family:'Segoe UI';color:#294c56;font-size:10pt;}h1{color:#267483;}h2{color:#267483;font-size:15pt;}th{background:#e5f1f3;text-align:left;}td,th{padding:8px;}table{border-collapse:collapse;width:100%;}p{line-height:140%;}</style></head><body><h1>ANIWORLD · Dossiers médicaux</h1>";
    html+="<p>Édition du "+QDate::currentDate().toString("dd/MM/yyyy")+" · "+QString::number(records.size())+" consultation(s)"+(demo?" · DÉMONSTRATION — données fictives":"")+"</p>";
    for(int i=0;i<records.size();++i) {
        const auto &r=records[i];
        html+="<div"+QString(i?" style='page-break-before:always'":"")+"><h2>"+escape(r.animal.name)+" · "+escape(r.medical.recordId)+"</h2><table>";
        auto row=[&](const QString &name,const QString &value){html+="<tr><th width='27%'>"+name+"</th><td>"+escape(value)+"</td></tr>";};
        row("Animal",r.animal.animalId+" · "+r.animal.species+" · "+r.animal.breed);
        row("Âge / sexe",QString::number(r.animal.age)+" ans · "+r.animal.sex); row("Propriétaire",r.animal.owner);
        row("Consultation",r.medical.consultationDate.toString("dd/MM/yyyy")); row("Poids",QString::number(r.medical.weight,'f',2)+" kg");
        row("Statut vaccinal",r.medical.vaccinationStatus); html+="</table>";
        for(const auto &item:QList<QPair<QString,QString>>{{"Motif",r.medical.reason},{"Diagnostic",r.medical.diagnosis},{"Traitement",r.medical.treatment},{"Vaccinations",r.medical.vaccinations},{"Allergies",r.medical.allergies},{"Observations",r.medical.observations}})
            html+="<p><b>"+item.first+"</b><br>"+(item.second.trimmed().isEmpty()?"Non renseigné":escape(item.second))+"</p>";
        html+="</div>";
    }
    return html+"</body></html>";
}
}
