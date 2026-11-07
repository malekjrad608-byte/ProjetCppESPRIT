#include "clinicmap.h"
#include "theme.h"

#include <QMouseEvent>
#include <QPainter>
#include <QSet>
#include <QVariantAnimation>
#include <QtMath>
#include <QFontMetricsF>
#include <algorithm>
#include <functional>

// --------------------------------------------------------------------- dimensions (unites "monde")
namespace {
constexpr qreal BoxH   = 0.9;    // hauteur d'un local
constexpr qreal SlabT  = 0.16;   // epaisseur d'une dalle d'etage
constexpr qreal Gap    = 0.24;   // espace entre deux locaux
constexpr qreal Margin = 0.40;   // bord de la dalle
constexpr qreal ZK     = 0.80;   // "ecrasement" de la hauteur a l'ecran
constexpr qreal GapNormal   = 1.8;   // espace entre etages (vue normale)
constexpr qreal GapExploded = 3.2;   // espace entre etages (vue eclatee)

qreal easeOut(qreal t) { return 1.0 - (1.0 - t) * (1.0 - t) * (1.0 - t); }
qreal lerp(qreal a, qreal b, qreal t) { return a + (b - a) * t; }
}

ClinicMap::ClinicMap(bool compact, QWidget *parent)
    : QWidget(parent), m_compact(compact), m_angle(qDegreesToRadians(45.0)), m_gap(GapNormal)
{
    setMouseTracking(true);
    setMinimumHeight(compact ? 230 : 420);
    setCursor(Qt::OpenHandCursor);

    auto makeAnim = [this](int ms) {
        auto *a = new QVariantAnimation(this);
        a->setDuration(ms);
        a->setEasingCurve(QEasingCurve::OutCubic);
        return a;
    };

    // apparition / disparition / deplacement des locaux
    m_anim = makeAnim(650);
    m_anim->setStartValue(0.0);
    m_anim->setEndValue(1.0);
    connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_t = v.toReal();
        update();
    });
    connect(m_anim, &QVariantAnimation::finished, this, [this] {
        m_boxes.erase(std::remove_if(m_boxes.begin(), m_boxes.end(),
                                     [](const Box &b) { return b.leaving; }),
                      m_boxes.end());
        for (Box &b : m_boxes) {
            b.isNew = false;
            b.moved = false;
        }
        update();
    });

    // rotation
    m_rotAnim = makeAnim(700);
    connect(m_rotAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_angle = v.toReal();
        update();
    });

    // vue eclatee
    m_gapAnim = makeAnim(600);
    connect(m_gapAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_gap = v.toReal();
        update();
    });

    // soulevement au survol
    m_liftAnim = makeAnim(220);
    connect(m_liftAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        m_lift = v.toReal();
        update();
    });

    // clignotement des locaux "À nettoyer"
    connect(&m_pulse, &QTimer::timeout, this, [this] {
        if (!Theme::animations || !isVisible())
            return;
        bool dirty = false;
        for (const Box &b : m_boxes)
            if (b.l.getEtat() == Local::ANettoyer)
                dirty = true;
        if (!dirty)
            return;
        m_phase += 0.025;
        if (m_phase >= 1.0)
            m_phase -= 1.0;
        update();
    });
    m_pulse.start(40);
}

QSize ClinicMap::sizeHint() const
{
    return m_compact ? QSize(300, 240) : QSize(820, 520);
}

// ===================================================================== donnees

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
    layoutBoxes();
}

qreal ClinicMap::floorZ(int floorIndex) const
{
    return floorIndex * (SlabT + BoxH + m_gap);
}

