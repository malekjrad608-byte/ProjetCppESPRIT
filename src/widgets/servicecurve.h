#pragma once
#include <QWidget>
#include <QVariantAnimation>
#include <QDate>
#include <QVector>

class ServiceCurve : public QWidget {
    Q_OBJECT
public:
    explicit ServiceCurve(QWidget *parent=nullptr);
    void setSeries(const QVector<QDate> &dates,const QVector<QVector<double>> &values,const QString &unit);
    const QVector<QVector<double>> &values() const {return m_values;}
    const QVector<QDate> &dates() const {return m_dates;}
protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;
private:
    QRectF plot() const;
    QVector<QDate> m_dates;
    QVector<QVector<double>> m_values;
    QString m_unit;
    QVariantAnimation m_animation;
    double m_progress=1;
    int m_hover=-1;
};
