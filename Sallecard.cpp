#include "Sallecard.h"
#include "salle.h"
#include "icones.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QGraphicsDropShadowEffect>
#include <QStyle>

QString SalleCard::cleEtat(const QString& etat)
{
    if (etat == "Libre")   return "dispo";
    if (etat == "Occupée") return "occupe";
    return "hors";
}

QString SalleCard::libelleEtat(const QString& etat)
{
    if (etat == "Libre")   return "Disponible";
    if (etat == "Occupée") return "Occupé";
    return "Hors service";
}

QString SalleCard::detailSalle(const Salle* s)
{
    if (auto c = dynamic_cast<const SalleConsultation*>(s))
        return QString("Spécialité : ") + c->getSpecialite();
    if (auto c = dynamic_cast<const SalleChirurgie*>(s))
        return QString("Bloc : ") + c->getTypeBloc();
    if (auto c = dynamic_cast<const SalleToilettage*>(s))
        return QString("Bain : ") + c->getTypeBain();
    if (auto c = dynamic_cast<const SallePension*>(s))
        return QString("Animaux : ") + c->getTypeAnimal();
    if (auto c = dynamic_cast<const SalleAdoption*>(s))
        return c->getDescription();
    if (auto c = dynamic_cast<const SalleStorage*>(s))
        return QString("Contenu : ") + c->getContenu();
    return QString();
}

SalleCard::SalleCard(const Salle* s, QWidget* parent)
    : QFrame(parent), id(s->getId())
{
    setObjectName("salleCard");
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(150);
    setProperty("selected", false);

    QVBoxLayout* v = new QVBoxLayout(this);
    v->setContentsMargins(16, 14, 16, 14);
    v->setSpacing(6);

    // --- Ligne du haut : icône + badge d'occupation ---
    QHBoxLayout* haut = new QHBoxLayout();

    QLabel* glyph = new QLabel();
    glyph->setPixmap(Icones::pixmap("grid", QColor("#9AA8AE"), 18));
    glyph->setFixedSize(18, 18);
    haut->addWidget(glyph);
    haut->addStretch();

    QLabel* badge = new QLabel();
    badge->setObjectName("cardBadge");
    if (s->getType() == "Storage") {
        badge->setText("Stockage");
    } else {
        badge->setText(QString("%1/%2 %3")
                           .arg(s->getOccupation())
                           .arg(s->getCapacite())
                           .arg(s->getOccupation() > 1 ? "animaux" : "animal"));
    }
    haut->addWidget(badge);
    v->addLayout(haut);
    v->addSpacing(4);

    // --- Nom + type ---
    QLabel* titre = new QLabel(s->getNom());
    titre->setObjectName("cardTitle");
    titre->setWordWrap(true);
    v->addWidget(titre);

    QLabel* sous = new QLabel(s->getType());
    sous->setObjectName("cardSub");
    v->addWidget(sous);

    // --- Pastille d'état ---
    QLabel* pill = new QLabel(libelleEtat(s->getEtat()));
    pill->setObjectName("cardPill");
    pill->setProperty("etat", cleEtat(s->getEtat()));
    QHBoxLayout* ligPill = new QHBoxLayout();
    ligPill->addWidget(pill);
    ligPill->addStretch();
    v->addLayout(ligPill);

    v->addStretch(1);

    // --- Détail ---
    QLabel* info = new QLabel(detailSalle(s));
    info->setObjectName("cardInfo");
    info->setWordWrap(true);
    v->addWidget(info);

    // --- Ombre + animation de survol ---
    m_ombre = new QGraphicsDropShadowEffect(this);
    m_ombre->setColor(QColor(32, 82, 94));
    setGraphicsEffect(m_ombre);
    appliquerOmbre();

    m_anim.setDuration(220);
    m_anim.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
        m_survol = val.toReal();
        appliquerOmbre();
    });
}

// Ombre au repos très légère ; grossit, s'éloigne et fonce au survol.
// Une carte sélectionnée garde une ombre intermédiaire.
void SalleCard::appliquerOmbre()
{
    const qreal h = qMax(m_survol, m_selectionnee ? 0.55 : 0.0);
    m_ombre->setBlurRadius(8 + 22 * h);
    m_ombre->setOffset(0, 1.5 + 5 * h);
    QColor c(32, 82, 94);
    c.setAlpha(int(28 + 62 * h));
    m_ombre->setColor(c);
}

void SalleCard::animerSurvol(qreal cible)
{
    m_anim.stop();
    m_anim.setStartValue(m_survol);
    m_anim.setEndValue(cible);
    m_anim.start();
}

void SalleCard::enterEvent(QEnterEvent* event)
{
    animerSurvol(1.0);
    QFrame::enterEvent(event);
}

void SalleCard::leaveEvent(QEvent* event)
{
    animerSurvol(0.0);
    QFrame::leaveEvent(event);
}

void SalleCard::setSelectionnee(bool selectionnee)
{
    m_selectionnee = selectionnee;
    setProperty("selected", selectionnee);
    style()->unpolish(this);
    style()->polish(this);
    appliquerOmbre();
    update();
}

void SalleCard::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        emit cliquee(id);
    QFrame::mousePressEvent(event);
}
