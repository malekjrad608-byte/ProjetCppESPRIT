#include "medicalwindow.h"
#include "motion.h"
#include "ui_medicalwindow.h"
#include "recordcard.h"
#include "medicalrecorddialog.h"
#include "medicalrecorddetailsdialog.h"
#include <QSqlDatabase>
#include "medicalrecord.h"
#include "statscurvewidget.h"
#include "reminderdialog.h"
#include "smsservice.h"
#include "vaccination.h"
#include <QTabWidget>

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
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include "appointment.h"
#include "appointmentdialog.h"
#include "animalpdf.h"
#include "healthwatch.h"
#include "healthwatchdialog.h"
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <algorithm>

namespace {
const int kCardMinWidth = 300;
const int kGridSpacing = 10;
const int kCardHeight = 130;
const int kPanelWidth = 240;
const int kPanelGap = 12;
// Menu latéral (Navigation) : true = affiché, false = masqué.
const bool kShowSidebar = false;      // intégré : le menu latéral est celui de la fenêtre principale ANIWORLD
}

MedicalWindow::MedicalWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MedicalWindow)
{
    ui->setupUi(this);      // builds everything described in mainwindow.ui

    m_db = QSqlDatabase::database().isOpen();     // connexion ouverte au démarrage par main.cpp (Connection de Linda)
    if (m_db)
        actualiser();        // Appel de la méthode afficher() via l'attribut Mtmp
    else
        loadStaticData();    // mode hors ligne
    syncVaccinationStatus();
    setupUI();              // wires the widgets from the .ui to the logic
    loadStylesheet();
    refreshView();
    QTimer::singleShot(600, this, &MedicalWindow::showStartupReminders);   // alerte de rappels au démarrage
    // Rendez-vous : vérification au démarrage puis toutes les 15 minutes (SMS automatiques)
    QTimer::singleShot(1500, this, &MedicalWindow::checkAppointments);
    QTimer::singleShot(3000, this, [this]() { checkHealthWatch(true); });   // veille sanitaire au démarrage
    m_rdvTimer = new QTimer(this);
    connect(m_rdvTimer, &QTimer::timeout, this, &MedicalWindow::checkAppointments);
    m_rdvTimer->start(15 * 60 * 1000);
}

MedicalWindow::~MedicalWindow()
{
    delete ui;
}

// ---------------------------------------------------------------- UI setup

