#pragma once
#include <QFrame>
#include <QVariantAnimation>

// Paint-only emphasis: panel content and layout never move or turn transparent.
class AnimatedPanel : public QFrame {
public:
    explicit AnimatedPanel(QWidget *parent=nullptr);
protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    QVariantAnimation *m_hover;
    qreal m_progress=0;
};
