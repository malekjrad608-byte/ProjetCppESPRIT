#include "mainwindow.h"
#include "ui/design.h"
#include "database/databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScreen>
#include <QApplication>
#include <QTimer>
#include <QShortcut>
#include <QLocale>

MainWindow::MainWindow(bool demo,QWidget *parent):QMainWindow(parent) {
    setWindowTitle(demo?"ANIWORLD — Démonstration":"ANIWORLD — Gestion des services");
    setMinimumSize(1100,700);
    const QSize available=QApplication::primaryScreen()->availableGeometry().size();
    resize(qMin(1400,available.width()-40),qMin(900,available.height()-60));
    auto *central=new QWidget; setCentralWidget(central);
    auto *root=new QHBoxLayout(central); root->setContentsMargins(0,0,0,0); root->setSpacing(0);
    auto *side=new QFrame; side->setObjectName("sidebar"); side->setFixedWidth(228);
    auto *sidebar=new QVBoxLayout(side); sidebar->setContentsMargins(17,20,17,17); sidebar->setSpacing(3);
    auto *wordmark=Design::label(""); wordmark->setPixmap(Design::brand("wordmark",QSize(178,74))); wordmark->setAlignment(Qt::AlignCenter); wordmark->setFixedHeight(74);
    sidebar->addWidget(wordmark); sidebar->addSpacing(12);
    auto *workspace=Design::card("workspace"); auto *wl=new QHBoxLayout(workspace); wl->setContentsMargins(12,11,10,11);
    auto *paw=Design::label(""); paw->setPixmap(Design::icon("paw").pixmap(23,23)); paw->setFixedSize(30,30); wl->addWidget(paw);
    auto *wt=new QVBoxLayout; wt->setSpacing(3); wt->addWidget(Design::label("Centre de soins",nullptr)); wt->addWidget(Design::label("Espace de gestion","muted")); wl->addLayout(wt); sidebar->addWidget(workspace);
    sidebar->addSpacing(16); sidebar->addWidget(Design::label("  ESPACE PRINCIPAL","eyebrow")); sidebar->addSpacing(5);
    const QStringList labels={"Tableau de bord","Services"}; const QStringList icons={"grid","services"};
    for(int i=0;i<labels.size();++i) {
        auto *button=Design::button("  "+labels[i],icons[i],"nav"); button->setCheckable(true); button->setObjectName(i==0?"navDashboard":"navServices");
        m_navigation.append(button); sidebar->addWidget(button); connect(button,&QPushButton::clicked,this,[this,i]{navigate(i);});
    }
    sidebar->addSpacing(16); sidebar->addWidget(Design::label("  PROCHAINS MODULES","eyebrow")); sidebar->addSpacing(5);
    const QStringList future={"Animaux & dossiers","Propriétaires","Rendez-vous","Vétérinaires","Employés & accès"};
    const QStringList fi={"paw","users","calendar","heart","shield"};
    for(int i=0;i<future.size();++i) {
        auto *button=Design::button("  "+future[i],fi[i],"nav"); button->setEnabled(false);
        button->setToolTip(i==0?"Animaux et dossiers médicaux réunis dans un futur module.":"Module prévu dans le projet global."); sidebar->addWidget(button);
    }
    sidebar->addStretch(1);
    auto *mission=Design::label("Leur bien-être,\nnotre plus belle mission."); mission->setStyleSheet("font-size:14px;font-weight:600;color:#638172;padding:12px 9px;"); sidebar->addWidget(mission);
    sidebar->addWidget(Design::label("  ANIWORLD  /  version 2.0","eyebrow")); root->addWidget(side);
    auto *right=new QVBoxLayout; right->setContentsMargins(0,0,0,0); right->setSpacing(0); root->addLayout(right,1);
    auto *top= new QFrame; top->setObjectName("topbar"); top->setFixedHeight(64);
    auto *topLayout=new QHBoxLayout(top); topLayout->setContentsMargins(28,0,28,0);
    m_breadcrumb=Design::label("Espace de gestion   /   Tableau de bord","muted"); topLayout->addWidget(m_breadcrumb); topLayout->addStretch();
    auto *date=Design::label("","muted");
    auto updateDate=[date]{ date->setText(QLocale().toString(QDate::currentDate(),"dddd d MMMM yyyy")); };
    updateDate(); auto *timer=new QTimer(this); connect(timer,&QTimer::timeout,this,updateDate); timer->start(60000);
    topLayout->addWidget(date); topLayout->addSpacing(18);
    auto *mode=Design::label(demo?"●  Démo · non enregistré":"●  Sauvegarde locale");
    mode->setStyleSheet("background:#EDF5EB;color:#4F805C;border-radius:12px;padding:6px 11px;font-size:11px;");
    mode->setToolTip(demo?"Données fictives. Les modifications disparaissent à la fermeture.":DatabaseManager::instance().storagePath());
    topLayout->addWidget(mode,0,Qt::AlignVCenter); right->addWidget(top);
    m_stack=new QStackedWidget; right->addWidget(m_stack,1);
    m_dashboard=new DashboardWidget; auto *scroll=new QScrollArea; scroll->setWidgetResizable(true); scroll->setWidget(m_dashboard); scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_services=new ServiceWidget; m_stack->addWidget(scroll); m_stack->addWidget(m_services);
    connect(m_dashboard,&DashboardWidget::createRequested,this,[this]{navigate(1);m_services->onAdd();});
    connect(m_dashboard,&DashboardWidget::servicesRequested,this,[this]{navigate(1);});
    connect(m_dashboard,&DashboardWidget::serviceRequested,this,[this](int id){navigate(1);m_services->viewService(id);});
    connect(m_services,&ServiceWidget::servicesChanged,m_dashboard,&DashboardWidget::refresh);
    auto *addShortcut=new QShortcut(QKeySequence("Ctrl+N"),this); connect(addShortcut,&QShortcut::activated,this,[this]{navigate(1);m_services->onAdd();});
    auto *searchShortcut=new QShortcut(QKeySequence("Ctrl+F"),this); connect(searchShortcut,&QShortcut::activated,this,[this]{navigate(1);m_services->focusSearch();});
    navigate(0);
}
void MainWindow::navigate(int page) {
    if(page<0 || page>=m_stack->count()) return;
    m_stack->setCurrentIndex(page);
    for(int i=0;i<m_navigation.size();++i) m_navigation[i]->setChecked(i==page);
    m_breadcrumb->setText("Espace de gestion   /   "+QString(page==0?"Tableau de bord":"Services"));
    if(page==0) m_dashboard->refresh(); else m_services->refresh();
}