void MedicalWindow::setupUI()
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
    QPixmap logoPix(":/med/images/logo.png");
    if (!logoPix.isNull())
        ui->logoIcon->setPixmap(logoPix.scaled(52, 52, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        ui->logoIcon->setPixmap(QPixmap(":/med/icons/paw.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_navButtons = { ui->btnNavDashboard, ui->btnNavServices, ui->btnNavEmployees,
                     ui->btnNavStock, ui->btnNavPremises, ui->btnNavRecords };
    for (QPushButton *btn : std::as_const(m_navButtons)) {
        const QString label = btn->text();
        connect(btn, &QPushButton::clicked, this, [this, label]() { onNavigationClicked(label); });
    }

    // --- Top bar ---------------------------------------------------------
    auto *menu = new QMenu(ui->btnFile);
    menu->addAction("Actualiser", this, &MedicalWindow::onRefreshClicked);
    menu->addAction("Exporter en CSV…", this, &MedicalWindow::onExportCsvClicked);
    menu->addAction("Fiche animal (PDF)…", this, &MedicalWindow::onExportPdfClicked);
    menu->addAction("Rendez-vous…", this, &MedicalWindow::onRdvClicked);
    menu->addAction("Veille sanitaire…", this, &MedicalWindow::onWatchClicked);
    menu->addSeparator();
    menu->addAction("Quitter", this, &QWidget::close);
    ui->btnFile->setText("Actions");
    ui->btnFile->setMenu(menu);
    // Intégré : la barre du haut de cette page est supprimée (la fenêtre principale a déjà la sienne).
    // Le menu d'actions (export CSV, fiche PDF, rendez-vous, veille) passe dans l'en-tête de la page.
    ui->btnFile->setParent(ui->recordsPage);
    ui->btnFile->show();
    ui->topBar->hide();

    // « ● Oracle connecté » + « Se déconnecter » (à la place de l'ancien libellé de rôle)
    ui->roleLabel->setObjectName("dbStatus");
    ui->roleLabel->setAlignment(Qt::AlignCenter);
    ui->roleLabel->setText(m_db ? QString::fromUtf8("●  Oracle connecté")
                                : QString::fromUtf8("●  Hors ligne"));
    ui->roleLabel->setProperty("tone", m_db ? "ok" : "warn");
    m_btnLogout = new QPushButton(QString::fromUtf8("⎋  Se déconnecter"), ui->topBar);
    m_btnLogout->setObjectName("logoutButton");
    m_btnLogout->setCursor(Qt::PointingHandCursor);
    connect(m_btnLogout, &QPushButton::clicked, this, &MedicalWindow::onLogoutClicked);
    m_btnLogout->hide();         // intégré : « Oracle connecté » et la déconnexion sont gérés par la fenêtre principale
    ui->roleLabel->hide();

    // Icône du titre de la page
    m_titleIcon = new QLabel(ui->recordsPage);
    m_titleIcon->setObjectName("titleIcon");
    m_titleIcon->setPixmap(QPixmap(":/med/icons/stethoscope.svg").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_titleIcon->show();

    // --- Footer ----------------------------------------------------------
    ui->footerDate->setText(QLocale(QLocale::French).toString(QDate::currentDate(), "d MMMM yyyy"));

    // --- Records page: header & toolbar ----------------------------------
    connect(ui->btnAdd, &QPushButton::clicked, this, &MedicalWindow::onAddRecordClicked);
    connect(ui->btnStatsTop, &QPushButton::clicked, this, &MedicalWindow::onShowStatisticsClicked);
    ui->btnStatsTop->setText(QString::fromUtf8("📊  Statistiques"));

    m_chipTotal = ui->chipTotal;
    m_chipConsultations = ui->chipConsultations;
    m_chipUpToDate = ui->chipUpToDate;
    m_chipToRenew = ui->chipToRenew;

    m_searchEdit = ui->searchEdit;
    m_searchEdit->addAction(QIcon(":/med/icons/search_grey.svg"), QLineEdit::LeadingPosition);
    m_filterSpecies = ui->filterSpecies;
    m_filterVaccination = ui->filterVaccination;
    m_sortCombo = ui->sortCombo;

    connect(m_searchEdit, &QLineEdit::textChanged, this, &MedicalWindow::onFilterChanged);
    for (QComboBox *c : {m_filterSpecies, m_filterVaccination, m_sortCombo})
        connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MedicalWindow::onFilterChanged);
    connect(ui->btnReset, &QPushButton::clicked, this, &MedicalWindow::onResetFiltersClicked);

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

    // Le panneau suit la maquette : « DOSSIER SÉLECTIONNÉ », un filet, des lignes
    // libellé / valeur, puis les trois actions pleine largeur.
    ui->detailContent->hide();
    ui->statusPill->hide();
    for (QPushButton *b : {ui->btnEdit, ui->btnToggle, ui->btnDelete}) {
        b->setParent(ui->selectedCard);     // sinon ils restent rognés par detailContent
        b->hide();
    }
    ui->placeholderText->setParent(ui->selectedCard);
    m_detailSep = new QFrame(ui->selectedCard);
    m_detailSep->setObjectName("detailSep");
    m_detailSep->setFixedHeight(1);
    const QStringList keys = { "Animal", "Espèce", "Âge", "Propriétaire",
                               "Dossier", "Consultation", "Motif", "Vaccins" };
    for (const QString &k : keys) {
        auto *key = new QLabel(k, ui->selectedCard);
        key->setObjectName("detailKey");
        auto *val = new QLabel(QString(), ui->selectedCard);
        val->setObjectName("detailVal");
        m_detailKeys.append(key);
        m_detailVals.append(val);
        key->hide();
        val->hide();
    }
    m_detailSep->hide();

    connect(m_btnView, &QPushButton::clicked, this, &MedicalWindow::onViewRecordClicked);
    connect(m_btnToggle, &QPushButton::clicked, this, &MedicalWindow::onToggleVaccinationClicked);
    connect(m_btnEdit, &QPushButton::clicked, this, &MedicalWindow::onEditRecordClicked);
    connect(m_btnDelete, &QPushButton::clicked, this, &MedicalWindow::onDeleteRecordClicked);

    // --- Actions de la page Dossiers : Veille, Rappels et Statistiques ----
    // Les trois boutons sont alignés sous le bouton Ajouter, comme sur la
    // maquette de référence. Le module Coupes n'est pas proposé sur cette page.
    QWidget *header = ui->btnStatsTop->parentWidget();
    m_btnReminders = new QPushButton(header);
    m_btnReminders->setObjectName("btnStatsTop");
    m_btnReminders->setCursor(Qt::PointingHandCursor);
    m_btnWatch = new QPushButton(QString::fromUtf8("🩺  Veille"), header);
    m_btnWatch->setObjectName("btnStatsTop");
    m_btnWatch->setCursor(Qt::PointingHandCursor);
    connect(m_btnWatch, &QPushButton::clicked, this, &MedicalWindow::onWatchClicked);
    m_btnWatch->show();
    m_btnReminders->show();
    connect(m_btnReminders, &QPushButton::clicked, this, &MedicalWindow::onRemindersClicked);
    ui->btnStatsTop->show();

    // SMS : un SMS réellement envoyé est enregistré (date) pour ne pas relancer le propriétaire à chaque ouverture
    m_sms = new SmsService(this);
    connect(m_sms, &SmsService::finished, this, [this](const QString &ref, bool ok, bool simulated, const QString &) {
        if (!ok || simulated)
            return;
        if (ref.startsWith("RDV|")) {          // "RDV|<id>|R" (rappel) ou "RDV|<id>|M" (manqué) : un seul SMS de chaque type
            const QStringList p = ref.split('|');
            if (p.size() == 3)
                RdvManager::marquerSms(p[1], p[2] == "M", QDate::currentDate());
            return;
        }
        VaccinationManager::marquerSmsEnvoye(ref, QDate::currentDate());
    });

    // Statistics are opened on demand from the top "Statistiques" button.
    // Keep the right column focused on the selected medical record.
    ui->globalCard->hide();
    m_globalValue = ui->bigValue;
    m_globalBar = ui->globalBar;
    m_globalText = ui->globalText;
    connect(ui->btnStatsPanel, &QPushButton::clicked, this, &MedicalWindow::onShowStatisticsClicked);
}

void MedicalWindow::loadStylesheet()
{
    QFile file(":/med/styles/style.qss");
    if (file.open(QIODevice::ReadOnly))
        setStyleSheet(QString::fromUtf8(file.readAll()));   // style limité à cette page (le style global reste celui d'ANIWORLD)
}

// ---------------------------------------------------------------- Data

void MedicalWindow::loadStaticData()
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
void MedicalWindow::actualiser()
{
    // Les champs sans colonne dans MEDICAL (vaccinations, allergies, ...) sont gardés en mémoire
    QMap<QString, MedicalRecord> anciens;
    for (const auto &r : m_records)
        anciens.insert(r.getIdDossier(), r);

    QSqlQueryModel *model = Mtmp.afficher();
    m_records.clear();
    for (int i = 0; i < model->rowCount(); ++i) {
        auto val = [model, i](int col) { return model->data(model->index(i, col)); };
        // ID_DOSSIER et REF_ANIMAL sont des NUMBER dans la base : affichés "MED-012" / "ANI-005"
        MedicalRecord M(MedicalRecord::versTexte(val(0), "MED"), val(1).toDate(), val(2).toString(), val(3).toString(),
                        MedicalRecord::versTexte(val(4), "ANI"), val(5).toString(), val(6).toString(), val(7).toString(),
                        val(8).toInt(), val(9).toDouble(), val(10).toString(), val(11).toString());
        M.setSexe(val(12).toString());          // colonnes SEXE et TRAITEMENT (si elles existent dans MEDICAL)
        M.setTraitement(val(13).toString());
        if (anciens.contains(M.getIdDossier())) {
            const MedicalRecord &a = anciens[M.getIdDossier()];
            if (M.getSexe().isEmpty())          // pas de colonne en base : on garde la valeur saisie
                M.setSexe(a.getSexe());
            if (M.getTraitement().isEmpty())
                M.setTraitement(a.getTraitement());
            M.setVaccinations(a.getVaccinations());
            M.setAllergies(a.getAllergies());
            M.setObservations(a.getObservations());
            M.setStatutVaccination(a.getStatutVaccination());
        }
        m_records.append(M);
    }
    delete model;
    syncVaccinationStatus();
}

int MedicalWindow::indexOfRecord(const QString &recordId) const
{
    for (int i = 0; i < m_records.size(); ++i)
        if (m_records[i].getIdDossier() == recordId)
            return i;
    return -1;
}

void MedicalWindow::applyFiltersAndSort()
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

void MedicalWindow::refreshView()
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
    updateReminderButton();
    layoutUi();
}

void MedicalWindow::rebuildCards()
{
    for (RecordCard *c : m_cards)
        c->deleteLater();
    m_cards.clear();

    for (const auto &r : m_filteredRecords) {
        auto *card = new RecordCard(r, m_cardsContainer);
        connect(card, &RecordCard::clicked, this, &MedicalWindow::onCardClicked);
        connect(card, &RecordCard::doubleClicked, this, [this](const QString &id) { viewRecord(id); });
        card->setSelected(r.getIdDossier() == m_selectedId);
        m_cards.append(card);
    }
    m_emptyLabel->setVisible(m_cards.isEmpty());
    relayoutCards();
}

// Cards are placed by hand (no layout): as many columns as fit in the viewport.
void MedicalWindow::relayoutCards()
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
void MedicalWindow::layoutUi()
{
    if (!ui || !m_scroll)
        return;

    const int W = width(), H = height();
    const int sidebarW = kShowSidebar ? qBound(200, qRound(W * 0.205), 270) : 0;
    const int topH = 0, footerH = 30;          // pas de barre supérieure propre : voir setupUI
    const int mainW = qMax(500, W - sidebarW);

    ui->sidebar->setVisible(kShowSidebar);
    ui->sidebar->setGeometry(0, 0, sidebarW, H);
    ui->sloganSub->setGeometry(18, H - 42, sidebarW - 36, 20);
    ui->sloganLabel->setGeometry(18, H - 66, sidebarW - 36, 24);
    ui->mainContent->setGeometry(sidebarW, 0, mainW, H);

    // Top strip: breadcrumb, demo status and file menu.
    ui->topBar->setGeometry(0, 0, mainW, topH);
    // De droite à gauche : Se déconnecter · Oracle connecté · Fichier · Données de démonstration
    const int barBtnH = 38, barY = (topH - barBtnH) / 2, barGap = 10;
    int barX = mainW - 20;
    if (m_btnLogout) {
        barX -= 170;
        m_btnLogout->setGeometry(barX, barY, 170, barBtnH);
        barX -= barGap;
    }
    barX -= 160;
    ui->roleLabel->setGeometry(barX, barY, 160, barBtnH);
    barX -= barGap + 100;
    barX -= barGap + 210;
    ui->demoBadge->setGeometry(qMax(10, barX), barY, 210, barBtnH);
    ui->demoBadge->setVisible(barX >= 320);   // petite fenêtre : évite le chevauchement avec le fil d'Ariane
    ui->crumbGestion->setGeometry(20, barY, 64, barBtnH);
    ui->crumbSep->setGeometry(88, barY, 14, barBtnH);
    ui->crumbCurrent->setGeometry(106, barY, 200, barBtnH);

    ui->pageFooter->setGeometry(0, H - footerH, mainW, footerH);
    ui->footerDate->setGeometry(mainW - 30 - 150, 6, 150, 16);

    const int pageTop = topH + 14;
    const int pageW = qMax(460, mainW - 40);
    const int pageH = qMax(200, H - footerH - 10 - pageTop);
    ui->pageArea->setGeometry(20, pageTop, pageW, pageH);

    // Main content: left cards area and a compact selected-record panel.
    const int panelW = qBound(300, qRound(pageW * 0.28), 380);
    const int panelGap = 12;
    const int leftW = qMax(300, pageW - panelW - panelGap);
    // La page couvre toute la largeur (boutons d'en-tête et filtres vont jusqu'au bord droit) ;
    // le panneau du dossier sélectionné est posé par-dessus, à droite.
    ui->recordsPage->setGeometry(0, 0, pageW, pageH);

    // Header: page title on the left, Add on the upper right.
    const int addW = 240;
    if (m_titleIcon)
        m_titleIcon->setGeometry(0, 10, 34, 40);
    ui->pageTitle->setGeometry(44, 8, qMax(180, leftW - 56), 44);
    ui->pageSubtitle->setGeometry(0, 56, qMax(180, leftW - 12), 22);
    ui->btnAdd->setGeometry(pageW - addW, 0, addW, 46);
    ui->btnFile->setGeometry(pageW - addW - 10 - 110, 4, 110, 38);
    ui->btnFile->raise();

    // Veille / Rappels / Statistiques sit directly under Add.
    const int actionGap = 10;
    const int actionW = 132;
    const int actionY = 58, actionH = 38;
    const int actionTotal = 3 * actionW + 2 * actionGap;
    const int actionX = pageW - actionTotal;
    if (m_btnWatch)
        m_btnWatch->setGeometry(actionX, actionY, actionW, actionH);
    if (m_btnReminders)
        m_btnReminders->setGeometry(actionX + actionW + actionGap, actionY, actionW, actionH);
    ui->btnStatsTop->setGeometry(actionX + 2 * (actionW + actionGap), actionY, actionW, actionH);

    // The reference screen has no summary chips; keep the toolbar compact.
    ui->chipTotal->hide();
    ui->chipConsultations->hide();
    ui->chipUpToDate->hide();
    ui->chipToRenew->hide();

    // Search and filters span the full content width, above both columns.
    const int toolbarY = 106;
    const int toolbarH = 40;
    const int gap = 10;
    const int resetW = 130;
    const int searchW = qRound(pageW * 0.405);
    const int remaining = pageW - searchW - resetW - 4 * gap;
    const int comboW = qMax(70, remaining / 3);
    ui->searchEdit->setGeometry(0, toolbarY, searchW, toolbarH);
    ui->filterSpecies->setGeometry(searchW + gap, toolbarY, comboW, toolbarH);
    ui->filterVaccination->setGeometry(searchW + gap + comboW + gap,
                                       toolbarY, comboW, toolbarH);
    ui->sortCombo->setGeometry(searchW + gap + 2 * (comboW + gap),
                               toolbarY, comboW, toolbarH);
    ui->btnReset->setGeometry(pageW - resetW, toolbarY, resetW, toolbarH);

    // Cards and selected dossier start on the same baseline.
    const int contentTop = 162;
    ui->cardsScroll->setGeometry(0, contentTop, leftW,
                                 qMax(100, pageH - contentTop));
    relayoutCards();

    // La largeur du panneau doit être connue avant de placer son contenu.
    ui->detailPanel->setGeometry(leftW + panelGap, contentTop, panelW,
                                 qMax(170, pageH - contentTop));
    layoutDetailPanel();
    ui->detailPanel->raise();
}

void MedicalWindow::layoutDetailPanel()
{
    const bool has = indexOfRecord(m_selectedId) >= 0;
    const int panelW = ui->detailPanel->width() > 0 ? ui->detailPanel->width() : kPanelWidth;
    const int innerX = 18;
    const int innerW = panelW - 2 * innerX;

    ui->eyebrow->setGeometry(innerX, 16, innerW, 18);
    m_detailSep->setGeometry(innerX, 42, innerW, 1);
    m_detailSep->setVisible(has);
    ui->btnView->hide();
    ui->detailContent->hide();
    ui->statusPill->hide();

    int cardH = 150;
    ui->placeholderText->setGeometry(innerX, 54, innerW, 60);
    ui->placeholderText->setVisible(!has);

    const int rowH = 28, rowsY = 52, keyW = qMin(104, innerW / 3);
    for (int i = 0; i < m_detailKeys.size(); ++i) {
        const int y = rowsY + i * rowH;
        m_detailKeys[i]->setGeometry(innerX, y, keyW, rowH);
        m_detailVals[i]->setGeometry(innerX + keyW, y, innerW - keyW, rowH);
        m_detailKeys[i]->setVisible(has);
        m_detailVals[i]->setVisible(has);
        if (has && i < m_detailValues.size()) {
            const QString full = m_detailValues.at(i);
            m_detailVals[i]->setText(m_detailVals[i]->fontMetrics().elidedText(
                full, Qt::ElideRight, innerW - keyW));
            m_detailVals[i]->setToolTip(full);
        }
    }

    const int actionH = 38, actionGap = 8;
    ui->btnEdit->setVisible(has);
    ui->btnToggle->setVisible(has);
    ui->btnDelete->setVisible(has);
    if (has) {
        ui->btnEdit->setText(QString::fromUtf8("✏️  Modifier"));
        ui->btnDelete->setText(QString::fromUtf8("🗑  Supprimer"));
        const int actionY = rowsY + m_detailKeys.size() * rowH + 12;
        ui->btnEdit->setGeometry(innerX, actionY, innerW, actionH);
        ui->btnToggle->setGeometry(innerX, actionY + actionH + actionGap, innerW, actionH);
        ui->btnDelete->setGeometry(innerX, actionY + 2 * (actionH + actionGap), innerW, actionH);
        cardH = actionY + 3 * actionH + 2 * actionGap + 20;
    }

    ui->selectedCard->setGeometry(0, 0, panelW, cardH);
    ui->globalCard->hide();
    ui->detailPanel->resize(panelW, cardH);
}

void MedicalWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    layoutUi();
}

