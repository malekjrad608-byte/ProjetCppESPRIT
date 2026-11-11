#include "motion.h"
#include <QApplication>
#include <QLabel>
#include <QPainter>
#include <QPointer>
#include <QProgressBar>
#include <QSettings>
#include <QTimer>
#include <QVariantAnimation>
#include <QEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QAbstractSpinBox>
#include <QTextEdit>
#include <QPainterPath>

namespace {
// An input-transparent overlay keeps native editing, dropdowns and validation intact.
class InputGlow : public QWidget {
public:
    explicit InputGlow(QWidget *target):QWidget(target),m_animation(new QVariantAnimation(this)) {
        setObjectName("inputHoverGlow"); setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground); setFocusPolicy(Qt::NoFocus);
        setGeometry(target->rect()); target->installEventFilter(this);
        m_animation->setObjectName("inputHoverMotion"); m_animation->setDuration(180);
        m_animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(m_animation,&QVariantAnimation::valueChanged,this,[this](const QVariant &v) { m_progress=v.toReal(); update(); });
        connect(&Motion::instance(),&Motion::changed,this,[this](bool enabled) {
            if(!enabled) { m_animation->stop(); m_progress=0; update(); }
        });
        show(); raise();
    }
protected:
    bool eventFilter(QObject *,QEvent *event) override {
        const auto type=event->type();
        if(type==QEvent::Resize) setGeometry(parentWidget()->rect());
        if(type==QEvent::Show) { show(); raise(); }
        if(type==QEvent::Hide || (type==QEvent::EnabledChange && !parentWidget()->isEnabled())) {
            m_animation->stop(); m_progress=0; m_hovered=false; update();
        } else if(type==QEvent::Enter || type==QEvent::Leave || type==QEvent::FocusIn || type==QEvent::FocusOut) {
            if(type==QEvent::Enter) m_hovered=true;
            if(type==QEvent::Leave) m_hovered=false;
            if(parentWidget()->isEnabled() && Motion::instance().enabled()) {
                const bool focused=type==QEvent::FocusIn || (type!=QEvent::FocusOut && parentWidget()->hasFocus());
                m_animation->stop(); m_animation->setStartValue(m_progress);
                m_animation->setEndValue(m_hovered || focused?1.0:0.0); m_animation->start(); raise();
            }
        }
        return false;
    }
    void paintEvent(QPaintEvent *) override {
        if(m_progress<=0) return;
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing); p.setBrush(Qt::NoBrush);
        // Preserve the error border when validation flags a field.
        const QColor accent(parentWidget()->property("invalid").toBool()?"#C26B5E":"#FF9B28");
        for(int width:{7,4,2}) {
            QColor color=accent; color.setAlphaF((width==2?.9:.08)*m_progress);
            p.setPen(QPen(color,width)); p.drawRoundedRect(QRectF(rect()).adjusted(2,2,-2,-2),6,6);
        }
    }
private:
    QVariantAnimation *m_animation;
    qreal m_progress=0;
    bool m_hovered=false;
};

// Fade a decorative veil over the current, fully painted page. Never change
// the opacity of the live page or include the outgoing page in the animation.
class Reveal : public QWidget {
public:
    Reveal(QWidget *target, const QColor &color, int duration) : QWidget(target), m_color(color) {
        setObjectName("motionReveal");
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setGeometry(target->rect());
        target->installEventFilter(this);
        auto *animation=new QVariantAnimation(this);
        animation->setObjectName("revealMotion");
        animation->setStartValue(0.0); animation->setEndValue(1.0);
        animation->setDuration(duration); animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation,&QVariantAnimation::valueChanged,this,[this](const QVariant &value) {
            m_progress=value.toReal(); update();
        });
        connect(animation,&QVariantAnimation::finished,this,[this] { hide(); deleteLater(); });
        connect(&Motion::instance(),&Motion::changed,this,[this](bool enabled) { if(!enabled) { hide(); deleteLater(); } });
        show(); raise(); animation->start();
    }
protected:
    bool eventFilter(QObject *,QEvent *event) override {
        if(event->type()==QEvent::Resize) setGeometry(parentWidget()->rect());
        if(event->type()==QEvent::Hide) { hide(); deleteLater(); }
        return false;
    }
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this); QColor color=m_color;
        color.setAlphaF(.72*(1-m_progress)); painter.fillRect(rect(),color);
    }
