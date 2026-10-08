#include "mainwindow.h"
#include "src/ui/motion.h"
#include "ui_mainwindow.h"
#include "recordcard.h"
#include "medicalrecorddialog.h"
#include "medicalrecorddetailsdialog.h"

#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollArea>
#include <QScrollBar>
#include <QProgressBar>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QStringConverter>
#include <QDate>
#include <QLocale>
#include <QMap>
#include <QIcon>
#include <QPixmap>
#include <QStyle>
#include <QtGlobal>
#include <QEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <algorithm>

namespace {
const int kCardMinWidth = 330;
const int kGridSpacing = 16;
const int kCardHeight = 186;
const int kPanelWidth = 360;
const int kPanelGap = 26;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);      // builds everything described in mainwindow.ui

    loadStaticData();
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
    styleId({ui->footerLeft, ui->footerDate}, "footerText");
    styleId({ui->btnAdd, ui->btnView}, "primaryButton");
    styleId({ui->btnStatsTop, ui->btnStatsPanel}, "outlineButton");
    styleId({ui->btnToggle}, "actionToggleButton");
    styleId({ui->btnEdit}, "actionEditButton");
    styleId({ui->btnDelete}, "actionDeleteButton");
    ui->btnView->setObjectName("actionViewButton");
    styleId({ui->btnReset}, "linkButton");
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

    // --- Global vaccination card ------------------------------------------
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
        AnimalInfo a;
        a.animalId = animalId; a.name = name; a.species = species; a.breed = breed;
        a.age = age; a.sex = sex; a.owner = owner;
        MedicalInfo m;
        m.recordId = recordId; m.consultationDate = date; m.reason = reason;
        m.diagnosis = diagnosis; m.treatment = treatment; m.vaccinations = vaccinations;
        m.allergies = allergies; m.weight = weight; m.observations = observations;
        m.vaccinationStatus = vaccStatus;
        m_records.append(MedicalRecord(a, m));
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

