#include "medicalrecorddetailsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QScrollArea>
#include <QDate>

MedicalRecordDetailsDialog::MedicalRecordDetailsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Détails du dossier médical");
    setMinimumSize(900, 700);
    resize(1000, 800);

    setupUI();
}

void MedicalRecordDetailsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollWidget = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setSpacing(20);

    // Card 1 - Animal Information
    QFrame *animalCard = new QFrame();
    animalCard->setObjectName("dialogCard");
    QVBoxLayout *animalCardLayout = new QVBoxLayout(animalCard);

    QLabel *animalTitle = new QLabel("Animal");
    animalTitle->setObjectName("cardTitle");
    animalCardLayout->addWidget(animalTitle);

    QGridLayout *animalGrid = new QGridLayout();
    animalGrid->setSpacing(12);

    animalGrid->addWidget(new QLabel("ID:"), 0, 0);
    m_labelAnimalId = new QLabel();
    m_labelAnimalId->setObjectName("detailValue");
    animalGrid->addWidget(m_labelAnimalId, 0, 1);

    animalGrid->addWidget(new QLabel("Nom:"), 0, 2);
    m_labelAnimalName = new QLabel();
    m_labelAnimalName->setObjectName("detailValue");
    animalGrid->addWidget(m_labelAnimalName, 0, 3);

    animalGrid->addWidget(new QLabel("Espèce:"), 1, 0);
    m_labelSpecies = new QLabel();
    m_labelSpecies->setObjectName("detailValue");
    animalGrid->addWidget(m_labelSpecies, 1, 1);

    animalGrid->addWidget(new QLabel("Race:"), 1, 2);
    m_labelBreed = new QLabel();
    m_labelBreed->setObjectName("detailValue");
    animalGrid->addWidget(m_labelBreed, 1, 3);

    animalGrid->addWidget(new QLabel("Âge:"), 2, 0);
    m_labelAge = new QLabel();
    m_labelAge->setObjectName("detailValue");
    animalGrid->addWidget(m_labelAge, 2, 1);

    animalGrid->addWidget(new QLabel("Sexe:"), 2, 2);
    m_labelSex = new QLabel();
    m_labelSex->setObjectName("detailValue");
    animalGrid->addWidget(m_labelSex, 2, 3);

    animalGrid->addWidget(new QLabel("Propriétaire:"), 3, 0);
    m_labelOwner = new QLabel();
    m_labelOwner->setObjectName("detailValue");
    animalGrid->addWidget(m_labelOwner, 3, 1, 1, 3);

    animalCardLayout->addLayout(animalGrid);
    scrollLayout->addWidget(animalCard);

    // Card 2 - Consultation
    QFrame *consultationCard = new QFrame();
    consultationCard->setObjectName("dialogCard");
    QVBoxLayout *consultationCardLayout = new QVBoxLayout(consultationCard);

    QLabel *consultationTitle = new QLabel("Consultation");
    consultationTitle->setObjectName("cardTitle");
    consultationCardLayout->addWidget(consultationTitle);

    QGridLayout *consultationGrid = new QGridLayout();
    consultationGrid->setSpacing(12);

    consultationGrid->addWidget(new QLabel("ID dossier:"), 0, 0);
    m_labelRecordId = new QLabel();
    m_labelRecordId->setObjectName("detailValue");
    consultationGrid->addWidget(m_labelRecordId, 0, 1);

    consultationGrid->addWidget(new QLabel("ID employé:"), 0, 2);
    m_labelEmployeeId = new QLabel();
    m_labelEmployeeId->setObjectName("detailValue");
    consultationGrid->addWidget(m_labelEmployeeId, 0, 3);

    consultationGrid->addWidget(new QLabel("Date:"), 1, 0);
    m_labelConsultationDate = new QLabel();
    m_labelConsultationDate->setObjectName("detailValue");
    consultationGrid->addWidget(m_labelConsultationDate, 1, 1);

    consultationGrid->addWidget(new QLabel("Motif:"), 2, 0);
    m_labelReason = new QLabel();
    m_labelReason->setObjectName("detailValue");
    m_labelReason->setWordWrap(true);
    consultationGrid->addWidget(m_labelReason, 2, 1, 1, 3);

    consultationGrid->addWidget(new QLabel("Diagnostic:"), 3, 0);
    m_labelDiagnosis = new QLabel();
    m_labelDiagnosis->setObjectName("detailValue");
    m_labelDiagnosis->setWordWrap(true);
    consultationGrid->addWidget(m_labelDiagnosis, 3, 1, 1, 3);

    consultationGrid->addWidget(new QLabel("Traitement:"), 4, 0);
    m_labelTreatment = new QLabel();
    m_labelTreatment->setObjectName("detailValue");
    m_labelTreatment->setWordWrap(true);
    consultationGrid->addWidget(m_labelTreatment, 4, 1, 1, 3);

    consultationCardLayout->addLayout(consultationGrid);
    scrollLayout->addWidget(consultationCard);

    // Card 3 - Health
    QFrame *healthCard = new QFrame();
    healthCard->setObjectName("dialogCard");
    QVBoxLayout *healthCardLayout = new QVBoxLayout(healthCard);

    QLabel *healthTitle = new QLabel("Santé");
    healthTitle->setObjectName("cardTitle");
    healthCardLayout->addWidget(healthTitle);

    QGridLayout *healthGrid = new QGridLayout();
    healthGrid->setSpacing(12);

    healthGrid->addWidget(new QLabel("Vaccinations:"), 0, 0);
    m_labelVaccinations = new QLabel();
    m_labelVaccinations->setObjectName("detailValue");
    m_labelVaccinations->setWordWrap(true);
    healthGrid->addWidget(m_labelVaccinations, 0, 1, 1, 3);

    healthGrid->addWidget(new QLabel("Allergies:"), 1, 0);
    m_labelAllergies = new QLabel();
    m_labelAllergies->setObjectName("detailValue");
    m_labelAllergies->setWordWrap(true);
    healthGrid->addWidget(m_labelAllergies, 1, 1, 1, 3);

    healthGrid->addWidget(new QLabel("Poids:"), 2, 0);
    m_labelWeight = new QLabel();
    m_labelWeight->setObjectName("detailValue");
    healthGrid->addWidget(m_labelWeight, 2, 1);

    healthGrid->addWidget(new QLabel("Statut vaccination:"), 2, 2);
    m_labelVaccinationStatus = new QLabel();
    m_labelVaccinationStatus->setObjectName("detailValue");
    healthGrid->addWidget(m_labelVaccinationStatus, 2, 3);

    healthGrid->addWidget(new QLabel("Observations:"), 3, 0);
    m_labelObservations = new QLabel();
    m_labelObservations->setObjectName("detailValue");
    m_labelObservations->setWordWrap(true);
    healthGrid->addWidget(m_labelObservations, 3, 1, 1, 3);

    healthCardLayout->addLayout(healthGrid);
    scrollLayout->addWidget(healthCard);

    // Card 4 - Medical History
    QFrame *historyCard = new QFrame();
    historyCard->setObjectName("dialogCard");
    QVBoxLayout *historyCardLayout = new QVBoxLayout(historyCard);

    QLabel *historyTitle = new QLabel("Historique médical");
    historyTitle->setObjectName("cardTitle");
    historyCardLayout->addWidget(historyTitle);

    m_historyTable = new QTableWidget();
    m_historyTable->setObjectName("historyTable");
    m_historyTable->setColumnCount(3);
    m_historyTable->setHorizontalHeaderLabels({"Date", "Type", "Détails"});
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setShowGrid(false);
    m_historyTable->setAlternatingRowColors(true);
    m_historyTable->setMaximumHeight(200);

    m_historyTable->setColumnWidth(0, 120);
    m_historyTable->setColumnWidth(1, 100);
    m_historyTable->setColumnWidth(2, 400);

    historyCardLayout->addWidget(m_historyTable);
    scrollLayout->addWidget(historyCard);

    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);

    // Close button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_btnClose = new QPushButton("Fermer");
    m_btnClose->setObjectName("secondaryButton");
    connect(m_btnClose, &QPushButton::clicked, this, &MedicalRecordDetailsDialog::onCloseClicked);
    buttonLayout->addWidget(m_btnClose);

    mainLayout->addLayout(buttonLayout);
}

