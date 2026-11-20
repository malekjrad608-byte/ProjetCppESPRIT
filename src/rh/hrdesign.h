#pragma once
#include <QtWidgets>
#include "../ui/motion.h"

namespace AniRh {


class AnimatedButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(qreal glow READ glow WRITE setGlow)
public:
    explicit AnimatedButton(const QString &text, QWidget *parent=nullptr);
    qreal glow() const { return m_glow; }
    void setGlow(qreal value) { m_glow=value; update(); }
protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    qreal m_glow=0;
    QPropertyAnimation *m_animation;
};


namespace Ui {
QLabel *label(const QString &text,const char *role=nullptr);
AnimatedButton *button(const QString &text,const QString &icon={},const char *role=nullptr);
QFrame *card(const char *name=nullptr);
QIcon icon(const QString &name,const QColor &color=QColor("#4E777F"));
QWidget *badge(const QString &text);
QWidget *person(const QString &name,const QString &detail,int seed=0);
QString money(qint64 millimes);
QString date(const QString &iso);
QString normalized(const QString &text);
void setupTable(QTableWidget *table,const QStringList &headers);
void applyTheme(QWidget *widget);
bool exportCsv(const QString &path,const QStringList &headers,const QList<QStringList> &rows,QString *error=nullptr);
}

} // namespace AniRh
