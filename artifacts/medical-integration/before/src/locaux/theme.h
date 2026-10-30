#pragma once
#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QRectF>
#include "local.h"

// Couleurs et petits outils de dessin partages par toute l'application
namespace Theme {

inline bool animations = true;   // bouton "Animations" du header

inline const QColor Teal(0x2f, 0x74, 0x82);
inline const QColor TealDark(0x1f, 0x55, 0x60);
inline const QColor TealLight(0xdb, 0xea, 0xee);
inline const QColor TealXL(0xee, 0xf6, 0xf8);
inline const QColor Orange(0xf0, 0xa4, 0x3a);
inline const QColor OrangeLight(0xff, 0xf4, 0xe2);
inline const QColor OrangeText(0xb9, 0x77, 0x12);
inline const QColor Green(0x2e, 0x8b, 0x57);
inline const QColor GreenLight(0xe3, 0xf3, 0xe9);
inline const QColor Blue(0x4a, 0x63, 0xb8);
inline const QColor BlueLight(0xe6, 0xe9, 0xf8);
inline const QColor Red(0xc0, 0x50, 0x4d);
inline const QColor RedLight(0xfb, 0xe9, 0xe8);
inline const QColor Ink(0x1d, 0x3b, 0x44);
inline const QColor Muted(0x6f, 0x8a, 0x92);
inline const QColor Line(0xe1, 0xeb, 0xee);
inline const QColor Background(0xf3, 0xf8, 0xf9);

inline QColor lerp(const QColor &a, const QColor &b, qreal t)
{
    return QColor::fromRgbF(float(a.redF() + (b.redF() - a.redF()) * t),
                            float(a.greenF() + (b.greenF() - a.greenF()) * t),
                            float(a.blueF() + (b.blueF() - a.blueF()) * t),
                            float(a.alphaF() + (b.alphaF() - a.alphaF()) * t));
}

inline QFont font(qreal pt, bool bold = false)
{
    QFont f(QStringLiteral("Segoe UI"));
    f.setPointSizeF(pt);
    f.setBold(bold);
    return f;
}

// Empreinte de patte (logo animation unique d'ANIWORLD)
inline QPainterPath pawPath(const QRectF &r)
{
    QPainterPath path;
    const qreal sx = r.width() / 24.0, sy = r.height() / 24.0;
    auto ell = [&](qreal cx, qreal cy, qreal rx, qreal ry) {
        path.addEllipse(QPointF(r.left() + cx * sx, r.top() + cy * sy), rx * sx, ry * sy);
    };
    ell(6, 11, 2.3, 2.9);
    ell(10.5, 6.5, 2.3, 2.9);
    ell(16, 7, 2.3, 2.9);
    ell(19.5, 12, 2.3, 2.9);
    ell(12.7, 17, 5.4, 4.3);
    return path;
}

inline void stateColors(RoomState s, QColor &bg, QColor &fg)
{
    switch (s) {
    case RoomState::Available:    bg = GreenLight;  fg = Green;      break;
    case RoomState::Occupied:     bg = BlueLight;   fg = Blue;       break;
    case RoomState::OutOfService: bg = OrangeLight; fg = OrangeText; break;
    }
}

inline void cleaningColors(CleaningState s, QColor &bg, QColor &fg)
{
    switch (s) {
    case CleaningState::Clean:      bg = GreenLight;  fg = Green;      break;
    case CleaningState::ToClean:    bg = RedLight;    fg = Red;        break;
    case CleaningState::InProgress: bg = OrangeLight; fg = OrangeText; break;
    }
}

inline qreal pillWidth(const QString &text, const QFont &f, bool dot)
{
    return QFontMetricsF(f).horizontalAdvance(text) + (dot ? 34.0 : 22.0);
}

// Petite pastille arrondie (badge d'etat, occupation...)
inline void drawPill(QPainter &p, const QRectF &r, const QString &text,
                     const QColor &bg, const QColor &fg, const QFont &f, bool dot)
{
    p.save();
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(r, r.height() / 2.0, r.height() / 2.0);
    qreal x = r.left() + 11;
    if (dot) {
        p.setBrush(fg);
        p.drawEllipse(QPointF(x + 3, r.center().y()), 3, 3);
        x += 12;
    }
    p.setPen(fg);
    p.setFont(f);
    p.drawText(QRectF(x, r.top(), r.right() - x, r.height()),
               Qt::AlignVCenter | Qt::AlignLeft, text);
    p.restore();
}

} // namespace Theme
