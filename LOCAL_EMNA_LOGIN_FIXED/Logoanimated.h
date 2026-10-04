#ifndef LOGOANIMATED_H
#define LOGOANIMATED_H

#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>
#include <QEnterEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QtMath>

// ============================================================
// LogoAnime : logo avec anneaux orbitaux, flottement doux et
// effet de survol (agrandissement + ombre), dans l'esprit de
// l'écran de connexion. 100 % en en-tête : rien à ajouter au .pro
// ============================================================
class LogoAnime : public QWidget
{
public:
    explicit LogoAnime(const QString &chemin, int taille = 54, QWidget *parent = nullptr)
        : QWidget(parent), m_logo(chemin), m_taille(taille)
    {
        setFixedSize(taille + 20, taille + 20);
        setCursor(Qt::PointingHandCursor);
        setAccessibleName("ANIWORLD logo");

        m_orbite.setStartValue(0.0);
        m_orbite.setEndValue(6.28318530718);
        m_orbite.setDuration(9000);
        m_orbite.setLoopCount(-1);
        connect(&m_orbite, &QVariantAnimation::valueChanged, this,
                [this](const QVariant &v) { m_phase = v.toReal(); update(); });

        m_survol.setDuration(240);
        m_survol.setEasingCurve(QEasingCurve::OutCubic);
        connect(&m_survol, &QVariantAnimation::valueChanged, this,
                [this](const QVariant &v) { m_hover = v.toReal(); update(); });
    }

protected:
    void showEvent(QShowEvent *e) override
    {
        QWidget::showEvent(e);
        if (m_orbite.state() != QAbstractAnimation::Running) m_orbite.start();
    }
    void hideEvent(QHideEvent *e) override
    {
        m_orbite.pause();
        QWidget::hideEvent(e);
    }
    void enterEvent(QEnterEvent *e) override { animer(1.0); QWidget::enterEvent(e); }
    void leaveEvent(QEvent *e) override { animer(0.0); QWidget::leaveEvent(e); }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        const qreal flotte = qSin(m_phase * 2.0) * 1.6;           // petit flottement vertical
        const QPointF c(width() / 2.0, height() / 2.0 + flotte);
        const qreal r = (m_taille / 2.0) * (1.0 + 0.06 * m_hover); // grossit au survol

        // --- Ombre douce (plus marquée et plus large au survol) ---
        p.setPen(Qt::NoPen);
        const int couches = 6;
        for (int i = couches; i > 0; --i) {
            const qreal s = i * (0.8 + 0.5 * m_hover);
            p.setBrush(QColor(20, 60, 70, int(5 + 5 * m_hover)));
            p.drawEllipse(QPointF(c.x(), c.y() + 3 + 2 * m_hover), r + s, r + s);
        }

        // --- Anneaux ---
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(57, 126, 137, int(45 + 90 * m_hover)), 1.2));
        p.drawEllipse(c, r + 5, r + 5);

        // --- Badge blanc + logo (sans rognage) ---
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawEllipse(c, r, r);

        if (!m_logo.isNull()) {
            p.save();
            QPainterPath badge;
            badge.addEllipse(c, r, r);
            p.setClipPath(badge);
            QSizeF taille = m_logo.size();
            taille.scale(r * 1.85, r * 1.85, Qt::KeepAspectRatio);
            p.drawPixmap(QRectF(c.x() - taille.width() / 2, c.y() - taille.height() / 2,
                                taille.width(), taille.height()),
                         m_logo, m_logo.rect());
            p.restore();
        }

        // --- Points en orbite ---
        const qreal a = m_phase - 0.9;
       p.setBrush(QColor(235, 164, 59));
        p.drawEllipse(QPointF(c.x() + qCos(a) * (r + 5), c.y() + qSin(a) * (r + 5)), 3.2, 3.2);
        p.setBrush(QColor(157, 203, 200));
        p.drawEllipse(QPointF(c.x() + qCos(a + 3.1) * (r + 5), c.y() + qSin(a + 3.1) * (r + 5)), 2.2, 2.2);
    }

private:
    void animer(qreal cible)
    {
        m_survol.stop();
        m_survol.setStartValue(m_hover);
        m_survol.setEndValue(cible);
        m_survol.start();
    }

    QPixmap m_logo;
    int m_taille;
    qreal m_phase{0.0};
    qreal m_hover{0.0};
    QVariantAnimation m_orbite;
    QVariantAnimation m_survol;
};

#endif // LOGOANIMATED_H
