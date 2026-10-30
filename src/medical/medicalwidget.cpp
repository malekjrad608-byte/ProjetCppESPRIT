#include "../database/oracleconnection.h"
#include "medicalwidget.h"
#include "../ui/motion.h"
#include "../ui/design.h"
#include "../ui/invoice.h"
#include "medicalrecord.h"
#include "../dialogs/servicedialog.h"
#include <QStatusBar>
#include <QSignalBlocker>
#include <QTextBrowser>
#include "ui_medicalwidget.h"
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
const int kCardMinWidth = 270;
const int kGridSpacing = 16;
const int kCardHeight = 190;
const int kPanelWidth = 360;
const int kPanelGap = 26;
}

MedicalWidget::MedicalWidget(bool demo,QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MedicalWidget), m_demo(demo)
{
    ui->setupUi(this);      // builds everything described in mainwindow.ui

    setWindowFlags(Qt::Widget);
    setMinimumSize(0,0);
    setObjectName("medicalPage");
    ui->sidebar->hide(); ui->topBar->hide();
    loadStaticData();
    setupUI();              // wires the widgets from the .ui to the logic
    loadStylesheet();
    refreshView();
    if(OracleConnection::isOracle()) {
      ui->pageSubtitle->setText("Suivi des animaux, propriétaires et consultations");
      ui->filterVaccination->hide();ui->chipUpToDate->hide();ui->chipToRenew->hide();
      ui->globalCard->hide();ui->btnStatsTop->hide();ui->btnToggle->hide();ui->statusPill->hide();
    }
}

MedicalWidget::~MedicalWidget()
{
    delete ui;
}

// ---------------------------------------------------------------- UI setup

void MedicalWidget::setupUI()
{
    auto animated=[](QPushButton *&button,const char *role) {
        auto *replacement=Design::button(button->text(),{},role);
        replacement->setParent(button->parentWidget());replacement->setObjectName(button->objectName());replacement->setGeometry(button->geometry());
        delete button;button=replacement;
    };
    animated(ui->btnAdd,"primary");animated(ui->btnStatsTop,nullptr);animated(ui->btnReset,nullptr);
    animated(ui->btnView,"primary");animated(ui->btnToggle,"success");animated(ui->btnEdit,"primary");animated(ui->btnDelete,"danger");
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
    styleId({ui->btnToggle}, "medicalToggle");
    styleId({ui->btnEdit}, "medicalEdit");
    styleId({ui->btnDelete}, "medicalDelete");
    ui->btnView->setObjectName("medicalView");
    // Keep btnReset and btnStatsTop object names so their compact styles apply.
    styleId({ui->btnFile}, "medicalExport");
    styleId({ui->selectedCard, ui->globalCard}, "panelCard");
    styleId({ui->detailLine1, ui->detailLine2, ui->detailLine3, ui->globalText}, "panelMeta");
    styleId({ui->filterSpecies, ui->filterVaccination, ui->sortCombo}, "filterCombo");

    // --- Sidebar ---------------------------------------------------------
    // Logo: uses :/assets/logo.png when present, paw glyph otherwise
    QPixmap logoPix(":/brand/logo.png");
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
    menu->addAction("Actualiser", this, &MedicalWidget::onRefreshClicked);
    menu->addAction("Exporter PDF…", this, &MedicalWidget::onExportPdfClicked);
    menu->addSeparator();

    ui->btnFile->setMenu(menu);
    ui->btnFile->setParent(ui->pageFooter);
    ui->btnFile->setText("Fichier"); ui->btnFile->show();
    m_pdfButton=Design::button("PDF du dossier","export");m_pdfButton->setParent(ui->detailContent);m_pdfButton->setObjectName("medicalRecordPdf");
    connect(m_pdfButton,&QPushButton::clicked,this,[this]{exportPdf(true);});

    // --- Footer ----------------------------------------------------------
    ui->footerDate->setText(QLocale(QLocale::French).toString(QDate::currentDate(), "d MMMM yyyy"));

    // --- Records page: header & toolbar ----------------------------------
    connect(ui->btnAdd, &QPushButton::clicked, this, &MedicalWidget::onAddRecordClicked);
    connect(ui->btnStatsTop, &QPushButton::clicked, this, &MedicalWidget::onShowStatisticsClicked);
    ui->btnStatsTop->setText("Statistiques");
    ui->btnStatsTop->setIcon(Design::icon("grid"));

    ui->searchEdit->setObjectName("medicalSearch");
    ui->filterSpecies->setObjectName("medicalSpecies");
    ui->filterVaccination->setObjectName("medicalVaccination");
    ui->sortCombo->setObjectName("medicalSort");
    ui->btnReset->setObjectName("medicalReset");
    ui->btnAdd->setObjectName("medicalAdd");
    ui->btnStatsTop->setObjectName("medicalStatistics");
    ui->cardsScroll->setObjectName("medicalCardsScroll");
    m_chipTotal = ui->chipTotal;
    m_chipConsultations = ui->chipConsultations;
    m_chipUpToDate = ui->chipUpToDate;
    m_chipToRenew = ui->chipToRenew;

    m_searchEdit = ui->searchEdit;
    m_searchEdit->addAction(Design::icon("search"), QLineEdit::LeadingPosition);
    m_filterSpecies = ui->filterSpecies;
    m_filterVaccination = ui->filterVaccination;
    m_sortCombo = ui->sortCombo;

    connect(m_searchEdit, &QLineEdit::textChanged, this, &MedicalWidget::onFilterChanged);
    for (QComboBox *c : {m_filterSpecies, m_filterVaccination, m_sortCombo})
        connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MedicalWidget::onFilterChanged);
    connect(ui->btnReset, &QPushButton::clicked, this, &MedicalWidget::onResetFiltersClicked);

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
    for(auto *label:{m_detailTitle,m_detailLine1,m_detailLine2,m_detailLine3}) {label->setTextFormat(Qt::PlainText);label->setWordWrap(true);}


    connect(m_btnView, &QPushButton::clicked, this, &MedicalWidget::onViewRecordClicked);
    connect(m_btnToggle, &QPushButton::clicked, this, &MedicalWidget::onToggleVaccinationClicked);
    connect(m_btnEdit, &QPushButton::clicked, this, &MedicalWidget::onEditRecordClicked);
    connect(m_btnDelete, &QPushButton::clicked, this, &MedicalWidget::onDeleteRecordClicked);

    // Statistics are opened on demand from the top "Statistiques" button.
    // Keep the right column focused on the selected medical record.
    ui->globalCard->hide();
    m_globalValue = ui->bigValue;
    m_globalBar = ui->globalBar;
    m_globalText = ui->globalText;
    connect(ui->btnStatsPanel, &QPushButton::clicked, this, &MedicalWidget::onShowStatisticsClicked);
}

