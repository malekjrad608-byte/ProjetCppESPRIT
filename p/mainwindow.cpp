#include "mainwindow.h"
#include "motion.h"
#include "ui_mainwindow.h"
#include "recordcard.h"
#include "medicalrecorddialog.h"
#include "medicalrecorddetailsdialog.h"
#include "Connection.h"
#include "medicalrecord.h"

#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollArea>
#include <QScrollBar>
#include <QProgressBar>
#include <QFont>
#include <QHeaderView>
#include <QTableWidget>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QStringConverter>
#include <QDate>
#include <QLocale>
#include <QMap>
#include <QSqlQueryModel>
#include <QIcon>
#include <QPixmap>
#include <QStyle>
#include <QtGlobal>
#include <QEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <algorithm>

namespace {
const int kCardMinWidth = 310;
const int kGridSpacing = 16;
const int kCardHeight = 190;
const int kPanelWidth = 360;
const int kPanelGap = 26;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);      // builds everything described in mainwindow.ui

    m_db = Connection::createInstance().isOpen();
    if (m_db)
        actualiser();        // Appel de la méthode afficher() via l'attribut Mtmp
    else
        loadStaticData();    // mode hors ligne
    setupUI();              // wires the widgets from the .ui to the logic
    loadStylesheet();
    refreshView();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------------------------------------------------------- UI setup