void MedicalRecordDetailsDialog::setMedicalRecord(const MedicalRecord& record)
{
    displayRecord(record);
    displayMedicalHistory(record);
}

void MedicalRecordDetailsDialog::displayRecord(const MedicalRecord& record)
{
    m_labelAnimalId->setText(record.getIdAnimal());
    m_labelAnimalName->setText(record.getNomAnimal());
    m_labelSpecies->setText(record.getEspece());
    m_labelBreed->setText(record.getRace());
    m_labelAge->setText(QString::number(record.getAge()) + " ans");
    m_labelSex->setText(record.getSexe());
    m_labelOwner->setText(record.getProprietaire());

    m_labelRecordId->setText(record.getIdDossier());
    m_labelEmployeeId->setText(record.getIdEmployes());
    m_labelConsultationDate->setText(record.getDateConsultation().toString("dd/MM/yyyy"));
    m_labelReason->setText(record.getMotif());
    m_labelDiagnosis->setText(record.getDiagnostic());
    m_labelTreatment->setText(record.getTraitement());

    m_labelVaccinations->setText(record.getVaccinations());
    m_labelAllergies->setText(record.getAllergies());
    m_labelWeight->setText(QString::number(record.getPoids()) + " kg");
    m_labelVaccinationStatus->setText(record.getStatutVaccination());
    m_labelObservations->setText(record.getObservations());
}