void MedicalWidget::loadStylesheet()
{
    QFile file(":/medical-source.qss");
    if(file.open(QIODevice::ReadOnly)) setStyleSheet(QString::fromUtf8(file.readAll()));
}

// ---------------------------------------------------------------- Data

void MedicalWidget::loadStaticData()
{
    QString error; m_records=MedicalRecord::afficher(&error);
    if(!error.isEmpty()) QMessageBox::warning(this,"Dossiers médicaux",error);
}

int MedicalWidget::indexOfRecord(const QString &recordId) const
{
    for (int i = 0; i < m_records.size(); ++i)
        if (m_records[i].medical.recordId == recordId)
            return i;
    return -1;
}

void MedicalWidget::applyFiltersAndSort()
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

void MedicalWidget::refreshView()
{
    applyFiltersAndSort();

    // Keep the selection only if it is still visible
    bool stillVisible = false;
    for (const auto &r : m_filteredRecords)
        if (r.medical.recordId == m_selectedId) { stillVisible = true; break; }
    if (!stillVisible)
        m_selectedId.clear();

    rebuildCards();
    ui->btnFile->setEnabled(!m_filteredRecords.isEmpty());
    updateChips();
    updateDetailPanel();
    updateGlobalPanel();
    layoutUi();
}

void MedicalWidget::rebuildCards()
{
    for (RecordCard *c : m_cards)
        { c->hide(); c->deleteLater(); }
    m_cards.clear();

    for (const auto &r : m_filteredRecords) {
        auto *card = new RecordCard(r, m_cardsContainer);
        connect(card, &RecordCard::clicked, this, &MedicalWidget::onCardClicked);
        connect(card, &RecordCard::doubleClicked, this, [this](const QString &id) { viewRecord(id); });
        card->setSelected(r.medical.recordId == m_selectedId);
        m_cards.append(card);
    }
    m_emptyLabel->setVisible(m_cards.isEmpty());
    relayoutCards();
}

