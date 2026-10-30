#pragma once
#include <QColor>
#include <QGraphicsEffect>
#include <QObject>
#include <QPointer>

class QPropertyAnimation;
class QWidget;

// Effet "hover" : une lueur teal qui apparait en douceur sous le widget survole.
// Usage :  HoverGlow::attach(monBouton);
class HoverGlow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal level READ level WRITE setLevel)
public:
    static void attach(QWidget *w, const QColor &color = QColor(31, 85, 96, 120),
                       qreal blur = 26, qreal offsetY = 6);

    qreal level() const { return m_level; }
    void setLevel(qreal v);

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    HoverGlow(QWidget *w, const QColor &color, qreal blur, qreal offsetY);
    void animateTo(qreal target);
    void ensureEffect();
    void removeEffect();

    QWidget *m_w;
    QColor m_color;
    qreal m_blur;
    qreal m_off;
    qreal m_level = 0.0;
    QPointer<QGraphicsDropShadowEffect> m_fx;
    QPropertyAnimation *m_anim;
};
