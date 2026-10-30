from pathlib import Path
import re, sqlite3, os
root=Path.cwd()
source=Path(r'C:\Users\malek\Downloads\med\med\dossier medical R')
dest=root/'src/medical'
db=Path(os.environ['LOCALAPPDATA'])/'ANIWORLD/ANIWORLD/services.sqlite'
if db.exists():
    with sqlite3.connect(db) as src, sqlite3.connect(root/'artifacts/medical-integration/before/services.sqlite') as out: src.backup(out)
def write(p,s): p.write_text(s,encoding='utf-8')
def edit(p,a,b):
    s=p.read_text(encoding='utf-8'); assert a in s,(p,a); write(p,s.replace(a,b))
for f in ['medicalrecord','medicalrecorddialog','medicalrecorddetailsdialog','recordcard']:
    for ext in ['h','cpp']: write(dest/f'{f}.{ext}',(source/f'{f}.{ext}').read_text(encoding='utf-8-sig'))
edit(dest/'medicalrecord.h','    AnimalInfo animal;','    qint64 storageId = 0;\n    AnimalInfo animal;')
edit(dest/'medicalrecord.h','    int age;','    int age = 0;')
edit(dest/'medicalrecord.h','    double weight;','    double weight = 0;')
edit(dest/'recordcard.h','#include <QFrame>','#include "../widgets/animatedpanel.h"')
edit(dest/'recordcard.h','public QFrame','public AnimatedPanel')
edit(dest/'recordcard.cpp',': QFrame(parent)',': AnimatedPanel(parent)')
edit(dest/'recordcard.cpp','#include "recordcard.h"','#include "recordcard.h"\n#include "../ui/design.h"')
edit(dest/'recordcard.cpp','QPixmap(":/icons/paw.svg").scaled(22, 22, Qt::KeepAspectRatio, Qt::SmoothTransformation)','Design::icon("paw").pixmap(22,22)')
edit(dest/'recordcard.cpp','    for (QLabel *l : findChildren<QLabel *>())\n        l->setAttribute(Qt::WA_TransparentForMouseEvents);','''    for (QLabel *l : findChildren<QLabel *>()) {
        l->setTextFormat(Qt::PlainText);
        l->setAttribute(Qt::WA_TransparentForMouseEvents);
        l->setToolTip(l->text());
    }
    for(auto *l : {title, sub, owner}) l->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);''')
for f in ['medicalrecorddialog','medicalrecorddetailsdialog']:
    edit(dest/f'{f}.h','#include <QDialog>','#include <QDialog>\nclass QPushButton;')
    edit(dest/f'{f}.cpp',f'#include "{f}.h"',f'#include "{f}.h"\n#include "../ui/design.h"\n#include "medicalstore.h"')
    p=dest/f'{f}.cpp';s=p.read_text(encoding='utf8')
    s=re.sub(r'setMinimumSize\(\d+, \d+\);','setMinimumSize(700, 520);',s)
    s=re.sub(r'resize\(\d+, \d+\);','resize(800, 640);',s)
    s=s.replace('    setupUI();','    setObjectName("'+f+'");\n    setupUI();')
    s=s.replace('new QPushButton("Annuler")','Design::button("Annuler")').replace('new QPushButton("Enregistrer")','Design::button("Enregistrer","check","primary")').replace('new QPushButton("Fermer")','Design::button("Fermer",{},"primary")')
    write(p,s)