bool MedicalWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (m_scroll && watched == m_scroll->viewport() && event->type() == QEvent::Resize)
        relayoutCards();
    return QMainWindow::eventFilter(watched, event);
}

void MedicalWindow::updateChips()
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

void MedicalWindow::updateDetailPanel()
{
    const int idx = indexOfRecord(m_selectedId);
    const bool has = idx >= 0;
    m_detailPlaceholder->setVisible(!has);
    m_detailContent->hide();           // remplacé par les lignes libellé / valeur
    if (!has) {
        m_detailValues.clear();
        return;
    }

    const MedicalRecord &r = m_records[idx];
    const bool upToDate = r.getStatutVaccination() == "À jour";

    m_detailValues = QStringList {
        QString("%1 (%2)").arg(r.getNomAnimal(), r.getIdAnimal()),
        QString("%1 · %2").arg(r.getEspece(), r.getRace()),
        QString("%1 an%2 · %3").arg(r.getAge()).arg(r.getAge() > 1 ? "s" : "", r.getSexe()),
        r.getProprietaire(),
        r.getIdDossier(),
        r.getDateConsultation().toString("dd/MM/yyyy"),
        r.getMotif(),
        r.getStatutVaccination()
    };
    m_btnToggle->setText(upToDate ? QString::fromUtf8("🔄  Vaccin à renouveler")
                                  : QString::fromUtf8("✅  Marquer à jour"));
}

