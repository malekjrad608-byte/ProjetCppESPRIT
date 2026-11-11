#include "serviceanimatedbutton.h"
#include "../ui/servicemotion.h"
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

ServiceAnimatedButton::ServiceAnimatedButton(const QString &text,QWidget *parent):QPushButton(text,parent),
    m_hover(new QVariantAnimation(this)),m_ripple(new QVariantAnimation(this)) {
    m_hover->setObjectName("buttonHoverMotion"); m_hover->setDuration(160);
    m_hover->setEasingCurve(QEasingCurve::OutCubic);
    m_ripple->setObjectName("buttonRippleMotion"); m_ripple->setDuration(360);
    m_ripple->setStartValue(0.0); m_ripple->setEndValue(1.0);
    m_ripple->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_hover,&QVariantAnimation::valueChanged,this,[this](const QVariant &v) { m_highlight=v.toReal(); update(); });
    connect(m_ripple,&QVariantAnimation::valueChanged,this,[this](const QVariant &v) { m_wave=v.toReal(); update(); });
    connect(this,&QPushButton::pressed,this,[this] {
        if(!ServiceMotion::instance().enabled()) return;
        if(!underMouse()) m_origin=rect().center();
        m_ripple->stop(); m_ripple->start();
    });
    connect(&ServiceMotion::instance(),&ServiceMotion::changed,this,[this](bool enabled) { if(!enabled) stopMotion(); });
}
void ServiceAnimatedButton::stopMotion() {
    m_hover->stop(); m_ripple->stop(); m_highlight=0; m_wave=1; update();
}
bool ServiceAnimatedButton::event(QEvent *event) {
    const auto type=event->type();
    if(type==QEvent::Hide || (type==QEvent::EnabledChange && !isEnabled())) stopMotion();
    else if((type==QEvent::Enter || type==QEvent::Leave || type==QEvent::FocusIn || type==QEvent::FocusOut) && isEnabled() && ServiceMotion::instance().enabled()) {
        m_hover->stop(); m_hover->setStartValue(m_highlight);
        m_hover->setEndValue(type==QEvent::Enter || type==QEvent::FocusIn ? 1.0 : 0.0); m_hover->start();
    }
    return QPushButton::event(event);
}
void ServiceAnimatedButton::mousePressEvent(QMouseEvent *event) {
    m_origin=event->position(); QPushButton::mousePressEvent(event);
}
void ServiceAnimatedButton::paintEvent(QPaintEvent *event) {
    QPushButton::paintEvent(event);
    if(!isEnabled() || !ServiceMotion::instance().enabled()) return;
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath clip; clip.addRoundedRect(QRectF(rect()).adjusted(1,1,-1,-1),7,7); painter.setClipPath(clip);
    const QColor accent("#FF9B28");
    if(m_highlight>0) {
        painter.setBrush(Qt::NoBrush);
        for(int width:{7,4,2}) {
            QColor border=accent; border.setAlphaF((width==2?.95:.08)*m_highlight);
            painter.setPen(QPen(border,width));
            painter.drawRoundedRect(QRectF(rect()).adjusted(2,2,-2,-2),6,6);
        }
        QLinearGradient shine(0,0,width(),height());
        QColor glow=accent; glow.setAlphaF(.18*m_highlight);
        shine.setColorAt(0,glow); shine.setColorAt(1,Qt::transparent);
        painter.fillPath(clip,shine);
    }
    if(m_wave<1) {
        QColor color=accent; color.setAlphaF(.17*(1-m_wave)); painter.setPen(Qt::NoPen); painter.setBrush(color);
        const qreal radius=qSqrt(width()*width()+height()*height())*m_wave;
        painter.drawEllipse(m_origin,radius,radius);
    }
}
