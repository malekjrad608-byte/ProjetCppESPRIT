#include "animatedpanel.h"
#include "../ui/motion.h"
#include <QEvent>
#include <QPainter>
#include <QPainterPath>

AnimatedPanel::AnimatedPanel(QWidget *parent):QFrame(parent),m_hover(new QVariantAnimation(this)) {
    m_hover->setDuration(180); m_hover->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_hover,&QVariantAnimation::valueChanged,this,[this](const QVariant &value) { m_progress=value.toReal(); update(); });
    connect(&Motion::instance(),&Motion::changed,this,[this](bool enabled) {
        if(!enabled) { m_hover->stop(); m_progress=0; update(); }
    });
}
bool AnimatedPanel::event(QEvent *event) {
    if(event->type()==QEvent::Hide) { m_hover->stop(); m_progress=0; }
    else if((event->type()==QEvent::Enter || event->type()==QEvent::Leave) && Motion::instance().enabled()) {
        m_hover->stop(); m_hover->setStartValue(m_progress);
        m_hover->setEndValue(event->type()==QEvent::Enter?1.0:0.0); m_hover->start();
    }
    return QFrame::event(event);
}
void AnimatedPanel::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);
    if(m_progress<=0) return;
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath clip; clip.addRoundedRect(QRectF(rect()).adjusted(1,1,-1,-1),13,13);
    painter.setClipPath(clip);
    QColor outline("#6198A5"); outline.setAlphaF(.65*m_progress);
    painter.setPen(QPen(outline,1.3)); painter.setBrush(Qt::NoBrush); painter.drawPath(clip);
    QLinearGradient accent(20,0,100,0);
    QColor teal("#377E8E"),gold("#E7A342"); teal.setAlphaF(m_progress); gold.setAlphaF(m_progress);
    accent.setColorAt(0,teal); accent.setColorAt(1,gold);
    painter.setPen(QPen(QBrush(accent),2.5,Qt::SolidLine,Qt::RoundCap));
    painter.drawLine(QPointF(20,2),QPointF(20+80*m_progress,2));
}
