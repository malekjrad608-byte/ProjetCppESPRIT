"""One-time, asserted source edits for the additive Oracle local relationships."""
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def edit(path, changes):
    p=ROOT/path; s=p.read_text(encoding='utf-8')
    for a,b in changes:
        assert a in s, (path,a)
        s=s.replace(a,b)
    p.write_text(s,encoding='utf-8')

edit('src/models/service.h', [('int id = 0;', 'int id = 0;\n    int localId = 0;')])
edit('src/dialogs/servicedialog.h',[('QComboBox *m_status;', 'QComboBox *m_status;\n    QComboBox *m_local;')])
edit('src/dialogs/servicedialog.cpp',[
('#include "servicedialog.h"','#include "servicedialog.h"\n#include "../ui/localchoice.h"'),
('form->addRow("Prestations",m_description);', 'm_local=new QComboBox; m_local->setObjectName("serviceLocal"); LocalChoice::populate(m_local,s.localId);\n    form->addRow("Local",m_local);\n    form->addRow("Prestations",m_description);'),
('m_category->setEnabled(false);','m_local->setEnabled(false); m_category->setEnabled(false);'),
('Service s; s.id=m_id;', 'Service s; s.id=m_id; s.localId=m_local->currentData().toInt();')])
edit('src/database/databasemanager.cpp',[
('#include "oracleconnection.h"','#include "oracleconnection.h"\n#include "../ui/localchoice.h"'),
('const QStringList additions={','const QStringList additions={"id_local INTEGER", '),
('invoiceNumber,invoiceDate FROM services','invoiceNumber,invoiceDate,id_local FROM services'),
('result.append(s);','s.localId=query.value(12).toInt();\n        result.append(s);'),
('price=?,clientName=? WHERE id=?','price=?,clientName=?,id_local=? WHERE id=?'),
('price,clientName) VALUES(?,?,?,?,?,?,?,?,?)','price,clientName,id_local) VALUES(?,?,?,?,?,?,?,?,?,?)'),
('if(update) query.addBindValue(s.id);','query.addBindValue(LocalChoice::sqlValue(s.localId));\n    if(update) query.addBindValue(s.id);')])
edit('src/medical/medicalrecord.h',[('QString recordId;', 'QString recordId;\n    int localId = 0;')])
edit('src/medical/medicalrecorddialog.h',[('QDateEdit *m_dateConsultation;', 'QDateEdit *m_dateConsultation;\n    QComboBox *m_local;')])
edit('src/medical/medicalrecorddialog.cpp',[
('#include "medicalrecorddialog.h"','#include "medicalrecorddialog.h"\n#include "../ui/localchoice.h"'),
('medicalCardLayout->addLayout(medicalGrid);','m_local=new QComboBox; m_local->setObjectName("medicalLocal"); LocalChoice::populate(m_local);\n    medicalGrid->addWidget(new QLabel("Local:"),4,0); medicalGrid->addWidget(m_local,4,1,1,3);\n    medicalCardLayout->addLayout(medicalGrid);'),
('m_editRecordId->setText(record.medical.recordId);','m_editRecordId->setText(record.medical.recordId);\n    LocalChoice::populate(m_local,record.medical.localId);'),
('MedicalInfo medical;', 'MedicalInfo medical;\n    medical.localId=m_local->currentData().toInt();')])
edit('src/medical/medicalstore.cpp',[
('#include "medicalstore.h"','#include "medicalstore.h"\n#include "../ui/localchoice.h"\n#include <QSqlRecord>'),
('if(error) error->clear();\n    return true;\n}\nQVector<MedicalRecord> all', 'if(!db.record("medical_records").contains("id_local") && !q.exec("ALTER TABLE medical_records ADD COLUMN id_local INTEGER")) return fail(error,q.lastError().text());\n    if(error) error->clear();\n    return true;\n}\nQVector<MedicalRecord> all'),
('observations,vaccination_status FROM medical_records','observations,vaccination_status,id_local FROM medical_records'),
('result.append(r);','r.medical.localId=q.value(17).toInt(); result.append(r);'),
('vaccination_status=? WHERE id=?','vaccination_status=?,id_local=? WHERE id=?'),
('observations,vaccination_status) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)','observations,vaccination_status,id_local) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)'),
('if(r.storageId>0) q.addBindValue', 'q.addBindValue(LocalChoice::sqlValue(r.medical.localId));\n    if(r.storageId>0) q.addBindValue')])
edit('src/stock/stockwidget.h',[('QDate expiry;', 'QDate expiry;\n    int localId = 0;')])
edit('src/stock/stockwidget.cpp',[
('#include "../database/oracleconnection.h"','#include "../database/oracleconnection.h"\n#include "../ui/localchoice.h"'),
('form->addRow("Date d\'échéance", expiryEdit);','form->addRow("Date d\'échéance", expiryEdit);\n        localBox=new QComboBox; localBox->setObjectName("stockLocal"); LocalChoice::populate(localBox,source.localId);\n        form->addRow("Local",localBox);'),
('updated.expiry = expiryEdit->date();','updated.expiry = expiryEdit->date();\n        updated.localId=localBox->currentData().toInt();'),
('QComboBox *categoryBox = nullptr;', 'QComboBox *categoryBox = nullptr;\n    QComboBox *localBox = nullptr;'),
('items<<item;', 'item.localId=r[7].toInt(); items<<item;'),
('r.expiry.toString(Qt::ISODate)};', 'r.expiry.toString(Qt::ISODate),LocalChoice::sqlValue(r.localId)};')])
edit('src/models/oraclecollection.cpp',[
('"unit","threshold","expiry"}', '"unit","threshold","expiry","id_local"}'),
('if(!q.exec()) return fail(db,error,q.lastError().text());\n        if(!audit(it.value()', 'if(!q.exec()) return fail(db,error,q.lastError().nativeErrorCode().contains("2292") ? QStringLiteral("Ce local est utilisé par un service, une consultation ou un lot. Réaffectez les éléments ou marquez le local hors service.") : q.lastError().text());\n        if(!audit(it.value()')])
print('Source changes completed')
