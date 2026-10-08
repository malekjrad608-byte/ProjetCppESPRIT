#include "medicalrecordswidget.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include "../database/databasemanager.h"
#include "../dialogs/medicalaccessdialog.h"
#include "animaldialog.h"
#include "medicalrecorddialog.h"
#include "vaccinationdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QDesktopServices>
#include <QPair>
#include <algorithm>

MedicalRecordsWidget::MedicalRecordsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    applyStyles();
    refresh();
}

void MedicalRecordsWidget::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Top bar
    auto *topBar = new QFrame;
    topBar->setObjectName("topbar");
    topBar->setFixedHeight(64);
    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(28, 0, 28, 0);

    auto *breadcrumb = Design::label("Espace de gestion   /   Animaux & dossiers médicaux", "muted");
    topLayout->addWidget(breadcrumb);
    topLayout->addStretch();

    // Dashboard button
    auto *dashboardBtn = Design::button("Tableau de bord", "grid");
    dashboardBtn->setObjectName("navDashboard");
    dashboardBtn->setCheckable(true);
    connect(dashboardBtn, &QPushButton::clicked, this, &MedicalRecordsWidget::showDashboard);
    topLayout->addWidget(dashboardBtn);

    // Animals list button
    auto *animalsBtn = Design::button("Liste des animaux", "list");
    animalsBtn->setObjectName("navAnimals");
    animalsBtn->setCheckable(true);
    connect(animalsBtn, &QPushButton::clicked, this, &MedicalRecordsWidget::showAnimalsList);
    topLayout->addWidget(animalsBtn);

    topLayout->addSpacing(8);

    // Export buttons
    auto *exportPDF = Design::button("Export PDF", "document");
    connect(exportPDF, &QPushButton::clicked, this, &MedicalRecordsWidget::onExportPDF);
    topLayout->addWidget(exportPDF);

    auto *exportExcel = Design::button("Export Excel", "spreadsheet");
    connect(exportExcel, &QPushButton::clicked, this, &MedicalRecordsWidget::onExportExcel);
    topLayout->addWidget(exportExcel);

    root->addWidget(topBar);

    // Stacked widget for pages
    m_stack = new QStackedWidget;
    m_stack->setObjectName("pageStack");
    m_stack->setAttribute(Qt::WA_OpaquePaintEvent);
    m_stack->setAutoFillBackground(true);
    QPalette colors = m_stack->palette();
    colors.setColor(QPalette::Window, QColor("#FFFDF9"));
    m_stack->setPalette(colors);

    setupDashboard();
    setupAnimalsList();
    setupAnimalDetails();

    m_stack->addWidget(m_dashboardPage);
    m_stack->addWidget(m_animalsListPage);
    m_stack->addWidget(m_animalDetailsPage);

    root->addWidget(m_stack, 1);
}

void MedicalRecordsWidget::setupDashboard()
{
    m_dashboardPage = new QWidget;
    auto *layout = new QVBoxLayout(m_dashboardPage);
    layout->setContentsMargins(48, 40, 48, 40);
    layout->setSpacing(24);

    auto *eyebrow = new QLabel("DOSSIERS MÉDICAUX");
    eyebrow->setObjectName("eyebrow");
    layout->addWidget(eyebrow);

    auto *title = Design::label("Tableau de bord", "title");
    layout->addWidget(title);

    auto *subtitle = Design::label("Vue d'ensemble des dossiers médicaux et des vaccinations", "muted");
    layout->addWidget(subtitle);

    layout->addSpacing(20);

    // Stats cards
    auto *statsLayout = new QGridLayout;
    statsLayout->setSpacing(20);

    m_totalAnimals = createStatCard("Animaux", "0", "shield");
    m_totalRecords = createStatCard("Dossiers médicaux", "0", "document");
    m_totalVaccinations = createStatCard("Vaccinations", "0", "syringe");
    m_vaccinationsDueSoon = createStatCard("À venir (7j)", "0", "clock");
    m_vaccinationsOverdue = createStatCard("En retard", "0", "alert");
    m_totalCost = createStatCard("Coût total", "0 DT", "currency");

    statsLayout->addWidget(m_totalAnimals, 0, 0);
    statsLayout->addWidget(m_totalRecords, 0, 1);
    statsLayout->addWidget(m_totalVaccinations, 0, 2);
    statsLayout->addWidget(m_vaccinationsDueSoon, 1, 0);
    statsLayout->addWidget(m_vaccinationsOverdue, 1, 1);
    statsLayout->addWidget(m_totalCost, 1, 2);

    layout->addLayout(statsLayout);
    layout->addStretch();
}