p=dest/'medicalrecorddialog.cpp'
edit(p,'    m_editRecordId->setPlaceholderText("MED-XXX");','    m_editRecordId->setPlaceholderText("Numéro automatique");\n    m_editRecordId->setReadOnly(true);')
edit(p,'    m_dateConsultation->setCalendarPopup(true);','    m_dateConsultation->setCalendarPopup(true);\n    m_dateConsultation->setDisplayFormat("dd/MM/yyyy");')
s=p.read_text(encoding='utf8')
s=s.replace('->text();','->text().trimmed();').replace('->text().isEmpty()','->text().trimmed().isEmpty()')
s=re.sub(r'    if \(m_editRecordId->text\(\).trimmed\(\).isEmpty\(\)\) \{.*?    \}\n','',s,flags=re.S)
for field in ['Reason','Diagnosis','Treatment','Vaccinations','Allergies','Observations']:
    s=s.replace(f'm_edit{field}->setText(',f'm_edit{field}->setPlainText(')
    s=s.replace(f'    m_edit{field} = new QTextEdit();',f'    m_edit{field} = new QTextEdit();\n    m_edit{field}->setAcceptRichText(false);\n    m_edit{field}->setObjectName("medical{field}");')
for field in ['AnimalId','AnimalName','Breed','Owner','RecordId']:
    s=s.replace(f'    m_edit{field} = new QLineEdit();',f'    m_edit{field} = new QLineEdit();\n    m_edit{field}->setMaxLength(120);\n    m_edit{field}->setObjectName("medical{field}");')
write(p,s)
p=dest/'medicalrecorddetailsdialog.cpp'
s=p.read_text(encoding='utf8');start=s.index('    // Add fictitious previous visits')
end=s.index('\n}\n',start)
s=s[:start]+s[end:]
start=s.index('    // Add current consultation');end=s.index('\n}\n',start)
s=s[:start]+'''    QString error;
    const auto records=MedicalStore::all(&error);
    if(!error.isEmpty()) { m_historyTable->setToolTip(error); return; }
    for(const auto &visit:records) {
        if(visit.animal.animalId!=record.animal.animalId) continue;
        const int row=m_historyTable->rowCount(); m_historyTable->insertRow(row);
        m_historyTable->setItem(row,0,new QTableWidgetItem(visit.medical.consultationDate.toString("dd/MM/yyyy")));
        m_historyTable->setItem(row,1,new QTableWidgetItem(visit.medical.recordId));
        m_historyTable->setItem(row,2,new QTableWidgetItem(visit.medical.reason+" — "+visit.medical.diagnosis));
    }
    m_historyTable->resizeRowsToContents();
'''+s[end:]
s=s.replace('{"Date", "Type", "Détails"}','{"Date", "Dossier", "Consultation"}')
s=s.replace('    m_historyTable->setAlternatingRowColors(true);','    m_historyTable->setAlternatingRowColors(false);\n    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);\n    m_historyTable->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);')
s=s.replace('    mainLayout->addLayout(buttonLayout);','''    mainLayout->addLayout(buttonLayout);
    for(auto *label:findChildren<QLabel *>("detailValue")) {
        label->setTextFormat(Qt::PlainText); label->setWordWrap(true);
        label->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }''')
