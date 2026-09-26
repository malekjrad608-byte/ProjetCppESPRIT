#include "design.h"
#include <QApplication>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLocale>

namespace Design {
QLabel *label(const QString &text, const char *role, QWidget *parent) {
    auto *result=new QLabel(text,parent); result->setTextFormat(Qt::PlainText);
    if(role) result->setProperty("role",role);
    return result;
}
QPushButton *button(const QString &text, const QString &iconName, const char *role) {
    QString displayText=text;
    displayText.replace("&","&&");
    auto *result=new QPushButton(displayText);
    if(!iconName.isEmpty()) result->setIcon(icon(iconName,role && QString(role)=="primary"?QColor("white"):QColor("#58796A")));
    result->setIconSize(QSize(17,17)); if(role) result->setProperty("role",role);
    result->setCursor(Qt::PointingHandCursor); return result;
}
QFrame *card(const char *name) { auto *f=new QFrame; f->setProperty("role","card"); if(name) f->setObjectName(name); return f; }
QIcon icon(const QString &name, const QColor &color) {
    QPixmap pix(48,48); pix.fill(Qt::transparent);
    QPainter p(&pix); p.setRenderHint(QPainter::Antialiasing); p.scale(2,2);
    p.setPen(QPen(color,1.6,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)); p.setBrush(Qt::NoBrush);
    if(name=="grid") { for(int x:{4,14}) for(int y:{4,14}) p.drawRoundedRect(QRectF(x,y,6,6),1,1); }
    else if(name=="plus") { p.drawLine(12,5,12,19); p.drawLine(5,12,19,12); }
    else if(name=="search") { p.drawEllipse(QRectF(4,4,11,11)); p.drawLine(14,14,20,20); }
    else if(name=="arrow") { p.drawLine(4,12,20,12); p.drawLine(15,7,20,12); p.drawLine(15,17,20,12); }
    else if(name=="check") { p.drawEllipse(QRectF(3,3,18,18)); p.drawLine(7,12,10,15); p.drawLine(10,15,17,8); }
    else if(name=="clock") { p.drawEllipse(QRectF(3,3,18,18)); p.drawLine(12,7,12,12); p.drawLine(12,12,16,14); }
    else if(name=="calendar") { p.drawRoundedRect(QRectF(3,5,18,16),2,2); p.drawLine(3,10,21,10); p.drawLine(8,3,8,7); p.drawLine(16,3,16,7); p.drawPoint(8,14); p.drawPoint(12,14); p.drawPoint(16,14); }
    else if(name=="users") { p.drawEllipse(QRectF(5,3,7,7)); p.drawArc(QRectF(2,12,14,12),0,180*16); p.drawArc(QRectF(12,5,6,6),-90*16,180*16); p.drawArc(QRectF(13,13,9,9),0,90*16); }
    else if(name=="heart") { QPainterPath a; a.moveTo(12,20); a.cubicTo(9,17,2,12,3,7); a.cubicTo(4,2,10,3,12,7); a.cubicTo(14,3,20,2,21,7); a.cubicTo(22,12,15,17,12,20); p.drawPath(a); }
    else if(name=="paw") { p.drawEllipse(QRectF(7,12,11,8)); p.drawEllipse(QRectF(3,7,4,5)); p.drawEllipse(QRectF(8,2,4,6)); p.drawEllipse(QRectF(14,2,4,6)); p.drawEllipse(QRectF(19,7,3,5)); }
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
    qApp->setStyle("Fusion"); QFile file(":/theme.qss");
    if(file.open(QIODevice::ReadOnly)) qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
    QPalette p; p.setColor(QPalette::Window,QColor("#F5F6F2")); p.setColor(QPalette::Base,Qt::white);
    p.setColor(QPalette::Text,QColor("#243D3D")); p.setColor(QPalette::WindowText,QColor("#243D3D"));
    p.setColor(QPalette::Highlight,QColor("#D5E7DC")); p.setColor(QPalette::HighlightedText,QColor("#234D3A")); qApp->setPalette(p);
}
}
