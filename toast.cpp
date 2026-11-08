#include "toast.h"
#include "theme.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

Toast::Toast(QWidget *parent) : QLabel(parent)
{
    setObjectName("toast");
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_fx = new QGraphicsOpacityEffect(this);
    m_fx->setOpacity(0.0);
    setGraphicsEffect(m_fx);

    m_anim = new QPropertyAnimation(m_fx, "opacity", this);
    m_anim->setDuration(250);
    connect(m_anim, &QPropertyAnimation::finished, this, [this] {
        if (m_fadingOut)
            hide();
    });

    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, [this] { fadeTo(0.0); });
    hide();
}

void Toast::fadeTo(qreal target)
{
    m_fadingOut = (target <= 0.0);
    m_anim->stop();
    m_anim->setStartValue(m_fx->opacity());
    m_anim->setEndValue(target);
    m_anim->setDuration(Theme::animations ? 250 : 0);
    m_anim->start();
}

void Toast::showMessage(const QString &text)
{
    setText(text);
    adjustSize();
    if (QWidget *p = parentWidget())
        move((p->width() - width()) / 2, p->height() - height() - 34);
    show();
    raise();
    fadeTo(1.0);
    m_timer.start(2400);
}
