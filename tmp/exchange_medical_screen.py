from pathlib import Path
import re,shutil
root=Path.cwd();source=Path(r'C:\Users\malek\Downloads\Dossier_Medical_ANIWORLD (2)\Dossier_Medical_ANIWORLD');dest=root/'src/medical'
old=(dest/'medicalwidget.cpp').read_text(encoding='utf8')
h=(source/'mainwindow.h').read_text(encoding='utf-8-sig').replace('MAINWINDOW_H','IMPORTED_MEDICAL_WIDGET_H').replace('MainWindow','MedicalWidget').replace('explicit MedicalWidget(QWidget *parent = nullptr);','explicit MedicalWidget(bool demo=false,QWidget *parent = nullptr);\n    void refresh();\n    void addRecord() { onAddRecordClicked(); }\n    void focusSearch();')
h=h.replace('    Ui::MedicalWidget *ui = nullptr;','    Ui::MedicalWidget *ui = nullptr;\n    bool m_demo=false;\n    void exportPdf(bool selectedOnly);\n    const MedicalRecord *selected() const;\n    void saveDialog(const QString &recordId);\n    QPushButton *m_pdfButton=nullptr;')
s=(source/'mainwindow.cpp').read_text(encoding='utf-8-sig').replace('#include "mainwindow.h"','#include "medicalwidget.h"').replace('#include "motion.h"','#include "../ui/motion.h"\n#include "../ui/design.h"\n#include "../ui/invoice.h"\n#include "medicalstore.h"\n#include <QSignalBlocker>\n#include <QTextBrowser>').replace('#include "ui_mainwindow.h"','#include "ui_medicalwidget.h"').replace('MainWindow','MedicalWidget')
s=s.replace('MedicalWidget::MedicalWidget(QWidget *parent)','MedicalWidget::MedicalWidget(bool demo,QWidget *parent)').replace(': QMedicalWidget(parent)',': QMainWindow(parent)').replace(', ui(new Ui::MedicalWidget)',', ui(new Ui::MedicalWidget), m_demo(demo)').replace('QMedicalWidget::','QMainWindow::')
s=s.replace('    loadStaticData();','    setWindowFlags(Qt::Widget);\n    setMinimumSize(0,0);\n    setObjectName("medicalPage");\n    ui->sidebar->hide(); ui->topBar->hide();\n    loadStaticData();',1)
def replace_method(name,body):
    global s
    pattern=r'(?:void|QString) MedicalWidget::'+name+r'\([^\n]*\)\s*\{'
    m=re.search(pattern,s);assert m,name
    start=m.end();depth=1;i=start
    # Existing methods contain no unmatched braces in string literals.
    while depth:
        if s[i]=='{':depth+=1
        elif s[i]=='}':depth-=1
        i+=1
    s=s[:start]+'\n'+body+'\n'+s[i-1:]
replace_method('loadStaticData','    QString error; m_records=MedicalStore::all(&error);\n    if(!error.isEmpty()) QMessageBox::warning(this,"Dossiers médicaux",error);')
replace_method('loadStylesheet','    QFile file(":/medical-source.qss");\n    if(file.open(QIODevice::ReadOnly)) setStyleSheet(QString::fromUtf8(file.readAll()));')
replace_method('editRecord','    saveDialog(recordId);')
replace_method('onAddRecordClicked','    saveDialog({});')
replace_method('onRefreshClicked','    refresh();')
replace_method('onToggleVaccinationClicked','''    const int idx=indexOfRecord(m_selectedId); if(idx<0)return;
    auto record=m_records[idx]; record.medical.vaccinationStatus=record.medical.vaccinationStatus=="À jour"?"À renouveler":"À jour";
    QString error; if(!MedicalStore::save(record,&error)) {QMessageBox::warning(this,"Enregistrement impossible",error);return;} refresh();''')
s=s.replace('        m_records.removeAt(idx);','        QString error; if(!MedicalStore::remove(m_records[idx].storageId,&error)) {QMessageBox::warning(this,"Suppression impossible",error);return;}\n        loadStaticData();').replace('QMessageBox::Yes | QMessageBox::No);','QMessageBox::Yes | QMessageBox::No,QMessageBox::No);')
replace_method('onExportCsvClicked','    exportPdf(false);')
s=s.replace('onExportCsvClicked','onExportPdfClicked');h=h.replace('onExportCsvClicked','onExportPdfClicked')
s=s.replace('Exporter en CSV…','Exporter PDF…').replace('    menu->addAction("Quitter", this, &QWidget::close);','')
s=s.replace('    ui->btnFile->setMenu(menu);','''    ui->btnFile->setMenu(menu);
    ui->btnFile->setParent(ui->recordsPage);
    ui->btnFile->setText("Fichier"); ui->btnFile->show();
    m_pdfButton=Design::button("PDF du dossier","export");m_pdfButton->setParent(ui->detailContent);m_pdfButton->setObjectName("medicalRecordPdf");
    connect(m_pdfButton,&QPushButton::clicked,this,[this]{exportPdf(true);});''')
