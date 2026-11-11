#pragma once
#include <QAbstractButton>
#include <QMovie>
#include <QPixmap>
#include <QVariantAnimation>

class AnimatedLogo : public QAbstractButton {
    Q_OBJECT
public:
    explicit AnimatedLogo(QWidget *parent = nullptr);
protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *) override;
private:
    void animateTo(bool active);
    QPixmap m_logo;
    QMovie *m_movie;
    QVariantAnimation *m_zoom;
    qreal m_progress = 0;
    bool m_hovered = false;
    bool m_keyboardFocus = false;
};