void MedicalRecordsWidget::setupAnimalsList()
{
    m_animalsListPage = new QWidget;
    auto *layout = new QVBoxLayout(m_animalsListPage);
    layout->setContentsMargins(48, 40, 48, 40);
    layout->setSpacing(24);

    // Title and actions
    auto *header = new QHBoxLayout;
    auto *eyebrow = new QLabel("ANIMAUX");
    eyebrow->setObjectName("eyebrow");
    header->addWidget(eyebrow);
    header->addStretch();

    auto *addBtn = Design::button("Ajouter un animal", "plus", "primary");
    addBtn->setObjectName("addAnimal");
    connect(addBtn, &QPushButton::clicked, this, &MedicalRecordsWidget::onAddAnimal);
    header->addWidget(addBtn);

    layout->addLayout(header);

    auto *title = Design::label("Animaux enregistrés", "title");
    layout->addWidget(title);

    layout->addSpacing(16);

    // Search and filters
    auto *filterLayout = new QHBoxLayout;
    filterLayout->setSpacing(12);

    m_searchInput = new QLineEdit;
    m_searchInput->setPlaceholderText("Rechercher par nom, espèce, race, propriétaire...");
    m_searchInput->setObjectName("medicalSearch");
    connect(m_searchInput, &QLineEdit::textChanged, this, &MedicalRecordsWidget::onSearch);
    filterLayout->addWidget(m_searchInput, 2);

    m_speciesFilter = new QComboBox;
    m_speciesFilter->addItem("Toutes les espèces");
    m_speciesFilter->addItems({"Chien", "Chat", "Oiseau", "Rongeur", "Reptile", "Autre"});
    m_speciesFilter->setObjectName("speciesFilter");
    connect(m_speciesFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MedicalRecordsWidget::onFilterChanged);
    filterLayout->addWidget(m_speciesFilter, 1);

    m_sortCombo = new QComboBox;
    m_sortCombo->addItem("Tri: Nom (A-Z)");
    m_sortCombo->addItem("Tri: Nom (Z-A)");
    m_sortCombo->addItem("Tri: Date d'ajout (récent)");
    m_sortCombo->addItem("Tri: Date d'ajout (ancien)");
    m_sortCombo->setObjectName("sortCombo");
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MedicalRecordsWidget::onSortChanged);
    filterLayout->addWidget(m_sortCombo, 1);

    layout->addLayout(filterLayout);

    // Animals table
    m_animalsTable = new QTableWidget;
    m_animalsTable->setObjectName("animalsTable");
    m_animalsTable->setColumnCount(7);
    m_animalsTable->setHorizontalHeaderLabels({"ID", "Nom", "Espèce", "Race", "Propriétaire", "Date naissance", "Actions"});
    m_animalsTable->horizontalHeader()->setStretchLastSection(true);
    m_animalsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_animalsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_animalsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_animalsTable->verticalHeader()->setVisible(false);
    connect(m_animalsTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        const int id = m_animalsTable->item(row, 0)->text().toInt();
        showAnimalDetails(id);
    });

    layout->addWidget(m_animalsTable, 1);
}

