#include "ui_integrated_pages.h"
#include "../ui/design.h"
#include "../database/oracleconnection.h"
#include "../ui/motion.h"
#include <QPainter>
#include <QHideEvent>
#include "../widgets/animatedbutton.h"
#include "pagelocaux.h"
#include "alerts.h"
#include "alertsdialog.h"
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
    setObjectName("locauxPage");
    Theme::animations=Motion::instance().enabled();
    connect(&Motion::instance(), &Motion::changed, this, [](bool enabled) { Theme::animations=enabled; });
    m_toast = new Toast(this);
    connect(repo,&LocalRepository::storageError,this,[this](const QString &error) {
        QMessageBox::warning(this,"Sauvegarde des locaux",error);
    });
    m_alertTimer = new QTimer(this);

    setupUi();

    // le plan et la liste se mettent a jour a chaque changement des donnees
    connect(m_repo, &LocalRepository::changed, this, [this] {
        m_miniMap->setLocaux(m_repo->all());
        refresh(false);
    });

    // alertes : re-evaluees toutes les 20 secondes (le temps passe !)
    connect(m_alertTimer, &QTimer::timeout, this, [this] {
        computeAlerts(isVisible());
        applyAlerts();
    });
    m_alertTimer->start(20000);

    m_miniMap->setLocaux(m_repo->all());
    refresh(true);
}