int MainWindow::indexOfRecord(const QString &recordId) const
{
    for (int i = 0; i < m_records.size(); ++i)
        if (m_records[i].medical.recordId == recordId)
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
            const bool hit = r.medical.recordId.toLower().contains(text)
                || r.animal.animalId.toLower().contains(text)
                || r.animal.name.toLower().contains(text)
                || r.animal.species.toLower().contains(text)
                || r.animal.breed.toLower().contains(text)
                || r.animal.owner.toLower().contains(text)
                || r.medical.diagnosis.toLower().contains(text);
            if (!hit)
                continue;
        }
        if (!species.isEmpty() && r.animal.species != species)
            continue;
        if (!vacc.isEmpty() && r.medical.vaccinationStatus != vacc)
            continue;
        m_filteredRecords.append(r);
    }

    switch (m_sortCombo->currentIndex()) {
    case 1:
        std::sort(m_filteredRecords.begin(), m_filteredRecords.end(),
                  [](const MedicalRecord &a, const MedicalRecord &b) {
                      return a.animal.name.localeAwareCompare(b.animal.name) < 0; });
        break;
    case 2:
        std::sort(m_filteredRecords.begin(), m_filteredRecords.end(),
                  [](const MedicalRecord &a, const MedicalRecord &b) {
                      return a.medical.consultationDate > b.medical.consultationDate; });
        break;
    case 3:
        std::sort(m_filteredRecords.begin(), m_filteredRecords.end(),
                  [](const MedicalRecord &a, const MedicalRecord &b) {
                      return a.animal.owner.localeAwareCompare(b.animal.owner) < 0; });
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
        if (r.medical.recordId == m_selectedId) { stillVisible = true; break; }
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
        card->setSelected(r.medical.recordId == m_selectedId);
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
    ui->sloganSub->setGeometry(18, H - 22 - 18, 254, 18);
    ui->sloganLabel->setGeometry(18, H - 22 - 18 - 4 - 18, 254, 18);

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

    // Chips: width follows the text
    int x = 0;
    for (QLabel *chip : {ui->chipTotal, ui->chipConsultations, ui->chipUpToDate, ui->chipToRenew}) {
        const int cw = qMax(chip->sizeHint().width(),
                            chip->fontMetrics().horizontalAdvance(chip->text()) + 36) + 8;
        chip->setGeometry(x, 90, cw, 39);
        x += cw + 10;
    }
    // Toolbar: the Statistics button stays on the SAME ROW as the summary chips,
    // like the original layout. The search field remains on the row below.
    // This avoids pushing Statistics too far down into the filters area.
    const int statsW = 130;
    const int statsY = 90;
    ui->btnStatsTop->setGeometry(LW - statsW, statsY, statsW, 45);

    // Search gets the complete row below the chips/statistics.
    const int ty = 147;
    ui->searchEdit->setGeometry(0, ty, LW, 45);

    // Filters stay on a separate row, so every control has enough breathing room.
    const int y2 = ty + 45 + 10;
    ui->filterSpecies->setGeometry(0, y2, 175, 45);
    ui->filterVaccination->setGeometry(187, y2, 160, 45);
    ui->sortCombo->setGeometry(359, y2, 170, 45);
    ui->btnReset->setGeometry(qMax(0, LW - 110), y2 + 5, 110, 34);
    const int toolbarBottom = y2 + 45;

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

        // Compact action layout matching the reference: one primary button,
        // two small buttons side-by-side, then one danger button.
        const int actionX = 0;
        const int actionW = 316;
        const int actionH = 36;
        const int actionGap = 9;
        const int halfW = (actionW - actionGap) / 2;
        const int actionY = 208;

        // Clean, reference-style action layout:
        // 1) full-width primary button
        // 2) two separated half-width buttons
        // 3) full-width danger button
        // Every row has a real gap so the buttons never touch/overlap.
        ui->btnView->setGeometry(actionX, actionY, actionW, actionH);
        ui->btnToggle->setGeometry(actionX, actionY + actionH + actionGap, halfW, actionH);
        ui->btnEdit->setGeometry(actionX + halfW + actionGap, actionY + actionH + actionGap, halfW, actionH);
        ui->btnDelete->setGeometry(actionX, actionY + 2 * (actionH + actionGap), actionW, actionH);

        // Give the action block enough room; do not clip the last button.
        ui->detailContent->setGeometry(22, 44, 316, 390);
    }
    const int cardH = has ? 434 : 126;
    ui->selectedCard->setGeometry(0, 0, kPanelWidth, cardH);
    ui->globalCard->setGeometry(0, cardH + 18, kPanelWidth, 215);
    ui->detailPanel->resize(kPanelWidth, cardH + 18 + 215);
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
        if (r.medical.vaccinationStatus == "À jour") ++upToDate;
        else if (r.medical.vaccinationStatus == "À renouveler") ++toRenew;
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
    const bool upToDate = r.medical.vaccinationStatus == "À jour";

    m_detailTitle->setText(r.animal.name);
    m_detailPill->setText(r.medical.vaccinationStatus);
    m_detailPill->setProperty("tone", upToDate ? "ok" : "warn");
    m_detailPill->style()->unpolish(m_detailPill);
    m_detailPill->style()->polish(m_detailPill);

    m_detailLine1->setText(QString("%1 · %2 · %3")
        .arg(r.animal.species, r.animal.breed, r.animal.animalId));
    m_detailLine2->setText(QString("Dossier %1 · Âge : %2 an%3 · Sexe : %4")
        .arg(r.medical.recordId).arg(r.animal.age).arg(r.animal.age > 1 ? "s" : "", r.animal.sex));
    m_detailLine3->setText(QString("Propriétaire : %1\nMotif : %2").arg(r.animal.owner, r.medical.reason));
    m_btnToggle->setText(upToDate ? "Vaccin à renouveler" : "Marquer à jour");
}

