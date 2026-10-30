#include "loginvisuals.h"

#include <QEnterEvent>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QtMath>

namespace {
constexpr qreal FullCircle = 6.28318530718;

class LogoArtwork : public QWidget
{
public:
    explicit LogoArtwork(QWidget *parent = nullptr)
        : QWidget(parent), m_logo(":/images/logo.png")
    {
        setFixedWidth(286);
        setMinimumHeight(180);
        setMaximumHeight(256);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        setAccessibleName("ANIWORLD logo");
    }

    void setPhase(qreal phase)
    {
        m_phase = phase;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        const QPointF center(width() / 2.0, height() / 2.0);
        p.translate(center);
        const qreal scale = qMin(width() / 286.0, height() / 256.0);
        p.scale(scale, scale);

        // The outer rings move; the original logo stays still and readable.
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(255, 255, 255, 24), 1));
        p.drawEllipse(QPointF(0, 0), 123, 123);
        p.setPen(QPen(QColor(255, 255, 255, 14), 1));
        p.drawEllipse(QPointF(0, 0), 112, 112);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(9, 31, 37, 35));
        p.drawEllipse(QPointF(0, 8), 102, 102);
        p.setBrush(Qt::white);
        p.drawEllipse(QPointF(0, 0), 101, 101);

        // Fit the complete supplied artwork inside the badge without cropping it.
        p.save();
        QPainterPath badge;
        badge.addEllipse(QPointF(0, 0), 101, 101);
        p.setClipPath(badge);
        QSizeF logoSize = m_logo.size();
        logoSize.scale(190, 190, Qt::KeepAspectRatio);
        p.drawPixmap(QRectF(-logoSize.width() / 2, -logoSize.height() / 2,
                           logoSize.width(), logoSize.height()), m_logo, m_logo.rect());
        p.restore();

        const qreal angle = m_phase - 0.9;
        const QPointF dot(qCos(angle) * 123, qSin(angle) * 123);
        p.setBrush(QColor("#EBA43B"));
        p.drawEllipse(dot, 5.5, 5.5);
        p.setBrush(QColor("#9DCBC8"));
        p.drawEllipse(QPointF(qCos(angle + 3.1) * 112, qSin(angle + 3.1) * 112), 3.5, 3.5);

        p.translate(85, 74 + qSin(m_phase) * 4);
        p.setBrush(QColor("#EBA43B"));
        p.drawRoundedRect(QRectF(-24, -24, 48, 48), 16, 16);
        p.setPen(QPen(QColor("#263E43"), 3, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(-8, 0), QPointF(8, 0));
        p.drawLine(QPointF(0, -8), QPointF(0, 8));
    }

private:
    QPixmap m_logo;
    qreal m_phase{0.0};
};

QLabel *brandLabel(const QString &text, const char *name)
{
    auto *label = new QLabel(text);
    label->setObjectName(name);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    return label;
}

void drawPaw(QPainter &p, const QPointF &position, qreal angle)
{
    p.save();
    p.translate(position);
    p.rotate(angle);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(185, 225, 222, 20));
    p.drawEllipse(QRectF(-9, -1, 18, 15));
    p.drawEllipse(QRectF(-15, -11, 7, 10));
    p.drawEllipse(QRectF(-6, -17, 7, 11));
    p.drawEllipse(QRectF(3, -16, 7, 11));
    p.drawEllipse(QRectF(11, -9, 7, 10));
    p.restore();
}
}

BrandPanel::BrandPanel(QWidget *parent) : QWidget(parent)
{
    setObjectName("brandPanel");
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 34, 30, 30);
    layout->setSpacing(0);

    layout->addWidget(brandLabel("ANIWORLD", "brandName"));
    layout->addSpacing(6);
    layout->addWidget(brandLabel("S T O C K   M A N A G E M E N T", "brandTag"));
    layout->addStretch(2);

    auto *artwork = new LogoArtwork;
    layout->addWidget(artwork, 0, Qt::AlignHCenter);
    layout->addSpacing(18);
    layout->addWidget(brandLabel("Know your stock.\nRun it smarter.", "brandHeadline"));
    layout->addSpacing(16);
    layout->addWidget(brandLabel("Inventory, traceability and\nreplenishment in one place.", "brandDesc"));
    layout->addStretch(2);
    layout->addSpacing(24);

    auto *footer = brandLabel("Inventory  /  Lots  /  Replenishment", "brandFooter");
    layout->addWidget(footer);

    m_animation.setStartValue(0.0);
    m_animation.setEndValue(FullCircle);
    m_animation.setDuration(28000);
    m_animation.setLoopCount(-1);
    connect(&m_animation, &QVariantAnimation::valueChanged, this,
            [this, artwork](const QVariant &value) {
        m_phase = value.toReal();
        if (!window()->isMinimized()) {
            artwork->setPhase(m_phase);
            update();
        }
    });
}

