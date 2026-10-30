#include "animatedlogo.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include <QFocusEvent>
#include <QPainter>

AnimatedLogo::AnimatedLogo(QWidget *parent) : QAbstractButton(parent),
    m_logo(Design::brand("logo", QSize(228,228))),
    m_movie(new QMovie(":/brand/logo-hover.gif", QByteArray(), this)),
    m_zoom(new QVariantAnimation(this)) {
    setObjectName("animatedBrandLogo");
    setFixedSize(76,76);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setAccessibleName("ANIWORLD — Tableau de bord");
    setAccessibleDescription("Logo animé au survol. Ouvrir le tableau de bord.");
    setToolTip("ANIWORLD · Une attention à chaque patte.\nCliquer pour ouvrir le tableau de bord.");
    m_movie->setObjectName("brandHoverMovie");
    m_movie->setCacheMode(QMovie::CacheAll);
    m_zoom->setDuration(220);
    m_zoom->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_movie,&QMovie::frameChanged,this,[this] { update(); });
    connect(m_zoom,&QVariantAnimation::valueChanged,this,[this](const QVariant &value) {
        m_progress=value.toReal(); update();
    });
    connect(m_zoom,&QVariantAnimation::finished,this,[this] {
        if (m_progress==0) m_movie->jumpToFrame(0);
    });
    connect(&Motion::instance(),&Motion::changed,this,[this](bool enabled) {
        if(!enabled) { m_zoom->stop(); m_movie->stop(); m_progress=0; update(); }
    });
}

void AnimatedLogo::animateTo(bool active) {
    m_zoom->stop();
    if(!Motion::instance().enabled()) { m_movie->stop(); m_progress=0; update(); return; }
    if (active && m_movie->isValid()) m_movie->start();
    else if (!active) m_movie->setPaused(true);
    m_zoom->setStartValue(m_progress);
    m_zoom->setEndValue(active ? 1.0 : 0.0);
    m_zoom->start();
}

bool AnimatedLogo::event(QEvent *event) {
    if (event->type()==QEvent::Enter) { m_hovered=true; animateTo(true); }
    else if (event->type()==QEvent::Leave) { m_hovered=false; animateTo(m_keyboardFocus); }
    else if (event->type()==QEvent::FocusIn) {
        const auto reason=static_cast<QFocusEvent *>(event)->reason();
        m_keyboardFocus=reason==Qt::TabFocusReason || reason==Qt::BacktabFocusReason || reason==Qt::ShortcutFocusReason;
        if (m_keyboardFocus) animateTo(true);
    } else if (event->type()==QEvent::FocusOut) {
        m_keyboardFocus=false; animateTo(m_hovered);
    } else if (event->type()==QEvent::Hide || event->type()==QEvent::WindowDeactivate) {
        m_zoom->stop(); m_movie->stop(); m_progress=0;
        m_hovered=false; m_keyboardFocus=false; update();
    }
    return QAbstractButton::event(event);
}

void AnimatedLogo::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    // Animate only the painted image, keeping the sidebar layout stationary.
    const qreal size=qMin(width(),height())*(62.0/76.0+(9.0/76.0)*m_progress)-(isDown()?2:0);
    const QRectF target((width()-size)/2,(height()-size)/2,size,size);
    painter.drawPixmap(target,m_logo,QRectF(m_logo.rect()));
    if (m_progress>0 && !m_movie->currentPixmap().isNull()) {
        painter.setOpacity(m_progress);
        painter.drawPixmap(rect(),m_movie->currentPixmap());
    }
    if (m_keyboardFocus) {
        painter.setOpacity(1); painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor("#377E8E"),1,Qt::DashLine));
        painter.drawRoundedRect(rect().adjusted(1,1,-2,-2),10,10);
    }
}
