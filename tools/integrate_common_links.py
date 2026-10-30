from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def edit(path,changes):
 p=ROOT/path;s=p.read_text(encoding='utf8')
 for a,b in changes:
  assert a in s,(path,a)
  s=s.replace(a,b)
 p.write_text(s,encoding='utf8')
edit('src/models/service.h',[('int localId = 0;','int localId = 0;\n    int employeeId = 0;\n    int clientId = 0;')])
edit('src/database/databasemanager.cpp',[
('const QStringList additions={','const QStringList additions={"id_employe INTEGER", "id_client INTEGER", '),
('invoiceDate,id_local FROM services','invoiceDate,id_local,id_employe,id_client FROM services'),
('s.localId=query.value(12).toInt();','s.localId=query.value(12).toInt(); s.employeeId=query.value(13).toInt(); s.clientId=query.value(14).toInt();'),
('clientName=?,id_local=? WHERE id=?','clientName=?,id_local=?,id_employe=?,id_client=? WHERE id=?'),
('price,clientName,id_local) VALUES(?,?,?,?,?,?,?,?,?,?)','price,clientName,id_local,id_employe,id_client) VALUES(?,?,?,?,?,?,?,?,?,?,?,?)'),
('query.addBindValue(LocalChoice::sqlValue(s.localId));','query.addBindValue(LocalChoice::sqlValue(s.localId));\n    query.addBindValue(LocalChoice::sqlValue(s.employeeId));\n    query.addBindValue(LocalChoice::sqlValue(s.clientId));'),
('if(!query.exec()) { m_error=query.lastError().text(); return false; }','if(!query.exec()) { m_error=query.lastError().nativeErrorCode().contains("2292") ? QStringLiteral("Ce service est lié à une consultation. Conservez-le ou réaffectez la consultation avant suppression.") : query.lastError().text(); return false; }')])
edit('src/dialogs/servicedialog.h',[('QComboBox *m_local;','QComboBox *m_local;\n    QComboBox *m_employee;\n    QComboBox *m_clientChoice;')])
edit('src/dialogs/servicedialog.cpp',[
('#include "../ui/localchoice.h"','#include "../ui/localchoice.h"\n#include "../ui/relationchoices.h"'),
('form->addRow("Service *",m_name); form->addRow("Client *",m_client);',
 '''m_clientChoice=new QComboBox; m_clientChoice->setObjectName("serviceClientChoice");
    RelationChoices::clients(m_clientChoice,s.clientId);
    connect(m_clientChoice,qOverload<int>(&QComboBox::currentIndexChanged),this,[this](int index){
        const bool existing=m_clientChoice->currentData().toInt()>0;
        m_client->setReadOnly(existing);
        if(existing) m_client->setText(m_clientChoice->itemData(index,Qt::UserRole+1).toString());
    });
    if(s.clientId>0) m_client->setReadOnly(true);
    form->addRow("Service *",m_name);
    if(OracleConnection::isOracle()) form->addRow("Client enregistré",m_clientChoice); else m_clientChoice->hide();
    form->addRow("Client *",m_client);'''),
('form->addRow("Responsable",m_responsible); form->addRow("Tarif",m_price);',
 '''m_employee=new QComboBox; m_employee->setObjectName("serviceEmployee"); RelationChoices::employees(m_employee,s.employeeId,s.responsible);
    if(OracleConnection::isOracle()) {form->addRow("Responsable",m_employee);m_responsible->hide();}
    else {form->addRow("Responsable",m_responsible);m_employee->hide();}
    form->addRow("Tarif",m_price);'''),
('m_local->setEnabled(false);','m_employee->setEnabled(false); m_clientChoice->setEnabled(false); m_local->setEnabled(false);'),
('s.clientName=m_client->text().trimmed(); s.responsible=m_responsible->text().trimmed();',
 '''s.clientName=m_client->text().trimmed(); s.clientId=m_clientChoice->currentData().toInt();
    s.employeeId=m_employee->currentData().toInt();
    s.responsible=OracleConnection::isOracle()?m_employee->currentData(Qt::UserRole+1).toString():m_responsible->text().trimmed();''')])
edit('src/medical/medicalrecord.h',[('int localId = 0;','int localId = 0;\n    int serviceId = 0;')])
edit('src/medical/medicalrecorddialog.h',[('QComboBox *m_local;','QComboBox *m_local;\n    QComboBox *m_service;')])
edit('src/medical/medicalrecorddialog.cpp',[
('#include "../ui/localchoice.h"','#include "../ui/localchoice.h"\n#include "../ui/relationchoices.h"'),
('medicalCardLayout->addLayout(medicalGrid);','m_service=new QComboBox; m_service->setObjectName("medicalService"); RelationChoices::services(m_service);\n    medicalGrid->addWidget(new QLabel("Service associé:"),5,0); medicalGrid->addWidget(m_service,5,1,1,3);\n    medicalCardLayout->addLayout(medicalGrid);'),
('LocalChoice::populate(m_local,record.medical.localId);','LocalChoice::populate(m_local,record.medical.localId);\n    RelationChoices::services(m_service,record.medical.serviceId);'),
('medical.localId=m_local->currentData().toInt();','medical.localId=m_local->currentData().toInt(); medical.serviceId=m_service->currentData().toInt();')])
edit('src/medical/medicalstore.cpp',[
('if(error) error->clear();\n    return true;\n}\nQVector<MedicalRecord>', 'if(!db.record("medical_records").contains("id_service") && !q.exec("ALTER TABLE medical_records ADD COLUMN id_service INTEGER")) return fail(error,q.lastError().text());\n    if(error) error->clear();\n    return true;\n}\nQVector<MedicalRecord>'),
('vaccination_status,id_local FROM medical_records','vaccination_status,id_local,id_service FROM medical_records'),
('r.medical.localId=q.value(17).toInt();','r.medical.localId=q.value(17).toInt(); r.medical.serviceId=q.value(18).toInt();'),
('vaccination_status=?,id_local=? WHERE id=?','vaccination_status=?,id_local=?,id_service=? WHERE id=?'),
('vaccination_status,id_local) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)','vaccination_status,id_local,id_service) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)'),
('q.addBindValue(LocalChoice::sqlValue(r.medical.localId));','q.addBindValue(LocalChoice::sqlValue(r.medical.localId));\n    q.addBindValue(LocalChoice::sqlValue(r.medical.serviceId));')])
edit('src/employees/gestionemployes.cpp',[
('QMessageBox::warning(this, "Erreur", " " + qDel.lastError().text());','QMessageBox::warning(this, "Employé conservé", qDel.lastError().nativeErrorCode().contains("2292") ? QStringLiteral("Cet employé possède un planning ou des services associés. Son historique est conservé. Vous pouvez le rendre indisponible.") : qDel.lastError().text());')])
print('Common links applied to Qt source')
