#include "pawburst.h"
#include "theme.h"

#include <QPainter>
#include <QRandomGenerator>

PawBurst::PawBurst(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    connect(&m_timer, &QTimer::timeout, this, &PawBurst::tick);
}

void PawBurst::spawn(const QPoint &pos)
{
    if (!Theme::animations)
        return;
    Paw p;
    p.pos = QPointF(pos);
    p.rot = QRandomGenerator::global()->bounded(-30, 30);
    p.t = 0.0;
    m_paws.append(p);
    if (!m_timer.isActive())
        m_timer.start(16);
}

void PawBurst::tick()
{
    for (Paw &p : m_paws)
        p.t += 0.02;
    m_paws.removeIf([](const Paw &p) { return p.t >= 1.0; });
    if (m_paws.isEmpty())
        m_timer.stop();
    update();
}

void PawBurst::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    for (const Paw &p : m_paws) {
        painter.save();
        painter.translate(p.pos.x(), p.pos.y() - 60.0 * p.t);
        painter.rotate(p.rot);
        const qreal s = 0.6 + 0.8 * p.t;
        painter.scale(s, s);
        painter.setOpacity(0.85 * (1.0 - p.t));
        painter.fillPath(Theme::pawPath(QRectF(-13, -13, 26, 26)), Theme::Teal);
        painter.restore();
    }
}
