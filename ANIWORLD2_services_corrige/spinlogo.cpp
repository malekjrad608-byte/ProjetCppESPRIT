#include "spinlogo.h"
#include "theme.h"

#include <QEnterEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QtMath>

SpinLogo::SpinLogo(const QPixmap &pm, int size, QWidget *parent)
    : QWidget(parent), m_pm(pm)
{
    setFixedSize(size, size);
    setCursor(Qt::PointingHandCursor);
    m_anim = new QPropertyAnimation(this, "angle", this);
    m_anim->setDuration(900);
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(360.0);
    m_anim->setEasingCurve(QEasingCurve::InOutCubic);
}

void SpinLogo::setAngle(qreal a)
{
    m_angle = a;
    update();
}

void SpinLogo::enterEvent(QEnterEvent *e)
{
    if (Theme::animations && m_anim->state() != QAbstractAnimation::Running)
        m_anim->start();
    QWidget::enterEvent(e);
}

void SpinLogo::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    const qreal s = 1.0 + 0.12 * qSin(qDegreesToRadians(m_angle / 2.0));
    p.translate(width() / 2.0, height() / 2.0);
    p.rotate(m_angle);
    p.scale(s, s);
    p.translate(-width() / 2.0, -height() / 2.0);
    p.drawPixmap(QRectF(rect()), m_pm, QRectF(m_pm.rect()));
}
