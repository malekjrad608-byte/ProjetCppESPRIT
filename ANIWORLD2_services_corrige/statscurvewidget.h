#ifndef STATSCURVEWIDGET_H
#define STATSCURVEWIDGET_H

#include <QWidget>
#include <QStringList>
#include <QVector>

// Courbe simple (sans module Qt Charts) : nombre de cas par mois.
class StatsCurveWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StatsCurveWidget(QWidget *parent = nullptr);
    // labels : un libellé par point (ex: "10/26"), values : nombre de cas
    void setData(const QStringList &labels, const QVector<int> &values, const QString &title);
    QSize minimumSizeHint() const override { return QSize(360, 220); }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QPointF pointAt(int i, const QRectF &plot, int maxY) const;
    QStringList m_labels;
    QVector<int> m_values;
    QString m_title;
    int m_hover = -1;
};

#endif // STATSCURVEWIDGET_H
