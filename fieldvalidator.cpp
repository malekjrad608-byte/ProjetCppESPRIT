#include "fieldvalidator.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVariantAnimation>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>
#include <utility>

namespace {
// Couleurs ANIWORLD : bordure neutre du thème (#D7E9ED), vert et rouge accordés à la palette turquoise.
const QColor kNeutral(0xD7, 0xE9, 0xED);
const QColor kValid(0x3F, 0xA6, 0x6B);
const QColor kInvalid(0xD0, 0x5A, 0x55);
const int kColorMs = 260;
const int kFadeMs = 220;
}

ValidatedField::ValidatedField(QWidget *field, Validator validator, bool required, QWidget *parent)
    : QWidget(parent), m_field(field), m_validator(std::move(validator)), m_required(required),
      m_currentBorder(kNeutral)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(2);

    auto *row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(6);
    row->addWidget(m_field, 1);

    m_icon = new QLabel(this);
    m_icon->setFixedWidth(20);
    m_icon->setAlignment(Qt::AlignCenter);
    m_iconFade = new QGraphicsOpacityEffect(m_icon);
    m_iconFade->setOpacity(0.0);
    m_icon->setGraphicsEffect(m_iconFade);
    row->addWidget(m_icon, 0, Qt::AlignVCenter);
    outer->addLayout(row);

    m_msg = new QLabel(this);
    m_msg->setWordWrap(true);
    m_msg->setStyleSheet(QString("color: %1; font-size: 11px; padding-left: 4px; background: transparent;")
                             .arg(kInvalid.name()));
    m_msg->setMaximumHeight(0);
    m_msg->hide();
    m_msgFade = new QGraphicsOpacityEffect(m_msg);
    m_msgFade->setOpacity(0.0);
    m_msg->setGraphicsEffect(m_msgFade);
    outer->addWidget(m_msg);

    m_colorAnim = new QVariantAnimation(this);
    m_colorAnim->setDuration(kColorMs);
    m_colorAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_colorAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
        applyBorder(v.value<QColor>());
    });
    connect(m_colorAnim, &QVariantAnimation::finished, this, [this]() {
        if (m_state == Neutral && m_hasOwnBorder) {     // retour au style normal du thème (focus, survol...)
            m_field->setStyleSheet(QString());
            m_hasOwnBorder = false;
        }
    });

    m_iconAnim = new QPropertyAnimation(m_iconFade, "opacity", this);
    m_iconAnim->setDuration(kFadeMs);
    m_iconAnim->setEasingCurve(QEasingCurve::OutCubic);

    m_msgHeightAnim = new QPropertyAnimation(m_msg, "maximumHeight", this);
    m_msgHeightAnim->setDuration(kFadeMs);
    m_msgHeightAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_msgHeightAnim, &QPropertyAnimation::finished, this, [this]() {
        if (m_msg->maximumHeight() == 0)
            m_msg->hide();
    });

    m_msgFadeAnim = new QPropertyAnimation(m_msgFade, "opacity", this);
    m_msgFadeAnim->setDuration(kFadeMs);
}

QString ValidatedField::typeSelector() const
{
    // Sélecteur de type : n'atteint pas les enfants (liste déroulante, calendrier, champ interne du combo).
    if (qobject_cast<QComboBox *>(m_field))        return "QComboBox";
    if (qobject_cast<QTextEdit *>(m_field))        return "QTextEdit";
    if (qobject_cast<QDateTimeEdit *>(m_field))    return "QDateTimeEdit";   // inclut QDateEdit
    if (qobject_cast<QDoubleSpinBox *>(m_field))   return "QDoubleSpinBox";
    if (qobject_cast<QSpinBox *>(m_field))         return "QSpinBox";
    return "QLineEdit";
}

void ValidatedField::applyBorder(const QColor &c)
{
    m_currentBorder = c;
    m_hasOwnBorder = true;
    m_field->setStyleSheet(QString("%1 { border: 2px solid %2; }").arg(typeSelector(), c.name()));
}

void ValidatedField::animateBorder(const QColor &to)
{
    m_colorAnim->stop();
    m_colorAnim->setStartValue(m_currentBorder);
    m_colorAnim->setEndValue(to);
    m_colorAnim->start();
}

