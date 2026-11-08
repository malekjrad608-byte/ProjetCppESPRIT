#pragma once
#include <QHash>
#include <QList>
#include <QPainterPath>
#include <QTimer>
#include <QWidget>
#include "local.h"

class QVariantAnimation;

// Plan de la clinique en 2.5D (vue isometrique), dessine avec QPainter.
// - chaque etage est une dalle, chaque local est une "boite" posee dessus
// - couleur de la boite = ETAT du local (Disponible, Occupé, À nettoyer, Hors service)
// - glisser avec la souris (ou bouton "Tourner") fait tourner le batiment
// - "Vue éclatée" ecarte les etages pour mieux voir chaque niveau
// - ajout d'un local : la boite "pousse" ; suppression : elle s'enfonce et disparait
// - survol : la boite se souleve ; clic : selection du local
class ClinicMap : public QWidget
{
    Q_OBJECT
public:
    explicit ClinicMap(bool compact, QWidget *parent = nullptr);

    void setLocaux(const QList<Local> &list);
    void setSelected(int id);

    void rotateQuarter();             // +90 degres, anime
    void setExploded(bool on);        // ecarte / rapproche les etages, anime
    bool isExploded() const { return m_exploded; }

    QSize sizeHint() const override;

signals:
    void roomClicked(int id);

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    struct Box {
        Local l;
        int floorIndex = 0;           // 0 = etage le plus bas affiche
        qreal x = 0, y = 0;           // coin de la boite sur la dalle (unites "monde")
        qreal w = 1, d = 1;           // largeur / profondeur
        qreal fromX = 0, fromY = 0, fromZ = 0;
        bool moved = false;
        bool isNew = false;
        bool leaving = false;
    };

    void layoutBoxes();
    QPointF project(qreal x, qreal y, qreal z) const;   // monde -> ecran
    qreal depth(qreal x, qreal y) const;                // pour l'ordre de dessin
    qreal floorZ(int floorIndex) const;
    void computeFit();
    int roomAt(const QPointF &p) const;
    QColor colorFor(const Local &l) const;

    bool m_compact;
    QList<Local> m_list;
    QList<Box> m_boxes;               // locaux affiches (+ ceux qui disparaissent)
    QList<int> m_floors;              // numeros d'etage, du plus bas au plus haut
    int m_cols = 4;
    int m_rows = 1;
    qreal m_slabW = 1, m_slabD = 1;

    // vue
    qreal m_angle;                    // rotation du batiment (radians)
    qreal m_gap;                      // espace entre deux etages
    bool m_exploded = false;
    qreal m_scale = 40;
    QPointF m_origin;

    // interaction
    int m_selected = 0;
    int m_hover = 0;
    qreal m_lift = 0;                 // 0..1 : soulevement de la boite survolee
    bool m_dragging = false;
    bool m_pressed = false;
    QPointF m_pressPos;
    qreal m_pressAngle = 0;
    QList<QPair<int, QPainterPath>> m_hit;   // zones cliquables (ordre de dessin)

    // animations
    qreal m_t = 1.0;                  // apparition / disparition / deplacement
    qreal m_phase = 0.0;              // clignotement "À nettoyer"
    QVariantAnimation *m_anim;
    QVariantAnimation *m_rotAnim;
    QVariantAnimation *m_gapAnim;
    QVariantAnimation *m_liftAnim;
    QTimer m_pulse;
};