// Cards are placed by hand (no layout): as many columns as fit in the viewport.
void MedicalWidget::relayoutCards()
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
    m_cardsContainer->setMinimumHeight(contentH);
    m_cardsContainer->resize(viewW,contentH);
    m_emptyLabel->setGeometry(0, 0, viewW, 160);
}

// Positions every block of the window by hand (the .ui has no layouts).
void MedicalWidget::layoutUi()
{
    if (!ui || !m_scroll)
        return;

    const int W = width(), H = height();
    const int sidebarW = 0, topH = 0, footerH = 30;
    const int mainW = W - sidebarW;

    // Sidebar (slogan stays glued to the bottom)
    ui->sidebar->hide(); ui->topBar->hide();
    ui->sloganSub->setGeometry(18, H - 28 - 18, 254, 22);
    ui->sloganLabel->setGeometry(18, H - 28 - 18 - 26, 254, 26);

    // Main area
    ui->mainContent->setGeometry(sidebarW, 0, mainW, H);

    // Top bar (right-aligned group)
    ui->topBar->setGeometry(0, 0, mainW, topH);
    ui->roleLabel->setGeometry(mainW - 26 - 170, 22, 170, 24);
    ui->demoBadge->setGeometry(ui->btnFile->x() - 14 - 210, 10, 210, 48);

    // Footer
    ui->pageFooter->setGeometry(0, H - footerH, mainW, footerH);
    ui->footerDate->setGeometry(mainW - 34 - 160, 2, 160, 24);
    ui->footerLeft->setGeometry(24,2,350,24);

    // Page area = records (left) + detail panel (right)
    const int pageTop = topH + 28;
    const int pageW = mainW - 48;
    const int pageH = qMax(200, H - footerH - 14 - pageTop);
    ui->pageArea->setGeometry(24, pageTop, pageW, pageH);

    const int LW = pageW - qMin(kPanelWidth,qMax(280,pageW/3)) - kPanelGap;   // records column width
    ui->recordsPage->setGeometry(0, 0, LW, pageH);

    ui->pageTitle->setGeometry(0, 0, LW, 44);
    ui->pageSubtitle->setGeometry(0, 50, LW, 20);
    ui->btnAdd->setGeometry(LW - 210, 0, 210, 49);

    // Summary chips: keep them on their own row, with a fixed compact height.
    // A smaller font and proportional widths keep the four chips inside the
    // records column even on narrower windows.
    const QList<QLabel *> chips = {ui->chipTotal, ui->chipConsultations,
                                   ui->chipUpToDate, ui->chipToRenew};
    const bool narrow=LW<580;
    const int shift=narrow?54:0;
    if(narrow) {ui->pageTitle->move(0,54);ui->pageSubtitle->move(0,99);}
    const int chipsY = 104+shift;
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
    ui->btnAdd->setGeometry(narrow?0:LW-addW,0,narrow?qMin(200,LW-qMin(158,qMax(140,LW/4))-12):addW,52);
    const int statsW = qMin(158, qMax(140, LW / 4));
    ui->btnStatsTop->setGeometry(LW - statsW, narrow?0:62, statsW, 40);

    // Search and filters form a separate toolbar, matching the Employees page.
    const int ty = 150+shift;
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

void MedicalWidget::layoutDetailPanel()
{
    const bool has=indexOfRecord(m_selectedId)>=0;
    const int panelW=qMin(kPanelWidth,qMax(280,ui->pageArea->width()/3)),inner=panelW-44;
    const int cardH=has?554:150;
    ui->selectedCard->setGeometry(0,0,panelW,cardH);
    ui->placeholderText->setGeometry(22,58,inner,64);
    ui->detailContent->setGeometry(22,44,inner,494);
    m_detailTitle->setGeometry(0,0,inner,32);m_detailPill->setGeometry(0,40,inner,28);
    m_detailLine1->setGeometry(0,78,inner,28);m_detailLine2->setGeometry(0,108,inner,46);m_detailLine3->setGeometry(0,156,inner,64);
    int y=226;
    for(auto *button:{m_btnView,m_btnToggle,m_btnEdit,m_pdfButton,m_btnDelete}) {button->setGeometry(0,y,inner,36);y+=44;}
    ui->globalCard->hide();ui->detailPanel->resize(panelW,cardH);

}

void MedicalWidget::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    layoutUi();
}

