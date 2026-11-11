#ifndef LOGINVISUALS_H
#define LOGINVISUALS_H

#include <QPushButton>
#include <QVariantAnimation>
#include <QWidget>

void playLogoutTransition(QWidget *target);

class BrandPanel : public QWidget
{
public:
    explicit BrandPanel(QWidget *parent = nullptr);
    void setMotionEnabled(bool enabled);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    QVariantAnimation m_animation;
    qreal m_phase{0.0};
    bool m_motionEnabled{true};
};

class LoginActionButton : public QPushButton
{
public:
    explicit LoginActionButton(const QString &text, QWidget *parent = nullptr);
    void setMotionEnabled(bool enabled);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void animateHighlight(bool highlighted);
    QVariantAnimation m_hoverAnimation;
    qreal m_highlight{0.0};
    bool m_motionEnabled{true};
};

#endif // LOGINVISUALS_H