void MedicalRecordsWidget::setupAnimalDetails()
{
    m_animalDetailsPage = new QWidget;
    auto *layout = new QVBoxLayout(m_animalDetailsPage);
    layout->setContentsMargins(48, 40, 48, 40);
    layout->setSpacing(24);

    // Header with back button
    auto *header = new QHBoxLayout;
    auto *backBtn = Design::button("← Retour", "arrow-left");
    connect(backBtn, &QPushButton::clicked, this, &MedicalRecordsWidget::showAnimalsList);
    header->addWidget(backBtn);

    auto *eyebrow = new QLabel("DOSSIER");
    eyebrow->setObjectName("eyebrow");
    header->addWidget(eyebrow);
    header->addStretch();

    auto *editBtn = Design::button("Modifier", "edit");
    editBtn->setObjectName("editAnimal");
    connect(editBtn, &QPushButton::clicked, this, &MedicalRecordsWidget::onEditAnimal);
    header->addWidget(editBtn);

    auto *deleteBtn = Design::button("Supprimer", "trash");
    deleteBtn->setObjectName("deleteAnimal");
    connect(deleteBtn, &QPushButton::clicked, this, &MedicalRecordsWidget::onDeleteAnimal);
    header->addWidget(deleteBtn);

    layout->addLayout(header);

    m_animalName = Design::label("", "title");
    layout->addWidget(m_animalName);

    // Animal info
    m_animalInfo = Design::label("", "muted");
    m_animalInfo->setWordWrap(true);
    layout->addWidget(m_animalInfo);

    layout->addSpacing(16);

    // Medical records section
    auto *recordsHeader = new QHBoxLayout;
    recordsHeader->addWidget(Design::label("Historique médical", "subtitle"));
    recordsHeader->addStretch();

    auto *addRecordBtn = Design::button("Ajouter dossier", "plus", "primary");
    connect(addRecordBtn, &QPushButton::clicked, this, &MedicalRecordsWidget::onAddMedicalRecord);
    recordsHeader->addWidget(addRecordBtn);

    layout->addLayout(recordsHeader);

    m_medicalRecordsTable = new QTableWidget;
    m_medicalRecordsTable->setObjectName("medicalRecordsTable");
    m_medicalRecordsTable->setColumnCount(6);
    m_medicalRecordsTable->setHorizontalHeaderLabels({"Date", "Type", "Vétérinaire", "Diagnostic", "Traitement", "Coût"});
    m_medicalRecordsTable->horizontalHeader()->setStretchLastSection(true);
    m_medicalRecordsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_medicalRecordsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_medicalRecordsTable->verticalHeader()->setVisible(false);
    layout->addWidget(m_medicalRecordsTable, 1);

    layout->addSpacing(20);

    // Vaccinations section
    auto *vaccHeader = new QHBoxLayout;
    vaccHeader->addWidget(Design::label("Vaccinations", "subtitle"));
    vaccHeader->addStretch();

    auto *addVaccBtn = Design::button("Ajouter vaccination", "syringe", "primary");
    connect(addVaccBtn, &QPushButton::clicked, this, &MedicalRecordsWidget::onAddVaccination);
    vaccHeader->addWidget(addVaccBtn);

    layout->addLayout(vaccHeader);

    m_vaccinationsTable = new QTableWidget;
    m_vaccinationsTable->setObjectName("vaccinationsTable");
    m_vaccinationsTable->setColumnCount(5);
    m_vaccinationsTable->setHorizontalHeaderLabels({"Vaccin", "Type", "Administré le", "Prochaine dose", "Statut"});
    m_vaccinationsTable->horizontalHeader()->setStretchLastSection(true);
    m_vaccinationsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_vaccinationsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_vaccinationsTable->verticalHeader()->setVisible(false);
    layout->addWidget(m_vaccinationsTable, 1);
}

QLabel *MedicalRecordsWidget::createStatCard(const QString &label, const QString &value, const QString &icon)
{
    auto *card = Design::card();
    card->setObjectName("medicalStatCard");
    card->setStyleSheet("QFrame#medicalStatCard { background: #FFFFFF; border-radius: 16px; border: 1px solid #E2EBE9; } QFrame#medicalStatCard:hover { border-color: #C9DDE4; background-color: #FAFCFD; }");

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto *top = new QHBoxLayout;
    auto *iconLabel = Design::label("");
    iconLabel->setPixmap(Design::icon(icon).pixmap(28, 28));
    top->addWidget(iconLabel);
    top->addStretch();

    auto *valueLabel = Design::label(value, "title");
    valueLabel->setStyleSheet("font-size: 32px; font-weight: 700; color: #263D43;");
    top->addWidget(valueLabel);

    layout->addLayout(top);

    auto *labelWidget = Design::label(label, "muted");
    labelWidget->setStyleSheet("font-size: 13px; color: #73868A; font-weight: 600;");
    layout->addWidget(labelWidget);

    return card;
}

