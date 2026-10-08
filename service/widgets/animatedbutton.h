#pragma once
#include <QPushButton>
#include <QVariantAnimation>

class AnimatedButton : public QPushButton {
public:
    explicit AnimatedButton(const QString &text,QWidget *parent=nullptr);
protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    void stopMotion();
    QVariantAnimation *m_hover;
    QVariantAnimation *m_ripple;
    qreal m_highlight=0;
    qreal m_wave=1;
    QPointF m_origin;
};