void BrandPanel::setMotionEnabled(bool enabled)
{
    m_motionEnabled = enabled;
    if (enabled && isVisible()) {
        if (m_animation.state() == QAbstractAnimation::Paused)
            m_animation.resume();
        else if (m_animation.state() == QAbstractAnimation::Stopped)
            m_animation.start();
    } else if (m_animation.state() == QAbstractAnimation::Running) {
        m_animation.pause();
    }
}

void BrandPanel::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setMotionEnabled(m_motionEnabled);
}

void BrandPanel::hideEvent(QHideEvent *event)
{
    if (m_animation.state() == QAbstractAnimation::Running)
        m_animation.pause();
    QWidget::hideEvent(event);
}

void BrandPanel::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath clip;
    clip.setFillRule(Qt::WindingFill);
    clip.addRoundedRect(QRectF(rect()), 24, 24);
    clip.addRect(QRectF(width() - 24, 0, 24, height()));
    p.setClipPath(clip.simplified());

    QLinearGradient background(0, 0, width(), height());
    background.setColorAt(0, QColor("#243C43"));
    background.setColorAt(1, QColor("#387D85"));
    p.fillRect(rect(), background);

    const QPointF glowCenter(width() * 0.3, height() * 0.42);
    QRadialGradient glow(glowCenter, width() * 0.85);
    glow.setColorAt(0, QColor(113, 191, 184, 34));
    glow.setColorAt(1, QColor(113, 191, 184, 0));
    p.fillRect(rect(), glow);

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(190, 231, 225, 16), 1));
    for (int i = 0; i < 3; ++i) {
        const qreal radius = 170 + i * 25;
        p.drawEllipse(QPointF(width() + 50, -35), radius, radius);
        p.drawEllipse(QPointF(-80, height() + 45), radius + 20, radius + 20);
    }
    drawPaw(p, QPointF(48, height() * 0.31 + qSin(m_phase) * 7), -25);
    drawPaw(p, QPointF(width() - 43, height() * 0.65 + qCos(m_phase) * 6), 23);
}

AnimatedButton::AnimatedButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    setObjectName("primaryBtn");
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(52);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_hoverAnimation.setDuration(180);
    m_hoverAnimation.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_hoverAnimation, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &value) { m_highlight = value.toReal(); update(); });
}

void AnimatedButton::setMotionEnabled(bool enabled)
{
    m_motionEnabled = enabled;
    if (!enabled) {
        m_hoverAnimation.stop();
        m_highlight = underMouse() || hasFocus() ? 1.0 : 0.0;
        update();
    }
}

void AnimatedButton::animateHighlight(bool highlighted)
{
    m_hoverAnimation.stop();
    if (!m_motionEnabled) {
        m_highlight = highlighted ? 1.0 : 0.0;
        update();
        return;
    }
    m_hoverAnimation.setStartValue(m_highlight);
    m_hoverAnimation.setEndValue(highlighted ? 1.0 : 0.0);
    m_hoverAnimation.start();
}

void AnimatedButton::enterEvent(QEnterEvent *event)
{
    animateHighlight(true);
    QPushButton::enterEvent(event);
}

void AnimatedButton::leaveEvent(QEvent *event)
{
    animateHighlight(hasFocus());
    QPushButton::leaveEvent(event);
}

void AnimatedButton::focusInEvent(QFocusEvent *event)
{
    animateHighlight(true);
    QPushButton::focusInEvent(event);
}

void AnimatedButton::focusOutEvent(QFocusEvent *event)
{
    animateHighlight(underMouse());
    QPushButton::focusOutEvent(event);
}

void AnimatedButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF buttonRect = QRectF(rect()).adjusted(1, 1, -1, -1);
    QLinearGradient fill(buttonRect.topLeft(), buttonRect.topRight());
    QColor start("#397E89");
    QColor end("#2A626C");
    start = start.lighter(100 + qRound(m_highlight * 12));
    end = end.lighter(100 + qRound(m_highlight * 12));
    if (isDown()) { start = QColor("#234D55"); end = start; }
    if (!isEnabled()) { start = QColor("#B7CACB"); end = start; }
    fill.setColorAt(0, start);
    fill.setColorAt(1, end);
    p.setPen(Qt::NoPen);
    p.setBrush(fill);
    p.drawRoundedRect(buttonRect, 12, 12);
    if (hasFocus()) {
        p.setPen(QPen(QColor("#EBA43B"), 2));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(buttonRect.adjusted(3, 3, -3, -3), 9, 9);
    }
    QFont buttonFont = font();
    buttonFont.setPixelSize(14);
    buttonFont.setWeight(QFont::DemiBold);
    p.setFont(buttonFont);
    p.setPen(Qt::white);
    p.drawText(buttonRect.adjusted(24, 0, -40, 0), Qt::AlignCenter, text());

    const qreal x = width() - 29 + m_highlight * 3;
    const qreal y = height() / 2.0;
    p.setPen(QPen(Qt::white, 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(QPointF(x - 8, y), QPointF(x + 3, y));
    p.drawLine(QPointF(x - 1, y - 4), QPointF(x + 3, y));
    p.drawLine(QPointF(x - 1, y + 4), QPointF(x + 3, y));
}
