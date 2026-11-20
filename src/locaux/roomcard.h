#pragma once
#include <QPointF>
#include <QWidget>
#include "local.h"

class QPropertyAnimation;

// Carte d'un local, entierement dessinee a la main (QPainter) pour pouvoir
// animer : soulevement, lueur qui suit la souris, patte en filigrane,
// icone qui se secoue, apparition en cascade.
class RoomCard : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal hover READ hover WRITE setHover)
    Q_PROPERTY(qreal intro READ intro WRITE setIntro)
public:
    static constexpr int CardWidth = 270;
    static constexpr int CardHeight = 182;

    explicit RoomCard(const Local &l, QWidget *parent = nullptr);

    int localId() const { return m_local.id; }
    void setSelected(bool on);
    void setAlertLevel(int level);   // 0 aucune, 1 attention, 2 critique
    void playIntro(int delayMs);

    qreal hover() const { return m_hover; }
    void setHover(qreal v);
    qreal intro() const { return m_intro; }
    void setIntro(qreal v);

signals:
    void clicked(int id);

protected:
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    void animateHover(qreal target);

    Local m_local;
    bool m_selected = false;
    bool m_pressed = false;
    int m_alert = 0;
    qreal m_hover = 0.0;
    qreal m_intro = 1.0;
    QPointF m_mouse;
    QPropertyAnimation *m_hoverAnim;
};