void MedicalWindow::updateGlobalPanel()
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

QString MedicalWindow::nextRecordNumber(const QVector<MedicalRecord> &records)
{
    int maxNum = 0;
    for (const auto &r : records)
        maxNum = qMax(maxNum, r.getIdDossier().section('-', 1).toInt());
    return QString("MED-%1").arg(maxNum + 1, 3, 10, QChar('0'));
}

void MedicalWindow::viewRecord(const QString &recordId)
{
    const int idx = indexOfRecord(recordId);
    if (idx < 0)
        return;
    MedicalRecordDetailsDialog dialog(this);
    dialog.setMedicalRecord(m_records[idx]);
    const QString idAnimal = m_records[idx].getIdAnimal();
    connect(&dialog, &MedicalRecordDetailsDialog::pdfRequested, this, [this, idAnimal]() { exportAnimalPdf(idAnimal); });
    dialog.exec();
}

void MedicalWindow::editRecord(const QString &recordId)
{
    const int idx = indexOfRecord(recordId);
    if (idx < 0)
        return;
    MedicalRecordDialog dialog(this);
    dialog.setExistingRecords(m_records);
    dialog.setMedicalRecord(m_records[idx]);
    RendezVous currentRdv;
    if (RdvManager::prochain(m_records[idx].getIdAnimal(), m_records, currentRdv))
        dialog.setRendezVous(currentRdv);
    if (dialog.exec() == QDialog::Accepted) {
        MedicalRecord M = dialog.getMedicalRecord();
        m_selectedId = M.getIdDossier();

        if (!m_db) {            // mode hors ligne
            m_records[idx] = M;
            applyRdv(dialog, M);
            QTimer::singleShot(400, this, [this]() { checkHealthWatch(true); });
            refreshView();
            return;
        }

        bool test = M.modifier();   // UPDATE dans la table MEDICAL
        if (test) {
            // Refresh (Actualiser)
            m_records[idx] = M;     // garde les champs affichés seulement (sexe, traitement, ...)
            applyRdv(dialog, M);
            QTimer::singleShot(400, this, [this]() { checkHealthWatch(true); });
            actualiser();
            refreshView();
            QMessageBox::information(nullptr, QObject::tr("OK"),
                QObject::tr("Modification effectuée\n"
                            "Click Cancel to exit."), QMessageBox::Cancel);
        } else
            QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                QObject::tr("Modification non effectuée.\n\n%1\n\n"
                            "Click Cancel to exit.").arg(MedicalRecord::derniereErreur()), QMessageBox::Cancel);
    }
}