void ClinicMap::layoutBoxes()
{
    const bool animate = Theme::animations && isVisible();

    // position actuelle de chaque boite (pour l'animation de deplacement)
    struct Pos { qreal x, y, z; };
    QHash<int, Pos> previous;
    QList<Box> leaving;
    for (const Box &b : m_boxes) {
        if (b.leaving)
            continue;
        const qreal t = b.moved ? easeOut(m_t) : 1.0;
        const qreal z = floorZ(b.floorIndex) + SlabT;
        previous.insert(b.l.getId(), { lerp(b.fromX, b.x, t), lerp(b.fromY, b.y, t),
                                       b.moved ? lerp(b.fromZ, z, t) : z });
    }
    QSet<int> newIds;
    for (const Local &l : m_list)
        newIds.insert(l.getId());
    for (const Box &b : m_boxes) {
        if (!b.leaving && !newIds.contains(b.l.getId()) && animate) {
            Box gone = b;
            gone.leaving = true;
            gone.moved = false;
            gone.isNew = false;
            gone.fromZ = floorZ(b.floorIndex) + SlabT;
            leaving.append(gone);
        }
    }

    // etages presents, du plus bas au plus haut
    QSet<int> seen;
    m_floors.clear();
    for (const Local &l : m_list)
        if (!seen.contains(l.getFloor())) {
            seen.insert(l.getFloor());
            m_floors.append(l.getFloor());
        }
    std::sort(m_floors.begin(), m_floors.end());

    // placement en grille : m_cols colonnes, un local de grande capacite prend 2 cases
    m_cols = 4;
    m_rows = 1;
    QList<Box> boxes;
    for (int fi = 0; fi < m_floors.size(); ++fi) {
        int col = 0, row = 0;
        for (const Local &l : m_list) {
            if (l.getFloor() != m_floors[fi])
                continue;
            const int span = l.getCapacity() >= 8 ? 2 : 1;
            if (col + span > m_cols) {
                col = 0;
                ++row;
            }
            Box b;
            b.l = l;
            b.floorIndex = fi;
            b.x = Margin + col * (1.0 + Gap);
            b.y = Margin + row * (1.0 + Gap);
            b.w = span + (span - 1) * Gap;
            b.d = 1.0;
            const qreal z = floorZ(fi) + SlabT;
            if (previous.contains(l.getId())) {
                const Pos p = previous.value(l.getId());
                b.moved = animate && (!qFuzzyCompare(p.x + 1, b.x + 1) || !qFuzzyCompare(p.y + 1, b.y + 1)
                                      || !qFuzzyCompare(p.z + 1, z + 1));
                b.fromX = p.x;
                b.fromY = p.y;
                b.fromZ = p.z;
            } else {
                b.isNew = animate;
                b.fromX = b.x;
                b.fromY = b.y;
                b.fromZ = z;
            }
            boxes.append(b);
            col += span;
        }
        m_rows = qMax(m_rows, row + 1);
    }
    m_slabW = m_cols * (1.0 + Gap) - Gap + 2 * Margin;
    m_slabD = m_rows * (1.0 + Gap) - Gap + 2 * Margin;

    m_boxes = boxes + leaving;

    if (animate) {
        m_anim->stop();
        m_t = 0.0;
        m_anim->start();
    } else {
        m_t = 1.0;
        for (Box &b : m_boxes) {
            b.isNew = false;
            b.moved = false;
        }
    }
    update();
}

// ===================================================================== projection

QPointF ClinicMap::project(qreal x, qreal y, qreal z) const
{
    const qreal x0 = x - m_slabW / 2, y0 = y - m_slabD / 2;
    const qreal c = qCos(m_angle), s = qSin(m_angle);
    const qreal X = x0 * c - y0 * s;
    const qreal Y = x0 * s + y0 * c;
    return m_origin + QPointF(X * m_scale, Y * m_scale * 0.5 - z * m_scale * ZK);
}

qreal ClinicMap::depth(qreal x, qreal y) const
{
    const qreal x0 = x - m_slabW / 2, y0 = y - m_slabD / 2;
    return x0 * qSin(m_angle) + y0 * qCos(m_angle);
}

// echelle choisie pour que le batiment entre dans le widget quelle que soit la rotation
void ClinicMap::computeFit()
{
    const qreal padL = m_compact ? 34 : 86;   // place pour les noms d'etage
    const qreal padR = m_compact ? 10 : 24;
    const qreal padY = m_compact ? 10 : 22;
    const int n = qMax(1, int(m_floors.size()));
    const qreal R = qSqrt(m_slabW * m_slabW + m_slabD * m_slabD) / 2;
    const qreal zTop = floorZ(n - 1) + SlabT + BoxH + 0.4;

    const qreal availW = width() - padL - padR;
    const qreal availH = height() - 2 * padY;
    m_scale = qMin(availW / (2 * R), availH / (R + zTop * ZK));
    m_scale = qBound<qreal>(8.0, m_scale, m_compact ? 40.0 : 78.0);

    const qreal usedH = (R + zTop * ZK) * m_scale;
    m_origin = QPointF(padL + availW / 2,
                       padY + (availH - usedH) / 2 + (R * 0.5 + zTop * ZK) * m_scale);
}

