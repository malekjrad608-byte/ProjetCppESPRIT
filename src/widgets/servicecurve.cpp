#include "servicecurve.h"
#include "../ui/motion.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QToolTip>
#include <QLocale>
#include <cmath>

namespace {
const QStringList names={"Planifié","En cours","Terminé","Annulé"};
const QList<QColor> colors={QColor("#7963BA"),QColor("#D48C23"),QColor("#32956B"),QColor("#B56870")};
}
ServiceCurve::ServiceCurve(QWidget *parent):QWidget(parent) {
    setObjectName("serviceTrendCurve");setMouseTracking(true);setMinimumSize(480,260);
    setAccessibleName("Évolution des services par date et par statut");
    m_animation.setDuration(380);m_animation.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_animation,&QVariantAnimation::valueChanged,this,[this](const QVariant &v){m_progress=v.toDouble();update();});
    connect(&Motion::instance(),&Motion::changed,this,[this](bool enabled){if(!enabled){m_animation.stop();m_progress=1;update();}});
}
QRectF ServiceCurve::plot() const {return QRectF(72,45,qMax(1,width()-102),qMax(1,height()-103));}
void ServiceCurve::setSeries(const QVector<QDate> &dates,const QVector<QVector<double>> &values,const QString &unit) {
    if(m_dates==dates && m_values==values && m_unit==unit)return;
    m_animation.stop();m_dates=dates;m_values=values;m_unit=unit;m_hover=-1;
    m_progress=1;
    if(Motion::instance().enabled()){m_progress=0;m_animation.setStartValue(0.0);m_animation.setEndValue(1.0);m_animation.start();}
    update();
}
void ServiceCurve::paintEvent(QPaintEvent *) {
    QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.fillRect(rect(),Qt::white);
    const auto area=plot();double maximum=0;
    for(const auto &series:m_values)for(double n:series)maximum=qMax(maximum,n);
    const double step=qMax(1.0,std::ceil(maximum/4.0)),top=step*4;
    p.setPen(QColor("#68828C"));p.drawText(QRectF(12,8,width()-24,25),Qt::AlignLeft|Qt::AlignVCenter,m_unit);
    for(int tick=0;tick<=4;++tick){const double y=area.bottom()-area.height()*tick/4;
        p.setPen(QColor("#E4EEF1"));p.drawLine(QPointF(area.left(),y),QPointF(area.right(),y));
        p.setPen(QColor("#68828C"));p.drawText(QRectF(0,y-10,area.left()-10,20),Qt::AlignRight|Qt::AlignVCenter,QLocale(QLocale::French).toString(step*tick,'g',5));}
    if(m_dates.isEmpty()){p.drawText(area,Qt::AlignCenter,"Aucune donnée pour cette période");return;}
    auto x=[&](int i){return m_dates.size()==1?area.center().x():area.left()+i*area.width()/(m_dates.size()-1);};
    const int stride=qMax(1,int(std::ceil(m_dates.size()/6.0)));
    for(int i=0;i<m_dates.size();++i)if(i%stride==0 || i==m_dates.size()-1){
        if(i==m_dates.size()-1 && i%stride!=0 && i%stride<stride/2)continue;
        p.setPen(QColor("#68828C"));p.drawText(QRectF(x(i)-40,area.bottom()+12,80,30),Qt::AlignCenter,m_dates[i].toString("dd/MM/yy"));}
    for(int s=0;s<m_values.size();++s){const auto &v=m_values[s];QPainterPath path;
        for(int i=0;i<v.size();++i){const QPointF pt(x(i),area.bottom()-area.height()*v[i]/top*m_progress);if(i==0)path.moveTo(pt);else path.lineTo(pt);}
        p.setPen(QPen(colors.value(s,QColor("#347C8A")),2.5));p.drawPath(path);
        for(int i=0;i<v.size();++i)if(v.size()<45 || i==m_hover){p.setBrush(Qt::white);p.drawEllipse(QPointF(x(i),area.bottom()-area.height()*v[i]/top*m_progress),3.5,3.5);}
    }
    if(m_hover>=0 && m_hover<m_dates.size()){p.setPen(QPen(QColor("#9BB4BC"),1,Qt::DashLine));p.drawLine(QPointF(x(m_hover),area.top()),QPointF(x(m_hover),area.bottom()));}
}
void ServiceCurve::mouseMoveEvent(QMouseEvent *event){
    const auto area=plot();if(m_dates.isEmpty() || !area.contains(event->position())){m_hover=-1;QToolTip::hideText();update();return;}
    m_hover=m_dates.size()==1?0:qBound(0,qRound((event->position().x()-area.left())/area.width()*(m_dates.size()-1)),int(m_dates.size()-1));
    QString text=m_dates[m_hover].toString("dd/MM/yyyy");
    for(int s=0;s<m_values.size();++s)text+=QString("\n%1 : %2 %3").arg(names.value(s),QLocale(QLocale::French).toString(m_values[s].value(m_hover),'g',6),m_unit);
    QToolTip::showText(event->globalPosition().toPoint(),text,this);update();
}
void ServiceCurve::leaveEvent(QEvent *){m_hover=-1;QToolTip::hideText();update();}
