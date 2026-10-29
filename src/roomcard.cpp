#include "roomcard.h"
#include "icons.h"
#include "theme.h"

#include <QEnterEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QRadialGradient>
#include <QTimer>
#include <QtMath>

RoomCard::RoomCard(const Local &l, QWidget *parent) : QWidget(parent), m_local(l)
{
    setFixedSize(CardWidth, CardHeight);
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    m_hoverAnim = new QPropertyAnimation(this, "hover", this);
    m_hoverAnim->setDuration(240);
    m_hoverAnim->setEasingCurve(QEasingCurve::OutCubic);
}

void RoomCard::setSelected(bool on)
{
    if (m_selected == on)
        return;
    m_selected = on;
    update();
}

void RoomCard::setAlertLevel(int level)
{
    if (m_alert == level)
        return;
    m_alert = level;
    update();
}

void RoomCard::setHover(qreal v)
{
    m_hover = v;
    update();
}

void RoomCard::setIntro(qreal v)
{
    m_intro = v;
    update();
}

void RoomCard::playIntro(int delayMs)
{
    if (!Theme::animations)
        return;
    m_intro = 0.0;
    auto *a = new QPropertyAnimation(this, "intro", this);
    a->setDuration(420);
    a->setStartValue(0.0);
    a->setEndValue(1.0);
    a->setEasingCurve(QEasingCurve::OutCubic);
    QTimer::singleShot(delayMs, this, [a] { a->start(QAbstractAnimation::DeleteWhenStopped); });
}

void RoomCard::animateHover(qreal target)
{
    m_hoverAnim->stop();
    if (!Theme::animations) {
        setHover(target);
        return;
    }
    m_hoverAnim->setStartValue(m_hover);
    m_hoverAnim->setEndValue(target);
    m_hoverAnim->start();
}

void RoomCard::enterEvent(QEnterEvent *e)
{
    animateHover(1.0);
    QWidget::enterEvent(e);
}

void RoomCard::leaveEvent(QEvent *e)
{
    animateHover(0.0);
    QWidget::leaveEvent(e);
}

void RoomCard::mouseMoveEvent(QMouseEvent *e)
{
    m_mouse = e->position();
    if (m_hover > 0.0)
        update();
    QWidget::mouseMoveEvent(e);
}

void RoomCard::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        m_pressed = true;
    QWidget::mousePressEvent(e);
}

void RoomCard::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_pressed && e->button() == Qt::LeftButton && rect().contains(e->position().toPoint()))
        emit clicked(m_local.getId());
    m_pressed = false;
    QWidget::mouseReleaseEvent(e);
}

