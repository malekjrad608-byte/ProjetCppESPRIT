#include "servicecard.h"
#include "../ui/design.h"
#include <QPainter>
#include <QLocale>

ServiceCard::ServiceCard(const Service &service, QWidget *parent)
    : QAbstractButton(parent), m_service(service) {
    setCheckable(true);
    setFixedHeight(184);
    setMinimumWidth(245);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setObjectName("serviceCard_" + QString::number(service.id));
    setAccessibleName(service.name + ", " + service.status);
    setToolTip(service.name + "\n" + service.description);
}

void ServiceCard::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setPen(QPen(isChecked() ? QColor("#E7A342") : hasFocus() ? QColor("#6194A1") : QColor("#DFE8EB"), isChecked() ? 2 : 1));
    p.setBrush(isChecked() ? QColor("#FFFAF2") : underMouse() ? QColor("#F8FCFD") : Qt::white);
    p.drawRoundedRect(QRectF(1,1,width()-2,height()-2), 12,12);
    p.setPen(Qt::NoPen); p.setBrush(QColor("#EAF3F5")); p.drawRoundedRect(QRect(18,17,31,31),8,8);
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