void MedicalWindow::deleteRecord(const QString &recordId)
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
                QObject::tr("Suppression non effectuée.\n\n%1\n\n"
                            "Click Cancel to exit.").arg(MedicalRecord::derniereErreur()), QMessageBox::Cancel);
    }
}

void MedicalWindow::onNavigationClicked(const QString &name)
{
    if (name == "Dossiers médicaux")
        return; // already here
    QMessageBox::information(this, "Navigation", QString("%1 - Non implémenté").arg(name));
}

void MedicalWindow::onLogoutClicked()
{
    const auto answer = QMessageBox::question(this, "Déconnexion",
        "Voulez-vous vraiment vous déconnecter ?");
    if (answer == QMessageBox::Yes)
        close();
}

void MedicalWindow::onAddRecordClicked()
{
    MedicalRecordDialog dialog(this);
    dialog.setExistingRecords(m_records);   // recherche d'animal par référence, diagnostics et traitements connus
    if (dialog.exec() == QDialog::Accepted) {
        // Objet MedicalRecord instancié avec les informations saisies dans le formulaire
        MedicalRecord M = dialog.getMedicalRecord();
        if (m_db) {             // en ligne : MAX(ID_DOSSIER)+1 lu dans la base (évite les doublons de clé primaire)
            const qlonglong n = MedicalRecord::prochainIdDossier();
            M.setIdDossier(n > 0 ? QString("MED-%1").arg(n, 3, 10, QChar('0')) : nextRecordNumber(m_records));
        } else if (M.getIdDossier().isEmpty() || indexOfRecord(M.getIdDossier()) >= 0) {
            M.setIdDossier(nextRecordNumber(m_records));
        }

        if (!m_db) {            // mode hors ligne
            m_records.append(M);
            applyRdv(dialog, M);
            QTimer::singleShot(400, this, [this]() { checkHealthWatch(true); });
            m_selectedId = M.getIdDossier();
            refreshView();
            return;
        }

        bool test = M.ajouter();   // Insérer l'objet dans la table MEDICAL et récupérer le retour de query.exec()
        if (test) {                // Si requête exécutée --> QMessageBox::information
            // Refresh (Actualiser)
            m_records.append(M);   // garde les champs affichés seulement (sexe, traitement, ...)
            applyRdv(dialog, M);
            QTimer::singleShot(400, this, [this]() { checkHealthWatch(true); });
            actualiser();
            m_selectedId = M.getIdDossier();
            refreshView();
            QMessageBox::information(nullptr, QObject::tr("OK"),
                QObject::tr("Ajout effectué\n"
                            "Click Cancel to exit."), QMessageBox::Cancel);
        } else                     // Si requête non exécutée ==> QMessageBox::critical
            QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                QObject::tr("Ajout non effectué.\n\n%1\n\n"
                            "Click Cancel to exit.").arg(MedicalRecord::derniereErreur()), QMessageBox::Cancel);
    }
}

