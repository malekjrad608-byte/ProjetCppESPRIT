#include "pagelocaux.h"
#include "cardgrid.h"
#include "clinicmap.h"
#include "confirmdialog.h"
#include "detailpanel.h"
#include "hoverglow.h"
#include "icons.h"
#include "localdialog.h"
#include "localrepository.h"
#include "pdfreport.h"
#include "plandialog.h"
#include "roomcard.h"
#include "smartsearch.h"
#include "smartassigndialog.h"
#include "statsdialog.h"
#include "theme.h"
#include "toast.h"

#include <QCollator>
#include <QComboBox>
#include <QDate>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStyle>
#include <QTableWidget>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>
#include <utility>

PageLocaux::PageLocaux(LocalRepository *repo, QWidget *parent)
    : QWidget(parent), m_repo(repo)
{
    setObjectName("page");
    setAttribute(Qt::WA_StyledBackground, true);
    m_toast = new Toast(this);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(26, 22, 26, 10);
    root->setSpacing(14);
    root->addLayout(buildHeader());
    root->addLayout(buildTools());
    root->addLayout(buildBody(), 1);

    // le plan et la liste se mettent a jour a chaque changement des donnees
    connect(m_repo, &LocalRepository::changed, this, [this] {
        m_miniMap->setLocaux(m_repo->all());
        refresh(false);
    });

    auto *find = new QShortcut(QKeySequence::Find, this);
    connect(find, &QShortcut::activated, this, [this] {
        m_search->setFocus();
        m_search->selectAll();
    });
    auto *add = new QShortcut(QKeySequence::New, this);
    connect(add, &QShortcut::activated, this, [this] { addLocal(); });

    m_miniMap->setLocaux(m_repo->all());
    refresh(true);
}

// ================================================================ construction

QPushButton *PageLocaux::makeButton(const QString &text, const QString &icon, const char *role)
{
    auto *b = new QPushButton(text);
    b->setProperty("role", role);
    b->setCursor(Qt::PointingHandCursor);
    const bool ghost = QString(role) == "ghost";
    b->setIcon(Icons::icon(icon, ghost ? Theme::TealDark : QColor(Qt::white), 16));
    b->setIconSize(QSize(16, 16));
    HoverGlow::attach(b);
    return b;
}

QPushButton *PageLocaux::makeChip(const char *tone)
{
    auto *b = new QPushButton;
    b->setProperty("chip", true);
    b->setProperty("tone", tone);
    b->setCursor(Qt::PointingHandCursor);
    HoverGlow::attach(b, QColor(31, 85, 96, 70), 16, 4);
    return b;
}

