#include "design.h"
#include "../widgets/animatedbutton.h"
#include "../widgets/animatedpanel.h"
#include "motion.h"
#include <QApplication>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLocale>

namespace Design {
namespace {
void paw(QPainter &p,QPointF center,qreal size,qreal angle,const QColor &color) {
    p.save(); p.translate(center); p.rotate(angle); p.scale(size/48,size/48);
    p.setPen(Qt::NoPen); p.setBrush(color);
    QPainterPath pad; pad.moveTo(-13,13); pad.cubicTo(-18,5,-7,-7,0,-7);
    pad.cubicTo(7,-7,18,5,13,13); pad.cubicTo(9,18,4,13,0,13); pad.cubicTo(-4,13,-9,18,-13,13);
    p.drawPath(pad); p.drawEllipse(QRectF(-21,-10,9,13)); p.drawEllipse(QRectF(-12,-23,10,14));
    p.drawEllipse(QRectF(2,-23,10,14)); p.drawEllipse(QRectF(12,-10,9,13)); p.restore();
}
}
void paintWorkspace(QPainter &p,const QRect &bounds) {
    p.save(); p.setRenderHint(QPainter::Antialiasing); p.setClipRect(bounds);
    const qreal w=bounds.width(),h=bounds.height();
    QLinearGradient base(0,0,w,h); base.setColorAt(0,QColor("#EDF6F8")); base.setColorAt(.55,QColor("#F6FAFB")); base.setColorAt(1,QColor("#E8F2F4"));
    p.fillRect(bounds,base);
    QRadialGradient light(QPointF(w*.82,30),w*.6); light.setColorAt(0,QColor(107,183,190,24)); light.setColorAt(1,Qt::transparent); p.fillRect(bounds,light);
    p.setPen(Qt::NoPen);
    QPainterPath wave; wave.moveTo(0,h-95); wave.cubicTo(w*.3,h-230,w*.65,h+90,w,h-135); wave.lineTo(w,h); wave.lineTo(0,h); wave.closeSubpath();
    p.fillPath(wave,QColor(77,153,164,13));
    p.setBrush(Qt::NoBrush); p.setPen(QPen(QColor(69,143,156,21),1));
    for(int i=0;i<3;++i) p.drawEllipse(QRectF(w-270-i*38,-185-i*38,370+i*76,370+i*76));
    p.setPen(QPen(QColor(237,165,73,34),1.3));
    QPainterPath line; line.moveTo(w*.27,h); line.cubicTo(w*.42,h-145,w*.65,h-125,w*.82,h); p.drawPath(line);
    p.setPen(Qt::NoPen); p.setBrush(QColor(51,126,141,27));
    for(int x=0;x<7;++x) for(int y=0;y<4;++y) p.drawEllipse(QPointF(w-170+x*17,70+y*17),1.3,1.3);
    paw(p,QPointF(w*.68,h-100),75,-22,QColor(51,126,141,17));
    paw(p,QPointF(w*.76,h-175),44,15,QColor(51,126,141,14));
    p.restore();
}
void paintSidebarArt(QPainter &p,const QRect &bounds) {
    if(bounds.height()<70) return;
    p.save(); p.setRenderHint(QPainter::Antialiasing); p.setClipRect(bounds);
    const qreal x=bounds.x(),y=bounds.y(),w=bounds.width(),h=bounds.height();
    QLinearGradient fade(0,y,0,y+h); fade.setColorAt(0,Qt::transparent); fade.setColorAt(1,QColor(104,175,184,17)); p.fillRect(bounds,fade);
    p.setPen(QPen(QColor(94,157,168,32),1)); p.setBrush(Qt::NoBrush);
    for(int i=0;i<3;++i) p.drawEllipse(QRectF(x-105-i*25,y+h-160-i*25,240+i*50,240+i*50));
    const qreal scale=qMin(1.0,h/240);
    paw(p,QPointF(x+w*.7,y+h*.77),46*scale,-22,QColor(55,133,148,39));
    paw(p,QPointF(x+w*.42,y+h*.52),32*scale,18,QColor(55,133,148,28));
    paw(p,QPointF(x+w*.7,y+h*.27),22*scale,-15,QColor(224,158,66,45));
    p.restore();
}

QLabel *label(const QString &text, const char *role, QWidget *parent) {
    auto *result=new QLabel(text,parent); result->setTextFormat(Qt::PlainText);
    if(role) result->setProperty("role",role);
    return result;
}
QPushButton *button(const QString &text, const QString &iconName, const char *role) {
    QString displayText=text;
    displayText.replace("&","&&");
    auto *result=new AnimatedButton(displayText);
    const bool filled=role && (QString(role)=="primary" || QString(role)=="success" || QString(role)=="danger");
    QColor color=filled?QColor("white"):QColor("#58796A");
    if(role && QString(role)=="nav") color=iconName=="stock"?QColor("#D69335"):iconName=="services"?QColor("#8268AE"):QColor("#5299AB");
    if(!iconName.isEmpty()) result->setIcon(icon(iconName,color));
    result->setIconSize(QSize(17,17)); if(role) result->setProperty("role",role);
    result->setCursor(Qt::PointingHandCursor); return result;
}
QFrame *card(const char *name) { auto *f=new AnimatedPanel; f->setProperty("role","card"); if(name) f->setObjectName(name); return f; }
QIcon icon(const QString &name, const QColor &color) {
    QPixmap pix(48,48); pix.fill(Qt::transparent);
    QPainter p(&pix); p.setRenderHint(QPainter::Antialiasing); p.scale(2,2);
    p.setPen(QPen(color,1.6,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)); p.setBrush(Qt::NoBrush);
    if(name=="grid") { for(int x:{4,14}) for(int y:{4,14}) p.drawRoundedRect(QRectF(x,y,6,6),1,1); }
    else if(name=="stock") {
        p.drawPolygon(QPolygonF{QPointF(3,6),QPointF(12,2),QPointF(21,6),QPointF(21,18),QPointF(12,22),QPointF(3,18)});
        p.drawPolyline(QPolygonF{QPointF(3,6),QPointF(12,10),QPointF(21,6)}); p.drawLine(12,10,12,22);
        p.drawLine(8,4,17,8);
    }
    else if(name=="plus") { p.drawLine(12,5,12,19); p.drawLine(5,12,19,12); }
    else if(name=="search") { p.drawEllipse(QRectF(4,4,11,11)); p.drawLine(14,14,20,20); }
    else if(name=="arrow") { p.drawLine(4,12,20,12); p.drawLine(15,7,20,12); p.drawLine(15,17,20,12); }
    else if(name=="logout") {
        p.drawPolyline(QPolygonF{QPointF(10,3),QPointF(4,3),QPointF(4,21),QPointF(10,21)});
        p.drawLine(9,12,21,12); p.drawLine(17,8,21,12); p.drawLine(21,12,17,16);
    }
    else if(name=="check") { p.drawEllipse(QRectF(3,3,18,18)); p.drawLine(7,12,10,15); p.drawLine(10,15,17,8); }
    else if(name=="clock") { p.drawEllipse(QRectF(3,3,18,18)); p.drawLine(12,7,12,12); p.drawLine(12,12,16,14); }
    else if(name=="calendar") { p.drawRoundedRect(QRectF(3,5,18,16),2,2); p.drawLine(3,10,21,10); p.drawLine(8,3,8,7); p.drawLine(16,3,16,7); p.drawPoint(8,14); p.drawPoint(12,14); p.drawPoint(16,14); }
    else if(name=="users") { p.drawEllipse(QRectF(5,3,7,7)); p.drawArc(QRectF(2,12,14,12),0,180*16); p.drawArc(QRectF(12,5,6,6),-90*16,180*16); p.drawArc(QRectF(13,13,9,9),0,90*16); }
    else if(name=="heart") { QPainterPath a; a.moveTo(12,20); a.cubicTo(9,17,2,12,3,7); a.cubicTo(4,2,10,3,12,7); a.cubicTo(14,3,20,2,21,7); a.cubicTo(22,12,15,17,12,20); p.drawPath(a); }
    else if(name=="paw") { p.drawEllipse(QRectF(7,12,11,8)); p.drawEllipse(QRectF(3,7,4,5)); p.drawEllipse(QRectF(8,2,4,6)); p.drawEllipse(QRectF(14,2,4,6)); p.drawEllipse(QRectF(19,7,3,5)); }
    else if(name=="motion") { p.drawLine(3,6,8,6); p.drawLine(2,12,6,12); p.drawLine(3,18,8,18); p.drawPolygon(QPolygonF{QPointF(10,4),QPointF(21,12),QPointF(10,20)}); }
    else if(name=="drop") { QPainterPath a; a.moveTo(12,2); a.cubicTo(8,8,4,11,4,15); a.cubicTo(4,24,20,24,20,15); a.cubicTo(20,11,16,8,12,2); p.drawPath(a); p.drawArc(QRectF(8,12,8,7),190*16,90*16); }
    else if(name=="home") { p.drawPolyline(QPolygonF{QPointF(2,11),QPointF(12,3),QPointF(22,11)}); p.drawPolyline(QPolygonF{QPointF(5,10),QPointF(5,21),QPointF(19,21),QPointF(19,10)}); p.drawRect(QRectF(10,14,4,7)); }
    else if(name=="star") { p.drawPolygon(QPolygonF{QPointF(12,2),QPointF(15,8),QPointF(22,9),QPointF(17,14),QPointF(18,21),QPointF(12,18),QPointF(6,21),QPointF(7,14),QPointF(2,9),QPointF(9,8)}); }
    else if(name=="edit") { p.drawLine(5,16,16,5); p.drawLine(8,19,19,8); p.drawLine(16,5,19,8); p.drawLine(5,16,4,20); p.drawLine(4,20,8,19); p.drawLine(14,20,21,20); }
    else if(name=="trash") { p.drawLine(4,6,20,6); p.drawLine(9,3,15,3); p.drawLine(7,6,8,21); p.drawLine(8,21,16,21); p.drawLine(16,21,17,6); p.drawLine(10,10,10,17); p.drawLine(14,10,14,17); }
    else if(name=="export") { p.drawLine(12,3,12,15); p.drawLine(8,11,12,15); p.drawLine(12,15,16,11); p.drawPolyline(QPolygonF{QPointF(4,15),QPointF(4,21),QPointF(20,21),QPointF(20,15)}); }
    else if(name=="eye") { QPainterPath a; a.moveTo(2,12); a.quadTo(12,0,22,12); a.quadTo(12,24,2,12); p.drawPath(a); p.drawEllipse(QRectF(9,9,6,6)); }
    else if(name=="shield") { QPainterPath a; a.moveTo(12,2); a.lineTo(21,6); a.cubicTo(21,15,18,19,12,22); a.cubicTo(6,19,3,15,3,6); a.closeSubpath(); p.drawPath(a); p.drawLine(8,12,11,15); p.drawLine(11,15,16,9); }
    else { p.drawRoundedRect(QRectF(4,6,16,15),2,2); p.drawRoundedRect(QRectF(8,3,8,5),1,1); p.drawLine(8,12,16,12); p.drawLine(8,16,13,16); }
    return QIcon(pix);
}
QPixmap brand(const QString &name, const QSize &size) {
    // Original files are preserved; transparent margins are trimmed at display time.
    QImage original(":/brand/"+name+".png"); if(original.isNull()) return {};
    int left=original.width(),top=original.height(),right=-1,bottom=-1;
    for(int y=0;y<original.height();++y) for(int x=0;x<original.width();++x)
        if(qAlpha(original.pixel(x,y))>20) { left=qMin(left,x); top=qMin(top,y); right=qMax(right,x); bottom=qMax(bottom,y); }
    if(right>=left) original=original.copy(QRect(QPoint(left,top),QPoint(right,bottom)));
    return QPixmap::fromImage(original).scaled(size,Qt::KeepAspectRatio,Qt::SmoothTransformation);
}
QWidget *badge(const QString &status) {
    QString bg="#EDF0FB",fg="#697DC3";
    if(status=="En cours") { bg="#FFF1D8"; fg="#B48332"; }
    else if(status=="Terminé") { bg="#E8F3EA"; fg="#468467"; }
    else if(status=="Annulé") { bg="#FAEBE7"; fg="#B56C61"; }
    auto *wrapper=new QWidget; auto *layout=new QHBoxLayout(wrapper); layout->setContentsMargins(8,0,8,0);
    auto *text=label("●  "+status); text->setAlignment(Qt::AlignCenter);
    text->setStyleSheet(QString("color:%1;background:%2;border-radius:10px;padding:5px 9px;font-size:11px;font-weight:600;").arg(fg,bg));
    layout->addWidget(text,0,Qt::AlignVCenter); layout->addStretch(); wrapper->setAttribute(Qt::WA_TransparentForMouseEvents); return wrapper;
}
QString money(double amount) { return QLocale(QLocale::French,QLocale::Tunisia).toString(amount,'f',2)+" DT"; }
QStringList categories() { return {"Soins & bien-être","Toilettage","Pension & garde","Éducation","Autre"}; }
QStringList statuses() { return {"Planifié","En cours","Terminé","Annulé"}; }
void setupTable(QTableWidget *table,const QStringList &headers) {
    table->setColumnCount(headers.size()); table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->hide(); table->setShowGrid(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows); table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers); table->setFocusPolicy(Qt::StrongFocus);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); table->horizontalHeader()->setHighlightSections(false);
    table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); table->setWordWrap(false);
}
void applyTheme() {
    Motion::installHoverEffects();
    qApp->setStyle("Fusion"); QFile file(":/theme.qss");
    if(file.open(QIODevice::ReadOnly)) qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
    QPalette p; p.setColor(QPalette::Window,QColor("#F3F7F8")); p.setColor(QPalette::Base,Qt::white);
    p.setColor(QPalette::Text,QColor("#243D3D")); p.setColor(QPalette::WindowText,QColor("#243D3D"));
    p.setColor(QPalette::Highlight,QColor("#D5E7DC")); p.setColor(QPalette::HighlightedText,QColor("#234D3A")); qApp->setPalette(p);
}
}
