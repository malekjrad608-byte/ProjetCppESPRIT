#include "../widgets/animatedbutton.h"
#include "../ui/motion.h"
#include "hoverglow.h"
#include "theme.h"

#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QWidget>

void HoverGlow::attach(QWidget *w, const QColor &color, qreal blur, qreal offsetY)
{
    if (w && !dynamic_cast<AnimatedButton *>(w))
        new HoverGlow(w, color, blur, offsetY);
}

HoverGlow::HoverGlow(QWidget *w, const QColor &color, qreal blur, qreal offsetY)
    : QObject(w), m_w(w), m_color(color), m_blur(blur), m_off(offsetY)
{
    m_color = QColor(245, 158, 11, 110);
    m_anim = new QPropertyAnimation(this, "level", this);
    m_anim->setDuration(220);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_anim, &QPropertyAnimation::finished, this, [this] {
        if (m_level < 0.001)
            removeEffect();
    });
    w->installEventFilter(this);
    connect(&Motion::instance(), &Motion::changed, this, [this](bool enabled) {
        if (!enabled) { m_anim->stop(); m_level=0; removeEffect(); }
    });
}

void HoverGlow::ensureEffect()
{
    if (m_fx)
        return;
    m_fx = new QGraphicsDropShadowEffect(m_w);
    m_fx->setOffset(0, 0);
    m_fx->setBlurRadius(0);
    QColor c = m_color;
    c.setAlpha(0);
    m_fx->setColor(c);
    m_w->setGraphicsEffect(m_fx);
}

void HoverGlow::removeEffect()
{
    if (m_fx) {
        m_w->setGraphicsEffect(nullptr);   // supprime aussi l'effet
        m_fx = nullptr;
    }
}

void HoverGlow::setLevel(qreal v)
{
    m_level = v;
    if (!m_fx)
        return;
    m_fx->setBlurRadius(m_blur * v);
    m_fx->setOffset(0, m_off * v);
    QColor c = m_color;
    c.setAlphaF(m_color.alphaF() * v);
    m_fx->setColor(c);
}

void HoverGlow::animateTo(qreal target)
{
    m_anim->stop();
    m_anim->setStartValue(m_level);
    m_anim->setEndValue(target);
    m_anim->start();
}

bool HoverGlow::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_w && Theme::animations) {
        if (e->type() == QEvent::Enter) {
            ensureEffect();
            animateTo(1.0);
        } else if (e->type() == QEvent::Leave) {
            if (m_fx)
                animateTo(0.0);
        }
    }
    return QObject::eventFilter(obj, e);
}