void MedicalRecordsWidget::refresh()
{
    m_animals = DatabaseManager::instance().getAllAnimals();
    m_analytics = DatabaseManager::instance().getMedicalAnalytics();

    updateAnalytics();
    populateAnimalsTable();
    applyFilters();
}

void MedicalRecordsWidget::updateAnalytics()
{
    m_totalAnimals->setText(QString::number(m_analytics.totalAnimals));
    m_totalRecords->setText(QString::number(m_analytics.totalRecords));
    m_totalVaccinations->setText(QString::number(m_analytics.totalVaccinations));
    m_vaccinationsDueSoon->setText(QString::number(m_analytics.vaccinationsDueSoon));
    m_vaccinationsOverdue->setText(QString::number(m_analytics.vaccinationsOverdue));
    m_totalCost->setText(QString::number(m_analytics.totalCost, 'f', 2) + " DT");
}

void MedicalRecordsWidget::populateAnimalsTable()
{
    m_animalsTable->setRowCount(0);

    for (const auto &animal : m_filteredAnimals) {
        const int row = m_animalsTable->rowCount();
        m_animalsTable->insertRow(row);

        m_animalsTable->setItem(row, 0, new QTableWidgetItem(QString::number(animal.id)));
        m_animalsTable->setItem(row, 1, new QTableWidgetItem(animal.name));
        m_animalsTable->setItem(row, 2, new QTableWidgetItem(animal.species));
        m_animalsTable->setItem(row, 3, new QTableWidgetItem(animal.breed));
        m_animalsTable->setItem(row, 4, new QTableWidgetItem(animal.ownerId));
        m_animalsTable->setItem(row, 5, new QTableWidgetItem(animal.birthDate.isValid() ? animal.birthDate.toString("dd/MM/yyyy") : "-"));

        auto *actionsWidget = new QWidget;
        auto *actionsLayout = new QHBoxLayout(actionsWidget);
        actionsLayout->setContentsMargins(5, 2, 5, 2);
        actionsLayout->setSpacing(5);

        auto *viewBtn = new QPushButton("Voir");
        viewBtn->setObjectName("viewBtn");
        viewBtn->setMaximumSize(50, 28);
        connect(viewBtn, &QPushButton::clicked, this, [this, row]() {
            showAnimalDetails(m_filteredAnimals[row].id);
        });
        actionsLayout->addWidget(viewBtn);

        m_animalsTable->setCellWidget(row, 6, actionsWidget);
    }

    m_animalsTable->resizeColumnsToContents();
}

void MedicalRecordsWidget::populateMedicalRecordsTable(int animalId)
{
    m_medicalRecordsTable->setRowCount(0);

    QList<MedicalRecord> records = DatabaseManager::instance().getMedicalRecords(animalId);

    for (const auto &record : records) {
        const int row = m_medicalRecordsTable->rowCount();
        m_medicalRecordsTable->insertRow(row);

        m_medicalRecordsTable->setItem(row, 0, new QTableWidgetItem(record.date.toString("dd/MM/yyyy HH:mm")));
        m_medicalRecordsTable->setItem(row, 1, new QTableWidgetItem(record.type));
        m_medicalRecordsTable->setItem(row, 2, new QTableWidgetItem(record.veterinarian));
        m_medicalRecordsTable->setItem(row, 3, new QTableWidgetItem(record.diagnosis));
        m_medicalRecordsTable->setItem(row, 4, new QTableWidgetItem(record.treatment));
        m_medicalRecordsTable->setItem(row, 5, new QTableWidgetItem(QString::number(record.cost, 'f', 2) + " DT"));
    }

    m_medicalRecordsTable->resizeColumnsToContents();
}

