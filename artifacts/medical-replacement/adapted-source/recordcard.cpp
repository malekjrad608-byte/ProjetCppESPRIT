#include "recordcard.h"
#include "../ui/design.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStyle>

RecordCard::RecordCard(const MedicalRecord &record, QWidget *parent)
    : AnimatedPanel(parent), m_recordId(record.medical.recordId)
{
    setObjectName("recordCard");
    setProperty("selected", false);
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(170);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 18, 20, 18);
    root->setSpacing(6);

    // Top row: icon + vaccination badge
    auto *top = new QHBoxLayout();
    auto *icon = new QLabel();
    icon->setObjectName("cardGlyph");
    icon->setPixmap(Design::icon("paw").pixmap(22,22));
    top->addWidget(icon);
    top->addStretch();

    const bool upToDate = record.medical.vaccinationStatus == "À jour";
    auto *badge = new QLabel(upToDate ? "Vaccins à jour" : "À renouveler");
    badge->setObjectName("cardBadge");
    badge->setProperty("tone", upToDate ? "ok" : "warn");
    top->addWidget(badge);
    root->addLayout(top);

    auto *title = new QLabel(record.animal.name);
    title->setObjectName("recordTitle");
    root->addWidget(title);

    auto *sub = new QLabel(QString("%1 · %2").arg(record.animal.species, record.animal.breed));
    sub->setObjectName("recordSub");
    root->addWidget(sub);

    auto *pillRow = new QHBoxLayout();
    auto *pill = new QLabel(record.medical.consultationDate.toString("dd/MM/yyyy"));
    pill->setObjectName("statusPill");
    pill->setProperty("tone", "ok");
    pillRow->addWidget(pill);
    pillRow->addStretch();
    root->addLayout(pillRow);

    auto *owner = new QLabel(QString("Propriétaire : %1").arg(record.animal.owner));
    owner->setObjectName("recordMeta");
    root->addWidget(owner);
    root->addStretch();

    for (QLabel *l : findChildren<QLabel *>()) {
        l->setTextFormat(Qt::PlainText);
        l->setAttribute(Qt::WA_TransparentForMouseEvents);
        l->setToolTip(l->text());
    }
    for(auto *l : {title, sub, owner}) l->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
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