void MedicalWindow::onViewRecordClicked()   { viewRecord(m_selectedId); }
void MedicalWindow::onEditRecordClicked()   { editRecord(m_selectedId); }
void MedicalWindow::onDeleteRecordClicked() { deleteRecord(m_selectedId); }

void MedicalWindow::onToggleVaccinationClicked()
{
    const int idx = indexOfRecord(m_selectedId);
    if (idx < 0)
        return;
    // Le statut de vaccination n'a pas de colonne dans MEDICAL : il change en mémoire seulement.
    const QString status = m_records[idx].getStatutVaccination();
    m_records[idx].setStatutVaccination(status == "À jour" ? "À renouveler" : "À jour");
    refreshView();
}

// ---------------------------------------------------------------- DM-I1 : rappels vaccinaux
void MedicalWindow::syncVaccinationStatus()
{
    for (MedicalRecord &r : m_records) {
        const QString st = VaccinationManager::statutAnimal(r.getIdAnimal(), m_records);
        if (!st.isEmpty())
            r.setStatutVaccination(st);
    }
}

void MedicalWindow::updateReminderButton()
{
    if (!m_btnReminders)
        return;
    const int n = VaccinationManager::aRappeler(m_records).size() + m_rdvPending;
    m_btnReminders->setText(n > 0 ? QString::fromUtf8("🔔  Rappels (%1)").arg(n)
                                  : QString::fromUtf8("🔔  Rappels"));
    updateWatchButton();         // badge de la veille sanitaire (même moment de rafraîchissement)
}

void MedicalWindow::showStartupReminders()
{
    ReminderDialog::envoyerAutomatiquement(m_records, m_sms);     // seulement si l'option est cochée et la passerelle configurée
    if (!VaccinationManager::aRappeler(m_records).isEmpty())
        onRemindersClicked();
}

void MedicalWindow::onRemindersClicked()
{
    ReminderDialog dialog(m_records, this);
    dialog.exec();
    syncVaccinationStatus();      // les vaccins renouvelés / ajoutés changent le statut des animaux
    refreshView();
}

void MedicalWindow::onRefreshClicked()
{
    refreshView();
}

void MedicalWindow::onFilterChanged()
{
    refreshView();
}