void MedicalRecordsWidget::populateVaccinationsTable(int animalId)
{
    m_vaccinationsTable->setRowCount(0);

    QList<Vaccination> vaccinations = DatabaseManager::instance().getVaccinations(animalId);

    for (const auto &vacc : vaccinations) {
        const int row = m_vaccinationsTable->rowCount();
        m_vaccinationsTable->insertRow(row);

        m_vaccinationsTable->setItem(row, 0, new QTableWidgetItem(vacc.vaccineName));
        m_vaccinationsTable->setItem(row, 1, new QTableWidgetItem(vacc.vaccineType));
        m_vaccinationsTable->setItem(row, 2, new QTableWidgetItem(vacc.administeredDate.toString("dd/MM/yyyy")));
        m_vaccinationsTable->setItem(row, 3, new QTableWidgetItem(vacc.nextDueDate.isValid() ? vacc.nextDueDate.toString("dd/MM/yyyy") : "-"));

        QString status;
        if (vacc.isOverdue()) {
            status = "En retard";
        } else if (vacc.isDueSoon(7)) {
            status = "À venir";
        } else {
            status = "À jour";
        }

        auto *statusItem = new QTableWidgetItem(status);
        if (vacc.isOverdue()) {
            statusItem->setForeground(QColor("#B25C4A"));
        } else if (vacc.isDueSoon(7)) {
            statusItem->setForeground(QColor("#E67E22"));
        } else {
            statusItem->setForeground(QColor("#27AE60"));
        }
        m_vaccinationsTable->setItem(row, 4, statusItem);
    }

    m_vaccinationsTable->resizeColumnsToContents();
}

void MedicalRecordsWidget::applyFilters()
{
    m_filteredAnimals = m_animals;

    // Search filter
    const QString search = m_searchInput->text().trimmed().toLower();
    if (!search.isEmpty()) {
        m_filteredAnimals = DatabaseManager::instance().searchAnimals(search);
    }

    // Species filter
    const QString species = m_speciesFilter->currentText();
    if (species != "Toutes les espèces") {
        QList<Animal> filtered;
        for (const auto &animal : m_filteredAnimals) {
            if (animal.species == species) {
                filtered.append(animal);
            }
        }
        m_filteredAnimals = filtered;
    }

    // Sort
    const int sortIndex = m_sortCombo->currentIndex();
    std::sort(m_filteredAnimals.begin(), m_filteredAnimals.end(), [sortIndex](const Animal &a, const Animal &b) {
        switch (sortIndex) {
            case 0: return a.name.toLower() < b.name.toLower();
            case 1: return a.name.toLower() > b.name.toLower();
            case 2: return a.createdAt > b.createdAt;
            case 3: return a.createdAt < b.createdAt;
            default: return a.name.toLower() < b.name.toLower();
        }
    });

    populateAnimalsTable();
}

void MedicalRecordsWidget::onSearch()
{
    applyFilters();
}

void MedicalRecordsWidget::onFilterChanged()
{
    applyFilters();
}

void MedicalRecordsWidget::onSortChanged()
{
    applyFilters();
}

void MedicalRecordsWidget::onAddAnimal()
{
    AnimalDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Animal animal = dialog.getAnimal();
        if (DatabaseManager::instance().addAnimal(animal)) {
            refresh();
            emit recordsChanged();
        } else {
            QMessageBox::critical(this, "Erreur", DatabaseManager::instance().lastError());
        }
    }
}

void MedicalRecordsWidget::onEditAnimal()
{
    if (m_currentAnimal.isEmpty()) return;

    AnimalDialog dialog(m_currentAnimal, false, this);
    if (dialog.exec() == QDialog::Accepted) {
        Animal animal = dialog.getAnimal();
        animal.id = m_currentAnimal.id;
        if (DatabaseManager::instance().updateAnimal(animal)) {
            refresh();
            showAnimalDetails(animal.id);
            emit recordsChanged();
        } else {
            QMessageBox::critical(this, "Erreur", DatabaseManager::instance().lastError());
        }
    }
}

void MedicalRecordsWidget::onDeleteAnimal()
{
    if (m_currentAnimal.isEmpty()) return;

    auto reply = QMessageBox::question(this, "Confirmation",
        "Êtes-vous sûr de vouloir supprimer cet animal et tous ses dossiers médicaux ?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteAnimal(m_currentAnimal.id)) {
            refresh();
            showAnimalsList();
            emit recordsChanged();
        } else {
            QMessageBox::critical(this, "Erreur", DatabaseManager::instance().lastError());
        }
    }
}

void MedicalRecordsWidget::onAddMedicalRecord()
{
    if (m_currentAnimal.isEmpty()) return;

    MedicalRecordDialog dialog(m_currentAnimal.id, this);
    if (dialog.exec() == QDialog::Accepted) {
        MedicalRecord record = dialog.getRecord();
        if (DatabaseManager::instance().addMedicalRecord(record)) {
            refresh();
            showAnimalDetails(m_currentAnimal.id);
            emit recordsChanged();
        } else {
            QMessageBox::critical(this, "Erreur", DatabaseManager::instance().lastError());
        }
    }
}

