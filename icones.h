#ifndef ICONES_H
#define ICONES_H

#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QPen>
#include <QColor>
#include <QString>

namespace Icones {

inline QPixmap pixmap(const QString& nom, const QColor& couleur, int taille = 18)
{
    const qreal dpr = 2.0;
    QPixmap pm(int(taille * dpr), int(taille * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.scale(taille / 24.0, taille / 24.0);
    p.setPen(QPen(couleur, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);

    if (nom == "dashboard") {
        p.drawRoundedRect(QRectF(3, 3, 18, 18), 2.5, 2.5);
        p.drawLine(QPointF(3, 9), QPointF(21, 9));
        p.drawLine(QPointF(9, 9), QPointF(9, 21));
    }
    else if (nom == "calendar") {
        p.drawRoundedRect(QRectF(3, 5, 18, 16), 2.5, 2.5);
        p.drawLine(QPointF(3, 10), QPointF(21, 10));
        p.drawLine(QPointF(8, 3), QPointF(8, 7));
        p.drawLine(QPointF(16, 3), QPointF(16, 7));
    }
    else if (nom == "users") {
        p.drawEllipse(QPointF(9, 8), 3.5, 3.5);
        QPainterPath corps;
        corps.moveTo(2.5, 20);
        corps.quadTo(2.5, 14, 9, 14);
        corps.quadTo(15.5, 14, 15.5, 20);
        p.drawPath(corps);
        p.drawEllipse(QPointF(17.5, 9), 2.5, 2.5);
        QPainterPath corps2;
        corps2.moveTo(18, 14.2);
        corps2.quadTo(21.5, 14.5, 21.5, 19);
        p.drawPath(corps2);
    }
    else if (nom == "box") {
        QPolygonF hexa;
        hexa << QPointF(12, 2.5) << QPointF(20.5, 7) << QPointF(20.5, 17)
             << QPointF(12, 21.5) << QPointF(3.5, 17) << QPointF(3.5, 7);
        p.drawPolygon(hexa);
        p.drawLine(QPointF(3.5, 7), QPointF(12, 11.5));
        p.drawLine(QPointF(12, 11.5), QPointF(20.5, 7));
        p.drawLine(QPointF(12, 11.5), QPointF(12, 21.5));
    }
    else if (nom == "rooms" || nom == "grid") {
        p.drawRoundedRect(QRectF(4, 4, 6, 6), 1.5, 1.5);
        p.drawRoundedRect(QRectF(14, 4, 6, 6), 1.5, 1.5);
        p.drawRoundedRect(QRectF(4, 14, 6, 6), 1.5, 1.5);
        p.drawRoundedRect(QRectF(14, 14, 6, 6), 1.5, 1.5);
    }
    else if (nom == "shield") {
        QPainterPath bouclier;
        bouclier.moveTo(12, 2.5);
        bouclier.lineTo(20, 5.5);
        bouclier.lineTo(20, 12);
        bouclier.cubicTo(20, 17, 16, 20.5, 12, 21.5);
        bouclier.cubicTo(8, 20.5, 4, 17, 4, 12);
        bouclier.lineTo(4, 5.5);
        bouclier.closeSubpath();
        p.drawPath(bouclier);
        p.drawLine(QPointF(8.5, 12), QPointF(11, 14.5));
        p.drawLine(QPointF(11, 14.5), QPointF(15.5, 9.5));
    }
    else if (nom == "search") {
        p.drawEllipse(QPointF(11, 11), 6.5, 6.5);
        p.drawLine(QPointF(16, 16), QPointF(21, 21));
    }

    p.end();
    return pm;
}

inline QIcon creer(const QString& nom, const QColor& couleur, int taille = 18)
{
    return QIcon(pixmap(nom, couleur, taille));
}

inline QIcon deuxEtats(const QString& nom, const QColor& off, const QColor& on, int taille = 18)
{
    QIcon ic;
    ic.addPixmap(pixmap(nom, off, taille), QIcon::Normal, QIcon::Off);
    ic.addPixmap(pixmap(nom, on,  taille), QIcon::Normal, QIcon::On);
    return ic;
}

} // namespace Icones

#endif // ICONES_H