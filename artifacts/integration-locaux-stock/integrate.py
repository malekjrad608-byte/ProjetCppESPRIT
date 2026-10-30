from pathlib import Path
import shutil, sqlite3, re

root=Path(r'C:\Users\malek\Documents\gestion_service')
stock=Path(r'C:\Users\malek\Downloads\ANIWORLD_Stock_finale\ANIWORLD_Stock_Modifie')
loc=Path(r'C:\Users\malek\Downloads\locauxF\ANIWORLD_Locaux')
backup=root/'artifacts/integration-locaux-stock/backup'
backup.mkdir(exist_ok=True)
for rel in ['src','resources','sources.pri','README.md','tests']:
    p=root/rel
    if p.exists() and not (backup/rel).exists():
        shutil.copytree(p,backup/rel) if p.is_dir() else shutil.copy2(p,backup/rel)
db=Path(r'C:\Users\malek\AppData\Local\ANIWORLD\ANIWORLD\services.sqlite')
if db.exists() and not (backup/'services.sqlite').exists():
    with sqlite3.connect(db) as a, sqlite3.connect(backup/'services.sqlite') as b: a.backup(b)
initial=Path(r'C:\Users\malek\AppData\Roaming\ANIWORLD\ANIWORLD\locaux_v2.json')
if initial.exists():
    shutil.copy2(initial,backup/'locaux_v2.json')
    shutil.copy2(initial,root/'resources/data/locaux-initial.json')

def read(rel): return (root/rel).read_text(encoding='utf-8-sig')
def write(rel,s): (root/rel).write_text(s,encoding='utf-8')

old=read('src/stock/stockwidget.cpp')
pdf=old[old.index('bool StockWidget::exportInventoryPdf'):].replace('filterModel->','tableModel->').replace('notice->setText','footerLabel->setText')
s=(stock/'stockwidget.cpp').read_text(encoding='utf-8-sig')
s='#include "../ui/design.h"\n#include "../ui/invoice.h"\n#include <QPainter>\n#include <QFileDialog>\n'+s
a=s.index('    QPushButton *button = new QPushButton(text);'); b=s.index('    return button;',a)
s=s[:a]+'''    Q_UNUSED(foreground); Q_UNUSED(border);
    auto *button = Design::button(text, {}, background == kPrimary ? "primary" : background == "#FCEDEC" ? "danger" : nullptr);
'''+s[b:]
a=s.index('    setStyleSheet',s.index('StockWidget::StockWidget')); b=s.index('    loadDemoData();',a)
s=s[:a]+s[b:]
a=s.index('    QHBoxLayout *windowLayout',s.index('void StockWidget::setupUi'));b=s.index('    contentLayout->setContentsMargins',a)
s=s[:a]+'''    contentWidget = this;
    auto *contentLayout = new QVBoxLayout(this);
'''+s[b:]
s=s.replace('    mainAreaLayout->addWidget(contentWidget, 1);\n    windowLayout->addWidget(mainArea, 1);','')
s=re.sub(r'^    connect\((statsNav|logoutButton),.*\n','',s,flags=re.M)
s=s.replace('new QLabel("📦  Stock")','new QLabel("Stock")')
s=s.replace('    titleLayout->addWidget(statsButton);\n    titleLayout->addWidget(editButton);\n    titleLayout->addWidget(deleteButton);','''    auto *exportButton=Design::button("Exporter PDF", "download");
    exportButton->setObjectName("stockExportButton");
    connect(exportButton,&QPushButton::clicked,this,&StockWidget::exportInventory);
    titleLayout->addWidget(exportButton);''')
s=s.replace('    contentLayout->addLayout(titleLayout);','''    contentLayout->addLayout(titleLayout);
    auto *actions=new QHBoxLayout;
    actions->addWidget(statsButton); actions->addStretch();
    actions->addWidget(editButton); actions->addWidget(deleteButton);
    contentLayout->addLayout(actions);''')
