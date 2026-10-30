from pathlib import Path
root=Path(__file__).resolve().parents[1]
def edit(path,old,new):
 p=root/path;s=p.read_text(encoding='utf8');assert old in s,(path,old[:70]);p.write_text(s.replace(old,new),encoding='utf8')

# Oracle has exactly five business tables. SQLite demo keeps its separate format.
edit('src/database/oracleconnection.cpp','    if(!q.exec("SELECT version FROM schema_version WHERE id=1")', '    if(!q.exec("SELECT version FROM schema_version WHERE id=1")')
p=root/'src/database/oracleconnection.cpp';s=p.read_text(encoding='utf8');a=s.index('    if(!q.exec("SELECT version');b=s.index('    error->clear();return true;',a)
s=s[:a]+'''    if(!q.exec("SELECT COUNT(*) FROM user_tables") || !q.next() || q.value(0).toInt()!=5) {
        *error="Le schéma Oracle doit contenir exactement les cinq tables métier.";db.close();return false;
    }
    if(!q.exec("SELECT COUNT(*) FROM user_tab_columns WHERE (table_name IN ('EMPLOYES','LOCAUX') AND column_name='ID_SERVICE') OR (table_name='MEDICAL_RECORDS' AND column_name='ID_EMPLOYE')") || !q.next() || q.value(0).toInt()!=3) {
        *error="Migration vers le modèle à cinq tables nécessaire.";db.close();return false;
    }
'''+s[b:];a=s.index('bool migrateLocal(');s=s[:a]+'''bool migrateLocal(const QString &,QString *error) {
    *error="L’import de l’ancien modèle est désactivé dans le schéma à cinq tables.";return false;
}
}
''';p.write_text(s,encoding='utf8')
p=root/'src/main.cpp';s=p.read_text(encoding='utf8').replace(',QString("accounts")','');a=s.index('    if(oracle) {\n        QSqlQuery');b=s.index('    if(!oracle &&',a);s=s[:a]+s[b:];p.write_text(s,encoding='utf8')

edit('src/medical/medicalstore.cpp','if(!q.exec("SELECT id,animal_id,name,species,breed,age,sex,owner,consultation_date,reason,diagnosis,treatment,vaccinations,allergies,weight,observations,vaccination_status,id_local,id_service FROM medical_records ORDER BY consultation_date DESC,id DESC"))',
'if(!q.exec(OracleConnection::isOracle()?"SELECT id,animal_id,name,species,breed,age,sex,owner,consultation_date,reason,diagnosis,treatment,vaccinations,allergies,weight,observations,vaccination_status,0,0,id_employe FROM medical_records ORDER BY consultation_date DESC,id DESC":"SELECT id,animal_id,name,species,breed,age,sex,owner,consultation_date,reason,diagnosis,treatment,vaccinations,allergies,weight,observations,vaccination_status,id_local,id_service,0 FROM medical_records ORDER BY consultation_date DESC,id DESC"))')
edit('src/medical/medicalstore.cpp','r.medical.serviceId=q.value(18).toInt(); result.append(r);','r.medical.serviceId=q.value(18).toInt(); r.medical.employeeId=q.value(19).toInt(); result.append(r);')
edit('src/medical/medicalstore.cpp','    QSqlQuery q(database());\n    q.prepare(r.storageId>0', '''    const bool oracle=OracleConnection::isOracle();
    if(oracle && r.medical.employeeId<=0)return fail(error,"Choisissez l’employé responsable du dossier.");
    QSqlQuery q(database());
    if(oracle)q.prepare(r.storageId>0
        ? "UPDATE medical_records SET animal_id=?,name=?,species=?,breed=?,age=?,sex=?,owner=?,consultation_date=?,reason=?,diagnosis=?,treatment=?,vaccinations=?,allergies=?,weight=?,observations=?,vaccination_status=?,id_employe=? WHERE id=?"
        : "INSERT INTO medical_records(animal_id,name,species,breed,age,sex,owner,consultation_date,reason,diagnosis,treatment,vaccinations,allergies,weight,observations,vaccination_status,id_employe) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    else q.prepare(r.storageId>0''')
