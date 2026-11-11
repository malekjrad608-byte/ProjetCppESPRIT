#include "servicecard.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include <QPainter>
#include <QLocale>
#include <QEvent>
#include <QTimer>

namespace {
QColor blend(const QColor &from, const QColor &to, qreal progress) {
    return QColor::fromRgbF(from.redF()+(to.redF()-from.redF())*progress,
                            from.greenF()+(to.greenF()-from.greenF())*progress,
                            from.blueF()+(to.blueF()-from.blueF())*progress);
}
}

ServiceCard::ServiceCard(const Service &service, QWidget *parent)
    : QAbstractButton(parent), m_service(service), m_hoverAnimation(new QVariantAnimation(this)),
      m_entranceAnimation(new QVariantAnimation(this)) {
    setCheckable(true);
    setFixedHeight(144);
    setMinimumWidth(245);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setObjectName("serviceCard_" + QString::number(service.getId()));
    setAccessibleName(service.getName() + ", " + service.getStatus());
    setToolTip(service.getName() + "\n" + service.getDescription());
    m_hoverAnimation->setObjectName("cardHoverAnimation");
    m_hoverAnimation->setDuration(160);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_hoverAnimation,&QVariantAnimation::valueChanged,this,[this](const QVariant &value) {
        m_hoverProgress=value.toReal(); update();
    });
    m_entranceAnimation->setObjectName("cardEntranceMotion");
    m_entranceAnimation->setDuration(270); m_entranceAnimation->setEasingCurve(QEasingCurve::OutCubic);
    m_entranceAnimation->setStartValue(0.0); m_entranceAnimation->setEndValue(1.0);
    connect(m_entranceAnimation,&QVariantAnimation::valueChanged,this,[this](const QVariant &v) { m_entranceProgress=v.toReal(); update(); });
    connect(&Motion::instance(),&Motion::changed,this,[this](bool enabled) {
        if(!enabled) { ++m_showGeneration; m_hoverAnimation->stop(); m_entranceAnimation->stop(); m_hoverProgress=0; m_entranceProgress=1; update(); }
    });
}

bool ServiceCard::event(QEvent *event) {
    const auto type=event->type();
    if(type==QEvent::Hide) {
        ++m_showGeneration; m_hoverAnimation->stop(); m_entranceAnimation->stop(); m_entranceProgress=1; m_hoverProgress=0; m_hovered=false;
    } else if(type==QEvent::Show && Motion::instance().enabled()) {
        const int generation=++m_showGeneration; m_entranceProgress=0;
        QTimer::singleShot(property("entranceDelay").toInt(),this,[this,generation] {
            if(generation==m_showGeneration && isVisible() && Motion::instance().enabled()) m_entranceAnimation->start();
        });
    } else if(Motion::instance().enabled() && (type==QEvent::Enter || type==QEvent::Leave || type==QEvent::FocusIn || type==QEvent::FocusOut)) {
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
    p.setOpacity(.2+.8*m_entranceProgress);
    p.translate(0,8*(1-m_entranceProgress));
    p.setPen(QPen(isChecked() ? QColor("#F5A133") : blend(QColor("#DFE8EB"),QColor("#FF9B28"),m_hoverProgress), isChecked() ? 1.5 : 1));
    p.setBrush(blend(isChecked()?QColor("#FFFAF2"):QColor(Qt::white),QColor("#FFF2DE"),m_hoverProgress));
    p.drawRoundedRect(QRectF(1,1,width()-2,height()-2), 12,12);
    if(m_hoverProgress>0) {
        p.setBrush(Qt::NoBrush);
        for(int width:{7,4,2}) {
            QColor orange("#FF9B28"); orange.setAlphaF((width==2?.9:.08)*m_hoverProgress);
            p.setPen(QPen(orange,width)); p.drawRoundedRect(QRectF(rect()).adjusted(3,3,-3,-3),7,7);
        }
    }
    const QString categoryIcon=m_service.getCategory()=="Soins & bien-être"?"heart":m_service.getCategory()=="Toilettage"?"drop":m_service.getCategory()=="Pension & garde"?"home":m_service.getCategory()=="Éducation"?"star":"paw";
    const QColor iconColor=categoryIcon=="star"?QColor("#B78130"):categoryIcon=="home"?QColor("#7C72A9"):QColor("#40889A");
    const QColor iconBackground=categoryIcon=="star"?QColor("#FFF2DA"):categoryIcon=="home"?QColor("#F0ECF9"):QColor("#EAF3F5");
    p.setPen(Qt::NoPen); p.setBrush(blend(iconBackground,iconColor.lighter(220),m_hoverProgress*.3)); p.drawRoundedRect(QRect(16,14,25,25),8,8);
    const int iconSize=qRound(17+2*m_hoverProgress);
    Design::icon(categoryIcon,iconColor).paint(&p,QRect(20-(iconSize-17)/2,18-(iconSize-17)/2,iconSize,iconSize));
    QString bg="#EAEAF8", fg="#7374A9";
    if (m_service.getStatus()=="En cours") { bg="#FFF0D7"; fg="#AA7830"; }
    if (m_service.getStatus()=="Terminé") { bg="#E5F2E9"; fg="#4D8A6B"; }
    if (m_service.getStatus()=="Annulé") { bg="#FBE9E8"; fg="#B36463"; }
    const QRect badge(width()-108,14,92,21);
    p.setBrush(QColor(bg)); p.drawRoundedRect(badge,12,12);
    QFont f("Segoe UI"); f.setPixelSize(10); f.setWeight(QFont::DemiBold); p.setFont(f); p.setPen(QColor(fg));
    p.drawText(badge,Qt::AlignCenter,"●  "+m_service.getStatus());
    f.setPixelSize(14); f.setWeight(QFont::DemiBold); p.setFont(f); p.setPen(QColor("#315D6C"));
    p.drawText(QRect(17,48,width()-34,22),Qt::AlignVCenter,p.fontMetrics().elidedText(m_service.getName(),Qt::ElideRight,width()-38));
    f.setPixelSize(12); f.setWeight(QFont::Normal); p.setFont(f); p.setPen(QColor("#84979E"));
    p.drawText(QRect(17,74,width()-34,18),m_service.getCategory());
    f.setPixelSize(10); p.setFont(f);
    p.drawText(QRect(17,95,width()-34,16),QString("SRV-%1   ·   %2 min").arg(m_service.getId(),3,10,QChar('0')).arg(m_service.getDuration()));

    Design::icon("calendar",QColor("#8B9EA5")).paint(&p,QRect(17,121,12,12));
    f.setPixelSize(11); p.setFont(f); p.setPen(QColor("#617F8A"));
    p.drawText(QRect(35,117,140,20),Qt::AlignVCenter,QLocale().toString(m_service.getDateTime(),"dd MMM · HH:mm"));
    f.setWeight(QFont::DemiBold); p.setFont(f); p.setPen(QColor("#377D8E"));
    p.drawText(QRect(width()-111,117,94,20),Qt::AlignRight|Qt::AlignVCenter,Design::money(m_service.getPrice()));
}
