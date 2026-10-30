#include "../database/oracleconnection.h"
#include "medicalrecorddetailsdialog.h"
#include "../ui/design.h"
#include "medicalrecord.h"

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
    setMinimumSize(700, 520);
    resize(800, 640);

    setObjectName("medicalrecorddetailsdialog");
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

    consultationGrid->addWidget(new QLabel("Date:"), 0, 2);
    m_labelConsultationDate = new QLabel();
    m_labelConsultationDate->setObjectName("detailValue");
    consultationGrid->addWidget(m_labelConsultationDate, 0, 3);

    consultationGrid->addWidget(new QLabel("Motif:"), 1, 0);
    m_labelReason = new QLabel();
    m_labelReason->setObjectName("detailValue");
    m_labelReason->setWordWrap(true);
    consultationGrid->addWidget(m_labelReason, 1, 1, 1, 3);

    consultationGrid->addWidget(new QLabel("Diagnostic:"), 2, 0);
    m_labelDiagnosis = new QLabel();
    m_labelDiagnosis->setObjectName("detailValue");
    m_labelDiagnosis->setWordWrap(true);
    consultationGrid->addWidget(m_labelDiagnosis, 2, 1, 1, 3);

    consultationGrid->addWidget(new QLabel("Traitement:"), 3, 0);
    m_labelTreatment = new QLabel();
    m_labelTreatment->setObjectName("detailValue");
    m_labelTreatment->setWordWrap(true);
    consultationGrid->addWidget(m_labelTreatment, 3, 1, 1, 3);

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
    if(OracleConnection::isOracle()){healthCard->hide();m_labelTreatment->hide();consultationGrid->itemAtPosition(3,0)->widget()->hide();}

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
    m_historyTable->setHorizontalHeaderLabels({"Date", "Dossier", "Consultation"});
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setShowGrid(false);
    m_historyTable->setAlternatingRowColors(false);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);
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

    m_btnClose = Design::button("Fermer",{},"primary");
    m_btnClose->setObjectName("secondaryButton");
    connect(m_btnClose, &QPushButton::clicked, this, &MedicalRecordDetailsDialog::onCloseClicked);
    buttonLayout->addWidget(m_btnClose);

    mainLayout->addLayout(buttonLayout);
    for(auto *label:findChildren<QLabel *>("detailValue")) {
        label->setTextFormat(Qt::PlainText); label->setWordWrap(true);
        label->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }
}

void MedicalRecordDetailsDialog::setMedicalRecord(const MedicalRecord& record)
{
    displayRecord(record);
    displayMedicalHistory(record);
}

void MedicalRecordDetailsDialog::displayRecord(const MedicalRecord& record)
{
    m_labelAnimalId->setText(record.animal.animalId);
    m_labelAnimalName->setText(record.animal.name);
    m_labelSpecies->setText(record.animal.species);
    m_labelBreed->setText(record.animal.breed);
    m_labelAge->setText(QString::number(record.animal.age) + " ans");
    m_labelSex->setText(record.animal.sex);
    m_labelOwner->setText(record.animal.owner);

    m_labelRecordId->setText(record.medical.recordId);
    m_labelConsultationDate->setText(record.medical.consultationDate.toString("dd/MM/yyyy"));
    m_labelReason->setText(record.medical.reason);
    m_labelDiagnosis->setText(record.medical.diagnosis);
    m_labelTreatment->setText(record.medical.treatment);

    m_labelVaccinations->setText(record.medical.vaccinations);
    m_labelAllergies->setText(record.medical.allergies);
    m_labelWeight->setText(QString::number(record.medical.weight) + " kg");
    m_labelVaccinationStatus->setText(record.medical.vaccinationStatus);
    m_labelObservations->setText(record.medical.observations);
}

void MedicalRecordDetailsDialog::displayMedicalHistory(const MedicalRecord& record)
{
    m_historyTable->setRowCount(0);

    QString error;
    const auto records=MedicalRecord::afficher(&error);
    if(!error.isEmpty()) { m_historyTable->setToolTip(error); return; }
    for(const auto &visit:records) {
        if(visit.animal.animalId!=record.animal.animalId) continue;
        const int row=m_historyTable->rowCount(); m_historyTable->insertRow(row);
        m_historyTable->setItem(row,0,new QTableWidgetItem(visit.medical.consultationDate.toString("dd/MM/yyyy")));
        m_historyTable->setItem(row,1,new QTableWidgetItem(visit.medical.recordId));
        m_historyTable->setItem(row,2,new QTableWidgetItem(visit.medical.reason+" — "+visit.medical.diagnosis));
    }
    m_historyTable->resizeRowsToContents();

}

void MedicalRecordDetailsDialog::onCloseClicked()
{
    accept();
}
