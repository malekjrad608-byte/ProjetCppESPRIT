#include "clinicmap.h"
#include "theme.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSet>
#include <QVariantAnimation>
#include <QtMath>
#include <algorithm>

ClinicMap::ClinicMap(bool compact, QWidget *parent) : QWidget(parent), m_compact(compact)
{
    setMouseTracking(true);
    m_anim = new QVariantAnimation(this);
    m_anim->setDuration(480);
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_t = v.toReal();
        update();
    });

    // pulsation des locaux a nettoyer
    connect(&m_pulse, &QTimer::timeout, this, [this] {
        if (!Theme::animations || !isVisible())
            return;
        bool dirty = false;
        for (const Tile &t : m_tiles) {
            if (t.l.cleaning == CleaningState::ToClean) {
                dirty = true;
                break;
            }
        }
        if (!dirty)
            return;
        m_phase += 0.045;
        if (m_phase >= 1.0)
            m_phase -= 1.0;
        update();
    });
    m_pulse.start(40);
}

void ClinicMap::setSelected(int id)
{
    if (m_selected == id)
        return;
    m_selected = id;
    update();
}

void ClinicMap::setLocaux(const QList<Local> &list)
{
    m_list = list;
    relayout(true);
}

QRectF ClinicMap::currentRect(const Tile &t) const
{
    if (t.isNew || m_t >= 1.0)
        return t.to;
    return QRectF(t.from.left() + (t.to.left() - t.from.left()) * m_t,
                  t.from.top() + (t.to.top() - t.from.top()) * m_t,
                  t.from.width() + (t.to.width() - t.from.width()) * m_t,
                  t.from.height() + (t.to.height() - t.from.height()) * m_t);
}

void ClinicMap::relayout(bool animate)
{
    // positions actuelles (pour faire glisser les tuiles qui restent)
    QHash<int, QRectF> previous;
    for (const Tile &t : m_tiles)
        previous.insert(t.l.id, currentRect(t));

    const qreal W = width() > 40 ? width() : 320;
    const qreal pad = m_compact ? 8 : 14;
    const qreal gap = m_compact ? 5 : 10;
    const int cols = m_compact ? 6 : 8;
    const qreal tileH = m_compact ? 30 : 62;
    const qreal titleH = m_compact ? 22 : 30;
    const qreal unit = (W - 2 * pad - gap * (cols - 1)) / cols;

    // etages : le plus haut en premier (comme un batiment vu de face)
    QList<int> floors;
    {
        QSet<int> seen;
        for (const Local &l : m_list) {
            if (!seen.contains(l.floor)) {
                seen.insert(l.floor);
                floors.append(l.floor);
            }
        }
        std::sort(floors.begin(), floors.end(), [](int a, int b) { return a > b; });
    }

    QList<Tile> tiles;
    QList<FloorBox> boxes;
    qreal y = 0;
    for (int f : floors) {
        FloorBox fb;
        fb.floor = f;
        const qreal top = y;
        qreal rowY = top + titleH;
        int col = 0;
        for (const Local &l : m_list) {
            if (l.floor != f)
                continue;
            ++fb.rooms;
            if (l.occupancy > 0)
                ++fb.busy;
            const int span = qMin(cols, l.capacity >= 8 ? 2 : 1);
            if (col + span > cols) {
                col = 0;
                rowY += tileH + gap;
            }
            Tile t;
            t.l = l;
            t.to = QRectF(pad + col * (unit + gap), rowY, unit * span + gap * (span - 1), tileH);
            if (previous.contains(l.id) && animate) {
                t.from = previous.value(l.id);
                t.isNew = false;
            } else if (previous.contains(l.id)) {
                t.from = t.to;
                t.isNew = false;
            } else {
                t.from = t.to;
                t.isNew = animate;
            }
            tiles.append(t);
            col += span;
        }
        const qreal bottom = rowY + tileH + pad;
        fb.box = QRectF(0, top, W, bottom - top);
        boxes.append(fb);
        y = bottom + 10;
    }

    m_tiles = tiles;
    m_floors = boxes;
    setMinimumHeight(int(qMax<qreal>(40.0, y - 10)) + 2);

    if (animate && Theme::animations) {
        m_anim->stop();
        m_t = 0.0;
        m_anim->start();
    } else {
        m_anim->stop();
        m_t = 1.0;
        for (Tile &t : m_tiles) {
            t.isNew = false;
            t.from = t.to;
        }
    }
    update();
}

void ClinicMap::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    if (e->size().width() != e->oldSize().width())
        relayout(false);
}

int ClinicMap::tileAt(const QPoint &p) const
{
    for (const Tile &t : m_tiles)
        if (t.to.contains(p))
            return t.l.id;
    return 0;
}

void ClinicMap::mouseMoveEvent(QMouseEvent *e)
{
    const int id = tileAt(e->position().toPoint());
    if (id != m_hover) {
        m_hover = id;
        setCursor(id ? Qt::PointingHandCursor : Qt::ArrowCursor);
        QString tip;
        for (const Tile &t : m_tiles) {
            if (t.l.id == id) {
                tip = QStringLiteral("%1 — %2 (%3/%4)\n%5")
                          .arg(t.l.name, t.l.stateText())
                          .arg(t.l.occupancy)
                          .arg(t.l.capacity)
                          .arg(QStringLiteral("Nettoyage : ") + t.l.cleaningText());
                break;
            }
        }
        setToolTip(tip);
        update();
    }
    QWidget::mouseMoveEvent(e);
}