QLayout *PageLocaux::buildHeader()
{
    auto *row = new QHBoxLayout;
    row->setSpacing(16);

    // gauche : titre + pastilles
    auto *left = new QVBoxLayout;
    left->setSpacing(4);
    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(10);
    auto *ic = new QLabel;
    ic->setPixmap(Icons::pixmap("rooms", Theme::Teal, 28));
    auto *h1 = new QLabel(QStringLiteral("Locaux"));
    h1->setObjectName("h1");
    titleRow->addWidget(ic);
    titleRow->addWidget(h1);
    titleRow->addStretch();
    left->addLayout(titleRow);

    auto *sub = new QLabel(QStringLiteral("Occupation, nettoyage et disponibilité des espaces"));
    sub->setObjectName("sub");
    left->addWidget(sub);
    left->addSpacing(6);

    auto *chips = new QHBoxLayout;
    chips->setSpacing(10);
    m_chipAvail = makeChip("green");
    m_chipOcc = makeChip("blue");
    m_chipOos = makeChip("orange");
    m_chipDirty = makeChip("red");
    chips->addWidget(m_chipAvail);
    chips->addWidget(m_chipOcc);
    chips->addWidget(m_chipOos);
    chips->addWidget(m_chipDirty);
    chips->addStretch();
    left->addLayout(chips);

    // les pastilles sont des raccourcis de recherche : elles ajoutent / retirent un mot-cle
    connect(m_chipAvail, &QPushButton::clicked, this, [this] { toggleChip(int(SmartSearch::Kw::Available)); });
    connect(m_chipOcc, &QPushButton::clicked, this, [this] { toggleChip(int(SmartSearch::Kw::Occupied)); });
    connect(m_chipOos, &QPushButton::clicked, this, [this] { toggleChip(int(SmartSearch::Kw::OutOfService)); });
    connect(m_chipDirty, &QPushButton::clicked, this, [this] { toggleChip(int(SmartSearch::Kw::Dirty)); });

    // droite : deux rangees de boutons
    m_btnSmart = makeButton(QStringLiteral("Affectation intelligente"), "sparkle", "ghost");
    m_btnAdd = makeButton(QStringLiteral("Ajouter un local"), "plus", "primary");
    m_btnPdf = makeButton(QStringLiteral("Rapport nettoyage PDF"), "download", "ghost");
    m_btnMap = makeButton(QStringLiteral("Plan de la clinique"), "map", "ghost");
    m_btnStats = makeButton(QStringLiteral("Voir les statistiques"), "chart", "ghost");

    auto *r1 = new QHBoxLayout;
    r1->setSpacing(10);
    r1->addStretch();
    r1->addWidget(m_btnSmart);
    r1->addWidget(m_btnAdd);
    auto *r2 = new QHBoxLayout;
    r2->setSpacing(10);
    r2->addStretch();
    r2->addWidget(m_btnPdf);
    r2->addWidget(m_btnMap);
    r2->addWidget(m_btnStats);
    auto *btnCol = new QVBoxLayout;
    btnCol->setSpacing(10);
    btnCol->addLayout(r1);
    btnCol->addLayout(r2);
    btnCol->addStretch();

    connect(m_btnSmart, &QPushButton::clicked, this, [this] { openSmartAssign(); });
    connect(m_btnAdd, &QPushButton::clicked, this, [this] { addLocal(); });
    connect(m_btnPdf, &QPushButton::clicked, this, [this] { exportPdf(); });
    connect(m_btnMap, &QPushButton::clicked, this, [this] { openPlan(); });
    connect(m_btnStats, &QPushButton::clicked, this, [this] {
        StatsDialog dlg(m_repo->all(), this);
        dlg.exec();
    });

    row->addLayout(left, 1);
    row->addLayout(btnCol);
    return row;
}

QLayout *PageLocaux::buildTools()
{
    auto *col = new QVBoxLayout;
    col->setSpacing(6);
    auto *row = new QHBoxLayout;
    row->setSpacing(10);

    m_search = new QLineEdit;
    m_search->setPlaceholderText(QStringLiteral(
        "Recherche intelligente : « chirurgie dispo », « sale étage 1 », « plein »…"));
    m_search->setClearButtonEnabled(true);
    m_search->addAction(Icons::icon("search", Theme::Muted, 16), QLineEdit::LeadingPosition);

    m_sortCombo = new QComboBox;
    m_sortCombo->addItems({ QStringLiteral("Trier : priorité"),
                            QStringLiteral("Trier : nom A → Z"),
                            QStringLiteral("Trier : remplissage") });
    m_sortCombo->setToolTip(QStringLiteral(
        "Priorité : locaux à nettoyer (blocs d'abord), puis les plus remplis.\n"
        "En vue Liste, clique sur les en-têtes du tableau pour trier par colonne."));

    auto *reset = new QPushButton(QStringLiteral("Réinitialiser"));
    reset->setProperty("role", "link");
    reset->setCursor(Qt::PointingHandCursor);

    m_count = new QLabel;
    m_count->setObjectName("sub");

    m_btnCards = makeButton(QStringLiteral("Cartes"), "grid", "ghost");
    m_btnList = makeButton(QStringLiteral("Liste"), "list", "ghost");
    m_btnCards->setCheckable(true);
    m_btnList->setCheckable(true);
    m_btnCards->setChecked(true);

    row->addWidget(m_search, 1);
    row->addWidget(m_sortCombo);
    row->addWidget(reset);
    row->addWidget(m_count);
    row->addWidget(m_btnCards);
    row->addWidget(m_btnList);
    col->addLayout(row);

    m_queryHint = new QLabel;
    m_queryHint->setObjectName("sub");
    m_queryHint->setTextFormat(Qt::RichText);
    m_queryHint->hide();
    col->addWidget(m_queryHint);

    connect(m_search, &QLineEdit::textChanged, this, [this] { refresh(true); });
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { refresh(true); });
    connect(reset, &QPushButton::clicked, this, [this] { resetFilters(); });
    connect(m_btnCards, &QPushButton::clicked, this, [this] { setView(0); });
    connect(m_btnList, &QPushButton::clicked, this, [this] { setView(1); });
    return col;
}