bool MedicalWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (m_scroll && watched == m_scroll->viewport() && event->type() == QEvent::Resize)
        relayoutCards();
    return QMainWindow::eventFilter(watched, event);
}

void MedicalWidget::updateChips()
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

void MedicalWidget::updateDetailPanel()
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
    m_btnToggle->setIcon(Design::icon("check"));
}

void MedicalWidget::updateGlobalPanel()
{
    int upToDate = 0;
    for (const auto &r : m_records)
        if (r.medical.vaccinationStatus == "À jour") ++upToDate;
    const int total = m_records.size();
    const int pct = total ? qRound(100.0 * upToDate / total) : 0;
    m_globalValue->setText(QString("%1 %").arg(pct));
    Motion::progressTo(m_globalBar,pct);
    m_globalText->setText(QString("%1 dossier%2 à jour sur %3").arg(upToDate).arg(upToDate > 1 ? "s" : "").arg(total));
}

// ---------------------------------------------------------------- Actions

QString MedicalWidget::nextRecordNumber(const QVector<MedicalRecord> &records)
{
    int maxNum = 0;
    for (const auto &r : records)
        maxNum = qMax(maxNum, r.medical.recordId.section('-', 1).toInt());
    return QString("MED-%1").arg(maxNum + 1, 3, 10, QChar('0'));
}

void MedicalWidget::viewRecord(const QString &recordId)
{
    const int idx = indexOfRecord(recordId);
    if (idx < 0)
        return;
    MedicalRecordDetailsDialog dialog(this);
    dialog.setMedicalRecord(m_records[idx]);
    dialog.exec();
}

void MedicalWidget::editRecord(const QString &recordId)
{
    saveDialog(recordId);
}

void MedicalWidget::deleteRecord(const QString &recordId)
{
    const int idx = indexOfRecord(recordId);
    if (idx < 0)
        return;
    const auto reply = QMessageBox::question(this, "Confirmation",
        QString("Êtes-vous sûr de vouloir supprimer le dossier de %1 ?").arg(m_records[idx].animal.name),
        QMessageBox::Yes | QMessageBox::No,QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QString error; if(!MedicalRecord::supprimer(m_records[idx].storageId,&error)) {QMessageBox::warning(this,"Suppression impossible",error);return;}
        loadStaticData();
        m_selectedId.clear();
        refreshView();
    }
}

void MedicalWidget::onNavigationClicked(const QString &name)
{
    if (name == "Dossiers médicaux")
        return; // already here
    QMessageBox::information(this, "Navigation", QString("%1 - Non implémenté").arg(name));
}

void MedicalWidget::onAddRecordClicked()
{
    saveDialog({});
}

void MedicalWidget::onViewRecordClicked()   { viewRecord(m_selectedId); }
void MedicalWidget::onEditRecordClicked()   { editRecord(m_selectedId); }
void MedicalWidget::onDeleteRecordClicked() { deleteRecord(m_selectedId); }

void MedicalWidget::onToggleVaccinationClicked()
{
    const int idx=indexOfRecord(m_selectedId); if(idx<0)return;
    auto record=m_records[idx]; record.medical.vaccinationStatus=record.medical.vaccinationStatus=="À jour"?"À renouveler":"À jour";
    QString error; if(!(record.storageId>0?record.modifier(&error):record.ajouter(&error))) {QMessageBox::warning(this,"Enregistrement impossible",error);return;} refresh();
}

void MedicalWidget::onRefreshClicked()
{
    refresh();
}

void MedicalWidget::onFilterChanged()
{
    refreshView();
}

void MedicalWidget::onResetFiltersClicked()
{
    const QSignalBlocker b1(m_searchEdit), b2(m_filterSpecies), b3(m_filterVaccination), b4(m_sortCombo);
    m_searchEdit->clear();
    m_filterSpecies->setCurrentIndex(0);
    m_filterVaccination->setCurrentIndex(0);
    m_sortCombo->setCurrentIndex(0);
    refreshView();
}

void MedicalWidget::onCardClicked(const QString &recordId)
{
    m_selectedId = recordId;
    for (RecordCard *c : m_cards)
        c->setSelected(c->recordId() == recordId);
    updateDetailPanel();
    layoutDetailPanel();
}

