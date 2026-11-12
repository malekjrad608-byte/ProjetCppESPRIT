#pragma once
#include <QLabel>
#include <QTimer>

class QGraphicsOpacityEffect;
class QPropertyAnimation;

// Petit message qui apparait en bas de la page puis disparait
class Toast : public QLabel
{
    Q_OBJECT
public:
    explicit Toast(QWidget *parent);
    void showMessage(const QString &text);

private:
    void fadeTo(qreal target);

    QGraphicsOpacityEffect *m_fx;
    QPropertyAnimation *m_anim;
    QTimer m_timer;
    bool m_fadingOut = false;
};
