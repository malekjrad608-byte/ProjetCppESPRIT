#pragma once
#include <QHash>
#include <QList>
#include <QTimer>
#include <QWidget>
#include "local.h"

class QVariantAnimation;

// Plan de la clinique dessine automatiquement a partir des locaux.
// - un bloc par etage, une tuile par local (couleur = etat)
// - quand un local est ajoute : la tuile "pousse" ; quand il est supprime :
//   les autres tuiles glissent pour combler le vide
// - les locaux a nettoyer clignotent
class ClinicMap : public QWidget
{
    Q_OBJECT
public:
    explicit ClinicMap(bool compact, QWidget *parent = nullptr);

    void setLocaux(const QList<Local> &list);
    void setSelected(int id);

signals:
    void roomClicked(int id);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    struct Tile {
        Local l;
        QRectF to;
        QRectF from;
        bool isNew = false;
    };
    struct FloorBox {
        int floor = 0;
        QRectF box;
        int rooms = 0;
        int busy = 0;
    };

    void relayout(bool animate);
    QRectF currentRect(const Tile &t) const;
    int tileAt(const QPoint &p) const;

    bool m_compact;
    QList<Local> m_list;
    QList<Tile> m_tiles;
    QList<FloorBox> m_floors;
    int m_selected = 0;
    int m_hover = 0;
    qreal m_t = 1.0;
    qreal m_phase = 0.0;
    QVariantAnimation *m_anim;
    QTimer m_pulse;
};
