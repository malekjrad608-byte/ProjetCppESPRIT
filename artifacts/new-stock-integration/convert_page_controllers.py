from pathlib import Path
import re,json,sys
root=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(root/'tools'))
from check_model_sql_ownership import mask_cpp
bindings=json.loads((root/'artifacts/new-stock-integration/designer/bindings.json').read_text())
def bound(prefix):
 return '\n'.join('    '+k[len(prefix)+1:]+'=ui.'+k+';' for k in bindings if k.startswith(prefix+'_m_') or (prefix=='Stock' and k.startswith('Stock_') and not k.startswith('Stock_control_')))
def function(s,marker):
 a=s.index(marker);masked=mask_cpp(s);start=masked.index('{',a);depth=1;i=start+1
 while depth:
  depth+=(masked[i]=='{')-(masked[i]=='}');i+=1
 return a,i,s[a:i]
def connects(s):
 masked=mask_cpp(s);i=0;out=[]
 while True:
  m=re.search(r'\bconnect\s*\(',masked[i:])
  if not m:break
  a=i+m.start();p=masked.index('(',a);depth=1;b=p+1
  while depth:depth+=(masked[b]=='(')-(masked[b]==')');b+=1
  out.append(s[a:b]+';');i=b
 return out
def erase(s,markers):
 for marker in markers:
  a,b,_=function(s,marker);s=s[:a]+s[b:]
 return s

# === EMPLOYES : Designer builds the static view; C++ retains data/actions ===
p=root/'src/employees/gestionemployes.cpp';s=p.read_text(encoding='utf-8')
_,_,stats=function(s,'QWidget *GestionEmployes::construireStatsPills')
call=next(c for c in connects(stats) if c.startswith('connect(btnStats,'))
statbody=call[call.index('[this]() {')+len('[this]() {'):call.rfind('});')]
code='''void GestionEmployes::construireUI() {
    Ui::EmployeesPage ui;ui.setupUi(this);
'''+bound('Employees')+'''
    m_gridCards->setAlignment(Qt::AlignTop);
    m_detailActions->hide();m_detailContent->hide();
    m_editRecherche->addAction(Design::icon("search"),QLineEdit::LeadingPosition);
    connect(ui.addEmployee,&QPushButton::clicked,this,&GestionEmployes::ajouterEmploye);
    connect(ui.employeeLeavePdf,&QPushButton::clicked,this,&GestionEmployes::exportLeaveCertificate);
    connect(ui.employeeStatistics,&QPushButton::clicked,this,&GestionEmployes::showStatistics);
    connect(ui.editEmployee,&QPushButton::clicked,this,&GestionEmployes::modifierEmploye);
    connect(ui.toggleEmployeeAvailability,&QPushButton::clicked,this,&GestionEmployes::basculerDisponibilite);
    connect(ui.deleteEmployee,&QPushButton::clicked,this,&GestionEmployes::supprimerEmploye);
    connect(m_editRecherche,&QLineEdit::textChanged,this,&GestionEmployes::onFiltresChange);
    for(auto *combo:{m_comboPoste,m_comboStatut,m_comboTri})connect(combo,&QComboBox::currentTextChanged,this,&GestionEmployes::onFiltresChange);
}

void GestionEmployes::showStatistics() {'''+statbody+'}\n'
s=erase(s,['void GestionEmployes::construireUI','QWidget *GestionEmployes::construireHeader','QWidget *GestionEmployes::construireStatsPills','QWidget *GestionEmployes::construireBarreRecherche','QWidget *GestionEmployes::construireGrid','QWidget *GestionEmployes::construireDetailPanel'])
s='#include "ui_integrated_pages.h"\n'+s;s=s.replace('// ==================== UI PRINCIPALE ====================', '// === MODULE EMPLOYES : LIAISON AU FICHIER MAINWINDOW.UI ===\n'+code,1);p.write_text(s,encoding='utf-8')
p=root/'src/employees/gestionemployes.h';s=p.read_text(encoding='utf-8');s=re.sub(r'    QWidget \*construire(?:Header|StatsPills|BarreRecherche|Grid|DetailPanel)\(\);\n','',s);s=s.replace('    void construireUI();','    void construireUI();\n    void showStatistics();');p.write_text(s,encoding='utf-8')

# === LOCAUX ===
p=root/'src/locaux/pagelocaux.cpp';s=p.read_text(encoding='utf-8');connections=[]
for name in ['buildHeader','buildTools','buildBody']:
 _,_,body=function(s,'QLayout *PageLocaux::'+name)
 connections+=connects(body)
