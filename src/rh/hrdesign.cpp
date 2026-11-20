#include "hrdesign.h"
#include <QSaveFile>
#include <cmath>

namespace AniRh {

AnimatedButton::AnimatedButton(const QString &text,QWidget *parent):QPushButton(text,parent),m_animation(new QPropertyAnimation(this,"glow",this)) {
    setCursor(Qt::PointingHandCursor); m_animation->setDuration(170); m_animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(&Motion::instance(),&Motion::changed,this,[this](bool enabled){ if(!enabled) { m_animation->stop(); setGlow(0); } });
}
void AnimatedButton::enterEvent(QEnterEvent *e) { QPushButton::enterEvent(e); if(Motion::instance().enabled()) { m_animation->stop(); m_animation->setEndValue(1.0); m_animation->start(); } }
void AnimatedButton::leaveEvent(QEvent *e) { QPushButton::leaveEvent(e); m_animation->stop(); m_animation->setEndValue(0.0); m_animation->start(); }
void AnimatedButton::paintEvent(QPaintEvent *e) {
    QPushButton::paintEvent(e); if(!isEnabled() || m_glow<=0) return;
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing); QColor c("#EAA438"); c.setAlphaF(m_glow*.6);
    p.setPen(QPen(c,1.3)); p.setBrush(Qt::NoBrush); p.drawRoundedRect(QRectF(rect()).adjusted(1,1,-1,-1),8,8);
}
namespace Ui {
QLabel *label(const QString &text,const char *role) { auto *w=new QLabel(text); w->setTextFormat(Qt::PlainText); if(role) w->setProperty("role",role); return w; }
AnimatedButton *button(const QString &text,const QString &name,const char *role) {
    auto *w=new AnimatedButton(QString(text).replace("&","&&")); if(role) w->setProperty("role",role);
    if(!name.isEmpty()) w->setIcon(icon(name,QString(role?role:"")=="primary"?QColor("white"):QColor("#4E777F")));
    w->setIconSize(QSize(18,18)); w->setMinimumHeight(38); return w;
}
QFrame *card(const char *name) { auto *w=new QFrame; w->setProperty("role","card"); if(name) w->setObjectName(name); return w; }
QIcon icon(const QString &name,const QColor &color) {
    QPixmap pix(48,48); pix.fill(Qt::transparent); QPainter p(&pix); p.setRenderHint(QPainter::Antialiasing); p.scale(2,2);
    p.setPen(QPen(color,1.65,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)); p.setBrush(Qt::NoBrush);
    if(name=="grid") { for(int x:{3,14}) for(int y:{3,14}) p.drawRoundedRect(QRectF(x,y,7,7),1.5,1.5); }
    else if(name=="plus") { p.drawLine(12,5,12,19); p.drawLine(5,12,19,12); }
    else if(name=="search") { p.drawEllipse(QRectF(3,3,12,12)); p.drawLine(14,14,21,21); }
    else if(name=="arrow") { p.drawLine(4,12,20,12); p.drawLine(15,7,20,12); p.drawLine(15,17,20,12); }
    else if(name=="clock") { p.drawEllipse(QRectF(3,3,18,18)); p.drawLine(12,7,12,12); p.drawLine(12,12,16,14); }
    else if(name=="calendar") { p.drawRoundedRect(QRectF(3,5,18,16),2,2); p.drawLine(3,10,21,10); p.drawLine(8,3,8,7); p.drawLine(16,3,16,7); p.drawPoint(8,14); p.drawPoint(12,14); p.drawPoint(16,14); }
    else if(name=="users") { p.drawEllipse(QRectF(5,3,7,7)); p.drawArc(QRectF(2,12,14,13),0,180*16); p.drawArc(QRectF(13,4,6,7),-90*16,180*16); p.drawArc(QRectF(14,13,9,10),0,90*16); }
    else if(name=="star") { p.drawPolygon(QPolygonF{QPointF(12,2),QPointF(15,8),QPointF(22,9),QPointF(17,14),QPointF(18,21),QPointF(12,18),QPointF(6,21),QPointF(7,14),QPointF(2,9),QPointF(9,8)}); }
    else if(name=="money") { p.drawRoundedRect(QRectF(2,5,20,15),2,2); p.drawEllipse(QRectF(8,8,8,8)); p.drawLine(4,9,5,9); p.drawLine(19,16,20,16); }
    else if(name=="check") { p.drawEllipse(QRectF(3,3,18,18)); p.drawPolyline(QPolygonF{QPointF(7,12),QPointF(10,15),QPointF(17,8)}); }
    else if(name=="edit") { p.drawPolygon(QPolygonF{QPointF(5,16),QPointF(16,5),QPointF(19,8),QPointF(8,19),QPointF(4,20)}); p.drawLine(14,20,21,20); }
    else if(name=="export") { p.drawLine(12,3,12,15); p.drawLine(8,11,12,15); p.drawLine(12,15,16,11); p.drawPolyline(QPolygonF{QPointF(4,15),QPointF(4,21),QPointF(20,21),QPointF(20,15)}); }
    else if(name=="motion") { p.drawLine(2,6,7,6); p.drawLine(2,12,6,12); p.drawLine(2,18,7,18); p.drawPolygon(QPolygonF{QPointF(10,4),QPointF(21,12),QPointF(10,20)}); }
    else if(name=="archive") { p.drawRect(QRectF(3,4,18,5)); p.drawPolyline(QPolygonF{QPointF(5,9),QPointF(5,21),QPointF(19,21),QPointF(19,9)}); p.drawLine(9,13,15,13); }
    else { p.drawRoundedRect(QRectF(4,5,16,17),2,2); p.drawRoundedRect(QRectF(8,2,8,5),1,1); p.drawLine(8,12,16,12); p.drawLine(8,16,14,16); }
    return QIcon(pix);
}
QWidget *badge(const QString &text) {
    QString bg="#EAF1F4",fg="#517682";
    if(QStringList{"Actif","Présent","Approuvé","Terminé","Payé"}.contains(text)) { bg="#EAF5EF"; fg="#3B8268"; }
    if(QStringList{"En attente","Retard","En cours","Haute","Brouillon"}.contains(text)) { bg="#FFF4DF"; fg="#A47727"; }
    if(QStringList{"Absent","Refusé","En retard"}.contains(text)) { bg="#FCEDEC"; fg="#B16461"; }
    if(text=="Archivé") { bg="#F0F1F3"; fg="#8A9196"; }
    auto *wrap=new QWidget; auto *lay=new QHBoxLayout(wrap); lay->setContentsMargins(10,0,4,0); lay->setSpacing(0);
    auto *l=label("●  "+text); l->setStyleSheet(QString("background:%1;color:%2;border-radius:9px;padding:5px 9px;font-size:11px;font-weight:600;").arg(bg,fg));
    lay->addWidget(l,0,Qt::AlignVCenter); lay->addStretch(); wrap->setAttribute(Qt::WA_TransparentForMouseEvents); return wrap;
}
QWidget *person(const QString &name,const QString &detail,int seed) {
    auto *w=new QWidget; auto *l=new QHBoxLayout(w); l->setContentsMargins(10,6,10,6); l->setSpacing(10);
    QStringList words=name.split(' ',Qt::SkipEmptyParts); QString initials; if(!words.isEmpty()) initials=words.first().left(1)+words.last().left(1);
    auto *avatar=label(initials.toUpper()); avatar->setAlignment(Qt::AlignCenter); avatar->setFixedSize(34,34);
    const QStringList colors={"#E3F0F1","#F9EFDB","#EBE8F5","#E6F0E8"};
    avatar->setStyleSheet("background:"+colors[qAbs(seed)%4]+";border-radius:11px;color:#43616B;font-size:11px;font-weight:700;"); l->addWidget(avatar);
    auto *text=new QVBoxLayout; text->setSpacing(2); auto *n=label(name); n->setStyleSheet("font-weight:600;font-size:12px;");
    n->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred); n->setToolTip(name);
    auto *d=label(detail,"muted"); d->setStyleSheet("font-size:10px;color:#8B9BA2;"); d->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred); d->setToolTip(detail);
    text->addWidget(n); text->addWidget(d); l->addLayout(text,1); w->setAttribute(Qt::WA_TransparentForMouseEvents); return w;
}
QString money(qint64 millimes) { return QLocale(QLocale::French,QLocale::Tunisia).toString(double(millimes)/1000,'f',3)+" DT"; }
QString date(const QString &iso) { const auto d=QDate::fromString(iso,Qt::ISODate); return d.isValid()?d.toString("dd/MM/yyyy"):"—"; }
QString normalized(const QString &text) { QString r; for(const auto ch:text.normalized(QString::NormalizationForm_D).toLower()) if(ch.category()!=QChar::Mark_NonSpacing) r+=ch; return r; }
void setupTable(QTableWidget *t,const QStringList &headers) {
    t->setColumnCount(headers.size()); t->setHorizontalHeaderLabels(headers); t->verticalHeader()->hide(); t->setShowGrid(false);
    t->setSelectionBehavior(QAbstractItemView::SelectRows); t->setSelectionMode(QAbstractItemView::SingleSelection); t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); t->horizontalHeader()->setMinimumSectionSize(70);
    t->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter); t->horizontalHeader()->setHighlightSections(false);
    t->verticalHeader()->setDefaultSectionSize(62); t->setWordWrap(false); t->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    t->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel); t->setAlternatingRowColors(false);
}
bool exportCsv(const QString &path,const QStringList &headers,const QList<QStringList> &rows,QString *error) {
    QSaveFile file(path); if(!file.open(QIODevice::WriteOnly)) { if(error) *error=file.errorString(); return false; }
    QByteArray output("\xEF\xBB\xBF");
    auto append=[&output](QStringList values) {
        for(auto &value:values) {
            if(!value.isEmpty() && QString("=+-@\t\r").contains(value.front())) value.prepend('\'');
            value.replace("\"","\"\""); value="\""+value+"\"";
        }
        output+=values.join(';').toUtf8()+"\r\n";
    };
    append(headers); for(const auto &r:rows) append(r);
    if(file.write(output)!=output.size() || !file.commit()) { if(error) *error=file.errorString(); return false; } return true;
}
void applyTheme(QWidget *widget) {
    QPalette colors=widget->palette();
    colors.setColor(QPalette::Window,QColor("#F3F7F8"));
    colors.setColor(QPalette::Base,Qt::white);
    widget->setPalette(colors); widget->setAutoFillBackground(true);
    QFile file(":/rh/theme.qss");
    if(file.open(QIODevice::ReadOnly)) widget->setStyleSheet(QString::fromUtf8(file.readAll()));
}
}

} // namespace AniRh
