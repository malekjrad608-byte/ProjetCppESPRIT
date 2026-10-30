#include "../database/oracleconnection.h"
#include "../widgets/animatedbutton.h"
#include "detailpanel.h"
#include "hoverglow.h"
#include "icons.h"
#include "theme.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QVBoxLayout>

static QPushButton *makeButton(const QString &text, const QString &icon, const char *role)
{
    auto *b = new AnimatedButton(text);
    b->setProperty("role", role);
    b->setCursor(Qt::PointingHandCursor);
    if (!icon.isEmpty()) {
        const bool light = QString(role) != "ghost";
        b->setIcon(Icons::icon(icon, light ? QColor(Qt::white) : Theme::TealDark, 16));
        b->setIconSize(QSize(16, 16));
    }
    HoverGlow::attach(b);
    return b;
}

DetailPanel::DetailPanel(QWidget *parent) : QFrame(parent)
{
    setObjectName("panel");
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(20, 18, 20, 18);
    v->setSpacing(8);

    auto *kicker = new QLabel(QStringLiteral("LOCAL SÉLECTIONNÉ"));
    kicker->setObjectName("kicker");
    QFont kf = kicker->font();
    kf.setLetterSpacing(QFont::AbsoluteSpacing, 1.6);
    kicker->setFont(kf);
    v->addWidget(kicker);

    m_name = new QLabel(QStringLiteral("—"));
    m_name->setWordWrap(true);
    m_name->setObjectName("panelName");
    v->addWidget(m_name);

    auto *badgeRow = new QHBoxLayout;
    m_badge = new QLabel;
    badgeRow->addWidget(m_badge);
    badgeRow->addStretch();
    v->addLayout(badgeRow);

    m_alerts = new QLabel;
    m_alerts->setTextFormat(Qt::RichText);
    m_alerts->setWordWrap(true);
    m_alerts->setStyleSheet(QStringLiteral(
        "background:#fff4e2;color:#b97712;border-radius:10px;padding:8px 12px;font-weight:600;"));
    m_alerts->hide();
    v->addWidget(m_alerts);

    m_hint = new QLabel(QStringLiteral("Clique sur un local pour voir ses détails et le gérer."));
    m_hint->setObjectName("sub");
    m_hint->setWordWrap(true);
    v->addWidget(m_hint);

    // informations
    m_info = new QWidget;
    auto *g = new QGridLayout(m_info);
    g->setContentsMargins(0, 8, 0, 8);
    g->setHorizontalSpacing(16);
    g->setVerticalSpacing(7);
    const QStringList keys = { "Identifiant", "Type", "Étage", "Occupation",
                               "Spécialité", "Nettoyage", "Dernier nettoyage" };
    for (int i = 0; i < 7; ++i) {
        auto *k = new QLabel(keys[i]);
        k->setObjectName("key");
        m_values[i] = new QLabel;
        m_values[i]->setWordWrap(true);
        g->addWidget(k, i, 0);
        g->addWidget(m_values[i], i, 1);
        if(OracleConnection::isOracle()){if(i==3)k->setText("Capacité");if(i>=5){k->hide();m_values[i]->hide();}}
    }
    g->setColumnStretch(1, 1);
    v->addWidget(m_info);

    // boutons
    m_buttons = new QWidget;
    auto *bl = new QGridLayout(m_buttons);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(8);
    m_btnAssign = makeButton(QStringLiteral("Affecter un animal"), "paw", "primary");
    m_btnFree = makeButton(QStringLiteral("Retirer un animal (−1)"), "", "ghost");
    m_btnEdit = makeButton(QStringLiteral("Modifier"), "edit", "primary");
    m_btnClean = makeButton(QStringLiteral("Nettoyage"), "sparkle", "success");
    m_btnDelete = makeButton(QStringLiteral("Supprimer"), "trash", "danger");
    m_btnAssign->setIcon(QIcon(Icons::paw(Qt::white, 16)));
    bl->addWidget(m_btnAssign, 0, 0, 1, 2);
    bl->addWidget(m_btnFree, 1, 0, 1, 2);
    bl->addWidget(m_btnClean, 2, 0, 1, 2);
    bl->addWidget(m_btnEdit, 3, 0, 1, 2);
    bl->addWidget(m_btnDelete, 4, 0, 1, 2);
    v->addWidget(m_buttons);
    if(OracleConnection::isOracle()){m_btnAssign->hide();m_btnFree->hide();m_btnClean->hide();}

    connect(m_btnAssign, &QPushButton::clicked, this, &DetailPanel::assignRequested);
    connect(m_btnFree, &QPushButton::clicked, this, &DetailPanel::freeRequested);
    connect(m_btnEdit, &QPushButton::clicked, this, &DetailPanel::editRequested);
    connect(m_btnClean, &QPushButton::clicked, this, &DetailPanel::cleanRequested);
    connect(m_btnDelete, &QPushButton::clicked, this, &DetailPanel::deleteRequested);

    setLocal(nullptr);
}

void DetailPanel::setLocal(const Local *l, const QStringList &alerts)
{
    const bool has = (l != nullptr);
    m_alerts->setVisible(has && !alerts.isEmpty());
    if (has && !alerts.isEmpty()) {
        QStringList lines;
        for (const QString &a : alerts)
            lines << QStringLiteral("!  ") + a.toHtmlEscaped();
        m_alerts->setText(lines.join("<br>"));
    }
    m_hint->setVisible(!has);
    m_badge->setVisible(has);
    m_info->setVisible(has);
    m_buttons->setVisible(has);
    m_name->setText(has ? l->name : QStringLiteral("—"));
    if (!has)
        return;

    QColor bg, fg;
    Theme::stateColors(l->state(), bg, fg);
    m_badge->setText(QStringLiteral("●  ") + l->stateText());
    m_badge->setStyleSheet(QStringLiteral(
        "background:%1;color:%2;border-radius:8px;padding:4px 12px;font-weight:600;")
        .arg(bg.name(), fg.name()));

    m_values[0]->setText(QString::number(l->id));
    m_values[1]->setText(l->type);
    m_values[2]->setText(l->floorText());
    m_values[3]->setText(OracleConnection::isOracle()?QString::number(l->capacity):QStringLiteral("%1 / %2").arg(l->occupancy).arg(l->capacity));
    m_values[4]->setText(l->specialty.isEmpty() ? QStringLiteral("—") : l->specialty);
    m_values[5]->setText(l->cleaningText());
    m_values[6]->setText(QLocale(QLocale::French).toString(l->lastCleaned, "dd MMM, HH:mm"));

    m_btnAssign->setEnabled(!l->outOfService && l->occupancy < l->capacity);
    m_btnFree->setEnabled(l->occupancy > 0);

    switch (l->cleaning) {
    case CleaningState::Clean:      m_btnClean->setText(QStringLiteral("Signaler à nettoyer"));   break;
    case CleaningState::ToClean:    m_btnClean->setText(QStringLiteral("Démarrer le nettoyage")); break;
    case CleaningState::InProgress: m_btnClean->setText(QStringLiteral("Nettoyage terminé"));     break;
    }
}