void MainWindow::setupUI()
{
    // --- Style ids -------------------------------------------------------
    // In the .ui every widget has a unique name (needed by Qt Designer).
    // The QSS file styles several widgets with the same "#id", so we give
    // those widgets their style id here.
    auto styleId = [](std::initializer_list<QWidget *> widgets, const char *id) {
        for (QWidget *w : widgets)
            w->setObjectName(QString::fromLatin1(id));
    };
    styleId({ui->btnNavDashboard, ui->btnNavServices, ui->btnNavEmployees,
             ui->btnNavStock, ui->btnNavPremises, ui->btnNavRecords}, "navButton");
    styleId({ui->crumbGestion, ui->crumbSep}, "crumb");
    styleId({ui->chipTotal, ui->chipConsultations, ui->chipUpToDate, ui->chipToRenew}, "chip");
    ui->chipTotal->setProperty("tone", "blue");
    ui->chipConsultations->setProperty("tone", "purple");
    ui->chipUpToDate->setProperty("tone", "green");
    ui->chipToRenew->setProperty("tone", "orange");
    styleId({ui->footerLeft, ui->footerDate}, "footerText");
    styleId({ui->btnView}, "primaryButton");
    styleId({ui->btnStatsPanel}, "outlineButton");
    styleId({ui->btnToggle}, "actionToggleButton");
    styleId({ui->btnEdit}, "actionEditButton");
    styleId({ui->btnDelete}, "actionDeleteButton");
    ui->btnView->setObjectName("actionViewButton");
    // Keep btnReset and btnStatsTop object names so their compact styles apply.
    styleId({ui->btnFile}, "topButton");
    styleId({ui->selectedCard, ui->globalCard}, "panelCard");
    styleId({ui->detailLine1, ui->detailLine2, ui->detailLine3, ui->globalText}, "panelMeta");
    styleId({ui->filterSpecies, ui->filterVaccination, ui->sortCombo}, "filterCombo");

    // --- Sidebar ---------------------------------------------------------
    // Logo: uses :/assets/logo.png when present, paw glyph otherwise
    QPixmap logoPix(":/images/logo.png");
    if (!logoPix.isNull())
        ui->logoIcon->setPixmap(logoPix.scaled(52, 52, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        ui->logoIcon->setPixmap(QPixmap(":/icons/paw.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_navButtons = { ui->btnNavDashboard, ui->btnNavServices, ui->btnNavEmployees,
                     ui->btnNavStock, ui->btnNavPremises, ui->btnNavRecords };
    for (QPushButton *btn : std::as_const(m_navButtons)) {
        const QString label = btn->text();
        connect(btn, &QPushButton::clicked, this, [this, label]() { onNavigationClicked(label); });
    }

    // --- Top bar ---------------------------------------------------------
    auto *menu = new QMenu(ui->btnFile);
    menu->addAction("Actualiser", this, &MainWindow::onRefreshClicked);
    menu->addAction("Exporter en CSV…", this, &MainWindow::onExportCsvClicked);
    menu->addSeparator();
    menu->addAction("Quitter", this, &QWidget::close);
    ui->btnFile->setMenu(menu);

    // --- Footer ----------------------------------------------------------
    ui->footerDate->setText(QLocale(QLocale::French).toString(QDate::currentDate(), "d MMMM yyyy"));

    // --- Records page: header & toolbar ----------------------------------
    connect(ui->btnAdd, &QPushButton::clicked, this, &MainWindow::onAddRecordClicked);
    connect(ui->btnStatsTop, &QPushButton::clicked, this, &MainWindow::onShowStatisticsClicked);
    ui->btnStatsTop->setText(QString::fromUtf8("📊  Statistiques"));

    m_chipTotal = ui->chipTotal;
    m_chipConsultations = ui->chipConsultations;
    m_chipUpToDate = ui->chipUpToDate;
    m_chipToRenew = ui->chipToRenew;

    m_searchEdit = ui->searchEdit;
    m_searchEdit->addAction(QIcon(":/icons/search_grey.svg"), QLineEdit::LeadingPosition);
    m_filterSpecies = ui->filterSpecies;
    m_filterVaccination = ui->filterVaccination;
    m_sortCombo = ui->sortCombo;

    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);
    for (QComboBox *c : {m_filterSpecies, m_filterVaccination, m_sortCombo})
        connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterChanged);
    connect(ui->btnReset, &QPushButton::clicked, this, &MainWindow::onResetFiltersClicked);

    // --- Cards area (cards are created and positioned in code) ------------
    m_scroll = ui->cardsScroll;
    m_scroll->viewport()->installEventFilter(this);
    m_cardsContainer = ui->cardsContainer;
    m_emptyLabel = ui->emptyLabel;
    m_emptyLabel->hide();

    // --- Detail panel ----------------------------------------------------
    m_detailPlaceholder = ui->placeholderText;
    m_detailContent = ui->detailContent;
    m_detailTitle = ui->panelTitle;
    m_detailPill = ui->statusPill;
    m_detailLine1 = ui->detailLine1;
    m_detailLine2 = ui->detailLine2;
    m_detailLine3 = ui->detailLine3;
    m_btnView = ui->btnView;
    m_btnToggle = ui->btnToggle;
    m_btnEdit = ui->btnEdit;
    m_btnDelete = ui->btnDelete;

    connect(m_btnView, &QPushButton::clicked, this, &MainWindow::onViewRecordClicked);
    connect(m_btnToggle, &QPushButton::clicked, this, &MainWindow::onToggleVaccinationClicked);
    connect(m_btnEdit, &QPushButton::clicked, this, &MainWindow::onEditRecordClicked);
    connect(m_btnDelete, &QPushButton::clicked, this, &MainWindow::onDeleteRecordClicked);

    // Statistics are opened on demand from the top "Statistiques" button.
    // Keep the right column focused on the selected medical record.
    ui->globalCard->hide();
    m_globalValue = ui->bigValue;
    m_globalBar = ui->globalBar;
    m_globalText = ui->globalText;
    connect(ui->btnStatsPanel, &QPushButton::clicked, this, &MainWindow::onShowStatisticsClicked);
}

void MainWindow::loadStylesheet()
{
    QFile file(":/styles/style.qss");
    if (file.open(QIODevice::ReadOnly))
        qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
}

// ---------------------------------------------------------------- Data

void MainWindow::loadStaticData()
{
    auto add = [this](const QString &animalId, const QString &name, const QString &species,
                      const QString &breed, int age, const QString &sex, const QString &owner,
                      const QString &recordId, const QDate &date, const QString &reason,
                      const QString &diagnosis, const QString &treatment,
                      const QString &vaccinations, const QString &allergies, double weight,
                      const QString &observations, const QString &vaccStatus) {
        MedicalRecord M(recordId, date, reason, diagnosis, animalId, name,
                        species, breed, age, weight, owner, QString());
        M.setSexe(sex);
        M.setTraitement(treatment);
        M.setVaccinations(vaccinations);
        M.setAllergies(allergies);
        M.setObservations(observations);
        M.setStatutVaccination(vaccStatus);
        m_records.append(M);
    };

    add("ANI-001", "Max", "Chien", "Golden Retriever", 5, "Mâle", "Jean Dupont",
        "MED-001", QDate(2026, 9, 12),
        "Visite de routine", "Animal en bonne santé",
        "Aucun traitement nécessaire",
        "Rage, Parvovirose, Distemper", "Aucune connue", 32.5,
        "Excellent état général. Poids stable.",
        "À jour");
    add("ANI-002", "Luna", "Chat", "Siamois", 3, "Femelle", "Marie Martin",
        "MED-002", QDate(2026, 9, 10),
        "Perte d'appétit", "Gastrite légère",
        "Médicament anti-acide, régime alimentaire doux",
        "Rage, Typhus, Coryza", "Poisson", 4.2,
        "Amélioration observée après 48h de traitement.",
        "À jour");
    add("ANI-003", "Rocky", "Chien", "Bulldog", 7, "Mâle", "Pierre Bernard",
        "MED-003", QDate(2026, 9, 8),
        "Boiterie patte arrière", "Arthrose débutante",
        "Anti-inflammatoires, suppléments articulaires",
        "Rage, Parvovirose", "Aucune connue", 28.0,
        "Surveillance recommandée. Exercice modéré.",
        "À renouveler");
    add("ANI-004", "Mimi", "Chat", "Européen", 2, "Femelle", "Sophie Petit",
        "MED-004", QDate(2026, 9, 5),
        "Vaccination annuelle", "Examen préventif",
        "Vaccination Rage, Typhus, Coryza",
        "Rage, Typhus, Coryza", "Aucune connue", 3.8,
        "Vaccination effectuée sans réaction.",
        "À jour");
    add("ANI-005", "Bella", "Lapin", "Lop", 4, "Femelle", "Lucas Dubois",
        "MED-005", QDate(2026, 9, 3),
        "Problèmes dentaires", "Malocclusion dentaire",
        "Lime dentaire, régime alimentaire riche en fibres",
        "Myxomatose", "Aucune connue", 2.1,
        "Suivi dentaire recommandé tous les 3 mois.",
        "À renouveler");
    add("ANI-006", "Simba", "Chat", "Maine Coon", 6, "Mâle", "Emma Laurent",
        "MED-006", QDate(2026, 8, 28),
        "Toux persistante", "Asthme félin",
        "Corticoïdes inhalés, environnement sans poussière",
        "Rage, Typhus, Coryza", "Acariens", 7.5,
        "Condition stable sous traitement.",
        "À jour");
    add("ANI-007", "Toby", "Chien", "Labrador", 8, "Mâle", "Thomas Moreau",
        "MED-007", QDate(2026, 8, 25),
        "Examen de routine", "Surpoids modéré",
        "Régime hypocalorique, exercice quotidien",
        "Rage, Parvovirose, Distemper", "Aucune connue", 42.0,
        "Perte de poids recommandée (objectif: 35kg).",
        "À jour");
    add("ANI-008", "Coco", "Oiseau", "Perroquet", 12, "Femelle", "Chloe Roux",
        "MED-008", QDate(2026, 8, 20),
        "Plumes anormales", "Carence en vitamines",
        "Suppléments vitaminiques, alimentation variée",
        "Aucune requise", "Aucune connue", 0.45,
        "Plumes en amélioration après 2 semaines de traitement.",
        "À jour");
}

// Refresh (Actualiser) : la requête SELECT est exécutée dans Mtmp.afficher()
// (classe MedicalRecord). Ici on lit seulement le modèle retourné pour
// remplir les cartes : aucune requête SQL dans la vue.
void MainWindow::actualiser()
{
    // Les champs sans colonne dans MEDICAL (sexe, traitement, ...) sont gardés en mémoire
    QMap<QString, MedicalRecord> anciens;
    for (const auto &r : m_records)
        anciens.insert(r.getIdDossier(), r);

    QSqlQueryModel *model = Mtmp.afficher();
    m_records.clear();
    for (int i = 0; i < model->rowCount(); ++i) {
        auto val = [model, i](int col) { return model->data(model->index(i, col)); };
        MedicalRecord M(val(0).toString(), val(1).toDate(), val(2).toString(), val(3).toString(),
                        val(4).toString(), val(5).toString(), val(6).toString(), val(7).toString(),
                        val(8).toInt(), val(9).toDouble(), val(10).toString(), val(11).toString());
        if (anciens.contains(M.getIdDossier())) {
            const MedicalRecord &a = anciens[M.getIdDossier()];
            M.setSexe(a.getSexe());
            M.setTraitement(a.getTraitement());
            M.setVaccinations(a.getVaccinations());
            M.setAllergies(a.getAllergies());
            M.setObservations(a.getObservations());
            M.setStatutVaccination(a.getStatutVaccination());
        }
        m_records.append(M);
    }
    delete model;
}

int MainWindow::indexOfRecord(const QString &recordId) const
{
    for (int i = 0; i < m_records.size(); ++i)
        if (m_records[i].getIdDossier() == recordId)
            return i;
    return -1;
}

void MainWindow::applyFiltersAndSort()
{
    const QString text = m_searchEdit->text().trimmed().toLower();
    const QString species = m_filterSpecies->currentIndex() == 0 ? QString() : m_filterSpecies->currentText();
    const QString vacc = m_filterVaccination->currentIndex() == 0 ? QString() : m_filterVaccination->currentText();

    m_filteredRecords.clear();
    for (const auto &r : m_records) {
        if (!text.isEmpty()) {
            const bool hit = r.getIdDossier().toLower().contains(text)
                || r.getIdAnimal().toLower().contains(text)
                || r.getNomAnimal().toLower().contains(text)
                || r.getEspece().toLower().contains(text)
                || r.getRace().toLower().contains(text)
                || r.getProprietaire().toLower().contains(text)
                || r.getDiagnostic().toLower().contains(text);
            if (!hit)
                continue;
        }
        if (!species.isEmpty() && r.getEspece() != species)
            continue;
        if (!vacc.isEmpty() && r.getStatutVaccination() != vacc)
            continue;
        m_filteredRecords.append(r);
    }

    switch (m_sortCombo->currentIndex()) {
    case 1:
        std::sort(m_filteredRecords.begin(), m_filteredRecords.end(),
                  [](const MedicalRecord &a, const MedicalRecord &b) {
                      return a.getNomAnimal().localeAwareCompare(b.getNomAnimal()) < 0; });
        break;
    case 2:
        std::sort(m_filteredRecords.begin(), m_filteredRecords.end(),
                  [](const MedicalRecord &a, const MedicalRecord &b) {
                      return a.getDateConsultation() > b.getDateConsultation(); });
        break;
    case 3:
        std::sort(m_filteredRecords.begin(), m_filteredRecords.end(),
                  [](const MedicalRecord &a, const MedicalRecord &b) {
                      return a.getProprietaire().localeAwareCompare(b.getProprietaire()) < 0; });
        break;
    default:
        break;
    }
}

// ---------------------------------------------------------------- View refresh

void MainWindow::refreshView()
{
    applyFiltersAndSort();

    // Keep the selection only if it is still visible
    bool stillVisible = false;
    for (const auto &r : m_filteredRecords)
        if (r.getIdDossier() == m_selectedId) { stillVisible = true; break; }
    if (!stillVisible)
        m_selectedId.clear();

    rebuildCards();
    updateChips();
    updateDetailPanel();
    updateGlobalPanel();
    layoutUi();
}

void MainWindow::rebuildCards()
{
    for (RecordCard *c : m_cards)
        c->deleteLater();
    m_cards.clear();

    for (const auto &r : m_filteredRecords) {
        auto *card = new RecordCard(r, m_cardsContainer);
        connect(card, &RecordCard::clicked, this, &MainWindow::onCardClicked);
        connect(card, &RecordCard::doubleClicked, this, [this](const QString &id) { viewRecord(id); });
        card->setSelected(r.getIdDossier() == m_selectedId);
        m_cards.append(card);
    }
    m_emptyLabel->setVisible(m_cards.isEmpty());
    relayoutCards();
}

// Cards are placed by hand (no layout): as many columns as fit in the viewport.
void MainWindow::relayoutCards()
{
    const int viewW = m_scroll->viewport()->width();
    const int available = viewW - 10;
    const int cols = qMax(1, (available + kGridSpacing) / (kCardMinWidth + kGridSpacing));
    const int cardW = qMax(100, (available - (cols - 1) * kGridSpacing) / cols);
    const int rows = (m_cards.size() + cols - 1) / cols;

    for (int i = 0; i < m_cards.size(); ++i) {
        const int row = i / cols, col = i % cols;
        m_cards[i]->setGeometry(col * (cardW + kGridSpacing), row * (kCardHeight + kGridSpacing),
                                cardW, kCardHeight);
        m_cards[i]->show();
    }

    const int contentH = m_cards.isEmpty() ? 160 : rows * (kCardHeight + kGridSpacing) - kGridSpacing;
    m_cardsContainer->setGeometry(0, 0, viewW, contentH);
    m_emptyLabel->setGeometry(0, 0, viewW, 160);
}

// Positions every block of the window by hand (the .ui has no layouts).
void MainWindow::layoutUi()
{
    if (!ui || !m_scroll)
        return;

    const int W = width(), H = height();
    const int sidebarW = 290, topH = 68, footerH = 38;
    const int mainW = W - sidebarW;

    // Sidebar (slogan stays glued to the bottom)
    ui->sidebar->setGeometry(0, 0, sidebarW, H);
    ui->sloganSub->setGeometry(18, H - 28 - 18, 254, 22);
    ui->sloganLabel->setGeometry(18, H - 28 - 18 - 26, 254, 26);

    // Main area
    ui->mainContent->setGeometry(sidebarW, 0, mainW, H);

    // Top bar (right-aligned group)
    ui->topBar->setGeometry(0, 0, mainW, topH);
    ui->roleLabel->setGeometry(mainW - 26 - 170, 22, 170, 24);
    ui->btnFile->setGeometry(mainW - 26 - 170 - 14 - 120, 11, 120, 45);
    ui->demoBadge->setGeometry(ui->btnFile->x() - 14 - 210, 10, 210, 48);

    // Footer
    ui->pageFooter->setGeometry(0, H - footerH, mainW, footerH);
    ui->footerDate->setGeometry(mainW - 34 - 160, 8, 160, 16);

    // Page area = records (left) + detail panel (right)
    const int pageTop = topH + 28;
    const int pageW = mainW - 68;
    const int pageH = qMax(200, H - footerH - 14 - pageTop);
    ui->pageArea->setGeometry(34, pageTop, pageW, pageH);

    const int LW = pageW - kPanelWidth - kPanelGap;   // records column width
    ui->recordsPage->setGeometry(0, 0, LW, pageH);

    ui->pageTitle->setGeometry(0, 0, qMax(100, LW - 220), 44);
    ui->pageSubtitle->setGeometry(0, 50, LW, 20);
    ui->btnAdd->setGeometry(LW - 210, 0, 210, 49);

    // Summary chips: keep them on their own row, with a fixed compact height.
    // A smaller font and proportional widths keep the four chips inside the
    // records column even on narrower windows.
    const QList<QLabel *> chips = {ui->chipTotal, ui->chipConsultations,
                                   ui->chipUpToDate, ui->chipToRenew};
    const int chipsY = 104;
    const int chipsH = 34;
    const int chipsGap = 8;
    int naturalTotal = 0;
    for (QLabel *chip : chips)
        naturalTotal += qMax(chip->fontMetrics().horizontalAdvance(chip->text()) + 24,
                             chip->sizeHint().width());
    naturalTotal += chipsGap * (chips.size() - 1);
    const int chipsAvailable = qMax(280, LW - 8);
    const qreal chipScale = naturalTotal > chipsAvailable
                                ? qreal(chipsAvailable - chipsGap * (chips.size() - 1)) /
                                      qMax(1, naturalTotal - chipsGap * (chips.size() - 1))
                                : 1.0;
    int x = 0;
    for (QLabel *chip : chips) {
        const int naturalW = qMax(chip->fontMetrics().horizontalAdvance(chip->text()) + 24,
                                  chip->sizeHint().width());
        const int cw = qMax(58, qRound(naturalW * chipScale));
        chip->setGeometry(x, chipsY, cw, chipsH);
        x += cw + chipsGap;
    }

    // The add and statistics buttons have explicit compact sizes.  Leave a
    // full row gap so Qt style metrics/high-DPI scaling cannot make them touch.
    const int addW = qMin(250, qMax(200, LW / 3));
    ui->btnAdd->setGeometry(LW - addW, 0, addW, 52);
    const int statsW = qMin(158, qMax(140, LW / 4));
    ui->btnStatsTop->setGeometry(LW - statsW, 62, statsW, 40);

    // Search and filters form a separate toolbar, matching the Employees page.
    const int ty = 150;
    ui->searchEdit->setGeometry(0, ty, LW, 42);

    const int y2 = ty + 42 + 10;
    const int filterGap = 8;
    const int resetW = qMin(126, qMax(96, LW / 6));
    const int availableForCombos = qMax(3, LW - resetW - 3 * filterGap);
    const int comboW = availableForCombos / 3;
    const int filterH = 36;
    ui->filterSpecies->setGeometry(0, y2, comboW, filterH);
    ui->filterVaccination->setGeometry(comboW + filterGap, y2, comboW, filterH);
    ui->sortCombo->setGeometry(2 * (comboW + filterGap), y2, comboW, filterH);
    ui->btnReset->setGeometry(LW - resetW, y2, resetW, filterH);
    const int toolbarBottom = y2 + filterH;

    const int scrollTop = toolbarBottom + 16;
    ui->cardsScroll->setGeometry(0, scrollTop, LW, qMax(100, pageH - scrollTop));
    relayoutCards();

    // Detail panel
    layoutDetailPanel();
    ui->detailPanel->move(LW + kPanelGap, 0);
}

void MainWindow::layoutDetailPanel()
{
    const bool has = indexOfRecord(m_selectedId) >= 0;

    if (has) {
        m_detailPill->adjustSize();
        m_detailPill->setGeometry(0, 44, m_detailPill->width() + 8, 28);

        // Four evenly sized, full-width actions; never overlap or touch.
        const int actionX = 0;
        const int actionW = 316;
        const int actionH = 36;
        const int actionGap = 9;
        const int actionY = 208;
        ui->btnView->setText("📋  Consulter le dossier");
        ui->btnEdit->setText("✏️  Modifier");
        ui->btnDelete->setText("🗑  Supprimer le dossier");
        ui->btnView->setGeometry(actionX, actionY, actionW, actionH);
        ui->btnToggle->setGeometry(actionX, actionY + actionH + actionGap, actionW, actionH);
        ui->btnEdit->setGeometry(actionX, actionY + 2 * (actionH + actionGap), actionW, actionH);
        ui->btnDelete->setGeometry(actionX, actionY + 3 * (actionH + actionGap), actionW, actionH);

        // Enough room for all four actions, including the Delete button.
        ui->detailContent->setGeometry(22, 44, 316, 430);
    }
    const int cardH = has ? 474 : 150;
    ui->selectedCard->setGeometry(0, 0, kPanelWidth, cardH);
    // The global statistics card is intentionally not shown permanently.
    // Clicking "Statistiques" opens the dedicated statistics window.
    ui->globalCard->hide();
    ui->detailPanel->resize(kPanelWidth, cardH);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    layoutUi();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (m_scroll && watched == m_scroll->viewport() && event->type() == QEvent::Resize)
        relayoutCards();
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::updateChips()
{
    int upToDate = 0, toRenew = 0;
    for (const auto &r : m_records) {
        if (r.getStatutVaccination() == "À jour") ++upToDate;
        else if (r.getStatutVaccination() == "À renouveler") ++toRenew;
    }
    const int total = m_records.size();
    m_chipTotal->setText(QString("%1 %2").arg(total).arg(total > 1 ? "dossiers" : "dossier"));
    m_chipConsultations->setText(QString("%1 %2").arg(total).arg(total > 1 ? "consultations" : "consultation"));
    m_chipUpToDate->setText(QString("%1 à jour").arg(upToDate));
    m_chipToRenew->setText(QString("%1 à renouveler").arg(toRenew));
}

void MainWindow::updateDetailPanel()
{
    const int idx = indexOfRecord(m_selectedId);
    const bool has = idx >= 0;
    m_detailPlaceholder->setVisible(!has);
    m_detailContent->setVisible(has);
    if (!has)
        return;

    const MedicalRecord &r = m_records[idx];
    const bool upToDate = r.getStatutVaccination() == "À jour";

    m_detailTitle->setText(r.getNomAnimal());
    m_detailPill->setText(r.getStatutVaccination());
    m_detailPill->setProperty("tone", upToDate ? "ok" : "warn");
    m_detailPill->style()->unpolish(m_detailPill);
    m_detailPill->style()->polish(m_detailPill);

    m_detailLine1->setText(QString("%1 · %2 · %3")
        .arg(r.getEspece(), r.getRace(), r.getIdAnimal()));
    m_detailLine2->setText(QString("Dossier %1 · Âge : %2 an%3 · Sexe : %4")
        .arg(r.getIdDossier()).arg(r.getAge()).arg(r.getAge() > 1 ? "s" : "", r.getSexe()));
    m_detailLine3->setText(QString("Propriétaire : %1\nMotif : %2").arg(r.getProprietaire(), r.getMotif()));
    m_btnToggle->setText(upToDate ? "💉  Vaccin à renouveler" : "✅  Marquer à jour");
}

void MainWindow::updateGlobalPanel()
{
    int upToDate = 0;
    for (const auto &r : m_records)
        if (r.getStatutVaccination() == "À jour") ++upToDate;
    const int total = m_records.size();
    const int pct = total ? qRound(100.0 * upToDate / total) : 0;
    m_globalValue->setText(QString("%1 %").arg(pct));
    m_globalBar->setValue(pct);
    m_globalText->setText(QString("%1 dossier%2 à jour sur %3").arg(upToDate).arg(upToDate > 1 ? "s" : "").arg(total));
}

// ---------------------------------------------------------------- Actions

QString MainWindow::nextRecordNumber(const QVector<MedicalRecord> &records)
{
    int maxNum = 0;
    for (const auto &r : records)
        maxNum = qMax(maxNum, r.getIdDossier().section('-', 1).toInt());
    return QString("MED-%1").arg(maxNum + 1, 3, 10, QChar('0'));
}

void MainWindow::viewRecord(const QString &recordId)
{
    const int idx = indexOfRecord(recordId);
    if (idx < 0)
        return;
    MedicalRecordDetailsDialog dialog(this);
    dialog.setMedicalRecord(m_records[idx]);
    dialog.exec();
}

void MainWindow::editRecord(const QString &recordId)
{
    const int idx = indexOfRecord(recordId);
    if (idx < 0)
        return;
    MedicalRecordDialog dialog(this);
    dialog.setMedicalRecord(m_records[idx]);
    if (dialog.exec() == QDialog::Accepted) {
        MedicalRecord M = dialog.getMedicalRecord();
        m_selectedId = M.getIdDossier();

        if (!m_db) {            // mode hors ligne
            m_records[idx] = M;
            refreshView();
            return;
        }

        bool test = M.modifier();   // UPDATE dans la table MEDICAL
        if (test) {
            // Refresh (Actualiser)
            m_records[idx] = M;     // garde les champs affichés seulement (sexe, traitement, ...)
            actualiser();
            refreshView();
            QMessageBox::information(nullptr, QObject::tr("OK"),
                QObject::tr("Modification effectuée\n"
                            "Click Cancel to exit."), QMessageBox::Cancel);
        } else
            QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                QObject::tr("Modification non effectuée.\n"
                            "Click Cancel to exit."), QMessageBox::Cancel);
    }
}

void MainWindow::deleteRecord(const QString &recordId)
{
    const int idx = indexOfRecord(recordId);
    if (idx < 0)
        return;
    const auto reply = QMessageBox::question(this, "Confirmation",
        QString("Êtes-vous sûr de vouloir supprimer le dossier de %1 ?").arg(m_records[idx].getNomAnimal()),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (!m_db) {            // mode hors ligne
            m_records.removeAt(idx);
            m_selectedId.clear();
            refreshView();
            return;
        }

        bool test = Mtmp.supprimer(recordId);   // Appel de supprimer() via l'attribut Mtmp
        if (test) {
            // Refresh (Actualiser)
            m_selectedId.clear();
            actualiser();
            refreshView();
            QMessageBox::information(nullptr, QObject::tr("OK"),
                QObject::tr("Suppression effectuée\n"
                            "Click Cancel to exit."), QMessageBox::Cancel);
        } else
            QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                QObject::tr("Suppression non effectuée.\n"
                            "Click Cancel to exit."), QMessageBox::Cancel);
    }
}