s=s.replace('QIcon(":/icons/search_grey.svg")','Design::icon("search")').replace('QPixmap(":/assets/logo.png")','Design::brand("logo",QSize(48,48))')
# Use shared animated buttons while retaining the imported UI and its geometry.
needle='    // --- Style ids'
insert='''    auto animated=[](QPushButton *&button,const char *role) {
        auto *replacement=Design::button(button->text(),{},role);
        replacement->setParent(button->parentWidget());replacement->setObjectName(button->objectName());replacement->setGeometry(button->geometry());
        delete button;button=replacement;
    };
    animated(ui->btnAdd,"primary");animated(ui->btnStatsTop,nullptr);animated(ui->btnReset,nullptr);
    animated(ui->btnView,"primary");animated(ui->btnToggle,"success");animated(ui->btnEdit,"primary");animated(ui->btnDelete,"danger");
'''
s=s.replace(needle,insert+needle,1)
s=s.replace('        c->deleteLater();','        { c->hide(); c->deleteLater(); }')
s=s.replace('const int kCardMinWidth = 310;','const int kCardMinWidth = 270;')
s=s.replace('m_cardsContainer->setGeometry(0, 0, viewW, contentH);','m_cardsContainer->setMinimumHeight(contentH);\n    m_cardsContainer->resize(viewW,contentH);')
# The supplied page is embedded within the application's existing shell.
s=s.replace('const int sidebarW = 290, topH = 68, footerH = 38;','const int sidebarW = 0, topH = 0, footerH = 30;')
s=s.replace('    ui->sidebar->setGeometry(0, 0, sidebarW, H);','    ui->sidebar->hide(); ui->topBar->hide();')
s=s.replace('const int pageW = mainW - 68;','const int pageW = mainW - 48;').replace('ui->pageArea->setGeometry(34, pageTop, pageW, pageH);','ui->pageArea->setGeometry(24, pageTop, pageW, pageH);')
s=s.replace('const int LW = pageW - kPanelWidth - kPanelGap;','const int LW = pageW - qMin(kPanelWidth,qMax(280,pageW/3)) - kPanelGap;')
s=s.replace('ui->btnStatsTop->setGeometry(LW - statsW, 62, statsW, 40);','ui->btnStatsTop->setGeometry(LW - statsW, 62, statsW, 40);\n    ui->btnFile->setGeometry(LW-statsW-130,62,118,40);')
s=s.replace('qMax(100, LW - 220)','qMax(100, LW - 200)').replace('ui->pageTitle->setGeometry(0, 0, qMax(100, LW - 200), 44);','ui->pageTitle->setGeometry(0, 0, LW, 44);')
# At narrow widths the add action moves above the title rather than overlapping it.
s=s.replace('    const int chipsY = 104;','    const bool narrow=LW<580;\n    const int shift=narrow?54:0;\n    if(narrow) {ui->pageTitle->move(0,54);ui->pageSubtitle->move(0,99);}\n    const int chipsY = 104+shift;')
s=s.replace('ui->btnStatsTop->setGeometry(LW - statsW, 62, statsW, 40);','ui->btnStatsTop->setGeometry(LW - statsW, narrow?0:62, statsW, 40);')
s=s.replace('ui->btnFile->setGeometry(LW-statsW-130,62,118,40);','ui->btnFile->setGeometry(narrow?LW-118:LW-statsW-130,narrow?104:62,118,40);')
s=s.replace('ui->btnAdd->setGeometry(LW - addW, 0, addW, 52);','ui->btnAdd->setGeometry(narrow?0:LW-addW,0,narrow?qMin(200,LW-statsW-12):addW,52);')
s=s.replace('const int ty = 150;','const int ty = 150+shift;')
replace_method('layoutDetailPanel','''    const bool has=indexOfRecord(m_selectedId)>=0;
    const int panelW=qMin(kPanelWidth,qMax(280,ui->pageArea->width()/3)),inner=panelW-44;
    const int cardH=has?510:150;
    ui->selectedCard->setGeometry(0,0,panelW,cardH);
    ui->placeholderText->setGeometry(22,58,inner,64);
    ui->detailContent->setGeometry(22,44,inner,450);
    m_detailTitle->setGeometry(0,0,inner,32);m_detailPill->setGeometry(0,40,inner,28);
    m_detailLine1->setGeometry(0,78,inner,28);m_detailLine2->setGeometry(0,108,inner,46);m_detailLine3->setGeometry(0,156,inner,64);
    int y=226;
    for(auto *button:{m_btnView,m_btnToggle,m_btnEdit,m_pdfButton,m_btnDelete}) {button->setGeometry(0,y,inner,36);y+=44;}
    ui->globalCard->hide();ui->detailPanel->resize(panelW,cardH);
''')
s=s.replace('    updateChips();','    ui->btnFile->setEnabled(!m_filteredRecords.isEmpty());\n    updateChips();')
s=s.replace('m_globalBar->setValue(pct);','Motion::progressTo(m_globalBar,pct);').replace('rate->setValue(speciesPercent);','Motion::progressTo(rate,speciesPercent);')
s=s.replace('dialog.setMinimumSize(900, 650);','dialog.setMinimumSize(740, 540);').replace('dialog.resize(1000, 760);','dialog.resize(880, 640);')
s=s.replace('table->setAlternatingRowColors(true);','table->setAlternatingRowColors(false);')
s=s.replace('    m_btnDelete = ui->btnDelete;','''    m_btnDelete = ui->btnDelete;
    for(auto *label:{m_detailTitle,m_detailLine1,m_detailLine2,m_detailLine3}) {label->setTextFormat(Qt::PlainText);label->setWordWrap(true);}
''')
s+='''
void MedicalWidget::refresh() { loadStaticData();refreshView(); }
void MedicalWidget::focusSearch() { m_searchEdit->setFocus();m_searchEdit->selectAll(); }
const MedicalRecord *MedicalWidget::selected() const {int idx=indexOfRecord(m_selectedId);return idx<0?nullptr:&m_records[idx];}
void MedicalWidget::saveDialog(const QString &id) {
    const int idx=indexOfRecord(id);const qint64 storageId=idx<0?0:m_records[idx].storageId;
    MedicalRecordDialog dialog(this);if(idx>=0)dialog.setMedicalRecord(m_records[idx]);
    auto *save=dialog.findChild<QPushButton *>("primaryButton");disconnect(save,nullptr,&dialog,nullptr);
    connect(save,&QPushButton::clicked,&dialog,[&]{auto record=dialog.getMedicalRecord();record.storageId=storageId;QString error;
        if(!MedicalStore::save(record,&error)){QMessageBox::warning(&dialog,"Enregistrement impossible",error);return;}
        m_selectedId=record.medical.recordId;dialog.accept();});
    if(dialog.exec()==QDialog::Accepted)refresh();
}
'''
export=old[old.index('void MedicalWidget::exportPdf('):].replace('records=m_filtered','records=m_filteredRecords')
s+='\n'+export
# Stable automation/accessibility names; style selectors are adapted with them.
names={'searchEdit':'medicalSearch','filterSpecies':'medicalSpecies','filterVaccination':'medicalVaccination','sortCombo':'medicalSort','btnReset':'medicalReset','btnAdd':'medicalAdd','btnStatsTop':'medicalStatistics','cardsScroll':'medicalCardsScroll','statisticsDialog':'medicalStatisticsDialog','statisticsTable':'medicalStatsTable','topButton':'medicalExport','actionViewButton':'medicalView','actionToggleButton':'medicalToggle','actionEditButton':'medicalEdit','actionDeleteButton':'medicalDelete'}
css=(source/'styles/style.qss').read_text(encoding='utf-8-sig')
ui=(source/'mainwindow.ui').read_text(encoding='utf8').replace('MainWindow','MedicalWidget').replace('QMedicalWidget','QMainWindow')
for a,b in names.items():
    s=s.replace('"'+a+'"','"'+b+'"');css=re.sub(r'#'+a+r'\b','#'+b,css)