void MedicalRecordsWidget::onAddVaccination()
{
    if (m_currentAnimal.isEmpty()) return;

    VaccinationDialog dialog(m_currentAnimal.id, this);
    if (dialog.exec() == QDialog::Accepted) {
        Vaccination vacc = dialog.getVaccination();
        if (DatabaseManager::instance().addVaccination(vacc)) {
            refresh();
            showAnimalDetails(m_currentAnimal.id);
            emit recordsChanged();
        } else {
            QMessageBox::critical(this, "Erreur", DatabaseManager::instance().lastError());
        }
    }
}

void MedicalRecordsWidget::showDashboard()
{
    m_stack->setCurrentIndex(0);
    updateNavigationButtons();
    refresh();
}

void MedicalRecordsWidget::showAnimalsList()
{
    m_stack->setCurrentIndex(1);
    updateNavigationButtons();
    populateAnimalsTable();
}

void MedicalRecordsWidget::showAnimalDetails(int animalId)
{
    m_currentAnimal = DatabaseManager::instance().getAnimalById(animalId);
    if (m_currentAnimal.isEmpty()) return;

    m_animalName->setText(m_currentAnimal.name);

    QString info = QString("Espèce: %1 | Race: %2 | Sexe: %3 | Né le: %4\n")
        .arg(m_currentAnimal.species)
        .arg(m_currentAnimal.breed)
        .arg(m_currentAnimal.gender)
        .arg(m_currentAnimal.birthDate.isValid() ? m_currentAnimal.birthDate.toString("dd/MM/yyyy") : "-");

    if (!m_currentAnimal.ownerId.isEmpty()) {
        info += QString("Propriétaire: %1").arg(m_currentAnimal.ownerId);
        if (!m_currentAnimal.ownerPhone.isEmpty()) {
            info += QString(" | Tél: %1").arg(m_currentAnimal.ownerPhone);
        }
    }

    m_animalInfo->setText(info);

    populateMedicalRecordsTable(animalId);
    populateVaccinationsTable(animalId);

    m_stack->setCurrentIndex(2);
    updateNavigationButtons();
}

void MedicalRecordsWidget::onExportPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", "", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    QTextCursor cursor(&doc);

    // Title
    QTextCharFormat titleFormat;
    titleFormat.setFontPointSize(18);
    titleFormat.setFontWeight(QFont::Bold);
    cursor.setCharFormat(titleFormat);
    cursor.insertText("Rapport Dossiers Médicaux - ANIWORLD\n\n");

    // Analytics
    QTextCharFormat headerFormat;
    headerFormat.setFontPointSize(14);
    headerFormat.setFontWeight(QFont::Bold);
    cursor.setCharFormat(headerFormat);
    cursor.insertText("Statistiques\n");

    QTextCharFormat normalFormat;
    normalFormat.setFontPointSize(10);
    cursor.setCharFormat(normalFormat);
    cursor.insertText(QString("Animaux: %1\n").arg(m_analytics.totalAnimals));
    cursor.insertText(QString("Dossiers médicaux: %1\n").arg(m_analytics.totalRecords));
    cursor.insertText(QString("Vaccinations: %1\n").arg(m_analytics.totalVaccinations));
    cursor.insertText(QString("Vaccinations à venir: %1\n").arg(m_analytics.vaccinationsDueSoon));
    cursor.insertText(QString("Vaccinations en retard: %1\n").arg(m_analytics.vaccinationsOverdue));
    cursor.insertText(QString("Coût total: %1 DT\n\n").arg(m_analytics.totalCost, 0, 'f', 2));

    // Animals table
    cursor.setCharFormat(headerFormat);
    cursor.insertText("Liste des animaux\n\n");

    QTextTableFormat tableFormat;
    tableFormat.setBorder(1);
    tableFormat.setCellPadding(5);
    tableFormat.setCellSpacing(0);

    QTextTable *table = cursor.insertTable(m_filteredAnimals.size() + 1, 6, tableFormat);

    // Header row
    QStringList headers = {"ID", "Nom", "Espèce", "Race", "Propriétaire", "Date naissance"};
    for (int col = 0; col < headers.size(); ++col) {
        QTextCharFormat cellFormat;
        cellFormat.setFontWeight(QFont::Bold);
        table->cellAt(0, col).firstCursorPosition().setCharFormat(cellFormat);
        table->cellAt(0, col).firstCursorPosition().insertText(headers[col]);
    }

    // Data rows
    for (int row = 0; row < m_filteredAnimals.size(); ++row) {
        const Animal &animal = m_filteredAnimals[row];
        table->cellAt(row + 1, 0).firstCursorPosition().insertText(QString::number(animal.id));
        table->cellAt(row + 1, 1).firstCursorPosition().insertText(animal.name);
        table->cellAt(row + 1, 2).firstCursorPosition().insertText(animal.species);
        table->cellAt(row + 1, 3).firstCursorPosition().insertText(animal.breed);
        table->cellAt(row + 1, 4).firstCursorPosition().insertText(animal.ownerId);
        table->cellAt(row + 1, 5).firstCursorPosition().insertText(animal.birthDate.isValid() ? animal.birthDate.toString("dd/MM/yyyy") : "-");
    }

    doc.print(&printer);
    QMessageBox::information(this, "Succès", "Rapport PDF exporté avec succès.");
}