void MainWindow::onNavigationClicked(const QString &name)
{
    if (name == "Dossiers médicaux")
        return; // already here
    QMessageBox::information(this, "Navigation", QString("%1 - Non implémenté").arg(name));
}

void MainWindow::onAddRecordClicked()
{
    MedicalRecordDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Objet MedicalRecord instancié avec les informations saisies dans le formulaire
        MedicalRecord M = dialog.getMedicalRecord();
        if (M.getIdDossier().isEmpty() || indexOfRecord(M.getIdDossier()) >= 0)
            M.setIdDossier(nextRecordNumber(m_records));

        if (!m_db) {            // mode hors ligne
            m_records.append(M);
            m_selectedId = M.getIdDossier();
            refreshView();
            return;
        }

        bool test = M.ajouter();   // Insérer l'objet dans la table MEDICAL et récupérer le retour de query.exec()
        if (test) {                // Si requête exécutée --> QMessageBox::information
            // Refresh (Actualiser)
            m_records.append(M);   // garde les champs affichés seulement (sexe, traitement, ...)
            actualiser();
            m_selectedId = M.getIdDossier();
            refreshView();
            QMessageBox::information(nullptr, QObject::tr("OK"),
                QObject::tr("Ajout effectué\n"
                            "Click Cancel to exit."), QMessageBox::Cancel);
        } else                     // Si requête non exécutée ==> QMessageBox::critical
            QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                QObject::tr("Ajout non effectué.\n"
                            "Click Cancel to exit."), QMessageBox::Cancel);
    }
}