void MedicalRecordDetailsDialog::displayMedicalHistory(const MedicalRecord& record)
{
    m_historyTable->setRowCount(0);

    // Add current consultation
    int row = 0;
    m_historyTable->insertRow(row);
    m_historyTable->setItem(row, 0, new QTableWidgetItem(record.getDateConsultation().toString("dd/MM/yyyy")));
    m_historyTable->setItem(row, 1, new QTableWidgetItem("Consultation"));
    m_historyTable->setItem(row, 2, new QTableWidgetItem(record.getDiagnostic()));

    // Add fictitious previous visits (static data for demonstration)
    QDate previousDate1 = record.getDateConsultation().addMonths(-6);
    row++;
    m_historyTable->insertRow(row);
    m_historyTable->setItem(row, 0, new QTableWidgetItem(previousDate1.toString("dd/MM/yyyy")));
    m_historyTable->setItem(row, 1, new QTableWidgetItem("Vaccination"));
    m_historyTable->setItem(row, 2, new QTableWidgetItem("Rappel vaccination annuelle"));

    QDate previousDate2 = record.getDateConsultation().addMonths(-12);
    row++;
    m_historyTable->insertRow(row);
    m_historyTable->setItem(row, 0, new QTableWidgetItem(previousDate2.toString("dd/MM/yyyy")));
    m_historyTable->setItem(row, 1, new QTableWidgetItem("Consultation"));
    m_historyTable->setItem(row, 2, new QTableWidgetItem("Examen de routine -良好"));

    QDate previousDate3 = record.getDateConsultation().addMonths(-18);
    row++;
    m_historyTable->insertRow(row);
    m_historyTable->setItem(row, 0, new QTableWidgetItem(previousDate3.toString("dd/MM/yyyy")));
    m_historyTable->setItem(row, 1, new QTableWidgetItem("Traitement"));
    m_historyTable->setItem(row, 2, new QTableWidgetItem("Suivi post-opératoire"));
}

void MedicalRecordDetailsDialog::onCloseClicked()
{
    accept();
}