void MedicalRecordsWidget::onExportExcel()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en Excel", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // Header
    out << "ID,Nom,Espece,Race,Proprietaire,Date naissance\n";

    // Data
    for (const auto &animal : m_filteredAnimals) {
        out << animal.id << ","
            << animal.name << ","
            << animal.species << ","
            << animal.breed << ","
            << animal.ownerId << ","
            << (animal.birthDate.isValid() ? animal.birthDate.toString("dd/MM/yyyy") : "-")
            << "\n";
    }

    file.close();
    QMessageBox::information(this, "Succès", "Fichier CSV exporté avec succès.");
}

void MedicalRecordsWidget::updateNavigationButtons()
{
    // Mettre à jour l'état des boutons de navigation
    // Cette méthode peut être étendue pour gérer les boutons dans la topbar
}

void MedicalRecordsWidget::applyStyles()
{
    setStyleSheet(R"(
        QWidget { font-family: "Segoe UI", sans-serif; font-size: 13px; color: #293F45; }
        QLabel { background: transparent; }
        QPushButton { padding: 0; }
        #eyebrow { color: #367985; font-size: 10px; font-weight: 700; }
        #welcome { color: #263D43; font-size: 36px; font-weight: 700; }
        #subtitle { color: #73868A; font-size: 13px; }
        #fieldLabel { color: #334D53; font-size: 12px; font-weight: 600; }
        QLineEdit {
            background: #F4F7F6; border: 1px solid #DDE6E3;
            border-radius: 11px; padding: 13px 12px;
            font-size: 13px; color: #293F45;
            selection-background-color: #397E89; selection-color: white;
        }
        QLineEdit:hover { border-color: #AAC7C6; }
        QLineEdit:focus { border: 1px solid #397E89; background: #FFFFFF; }
        QComboBox {
            background: #F4F7F6; border: 1px solid #DDE6E3;
            border-radius: 11px; padding: 8px 12px;
            font-size: 13px; color: #293F45;
        }
        QComboBox:hover { border-color: #AAC7C6; }
        QComboBox:focus { border-color: #397E89; background: #FFFFFF; }
        QComboBox::drop-down { border: none; width: 30px; }
        QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid #397E89; }
        QTableWidget {
            background: #FFFFFF; border: 1px solid #E2EBE9;
            border-radius: 12px; gridline-color: #E2EBE9;
        }
        QTableWidget::item { padding: 8px; }
        QTableWidget::item:selected { background: #EAF3F1; color: #293F45; }
        QHeaderView::section {
            background: #F4F7F6; color: #334D53;
            padding: 12px; border: none; border-bottom: 1px solid #E2EBE9;
            font-weight: 600; font-size: 12px;
        }
        QScrollBar:vertical { width: 7px; background: transparent; margin: 5px 0; }
        QScrollBar::handle:vertical { background: #C8D9D4; border-radius: 3px; min-height: 30px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
    )");
}