QColor ClinicMap::colorFor(const Local &l) const
{
    switch (l.state()) {
    case RoomState::Available:    return QColor(0x4a, 0xa5, 0x7a);
    case RoomState::Occupied:     return QColor(0x5b, 0x76, 0xc9);
    case RoomState::OutOfService: return QColor(0xa9, 0xb4, 0xb8);
    case RoomState::ToClean:      return QColor(0xd0, 0x5a, 0x55);
    }
    return QColor(0x4a, 0xa5, 0x7a);
}

// ===================================================================== dessin

namespace {

// Dessine une boite (x, y, z) de taille (w, d, h) : faces visibles puis dessus.
// Retourne la silhouette (pour le clic) et le polygone du dessus.
QPainterPath drawBox(QPainter &p, const std::function<QPointF(qreal, qreal, qreal)> &proj,
                     qreal angle, qreal x, qreal y, qreal z, qreal w, qreal d, qreal h,
                     const QColor &top, QPolygonF *topOut)
{
    const QPointF bottom[4] = { { x, y }, { x + w, y }, { x + w, y + d }, { x, y + d } };
    const QPointF normals[4] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
    const qreal c = qCos(angle), s = qSin(angle);

    QPainterPath silhouette;
    p.setPen(QPen(top.darker(150), 0.8));
    for (int i = 0; i < 4; ++i) {
        const qreal nX = normals[i].x() * c - normals[i].y() * s;
        const qreal nY = normals[i].x() * s + normals[i].y() * c;
        if (nY <= 0.001)
            continue;                          // face cachee (elle regarde vers le fond)
        const QPointF a = bottom[i], b = bottom[(i + 1) % 4];
        QPolygonF face;
        face << proj(a.x(), a.y(), z) << proj(b.x(), b.y(), z)
             << proj(b.x(), b.y(), z + h) << proj(a.x(), a.y(), z + h);
        // ombrage : face de gauche plus sombre que face de droite
        p.setBrush(top.darker(int(118 + 26 * (nX + 1) / 2)));
        p.drawPolygon(face);
        silhouette.addPolygon(face);
    }
    QPolygonF topFace;
    for (const QPointF &b : bottom)
        topFace << proj(b.x(), b.y(), z + h);
    p.setBrush(top);
    p.drawPolygon(topFace);
    silhouette.addPolygon(topFace);
    if (topOut)
        *topOut = topFace;
    return silhouette.simplified();
}

} // namespace

