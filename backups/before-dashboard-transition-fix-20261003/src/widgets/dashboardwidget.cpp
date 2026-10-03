#include "dashboardwidget.h"
#include "../database/databasemanager.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPainter>
#include <QVariantAnimation>
#include <QTimer>
#include <QtMath>
#include <QEvent>
#include <algorithm>

namespace {
class Mascot : public QWidget {
public:
    explicit Mascot(QWidget *parent=nullptr):QWidget(parent),m_logo(Design::brand("logo",QSize(246,246))),m_motion(new QVariantAnimation(this)) {
        setFixedSize(214,146); m_motion->setDuration(1000);
        m_motion->setStartValue(0.0); m_motion->setEndValue(1.0);
        connect(m_motion,&QVariantAnimation::valueChanged,this,[this](const QVariant &v) { m_phase=v.toReal(); update(); });
        connect(&Motion::instance(),&Motion::changed,this,[this](bool enabled) { if(!enabled) { m_motion->stop(); m_phase=1; update(); } });
    }
protected:
    bool event(QEvent *event) override {
        if((event->type()==QEvent::Show || event->type()==QEvent::Enter) && Motion::instance().enabled()) { m_motion->stop(); m_motion->start(); }
        if(event->type()==QEvent::Hide) { m_motion->stop(); m_phase=1; }
        return QWidget::event(event);
    }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.setRenderHint(QPainter::SmoothPixmapTransform); p.setPen(Qt::NoPen); p.scale(0.92,0.92);
        p.setBrush(QColor("#CEE2E7")); p.drawEllipse(QRectF(46,-12,178,178));
        p.setBrush(Qt::white); p.drawEllipse(QRectF(65,6,141,141));
        p.drawPixmap(QRect(74,15,123,123),m_logo);
        p.translate(0,-4*qSin(m_phase*2*M_PI));
        p.setBrush(QColor("#F9E5BF")); p.drawEllipse(QRectF(26,100,37,37));
        Design::icon("heart",QColor("#A48142")).paint(&p,QRect(34,109,21,21));
        Design::icon("paw",QColor("#7DA189")).paint(&p,QRect(214,15,15,15));
        p.setBrush(QColor("#B0CCB3")); p.drawEllipse(QRectF(35,30,6,6));
    }
private:
    QPixmap m_logo;
    QVariantAnimation *m_motion;
    qreal m_phase=1;
};
}
DashboardWidget::DashboardWidget(QWidget *parent):QWidget(parent) {
    setObjectName("dashboardPage");
    auto *root=new QVBoxLayout(this); root->setContentsMargins(28,24,28,20); root->setSpacing(16);
    auto *heading=new QHBoxLayout; auto *titles=new QVBoxLayout; titles->setSpacing(5);
    titles->addWidget(Design::label("Tableau de bord","title")); titles->addWidget(Design::label("Une belle journée pour prendre soin d'eux.","muted")); heading->addLayout(titles); heading->addStretch();
    auto *add=Design::button("Nouveau service","plus","primary"); add->setObjectName("dashboardAdd"); heading->addWidget(add); connect(add,&QPushButton::clicked,this,&DashboardWidget::createRequested); root->addLayout(heading);
    auto *hero=Design::card("hero"); auto *heroLayout=new QHBoxLayout(hero); heroLayout->setContentsMargins(26,12,20,12);
    auto *heroText=new QVBoxLayout; heroText->setSpacing(7); heroText->addWidget(Design::label("CHAQUE ATTENTION COMPTE","eyebrow"));
    auto *headline=Design::label("De petits soins.\nDe grands moments de bonheur."); headline->setStyleSheet("font-size:25px;font-weight:650;color:#285F6D;"); headline->setWordWrap(true); heroText->addWidget(headline);
    auto *sub=Design::label("Tous vos services, réunis pour leur bien-être.","muted"); sub->setWordWrap(true); heroText->addWidget(sub); heroLayout->addLayout(heroText,1); heroLayout->addWidget(new Mascot); root->addWidget(hero);
    auto *stats=new QHBoxLayout; stats->setSpacing(14);
    const QStringList names={"Total des services","Services planifiés","En cours","Services terminés"};
    const QStringList hints={"Toutes les prestations","À organiser","En prise en charge","Soins réalisés"};
    const QStringList symbols={"services","calendar","clock","check"};
    const QStringList colors={"#E9F0E7","#EEF0FA","#FFF2DA","#E7F3EC"};
    for(int i=0;i<4;++i) {
        auto *card=Design::card(); auto *layout=new QVBoxLayout(card); layout->setContentsMargins(17,11,17,11); layout->setSpacing(4);
        auto *first=new QHBoxLayout; auto *title=Design::label(names[i],"muted"); title->setWordWrap(true); first->addWidget(title,1);
        auto *symbol=Design::label(""); symbol->setPixmap(Design::icon(symbols[i]).pixmap(18,18)); symbol->setAlignment(Qt::AlignCenter); symbol->setFixedSize(30,30); symbol->setStyleSheet("background:"+colors[i]+";border-radius:9px;"); first->addWidget(symbol); layout->addLayout(first);
        auto *value=Design::label("0"); value->setObjectName("stat"+QString::number(i)); value->setStyleSheet("font-size:29px;font-weight:650;color:#285F6D;"); layout->addWidget(value); m_values.append(value);
        layout->addWidget(Design::label(hints[i],"muted")); stats->addWidget(card,1);
    }
    root->addLayout(stats);
    auto *lower=new QHBoxLayout; lower->setSpacing(18);
    auto *agenda=Design::card(); auto *al=new QVBoxLayout(agenda); al->setContentsMargins(0,0,0,9); al->setSpacing(8);
    auto *ah=new QHBoxLayout; ah->setContentsMargins(20,16,19,4); auto *at=new QVBoxLayout; at->setSpacing(3);
    at->addWidget(Design::label("Les services du jour","section")); m_todayCount=Design::label("","muted"); at->addWidget(m_todayCount); ah->addLayout(at); ah->addStretch();
    auto *all=Design::button("Tout voir  →",{},"link"); connect(all,&QPushButton::clicked,this,&DashboardWidget::servicesRequested); ah->addWidget(all); al->addLayout(ah);
    m_table=new QTableWidget; m_table->setObjectName("todayTable"); Design::setupTable(m_table,{"SERVICE","HORAIRE","STATUT"});
    m_table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch); m_table->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed); m_table->setColumnWidth(1,95); m_table->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Fixed); m_table->setColumnWidth(2,128);
    m_table->setMinimumHeight(248); al->addWidget(m_table,1); connect(m_table,&QTableWidget::cellDoubleClicked,this,[this](int row,int){emit serviceRequested(m_table->item(row,0)->data(Qt::UserRole).toInt());});
    m_empty=new QWidget; auto *emptyLayout=new QVBoxLayout(m_empty); emptyLayout->setContentsMargins(30,24,30,24); emptyLayout->setSpacing(10);
    auto *emptyIcon=Design::label(""); emptyIcon->setPixmap(Design::icon("calendar",QColor("#8FAB95")).pixmap(38,38)); emptyIcon->setAlignment(Qt::AlignCenter); emptyLayout->addWidget(emptyIcon);
    auto *emptyTitle=Design::label("Une journée pleine de possibilités","section"); emptyTitle->setAlignment(Qt::AlignCenter); emptyLayout->addWidget(emptyTitle);
    auto *emptyText=Design::label("Ajoutez votre premier service et retrouvez ici\nles prestations prévues aujourd'hui.","muted"); emptyText->setAlignment(Qt::AlignCenter); emptyText->setWordWrap(true); emptyLayout->addWidget(emptyText);
    auto *first=Design::button("Planifier un service","plus"); connect(first,&QPushButton::clicked,this,&DashboardWidget::createRequested); emptyLayout->addWidget(first,0,Qt::AlignCenter); al->addWidget(m_empty,1); lower->addWidget(agenda,2);
    auto *right=new QVBoxLayout; right->setSpacing(14); auto *distribution=Design::card(); auto *dl=new QVBoxLayout(distribution); dl->setContentsMargins(18,16,18,17); dl->setSpacing(7);
    dl->addWidget(Design::label("Nos prestations","section")); dl->addWidget(Design::label("Répartition par catégorie","muted"));
    for(const auto &category:Design::categories()) {
        auto *line=new QHBoxLayout; auto *title=Design::label(category); title->setStyleSheet("font-size:11px;"); line->addWidget(title); line->addStretch(); auto *count=Design::label("0","muted"); m_categoryCounts.append(count); line->addWidget(count); dl->addLayout(line);
        auto *bar=new QProgressBar; bar->setRange(0,100); bar->setTextVisible(false); m_bars.append(bar); dl->addWidget(bar);
    }
    right->addWidget(distribution,1);
    auto *care=Design::card("careCard"); auto *cl=new QHBoxLayout(care); cl->setContentsMargins(16,13,16,13); auto *heart=Design::label(""); heart->setPixmap(Design::icon("heart",QColor("#AC853F")).pixmap(26,26)); cl->addWidget(heart);
    m_completion=Design::label("Chaque soin fait la différence."); m_completion->setWordWrap(true); m_completion->setStyleSheet("font-size:12px;color:#967539;"); cl->addWidget(m_completion,1); right->addWidget(care);
    lower->addLayout(right,1); root->addLayout(lower,1);
    auto *footer=new QHBoxLayout; footer->addWidget(Design::label("Pensé pour eux. Conçu pour vous.","muted")); footer->addStretch(); footer->addWidget(Design::label("ANIWORLD  •  Smart Pet Care","muted")); root->addLayout(footer);
    refresh();
}
void DashboardWidget::refresh() {
    const auto services=DatabaseManager::instance().getAllServices();
    int planned=0,active=0,done=0; QList<Service> today;
    QMap<QString,int> counts;
    for(const auto &s:services) {
        planned+=s.status=="Planifié"; active+=s.status=="En cours"; done+=s.status=="Terminé"; ++counts[s.category];
        if(s.dateTime.date()==QDate::currentDate()) today.append(s);
    }
    const QList<int> values={int(services.size()),planned,active,done};
    for(int i=0;i<values.size();++i) Motion::countTo(m_values[i],values[i]);
    const auto categories=Design::categories();
    for(int i=0;i<categories.size();++i) { const int n=counts.value(categories[i]); m_categoryCounts[i]->setText(QString::number(n)); Motion::progressTo(m_bars[i],services.isEmpty()?0:100*n/services.size()); }
    std::sort(today.begin(),today.end(),[](const Service &a,const Service &b){return a.dateTime<b.dateTime;});
    m_table->setRowCount(today.size());
    for(int i=0;i<today.size();++i) {
        const auto &s=today[i]; auto *name=new QTableWidgetItem(s.name); name->setData(Qt::UserRole,s.id); name->setToolTip(s.name+"\n"+s.category+" · "+s.responsible+"\nDouble-cliquez pour consulter"); m_table->setItem(i,0,name);
        m_table->setItem(i,1,new QTableWidgetItem(s.dateTime.toString("HH:mm"))); m_table->setItem(i,2,new QTableWidgetItem); m_table->setCellWidget(i,2,Design::badge(s.status)); m_table->setRowHeight(i,50);
    }
    m_table->setVisible(!today.isEmpty()); m_empty->setVisible(today.isEmpty());
    m_todayCount->setText(QString("%1 prestation%2 prévue%2 aujourd'hui").arg(today.size()).arg(today.size()==1?"":"s"));
    m_completion->setText(done?QString("%1 soin%2 terminé%2.\nAutant d'attentions qui comptent.").arg(done).arg(done==1?"":"s"):"Chaque soin fait\nla différence.");
}
void DashboardWidget::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    QTimer::singleShot(0,this,[this] {
        if(!isVisible()) return;
        for(auto *label:m_values) Motion::countTo(label,label->property("motionTarget").toInt(),true);
        for(auto *bar:m_bars) Motion::progressTo(bar,bar->property("motionTarget").toInt(),true);
    });
}
void DashboardWidget::hideEvent(QHideEvent *event) {
    QWidget::hideEvent(event);
    for(auto *label:m_values) Motion::countTo(label,label->property("motionTarget").toInt());
    for(auto *bar:m_bars) Motion::progressTo(bar,bar->property("motionTarget").toInt());
}