void MainWindow::onViewRecordClicked()   { viewRecord(m_selectedId); }
void MainWindow::onEditRecordClicked()   { editRecord(m_selectedId); }
void MainWindow::onDeleteRecordClicked() { deleteRecord(m_selectedId); }

void MainWindow::onToggleVaccinationClicked()
{
    const int idx = indexOfRecord(m_selectedId);
    if (idx < 0)
        return;
    // Le statut de vaccination n'a pas de colonne dans MEDICAL : il change en mémoire seulement.
    const QString status = m_records[idx].getStatutVaccination();
    m_records[idx].setStatutVaccination(status == "À jour" ? "À renouveler" : "À jour");
    refreshView();
}

void MainWindow::onRefreshClicked()
{
    refreshView();
}

void MainWindow::onFilterChanged()
{
    refreshView();
}

void MainWindow::onResetFiltersClicked()
{
    const QSignalBlocker b1(m_searchEdit), b2(m_filterSpecies), b3(m_filterVaccination), b4(m_sortCombo);
    m_searchEdit->clear();
    m_filterSpecies->setCurrentIndex(0);
    m_filterVaccination->setCurrentIndex(0);
    m_sortCombo->setCurrentIndex(0);
    refreshView();
}

void MainWindow::onCardClicked(const QString &recordId)
{
    m_selectedId = recordId;
    for (RecordCard *c : m_cards)
        c->setSelected(c->recordId() == recordId);
    updateDetailPanel();
    layoutDetailPanel();
}

