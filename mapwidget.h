#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QVector>
#include <QRect>
#include <QColor>
#include "salle.h"

class MapWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapWidget(QWidget* parent = nullptr);

    void setSalles(const QVector<Salle*>& salles);

signals:
    void salleCliquee(int idSalle);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QVector<Salle*> salles;
    QVector<QRect> rectangles;

    QColor couleurPourType(const QString& type) const;
    void calculerPositions();
};

#endif // MAPWIDGET_H