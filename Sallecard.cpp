#include "sallecard.h"
#include "salle.h"
#include "icones.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
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

    // --- Détail (spécialité, bloc, contenu...) ---
    QLabel* info = new QLabel(detailSalle(s));
    info->setObjectName("cardInfo");
    info->setWordWrap(true);
    v->addWidget(info);
}

void SalleCard::setSelectionnee(bool selectionnee)
{
    setProperty("selected", selectionnee);
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void SalleCard::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        emit cliquee(id);
    QFrame::mousePressEvent(event);
}
