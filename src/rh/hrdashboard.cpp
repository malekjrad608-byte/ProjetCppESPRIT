#include "hrdashboard.h"
#include <cmath>

namespace AniRh {

namespace {
class TeamArt : public QWidget {
    QTimer m_timer;
    qreal phase=0;
public:
    TeamArt() {
        setFixedSize(260,150); setAttribute(Qt::WA_TransparentForMouseEvents);
        m_timer.setInterval(35); connect(&m_timer,&QTimer::timeout,this,[this]{phase+=0.025; update();});
        connect(&Motion::instance(),&Motion::changed,this,[this](bool on){ if(on && isVisible()) m_timer.start(); else {m_timer.stop();phase=0;update();} });
    }
    void showEvent(QShowEvent *e) override { QWidget::showEvent(e); if(Motion::instance().enabled()) m_timer.start(); }
    void hideEvent(QHideEvent *e) override { QWidget::hideEvent(e); m_timer.stop(); }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.setPen(Qt::NoPen);
        p.setBrush(QColor("#D3E8EB")); p.drawEllipse(QRectF(45,6,150,150));
        p.setBrush(QColor("#F9EACC")); p.drawEllipse(QRectF(191,19+std::sin(phase)*4,34,34));
        p.setPen(QPen(QColor("#C0DDE1"),1)); p.setBrush(Qt::NoBrush); p.drawEllipse(QRectF(22,17,195,130)); p.setPen(Qt::NoPen);
        auto human=[&p](qreal x,qreal y,qreal scale,const QColor &shirt,const QColor &hair){
            p.save(); p.translate(x,y); p.scale(scale,scale); p.setBrush(shirt); p.drawRoundedRect(QRectF(-29,23,58,66),22,22);
            p.setBrush(QColor("#EBCDB6")); p.drawRoundedRect(QRectF(-6,13,12,20),4,4); p.drawEllipse(QRectF(-17,-16,34,40));
            p.setBrush(hair); QPainterPath h; h.moveTo(-17,6); h.cubicTo(-27,-31,22,-33,19,4); h.lineTo(9,-4); h.quadTo(-6,1,-17,-1); p.drawPath(h);
            p.setPen(QPen(QColor("#48626B"),1.5,Qt::SolidLine,Qt::RoundCap)); p.drawLine(-6,8,-5,8); p.drawLine(6,8,7,8); p.drawArc(QRectF(-4,10,9,8),190*16,150*16); p.setPen(Qt::NoPen);
            p.restore();
        };
        human(68,62,0.85,QColor("#E8AF54"),QColor("#475D66")); human(187,64,0.84,QColor("#6DA4AE"),QColor("#6A5950")); human(126,46,1.08,QColor("#377F8C"),QColor("#293F48"));
        p.setBrush(Qt::white); p.drawRoundedRect(QRectF(164,97+std::sin(phase)*3,74,31),9,9); p.setPen(QPen(QColor("#E7A23D"),2.4,Qt::SolidLine,Qt::RoundCap)); p.drawLine(177,113+std::sin(phase)*3,181,117+std::sin(phase)*3); p.drawLine(181,117+std::sin(phase)*3,189,108+std::sin(phase)*3);
        p.setPen(QPen(QColor("#8FADB4"),3,Qt::SolidLine,Qt::RoundCap)); p.drawLine(199,110+std::sin(phase)*3,225,110+std::sin(phase)*3); p.drawLine(199,117+std::sin(phase)*3,217,117+std::sin(phase)*3);
    }
};
class AttendanceChart : public QWidget {
    QList<int> m_present,m_absent;
    QDate m_today;
    qreal progress=0;
public:
    AttendanceChart(HrStore *store):m_today(QDate::fromString(HrStore::today(),Qt::ISODate)) {
        setMinimumHeight(142);
        for(int i=6;i>=0;--i) {
            auto day=m_today.addDays(-i).toString(Qt::ISODate);
            m_present.append(int(store->scalar("SELECT COUNT(*) FROM attendance WHERE day=? AND status<>'Absent'",{day})));
            m_absent.append(int(store->scalar("SELECT COUNT(*) FROM attendance WHERE day=? AND status='Absent'",{day})));
        }
        auto *a=new QVariantAnimation(this); a->setDuration(700); a->setStartValue(0.0); a->setEndValue(1.0); a->setEasingCurve(QEasingCurve::OutCubic);
        connect(a,&QVariantAnimation::valueChanged,this,[this](const QVariant &v){progress=v.toReal();update();});
        connect(&Motion::instance(),&Motion::changed,this,[this,a](bool on){if(!on){a->stop();progress=1;update();}});
        if(Motion::instance().enabled()) a->start(); else progress=1;
    }
    void paintEvent(QPaintEvent *) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); int max=1; for(int i=0;i<7;++i) max=qMax(max,m_present[i]+m_absent[i]);
        const qreal plotH=height()-42.,step=(width()-38.)/7; const int baseline=height()-24;
        p.setFont(QFont("Segoe UI",8));
        for(int j=0;j<3;++j) { qreal y=baseline-plotH*j/2; p.setPen(QPen(QColor("#EDF2F4"),1,Qt::DashLine)); p.drawLine(QPointF(28,y),QPointF(width()-6,y)); p.setPen(QColor("#96A6AD")); p.drawText(QRectF(0,y-8,23,16),Qt::AlignRight|Qt::AlignVCenter,QString::number(max*j/2.0,'f',j==1 && max%2?1:0)); }
        for(int i=0;i<7;++i) {
            qreal x=34+step*i+step*.2,w=step*.45,h=plotH*m_present[i]/max*progress,absent=plotH*m_absent[i]/max*progress;
            p.setPen(Qt::NoPen); p.setBrush(i==6?QColor("#3B8290"):QColor("#A7CCD3")); p.drawRoundedRect(QRectF(x,baseline-h,w,h),4,4);
            p.setBrush(QColor("#E9AD55")); p.drawRoundedRect(QRectF(x,baseline-h-absent,w,absent),3,3);
            p.setPen(QColor("#82979F")); p.drawText(QRectF(x-12,baseline+6,w+24,17),Qt::AlignCenter,QLocale().toString(m_today.addDays(i-6),"ddd"));
        }
    }
};
QWidget *metricCard(const QString &title,const QString &value,const QString &detail,const QString &icon,const QString &tint) {
    auto *card=Ui::card(); auto *l=new QVBoxLayout(card); l->setContentsMargins(19,17,19,17); l->setSpacing(9);
    auto *row=new QHBoxLayout; row->addWidget(Ui::label(title,"muted")); row->addStretch(); auto *i=Ui::label(""); i->setPixmap(Ui::icon(icon,QColor(tint)).pixmap(20,20)); row->addWidget(i); l->addLayout(row);
    auto *number=Ui::label(value); number->setStyleSheet("font-size:28px;font-weight:700;color:#304C56;"); l->addWidget(number); l->addWidget(Ui::label(detail,"muted")); return card;
}
}
Dashboard::Dashboard(HrStore *store,QWidget *parent):QWidget(parent),m_store(store) {
    m_root=new QVBoxLayout(this); m_root->setContentsMargins(30,25,30,25); m_root->setSpacing(20); refresh();
}
void Dashboard::refresh() {
    while(auto *item=m_root->takeAt(0)) { if(item->widget()) delete item->widget(); delete item; }
    auto *header=new QWidget; auto *hl=new QHBoxLayout(header); hl->setContentsMargins(0,0,0,0); auto *ht=new QVBoxLayout; ht->setSpacing(5);
    ht->addWidget(Ui::label("Tableau de bord","title")); ht->addWidget(Ui::label("Votre équipe, au cœur de chaque attention.","muted")); hl->addLayout(ht); hl->addStretch();
    auto *add=Ui::button("Ajouter un employé","plus","primary"); add->setObjectName("dashboardAdd"); hl->addWidget(add); connect(add,&QPushButton::clicked,this,&Dashboard::addEmployeeRequested); m_root->addWidget(header);
    auto *hero=Ui::card("hero"); auto *heroLayout=new QHBoxLayout(hero); heroLayout->setContentsMargins(25,16,20,16); auto *heroText=new QVBoxLayout; heroText->setSpacing(8);
    auto *eyebrow=Ui::label("L'HUMAIN FAIT LA DIFFÉRENCE","eyebrow"); eyebrow->setStyleSheet("color:#5F98A3;font-size:10px;letter-spacing:1.5px;font-weight:600;"); heroText->addWidget(eyebrow);
    auto *headline=Ui::label("Une équipe épanouie.\nDes animaux bien entourés."); headline->setStyleSheet("font-size:23px;font-weight:700;color:#2C626F;"); heroText->addWidget(headline);
    heroText->addWidget(Ui::label("Présences, congés et talents : tout se retrouve ici.","muted")); heroText->addStretch(); heroLayout->addLayout(heroText,1); heroLayout->addWidget(new TeamArt); m_root->addWidget(hero);
    const QString today=HrStore::today(),month=today.left(7);
    const int total=int(m_store->scalar("SELECT COUNT(*) FROM employees WHERE active=1 AND hired<=?",{today}));
    const int present=int(m_store->scalar("SELECT COUNT(*) FROM attendance a JOIN employees e ON e.id=a.employee_id WHERE e.active=1 AND a.day=? AND a.status<>'Absent'",{today}));
    const int pending=int(m_store->scalar("SELECT COUNT(*) FROM leaves WHERE status='En attente'"));
    const int tasks=int(m_store->scalar("SELECT COUNT(*) FROM tasks WHERE status<>'Terminé'"));
    auto *metrics=new QWidget; auto *ml=new QHBoxLayout(metrics); ml->setContentsMargins(0,0,0,0); ml->setSpacing(14);
    ml->addWidget(metricCard("Effectif actif",QString::number(total),"Collaborateurs en poste","users","#4D929E"));
    ml->addWidget(metricCard("Présents aujourd'hui",QString::number(present),total?QString("%1 % de l'effectif actif").arg(qRound(100.0*present/total)):"Aucun pointage pour le moment","clock","#739C87"));
    ml->addWidget(metricCard("Congés à valider",QString::number(pending),"Demandes en attente","calendar","#D99D44"));
    ml->addWidget(metricCard("Missions ouvertes",QString::number(tasks),"À faire et en cours","tasks","#9488B2")); m_root->addWidget(metrics);
    auto *panels=new QWidget; auto *pl=new QHBoxLayout(panels); pl->setContentsMargins(0,0,0,0); pl->setSpacing(18);
    auto *chart=Ui::card(); auto *cl=new QVBoxLayout(chart); cl->setContentsMargins(20,18,20,16); cl->setSpacing(10);
    auto *ct=new QHBoxLayout; ct->addWidget(Ui::label("Le rythme de l'équipe","section")); ct->addStretch(); ct->addWidget(Ui::label("7 derniers jours","muted")); cl->addLayout(ct);
    cl->addWidget(Ui::label("Pointages enregistrés · tous les collaborateurs","muted")); cl->addWidget(new AttendanceChart(m_store),1);
    auto *legend=Ui::label("●  Présence / retard / télétravail      ●  Absence déclarée","muted"); legend->setStyleSheet("font-size:10px;color:#79949C;"); cl->addWidget(legend); pl->addWidget(chart,3);
    auto *requests=Ui::card(); auto *rl=new QVBoxLayout(requests); rl->setContentsMargins(20,18,20,16); rl->setSpacing(9);
    auto *rt=new QHBoxLayout; rt->addWidget(Ui::label("À votre attention","section")); rt->addStretch(); auto *all=Ui::button("Voir tout","arrow","link"); rt->addWidget(all); connect(all,&QPushButton::clicked,this,[this]{emit navigateRequested(3);}); rl->addLayout(rt);
    const auto leaves=m_store->rows("SELECT l.*,e.first_name||' '||e.last_name AS name FROM leaves l JOIN employees e ON e.id=l.employee_id WHERE l.status='En attente' ORDER BY start LIMIT 3");
    if(leaves.isEmpty()) { auto *empty=Ui::label("Tout est à jour.\nAucune demande de congé en attente.","muted"); empty->setWordWrap(true); rl->addWidget(empty,1); }
    else for(const auto &r:leaves) {
        auto *entry=new QWidget; auto *el=new QHBoxLayout(entry); el->setContentsMargins(0,3,0,3);
        el->addWidget(Ui::person(r["name"].toString(),Ui::date(r["start"].toString())+" · "+r["days"].toString()+" jours",r["employee_id"].toInt()),1);
        auto *action=Ui::button("Traiter",{},"toggle"); connect(action,&QPushButton::clicked,this,[this]{emit navigateRequested(3);}); el->addWidget(action); rl->addWidget(entry);
    }
    rl->addStretch(); auto *pay=Ui::label(QString("%1 bulletin(s) de paie en brouillon ce mois-ci").arg(m_store->scalar("SELECT COUNT(*) FROM payroll WHERE month=? AND status='Brouillon'",{month})),"muted"); pay->setWordWrap(true); pay->setStyleSheet("background:#FFF6E6;color:#A68143;padding:11px;border-radius:8px;font-size:11px;"); rl->addWidget(pay); pl->addWidget(requests,2); m_root->addWidget(panels);
    auto *bottom=Ui::card(); auto *bl=new QHBoxLayout(bottom); bl->setContentsMargins(20,13,20,13);
    const int overdue=int(m_store->scalar("SELECT COUNT(*) FROM tasks WHERE status<>'Terminé' AND due<?",{today}));
    auto *bt=new QVBoxLayout; bt->setSpacing(4); bt->addWidget(Ui::label(overdue?QString("%1 mission(s) à relancer").arg(overdue):"Prêt pour une nouvelle journée ?","section")); bt->addWidget(Ui::label(overdue?"Ces échéances sont dépassées. Faites le point avec les responsables.":"Attribuez les responsabilités et accompagnez les progrès de votre équipe.","muted")); bl->addLayout(bt,1);
    auto *view=Ui::button("Voir les missions","arrow"); bl->addWidget(view); connect(view,&QPushButton::clicked,this,[this]{emit navigateRequested(4);}); m_root->addWidget(bottom); m_root->addStretch();
}

} // namespace AniRh