void MedicalWidget::onShowStatisticsClicked()
{
    QMap<QString, QPair<int, QPair<int, int>>> bySpecies;
    int upToDate = 0, toRenew = 0;
    for (const auto &r : m_records) {
        auto &entry = bySpecies[r.animal.species];
        ++entry.first;
        if (r.medical.vaccinationStatus == "À jour") {
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
    dialog.setObjectName("medicalStatisticsDialog");
    dialog.setWindowTitle("Statistiques des dossiers médicaux");
    dialog.setMinimumSize(740, 540);
    dialog.resize(880, 640);

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
    QPixmap logoPix(":/brand/logo.png");
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
        Q_UNUSED(iconText);
        auto *ico = new QLabel;
        const QString glyph=tone=="green"?"check":tone=="red"?"clock":tone=="orange"?"grid":"services";
        ico->setPixmap(Design::icon(glyph,tone=="red"?QColor("#CD575B"):QColor("#377E8E")).pixmap(24,24));
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
    table->setObjectName("medicalStatsTable");
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Espèce", "Dossiers", "Vaccins à jour", "À renouveler", "Taux à jour"});
    table->setRowCount(bySpecies.size());
    table->setAlternatingRowColors(false);
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
        Motion::progressTo(rate,speciesPercent);
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
    auto *close = Design::button("Fermer",{},"primary");
    close->setObjectName("statisticsClose");
    close->setCursor(Qt::PointingHandCursor);
    close->setMinimumSize(180, 48);
    actions->addWidget(close);
    root->addLayout(actions);
    connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}

void MedicalWidget::onExportPdfClicked()
{
    exportPdf(false);
}

void MedicalWidget::refresh() { loadStaticData();refreshView(); }
void MedicalWidget::focusSearch() { m_searchEdit->setFocus();m_searchEdit->selectAll(); }
QPushButton *MedicalWidget::fileMenuButton() const {return ui->btnFile;}
const MedicalRecord *MedicalWidget::selected() const {int idx=indexOfRecord(m_selectedId);return idx<0?nullptr:&m_records[idx];}

void MedicalWidget::saveDialog(const QString &id) {
    const int idx=indexOfRecord(id);const qint64 storageId=idx<0?0:m_records[idx].storageId;
    MedicalRecordDialog dialog(this);if(idx>=0)dialog.setMedicalRecord(m_records[idx]);
    auto *save=dialog.findChild<QPushButton *>("primaryButton");disconnect(save,nullptr,&dialog,nullptr);
    connect(save,&QPushButton::clicked,&dialog,[&]{auto record=dialog.getMedicalRecord();record.storageId=storageId;QString error;
        if(!(record.storageId>0?record.modifier(&error):record.ajouter(&error))){QMessageBox::warning(&dialog,"Enregistrement impossible",error);return;}
        m_selectedId=record.medical.recordId;dialog.accept();});
    if(dialog.exec()==QDialog::Accepted)refresh();
}

void MedicalWidget::exportPdf(bool selectedOnly) {
    QVector<MedicalRecord> records=m_filteredRecords;if(selectedOnly){const auto *r=selected();if(!r)return;records={*r};}if(records.isEmpty())return;
    const QString html=MedicalRecord::reportHtml(records,m_demo);
    QDialog dialog(this);dialog.setObjectName("medicalPdfPreview");dialog.setWindowTitle("Aperçu du PDF médical");dialog.resize(780,640);
    auto *layout=new QVBoxLayout(&dialog);auto *preview=new QTextBrowser;preview->setOpenExternalLinks(false);preview->setHtml(html);layout->addWidget(preview);
    auto *buttons=new QHBoxLayout;buttons->addStretch();auto *close=Design::button("Fermer");auto *save=Design::button("Enregistrer le PDF","export","primary");buttons->addWidget(close);buttons->addWidget(save);layout->addLayout(buttons);
    connect(close,&QPushButton::clicked,&dialog,&QDialog::reject);
    connect(save,&QPushButton::clicked,&dialog,[&]{QString path=QFileDialog::getSaveFileName(&dialog,"Enregistrer le PDF",selectedOnly?records.first().medical.recordId+".pdf":"ANIWORLD-dossiers-medicaux.pdf","Document PDF (*.pdf)");if(path.isEmpty())return;if(!path.endsWith(".pdf",Qt::CaseInsensitive))path+=".pdf";QString error;
        if(!Invoice::writeDocument(path,html,"Dossiers médicaux ANIWORLD",&error)){QMessageBox::warning(&dialog,"Export impossible",error);return;}dialog.accept();});dialog.exec();
}