void RoomCard::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing
                     | QPainter::SmoothPixmapTransform);
    p.setOpacity(m_intro);

    const qreal lift = m_hover * 5.0;
    const QRectF cr(10, 16 - lift - (1.0 - m_intro) * 10.0, 250, 152);
    const qreal pi = 3.14159265358979;

    // ombre douce (plus forte et plus haute au survol)
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 7; ++i) {
        QColor c = Theme::TealDark;
        c.setAlphaF((0.035 + 0.06 * m_hover) * (1.0 - i / 7.0));
        p.setBrush(c);
        p.drawRoundedRect(cr.adjusted(-i, -i, i, i).translated(0, 3 + 4 * m_hover), 14 + i, 14 + i);
    }

    // fond
    QPainterPath body;
    body.addRoundedRect(cr, 14, 14);
    p.fillPath(body, m_selected ? QColor(255, 250, 241) : QColor(Qt::white));

    // lueur qui suit la souris + patte en filigrane
    if (m_hover > 0.01) {
        p.save();
        p.setClipPath(body);
        QRadialGradient g(m_mouse.isNull() ? cr.center() : m_mouse, 150);
        QColor c0 = Theme::Teal;
        c0.setAlphaF(0.13 * m_hover);
        QColor c1 = Theme::Teal;
        c1.setAlphaF(0.0);
        g.setColorAt(0, c0);
        g.setColorAt(1, c1);
        p.fillRect(cr, g);

        p.translate(cr.center().x() + 78, cr.center().y() + 6);
        p.rotate(-18 + 30 * m_hover);
        QColor pc = Theme::Teal;
        pc.setAlphaF(0.09 * m_hover);
        p.fillPath(Theme::pawPath(QRectF(-38, -38, 76, 76)), pc);
        p.restore();
    }

    // bordure
    const QColor border = m_selected
        ? Theme::Orange
        : Theme::lerp(Theme::Line, QColor(0x9c, 0xc6, 0xcf), m_hover);
    p.setPen(QPen(border, m_selected ? 2.0 : 1.4));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(cr, 14, 14);

    // icone (se secoue au survol)
    const QRectF ib(cr.left() + 14, cr.top() + 14, 34, 34);
    p.save();
    p.translate(ib.center());
    p.rotate(qSin(m_hover * pi * 2.0) * 12.0 * (1.0 - 0.3 * m_hover));
    p.translate(-ib.center());
    p.setPen(Qt::NoPen);
    p.setBrush(Theme::lerp(Theme::TealXL, Theme::Teal, m_hover));
    p.drawRoundedRect(ib, 10, 10);
    const QPointF ip(ib.center().x() - 9, ib.center().y() - 9);
    p.setOpacity(m_intro * (1.0 - m_hover));
    p.drawPixmap(ip, Icons::pixmap(m_local.iconName(), Theme::Teal, 18));
    p.setOpacity(m_intro * m_hover);
    p.drawPixmap(ip, Icons::pixmap(m_local.iconName(), Qt::white, 18));
    p.restore();
    p.setOpacity(m_intro);

    // occupation (haut a droite)
    {
        const QFont f = Theme::font(8.5);
        const QString t = QStringLiteral("%1/%2 animal%3")
                              .arg(m_local.getOccupancy())
                              .arg(m_local.getCapacity())
                              .arg(QString(m_local.getCapacity() > 1 ? "aux" : ""));
        const qreal w = Theme::pillWidth(t, f, false);
        Theme::drawPill(p, QRectF(cr.right() - 14 - w, cr.top() + 20, w, 22), t,
                        Theme::TealXL, Theme::TealDark, f, false);
    }

    const qreal left = cr.left() + 16;
    const qreal textW = cr.width() - 32;

    // nom
    {
        const QFont f = Theme::font(11.5, true);
        p.setFont(f);
        p.setPen(Theme::Ink);
        p.drawText(QRectF(left, cr.top() + 58, textW, 24), Qt::AlignVCenter | Qt::AlignLeft,
                   QFontMetricsF(f).elidedText(m_local.getName(), Qt::ElideRight, textW));
    }
    // type
    {
        const QFont f = Theme::font(9.5);
        p.setFont(f);
        p.setPen(Theme::Muted);
        p.drawText(QRectF(left, cr.top() + 82, textW, 18), Qt::AlignVCenter | Qt::AlignLeft,
                   m_local.getType());
    }
    // etat + nettoyage
    {
        const QFont f = Theme::font(8.5, true);
        QColor bg, fg;
        Theme::stateColors(m_local.state(), bg, fg);
        const QString st = m_local.stateText();
        Theme::drawPill(p, QRectF(left, cr.top() + 108, Theme::pillWidth(st, f, true), 24),
                        st, bg, fg, f, true);

        Theme::cleaningColors(m_local.getCleaning(), bg, fg);
        const QString cl = m_local.cleaningText();
        const qreal w = Theme::pillWidth(cl, f, true);
        Theme::drawPill(p, QRectF(cr.right() - 14 - w, cr.top() + 108, w, 24), cl, bg, fg, f, true);
    }
    // specialite + etage
    {
        const QFont f = Theme::font(8.5);
        p.setFont(f);
        p.setPen(Theme::Muted);
        QString info = m_local.floorShort();
        if (!m_local.getSpecialty().isEmpty())
            info = m_local.getSpecialty() + QStringLiteral("  ·  ") + info;
        p.drawText(QRectF(left, cr.top() + 134, textW, 16), Qt::AlignVCenter | Qt::AlignLeft,
                   QFontMetricsF(f).elidedText(info, Qt::ElideRight, textW));
    }
    // pastille d'alerte (coin haut droit)
    if (m_alert > 0) {
        const QPointF c(cr.right() - 4, cr.top() + 2);
        p.setPen(QPen(Qt::white, 2));
        p.setBrush(m_alert >= 2 ? Theme::Red : Theme::Orange);
        p.drawEllipse(c, 10, 10);
        p.setPen(Qt::white);
        p.setFont(Theme::font(10, true));
        p.drawText(QRectF(c.x() - 10, c.y() - 10, 20, 20), Qt::AlignCenter, QStringLiteral("!"));
    }
}
