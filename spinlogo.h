#pragma once
#include <QPixmap>
#include <QWidget>

class QPropertyAnimation;

// Logo qui fait un tour complet quand on le survole
class SpinLogo : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal angle READ angle WRITE setAngle)
public:
    SpinLogo(const QPixmap &pm, int size, QWidget *parent = nullptr);
    qreal angle() const { return m_angle; }
    void setAngle(qreal a);

protected:
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *e) override;

private:
    QPixmap m_pm;
    qreal m_angle = 0.0;
    QPropertyAnimation *m_anim;
};