connections=[c.replace('connect(reset,','connect(ui.locauxResetFilters,').replace('connect(enlarge,','connect(ui.locauxEnlargeMap,') for c in connections]
code='''void PageLocaux::setupUi() {
    Ui::LocauxPage ui;ui.setupUi(this);
'''+bound('Locaux')+'''
    m_search->addAction(Icons::icon("search",Theme::Muted,16),QLineEdit::LeadingPosition);
    m_table->setColumnCount(7);m_table->setHorizontalHeaderLabels({"ID","Local","Type","Étage","Occupation","État","Nettoyage"});
    if(OracleConnection::isOracle()) {
        m_table->setColumnCount(8);m_table->setHorizontalHeaderLabels({"ID local","Nom local","Type","Étage","Capacité","Spécialité","État","ID service"});
        for(auto *b:{m_chipDirty,m_btnPdf,m_btnSmart,m_btnStats,m_btnAlerts})b->hide();
        m_bar->parentWidget()->hide();
    }
    m_table->verticalHeader()->hide();m_table->verticalHeader()->setDefaultSectionSize(42);
    m_table->horizontalHeader()->setHighlightSections(false);m_table->horizontalHeader()->setMinimumHeight(40);
    m_table->horizontalHeader()->setSectionsClickable(true);m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);m_table->setColumnWidth(0,70);
    m_queryHint->hide();m_empty->hide();
'''+ '\n'.join('    '+c for c in connections)+'\n}\n'
a=s.index('    auto *root = new QVBoxLayout(this);');b=s.index('    // le plan et la liste',a);s=s[:a]+'    setupUi();\n\n'+s[b:]
s=erase(s,['QPushButton *PageLocaux::makeButton','QPushButton *PageLocaux::makeChip','QLayout *PageLocaux::buildHeader','QLayout *PageLocaux::buildTools','QLayout *PageLocaux::buildBody'])
s='#include "ui_integrated_pages.h"\n'+s;s=s.replace('// ================================================================ construction','// === MODULE LOCAUX : LIAISON AU FICHIER MAINWINDOW.UI ===\n'+code,1);p.write_text(s,encoding='utf-8')
p=root/'src/locaux/pagelocaux.h';s=p.read_text(encoding='utf-8');s=re.sub(r'    QLayout \*build(?:Header|Tools|Body)\(\);\n','',s);s=re.sub(r'    QPushButton \*make(?:Button|Chip)\([^;]+;\n','',s);s=s.replace('    // construction','    // Interface commune : src/mainwindow.ui\n    void setupUi();');p.write_text(s,encoding='utf-8')

# === STOCK ===
p=root/'src/stock/stockwidget.cpp';s=p.read_text(encoding='utf-8');a,b,old=function(s,'void StockWidget::setupUi()')
start=old.index('    stockTable->setModel(tableModel);');end=old.index('    contentLayout->addWidget(stockTable, 1);',start)
configure=old[start:end]
code='''void StockWidget::setupUi() {
    Ui::StockPage ui;ui.setupUi(this);contentWidget=this;
'''+bound('Stock')+'''
    tableModel->setHorizontalHeaderLabels(OracleConnection::isOracle()
      ?QStringList{"ID ARTICLE","ARTICLE","NOM ARTICLE","LOT","QUANTITÉ DISP.","TYPE ARTICLE","SEUIL MIN.","DATE ÉCHÉANCE","ID LOCAL"}
      :QStringList{"ARTICLE","LOT","DISPONIBLE","SEUIL","ÉCHÉANCE","ÉTAT"});
'''+configure+'''
    connect(ui.stockMovementButton,&QPushButton::clicked,this,&StockWidget::openMovementDialog);
    connect(ui.stockExportButton,&QPushButton::clicked,this,&StockWidget::exportInventory);
    connect(ui.stockStatisticsButton,&QPushButton::clicked,this,&StockWidget::showStatistics);
    connect(ui.stockEditButton,&QPushButton::clicked,this,&StockWidget::editSelectedItem);
    connect(ui.stockDeleteButton,&QPushButton::clicked,this,&StockWidget::deleteSelectedItem);
    connect(ui.stockExpiryButton,&QPushButton::clicked,this,&StockWidget::manageExpiryItems);
    connect(ui.stockPurchasesButton,&QPushButton::clicked,this,&StockWidget::openPurchasesModule);
    connect(alertsNavButton,&QPushButton::clicked,this,&StockWidget::openAlertsModule);
    connect(ui.stockReplenishmentButton,&QPushButton::clicked,this,&StockWidget::openReplenishmentModule);
    connect(searchLineEdit,&QLineEdit::textChanged,this,&StockWidget::refreshTable);
    for(auto *combo:{categoryFilter,sortFilter,stateFilter})connect(combo,&QComboBox::currentIndexChanged,this,&StockWidget::refreshTable);
}'''
s='#include "ui_integrated_pages.h"\n'+s[:a]+code+s[b:];p.write_text(s,encoding='utf-8')

