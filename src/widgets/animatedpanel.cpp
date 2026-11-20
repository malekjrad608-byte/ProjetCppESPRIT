#include "animatedpanel.h"
#include "../ui/motion.h"
#include <QEvent>
#include <QPainter>
#include <QPainterPath>

AnimatedPanel::AnimatedPanel(QWidget *parent):QFrame(parent),m_hover(new QVariantAnimation(this)) {
    m_hover->setObjectName("panelHoverMotion"); m_hover->setDuration(180); m_hover->setEasingCurve(QEasingCurve::OutCubic);
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
    QPainterPath clip; clip.addRoundedRect(QRectF(rect()).adjusted(2,2,-2,-2),8,8);
    painter.setClipPath(clip);
    painter.setBrush(Qt::NoBrush);
    for(int width:{7,4,2}) {
        QColor outline("#FF9B28"); outline.setAlphaF((width==2?.9:.08)*m_progress);
        painter.setPen(QPen(outline,width)); painter.drawPath(clip);
    }
}