private:
    QColor m_color;
    qreal m_progress=0;
};
}

Motion::Motion(QObject *parent):QObject(parent),
    m_enabled(QSettings("ANIWORLD","ANIWORLD").value("interface/animations",true).toBool()) {}
Motion &Motion::instance() {
    static auto *motion=new Motion(qApp);
    return *motion;
}
void Motion::installHoverEffects() { qApp->installEventFilter(&instance()); }
bool Motion::eventFilter(QObject *object,QEvent *event) {
    if(event->type()==QEvent::Polish) {
        auto *widget=qobject_cast<QWidget *>(object);
        const bool input=qobject_cast<QLineEdit *>(widget) || qobject_cast<QComboBox *>(widget)
            || qobject_cast<QAbstractSpinBox *>(widget) || qobject_cast<QTextEdit *>(widget);
        if(input && !widget->findChild<QWidget *>("inputHoverGlow",Qt::FindDirectChildrenOnly)
            && !qobject_cast<QAbstractSpinBox *>(widget->parentWidget()) && !qobject_cast<QComboBox *>(widget->parentWidget()))
            new InputGlow(widget);
    }
    return QObject::eventFilter(object,event);
}
void Motion::setEnabled(bool enabled,bool persist) {
    if(persist) QSettings("ANIWORLD","ANIWORLD").setValue("interface/animations",enabled);
    if(m_enabled==enabled) return;
    m_enabled=enabled; emit changed(enabled);
}
void Motion::reveal(QWidget *widget,const QColor &background,int duration) {
    if(!widget) return;
    const int generation=widget->property("revealGeneration").toInt()+1;
    widget->setProperty("revealGeneration",generation);
    for(auto *old:widget->findChildren<QWidget *>("motionReveal",Qt::FindDirectChildrenOnly)) delete old;
    if(!instance().enabled()) return;
    QPointer<QWidget> guarded(widget);
    QTimer::singleShot(0,widget,[guarded,background,duration,generation] {
        if(guarded && guarded->isVisible() && Motion::instance().enabled()
            && guarded->property("revealGeneration").toInt()==generation)
            new Reveal(guarded,background,duration);
    });
}
void Motion::countTo(QLabel *label,int value,bool fromZero) {
    label->setProperty("motionTarget",value);
    auto *animation=label->findChild<QVariantAnimation *>("countMotion",Qt::FindDirectChildrenOnly);
    if(!animation) {
        animation=new QVariantAnimation(label); animation->setObjectName("countMotion");
        animation->setDuration(420); animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation,&QVariantAnimation::valueChanged,label,[label](const QVariant &v) { label->setText(QString::number(v.toInt())); });
        connect(&instance(),&Motion::changed,label,[label,animation](bool enabled) {
            if(!enabled) { animation->stop(); label->setText(QString::number(label->property("motionTarget").toInt())); }
        });
    }
    animation->stop();
    if(!instance().enabled() || !label->isVisible()) { label->setText(QString::number(value)); return; }
    const int start=fromZero?0:label->text().toInt();
    if(start==value) return;
    animation->setStartValue(start); animation->setEndValue(value); animation->start();
}
void Motion::progressTo(QProgressBar *bar,int value,bool fromZero) {
    bar->setProperty("motionTarget",value);
    auto *animation=bar->findChild<QVariantAnimation *>("progressMotion",Qt::FindDirectChildrenOnly);
    if(!animation) {
        animation=new QVariantAnimation(bar); animation->setObjectName("progressMotion");
        animation->setDuration(460); animation->setEasingCurve(QEasingCurve::OutCubic);
        connect(animation,&QVariantAnimation::valueChanged,bar,[bar](const QVariant &v) { bar->setValue(v.toInt()); });
        connect(&instance(),&Motion::changed,bar,[bar,animation](bool enabled) {
            if(!enabled) { animation->stop(); bar->setValue(bar->property("motionTarget").toInt()); }
        });
    }
    animation->stop();
    if(!instance().enabled() || !bar->isVisible()) { bar->setValue(value); return; }
    const int start=fromZero?0:qMax(0,bar->value());
    if(start==value) { bar->setValue(value); return; }
    animation->setStartValue(start); animation->setEndValue(value); animation->start();
}