// === MODULE LOCAUX : LIAISON AU FICHIER MAINWINDOW.UI ===
void PageLocaux::setupUi() {
    Ui::LocauxPage ui;ui.setupUi(this);
    m_bar=ui.Locaux_m_bar;
    m_btnAdd=ui.Locaux_m_btnAdd;
    m_btnAlerts=ui.Locaux_m_btnAlerts;
    m_btnCards=ui.Locaux_m_btnCards;
    m_btnList=ui.Locaux_m_btnList;
    m_btnMap=ui.Locaux_m_btnMap;
    m_btnPdf=ui.Locaux_m_btnPdf;
    m_btnSmart=ui.Locaux_m_btnSmart;
    m_btnStats=ui.Locaux_m_btnStats;
    m_cardScroll=ui.Locaux_m_cardScroll;
    m_chipAvail=ui.Locaux_m_chipAvail;
    m_chipDirty=ui.Locaux_m_chipDirty;
    m_chipOcc=ui.Locaux_m_chipOcc;
    m_chipOos=ui.Locaux_m_chipOos;
    m_count=ui.Locaux_m_count;
    m_detail=ui.Locaux_m_detail;
    m_empty=ui.Locaux_m_empty;
    m_globalText=ui.Locaux_m_globalText;
    m_grid=ui.Locaux_m_grid;
    m_miniMap=ui.Locaux_m_miniMap;
    m_pct=ui.Locaux_m_pct;
    m_queryHint=ui.Locaux_m_queryHint;
    m_search=ui.Locaux_m_search;
    m_sortCombo=ui.Locaux_m_sortCombo;
    m_table=ui.Locaux_m_table;
    m_views=ui.Locaux_m_views;
    m_search->addAction(Icons::icon("search",Theme::Muted,16),QLineEdit::LeadingPosition);
    m_table->setColumnCount(7);m_table->setHorizontalHeaderLabels({"ID","Local","Type","Étage","Occupation","État","Nettoyage"});
    if(OracleConnection::isOracle()) {
        m_table->setColumnCount(8);m_table->setHorizontalHeaderLabels({"ID local","Nom local","Type","Étage","Capacité","Spécialité","État","ID service"});
        for(auto *b:{m_chipDirty,m_btnPdf,m_btnSmart,m_btnStats,m_btnAlerts})b->hide();
        m_bar->parentWidget()->hide();
    }
    m_table->verticalHeader()->hide();m_table->verticalHeader()->setDefaultSectionSize(42);
    m_table->horizontalHeader()->setHighlightSections(false);m_table->horizontalHeader()->setMinimumHeight(40);
    m_table->horizontalHeader()->setSectionsClickable(true);m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);m_table->setColumnWidth(0,70);
    m_queryHint->hide();m_empty->hide();
    connect(m_chipAvail, &QPushButton::clicked, this, [this] { toggleChip(int(SmartSearch::Kw::Available)); });
    connect(m_chipOcc, &QPushButton::clicked, this, [this] { toggleChip(int(SmartSearch::Kw::Occupied)); });
    connect(m_chipOos, &QPushButton::clicked, this, [this] { toggleChip(int(SmartSearch::Kw::OutOfService)); });
    connect(m_chipDirty, &QPushButton::clicked, this, [this] { toggleChip(int(SmartSearch::Kw::Dirty)); });
    connect(m_btnAlerts, &QPushButton::clicked, this, [this] { openAlerts(); });
    connect(m_btnSmart, &QPushButton::clicked, this, [this] { openSmartAssign(); });
    connect(m_btnAdd, &QPushButton::clicked, this, [this] { addLocal(); });
    connect(m_btnPdf, &QPushButton::clicked, this, [this] { exportPdf(); });
    connect(m_btnMap, &QPushButton::clicked, this, [this] { openPlan(); });
    connect(m_btnStats, &QPushButton::clicked, this, [this] {
        StatsDialog dlg(m_repo->all(), this);
        dlg.exec();
    });
    connect(m_search, &QLineEdit::textChanged, this, [this] { refresh(true); });
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { refresh(true); });
    connect(ui.locauxResetFilters, &QPushButton::clicked, this, [this] { resetFilters(); });
    connect(m_btnCards, &QPushButton::clicked, this, [this] { setView(0); });
    connect(m_btnList, &QPushButton::clicked, this, [this] { setView(1); });
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
    connect(ui.locauxEnlargeMap, &QPushButton::clicked, this, [this] { openPlan(); });
    connect(m_miniMap, &ClinicMap::roomClicked, this, [this](int id) { onCardClicked(id); });
    connect(m_detail, &DetailPanel::assignRequested, this, [this] {
        modifySelected([this](Local &l) {
            if (l.outOfService || l.occupancy >= l.capacity)
                return false;
            l.occupancy++;
            m_toast->showMessage(QStringLiteral("Animal affecté à ") + l.name);
            return true;
        });
    });
    connect(m_detail, &DetailPanel::freeRequested, this, [this] {
        modifySelected([this](Local &l) {
            if (l.occupancy <= 0)
                return false;
            l.occupancy--;                       // un animal à la fois (-1)
            if (l.occupancy == 0) {              // dernier animal parti : le local est vide -> à nettoyer
                l.markToClean();
                m_toast->showMessage(l.name + QStringLiteral(" est vide — à nettoyer"));
            } else {
                m_toast->showMessage(QStringLiteral("Animal retiré de %1 (%2 / %3)")
                                         .arg(l.name).arg(l.occupancy).arg(l.capacity));
            }
            return true;
        });
    });
    connect(m_detail, &DetailPanel::cleanRequested, this, [this] {
        modifySelected([this](Local &l) {
            switch (l.cleaning) {
            case CleaningState::Clean:
                l.markToClean();
                m_toast->showMessage(QStringLiteral("Signalé à nettoyer"));
                break;
            case CleaningState::ToClean:
                l.startCleaning();
                m_toast->showMessage(QStringLiteral("Nettoyage démarré"));
                break;
            case CleaningState::InProgress:
                l.markClean();
                m_toast->showMessage(QStringLiteral("Local propre"));
                break;
            }
            return true;
        });
    });
    connect(m_detail, &DetailPanel::editRequested, this, [this] { editSelected(); });
    connect(m_detail, &DetailPanel::deleteRequested, this, [this] { deleteSelected(); });
}












// ================================================================ donnees affichees

QList<Local> PageLocaux::filteredSorted() const
{
    const SmartQuery q = SmartSearch::parse(m_search->text());

    QList<Local> out;
    for (const Local &l : m_repo->all())
        if (q.matches(l, m_alertLevel.value(l.id)))
            out.append(l);

    QCollator coll;
    coll.setNumericMode(true);
    coll.setCaseSensitivity(Qt::CaseInsensitive);
    switch (m_sortCombo->currentIndex()) {
    case 1:     // nom A -> Z
        std::sort(out.begin(), out.end(), [&](const Local &a, const Local &b) {
            return coll.compare(a.name, b.name) < 0;
        });
        break;
    case 2:     // remplissage (taux d'occupation)
        std::stable_sort(out.begin(), out.end(), [](const Local &a, const Local &b) {
            const double ra = a.capacity > 0 ? double(a.occupancy) / a.capacity : 0.0;
            const double rb = b.capacity > 0 ? double(b.occupancy) / b.capacity : 0.0;
            return ra > rb;
        });
        break;
    default:    // priorite : alertes > a nettoyer (urgent d'abord, le plus ancien d'abord) > remplissage
        std::stable_sort(out.begin(), out.end(), [this](const Local &a, const Local &b) {
            const int la = m_alertLevel.value(a.id), lb = m_alertLevel.value(b.id);
            if (la != lb)
                return la > lb;
            const bool da = a.cleaning == CleaningState::ToClean, db = b.cleaning == CleaningState::ToClean;
            if (da != db)
                return da;
            if (da && a.isUrgentType() != b.isUrgentType())
                return a.isUrgentType();
            if (da && a.dirtyMinutes() != b.dirtyMinutes())
                return a.dirtyMinutes() > b.dirtyMinutes();
            const double ra = a.capacity > 0 ? double(a.occupancy) / a.capacity : 0.0;
            const double rb = b.capacity > 0 ? double(b.occupancy) / b.capacity : 0.0;
            return ra > rb;
        });
        break;
    }
    return out;
}

