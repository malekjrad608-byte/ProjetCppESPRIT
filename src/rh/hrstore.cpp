#include "hrstore.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QUuid>
#include <QRegularExpression>
#include <QDateTime>
#include <QTimeZone>
#include <QSignalBlocker>

namespace AniRh {

HrStore::HrStore(QObject *parent) : QObject(parent), m_connection(QUuid::createUuid().toString()) {}
HrStore::~HrStore() {
    if(m_db.isValid()) { m_db.close(); m_db = {}; QSqlDatabase::removeDatabase(m_connection); }
}
QString HrStore::today() { return QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone("Africa/Tunis")).date().toString(Qt::ISODate); }
bool HrStore::fail(const QString &message) { m_error = message; return false; }
bool HrStore::exec(const QString &sql, const QVariantList &args) {
    QSqlQuery q(m_db); q.prepare(sql); for(const auto &a : args) q.addBindValue(a);
    if(!q.exec()) return fail(q.lastError().text());
    return true;
}
Records HrStore::rows(const QString &sql, const QVariantList &args) const {
    Records result; QSqlQuery q(m_db); q.prepare(sql); for(const auto &a : args) q.addBindValue(a);
    if(!q.exec()) { m_error=q.lastError().text(); return result; }
    while(q.next()) { Record r; for(int i=0;i<q.record().count();++i) r[q.record().fieldName(i)]=q.value(i); result.append(r); }
    return result;
}
Record HrStore::row(const QString &sql,const QVariantList &args) const { return rows(sql,args).value(0); }
qint64 HrStore::scalar(const QString &sql,const QVariantList &args) const {
    QSqlQuery q(m_db); q.prepare(sql); for(const auto &a:args) q.addBindValue(a);
    if(!q.exec()) { m_error=q.lastError().text(); return 0; } return q.next()?q.value(0).toLongLong():0;
}
bool HrStore::open(const QString &path) {
    m_path=path; m_db=QSqlDatabase::addDatabase("QSQLITE",m_connection); m_db.setDatabaseName(path);
    if(!m_db.open()) return fail(m_db.lastError().text());
    if(!exec("PRAGMA foreign_keys=ON") || !exec("PRAGMA busy_timeout=5000")) return false;
    if(scalar("PRAGMA user_version")>2) return fail("Cette base nécessite une version plus récente de l'application.");
    if(!m_db.transaction()) return fail(m_db.lastError().text());
    const QStringList schema={
        "CREATE TABLE IF NOT EXISTS employees (id INTEGER PRIMARY KEY AUTOINCREMENT, first_name TEXT NOT NULL, last_name TEXT NOT NULL, email TEXT NOT NULL COLLATE NOCASE UNIQUE, phone TEXT NOT NULL DEFAULT '', role TEXT NOT NULL, department TEXT NOT NULL, contract TEXT NOT NULL, hired TEXT NOT NULL, salary INTEGER NOT NULL CHECK(salary>=0), allowance INTEGER NOT NULL CHECK(allowance>=0), active INTEGER NOT NULL DEFAULT 1 CHECK(active IN(0,1)))",
        "CREATE TABLE IF NOT EXISTS attendance (id INTEGER PRIMARY KEY AUTOINCREMENT, employee_id INTEGER NOT NULL REFERENCES employees(id), day TEXT NOT NULL, status TEXT NOT NULL CHECK(status IN('Présent','Retard','Absent','Télétravail')), arrival TEXT NOT NULL DEFAULT '', departure TEXT NOT NULL DEFAULT '', notes TEXT NOT NULL DEFAULT '', UNIQUE(employee_id,day))",
        "CREATE TABLE IF NOT EXISTS leaves (id INTEGER PRIMARY KEY AUTOINCREMENT, employee_id INTEGER NOT NULL REFERENCES employees(id), type TEXT NOT NULL, start TEXT NOT NULL, end TEXT NOT NULL, days INTEGER NOT NULL CHECK(days>0), reason TEXT NOT NULL DEFAULT '', status TEXT NOT NULL DEFAULT 'En attente' CHECK(status IN('En attente','Approuvé','Refusé')), created TEXT NOT NULL)",
        "CREATE TABLE IF NOT EXISTS tasks (id INTEGER PRIMARY KEY AUTOINCREMENT, employee_id INTEGER NOT NULL REFERENCES employees(id), title TEXT NOT NULL, description TEXT NOT NULL DEFAULT '', due TEXT NOT NULL, priority TEXT NOT NULL, status TEXT NOT NULL)",
        "CREATE TABLE IF NOT EXISTS reviews (id INTEGER PRIMARY KEY AUTOINCREMENT, employee_id INTEGER NOT NULL REFERENCES employees(id), day TEXT NOT NULL, reviewer TEXT NOT NULL, quality INTEGER NOT NULL CHECK(quality BETWEEN 1 AND 5), teamwork INTEGER NOT NULL CHECK(teamwork BETWEEN 1 AND 5), punctuality INTEGER NOT NULL CHECK(punctuality BETWEEN 1 AND 5), initiative INTEGER NOT NULL CHECK(initiative BETWEEN 1 AND 5), feedback TEXT NOT NULL DEFAULT '', goals TEXT NOT NULL DEFAULT '')",
        "CREATE TABLE IF NOT EXISTS payroll (id INTEGER PRIMARY KEY AUTOINCREMENT, employee_id INTEGER NOT NULL REFERENCES employees(id), month TEXT NOT NULL, employee_name TEXT NOT NULL, employee_role TEXT NOT NULL, base INTEGER NOT NULL CHECK(base>=0), bonus INTEGER NOT NULL DEFAULT 0 CHECK(bonus>=0), deductions INTEGER NOT NULL DEFAULT 0 CHECK(deductions>=0), notes TEXT NOT NULL DEFAULT '', status TEXT NOT NULL DEFAULT 'Brouillon' CHECK(status IN('Brouillon','Payé')), paid_at TEXT NOT NULL DEFAULT '', UNIQUE(employee_id,month), CHECK(base+bonus>=deductions))",
        "CREATE INDEX IF NOT EXISTS leaves_dates ON leaves(employee_id,start,end)",
        "CREATE INDEX IF NOT EXISTS attendance_day ON attendance(day)",
        "CREATE TABLE IF NOT EXISTS shifts (id INTEGER PRIMARY KEY AUTOINCREMENT, employee_id INTEGER NOT NULL REFERENCES employees(id), day TEXT NOT NULL, start TEXT NOT NULL, end TEXT NOT NULL, assignment TEXT NOT NULL, notes TEXT NOT NULL DEFAULT '')",
        "CREATE INDEX IF NOT EXISTS shifts_day ON shifts(employee_id,day)",
        "PRAGMA user_version=2"
    };
    for(const auto &sql:schema) if(!exec(sql)) { m_db.rollback(); return false; }
    if(!m_db.commit()) return fail(m_db.lastError().text());
    return true;
}
bool HrStore::save(const QString &table, const Record &data,int id) {
    QStringList fields=data.keys(), placeholders, assignments; QVariantList values;
    for(const auto &f:fields) { values.append(data[f]); placeholders.append("?"); assignments.append(f+"=?"); }
    QString sql;
    if(id) {
        if(!scalar("SELECT COUNT(*) FROM "+table+" WHERE id=?",{id})) return fail("Enregistrement introuvable.");
        sql="UPDATE "+table+" SET "+assignments.join(',')+" WHERE id=?"; values.append(id);
    } else sql="INSERT INTO "+table+" ("+fields.join(',')+") VALUES ("+placeholders.join(',')+")";
    if(!exec(sql,values)) return false;
    emit changed(); return true;
}
Records HrStore::employees(bool activeOnly) const { return rows("SELECT *,first_name||' '||last_name AS name FROM employees "+QString(activeOnly?"WHERE active=1 ":"")+"ORDER BY last_name,first_name"); }
bool HrStore::validateEmployeeDate(int employee,const QDate &date,bool requireActive) {
    const auto e=row("SELECT * FROM employees WHERE id=?",{employee});
    if(e.isEmpty()) return fail("Choisissez un employé existant.");
    if(requireActive && !e["active"].toBool()) return fail("Cet employé est archivé. Réactivez son dossier pour continuer.");
    if(!date.isValid() || date<QDate::fromString(e["hired"].toString(),Qt::ISODate)) return fail("La date doit être valide et postérieure ou égale à l'embauche.");
    return true;
}
bool HrStore::saveEmployee(Record d,int id) {
    Record clean;
    for(const QString &key:QStringList{"first_name","last_name","email","phone","role","department","contract","hired","salary","allowance"}) clean[key]=d.value(key);
    for(const QString &key:QStringList{"first_name","last_name","email","phone","role","department"}) clean[key]=clean[key].toString().trimmed();
    for(const QString &key:QStringList{"first_name","last_name","role","department"})
        if(clean[key].toString().isEmpty() || clean[key].toString().size()>100) return fail("Renseignez le prénom, le nom, le poste et le département (100 caractères maximum).");
    if(!QRegularExpression("^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$").match(clean["email"].toString()).hasMatch()) return fail("Saisissez une adresse email valide.");
    if(scalar("SELECT COUNT(*) FROM employees WHERE email=? AND id<>?",{clean["email"],id})) return fail("Cette adresse email est déjà utilisée, y compris dans les dossiers archivés.");
    const QDate hired=QDate::fromString(clean["hired"].toString(),Qt::ISODate);
    if(!hired.isValid() || clean["salary"].toLongLong()<0 || clean["salary"].toLongLong()>1000000000 || clean["allowance"].toInt()<0 || clean["allowance"].toInt()>366) return fail("Vérifiez la date d'embauche, le salaire et le quota de congés.");
    if(!QStringList{"CDI","CDD","Stage","Temps partiel"}.contains(clean["contract"].toString())) return fail("Choisissez un contrat valide.");
    if(id) {
        if(!scalar("SELECT COUNT(*) FROM employees WHERE id=?",{id})) return fail("Dossier introuvable.");
        if(scalar("SELECT COUNT(*) FROM (SELECT day AS d FROM attendance WHERE employee_id=? UNION ALL SELECT start AS d FROM leaves WHERE employee_id=? UNION ALL SELECT day AS d FROM reviews WHERE employee_id=? UNION ALL SELECT day AS d FROM shifts WHERE employee_id=? UNION ALL SELECT due AS d FROM tasks WHERE employee_id=?) WHERE d<?",{id,id,id,id,id,clean["hired"]})) return fail("Des enregistrements existent avant cette nouvelle date d'embauche.");
        const auto years=rows("SELECT DISTINCT substr(start,1,4) AS year FROM leaves WHERE employee_id=? AND type='Congé annuel' AND status='Approuvé'",{id});
        for(const auto &year:years) if(usedLeave(id,year["year"].toInt())>clean["allowance"].toInt()) return fail("Le quota ne peut pas être inférieur aux congés annuels déjà approuvés.");
    }
    return save("employees",clean,id);
}
bool HrStore::setEmployeeActive(int id,bool active) {
    if(!scalar("SELECT COUNT(*) FROM employees WHERE id=?",{id})) return fail("Dossier introuvable.");
    if(!exec("UPDATE employees SET active=? WHERE id=?",{active?1:0,id})) return false;
    emit changed(); return true;
}
bool HrStore::saveAttendance(Record d,int id) {
    const int employee=d["employee_id"].toInt(); const QString day=d["day"].toString(), status=d["status"].toString();
    if(!validateEmployeeDate(employee,QDate::fromString(day,Qt::ISODate),!id)) return false;
    if(day>today()) return fail("Le pointage ne peut pas être enregistré dans le futur.");
    if(!QStringList{"Présent","Retard","Absent","Télétravail"}.contains(status)) return fail("Statut de présence invalide.");
    if(scalar("SELECT COUNT(*) FROM attendance WHERE employee_id=? AND day=? AND id<>?",{employee,day,id})) return fail("Un pointage existe déjà pour cet employé à cette date. Modifiez-le depuis la liste.");
    if(scalar("SELECT COUNT(*) FROM leaves WHERE employee_id=? AND status='Approuvé' AND start<=? AND end>=?",{employee,day,day})) return fail("Un congé approuvé couvre cette date. Le statut est déjà « En congé ».");
    if(status=="Absent") { d["arrival"]=""; d["departure"]=""; }
    else {
        const QTime arrival=QTime::fromString(d["arrival"].toString(),"HH:mm"), departure=QTime::fromString(d["departure"].toString(),"HH:mm");
        if(!arrival.isValid()) return fail("Renseignez une heure d'arrivée valide.");
        if(!d["departure"].toString().isEmpty() && (!departure.isValid() || departure<=arrival)) return fail("Le départ doit être après l'arrivée (pointage sur une même journée).");
    }
    Record clean; for(const QString &key:QStringList{"employee_id","day","status","arrival","departure","notes"}) clean[key]=d.value(key,QString());
    return save("attendance",clean,id);
}
int HrStore::usedLeave(int employee,int year) const { return int(scalar("SELECT COALESCE(SUM(days),0) FROM leaves WHERE employee_id=? AND status='Approuvé' AND type='Congé annuel' AND substr(start,1,4)=?",{employee,QString::number(year)})); }
bool HrStore::saveLeave(Record d) {
    const int employee=d["employee_id"].toInt(); const QDate start=QDate::fromString(d["start"].toString(),Qt::ISODate),end=QDate::fromString(d["end"].toString(),Qt::ISODate);
    if(!validateEmployeeDate(employee,start)) return false;
    if(!end.isValid() || end<start) return fail("La fin du congé doit être postérieure ou égale au début.");
    if(start.year()!=end.year()) return fail("Créez une demande par année civile pour un congé à cheval sur deux années.");
    if(!QStringList{"Congé annuel","Maladie","Exceptionnel","Sans solde"}.contains(d["type"].toString())) return fail("Type de congé invalide.");
    if(scalar("SELECT COUNT(*) FROM leaves WHERE employee_id=? AND status<>'Refusé' AND start<=? AND end>=?",{employee,d["end"],d["start"]})) return fail("Une demande en attente ou approuvée couvre déjà une partie de cette période.");
    d["days"]=int(start.daysTo(end)+1); d["status"]="En attente"; d["created"]=today();
    if(d["type"]=="Congé annuel" && usedLeave(employee,start.year())+d["days"].toInt()>scalar("SELECT allowance FROM employees WHERE id=?",{employee})) return fail("Le solde annuel est insuffisant (décompte en jours calendaires).");
    Record clean; for(const QString &key:QStringList{"employee_id","type","start","end","days","reason","status","created"}) clean[key]=d.value(key,QString());
    return save("leaves",clean,0);
}
bool HrStore::decideLeave(int id,bool approve) {
    if(!m_db.transaction()) return fail(m_db.lastError().text());
    const auto d=row("SELECT * FROM leaves WHERE id=?",{id});
    auto abort=[this](const QString &message){ m_db.rollback(); return fail(message); };
    if(d.isEmpty() || d["status"]!="En attente") return abort("Seules les demandes en attente peuvent être traitées.");
    if(approve) {
        const int employee=d["employee_id"].toInt();
        if(!validateEmployeeDate(employee,QDate::fromString(d["start"].toString(),Qt::ISODate))) { m_db.rollback(); return false; }
        if(d["type"]=="Congé annuel" && usedLeave(employee,QDate::fromString(d["start"].toString(),Qt::ISODate).year())+d["days"].toInt()>scalar("SELECT allowance FROM employees WHERE id=?",{employee})) return abort("Le solde annuel est insuffisant après les dernières approbations.");
        if(scalar("SELECT COUNT(*) FROM attendance WHERE employee_id=? AND day BETWEEN ? AND ?",{employee,d["start"],d["end"]})) return abort("Des pointages existent sur cette période. Corrigez-les avant d'approuver le congé.");
        if(scalar("SELECT COUNT(*) FROM shifts WHERE employee_id=? AND day BETWEEN ? AND ?",{employee,d["start"],d["end"]})) return abort("Des créneaux de travail sont planifiés sur cette période. Réaffectez-les ou supprimez-les avant d'approuver le congé.");
    }
    if(!exec("UPDATE leaves SET status=? WHERE id=? AND status='En attente'",{approve?"Approuvé":"Refusé",id})) { m_db.rollback(); return false; }
    if(!m_db.commit()) return fail(m_db.lastError().text());
    emit changed(); return true;
}
bool HrStore::saveTask(Record d,int id) {
    if(!validateEmployeeDate(d["employee_id"].toInt(),QDate::fromString(d["due"].toString(),Qt::ISODate),!id)) return false;
    d["title"]=d["title"].toString().trimmed();
    if(d["title"].toString().isEmpty() || d["title"].toString().size()>150) return fail("Renseignez un titre (150 caractères maximum).");
    if(!QStringList{"À faire","En cours","Terminé"}.contains(d["status"].toString()) || !QStringList{"Basse","Normale","Haute"}.contains(d["priority"].toString())) return fail("Choisissez un statut et une priorité valides.");
    Record clean; for(const QString &key:QStringList{"employee_id","title","description","due","priority","status"}) clean[key]=d.value(key,QString());
    return save("tasks",clean,id);
}
bool HrStore::saveReview(Record d,int id) {
    if(!validateEmployeeDate(d["employee_id"].toInt(),QDate::fromString(d["day"].toString(),Qt::ISODate),!id)) return false;
    if(d["day"].toString()>today()) return fail("La date d'évaluation ne peut pas être dans le futur.");
    d["reviewer"]=d["reviewer"].toString().trimmed();
    if(d["reviewer"].toString().isEmpty()) return fail("Renseignez le nom de l'évaluateur.");
    for(const QString &key:QStringList{"quality","teamwork","punctuality","initiative"}) if(d[key].toInt()<1 || d[key].toInt()>5) return fail("Chaque note doit être comprise entre 1 et 5.");
    Record clean; for(const QString &key:QStringList{"employee_id","day","reviewer","quality","teamwork","punctuality","initiative","feedback","goals"}) clean[key]=d.value(key,QString());
    return save("reviews",clean,id);
}
bool HrStore::removeRecord(const QString &entity,int id) {
    if(!QStringList{"attendance","leaves","tasks","reviews","shifts"}.contains(entity)) return fail("Suppression non autorisée.");
    if(!scalar("SELECT COUNT(*) FROM "+entity+" WHERE id=?",{id})) return fail("Enregistrement introuvable.");
    if(entity=="leaves" && row("SELECT status FROM leaves WHERE id=?",{id})["status"]=="Approuvé") return fail("Un congé approuvé est conservé dans l'historique.");
    if(!exec("DELETE FROM "+entity+" WHERE id=?",{id})) return false;
    emit changed(); return true;
}
bool HrStore::preparePayroll(const QString &month) {
    const QDate period=QDate::fromString(month+"-01",Qt::ISODate);
    if(!period.isValid() || period.toString("yyyy-MM")!=month) return fail("Mois de paie invalide.");
    if(!m_db.transaction()) return fail(m_db.lastError().text());
    const bool ok=exec("INSERT INTO payroll(employee_id,month,employee_name,employee_role,base) SELECT id,?,first_name||' '||last_name,role,salary FROM employees WHERE active=1 AND hired<=? AND NOT EXISTS(SELECT 1 FROM payroll p WHERE p.employee_id=employees.id AND p.month=?)",{month,period.addMonths(1).addDays(-1).toString(Qt::ISODate),month});
    if(!ok) { m_db.rollback(); return false; }
    if(!m_db.commit()) return fail(m_db.lastError().text());
    emit changed(); return true;
}
bool HrStore::updatePayroll(int id,qint64 bonus,qint64 deductions,const QString &notes) {
    const auto p=row("SELECT * FROM payroll WHERE id=?",{id});
    if(p.isEmpty() || p["status"]!="Brouillon") return fail("Seuls les bulletins en brouillon sont modifiables.");
    if(bonus<0 || deductions<0 || bonus>1000000000 || deductions>1000000000 || p["base"].toLongLong()+bonus<deductions) return fail("Les montants doivent être positifs et les retenues ne peuvent pas dépasser le total brut.");
    if(!exec("UPDATE payroll SET bonus=?,deductions=?,notes=? WHERE id=? AND status='Brouillon'",{bonus,deductions,notes,id})) return false;
    emit changed(); return true;
}
bool HrStore::markPaid(int id) {
    if(row("SELECT status FROM payroll WHERE id=?",{id})["status"]!="Brouillon") return fail("Ce bulletin n'est plus en brouillon.");
    if(!exec("UPDATE payroll SET status='Payé',paid_at=? WHERE id=? AND status='Brouillon'",{today(),id})) return false;
    emit changed(); return true;
}
bool HrStore::saveShift(Record d,int id) {
    const int employee=d["employee_id"].toInt();const QString day=d["day"].toString();
    const bool reassigned=id && row("SELECT employee_id FROM shifts WHERE id=?",{id})["employee_id"].toInt()!=employee;
    if(!validateEmployeeDate(employee,QDate::fromString(day,Qt::ISODate),!id || reassigned))return false;
    const QTime start=QTime::fromString(d["start"].toString(),"HH:mm"),end=QTime::fromString(d["end"].toString(),"HH:mm");
    if(!start.isValid() || !end.isValid() || end<=start)return fail("Le créneau doit commencer et finir le même jour, avec une fin après le début.");
    d["assignment"]=d["assignment"].toString().trimmed();
    if(d["assignment"].toString().isEmpty() || d["assignment"].toString().size()>100)return fail("Renseignez une affectation (100 caractères maximum).");
    if(scalar("SELECT COUNT(*) FROM shifts WHERE employee_id=? AND day=? AND id<>? AND start<? AND end>?",{employee,day,id,d["end"],d["start"]}))return fail("Ce créneau chevauche un autre créneau du même employé.");
    if(scalar("SELECT COUNT(*) FROM leaves WHERE employee_id=? AND status='Approuvé' AND start<=? AND end>=?",{employee,day,day}))return fail("Cet employé est en congé approuvé à cette date.");
    Record clean;for(const QString &key:QStringList{"employee_id","day","start","end","assignment","notes"})clean[key]=d.value(key,QString());
    return save("shifts",clean,id);
}
bool HrStore::copyScheduleWeek(const QDate &monday) {
    if(!monday.isValid() || monday.dayOfWeek()!=1)return fail("Choisissez une semaine valide.");
    const auto source=rows("SELECT * FROM shifts WHERE day BETWEEN ? AND ? ORDER BY day,start",{monday.toString(Qt::ISODate),monday.addDays(6).toString(Qt::ISODate)});
    if(source.isEmpty())return fail("La semaine affichée ne contient aucun créneau à copier.");
    if(!m_db.transaction())return fail(m_db.lastError().text());
    {
        QSignalBlocker blocker(this);
        for(auto r:source){r["day"]=QDate::fromString(r["day"].toString(),Qt::ISODate).addDays(7).toString(Qt::ISODate);if(!saveShift(r)){m_db.rollback();return false;}}
    }
    if(!m_db.commit())return fail(m_db.lastError().text());
    emit changed();return true;
}
bool HrStore::seedDemo() {
    if(scalar("SELECT COUNT(*) FROM employees")) return true;
    const QDate now=QDate::fromString(today(),Qt::ISODate);
    const QStringList first={"Amira","Youssef","Lina","Mehdi","Sarah","Aziz","Nour","Sami"};
    const QStringList last={"Ben Ali","Trabelsi","Mansour","Gharbi","Kacem","Bouzid","Saïdi","Hamdi"};
    const QStringList roles={"Vétérinaire","Responsable soins","Auxiliaire vétérinaire","Toiletteur","Chargée d'accueil","Soigneur animalier","Responsable RH","Agent de logistique"};
    const QStringList depts={"Soins vétérinaires","Soins vétérinaires","Soins vétérinaires","Bien-être animal","Administration","Bien-être animal","Administration","Logistique"};
    for(int i=0;i<8;++i) {
        if(!saveEmployee({{"first_name",first[i]},{"last_name",last[i]},{"email",first[i].toLower()+"@example.com"},{"phone","+216 00 000 000"},{"role",roles[i]},{"department",depts[i]},{"contract",i==7?"CDD":"CDI"},{"hired",now.addYears(-2).toString(Qt::ISODate)},{"salary",(1800+i*230)*1000},{"allowance",30}})) return false;
    }
    for(int offset=6;offset>=0;--offset) for(int employee=1;employee<=8;++employee) {
        if(offset==0 && employee>6) continue;
        if(!saveAttendance({{"employee_id",employee},{"day",now.addDays(-offset).toString(Qt::ISODate)},{"status",employee==3?"Retard":(employee==6 && offset==0?"Absent":"Présent")},{"arrival",employee==3?"09:20":"08:30"},{"departure",offset?"17:30":""},{"notes",""}})) return false;
    }
    // Keep demo requests in a single calendar year, including late December.
    const QDate future=now.month()==12 && now.day()>18?QDate(now.year()+1,1,3):now.addDays(3);
    for(int i=0;i<3;++i) if(!saveLeave({{"employee_id",i+1},{"type","Congé annuel"},{"start",future.addDays(i*3).toString(Qt::ISODate)},{"end",future.addDays(1+i*3).toString(Qt::ISODate)},{"reason","Congé personnel"}})) return false;
    if(!decideLeave(3,true)) return false;
    const QStringList titles={"Organiser le planning de soins","Préparer les dossiers du personnel","Accueillir les nouveaux arrivants","Vérifier le matériel de la clinique"};
    for(int i=0;i<4;++i) if(!saveTask({{"employee_id",i+1},{"title",titles[i]},{"description","Coordonner avec l'équipe et faire un point en fin de journée."},{"due",now.addDays(i-1).toString(Qt::ISODate)},{"priority",i==0?"Haute":"Normale"},{"status",i==2?"Terminé":(i==0?"En cours":"À faire")}})) return false;
    for(int i=1;i<=4;++i) if(!saveReview({{"employee_id",i},{"day",now.addDays(-i).toString(Qt::ISODate)},{"reviewer","Nour Saïdi"},{"quality",5},{"teamwork",4},{"punctuality",i==3?3:4},{"initiative",4},{"feedback","Bonne implication et attention portée aux animaux."},{"goals","Poursuivre la formation et partager les bonnes pratiques."}})) return false;
    const QDate monday=now.addDays(1-now.dayOfWeek());
    for(int employee=1;employee<=8;++employee)for(int i=0;i<5;++i){
        const QString day=monday.addDays(i).toString(Qt::ISODate);
        if(scalar("SELECT COUNT(*) FROM leaves WHERE employee_id=? AND status='Approuvé' AND start<=? AND end>=?",{employee,day,day}))continue;
        if(!saveShift({{"employee_id",employee},{"day",day},{"start",employee%2?"08:30":"09:00"},{"end",employee%2?"16:30":"17:00"},{"assignment",roles[employee-1]},{"notes","Pause à organiser avec le responsable."}}))return false;
    }
    return preparePayroll(now.toString("yyyy-MM"));
}

} // namespace AniRh
