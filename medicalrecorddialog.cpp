#include "medicalrecorddialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include <QDate>
#include <QScrollArea>

MedicalRecordDialog::MedicalRecordDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Dossier médical");
    setMinimumSize(700, 600);
    resize(800, 700);

    setupUI();
}

void MedicalRecordDialog::setupUI()
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

    QLabel *animalTitle = new QLabel("Informations de l'animal");
    animalTitle->setObjectName("cardTitle");
    animalCardLayout->addWidget(animalTitle);

    QGridLayout *animalGrid = new QGridLayout();
    animalGrid->setSpacing(12);

    animalGrid->addWidget(new QLabel("ID animal:"), 0, 0);
    m_editAnimalId = new QLineEdit();
    m_editAnimalId->setPlaceholderText("ANI-XXX");
    animalGrid->addWidget(m_editAnimalId, 0, 1);

    animalGrid->addWidget(new QLabel("Nom:"), 0, 2);
    m_editAnimalName = new QLineEdit();
    animalGrid->addWidget(m_editAnimalName, 0, 3);

    animalGrid->addWidget(new QLabel("Espèce:"), 1, 0);
    m_comboSpecies = new QComboBox();
    m_comboSpecies->addItem("Chien");
    m_comboSpecies->addItem("Chat");
    m_comboSpecies->addItem("Lapin");
    m_comboSpecies->addItem("Oiseau");
    animalGrid->addWidget(m_comboSpecies, 1, 1);

    animalGrid->addWidget(new QLabel("Race:"), 1, 2);
    m_editBreed = new QLineEdit();
    animalGrid->addWidget(m_editBreed, 1, 3);

    animalGrid->addWidget(new QLabel("Âge:"), 2, 0);
    m_spinAge = new QSpinBox();
    m_spinAge->setRange(0, 30);
    m_spinAge->setSuffix(" ans");
    animalGrid->addWidget(m_spinAge, 2, 1);

    animalGrid->addWidget(new QLabel("Sexe:"), 2, 2);
    m_comboSex = new QComboBox();
    m_comboSex->addItem("Mâle");
    m_comboSex->addItem("Femelle");
    animalGrid->addWidget(m_comboSex, 2, 3);

    animalGrid->addWidget(new QLabel("Propriétaire:"), 3, 0);
    m_editOwner = new QLineEdit();
    animalGrid->addWidget(m_editOwner, 3, 1, 1, 3);

    animalCardLayout->addLayout(animalGrid);
    scrollLayout->addWidget(animalCard);

    // Card 2 - Medical Record
    QFrame *medicalCard = new QFrame();
    medicalCard->setObjectName("dialogCard");
    QVBoxLayout *medicalCardLayout = new QVBoxLayout(medicalCard);

    QLabel *medicalTitle = new QLabel("Dossier médical");
    medicalTitle->setObjectName("cardTitle");
    medicalCardLayout->addWidget(medicalTitle);

    QGridLayout *medicalGrid = new QGridLayout();
    medicalGrid->setSpacing(12);

    medicalGrid->addWidget(new QLabel("ID dossier:"), 0, 0);
    m_editRecordId = new QLineEdit();
    m_editRecordId->setPlaceholderText("MED-XXX");
    medicalGrid->addWidget(m_editRecordId, 0, 1);

    medicalGrid->addWidget(new QLabel("Date consultation:"), 0, 2);
    m_dateConsultation = new QDateEdit();
    m_dateConsultation->setCalendarPopup(true);
    m_dateConsultation->setDate(QDate::currentDate());
    medicalGrid->addWidget(m_dateConsultation, 0, 3);

    medicalGrid->addWidget(new QLabel("Motif:"), 1, 0);
    m_editReason = new QTextEdit();
    m_editReason->setMaximumHeight(60);
    medicalGrid->addWidget(m_editReason, 1, 1, 1, 3);

    medicalGrid->addWidget(new QLabel("Diagnostic:"), 2, 0);
    m_editDiagnosis = new QTextEdit();
    m_editDiagnosis->setMaximumHeight(60);
    medicalGrid->addWidget(m_editDiagnosis, 2, 1, 1, 3);

    medicalGrid->addWidget(new QLabel("Traitement:"), 3, 0);
    m_editTreatment = new QTextEdit();
    m_editTreatment->setMaximumHeight(60);
    medicalGrid->addWidget(m_editTreatment, 3, 1, 1, 3);

    medicalCardLayout->addLayout(medicalGrid);
    scrollLayout->addWidget(medicalCard);

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
    m_editVaccinations = new QTextEdit();
    m_editVaccinations->setMaximumHeight(60);
    healthGrid->addWidget(m_editVaccinations, 0, 1, 1, 3);

    healthGrid->addWidget(new QLabel("Allergies:"), 1, 0);
    m_editAllergies = new QTextEdit();
    m_editAllergies->setMaximumHeight(60);
    healthGrid->addWidget(m_editAllergies, 1, 1, 1, 3);

    healthGrid->addWidget(new QLabel("Poids (kg):"), 2, 0);
    m_spinWeight = new QDoubleSpinBox();
    m_spinWeight->setRange(0.0, 100.0);
    m_spinWeight->setSingleStep(0.1);
    m_spinWeight->setDecimals(2);
    healthGrid->addWidget(m_spinWeight, 2, 1);

    healthGrid->addWidget(new QLabel("Statut vaccination:"), 2, 2);
    m_comboVaccinationStatus = new QComboBox();
    m_comboVaccinationStatus->addItem("À jour");
    m_comboVaccinationStatus->addItem("À renouveler");
    healthGrid->addWidget(m_comboVaccinationStatus, 2, 3);

    healthGrid->addWidget(new QLabel("Observations:"), 3, 0);
    m_editObservations = new QTextEdit();
    m_editObservations->setMaximumHeight(80);
    healthGrid->addWidget(m_editObservations, 3, 1, 1, 3);

    healthCardLayout->addLayout(healthGrid);
    scrollLayout->addWidget(healthCard);

    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_btnCancel = new QPushButton("Annuler");
    m_btnCancel->setObjectName("secondaryButton");
    connect(m_btnCancel, &QPushButton::clicked, this, &MedicalRecordDialog::onCancelClicked);
    buttonLayout->addWidget(m_btnCancel);

    m_btnSave = new QPushButton("Enregistrer");
    m_btnSave->setObjectName("primaryButton");
    connect(m_btnSave, &QPushButton::clicked, this, &MedicalRecordDialog::onSaveClicked);
    buttonLayout->addWidget(m_btnSave);

    mainLayout->addLayout(buttonLayout);
}

