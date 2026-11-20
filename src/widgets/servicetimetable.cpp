#include "servicetimetable.h"
#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>
#include <algorithm>

namespace {
QColor color(const QString &status){return status=="Terminé"?QColor("#DFF3E9"):status=="En cours"?QColor("#FFF0D4"):status=="Annulé"?QColor("#F8E4E7"):QColor("#ECE7FA");}
}
ServiceTimetable::ServiceTimetable(QWidget *parent):QWidget(parent){setObjectName("serviceWeekTimetable");setMinimumWidth(760);setFixedHeight(24*HourHeight+1);setMouseTracking(true);setAccessibleName("Emploi du temps hebdomadaire, de 0 h à 24 h");}
void ServiceTimetable::setAppointments(QDate monday,const QList<Service::Appointment> &entries,const QMap<int,QString> &names){m_monday=monday;m_entries=entries;m_names=names;m_selected=-1;update();}
void ServiceTimetable::selectEntry(int index){m_selected=index;update();}
int ServiceTimetable::firstHour() const {int hour=8;for(const auto &a:m_entries)hour=qMin(hour,a.start.date()<m_monday?0:a.start.time().hour());return hour;}
QList<ServiceTimetable::Block> ServiceTimetable::blocks() const {
    QList<Block> result;const double dayWidth=(width()-64.0)/7;
    struct Segment {int index;double start,end;int lane=0;};
    for(int day=0;day<7;++day){const QDate date=m_monday.addDays(day);const QDateTime start(date,QTime(0,0)),end(date.addDays(1),QTime(0,0));QList<Segment> segments;
        for(int i=0;i<m_entries.size();++i){const auto &a=m_entries[i];if(a.start>=end||a.finish<=start||a.finish<=a.start)continue;
            const double y=a.start<start?0:(a.start.time().hour()*60+a.start.time().minute())*HourHeight/60.0;
            const double bottom=a.finish>=end?24*HourHeight:(a.finish.time().hour()*60+a.finish.time().minute())*HourHeight/60.0;
            segments.append({i,y,qMin(double(24*HourHeight),qMax(y+12,bottom))});}
        std::sort(segments.begin(),segments.end(),[](const Segment &a,const Segment &b){return a.start==b.start?a.end<b.end:a.start<b.start;});
        int first=0;
        while(first<segments.size()){int last=first+1;double groupEnd=segments[first].end;
            while(last<segments.size()&&segments[last].start<groupEnd){groupEnd=qMax(groupEnd,segments[last].end);++last;}
            QVector<double> laneEnds;
            for(int i=first;i<last;++i){int lane=0;while(lane<laneEnds.size()&&laneEnds[lane]>segments[i].start)++lane;if(lane==laneEnds.size())laneEnds<<0;laneEnds[lane]=segments[i].end;segments[i].lane=lane;}
            const double laneWidth=(dayWidth-6)/qMax(1,int(laneEnds.size()));
            for(int i=first;i<last;++i){const auto &s=segments[i];result.append({QRectF(64+day*dayWidth+3+s.lane*laneWidth,s.start+1,qMax(2.0,laneWidth-2),qMax(4.0,s.end-s.start-2)),s.index,day});}
            first=last;
        }
    }return result;
}
void ServiceTimetable::paintEvent(QPaintEvent *){
    QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.fillRect(rect(),Qt::white);const double dw=(width()-64.0)/7;
    for(int day=0;day<7;++day){if(m_monday.addDays(day)==QDate::currentDate())p.fillRect(QRectF(64+day*dw,0,dw,height()),QColor("#F0F8FA"));else if(day>=5)p.fillRect(QRectF(64+day*dw,0,dw,height()),QColor("#F8FAFB"));}
    for(int h=0;h<=24;++h){p.setPen(QColor("#DBE7EB"));p.drawLine(64,h*HourHeight,width(),h*HourHeight);if(h<24){p.setPen(QColor("#718B94"));p.drawText(QRect(0,h*HourHeight+3,56,22),Qt::AlignRight,QString("%1:00").arg(h,2,10,QChar('0')));p.setPen(QPen(QColor("#EEF3F5"),1,Qt::DashLine));p.drawLine(64,h*HourHeight+HourHeight/2,width(),h*HourHeight+HourHeight/2);}}
    for(int day=0;day<=7;++day){p.setPen(QColor("#DFE9ED"));p.drawLine(QPointF(64+day*dw,0),QPointF(64+day*dw,height()));}
    for(const auto &b:blocks()){const auto &a=m_entries[b.entry];p.setBrush(color(a.status));p.setPen(QPen(b.entry==m_selected?QColor("#E58B26"):QColor("#B3C8D0"),b.entry==m_selected?2.5:1));p.drawRoundedRect(b.rect,4,4);
        p.save();p.setClipRect(b.rect.adjusted(4,2,-3,-2));p.setPen(QColor("#285464"));QFont f=p.font();f.setPointSize(8);f.setBold(true);p.setFont(f);
        const int availableWidth=qMax(1,int(b.rect.width()-10));
        const auto heading=a.start.toString("HH:mm")+"–"+a.finish.toString("HH:mm")+"  "+m_names.value(a.employeeId,"Non affecté");
        const int lineHeight=p.fontMetrics().height();
        p.drawText(b.rect.adjusted(5,3,-5,-3),Qt::AlignLeft|Qt::AlignTop,p.fontMetrics().elidedText(heading,Qt::ElideRight,availableWidth));
        if(b.rect.height()>lineHeight*2+4){f.setBold(false);p.setFont(f);p.drawText(b.rect.adjusted(5,3+lineHeight,-5,-3),Qt::AlignLeft|Qt::AlignTop,p.fontMetrics().elidedText(a.title,Qt::ElideRight,availableWidth));}p.restore();}
    const auto today=QDate::currentDate();const int day=m_monday.daysTo(today);if(day>=0&&day<7){const auto now=QTime::currentTime();const double y=(now.hour()*60+now.minute())*HourHeight/60.0;p.setPen(QPen(QColor("#D65C55"),2));p.drawLine(QPointF(64+day*dw,y),QPointF(64+(day+1)*dw,y));}
}
int ServiceTimetable::entryAt(QPointF point) const {for(const auto &b:blocks())if(b.rect.contains(point))return b.entry;return -1;}
void ServiceTimetable::mousePressEvent(QMouseEvent *event){if(event->button()!=Qt::LeftButton)return;m_selected=entryAt(event->position());emit entrySelected(m_selected);update();}
void ServiceTimetable::mouseDoubleClickEvent(QMouseEvent *event){if(event->button()!=Qt::LeftButton)return;const int index=entryAt(event->position());if(index>=0)emit entryActivated(index);}
void ServiceTimetable::mouseMoveEvent(QMouseEvent *event){const int index=entryAt(event->position());setCursor(index>=0?Qt::PointingHandCursor:Qt::ArrowCursor);if(index<0){QToolTip::hideText();return;}const auto &a=m_entries[index];
    QToolTip::showText(event->globalPosition().toPoint(),QString("%1\n%2 → %3\n%4\n%5\n%6").arg(a.title,a.start.toString("dd/MM HH:mm"),a.finish.toString("dd/MM HH:mm"),m_names.value(a.employeeId,"Non affecté"),a.client,a.status),this);}
