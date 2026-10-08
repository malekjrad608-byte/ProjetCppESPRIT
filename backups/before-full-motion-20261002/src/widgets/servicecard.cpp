#include "servicecard.h"
#include "../ui/design.h"
#include <QPainter>
#include <QLocale>
#include <QEvent>

namespace {
QColor blend(const QColor &from, const QColor &to, qreal progress) {
    return QColor::fromRgbF(from.redF()+(to.redF()-from.redF())*progress,
                            from.greenF()+(to.greenF()-from.greenF())*progress,
                            from.blueF()+(to.blueF()-from.blueF())*progress);
}
}

ServiceCard::ServiceCard(const Service &service, QWidget *parent)
    : QAbstractButton(parent), m_service(service), m_hoverAnimation(new QVariantAnimation(this)) {
    setCheckable(true);
    setFixedHeight(184);
    setMinimumWidth(245);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setObjectName("serviceCard_" + QString::number(service.id));
    setAccessibleName(service.name + ", " + service.status);
    setToolTip(service.name + "\n" + service.description);
    m_hoverAnimation->setObjectName("cardHoverAnimation");
    m_hoverAnimation->setDuration(160);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_hoverAnimation,&QVariantAnimation::valueChanged,this,[this](const QVariant &value) {
        m_hoverProgress=value.toReal(); update();
    });
}

bool ServiceCard::event(QEvent *event) {
    const auto type=event->type();
    if(type==QEvent::Hide) {
        m_hoverAnimation->stop(); m_hoverProgress=0; m_hovered=false;
    } else if(type==QEvent::Enter || type==QEvent::Leave || type==QEvent::FocusIn || type==QEvent::FocusOut) {
        if(type==QEvent::Enter) m_hovered=true;
        if(type==QEvent::Leave) m_hovered=false;
        const bool focused=type==QEvent::FocusIn || (type!=QEvent::FocusOut && hasFocus());
        m_hoverAnimation->stop();
        m_hoverAnimation->setStartValue(m_hoverProgress);
        m_hoverAnimation->setEndValue(m_hovered || focused ? 1.0 : 0.0);
        m_hoverAnimation->start();
    }
    return QAbstractButton::event(event);
}

void ServiceCard::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setPen(QPen(isChecked() ? QColor("#E7A342") : blend(QColor("#DFE8EB"),QColor("#72A3AE"),m_hoverProgress), isChecked() ? 2 : 1));
    p.setBrush(isChecked() ? QColor("#FFFAF2") : blend(Qt::white,QColor("#EFF7F9"),m_hoverProgress));
    p.drawRoundedRect(QRectF(1,1,width()-2,height()-2), 12,12);
    p.setPen(Qt::NoPen); p.setBrush(blend(QColor("#EAF3F5"),QColor("#D7EBF0"),m_hoverProgress)); p.drawRoundedRect(QRect(18,17,31,31),8,8);
    Design::icon(m_service.category == "Soins & bien-être" ? "heart" : "paw", QColor("#4C8291")).paint(&p,QRect(25,24,17,17));
    QString bg="#EAEAF8", fg="#7374A9";
    if (m_service.status=="En cours") { bg="#FFF0D7"; fg="#AA7830"; }
    if (m_service.status=="Terminé") { bg="#E5F2E9"; fg="#4D8A6B"; }
    if (m_service.status=="Annulé") { bg="#FBE9E8"; fg="#B36463"; }
    const QRect badge(width()-113,21,94,25);
    p.setBrush(QColor(bg)); p.drawRoundedRect(badge,12,12);
    QFont f("Segoe UI"); f.setPixelSize(10); f.setWeight(QFont::DemiBold); p.setFont(f); p.setPen(QColor(fg));
    p.drawText(badge,Qt::AlignCenter,"●  "+m_service.status);
    f.setPixelSize(16); f.setWeight(QFont::DemiBold); p.setFont(f); p.setPen(QColor("#315D6C"));
    p.drawText(QRect(19,60,width()-38,25),Qt::AlignVCenter,p.fontMetrics().elidedText(m_service.name,Qt::ElideRight,width()-38));
    f.setPixelSize(12); f.setWeight(QFont::Normal); p.setFont(f); p.setPen(QColor("#84979E"));
    p.drawText(QRect(19,89,width()-38,20),m_service.category);
    f.setPixelSize(10); p.setFont(f);
    p.drawText(QRect(19,115,width()-38,18),QString("SRV-%1   ·   %2 min").arg(m_service.id,3,10,QChar('0')).arg(m_service.duration));
    p.setPen(QColor("#E8EEF0")); p.drawLine(19,143,width()-19,143);
    Design::icon("calendar",QColor("#8B9EA5")).paint(&p,QRect(19,156,13,13));
    f.setPixelSize(11); p.setFont(f); p.setPen(QColor("#617F8A"));
    p.drawText(QRect(39,152,140,23),Qt::AlignVCenter,QLocale().toString(m_service.dateTime,"dd MMM · HH:mm"));
    f.setWeight(QFont::DemiBold); p.setFont(f); p.setPen(QColor("#377D8E"));
    p.drawText(QRect(width()-111,152,91,23),Qt::AlignRight|Qt::AlignVCenter,Design::money(m_service.price));
}
