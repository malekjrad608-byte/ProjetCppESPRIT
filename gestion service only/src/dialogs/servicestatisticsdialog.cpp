#include "servicestatisticsdialog.h"
#include "../widgets/servicecurve.h"
#include "../database/databasemanager.h"
#include "../ui/design.h"
#include <QDateEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QScreen>
#include <QSignalBlocker>
#include <QSet>
#include <algorithm>

ServiceStatisticsDialog::ServiceStatisticsDialog(QWidget *parent):QDialog(parent){
    setObjectName("serviceStatisticsDialog");setWindowTitle("ANIWORLD — Évolution des services");
    resize(QSize(980,680).boundedTo(screen()->availableGeometry().size()-QSize(50,70)));
    auto *root=new QVBoxLayout(this);root->setContentsMargins(24,20,24,20);root->setSpacing(14);
    root->addWidget(Design::label("Évolution des services","title"));
    root->addWidget(Design::label("Choisissez une période et une mesure. Survolez les points pour voir leurs valeurs.","muted"));
    auto *filters=new QGridLayout;filters->setHorizontalSpacing(12);
    m_from=new QDateEdit;m_to=new QDateEdit;
    m_from->setObjectName("statisticsFrom");m_to->setObjectName("statisticsTo");
    for(auto *date:{m_from,m_to}){date->setCalendarPopup(true);date->setDisplayFormat("dd/MM/yyyy");date->setMinimumWidth(145);}
    m_category=new QComboBox;m_category->setObjectName("statisticsCategory");
    m_metric=new QComboBox;m_metric->setObjectName("statisticsMetric");m_metric->addItems({"Nombre de services","Montant des prestations (DT)","Durée des prestations (h)"});
    m_group=new QComboBox;m_group->setObjectName("statisticsGrouping");m_group->addItems({"Par jour","Par semaine","Par mois"});
    filters->addWidget(Design::label("Du"),0,0);filters->addWidget(m_from,0,1);
    filters->addWidget(Design::label("Au"),0,2);filters->addWidget(m_to,0,3);
    filters->addWidget(Design::label("Catégorie"),0,4);filters->addWidget(m_category,0,5);
    filters->addWidget(Design::label("Mesure"),1,0);filters->addWidget(m_metric,1,1,1,3);filters->addWidget(m_group,1,4,1,2);
    filters->setColumnStretch(5,1);root->addLayout(filters);
    m_summary=Design::label("","section");m_summary->setObjectName("statisticsSummary");m_summary->setWordWrap(true);root->addWidget(m_summary);
    m_curve=new ServiceCurve;root->addWidget(m_curve,1);
    auto *legend=new QHBoxLayout;const QStringList names={"Planifié","En cours","Terminé","Annulé"},colors={"#7963BA","#D48C23","#32956B","#B56870"};
    for(int i=0;i<4;++i){auto *l=Design::label("●  "+names[i]);l->setStyleSheet("color:"+colors[i]+";font-weight:600;");legend->addWidget(l);}legend->addStretch();root->addLayout(legend);
    auto *note=Design::label("Montants des prestations, sans suivi des encaissements. Les annulations sont tracées séparément.","muted");note->setWordWrap(true);root->addWidget(note);
    m_notice=Design::label("","muted");m_notice->setObjectName("statisticsNotice");m_notice->setWordWrap(true);root->addWidget(m_notice);
    auto *buttons=new QHBoxLayout;auto *refresh=Design::button("Actualiser");refresh->setObjectName("statisticsRefresh");buttons->addWidget(refresh);buttons->addStretch();
    auto *close=Design::button("Fermer",{},"primary");buttons->addWidget(close);root->addLayout(buttons);
    connect(close,&QPushButton::clicked,this,&QDialog::accept);connect(refresh,&QPushButton::clicked,this,[this]{reload();});
    connect(m_from,&QDateEdit::dateChanged,this,[this](QDate date){if(date>m_to->date()){QSignalBlocker b(m_to);m_to->setDate(date);}rebuild();});
    connect(m_to,&QDateEdit::dateChanged,this,[this](QDate date){if(date<m_from->date()){QSignalBlocker b(m_from);m_from->setDate(date);}rebuild();});
    for(auto *combo:{m_category,m_metric,m_group})connect(combo,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{rebuild();});
    reload(true);
    auto *timer=new QTimer(this);timer->setObjectName("statisticsLiveRefresh");timer->setInterval(15000);connect(timer,&QTimer::timeout,this,[this]{reload();});timer->start();
}
void ServiceStatisticsDialog::reload(bool initial){
    auto &db=DatabaseManager::instance();const auto fresh=db.getAllServices();
    if(!db.lastError().isEmpty()){m_notice->setText("Actualisation impossible : "+db.lastError()+" — les dernières données restent affichées.");return;}
    m_services=fresh;QSet<QString> categories;
    QDate first,last;for(const auto &s:m_services){categories.insert(s.getCategory());const auto d=s.getDateTime().date();if(d.isValid()){if(!first.isValid()||d<first)first=d;if(!last.isValid()||d>last)last=d;}}
    const QString selected=m_category->currentData().toString();
    {QSignalBlocker b(m_category);m_category->clear();m_category->addItem("Toutes les catégories",QString());auto names=categories.values();std::sort(names.begin(),names.end());for(const auto &name:names)m_category->addItem(name,name);m_category->setCurrentIndex(qMax(0,m_category->findData(selected)));}
    if(initial){QSignalBlocker a(m_from),b(m_to);m_to->setDate(last.isValid()?last:QDate::currentDate());m_from->setDate(first.isValid()?qMax(first,m_to->date().addDays(-29)):m_to->date().addDays(-29));}
    rebuild();m_notice->setText("Mis à jour à "+QTime::currentTime().toString("HH:mm:ss")+" · Actualisation automatique toutes les 15 secondes.");
}
void ServiceStatisticsDialog::rebuild(){
    const auto from=m_from->date(),to=m_to->date();int grouping=m_group->currentIndex();
    // Keep long ranges readable and bounded even for several years of records.
    if(grouping==0 && from.daysTo(to)>365){QSignalBlocker b(m_group);m_group->setCurrentIndex(2);grouping=2;}
    if(grouping==1 && from.daysTo(to)>2555){QSignalBlocker b(m_group);m_group->setCurrentIndex(2);grouping=2;}
    auto bucket=[grouping](QDate d){return grouping==1?d.addDays(1-d.dayOfWeek()):grouping==2?QDate(d.year(),d.month(),1):d;};
    QVector<QDate> dates;QMap<QDate,int> indices;
    for(auto d=bucket(from);d<=bucket(to);d=grouping==2?d.addMonths(1):d.addDays(grouping==1?7:1)){indices[d]=dates.size();dates<<d;}
    QVector<QVector<double>> series(4,QVector<double>(dates.size(),0));const QStringList statuses={"Planifié","En cours","Terminé","Annulé"};
    int count=0;double total=0;const auto category=m_category->currentData().toString();
    for(const auto &s:m_services){const auto d=s.getDateTime().date();if(!d.isValid()||d<from||d>to||(!category.isEmpty()&&s.getCategory()!=category))continue;
        const int status=statuses.indexOf(s.getStatus());if(status<0)continue;
        const double value=m_metric->currentIndex()==0?1:m_metric->currentIndex()==1?s.getPrice():s.getDuration()/60.0;
        series[status][indices[bucket(d)]]+=value;total+=value;++count;
    }
    const QString unit=m_metric->currentIndex()==0?"services":m_metric->currentIndex()==1?"DT":"heures";
    m_curve->setSeries(dates,series,unit);
    const auto amount=m_metric->currentIndex()==0?QString():QString(" · %1 %2").arg(QLocale(QLocale::French).toString(total,'g',7),unit);
    m_summary->setText(count?QString("%1 services%2 · du %3 au %4").arg(count).arg(amount,from.toString("dd/MM/yyyy"),to.toString("dd/MM/yyyy")):"Aucun service pour cette période et cette catégorie.");
}