void MedicalWindow::onResetFiltersClicked()
{
    const QSignalBlocker b1(m_searchEdit), b2(m_filterSpecies), b3(m_filterVaccination), b4(m_sortCombo);
    m_searchEdit->clear();
    m_filterSpecies->setCurrentIndex(0);
    m_filterVaccination->setCurrentIndex(0);
    m_sortCombo->setCurrentIndex(0);
    refreshView();
}

void MedicalWindow::onCardClicked(const QString &recordId)
{
    m_selectedId = recordId;
    for (RecordCard *c : m_cards)
        c->setSelected(c->recordId() == recordId);
    updateDetailPanel();
    layoutDetailPanel();
}

void MedicalWindow::onShowStatisticsClicked()
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
    QPixmap logoPix(":/med/assets/logo.png");
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
    auto *tabs = new QTabWidget;
    tabs->setObjectName("statisticsTabs");
    auto *pageVacc = new QWidget;
    auto *vLay = new QVBoxLayout(pageVacc);
    vLay->setContentsMargins(0, 12, 0, 0);
    vLay->setSpacing(10);
    vLay->addWidget(sectionTitle);

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
    vLay->addWidget(table, 1);
    tabs->addTab(pageVacc, "Vaccinations");

    // ---- Onglet Diagnostics : courbe du nombre de cas par mois ----
    const QVector<MedicalRecord> records = m_records;
    QMap<QString, int> diagCount;            // clé en minuscules (regroupe "Otite" et "otite")
    QMap<QString, QString> diagName;
    for (const auto &r : records) {
        const QString d = r.getDiagnostic().trimmed();
        if (d.isEmpty()) continue;
        diagCount[d.toLower()]++;
        if (!diagName.contains(d.toLower())) diagName[d.toLower()] = d;
    }
    QList<QPair<int, QString>> diagSorted;   // (nombre de cas, nom), du plus fréquent au moins fréquent
    for (auto it = diagCount.cbegin(); it != diagCount.cend(); ++it)
        diagSorted.append({ it.value(), diagName[it.key()] });
    std::sort(diagSorted.begin(), diagSorted.end(), [](const auto &a, const auto &b) {
        return a.first != b.first ? a.first > b.first : a.second.localeAwareCompare(b.second) < 0; });

    auto *pageDiag = new QWidget;
    auto *dLay = new QVBoxLayout(pageDiag);
    dLay->setContentsMargins(0, 12, 0, 0);
    dLay->setSpacing(10);

    auto *diagTop = new QHBoxLayout;
    auto *diagTitle = new QLabel("Évolution des diagnostics");
    diagTitle->setObjectName("statisticsSectionTitle");
    auto *diagCombo = new QComboBox;
    diagCombo->setMinimumWidth(260);
    diagCombo->addItem("Tous les diagnostics");
    for (const auto &p : diagSorted)
        diagCombo->addItem(p.second);
    diagTop->addWidget(diagTitle);
    diagTop->addStretch();
    diagTop->addWidget(diagCombo);
    dLay->addLayout(diagTop);

    auto *diagRow = new QHBoxLayout;
    diagRow->setSpacing(14);
    auto *curve = new StatsCurveWidget;
    diagRow->addWidget(curve, 1);

    auto *diagTable = new QTableWidget;
    diagTable->setObjectName("statisticsTable");
    diagTable->setColumnCount(2);
    diagTable->setHorizontalHeaderLabels({"Diagnostic", "Cas"});
    diagTable->setRowCount(diagSorted.size());
    diagTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    diagTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    diagTable->setAlternatingRowColors(true);
    diagTable->verticalHeader()->setVisible(false);
    diagTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    diagTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    diagTable->setFixedWidth(280);
    for (int i = 0; i < diagSorted.size(); ++i) {
        diagTable->setItem(i, 0, new QTableWidgetItem(diagSorted[i].second));
        auto *n = new QTableWidgetItem(QString::number(diagSorted[i].first));
        n->setTextAlignment(Qt::AlignCenter);
        diagTable->setItem(i, 1, n);
        diagTable->setRowHeight(i, 40);
    }
    diagRow->addWidget(diagTable);
    dLay->addLayout(diagRow, 1);

    // Recalcule la courbe selon le diagnostic choisi (ou tous)
    auto refreshCurve = [=]() {
        const QString sel = diagCombo->currentIndex() > 0 ? diagCombo->currentText().toLower() : QString();
        QDate firstM, lastM = QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1);
        for (const auto &r : records) {
            const QDate d = r.getDateConsultation();
            if (!d.isValid()) continue;
            const QDate m(d.year(), d.month(), 1);
            if (!firstM.isValid() || m < firstM) firstM = m;
            if (m > lastM) lastM = m;
        }
        // fenêtre de 6 à 12 mois se terminant au dernier mois connu
        QDate startM = lastM.addMonths(-5);
        if (firstM.isValid() && firstM < startM) startM = qMax(firstM, lastM.addMonths(-11));

        QMap<QDate, int> perMonth;
        for (const auto &r : records) {
            const QDate d = r.getDateConsultation();
            if (!d.isValid()) continue;
            if (!sel.isEmpty() && r.getDiagnostic().trimmed().toLower() != sel) continue;
            perMonth[QDate(d.year(), d.month(), 1)]++;
        }
        QStringList labels;
        QVector<int> values;
        for (QDate m = startM; m <= lastM; m = m.addMonths(1)) {
            labels << m.toString("MM/yy");
            values << perMonth.value(m, 0);
        }
        curve->setData(labels, values,
                       sel.isEmpty() ? QString("Nombre de consultations par mois")
                                     : QString("Nombre de cas par mois : %1").arg(diagCombo->currentText()));
    };
    connect(diagCombo, &QComboBox::currentIndexChanged, &dialog, refreshCurve);
    connect(diagTable, &QTableWidget::cellClicked, &dialog, [diagCombo](int row, int) {
        diagCombo->setCurrentIndex(row + 1);
    });
    refreshCurve();

    tabs->addTab(pageDiag, "Diagnostics");
    root->addWidget(tabs, 1);

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

