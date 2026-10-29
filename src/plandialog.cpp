#include "plandialog.h"
#include "clinicmap.h"
#include "hoverglow.h"
#include "icons.h"
#include "localrepository.h"
#include "theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

PlanDialog::PlanDialog(LocalRepository *repo, QWidget *parent) : QDialog(parent), m_repo(repo)
{
    setObjectName("statsDialog");
    setWindowTitle(QStringLiteral("Plan de la clinique"));
    setAttribute(Qt::WA_DeleteOnClose);
    resize(900, 680);
    setMinimumSize(640, 480);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 20);
    root->setSpacing(14);

    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(12);
    auto *ic = new QLabel;
    ic->setPixmap(Icons::pixmap("map", Theme::Teal, 34));
    auto *col = new QVBoxLayout;
    col->setSpacing(0);
    auto *title = new QLabel(QStringLiteral("Plan de la clinique"));
    title->setObjectName("statsTitle");
    auto *sub = new QLabel(QStringLiteral("Le plan se met à jour tout seul quand tu ajoutes, modifies ou supprimes un local"));
    sub->setObjectName("sub");
    col->addWidget(title);
    col->addWidget(sub);
    titleRow->addWidget(ic);
    titleRow->addLayout(col);
    titleRow->addStretch();
    root->addLayout(titleRow);

    m_map = new ClinicMap(false);
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidget(m_map);
    root->addWidget(scroll, 1);

    auto *legend = new QLabel(QStringLiteral(
        "<span style='color:#4aa57a'>■</span> Disponible &nbsp;&nbsp; "
        "<span style='color:#5b76c9'>■</span> Occupé &nbsp;&nbsp; "
        "<span style='color:#d9972f'>■</span> Hors service &nbsp;&nbsp; "
        "<span style='color:#c0504d'>●</span> À nettoyer (clignote)"));
    legend->setTextFormat(Qt::RichText);
    root->addWidget(legend);

    m_info = new QLabel;
    m_info->setTextFormat(Qt::RichText);
    m_info->setWordWrap(true);
    m_info->setStyleSheet(QStringLiteral(
        "background:#eef6f8;border:1px solid #e1ebee;border-radius:12px;padding:12px 16px;"));
    root->addWidget(m_info);

    auto *btns = new QHBoxLayout;
    btns->setSpacing(10);
    btns->addStretch();
    m_goto = new QPushButton(QStringLiteral("Voir ce local dans la liste"));
    m_goto->setProperty("role", "ghost");
    m_goto->setCursor(Qt::PointingHandCursor);
    HoverGlow::attach(m_goto);
    auto *close = new QPushButton(QStringLiteral("Fermer"));
    close->setProperty("role", "primary");
    close->setCursor(Qt::PointingHandCursor);
    close->setMinimumWidth(140);
    HoverGlow::attach(close);
    btns->addWidget(m_goto);
    btns->addWidget(close);
    root->addLayout(btns);

    connect(close, &QPushButton::clicked, this, &QDialog::close);
    connect(m_goto, &QPushButton::clicked, this, [this] {
        if (m_selected)
            emit selectRequested(m_selected);
    });
    connect(m_map, &ClinicMap::roomClicked, this, [this](int id) {
        m_selected = id;
        m_map->setSelected(id);
        updateInfo();
    });
    connect(m_repo, &LocalRepository::changed, this, [this] {
        m_map->setLocaux(m_repo->all());
        updateInfo();
    });

    m_map->setLocaux(m_repo->all());
    updateInfo();
}

void PlanDialog::setSelected(int id)
{
    m_selected = id;
    m_map->setSelected(id);
    updateInfo();
}

void PlanDialog::updateInfo()
{
    const Local *l = m_repo->find(m_selected);
    m_goto->setEnabled(l != nullptr);
    if (!l) {
        m_selected = 0;
        m_map->setSelected(0);
        m_info->setText(QStringLiteral("Clique sur un local du plan pour voir ses détails."));
        return;
    }
    m_info->setText(QStringLiteral(
        "<b>%1</b> &nbsp;·&nbsp; %2 &nbsp;·&nbsp; %3 &nbsp;·&nbsp; %4 &nbsp;·&nbsp; "
        "Occupation <b>%5/%6</b> &nbsp;·&nbsp; Nettoyage : <b>%7</b>")
        .arg(l->getName().toHtmlEscaped(), l->getType().toHtmlEscaped(), l->floorText(), l->stateText())
        .arg(l->getOccupancy())
        .arg(l->getCapacity())
        .arg(l->cleaningText()));
}