a=s.index('    stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive)');b=s.index('    contentLayout->addWidget(stockTable',a)
s=s[:a]+'''    stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    stockTable->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    stockTable->setMinimumHeight(230);
'''+s[b:]
for var,name in [('searchLineEdit','stockSearch'),('categoryFilter','stockCategoryFilter'),('sortFilter','stockSortFilter'),('stateFilter','stockStateFilter'),('stockTable','stockTable'),('movementButton','stockMovementButton'),('statsButton','stockStatisticsButton'),('editButton','stockEditButton'),('deleteButton','stockDeleteButton'),('expiryButton','stockExpiryButton')]:
    s=s.replace('    connect(statsButton,',f'    {var}->setObjectName("{name}");\n    connect(statsButton,',1)
s+='''
void StockWidget::refresh() { refreshTable(); }
void StockWidget::focusSearch() { searchLineEdit->setFocus(); searchLineEdit->selectAll(); }
void StockWidget::paintEvent(QPaintEvent *) { QPainter p(this); Design::paintWorkspace(p,rect()); }
'''+pdf
write('src/stock/stockwidget.cpp',s)
h=(stock/'stockwidget.h').read_text(encoding='utf-8-sig').replace('signals:\n    void logoutRequested();','''    void refresh();
    void focusSearch();
    bool exportInventoryPdf(const QString &path,QString *error=nullptr) const;
protected:
    void paintEvent(QPaintEvent *) override;''').replace('private slots:\n    void openMovementDialog();','public slots:\n    void openMovementDialog();\nprivate slots:\n    void exportInventory();')
write('src/stock/stockwidget.h',h)

(root/'src/locaux').mkdir(exist_ok=True)
for p in (loc/'src').glob('*'):
    if p.stem not in ['mainwindow','pawburst','spinlogo']: shutil.copy2(p,root/'src/locaux'/p.name)
pri=read('sources.pri')+'\nQT += svg\n'
for ext,key in [('cpp','SOURCES'),('h','HEADERS')]:
    pri+='\n'+key+' += '+' \\\n    '.join('$$PWD/src/locaux/'+p.name for p in sorted((root/'src/locaux').glob('*.'+ext)))+'\n'
write('sources.pri',pri)
write('resources/resources.qrc',read('resources/resources.qrc').replace('</qresource>','    <file>data/locaux-initial.json</file>\n</qresource>'))

# Embed the module in the shared shell, buttons, backgrounds and motion preference.
for p in (root/'src/locaux').glob('*.cpp'):
    s=p.read_text(encoding='utf-8-sig')
    if 'new QPushButton' in s:
        s='#include "../widgets/animatedbutton.h"\n'+s
        s=s.replace('new QPushButton;', 'new AnimatedButton(QString());').replace('new QPushButton(', 'new AnimatedButton(')
    s=s.replace(':/images/logo.png',':/brand/logo.png')
    p.write_text(s,encoding='utf-8')
s=read('src/locaux/hoverglow.cpp')
s='#include "../widgets/animatedbutton.h"\n#include "../ui/motion.h"\n'+s
s=s.replace('    if (w)\n','    if (w && !dynamic_cast<AnimatedButton *>(w))\n')
s=s.replace('    m_anim = new QPropertyAnimation', '    m_color = QColor(245, 158, 11, 110);\n    m_anim = new QPropertyAnimation',1)
s=s.replace('    w->installEventFilter(this);','''    w->installEventFilter(this);
    connect(&Motion::instance(), &Motion::changed, this, [this](bool enabled) {
        if (!enabled) { m_anim->stop(); m_level=0; removeEffect(); }
    });''')
write('src/locaux/hoverglow.cpp',s)
s=read('src/locaux/pagelocaux.cpp')
s='#include "../ui/design.h"\n#include "../ui/motion.h"\n#include <QPainter>\n#include <QHideEvent>\n'+s
s=s.replace('setObjectName("page");','''setObjectName("locauxPage");
    Theme::animations=Motion::instance().enabled();
    connect(&Motion::instance(), &Motion::changed, this, [](bool enabled) { Theme::animations=enabled; });''')