void ClinicMap::leaveEvent(QEvent *e)
{
    if (m_hover) {
        m_hover = 0;
        update();
    }
    QWidget::leaveEvent(e);
}

void ClinicMap::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        const int id = tileAt(e->position().toPoint());
        if (id)
            emit roomClicked(id);
    }
    QWidget::mousePressEvent(e);
}

void ClinicMap::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // blocs d'etage
    for (const FloorBox &fb : m_floors) {
        QPainterPath path;
        path.addRoundedRect(fb.box.adjusted(0.5, 0.5, -0.5, -0.5), 12, 12);
        p.fillPath(path, Theme::TealXL);
        p.setPen(QPen(QColor(0xb9, 0xd4, 0xda), 1.6));
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);

        QFont tf = Theme::font(m_compact ? 7.0 : 8.5, true);
        tf.setLetterSpacing(QFont::AbsoluteSpacing, 1.2);
        p.setFont(tf);
        p.setPen(Theme::Muted);
        const QString name = fb.floor == 0 ? QStringLiteral("REZ-DE-CHAUSSÉE")
                                           : QStringLiteral("ÉTAGE %1").arg(fb.floor);
        const qreal pad = m_compact ? 10 : 16;
        p.drawText(QRectF(pad, fb.box.top() + 4, fb.box.width() - 2 * pad, m_compact ? 18 : 24),
                   Qt::AlignVCenter | Qt::AlignLeft, name);
        QFont cf = Theme::font(m_compact ? 7.0 : 8.5);
        p.setFont(cf);
        p.drawText(QRectF(pad, fb.box.top() + 4, fb.box.width() - 2 * pad, m_compact ? 18 : 24),
                   Qt::AlignVCenter | Qt::AlignRight,
                   QStringLiteral("%1 local(aux) · %2 occupé(s)").arg(fb.rooms).arg(fb.busy));
    }

    // tuiles
    for (const Tile &t : m_tiles) {
        QRectF r = currentRect(t);
        qreal opacity = 1.0;
        if (t.isNew) {
            const qreal s = qMax<qreal>(0.01, m_t);
            const QPointF c = r.center();
            r = QRectF(c.x() - r.width() * s / 2, c.y() - r.height() * s / 2, r.width() * s, r.height() * s);
            opacity = m_t;
        }
        if (t.l.id == m_hover)
            r.adjust(-1.5, -1.5, 1.5, 1.5);

        p.setOpacity(opacity);
        QColor base;
        switch (t.l.state()) {
        case RoomState::Available:    base = QColor(0x4a, 0xa5, 0x7a); break;
        case RoomState::Occupied:     base = QColor(0x5b, 0x76, 0xc9); break;
        case RoomState::OutOfService: base = QColor(0xd9, 0x97, 0x2f); break;
        }
        if (t.l.id == m_hover)
            base = base.lighter(112);

        p.setPen(Qt::NoPen);
        p.setBrush(base);
        p.drawRoundedRect(r, m_compact ? 6 : 10, m_compact ? 6 : 10);

        // nom
        p.setPen(Qt::white);
        const QFont nf = Theme::font(m_compact ? 7.0 : 9.0, true);
        p.setFont(nf);
        const QRectF textR = r.adjusted(4, m_compact ? 0 : 6, -4, m_compact ? 0 : -22);
        p.drawText(textR, Qt::AlignCenter,
                   QFontMetricsF(nf).elidedText(t.l.name, Qt::ElideRight, textR.width()));
        if (!m_compact) {
            const QFont sf = Theme::font(8.0);
            p.setFont(sf);
            QColor soft = Qt::white;
            soft.setAlphaF(0.85);
            p.setPen(soft);
            p.drawText(QRectF(r.left() + 4, r.bottom() - 24, r.width() - 8, 18), Qt::AlignCenter,
                       QStringLiteral("%1/%2 · %3").arg(t.l.occupancy).arg(t.l.capacity).arg(t.l.cleaningText()));
        }

        // selection
        if (t.l.id == m_selected) {
            p.setPen(QPen(Theme::Orange, 3));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(r.adjusted(-2, -2, 2, 2), m_compact ? 7 : 11, m_compact ? 7 : 11);
        }

        // local a nettoyer : point clignotant
        if (t.l.cleaning == CleaningState::ToClean && opacity > 0.5) {
            const QPointF c(r.right() - 7, r.top() + 7);
            if (Theme::animations) {
                const qreal ring = 4.0 + 7.0 * m_phase;
                QColor rc = Theme::Red;
                rc.setAlphaF(0.55 * (1.0 - m_phase));
                p.setPen(Qt::NoPen);
                p.setBrush(rc);
                p.drawEllipse(c, ring, ring);
            }
            p.setPen(QPen(Theme::Red, 2));
            p.setBrush(Qt::white);
            p.drawEllipse(c, 3.5, 3.5);
        } else if (t.l.cleaning == CleaningState::InProgress && opacity > 0.5) {
            p.setPen(QPen(QColor(0xe0, 0x9b, 0x3d), 2));
            p.setBrush(Qt::white);
            p.drawEllipse(QPointF(r.right() - 7, r.top() + 7), 3.0, 3.0);
        }
        p.setOpacity(1.0);
    }
}