void MedicalWindow::onExportCsvClicked()
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


// ---------------------------------------------------------------- Rendez-vous + SMS automatiques
void MedicalWindow::applyRdv(const MedicalRecordDialog &dialog, const MedicalRecord &record)
{
    RendezVous current;
    const bool had = RdvManager::prochain(record.getIdAnimal(), m_records, current);

    if (!dialog.rdvPlanifie()) {
        if (had)
            RdvManager::changerStatut(current.idRdv, "Annulé");     // case décochée : le rendez-vous est annulé
        updateRdvButton(RdvManager::aEnvoyer(m_records).size());
        return;
    }
    // rien à changer : on garde le rendez-vous (et ses SMS déjà envoyés)
    if (had && current.dateRdv == dialog.rdvDate() && current.telephone == dialog.telephone())
        return;

    RendezVous r;
    r.idAnimal = record.getIdAnimal();
    r.nomAnimal = record.getNomAnimal();
    r.proprietaire = record.getProprietaire();
    r.telephone = dialog.telephone();
    r.motif = record.getMotif().left(200);
    r.dateRdv = dialog.rdvDate();
    RdvManager::planifier(r, m_records);
    updateRdvButton(RdvManager::aEnvoyer(m_records).size());
}

void MedicalWindow::updateRdvButton(int pending)
{
    m_rdvPending = pending;
    updateReminderButton();      // un seul bouton "Rappels" : vaccins + rendez-vous
}

void MedicalWindow::checkAppointments()
{
    const QList<RdvSmsAction> actions = RdvManager::aEnvoyer(m_records);   // met aussi les statuts à jour
    updateRdvButton(actions.size());
    if (!m_sms->isConfigured())
        return;      // simulation : rien n'est envoyé automatiquement (envoi manuel dans la fenêtre RDV)

    for (const RdvSmsAction &a : actions) {
        if (SmsService::normalizePhone(a.rdv.telephone).isEmpty())
            continue;
        m_sms->send(a.rdv.telephone,
                    a.manque ? SmsService::composeRdvManque(a.rdv) : SmsService::composeRdvRappel(a.rdv),
                    QString("RDV|%1|%2").arg(a.rdv.idRdv, a.manque ? "M" : "R"));
    }
}

void MedicalWindow::onRdvClicked()
{
    AppointmentDialog dialog(m_records, this);
    dialog.exec();
    checkAppointments();
}

// ---------------------------------------------------------------- Fiche PDF de l'animal
void MedicalWindow::onExportPdfClicked()
{
    const int idx = indexOfRecord(m_selectedId);
    if (idx < 0) {
        QMessageBox::information(this, "Fiche PDF", "Sélectionnez d'abord un dossier (carte) de l'animal.");
        return;
    }
    exportAnimalPdf(m_records[idx].getIdAnimal());
}

void MedicalWindow::exportAnimalPdf(const QString &idAnimal)
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString path = QFileDialog::getSaveFileName(this, "Enregistrer la fiche de l'animal",
        dir + "/" + AnimalPdf::nomFichierPropose(idAnimal, m_records), "PDF (*.pdf)");
    if (path.isEmpty())
        return;
    if (!AnimalPdf::exporter(idAnimal, m_records, path)) {
        QMessageBox::warning(this, "Fiche PDF", "Impossible de créer le fichier PDF.");
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));      // ouvre le PDF directement
}


// ---------------------------------------------------------------- Veille sanitaire
void MedicalWindow::updateWatchButton()
{
    if (!m_btnWatch)
        return;
    const int n = HealthWatch::nonVues(HealthWatch::analyser(m_records)).size();
    m_btnWatch->setText(n > 0 ? QString::fromUtf8("🩺  Veille (%1)").arg(n) : QString::fromUtf8("🩺  Veille"));
}

void MedicalWindow::checkHealthWatch(bool popup)
{
    updateWatchButton();
    if (!popup)
        return;
    if (QApplication::activeModalWidget()) {          // une autre fenêtre est ouverte : on réessaie plus tard
        QTimer::singleShot(4000, this, [this]() { checkHealthWatch(true); });
        return;
    }
    if (!HealthWatch::nonVues(HealthWatch::analyser(m_records)).isEmpty())
        onWatchClicked();                              // nouvelle alerte : la fenêtre s'ouvre toute seule
}

void MedicalWindow::onWatchClicked()
{
    HealthWatchDialog dialog(m_records, this);
    dialog.exec();
    updateWatchButton();
}