a=s.index('    auto *find = new QShortcut');b=s.index('    m_miniMap->setLocaux',a)
s=s[:a]+s[b:]
s=s.replace('computeAlerts(true);','computeAlerts(isVisible());')
s=s.replace('    auto *row = new QHBoxLayout;\n    row->setSpacing(16);','    auto *row = new QVBoxLayout;\n    row->setSpacing(10);',1)
s=s.replace('    r1->addWidget(m_btnAdd);','    titleRow->addWidget(m_btnAdd);')
s=s.replace('    r2->addStretch();','')
s=s.replace('    btnCol->addLayout(r1);\n    btnCol->addLayout(r2);','    r1->addLayout(r2);\n    btnCol->addLayout(r1);')
s=s.replace('rightScroll->setFixedWidth(356);','rightScroll->setFixedWidth(306);')
s=s.replace('    m_search = new QLineEdit;','    m_search = new QLineEdit; m_search->setObjectName("locauxSearch");')
s=s.replace('    m_table = new QTableWidget(0, 7);','    m_table = new QTableWidget(0, 7); m_table->setObjectName("locauxTable");')
for var,name in [('m_btnAdd','locauxAddButton'),('m_btnPdf','locauxPdfButton'),('m_btnMap','locauxMapButton'),('m_btnStats','locauxStatisticsButton'),('m_btnSmart','locauxAssignButton'),('m_btnList','locauxListButton'),('m_btnAlerts','locauxAlertsButton')]:
    s=re.sub(r'('+var+r' = makeButton[^\n]+)',r'\1\n    '+var+'->setObjectName("'+name+'");',s)
s=s.replace('    m_repo->update(dlg.result());','    if (!m_repo->update(dlg.result())) return;')
s=s.replace('    m_repo->remove(id);','    if (!m_repo->remove(id)) return;')
s=s.replace('    const int id = m_repo->add(dlg.result());','    const int id = m_repo->add(dlg.result());\n    if (id<=0) return;')
s=s.replace('        m_repo->update(copy);','        if (!m_repo->update(copy)) return;')
s=s.replace('    m_toast = new Toast(this);','''    m_toast = new Toast(this);
    connect(repo,&LocalRepository::storageError,this,[this](const QString &error) {
        QMessageBox::warning(this,"Sauvegarde des locaux",error);
    });''')
s+='''
void PageLocaux::focusSearch() { m_search->setFocus(); m_search->selectAll(); }
void PageLocaux::refreshPage() { refresh(false); }
void PageLocaux::paintEvent(QPaintEvent *) { QPainter p(this); Design::paintWorkspace(p,rect()); }
void PageLocaux::hideEvent(QHideEvent *event) { if(m_plan) m_plan->close(); QWidget::hideEvent(event); }
'''
write('src/locaux/pagelocaux.cpp',s)
h=read('src/locaux/pagelocaux.h').replace('private:\n    // construction','''    void focusSearch();
    void refreshPage();
    void addLocal();
protected:
    void paintEvent(QPaintEvent *) override;
    void hideEvent(QHideEvent *) override;
private:
    // construction''').replace('    // CRUD + actions metier\n    void addLocal();','    // CRUD + actions metier')
write('src/locaux/pagelocaux.h',h)
s=read('src/locaux/smartassigndialog.cpp').replace('            m_repo->update(l);','            if (!m_repo->update(l)) return;')
write('src/locaux/smartassigndialog.cpp',s)
# Retain module-specific labels/chips while shared controls retain the common theme.
style=(loc/'resources/style.qss').read_text(encoding='utf-8-sig')
rules=re.findall(r'([^{}]+)\{([^{}]*)\}',re.sub(r'/\*.*?\*/','',style,flags=re.S))
extra='\n/* Integrated Locaux: scoped typography, panels and filter chips. */\n'
for selector,body in rules:
    selector=selector.strip()
    if selector.startswith(('#h1','#sub','#key','#panel','#big','#toast','#statsTitle','#kicker','QPushButton[chip','QProgressBar')) or '[alert=' in selector:
        if ':hover' in selector: body='background:#FFF3DF; border-color:#F59E0B; color:#B56D10;'
        extra+='#locauxPage '+selector+' {'+body+'}\n'
extra+='''#locauxPage #h1 { font-size:26px; }
#locauxPage #panelName { font-size:20px; }
#locauxPage QPushButton[chip="true"] { padding:6px 11px; }
#locauxPage QPushButton[role="ghost"]:checked { background:#DDEEF1; border-color:#BEDBE1; }
'''
write('resources/theme.qss',read('resources/theme.qss')+extra)