void MedicalRecordDialog::setMedicalRecord(const MedicalRecord& record)
{
    m_editAnimalId->setText(record.animal.animalId);
    m_editAnimalName->setText(record.animal.name);
    m_comboSpecies->setCurrentText(record.animal.species);
    m_editBreed->setText(record.animal.breed);
    m_spinAge->setValue(record.animal.age);
    m_comboSex->setCurrentText(record.animal.sex);
    m_editOwner->setText(record.animal.owner);

    m_editRecordId->setText(record.medical.recordId);
    m_dateConsultation->setDate(record.medical.consultationDate);
    m_editReason->setText(record.medical.reason);
    m_editDiagnosis->setText(record.medical.diagnosis);
    m_editTreatment->setText(record.medical.treatment);

    m_editVaccinations->setText(record.medical.vaccinations);
    m_editAllergies->setText(record.medical.allergies);
    m_spinWeight->setValue(record.medical.weight);
    m_comboVaccinationStatus->setCurrentText(record.medical.vaccinationStatus);
    m_editObservations->setText(record.medical.observations);
}

MedicalRecord MedicalRecordDialog::getMedicalRecord() const
{
    AnimalInfo animal;
    animal.animalId = m_editAnimalId->text();
    animal.name = m_editAnimalName->text();
    animal.species = m_comboSpecies->currentText();
    animal.breed = m_editBreed->text();
    animal.age = m_spinAge->value();
    animal.sex = m_comboSex->currentText();
    animal.owner = m_editOwner->text();

    MedicalInfo medical;
    medical.recordId = m_editRecordId->text();
    medical.consultationDate = m_dateConsultation->date();
    medical.reason = m_editReason->toPlainText();
    medical.diagnosis = m_editDiagnosis->toPlainText();
    medical.treatment = m_editTreatment->toPlainText();
    medical.vaccinations = m_editVaccinations->toPlainText();
    medical.allergies = m_editAllergies->toPlainText();
    medical.weight = m_spinWeight->value();
    medical.vaccinationStatus = m_comboVaccinationStatus->currentText();
    medical.observations = m_editObservations->toPlainText();

    return MedicalRecord(animal, medical);
}

bool MedicalRecordDialog::validateForm()
{
    if (m_editAnimalId->text().isEmpty()) {
        QMessageBox::warning(this, "Validation", "L'ID animal est obligatoire.");
        return false;
    }

    if (m_editAnimalName->text().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Le nom de l'animal est obligatoire.");
        return false;
    }

    if (m_editOwner->text().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Le propriétaire est obligatoire.");
        return false;
    }

    if (m_editRecordId->text().isEmpty()) {
        QMessageBox::warning(this, "Validation", "L'ID dossier est obligatoire.");
        return false;
    }

    return true;
}

void MedicalRecordDialog::onSaveClicked()
{
    if (validateForm()) {
        accept();
    }
}

void MedicalRecordDialog::onCancelClicked()
{
    reject();
}