edit('src/medical/medicalstore.cpp','    q.addBindValue(LocalChoice::sqlValue(r.medical.localId));\n    q.addBindValue(LocalChoice::sqlValue(r.medical.serviceId));', '''    if(oracle)q.addBindValue(r.medical.employeeId);
    else {q.addBindValue(LocalChoice::sqlValue(r.medical.localId));q.addBindValue(LocalChoice::sqlValue(r.medical.serviceId));}''')
edit('src/locaux/localrepository.cpp','l.dirtySince=QDateTime::fromString(r[10].toString(),Qt::ISODate);m_items<<l;', 'l.dirtySince=QDateTime::fromString(r[10].toString(),Qt::ISODate);l.serviceId=r[11].toInt();m_items<<l;')
edit('src/locaux/localrepository.cpp','l.dirtySince.toString(Qt::ISODate)};', 'l.dirtySince.toString(Qt::ISODate),l.serviceId};')

# Flatten the existing single-line invoice into its service; no invoice tables.
p=root/'src/ui/invoice.cpp';s=p.read_text(encoding='utf8');a=s.index('    q.prepare("SELECT f.service_reference');b=s.index('    q.addBindValue(source.id);',a)
s=s[:a]+'''    q.prepare("SELECT id,invoiceNumber,COALESCE(invoice_emitted_at,invoiceDate),clientName,name,category,responsible,datetime,duration,price,description,invoice_status FROM services WHERE id=?");
'''+s[b:];a=s.index('q.prepare("UPDATE factures SET');b=s.index('\n        q.addBindValue',a);s=s[:a]+'''q.prepare("UPDATE services SET invoice_status='EMISE',invoice_emitted_at=? WHERE id=? AND invoice_status='BROUILLON'");'''+s[b:];p.write_text(s,encoding='utf8')

# Employee form carries the mandatory service reference; central persistence handles moves.
edit('src/employees/gestionemployes.cpp','#include "../database/oracleconnection.h"','#include "../database/oracleconnection.h"\n#include "../database/fivetablestore.h"\n#include "../ui/relationchoices.h"\n#include "../dialogs/serviceplanningdialog.h"')
edit('src/employees/gestionemployes.cpp','    form->addRow("Poste", comboPoste);','''    form->addRow("Poste", comboPoste);
    auto *serviceChoice=new QComboBox;serviceChoice->setObjectName("employeeService");
    RelationChoices::services(serviceChoice,OracleConnection::isOracle()?FiveTableStore::employeeService(e.getId()):0);
    if(OracleConnection::isOracle())form->addRow("Service *",serviceChoice);else serviceChoice->hide();''')
edit('src/employees/gestionemployes.cpp','        dlg.accept();','''        if(OracleConnection::isOracle() && serviceChoice->currentData().toInt()<=0){validation->setText("Choisissez un service. Pour le premier service, créez son premier employé depuis Services.");return;}
        dlg.accept();''')
edit('src/employees/gestionemployes.cpp','    QSqlQuery q(QSqlDatabase::database("aniworld-services"));\n    if (modification) {','''    if(OracleConnection::isOracle()) {
        QString error;
        if(!FiveTableStore::saveEmployee(modification?e.getId():0,serviceChoice->currentData().toInt(),
          {editNom->text().trimmed(),editPrenom->text().trimmed(),editCourriel->text().trimmed(),editTel->text().trimmed(),comboPoste->currentText(),dateEmb->date().toString(Qt::ISODate),checkDispo->isChecked()?1:0,spinCharge->value(),spinDispo->value(),editComp->text().trimmed()},&error)){
            QMessageBox::warning(this,"Enregistrement impossible",error);return false;
        }
    }else {
    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
    if (modification) {''')
edit('src/employees/gestionemployes.cpp','    e.setNom(editNom->text().trimmed());','    }\n    e.setNom(editNom->text().trimmed());')
edit('src/employees/gestionemployes.cpp','void GestionEmployes::ouvrirPlanification()\n{','''void GestionEmployes::ouvrirPlanification()
{
    if(OracleConnection::isOracle()){ServicePlanningDialog planning(QDate::currentDate(),this);planning.showEmployee(m_idSelectionne>0?m_idSelectionne:0);planning.exec();return;}''')
edit('src/employees/gestionemployes.cpp','QPushButton *btnPlanifier = Design::button("Planifier et affecter","calendar","primary");','QPushButton *btnPlanifier = Design::button(OracleConnection::isOracle()?"Planning des services":"Planifier et affecter","calendar","primary");')
print('Five-table adaptations written')