void MainWindow::updateGlobalPanel()
{
    int upToDate = 0;
    for (const auto &r : m_records)
        if (r.medical.vaccinationStatus == "À jour") ++upToDate;
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
        maxNum = qMax(maxNum, r.medical.recordId.section('-', 1).toInt());
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
        m_records[idx] = dialog.getMedicalRecord();
        m_selectedId = m_records[idx].medical.recordId;
        refreshView();
    }
}

void MainWindow::deleteRecord(const QString &recordId)
{
    const int idx = indexOfRecord(recordId);
    if (idx < 0)
        return;
    const auto reply = QMessageBox::question(this, "Confirmation",
        QString("Êtes-vous sûr de vouloir supprimer le dossier de %1 ?").arg(m_records[idx].animal.name),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_records.removeAt(idx);
        m_selectedId.clear();
        refreshView();
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
        MedicalRecord rec = dialog.getMedicalRecord();
        if (rec.medical.recordId.trimmed().isEmpty() || indexOfRecord(rec.medical.recordId) >= 0)
            rec.medical.recordId = nextRecordNumber(m_records);
        m_records.append(rec);
        m_selectedId = rec.medical.recordId;
        refreshView();
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
    QString &status = m_records[idx].medical.vaccinationStatus;
    status = (status == "À jour") ? "À renouveler" : "À jour";
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
    QMap<QString, int> bySpecies;
    int upToDate = 0, toRenew = 0;
    for (const auto &r : m_records) {
        ++bySpecies[r.animal.species];
        if (r.medical.vaccinationStatus == "À jour") ++upToDate;
        else ++toRenew;
    }

    // A real statistics interface (instead of a QMessageBox): cards, progress
    // bars and a compact breakdown. It deliberately keeps the main
    // Medecaldosss window untouched, as requested.
    QDialog dialog(this);
    dialog.setObjectName("statisticsDialog");
    dialog.setWindowTitle("Medecaldosss — Statistiques");
    dialog.setMinimumSize(720, 560);
    dialog.resize(820, 620);

    auto *root = new QVBoxLayout(&dialog);
    root->setContentsMargins(30, 26, 30, 26);
    root->setSpacing(18);

    auto *heading = new QHBoxLayout;
    auto *titleBox = new QVBoxLayout;
    titleBox->setSpacing(4);
    auto *title = new QLabel("Statistiques");
    title->setObjectName("statisticsTitle");
    auto *subtitle = new QLabel("Vue globale de vos dossiers médicaux et vaccinations");
    subtitle->setObjectName("statisticsSubtitle");
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);
    heading->addLayout(titleBox);
    heading->addStretch();

    auto *logo = new QLabel;
    QPixmap logoPix(":/assets/logo.png");
    if (!logoPix.isNull())
        logo->setPixmap(logoPix.scaled(52, 52, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setObjectName("statisticsLogo");
    heading->addWidget(logo);
    root->addLayout(heading);

    auto *totals = new QHBoxLayout;
    totals->setSpacing(14);
    auto addMetric = [&](const QString &caption, const QString &value, const QString &tone) {
        auto *card = new QFrame;
        card->setObjectName("statisticsMetric");
        card->setProperty("tone", tone);
        auto *box = new QVBoxLayout(card);
        box->setContentsMargins(17, 14, 17, 14);
        box->setSpacing(5);
        auto *c = new QLabel(caption.toUpper());
        c->setObjectName("statisticsMetricCaption");
        auto *v = new QLabel(value);
        v->setObjectName("statisticsMetricValue");
        box->addWidget(c);
        box->addWidget(v);
        totals->addWidget(card, 1);
        return v;
    };
    addMetric("Dossiers", QString::number(m_records.size()), "blue");
    addMetric("Vaccins à jour", QString::number(upToDate), "green");
    addMetric("À renouveler", QString::number(toRenew), "orange");
    root->addLayout(totals);

    auto *content = new QHBoxLayout;
    content->setSpacing(16);

    auto *speciesCard = new QFrame;
    speciesCard->setObjectName("statisticsPanel");
    auto *speciesLayout = new QVBoxLayout(speciesCard);
    speciesLayout->setContentsMargins(20, 18, 20, 18);
    speciesLayout->setSpacing(12);
    auto *speciesTitle = new QLabel("Répartition par espèce");
    speciesTitle->setObjectName("statisticsSectionTitle");
    speciesLayout->addWidget(speciesTitle);

    QList<QPair<QProgressBar *, int>> speciesBars;
    int maxSpecies = 1;
    for (auto it = bySpecies.cbegin(); it != bySpecies.cend(); ++it)
        maxSpecies = qMax(maxSpecies, it.value());
    for (auto it = bySpecies.cbegin(); it != bySpecies.cend(); ++it) {
        auto *line = new QHBoxLayout;
        auto *name = new QLabel(it.key());
        name->setObjectName("statisticsRowLabel");
        name->setMinimumWidth(90);
        auto *bar = new QProgressBar;
        bar->setRange(0, maxSpecies);
        bar->setValue(0);
        bar->setTextVisible(false);
        bar->setObjectName("statisticsBar");
        auto *count = new QLabel(QString::number(it.value()));
        count->setObjectName("statisticsCount");
        line->addWidget(name);
        line->addWidget(bar, 1);
        line->addWidget(count);
        speciesLayout->addLayout(line);
        speciesBars.append({bar, it.value()});
    }
    if (bySpecies.isEmpty())
        speciesLayout->addWidget(new QLabel("Aucun dossier disponible."));
    content->addWidget(speciesCard, 3);

    auto *vaccCard = new QFrame;
    vaccCard->setObjectName("statisticsPanel");
    auto *vaccLayout = new QVBoxLayout(vaccCard);
    vaccLayout->setContentsMargins(20, 18, 20, 18);
    vaccLayout->setSpacing(12);
    auto *vaccTitle = new QLabel("État des vaccinations");
    vaccTitle->setObjectName("statisticsSectionTitle");
    vaccLayout->addWidget(vaccTitle);
    auto *vaccBar = new QProgressBar;
    const int total = m_records.size();
    const int percent = total ? qRound(100.0 * upToDate / total) : 0;
    vaccBar->setRange(0, 100);
    vaccBar->setValue(0);
    vaccBar->setFormat("%p%");
    vaccBar->setObjectName("statisticsVaccBar");
    vaccLayout->addWidget(vaccBar);
    auto *vaccText = new QLabel(QString("%1 sur %2 dossiers sont à jour").arg(upToDate).arg(total));
    vaccText->setObjectName("statisticsRowLabel");
    vaccLayout->addWidget(vaccText);
    vaccLayout->addStretch();
    content->addWidget(vaccCard, 2);
    root->addLayout(content, 1);

    auto *actions = new QHBoxLayout;
    actions->addStretch();
    auto *close = new QPushButton("Fermer");
    close->setObjectName("statisticsClose");
    close->setCursor(Qt::PointingHandCursor);
    actions->addWidget(close);
    root->addLayout(actions);
    connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);

    // Gentle entrance + animated bars, like the reference interface.
    QTimer::singleShot(0, &dialog, [&dialog, speciesBars, vaccBar, percent] {
        Motion::reveal(&dialog, QColor("#F3F7F8"), 220);
        for (const auto &entry : speciesBars)
            Motion::progressTo(entry.first, entry.second, true);
        Motion::progressTo(vaccBar, percent, true);
    });

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
        out << esc(r.medical.recordId) << ';' << esc(r.animal.animalId) << ';' << esc(r.animal.name) << ';'
            << esc(r.animal.species) << ';' << esc(r.animal.breed) << ';' << r.animal.age << ';'
            << esc(r.animal.sex) << ';' << esc(r.animal.owner) << ';'
            << r.medical.consultationDate.toString("dd/MM/yyyy") << ';' << esc(r.medical.reason) << ';'
            << esc(r.medical.diagnosis) << ';' << esc(r.medical.treatment) << ';'
            << esc(r.medical.vaccinations) << ';' << esc(r.medical.allergies) << ';'
            << r.medical.weight << ';' << esc(r.medical.vaccinationStatus) << '\n';
    }
}