void ClinicMap::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    computeFit();
    m_hit.clear();

    if (m_list.isEmpty() && m_boxes.isEmpty()) {
        p.setPen(Theme::Muted);
        p.setFont(Theme::font(9.5));
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("Aucun local : ajoute un local pour voir le plan"));
        return;
    }

    auto proj = [this](qreal x, qreal y, qreal z) { return project(x, y, z); };

    // ombre au sol
    {
        const QPointF c = project(m_slabW / 2, m_slabD / 2, -0.05);
        const qreal R = qSqrt(m_slabW * m_slabW + m_slabD * m_slabD) / 2 * m_scale;
        QRadialGradient g(c, R * 1.05);
        g.setColorAt(0.0, QColor(31, 85, 96, 55));
        g.setColorAt(1.0, QColor(31, 85, 96, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        p.drawEllipse(c, R * 1.05, R * 0.55);
    }

    const qreal bt = easeOut(m_t);
    for (int fi = 0; fi < qMax(1, int(m_floors.size())); ++fi) {
        const qreal fz = floorZ(fi);

        // ---- dalle de l'etage
        QPolygonF slabTop;
        p.setOpacity(fi == 0 ? 1.0 : 0.85);   // dalles du haut un peu transparentes
        drawBox(p, proj, m_angle, 0, 0, fz, m_slabW, m_slabD, SlabT, QColor(0xe4, 0xef, 0xf2), &slabTop);
        p.setOpacity(1.0);

        // ---- nom de l'etage, a gauche, a la hauteur de la dalle
        if (fi < m_floors.size()) {
            qreal minX = 1e9, yAtMin = 0;
            for (const QPointF &pt : slabTop)
                if (pt.x() < minX) {
                    minX = pt.x();
                    yAtMin = pt.y();
                }
            const int f = m_floors[fi];
            const QString label = m_compact ? (f == 0 ? QStringLiteral("RDC") : QStringLiteral("É%1").arg(f))
                                            : (f == 0 ? QStringLiteral("Rez-de-chaussée") : QStringLiteral("Étage %1").arg(f));
            QFont lf = Theme::font(m_compact ? 7.0 : 8.5, true);
            p.setFont(lf);
            p.setPen(Theme::TealDark);
            const QRectF lr(2, yAtMin - 10, minX - 8, 20);
            p.drawText(lr, Qt::AlignVCenter | Qt::AlignRight,
                       QFontMetricsF(lf).elidedText(label, Qt::ElideRight, lr.width()));
            p.setPen(QPen(QColor(0x9f, 0xc2, 0xca), 1, Qt::DashLine));
            p.drawLine(QPointF(minX - 5, yAtMin), QPointF(minX, yAtMin));
        }

        // ---- locaux de cet etage, du fond vers l'avant
        QList<const Box *> order;
        for (const Box &b : m_boxes)
            if (b.floorIndex == fi)
                order.append(&b);
        std::sort(order.begin(), order.end(), [this](const Box *a, const Box *b) {
            return depth(a->x + a->w / 2, a->y + a->d / 2) < depth(b->x + b->w / 2, b->y + b->d / 2);
        });

        for (const Box *b : order) {
            const int id = b->l.getId();
            qreal x = b->x, y = b->y, z = fz + SlabT, h = BoxH, alpha = 1.0;
            if (b->moved) {
                x = lerp(b->fromX, b->x, bt);
                y = lerp(b->fromY, b->y, bt);
                z = lerp(b->fromZ, z, bt);
            }
            if (b->isNew) {
                h = BoxH * qMax<qreal>(0.02, bt);
                alpha = qMin<qreal>(1.0, m_t * 2);
            } else if (b->leaving) {
                z = b->fromZ;
                h = BoxH * qMax<qreal>(0.02, 1.0 - bt);
                alpha = 1.0 - bt;
            }
            const bool hovered = (id == m_hover && !b->leaving);
            if (hovered)
                z += 0.35 * m_lift;

            QColor top = colorFor(b->l);
            if (b->l.getEtat() == Local::ANettoyer && Theme::animations)
                top = Theme::lerp(top, top.lighter(135), 0.5 + 0.5 * qSin(m_phase * 2 * M_PI));
            if (hovered)
                top = top.lighter(110);

            p.setOpacity(alpha);
            QPolygonF topFace;
            const QPainterPath shape = drawBox(p, proj, m_angle, x, y, z, b->w, b->d, h, top, &topFace);

            // selection : contour orange du dessus
            if (id == m_selected && !b->leaving) {
                p.setPen(QPen(Theme::Orange, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                p.setBrush(Qt::NoBrush);
                p.drawPolygon(topFace);
            }

            // nom (+ occupation) ecrit sur le dessus
            if (!m_compact && m_scale > 34 && alpha > 0.6) {
                const QPointF c = project(x + b->w / 2, y + b->d / 2, z + h);
                const qreal tw = m_scale * b->w * 0.95;
                QFont nf = Theme::font(qBound<qreal>(6.5, m_scale * 0.12, 9.5), true);
                p.setFont(nf);
                p.setPen(QColor(0, 0, 0, 60));
                const QString nm = QFontMetricsF(nf).elidedText(b->l.getName(), Qt::ElideRight, tw);
                p.drawText(QRectF(c.x() - tw / 2 + 1, c.y() - 15 + 1, tw, 16), Qt::AlignCenter, nm);
                p.setPen(Qt::white);
                p.drawText(QRectF(c.x() - tw / 2, c.y() - 15, tw, 16), Qt::AlignCenter, nm);
                QFont sf = Theme::font(qBound<qreal>(6.0, m_scale * 0.10, 8.5));
                p.setFont(sf);
                p.drawText(QRectF(c.x() - tw / 2, c.y() + 1, tw, 14), Qt::AlignCenter,
                           QStringLiteral("%1/%2").arg(b->l.getOccupancy()).arg(b->l.getCapacity()));
            }
            p.setOpacity(1.0);

            if (!b->leaving)
                m_hit.append({ id, shape });
        }
    }
}

// ===================================================================== interaction

int ClinicMap::roomAt(const QPointF &pt) const
{
    for (int i = m_hit.size() - 1; i >= 0; --i)   // la boite dessinee en dernier est devant
        if (m_hit[i].second.contains(pt))
            return m_hit[i].first;
    return 0;
}

void ClinicMap::rotateQuarter()
{
    const qreal from = m_angle;
    const qreal to = (m_rotAnim->state() == QAbstractAnimation::Running ? m_rotAnim->endValue().toReal() : m_angle)
                     + M_PI / 2;
    m_rotAnim->stop();
    if (!Theme::animations) {
        m_angle = to;
        update();
        return;
    }
    m_rotAnim->setStartValue(from);
    m_rotAnim->setEndValue(to);
    m_rotAnim->start();
}

void ClinicMap::setExploded(bool on)
{
    m_exploded = on;
    const qreal to = on ? GapExploded : GapNormal;
    m_gapAnim->stop();
    if (!Theme::animations) {
        m_gap = to;
        update();
        return;
    }
    m_gapAnim->setStartValue(m_gap);
    m_gapAnim->setEndValue(to);
    m_gapAnim->start();
}

void ClinicMap::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_pressed = true;
        m_dragging = false;
        m_pressPos = e->position();
        m_pressAngle = m_angle;
    }
    QWidget::mousePressEvent(e);
}

void ClinicMap::mouseMoveEvent(QMouseEvent *e)
{
    // glisser = tourner le batiment
    if (m_pressed) {
        const qreal dx = e->position().x() - m_pressPos.x();
        if (!m_dragging && qAbs(dx) > 4) {
            m_dragging = true;
            m_rotAnim->stop();
            setCursor(Qt::ClosedHandCursor);
        }
        if (m_dragging) {
            m_angle = m_pressAngle + dx * 0.012;
            update();
            return;
        }
    }

    const int id = roomAt(e->position());
    if (id != m_hover) {
        m_hover = id;
        setCursor(id ? Qt::PointingHandCursor : Qt::OpenHandCursor);
        QString tip;
        for (const Box &b : m_boxes)
            if (b.l.getId() == id && !b.leaving) {
                tip = QStringLiteral("%1 — %2\n%3 · %4/%5 animal(aux)")
                          .arg(b.l.getName(), b.l.getEtat(), b.l.floorText())
                          .arg(b.l.getOccupancy())
                          .arg(b.l.getCapacity());
                break;
            }
        setToolTip(tip);
        m_liftAnim->stop();
        if (id && Theme::animations) {
            m_liftAnim->setStartValue(0.0);
            m_liftAnim->setEndValue(1.0);
            m_liftAnim->start();
        } else {
            m_lift = id ? 1.0 : 0.0;
        }
        update();
    }
    QWidget::mouseMoveEvent(e);
}

void ClinicMap::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_pressed) {
        if (!m_dragging) {
            const int id = roomAt(e->position());
            if (id)
                emit roomClicked(id);
        }
        m_pressed = false;
        m_dragging = false;
        setCursor(m_hover ? Qt::PointingHandCursor : Qt::OpenHandCursor);
    }
    QWidget::mouseReleaseEvent(e);
}

void ClinicMap::leaveEvent(QEvent *e)
{
    if (m_hover) {
        m_hover = 0;
        m_lift = 0;
        update();
    }
    QWidget::leaveEvent(e);
}
