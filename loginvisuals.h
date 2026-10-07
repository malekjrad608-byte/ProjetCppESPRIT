#ifndef LOGINVISUALS_H
#define LOGINVISUALS_H

#include <QPixmap>
#include <QPushButton>
#include <QVariantAnimation>
#include <QWidget>

// Transparent logo that shrinks then grows when the mouse hovers over it.
class HoverLogo : public QWidget
{
public:
    explicit HoverLogo(const QString &path, const QSize &boxSize, bool animated = true,
                       QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QPixmap m_pixmap;
    QVariantAnimation m_enter;
    QVariantAnimation m_leave;
    qreal m_scale{1.0};
    bool m_animated{true};
};

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

class LoginAnimatedButton : public QPushButton
{
public:
    explicit LoginAnimatedButton(const QString &text, QWidget *parent = nullptr);
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
