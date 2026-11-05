#include "statscurvewidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QToolTip>
#include <QtMath>
#include <algorithm>

StatsCurveWidget::StatsCurveWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_StyledBackground, false);
}

void StatsCurveWidget::setData(const QStringList &labels, const QVector<int> &values, const QString &title)
{
    m_labels = labels;
    m_values = values;
    m_title = title;
    m_hover = -1;
    update();
}

static int niceMax(int v)
{
    if (v <= 4) return 4;
    const int step = (v <= 10) ? 2 : (v <= 20 ? 5 : 10);
    return ((v + step - 1) / step) * step;
}

QPointF StatsCurveWidget::pointAt(int i, const QRectF &plot, int maxY) const
{
    const int n = m_values.size();
    const qreal x = n > 1 ? plot.left() + plot.width() * i / (n - 1) : plot.center().x();
    const qreal y = plot.bottom() - plot.height() * m_values[i] / qreal(maxY);
    return QPointF(x, y);
}

void StatsCurveWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#FFFFFF"));
    p.setPen(QPen(QColor("#D7E9ED"), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 12, 12);

    p.setPen(QColor("#176078"));
    QFont tf = font(); tf.setPointSize(10); tf.setBold(true);
    p.setFont(tf);
    p.drawText(QRectF(16, 8, width() - 32, 24), Qt::AlignLeft | Qt::AlignVCenter, m_title);

    if (m_values.isEmpty()) {
        p.setPen(QColor("#78909C"));
        p.drawText(rect(), Qt::AlignCenter, "Aucune donnée");
        return;
    }

    const QRectF plot(46, 44, width() - 46 - 24, height() - 44 - 36);
    const int maxY = niceMax(*std::max_element(m_values.begin(), m_values.end()));

    QFont sf = font(); sf.setPointSize(8); sf.setBold(false);
    p.setFont(sf);
    const int ticks = 4;
    for (int t = 0; t <= ticks; ++t) {
        const qreal y = plot.bottom() - plot.height() * t / ticks;
        p.setPen(QPen(QColor("#E4EEF1"), 1));
        p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        p.setPen(QColor("#78909C"));
        p.drawText(QRectF(0, y - 9, plot.left() - 8, 18), Qt::AlignRight | Qt::AlignVCenter,
                   QString::number(maxY * t / ticks));
    }

    const int n = m_values.size();
    // étiquettes de l'axe X (on en saute si trop serrées)
    const int skip = qMax(1, qCeil(n / qMax(1.0, plot.width() / 52.0)));
    for (int i = 0; i < n; ++i) {
        if (i % skip) continue;
        const QPointF pt = pointAt(i, plot, maxY);
        p.setPen(QColor("#557783"));
        p.drawText(QRectF(pt.x() - 30, plot.bottom() + 8, 60, 18), Qt::AlignCenter, m_labels.value(i));
    }

    // courbe lissée (Bézier cubique)
    QVector<QPointF> pts;
    for (int i = 0; i < n; ++i) pts << pointAt(i, plot, maxY);

    QPainterPath line;
    line.moveTo(pts[0]);
    for (int i = 1; i < n; ++i) {
        const qreal dx = (pts[i].x() - pts[i - 1].x()) / 2.0;
        line.cubicTo(QPointF(pts[i - 1].x() + dx, pts[i - 1].y()),
                     QPointF(pts[i].x() - dx, pts[i].y()), pts[i]);
    }

    if (n > 1) {   // zone sous la courbe
        QPainterPath area = line;
        area.lineTo(pts.last().x(), plot.bottom());
        area.lineTo(pts.first().x(), plot.bottom());
        area.closeSubpath();
        QLinearGradient g(0, plot.top(), 0, plot.bottom());
        g.setColorAt(0, QColor(74, 146, 160, 90));
        g.setColorAt(1, QColor(74, 146, 160, 5));
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        p.drawPath(area);
    }

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor("#1E5A68"), 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(line);

    for (int i = 0; i < n; ++i) {
        const bool h = (i == m_hover);
        p.setPen(QPen(QColor("#1E5A68"), 2));
        p.setBrush(h ? QColor("#1E5A68") : QColor("#FFFFFF"));
        p.drawEllipse(pts[i], h ? 6.0 : 4.0, h ? 6.0 : 4.0);
        if (m_values[i] > 0 || h) {
            p.setPen(QColor("#176078"));
            p.drawText(QRectF(pts[i].x() - 20, pts[i].y() - 24, 40, 16), Qt::AlignCenter,
                       QString::number(m_values[i]));
        }
    }
}

void StatsCurveWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (m_values.isEmpty()) return;
    const QRectF plot(46, 44, width() - 46 - 24, height() - 44 - 36);
    const int maxY = niceMax(*std::max_element(m_values.begin(), m_values.end()));
    int best = -1; qreal bestD = 14;
    for (int i = 0; i < m_values.size(); ++i) {
        const qreal d = QLineF(pointAt(i, plot, maxY), e->position()).length();
        if (d < bestD) { bestD = d; best = i; }
    }
    if (best != m_hover) {
        m_hover = best;
        update();
    }
    if (best >= 0)
        QToolTip::showText(e->globalPosition().toPoint(),
                           QString("%1 : %2 cas").arg(m_labels.value(best)).arg(m_values[best]), this);
    else
        QToolTip::hideText();
}

void StatsCurveWidget::leaveEvent(QEvent *)
{
    m_hover = -1;
    update();
}