void MainWindow::onShowStatisticsClicked()
{
    QMap<QString, QPair<int, QPair<int, int>>> bySpecies;
    int upToDate = 0, toRenew = 0;
    for (const auto &r : m_records) {
        auto &entry = bySpecies[r.getEspece()];
        ++entry.first;
        if (r.getStatutVaccination() == "À jour") {
            ++upToDate;
            ++entry.second.first;
        } else {
            ++toRenew;
            ++entry.second.second;
        }
    }
    const int total = m_records.size();
    const int percent = total ? qRound(100.0 * upToDate / total) : 0;

    // Dedicated statistics window styled like the Employés statistics window.
    // It only reads m_records; no medical-record behavior is changed.
    QDialog dialog(this);
    dialog.setObjectName("statisticsDialog");
    dialog.setWindowTitle("Statistiques des dossiers médicaux");
    dialog.setMinimumSize(900, 650);
    dialog.resize(1000, 760);

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(30, 24, 30, 24);
    root->setSpacing(18);

    auto *heading = new QHBoxLayout;
    auto *titleBox = new QVBoxLayout;
    titleBox->setSpacing(5);
    auto *title = new QLabel("Statistiques des dossiers médicaux");
    title->setObjectName("statisticsTitle");
    auto *subtitle = new QLabel("Suivi des dossiers, espèces et vaccinations");
    subtitle->setObjectName("statisticsSubtitle");
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);
    heading->addLayout(titleBox);
    heading->addStretch();
    auto *logo = new QLabel;
    QPixmap logoPix(":/assets/logo.png");
    if (!logoPix.isNull())
        logo->setPixmap(logoPix.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setObjectName("statisticsLogo");
    heading->addWidget(logo);
    root->addLayout(heading);

    auto *metrics = new QHBoxLayout;
    metrics->setSpacing(14);
    auto addMetric = [&](const QString &caption, const QString &value, const QString &tone, const QString &iconText) {
        auto *card = new QFrame;
        card->setObjectName("statisticsMetric");
        card->setProperty("tone", tone);
        auto *row = new QHBoxLayout(card);
        row->setContentsMargins(18, 14, 18, 14);
        row->setSpacing(12);
        auto *ico = new QLabel(iconText);
        ico->setObjectName("statisticsMetricIcon");
        auto *box = new QVBoxLayout;
        box->setSpacing(3);
        auto *cap = new QLabel(caption);
        cap->setObjectName("statisticsMetricCaption");
        auto *val = new QLabel(value);
        val->setObjectName("statisticsMetricValue");
        box->addWidget(cap);
        box->addWidget(val);
        row->addWidget(ico);
        row->addLayout(box, 1);
        metrics->addWidget(card, 1);
    };
    addMetric("Dossiers", QString::number(total), "blue", "▣");
    addMetric("Vaccins à jour", QString::number(upToDate), "green", "✓");
    addMetric("À renouveler", QString::number(toRenew), "red", "×");
    addMetric("Taux à jour", QString("%1 %").arg(percent), "orange", "↗");
    root->addLayout(metrics);

    auto *sectionTitle = new QLabel("Répartition et état des dossiers");
    sectionTitle->setObjectName("statisticsSectionTitle");
    root->addWidget(sectionTitle);

    auto *table = new QTableWidget;
    table->setObjectName("statisticsTable");
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Espèce", "Dossiers", "Vaccins à jour", "À renouveler", "Taux à jour"});
    table->setRowCount(bySpecies.size());
    table->setAlternatingRowColors(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int col = 1; col < 4; ++col)
        table->horizontalHeader()->setSectionResizeMode(col, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    int row = 0;
    for (auto it = bySpecies.cbegin(); it != bySpecies.cend(); ++it, ++row) {
        auto *species = new QTableWidgetItem(it.key());
        species->setFont(QFont("Segoe UI", 10, QFont::DemiBold));
        table->setItem(row, 0, species);
        table->setItem(row, 1, new QTableWidgetItem(QString::number(it.value().first)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(it.value().second.first)));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(it.value().second.second)));
        for (int col = 1; col < 4; ++col)
            table->item(row, col)->setTextAlignment(Qt::AlignCenter);
        const int speciesTotal = it.value().first;
        const int speciesUpToDate = it.value().second.first;
        const int speciesPercent = speciesTotal ? qRound(100.0 * speciesUpToDate / speciesTotal) : 0;
        auto *rate = new QProgressBar;
        rate->setObjectName("statisticsRateBar");
        rate->setRange(0, 100);
        rate->setValue(speciesPercent);
        rate->setFormat(QString("%1 %").arg(speciesPercent));
        rate->setTextVisible(true);
        rate->setMinimumHeight(24);
        table->setCellWidget(row, 4, rate);
        table->setRowHeight(row, 52);
    }
    root->addWidget(table, 1);

    auto *summary = new QFrame;
    summary->setObjectName("statisticsSummary");
    auto *summaryLayout = new QHBoxLayout(summary);
    summaryLayout->setContentsMargins(18, 12, 18, 12);
    auto *summaryText = new QLabel(QString("Dossiers : %1    •    Vaccins à jour : %2    •    À renouveler : %3    •    Taux : %4 %")
                                      .arg(total).arg(upToDate).arg(toRenew).arg(percent));
    summaryText->setObjectName("statisticsSummaryText");
    summaryLayout->addWidget(summaryText);
    root->addWidget(summary);

    auto *actions = new QHBoxLayout;
    actions->addStretch();
    auto *close = new QPushButton("Fermer");
    close->setObjectName("statisticsClose");
    close->setCursor(Qt::PointingHandCursor);
    close->setMinimumSize(180, 48);
    actions->addWidget(close);
    root->addLayout(actions);
    connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}

