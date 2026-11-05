#include "recordcard.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QSizePolicy>
#include <QStyle>

// Carte compacte (même présentation que la maquette) :
//   [                         ✔ Vaccins à jour ]
//   Nom de l'animal
//   Espèce · Race
//   🕒 date de consultation              Propriétaire
RecordCard::RecordCard(const MedicalRecord &record, QWidget *parent)
    : QFrame(parent), m_recordId(record.getIdDossier())
{
    setObjectName("recordCard");
    setProperty("selected", false);
    setCursor(Qt::PointingHandCursor);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 12, 16, 12);
    root->setSpacing(3);

    // Ligne du haut : badge de statut à droite
    auto *top = new QHBoxLayout();
    top->setContentsMargins(0, 0, 0, 0);
    top->addStretch();
    const bool upToDate = record.getStatutVaccination() == "À jour";
    auto *badge = new QLabel(upToDate ? QString::fromUtf8("✔  Vaccins à jour")
                                      : QString::fromUtf8("⚠  À renouveler"));
    badge->setObjectName("cardBadge");
    badge->setProperty("tone", upToDate ? "ok" : "warn");
    top->addWidget(badge);
    root->addLayout(top);

    auto *title = new QLabel(record.getNomAnimal());
    title->setObjectName("recordTitle");
    title->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    root->addWidget(title);

    auto *sub = new QLabel(QString("%1 · %2").arg(record.getEspece(), record.getRace()));
    sub->setObjectName("recordSub");
    sub->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    root->addWidget(sub);

    root->addStretch();

    // Ligne du bas : date de consultation à gauche, propriétaire à droite
    auto *bottom = new QHBoxLayout();
    bottom->setContentsMargins(0, 0, 0, 0);
    auto *date = new QLabel(QString::fromUtf8("🕒 ") + record.getDateConsultation().toString("dd/MM/yyyy"));
    date->setObjectName("recordMeta");
    bottom->addWidget(date);
    bottom->addStretch();
    auto *owner = new QLabel(record.getProprietaire());
    owner->setObjectName("recordMeta");
    owner->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    owner->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    bottom->addWidget(owner, 1);
    root->addLayout(bottom);

    for (QLabel *l : findChildren<QLabel *>())
        l->setAttribute(Qt::WA_TransparentForMouseEvents);
}

void RecordCard::setSelected(bool selected)
{
    setProperty("selected", selected);
    style()->unpolish(this);
    style()->polish(this);
}

void RecordCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked(m_recordId);
    QFrame::mousePressEvent(event);
}

void RecordCard::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit doubleClicked(m_recordId);
    QFrame::mouseDoubleClickEvent(event);
}