void PageLocaux::refresh(bool playIntro)
{
    computeAlerts(isVisible());
    if (playIntro) {
        m_tableSortCol = -1;
        m_table->horizontalHeader()->setSortIndicatorShown(false);
    }

    const QList<Local> list = filteredSorted();

    QList<RoomCard *> cards;
    int i = 0;
    for (const Local &l : list) {
        auto *c = new RoomCard(l);
        c->setSelected(l.id == m_selectedId);
        c->setAlertLevel(m_alertLevel.value(l.id));
        connect(c, &RoomCard::clicked, this, &PageLocaux::onCardClicked);
        if (playIntro)
            c->playIntro(qMin(i, 14) * 35);
        cards.append(c);
        ++i;
    }
    m_grid->setCards(cards);
    updateTable(list);
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
    updateGlobal();
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
        idItem->setData(Qt::DisplayRole, l.id);
        idItem->setData(Qt::UserRole, l.id);
        idItem->setTextAlignment(Qt::AlignCenter);

        auto *name = new QTableWidgetItem(Icons::icon(l.iconName(), Theme::Teal, 18), "  " + l.name);
        QFont bold = name->font();
        bold.setBold(true);
        name->setFont(bold);

        auto *type = new QTableWidgetItem(l.type);
        auto *floor = new QTableWidgetItem(l.floorText());
        auto *occ = new QTableWidgetItem(OracleConnection::isOracle()?QString::number(l.capacity):QStringLiteral("%1 / %2").arg(l.occupancy).arg(l.capacity));

        QColor bg, fg;
        auto *state = new QTableWidgetItem(l.stateText());
        Theme::stateColors(l.state(), bg, fg);
        state->setForeground(fg);
        state->setFont(bold);

        auto *clean = new QTableWidgetItem(l.cleaningText());
        Theme::cleaningColors(l.cleaning, bg, fg);
        clean->setForeground(fg);
        clean->setFont(bold);

        m_table->setItem(r, 0, idItem);
        m_table->setItem(r, 1, name);
        m_table->setItem(r, 2, type);
        m_table->setItem(r, 3, floor);
        m_table->setItem(r, 4, occ);
        if(OracleConnection::isOracle()) {
            m_table->setItem(r,5,new QTableWidgetItem(l.specialty));m_table->setItem(r,6,state);
            m_table->setItem(r,7,new QTableWidgetItem(QString::number(l.serviceId)));delete clean;
        } else {m_table->setItem(r,5,state);m_table->setItem(r,6,clean);}
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
        }
        if (l.cleaning == CleaningState::ToClean)
            ++dirty;
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

void PageLocaux::updateGlobal()
{
    int cap = 0, occ = 0;
    for (const Local &l : m_repo->all()) {
        if (l.outOfService)
            continue;
        cap += l.capacity;
        occ += l.occupancy;
    }
    const int pct = cap > 0 ? qRound(100.0 * occ / cap) : 0;
    m_pct->setText(QStringLiteral("%1 %").arg(pct));
    m_globalText->setText(QStringLiteral("%1 animaux pour %2 places disponibles").arg(occ).arg(cap));

    Motion::progressTo(m_bar,pct);
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
    m_detail->setLocal(l, l ? m_alertTexts.value(m_selectedId) : QStringList());
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
    if (id<=0) return;
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
    if (!m_repo->update(dlg.result())) return;
    m_toast->showMessage(QStringLiteral("Local modifié"));
}