void ValidatedField::showMessage(const QString &text, bool animate)
{
    m_msgHeightAnim->stop();
    m_msgFadeAnim->stop();
    if (text.isEmpty()) {
        if (!m_msg->isVisible()) {
            m_msg->setMaximumHeight(0);
            return;
        }
        if (!animate) {
            m_msg->setMaximumHeight(0);
            m_msg->hide();
            m_msgFade->setOpacity(0.0);
            return;
        }
        m_msgHeightAnim->setStartValue(m_msg->maximumHeight());
        m_msgHeightAnim->setEndValue(0);
        m_msgFadeAnim->setStartValue(m_msgFade->opacity());
        m_msgFadeAnim->setEndValue(0.0);
        m_msgHeightAnim->start();
        m_msgFadeAnim->start();
        return;
    }

    m_msg->setText(text);
    const int w = qMax(120, m_msg->width() > 0 ? m_msg->width() : width());
    const int target = qMax(16, m_msg->heightForWidth(w)) + 2;
    const bool wasHidden = !m_msg->isVisible();
    m_msg->show();
    if (!animate) {
        m_msg->setMaximumHeight(target);
        m_msgFade->setOpacity(1.0);
        return;
    }
    if (wasHidden)
        m_msg->setMaximumHeight(0);
    m_msgHeightAnim->setStartValue(m_msg->maximumHeight());
    m_msgHeightAnim->setEndValue(target);
    m_msgFadeAnim->setStartValue(m_msgFade->opacity());
    m_msgFadeAnim->setEndValue(1.0);
    m_msgHeightAnim->start();
    m_msgFadeAnim->start();
}

void ValidatedField::applyState(State s, const QString &message, bool animate)
{
    const bool changed = (s != m_state);
    m_state = s;
    m_message = message;

    // bordure
    const QColor target = (s == Valid) ? kValid : (s == Invalid) ? kInvalid : kNeutral;
    if (changed) {
        if (animate) {
            animateBorder(target);
        } else {
            m_colorAnim->stop();
            if (s == Neutral) {
                m_field->setStyleSheet(QString());
                m_hasOwnBorder = false;
                m_currentBorder = kNeutral;
            } else {
                applyBorder(target);
            }
        }
    }

    // icône ✓ / ✕ (fondu)
    if (changed) {
        if (s == Valid) {
            m_icon->setText(QString::fromUtf8("✓"));
            m_icon->setStyleSheet(QString("color: %1; font-size: 16px; font-weight: 800; background: transparent;")
                                      .arg(kValid.name()));
        } else if (s == Invalid) {
            m_icon->setText(QString::fromUtf8("✕"));
            m_icon->setStyleSheet(QString("color: %1; font-size: 15px; font-weight: 800; background: transparent;")
                                      .arg(kInvalid.name()));
        }
        m_iconAnim->stop();
        const qreal end = (s == Neutral) ? 0.0 : 1.0;
        if (animate) {
            if (s != Neutral)
                m_iconFade->setOpacity(0.0);     // l'icône "apparaît" à chaque changement d'état
            m_iconAnim->setStartValue(m_iconFade->opacity());
            m_iconAnim->setEndValue(end);
            m_iconAnim->start();
        } else {
            m_iconFade->setOpacity(end);
        }
    }

    // message explicatif (seulement si invalide)
    showMessage(s == Invalid ? message : QString(), animate);
}

void ValidatedField::revalidate(bool animate)
{
    Result r = m_validator();
    if (!m_touched && r.state != Invalid) {
        // jamais touché : reste neutre, même si la valeur par défaut est correcte
        applyState(Neutral, QString(), animate);
        return;
    }
    if (!m_touched)
        r = neutral();
    applyState(r.state, r.message, animate);
}

void ValidatedField::onInput()
{
    m_touched = true;
    revalidate(true);
}

void ValidatedField::resetTouched()
{
    m_touched = false;
    revalidate(false);
}

bool ValidatedField::validateNow()
{
    m_touched = true;
    const Result r = m_validator();
    applyState(r.state, r.message, true);
    return r.state != Invalid;
}