h=read('src/auth/accounts.h').replace('Stock, Employees','Stock, Employees, Locaux');write('src/auth/accounts.h',h)
s=read('src/auth/accounts.cpp').replace('    if(role=="employes")','    if(role=="locaux") return Accounts::Role::Locaux;\n    if(role=="employes")')
a=s.index('    QSqlQuery q(db);');b=s.index('    for(const QString &name:',a)
s=s[:a]+'''    QSqlQuery q(db);
    auto fail=[&] { if(error) *error=q.lastError().text(); q.finish(); db.rollback(); return false; };
    if(!db.transaction()) { if(error) *error=db.lastError().text(); return false; }
    if(!q.exec("SELECT sql FROM sqlite_master WHERE type='table' AND name='accounts'")) return fail();
    const bool migrate=q.next() && !q.value(0).toString().contains("'locaux'");
    q.finish();
    if(migrate && !q.exec("ALTER TABLE accounts RENAME TO accounts_before_locaux")) return fail();
    if(!q.exec("CREATE TABLE IF NOT EXISTS accounts(username TEXT PRIMARY KEY COLLATE NOCASE, role TEXT NOT NULL CHECK(role IN ('admin','services','stock','employes','locaux')), salt BLOB NOT NULL, password_hash BLOB NOT NULL, iterations INTEGER NOT NULL)")) return fail();
    if(migrate) {
        if(!q.exec("INSERT INTO accounts SELECT * FROM accounts_before_locaux")) return fail();
        if(!q.exec("DROP TABLE accounts_before_locaux")) return fail();
    }
'''+s[b:]
s=s.replace('QString("employes")})','QString("employes"),QString("locaux")})')
s=s.replace('page<=3','page<=4').replace('(role==Role::Employees && page==3);','(role==Role::Employees && page==3) || (role==Role::Locaux && page==4);')
s=s.replace('return role==Role::Stock?2:', 'return role==Role::Locaux?4:role==Role::Stock?2:')
s=s.replace('return role==Role::Admin?"Administration":','return role==Role::Locaux?"Locaux":role==Role::Admin?"Administration":')
write('src/auth/accounts.cpp',s)
h=read('src/mainwindow.h').replace('class StockWidget;','class StockWidget;\nclass PageLocaux;').replace('    StockWidget *m_stock;','    StockWidget *m_stock;\n    PageLocaux *m_locaux;');write('src/mainwindow.h',h)
s=read('src/mainwindow.cpp')
s='#include "locaux/pagelocaux.h"\n#include "locaux/localrepository.h"\n#include <QFileInfo>\n'+s
s=s.replace('"Stock","Employés"','"Stock","Employés","Locaux"').replace('"stock","users"','"stock","users","home"').replace('"navStock","navEmployees"','"navStock","navEmployees","navLocaux"')
s=s.replace('future={"Locaux","Animaux & dossiers"}', 'future={"Animaux & dossiers"}').replace('fi={"grid","shield"}','fi={"shield"}')
s=s.replace('    QPalette pageColors=', '''    const QString dbPath=DatabaseManager::instance().storagePath();
    const QString locauxPath=(demo || dbPath.isEmpty() || dbPath==":memory:")?":memory:":QFileInfo(dbPath).absolutePath()+"/locaux_v2.json";
    auto *locauxRepo=new LocalRepository(this,locauxPath);
    m_locaux=new PageLocaux(locauxRepo); m_locaux->setProperty("storagePath",locauxPath); m_stack->addWidget(m_locaux);
    QPalette pageColors=''')
s=s.replace('else if(m_stack->currentIndex()==3) m_employees->ajouterEmploye();','else if(m_stack->currentIndex()==3) m_employees->ajouterEmploye();\n        else if(m_stack->currentIndex()==4) m_locaux->addLocal();')
s=s.replace('else if(m_stack->currentIndex()==3) m_employees->focusSearch();','else if(m_stack->currentIndex()==3) m_employees->focusSearch();\n        else if(m_stack->currentIndex()==4) m_locaux->focusSearch();')
s=s.replace('    else if(page==3) m_employees->actualiserStats();','    else if(page==3) m_employees->actualiserStats();\n    else if(page==4) { m_locaux->refreshPage(); m_storageBadge->setToolTip(m_locaux->property("storagePath").toString()); }')
write('src/mainwindow.cpp',s)
print('Sources integrated; originals and data backed up in',backup)
