#include "plandialog.h"
#include "clinicmap.h"
#include "flatmap.h"
#include "hoverglow.h"
#include "icons.h"
#include "localrepository.h"
#include "theme.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
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
    m_sub = new QLabel;
    m_sub->setObjectName("sub");
    col->addWidget(title);
    col->addWidget(m_sub);
    titleRow->addWidget(ic);
    titleRow->addLayout(col);
    titleRow->addStretch();

    // choix de la vue : 2.5D ou plan (comme les boutons Cartes / Liste de la page)
    auto makeToggle = [](const QString &text, const QString &icon) {
        auto *b = new QPushButton(text);
        b->setIcon(Icons::icon(icon, Theme::TealDark, 16));
        b->setProperty("role", "ghost");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        HoverGlow::attach(b);
        return b;
    };
    m_btn3d = makeToggle(QStringLiteral("Vue 2.5D"), "rooms");
    m_btnFlat = makeToggle(QStringLiteral("Vue plan"), "map");
    titleRow->addWidget(m_btn3d);
    titleRow->addWidget(m_btnFlat);
    root->addLayout(titleRow);

    // commandes propres a la vue 2.5D
    auto *tools = new QHBoxLayout;
    tools->setSpacing(10);
    m_rotate = new QPushButton(QStringLiteral("Tourner"));
    m_rotate->setIcon(Icons::icon("refresh", Theme::TealDark, 16));
    m_rotate->setProperty("role", "ghost");
    m_rotate->setCursor(Qt::PointingHandCursor);
    m_rotate->setToolTip(QStringLiteral("Tourne le bâtiment d'un quart de tour"));
    HoverGlow::attach(m_rotate);
    m_explode = new QPushButton(QStringLiteral("Vue éclatée"));
    m_explode->setIcon(Icons::icon("grid", Theme::TealDark, 16));
    m_explode->setProperty("role", "ghost");
    m_explode->setCheckable(true);
    m_explode->setCursor(Qt::PointingHandCursor);
    m_explode->setToolTip(QStringLiteral("Écarte les étages pour bien voir chaque niveau"));
    HoverGlow::attach(m_explode);
    tools->addStretch();
    tools->addWidget(m_rotate);
    tools->addWidget(m_explode);
    root->addLayout(tools);

    // les deux vues, une seule visible a la fois
    m_views = new QStackedWidget;
    m_map = new ClinicMap(false);
    m_flat = new FlatMap(false);
    auto *flatScroll = new QScrollArea;            // la vue plan peut etre haute : on la fait defiler
    flatScroll->setWidgetResizable(true);
    flatScroll->setFrameShape(QFrame::NoFrame);
    flatScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    flatScroll->setWidget(m_flat);
    m_views->addWidget(m_map);       // index 0
    m_views->addWidget(flatScroll);  // index 1
    root->addWidget(m_views, 1);

    connect(m_btn3d, &QPushButton::clicked, this, [this] { setView(0); });
    connect(m_btnFlat, &QPushButton::clicked, this, [this] { setView(1); });
    connect(m_rotate, &QPushButton::clicked, m_map, &ClinicMap::rotateQuarter);
    connect(m_explode, &QPushButton::toggled, m_map, &ClinicMap::setExploded);

    auto *legend = new QLabel(QStringLiteral(
        "<span style='color:#4aa57a'>■</span> Disponible &nbsp;&nbsp; "
        "<span style='color:#5b76c9'>■</span> Occupé &nbsp;&nbsp; "
        "<span style='color:#d05a55'>■</span> À nettoyer (clignote) &nbsp;&nbsp; "
        "<span style='color:#a9b4b8'>■</span> Hors service"));
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
    // un clic dans l'une ou l'autre vue selectionne le local dans les deux
    auto onRoom = [this](int id) {
        m_selected = id;
        m_map->setSelected(id);
        m_flat->setSelected(id);
        updateInfo();
    };
    connect(m_map, &ClinicMap::roomClicked, this, onRoom);
    connect(m_flat, &FlatMap::roomClicked, this, onRoom);
    connect(m_repo, &LocalRepository::changed, this, [this] {
        m_map->setLocaux(m_repo->all());
        m_flat->setLocaux(m_repo->all());
        updateInfo();
    });

    m_map->setLocaux(m_repo->all());
    m_flat->setLocaux(m_repo->all());
    setView(0);                      // on ouvre sur la vue 2.5D
    updateInfo();
}

void PlanDialog::setView(int index)
{
    m_views->setCurrentIndex(index);
    m_btn3d->setChecked(index == 0);
    m_btnFlat->setChecked(index == 1);
    m_rotate->setVisible(index == 0);
    m_explode->setVisible(index == 0);
    m_sub->setText(index == 0
        ? QStringLiteral("Vue 2.5D en direct · glisse avec la souris pour tourner le bâtiment · clique sur un local")
        : QStringLiteral("Vue plan · un bloc par étage · clique sur un local"));
}

void PlanDialog::setSelected(int id)
{
    m_selected = id;
    m_map->setSelected(id);
    m_flat->setSelected(id);
    updateInfo();
}

void PlanDialog::updateInfo()
{
    const Local *l = m_repo->find(m_selected);
    m_goto->setEnabled(l != nullptr);
    if (!l) {
        m_selected = 0;
        m_map->setSelected(0);
        m_flat->setSelected(0);
        m_info->setText(QStringLiteral("Clique sur un local du plan pour voir ses détails."));
        return;
    }
    m_info->setText(QStringLiteral(
        "<b>%1</b> &nbsp;·&nbsp; %2 &nbsp;·&nbsp; %3 &nbsp;·&nbsp; %4 &nbsp;·&nbsp; "
        "Occupation <b>%5/%6</b>")
        .arg(l->getName().toHtmlEscaped(), l->getType().toHtmlEscaped(), l->floorText(), l->stateText())
        .arg(l->getOccupancy())
        .arg(l->getCapacity()));
}