QLayout *PageLocaux::buildBody()
{
    auto *row = new QHBoxLayout;
    row->setSpacing(18);

    // ---------------------------------------------------- gauche : cartes / liste
    auto *leftCol = new QVBoxLayout;
    m_views = new QStackedWidget;

    m_cardScroll = new QScrollArea;
    auto *scroll = m_cardScroll;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_grid = new CardGrid;
    scroll->setWidget(m_grid);
    m_views->addWidget(scroll);

    m_table = new QTableWidget(0, 6);
    m_table->setHorizontalHeaderLabels({ "ID", "Local", "Type", "Étage", "Occupation", "État" });
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(42);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->horizontalHeader()->setMinimumHeight(40);
    m_table->horizontalHeader()->setSectionsClickable(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_table->setColumnWidth(0, 70);
    m_table->setShowGrid(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_views->addWidget(m_table);

    m_empty = new QLabel(QStringLiteral("Aucun local ne correspond à la recherche"));
    m_empty->setObjectName("sub");
    m_empty->setAlignment(Qt::AlignCenter);
    m_empty->hide();
    leftCol->addWidget(m_views, 1);
    leftCol->addWidget(m_empty);
    row->addLayout(leftCol, 1);

    connect(m_table, &QTableWidget::itemSelectionChanged, this, [this] {
        if (m_updatingTable)
            return;
        const QModelIndexList rows = m_table->selectionModel()->selectedRows();
        if (rows.isEmpty())
            return;
        const int id = m_table->item(rows.first().row(), 0)->data(Qt::UserRole).toInt();
        if (id != m_selectedId)
            onCardClicked(id);
    });
    connect(m_table->horizontalHeader(), &QHeaderView::sectionClicked, this, [this](int c) {
        if (m_tableSortCol == c)
            m_tableSortOrder = (m_tableSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
        else {
            m_tableSortCol = c;
            m_tableSortOrder = Qt::AscendingOrder;
        }
        m_table->horizontalHeader()->setSortIndicatorShown(true);
        m_table->horizontalHeader()->setSortIndicator(c, m_tableSortOrder);
        m_table->sortItems(c, m_tableSortOrder);
    });

    // ---------------------------------------------------- droite : details, plan, occupation
    auto *rightScroll = new QScrollArea;
    rightScroll->setWidgetResizable(true);
    rightScroll->setFrameShape(QFrame::NoFrame);
    rightScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rightScroll->setFixedWidth(356);

    auto *rightWidget = new QWidget;
    auto *rv = new QVBoxLayout(rightWidget);
    rv->setContentsMargins(0, 0, 8, 0);
    rv->setSpacing(16);

    m_detail = new DetailPanel;
    rv->addWidget(m_detail);

    // plan en direct
    auto *planPanel = new QFrame;
    planPanel->setObjectName("panel");
    auto *pv = new QVBoxLayout(planPanel);
    pv->setContentsMargins(16, 14, 16, 14);
    pv->setSpacing(8);
    auto *planHead = new QHBoxLayout;
    auto *kicker = new QLabel(QStringLiteral("PLAN EN DIRECT"));
    kicker->setObjectName("kicker");
    QFont kf = kicker->font();
    kf.setLetterSpacing(QFont::AbsoluteSpacing, 1.6);
    kicker->setFont(kf);
    auto *enlarge = new QPushButton(QStringLiteral("Agrandir"));
    enlarge->setProperty("role", "link");
    enlarge->setCursor(Qt::PointingHandCursor);
    planHead->addWidget(kicker);
    planHead->addStretch();
    planHead->addWidget(enlarge);
    pv->addLayout(planHead);
    m_miniMap = new ClinicMap(true);
    pv->addWidget(m_miniMap);
    auto *legend = new QLabel(QStringLiteral(
        "<span style='color:#4aa57a'>■</span> Dispo &nbsp; "
        "<span style='color:#5b76c9'>■</span> Occupé &nbsp; "
        "<span style='color:#d05a55'>■</span> À nettoyer &nbsp; "
        "<span style='color:#a9b4b8'>■</span> H.S."));
    legend->setTextFormat(Qt::RichText);
    legend->setStyleSheet(QStringLiteral("font-size:11px;color:#6f8a92;"));
    pv->addWidget(legend);
    rv->addWidget(planPanel);
    connect(enlarge, &QPushButton::clicked, this, [this] { openPlan(); });
    connect(m_miniMap, &ClinicMap::roomClicked, this, [this](int id) { onCardClicked(id); });

    rv->addStretch();

    rightScroll->setWidget(rightWidget);
    row->addWidget(rightScroll);

    // ---------------------------------------------------- actions du panneau de detail
    connect(m_detail, &DetailPanel::assignRequested, this, [this] {
        modifySelected([this](Local &l) {
            if (!l.ajouterAnimal())
                return false;
            m_toast->showMessage(QStringLiteral("Animal affecté à %1 (%2 / %3)")
                                     .arg(l.getName()).arg(l.getOccupancy()).arg(l.getCapacity()));
            return true;
        });
    });
    connect(m_detail, &DetailPanel::freeRequested, this, [this] {
        modifySelected([this](Local &l) {
            if (!l.retirerAnimal())                    // un animal a la fois (-1)
                return false;
            if (l.getOccupancy() == 0) {               // dernier animal parti -> "À nettoyer"
                m_toast->showMessage(l.getName() + QStringLiteral(" est vide — à nettoyer"));
            } else {
                m_toast->showMessage(QStringLiteral("Animal retiré de %1 (%2 / %3)")
                                         .arg(l.getName()).arg(l.getOccupancy()).arg(l.getCapacity()));
            }
            return true;
        });
    });
    connect(m_detail, &DetailPanel::cleanRequested, this, [this] {
        modifySelected([this](Local &l) {
            if (l.terminerNettoyage()) {
                m_toast->showMessage(l.getName() + QStringLiteral(" est propre — disponible"));
                return true;
            }
            if (l.signalerANettoyer()) {
                m_toast->showMessage(l.getName() + QStringLiteral(" signalé à nettoyer"));
                return true;
            }
            return false;
        });
    });
    connect(m_detail, &DetailPanel::editRequested, this, [this] { editSelected(); });
    connect(m_detail, &DetailPanel::deleteRequested, this, [this] { deleteSelected(); });
    return row;
}

// ================================================================ donnees affichees

QList<Local> PageLocaux::filteredSorted() const
{
    const SmartQuery q = SmartSearch::parse(m_search->text());

    QList<Local> out;
    for (const Local &l : m_repo->all())
        if (q.matches(l))
            out.append(l);

    QCollator coll;
    coll.setNumericMode(true);
    coll.setCaseSensitivity(Qt::CaseInsensitive);
    switch (m_sortCombo->currentIndex()) {
    case 1:     // nom A -> Z
        std::sort(out.begin(), out.end(), [&](const Local &a, const Local &b) {
            return coll.compare(a.getName(), b.getName()) < 0;
        });
        break;
    case 2:     // remplissage (taux d'occupation)
        std::stable_sort(out.begin(), out.end(), [](const Local &a, const Local &b) {
            const double ra = a.getCapacity() > 0 ? double(a.getOccupancy()) / a.getCapacity() : 0.0;
            const double rb = b.getCapacity() > 0 ? double(b.getOccupancy()) / b.getCapacity() : 0.0;
            return ra > rb;
        });
        break;
    default:    // priorite : a nettoyer (urgent d'abord) > remplissage
        std::stable_sort(out.begin(), out.end(), [](const Local &a, const Local &b) {
            const bool da = a.getCleaning() == CleaningState::ToClean, db = b.getCleaning() == CleaningState::ToClean;
            if (da != db)
                return da;
            if (da && a.isUrgentType() != b.isUrgentType())
                return a.isUrgentType();
            const double ra = a.getCapacity() > 0 ? double(a.getOccupancy()) / a.getCapacity() : 0.0;
            const double rb = b.getCapacity() > 0 ? double(b.getOccupancy()) / b.getCapacity() : 0.0;
            return ra > rb;
        });
        break;
    }
    return out;
}

void PageLocaux::refresh(bool playIntro)
{
    if (playIntro) {
        m_tableSortCol = -1;
        m_table->horizontalHeader()->setSortIndicatorShown(false);
    }

    const QList<Local> list = filteredSorted();

    QList<RoomCard *> cards;
    int i = 0;
    for (const Local &l : list) {
        auto *c = new RoomCard(l);
        c->setSelected(l.getId() == m_selectedId);
        connect(c, &RoomCard::clicked, this, &PageLocaux::onCardClicked);
        if (playIntro)
            c->playIntro(qMin(i, 14) * 35);
        cards.append(c);
        ++i;
    }
    m_grid->setCards(cards);
    updateTable(list);
    // liste vide : soit la recherche ne trouve rien, soit la table LOCAUX n'a pas pu etre lue
    if (!m_repo->loadError().isEmpty())
        m_empty->setText(QStringLiteral("Impossible de lire la table LOCAUX :\n") + m_repo->loadError());
    else if (m_repo->all().isEmpty())
        m_empty->setText(QStringLiteral("Aucun local pour le moment : clique sur « Ajouter un local »"));
    else
        m_empty->setText(QStringLiteral("Aucun local ne correspond à la recherche"));
    m_empty->setWordWrap(true);
    m_empty->setVisible(list.isEmpty());
    updateQueryHint();
    m_count->setText(QStringLiteral("%1 / %2 locaux").arg(list.size()).arg(m_repo->all().size()));

    if (!m_repo->find(m_selectedId))
        m_selectedId = 0;
    m_miniMap->setSelected(m_selectedId);
    if (m_plan)
        m_plan->setSelected(m_selectedId);
    showDetail();
    updateChips();
}

void PageLocaux::updateTable(const QList<Local> &list)
{
    m_updatingTable = true;
    m_table->clearSelection();
    m_table->setRowCount(0);
    m_table->setRowCount(list.size());
    for (int r = 0; r < list.size(); ++r) {
        const Local &l = list[r];

        auto *idItem = new QTableWidgetItem;
        idItem->setData(Qt::DisplayRole, l.getId());
        idItem->setData(Qt::UserRole, l.getId());
        idItem->setTextAlignment(Qt::AlignCenter);

        auto *name = new QTableWidgetItem(Icons::icon(l.iconName(), Theme::Teal, 18), "  " + l.getName());
        QFont bold = name->font();
        bold.setBold(true);
        name->setFont(bold);

        auto *type = new QTableWidgetItem(l.getType());
        auto *floor = new QTableWidgetItem(l.floorText());
        auto *occ = new QTableWidgetItem(QStringLiteral("%1 / %2").arg(l.getOccupancy()).arg(l.getCapacity()));

        QColor bg, fg;
        auto *state = new QTableWidgetItem(l.stateText());
        Theme::stateColors(l.state(), bg, fg);
        state->setForeground(fg);
        state->setFont(bold);

        m_table->setItem(r, 0, idItem);
        m_table->setItem(r, 1, name);
        m_table->setItem(r, 2, type);
        m_table->setItem(r, 3, floor);
        m_table->setItem(r, 4, occ);
        m_table->setItem(r, 5, state);
    }
    if (m_tableSortCol >= 0)
        m_table->sortItems(m_tableSortCol, m_tableSortOrder);
    for (int r = 0; r < m_table->rowCount(); ++r) {
        if (m_table->item(r, 0)->data(Qt::UserRole).toInt() == m_selectedId) {
            m_table->selectRow(r);
            break;
        }
    }
    m_updatingTable = false;
}

void PageLocaux::updateChips()
{
    int avail = 0, occ = 0, oos = 0, dirty = 0;
    for (const Local &l : m_repo->all()) {
        switch (l.state()) {
        case RoomState::Available:    ++avail; break;
        case RoomState::Occupied:     ++occ;   break;
        case RoomState::OutOfService: ++oos;   break;
        case RoomState::ToClean:      ++dirty; break;
        }
    }
    m_chipAvail->setText(QStringLiteral("%1 disponible(s)").arg(avail));
    m_chipOcc->setText(QStringLiteral("%1 occupé(s)").arg(occ));
    m_chipOos->setText(QStringLiteral("%1 hors service").arg(oos));
    m_chipDirty->setText(QStringLiteral("%1 à nettoyer").arg(dirty));

    // pastille "active" quand son mot-cle est dans la recherche
    const SmartQuery q = SmartSearch::parse(m_search->text());
    const std::pair<QPushButton *, SmartSearch::Kw> chips[] = {
        { m_chipAvail, SmartSearch::Kw::Available }, { m_chipOcc, SmartSearch::Kw::Occupied },
        { m_chipOos, SmartSearch::Kw::OutOfService }, { m_chipDirty, SmartSearch::Kw::Dirty } };
    for (const auto &c : chips) {
        const bool on = SmartSearch::isActive(q, c.second);
        if (c.first->property("on").toBool() != on) {
            c.first->setProperty("on", on);
            c.first->style()->unpolish(c.first);
            c.first->style()->polish(c.first);
        }
    }
}

void PageLocaux::updateQueryHint()
{
    const SmartQuery q = SmartSearch::parse(m_search->text());
    if (q.isEmpty()) {
        m_queryHint->hide();
        return;
    }
    m_queryHint->setText(QStringLiteral("<span style='color:#6f8a92'>Compris :</span> <b>%1</b>")
                             .arg(q.understood.join(QStringLiteral("  ·  ")).toHtmlEscaped()));
    m_queryHint->show();
}

void PageLocaux::setView(int index)
{
    m_views->setCurrentIndex(index);
    m_btnCards->setChecked(index == 0);
    m_btnList->setChecked(index == 1);
    m_sortCombo->setVisible(index == 0);   // en liste : on trie en cliquant sur les en-tetes
}

// ================================================================ filtres

void PageLocaux::resetFilters()
{
    {
        QSignalBlocker b1(m_search), b2(m_sortCombo);
        m_search->clear();
        m_sortCombo->setCurrentIndex(0);
    }
    refresh(true);
}

void PageLocaux::toggleChip(int kw)
{
    m_search->setText(SmartSearch::toggle(m_search->text(), SmartSearch::Kw(kw)));   // relance refresh
}

// ================================================================ selection

void PageLocaux::showDetail()
{
    const Local *l = m_repo->find(m_selectedId);
    m_detail->setLocal(l);
}

void PageLocaux::onCardClicked(int id)
{
    m_selectedId = id;
    for (RoomCard *c : m_grid->cards()) {
        c->setSelected(c->localId() == id);
        if (c->localId() == id && m_views->currentIndex() == 0)
            m_cardScroll->ensureWidgetVisible(c);
    }
    m_miniMap->setSelected(id);
    if (m_plan)
        m_plan->setSelected(id);

    m_updatingTable = true;
    m_table->clearSelection();
    for (int r = 0; r < m_table->rowCount(); ++r) {
        if (m_table->item(r, 0)->data(Qt::UserRole).toInt() == id) {
            m_table->selectRow(r);
            break;
        }
    }
    m_updatingTable = false;

    showDetail();
}

void PageLocaux::selectRoom(int id)
{
    if (!m_repo->find(id))
        return;
    bool visible = false;
    for (RoomCard *c : m_grid->cards())
        if (c->localId() == id)
            visible = true;
    if (!visible)
        resetFilters();
    onCardClicked(id);
}

// ================================================================ CRUD

void PageLocaux::addLocal()
{
    LocalDialog dlg(nullptr, m_repo, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    const int id = m_repo->add(dlg.result());
    if (id == 0) {
        m_toast->showMessage(QStringLiteral("Ajout non effectué : ") + m_repo->lastDbError());
        return;
    }
    selectRoom(id);
    m_toast->showMessage(QStringLiteral("Local ajouté — visible sur le plan"));
}

void PageLocaux::editSelected()
{
    const Local *cur = m_repo->find(m_selectedId);
    if (!cur)
        return;
    LocalDialog dlg(cur, m_repo, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    if (!m_repo->update(dlg.result())) {
        m_toast->showMessage(QStringLiteral("Modification non effectuée : ") + m_repo->lastDbError());
        return;
    }
    m_toast->showMessage(QStringLiteral("Local modifié"));
}

void PageLocaux::deleteSelected()
{
    const Local *cur = m_repo->find(m_selectedId);
    if (!cur)
        return;
    const QString name = cur->getName();
    QString text = QStringLiteral("« %1 » sera retiré de la liste et du plan de la clinique.").arg(name);
    if (cur->getOccupancy() > 0)
        text += QStringLiteral("\n\nAttention : %1 animal(aux) y sont encore affecté(s).").arg(cur->getOccupancy());
    if (!Confirm::ask(this, QStringLiteral("Supprimer ce local ?"), text, QStringLiteral("Supprimer")))
        return;
    const int id = m_selectedId;
    if (!m_repo->remove(id)) {
        m_toast->showMessage(QStringLiteral("Suppression non effectuée : ") + m_repo->lastDbError());
        return;
    }
    m_selectedId = 0;
    m_toast->showMessage(QStringLiteral("« %1 » supprimé — le plan a été mis à jour").arg(name));
}

void PageLocaux::modifySelected(const std::function<bool(Local &)> &fn)
{
    const Local *cur = m_repo->find(m_selectedId);
    if (!cur)
        return;
    Local copy = *cur;
    if (fn(copy) && !m_repo->update(copy))
        m_toast->showMessage(QStringLiteral("Modification non effectuée : ") + m_repo->lastDbError());
}

// ================================================================ fonctionnalites avancees

void PageLocaux::openPlan()
{
    if (m_plan) {
        m_plan->raise();
        m_plan->activateWindow();
        return;
    }
    m_plan = new PlanDialog(m_repo, this);
    m_plan->setSelected(m_selectedId);
    connect(m_plan, &PlanDialog::selectRequested, this, [this](int id) { selectRoom(id); });
    m_plan->show();
}

void PageLocaux::openSmartAssign()
{
    SmartAssignDialog dlg(m_repo, this);
    if (dlg.exec() == QDialog::Accepted && dlg.assignedId() > 0) {
        selectRoom(dlg.assignedId());
        m_toast->showMessage(QStringLiteral("Animal affecté à %1 (affectation intelligente)").arg(dlg.assignedName()));
    }
}

void PageLocaux::exportPdf()
{
    const QString def = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                        + "/rapport-nettoyage-" + QDate::currentDate().toString("yyyy-MM-dd") + ".pdf";
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Enregistrer le rapport de nettoyage"),
                                                def, QStringLiteral("Fichier PDF (*.pdf)"));
    if (path.isEmpty())
        return;
    if (!path.endsWith(".pdf", Qt::CaseInsensitive))
        path += ".pdf";
    QString err;
    if (!PdfReport::generate(m_repo->all(), path, &err)) {
        QMessageBox::warning(this, QStringLiteral("ANIWORLD"), err);
        return;
    }
    m_toast->showMessage(QStringLiteral("Rapport PDF généré"));
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