void MainWindow::onExportCsvClicked()
{
    const QString path = QFileDialog::getSaveFileName(this, "Exporter les dossiers",
                                                      "dossiers_medicaux.csv", "CSV (*.csv)");
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export", "Impossible d'écrire le fichier.");
        return;
    }
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out.setGenerateByteOrderMark(true);
    auto esc = [](QString s) { s.replace('"', "\"\""); return '"' + s + '"'; };
    out << "ID dossier;ID animal;Nom;Espèce;Race;Âge;Sexe;Propriétaire;Date;Motif;Diagnostic;Traitement;Vaccinations;Allergies;Poids;Statut vaccination\n";
    for (const auto &r : m_filteredRecords) {
        out << esc(r.getIdDossier()) << ';' << esc(r.getIdAnimal()) << ';' << esc(r.getNomAnimal()) << ';'
            << esc(r.getEspece()) << ';' << esc(r.getRace()) << ';' << r.getAge() << ';'
            << esc(r.getSexe()) << ';' << esc(r.getProprietaire()) << ';'
            << r.getDateConsultation().toString("dd/MM/yyyy") << ';' << esc(r.getMotif()) << ';'
            << esc(r.getDiagnostic()) << ';' << esc(r.getTraitement()) << ';'
            << esc(r.getVaccinations()) << ';' << esc(r.getAllergies()) << ';'
            << r.getPoids() << ';' << esc(r.getStatutVaccination()) << '\n';
    }
}