void PageLocaux::deleteSelected()
{
    const Local *cur = m_repo->find(m_selectedId);
    if (!cur)
        return;
    const QString name = cur->name;
    QString text = QStringLiteral("« %1 » sera retiré de la liste et du plan de la clinique.").arg(name);
    if (cur->occupancy > 0 && !OracleConnection::isOracle())
        text += QStringLiteral("\n\nAttention : %1 animal(aux) y sont encore affecté(s).").arg(cur->occupancy);
    if (!Confirm::ask(this, QStringLiteral("Supprimer ce local ?"), text, QStringLiteral("Supprimer")))
        return;
    const int id = m_selectedId;
    m_selectedId = 0;
    if (!m_repo->remove(id)) return;
    m_toast->showMessage(QStringLiteral("« %1 » supprimé — le plan a été mis à jour").arg(name));
}

void PageLocaux::modifySelected(const std::function<bool(Local &)> &fn)
{
    const Local *cur = m_repo->find(m_selectedId);
    if (!cur)
        return;
    Local copy = *cur;
    if (fn(copy))
        if (!m_repo->update(copy)) return;
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

void PageLocaux::openAlerts()
{
    AlertsDialog dlg(m_repo->all(), this);
    const bool accepted = (dlg.exec() == QDialog::Accepted);
    computeAlerts(false);     // le seuil a peut-etre change
    applyAlerts();
    if (accepted && dlg.selectedLocalId() > 0)
        selectRoom(dlg.selectedLocalId());
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

// ================================================================ alertes automatiques

void PageLocaux::computeAlerts(bool notify)
{
    const QList<Alert> alerts = AlertEngine::evaluate(m_repo->all());
    m_alertLevel.clear();
    m_alertTexts.clear();

    QSet<QString> keys;
    QStringList newTitles;
    int crit = 0, warn = 0;
    for (const Alert &a : alerts) {
        keys.insert(a.key);
        if (a.level == Alert::Critical)
            ++crit;
        else if (a.level == Alert::Warning)
            ++warn;
        if (a.localId > 0) {
            m_alertLevel[a.localId] = qMax(m_alertLevel.value(a.localId), int(a.level));
            m_alertTexts[a.localId] << a.shortText;
        }
        if (a.level >= Alert::Warning && !m_knownAlertKeys.contains(a.key))
            newTitles << a.title;
    }

    if (notify) {
        if (!m_alertsReady) {
            if (crit + warn > 0)
                m_toast->showMessage(QStringLiteral("%1 alerte(s) à traiter — clique sur « Alertes »").arg(crit + warn));
        } else if (!newTitles.isEmpty()) {
            QString msg = QStringLiteral("Nouvelle alerte : ") + newTitles.first();
            if (newTitles.size() > 1)
                msg += QStringLiteral(" (+%1)").arg(newTitles.size() - 1);
            m_toast->showMessage(msg);
        }
    }
    m_alertsReady = true;
    m_knownAlertKeys = keys;

    // bouton "Alertes" : rouge s'il y a du critique, orange sinon
    const int n = crit + warn;
    m_btnAlerts->setText(n > 0 ? QStringLiteral("Alertes (%1)").arg(n) : QStringLiteral("Alertes"));
    const QString level = crit > 0 ? QStringLiteral("critical") : warn > 0 ? QStringLiteral("warning") : QStringLiteral("none");
    if (m_btnAlerts->property("alert").toString() != level) {
        m_btnAlerts->setProperty("alert", level);
        const QColor ic = crit > 0 ? Theme::Red : warn > 0 ? Theme::OrangeText : Theme::TealDark;
        m_btnAlerts->setIcon(Icons::icon("bell", ic, 16));
        m_btnAlerts->style()->unpolish(m_btnAlerts);
        m_btnAlerts->style()->polish(m_btnAlerts);
    }
}

void PageLocaux::applyAlerts()
{
    for (RoomCard *c : m_grid->cards())
        c->setAlertLevel(m_alertLevel.value(c->localId()));
    showDetail();
}

void PageLocaux::focusSearch() { m_search->setFocus(); m_search->selectAll(); }
void PageLocaux::refreshPage() { if(OracleConnection::isOracle()) m_repo->reload(); refresh(false); }
void PageLocaux::paintEvent(QPaintEvent *) { QPainter p(this); Design::paintWorkspace(p,rect()); }
void PageLocaux::hideEvent(QHideEvent *event) { if(m_plan) m_plan->close(); QWidget::hideEvent(event); }