# === SERVICES ===
p=root/'src/widgets/servicewidget.cpp';s=p.read_text(encoding='utf-8');a,b,old=function(s,'ServiceWidget::ServiceWidget(')
ta=old.index('    m_table->setModel(');tb=old.index('    m_views->addWidget(m_table);',ta)
configure=old[ta:tb]
code='''ServiceWidget::ServiceWidget(QWidget *parent) : QWidget(parent) {
    Ui::ServicePage ui;ui.setupUi(this);setObjectName("servicesPage");
'''+bound('Service')+ '\n'
for i in range(4):code+=f'    m_summary.append(ui.Service_summary_{i});\n'
for i in range(8):code+=f'    m_detailValues.append(ui.Service_detailValue_{i});\n'
code+='''    m_grid->setAlignment(Qt::AlignTop);m_grid->setSizeConstraint(QLayout::SetMinimumSize);
    m_gridScroll->widget()->setAutoFillBackground(false);m_gridScroll->viewport()->setAutoFillBackground(false);
    m_search->addAction(Design::icon("search"),QLineEdit::LeadingPosition);
    m_notice->hide();m_attention->hide();
    m_noticeTimer=new QTimer(this);m_noticeTimer->setSingleShot(true);
    connect(m_noticeTimer,&QTimer::timeout,m_notice,&QWidget::hide);
    connect(ui.servicePlanning,&QPushButton::clicked,this,&ServiceWidget::showPlanning);
    connect(ui.addService,&QPushButton::clicked,this,&ServiceWidget::onAdd);
    connect(ui.serviceFirstAdd,&QPushButton::clicked,this,&ServiceWidget::onAdd);
    connect(ui.serviceStatistics,&QPushButton::clicked,this,&ServiceWidget::showStatistics);
    connect(m_edit,&QPushButton::clicked,this,&ServiceWidget::editSelected);
    connect(m_view,&QPushButton::clicked,this,[this]{viewService(m_selectedId);});
    connect(m_invoice,&QPushButton::clicked,this,&ServiceWidget::showInvoice);
    connect(m_advance,&QPushButton::clicked,this,&ServiceWidget::advanceStatus);
    connect(m_delete,&QPushButton::clicked,this,&ServiceWidget::deleteSelected);
    auto *group=new QButtonGroup(this);int mode=0;
    for(auto *toggle:{ui.cardsView,ui.tableView}){
        const int index=mode++;group->addButton(toggle,index);
        connect(toggle,&QPushButton::clicked,this,[this,index]{m_viewMode=index;m_views->setCurrentIndex(m_filtered.isEmpty()?2:index);Motion::reveal(m_views,QColor("#F3F7F8"));});
    }
'''+configure+'''
    connect(m_search,&QLineEdit::textChanged,this,&ServiceWidget::filter);
    for(auto *combo:{m_status,m_category,m_sort,m_period})connect(combo,&QComboBox::currentIndexChanged,this,&ServiceWidget::filter);
    connect(ui.resetFilters,&QPushButton::clicked,this,[this]{
        const QSignalBlocker a(m_search),b(m_status),c(m_category),d(m_period),e(m_sort);
        m_search->clear();m_status->setCurrentIndex(0);m_category->setCurrentIndex(0);m_period->setCurrentIndex(0);m_sort->setCurrentIndex(0);filter();
    });
    connect(m_table,&QTableView::doubleClicked,this,[this](const QModelIndex &index){viewService(index.siblingAtColumn(0).data().toInt());});
    auto *timer=new QTimer(this);connect(timer,&QTimer::timeout,this,[this]{updateSummary();if(m_period->currentIndex()>0)filter();else updateDetail();});timer->start(60000);
    refresh();
}'''
s=s[:a]+code+s[b:];s=erase(s,['void ServiceWidget::setupDetailPanel(']);s='#include "ui_integrated_pages.h"\n'+s;p.write_text(s,encoding='utf-8')
p=root/'src/widgets/servicewidget.h';s=p.read_text(encoding='utf-8').replace('    void setupDetailPanel(QHBoxLayout *body);\n','');p.write_text(s,encoding='utf-8')
print('Converted page controllers to generated Ui bindings.')