# Designer names remain unchanged as C++ member names; runtime identifiers follow the mapping.
extra='\n'.join(f'    ui->{a}->setObjectName("{b}");' for a,b in names.items() if a in ['searchEdit','filterSpecies','filterVaccination','sortCombo','btnReset','btnAdd','btnStatsTop','cardsScroll'])
s=s.replace('    m_chipTotal = ui->chipTotal;',extra+'\n    m_chipTotal = ui->chipTotal;')
(dest/'medicalwidget.cpp').write_text(s,encoding='utf8');(dest/'medicalwidget.h').write_text(h,encoding='utf8');(dest/'medicalwidget.ui').write_text(ui,encoding='utf8')
css+='''
QPushButton:hover {border-color:#FF9B28;}
QPushButton:focus {border-color:#FF9B28;}
QProgressBar#statisticsRateBar {min-height:24px;max-height:24px;}
QWidget#medicalrecorddialog, QWidget#medicalrecorddetailsdialog {background:#EEF3F7;}
QLineEdit,QComboBox {min-width:0;}
QLabel#pageTitle {font-size:26px;}
QPushButton#medicalAdd {font-size:12px;padding:8px;}
'''
(root/'resources/medical-source.qss').write_text(css,encoding='utf8')
p=root/'resources/resources.qrc';t=p.read_text(encoding='utf8');t=t.replace('    <file>theme.qss</file>','    <file>theme.qss</file>\n    <file>medical-source.qss</file>');p.write_text(t,encoding='utf8')
p=root/'sources.pri';t=p.read_text(encoding='utf8');t+='\nFORMS += $$PWD/src/medical/medicalwidget.ui\n';p.write_text(t,encoding='utf8')
print('Replaced MedicalWidget with the supplied MainWindow implementation and Designer UI.')