write(p,s)
# Retain the supplied demonstration dataset only in explicitly requested demo mode.
s=(source/'mainwindow.cpp').read_text(encoding='utf-8-sig')
start=s.index('void MainWindow::loadStaticData()');end=s.index('\nint MainWindow::indexOfRecord',start)
s=s[start:end].replace('void MainWindow::loadStaticData()','QVector<MedicalRecord> MedicalStore::examples()').replace('    auto add = [this]','    QVector<MedicalRecord> records;\n    auto add = [&records]').replace('m_records.append','records.append')
last=s.rfind('}');s=s[:last]+'    return records;\n'+s[last:]
write(dest/'medicalexamples.cpp','#include "medicalstore.h"\n'+s)
# Shared authentication and navigation.
edit(root/'src/auth/accounts.h','Employees, Locaux','Employees, Locaux, Medical')
p=root/'src/auth/accounts.cpp'
edit(p,'    if(role=="locaux")','    if(role=="medical") return Accounts::Role::Medical;\n    if(role=="locaux")')
edit(p,'contains("\'locaux\'")','contains("\'medical\'")')
edit(p,'accounts_before_locaux','accounts_before_medical')
edit(p,"'employes','locaux'","'employes','locaux','medical'")
edit(p,'QString("locaux")})','QString("locaux"),QString("medical")})')
edit(p,'page<=4','page<=5')
edit(p,'(role==Role::Locaux && page==4);','(role==Role::Locaux && page==4) || (role==Role::Medical && page==5);')
edit(p,'return role==Role::Locaux?4:','return role==Role::Medical?5:role==Role::Locaux?4:')
edit(p,'return role==Role::Locaux?"Locaux":','return role==Role::Medical?"Dossiers médicaux":role==Role::Locaux?"Locaux":')
edit(root/'src/database/databasemanager.cpp','#include "../auth/accounts.h"','#include "../auth/accounts.h"\n#include "../medical/medicalstore.h"')
edit(root/'src/database/databasemanager.cpp','!Accounts::initialize(m_db,&m_error)','!Accounts::initialize(m_db,&m_error) || !MedicalStore::initialize(m_db,&m_error)')
edit(root/'src/mainwindow.h','class StockWidget;','class StockWidget;\nclass MedicalWidget;')
edit(root/'src/mainwindow.h','    StockWidget *m_stock;','    StockWidget *m_stock;\n    MedicalWidget *m_medical;')
p=root/'src/mainwindow.cpp'
edit(p,'#include "mainwindow.h"','#include "mainwindow.h"\n#include "medical/medicalwidget.h"')
edit(p,'"Employés","Locaux"','"Employés","Locaux","Dossiers médicaux"')
edit(p,'"users","home"','"users","home","shield"')
edit(p,'"navEmployees","navLocaux"','"navEmployees","navLocaux","navMedical"')
s=p.read_text(encoding='utf8');start=s.index('    sidebar->addSpacing(16); sidebar->addWidget(Design::label("  PROCHAINS MODULES"');end=s.index('    sidebar->addWidget(side->artworkSpace()',start);s=s[:start]+s[end:];write(p,s)
edit(p,'    QPalette pageColors=m_stack->palette();','    m_medical=new MedicalWidget(demo); m_stack->addWidget(m_medical);\n    QPalette pageColors=m_stack->palette();')
edit(p,'else if(m_stack->currentIndex()==4) m_locaux->addLocal();','else if(m_stack->currentIndex()==4) m_locaux->addLocal();\n        else if(m_stack->currentIndex()==5) m_medical->addRecord();')
edit(p,'else if(m_stack->currentIndex()==4) m_locaux->focusSearch();','else if(m_stack->currentIndex()==4) m_locaux->focusSearch();\n        else if(m_stack->currentIndex()==5) m_medical->focusSearch();')
edit(p,'    positionNavigationMarker(changed);','    else if(page==5) m_medical->refresh();\n    positionNavigationMarker(changed);')
edit(root/'src/main.cpp','#include "employees/employeestore.h"','#include "employees/employeestore.h"\n#include "medical/medicalstore.h"')
edit(root/'src/main.cpp','    AppController controller(demo);','    if(demo && !MedicalStore::seedExamples(&error)) { QMessageBox::critical(nullptr,"Dossiers médicaux",error); return 1; }\n    AppController controller(demo);')
p=root/'sources.pri';s=p.read_text(encoding='utf8')+'\n'
for name in ['medicalrecord','medicalrecorddialog','medicalrecorddetailsdialog','recordcard','medicalstore','medicalwidget']:
    s+=f'SOURCES += $$PWD/src/medical/{name}.cpp\nHEADERS += $$PWD/src/medical/{name}.h\n'
s+='SOURCES += $$PWD/src/medical/medicalexamples.cpp\n';write(p,s)
p=root/'tests/service_tests.cpp';s=p.read_text(encoding='utf8').replace('count(),5','count(),6').replace('page<5','page<6').replace('QString("locaux"),QString("admin")','QString("locaux"),QString("medical"),QString("admin")').replace("WHERE role!='locaux'","WHERE role NOT IN ('locaux','medical')");write(p,s)
print('Medical source adapted; backup, account migration and navigation prepared.')

