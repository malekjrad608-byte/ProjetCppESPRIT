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

    medicalGrid->addWidget(new QLabel("ID employé:"), 0, 2);
    m_editEmployeeId = new QLineEdit();
    m_editEmployeeId->setPlaceholderText("EMP-XXX");
    medicalGrid->addWidget(m_editEmployeeId, 0, 3);

    medicalGrid->addWidget(new QLabel("Date consultation:"), 1, 0);
    m_dateConsultation = new QDateEdit();
    m_dateConsultation->setCalendarPopup(true);
    m_dateConsultation->setDate(QDate::currentDate());
    medicalGrid->addWidget(m_dateConsultation, 1, 1);

    medicalGrid->addWidget(new QLabel("Motif:"), 2, 0);
    m_editReason = new QTextEdit();
    m_editReason->setMaximumHeight(60);
    medicalGrid->addWidget(m_editReason, 2, 1, 1, 3);

    medicalGrid->addWidget(new QLabel("Diagnostic:"), 3, 0);
    m_editDiagnosis = new QTextEdit();
    m_editDiagnosis->setMaximumHeight(60);
    medicalGrid->addWidget(m_editDiagnosis, 3, 1, 1, 3);

    medicalGrid->addWidget(new QLabel("Traitement:"), 4, 0);
    m_editTreatment = new QTextEdit();
    m_editTreatment->setMaximumHeight(60);
    medicalGrid->addWidget(m_editTreatment, 4, 1, 1, 3);

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
    m_editAnimalId->setText(record.getIdAnimal());
    m_editAnimalName->setText(record.getNomAnimal());
    m_comboSpecies->setCurrentText(record.getEspece());
    m_editBreed->setText(record.getRace());
    m_spinAge->setValue(record.getAge());
    m_comboSex->setCurrentText(record.getSexe());
    m_editOwner->setText(record.getProprietaire());

    m_editRecordId->setText(record.getIdDossier());
    // La clé primaire ne doit pas changer pendant un UPDATE :
    // elle est utilisée dans la clause WHERE ID_DOSSIER.
    m_editRecordId->setReadOnly(true);
    m_editEmployeeId->setText(record.getIdEmployes());
    m_dateConsultation->setDate(record.getDateConsultation());
    m_editReason->setText(record.getMotif());
    m_editDiagnosis->setText(record.getDiagnostic());
    m_editTreatment->setText(record.getTraitement());

    m_editVaccinations->setText(record.getVaccinations());
    m_editAllergies->setText(record.getAllergies());
    m_spinWeight->setValue(record.getPoids());
    m_comboVaccinationStatus->setCurrentText(record.getStatutVaccination());
    m_editObservations->setText(record.getObservations());
}

MedicalRecord MedicalRecordDialog::getMedicalRecord() const
{
    //Récupération des informations saisies dans le formulaire
    QString idDossier = m_editRecordId->text().trimmed();
    QDate dateConsultation = m_dateConsultation->date();
    QString motif = m_editReason->toPlainText().trimmed();
    QString diagnostic = m_editDiagnosis->toPlainText().trimmed();
    QString idAnimal = m_editAnimalId->text().trimmed();
    QString nomAnimal = m_editAnimalName->text().trimmed();
    QString espece = m_comboSpecies->currentText();
    QString race = m_editBreed->text().trimmed();
    int age = m_spinAge->value();
    double poids = m_spinWeight->value();
    QString proprietaire = m_editOwner->text().trimmed();
    QString idEmployes = m_editEmployeeId->text().trimmed();

    // instancier un objet de la classe MedicalRecord avec les informations saisies
    MedicalRecord M(idDossier, dateConsultation, motif, diagnostic, idAnimal, nomAnimal,
                    espece, race, age, poids, proprietaire, idEmployes);

    // informations affichées seulement (pas de colonne dans la table MEDICAL)
    M.setSexe(m_comboSex->currentText());
    M.setTraitement(m_editTreatment->toPlainText());
    M.setVaccinations(m_editVaccinations->toPlainText());
    M.setAllergies(m_editAllergies->toPlainText());
    M.setObservations(m_editObservations->toPlainText());
    M.setStatutVaccination(m_comboVaccinationStatus->currentText());

    return M;
}

bool MedicalRecordDialog::validateForm()
{
    auto required = [this](const QString &value, const QString &label) {
        if (!value.trimmed().isEmpty())
            return true;
        QMessageBox::warning(this, "Saisie incomplète", label + " est obligatoire.");
        return false;
    };
    auto maxLength = [this](const QString &value, int max, const QString &label) {
        if (value.trimmed().size() <= max)
            return true;
        QMessageBox::warning(this, "Saisie invalide",
                             QString("%1 ne doit pas dépasser %2 caractères.").arg(label).arg(max));
        return false;
    };

    if (!required(m_editAnimalId->text(), "L'ID animal") ||
        !required(m_editAnimalName->text(), "Le nom de l'animal") ||
        !required(m_editOwner->text(), "Le propriétaire") ||
        !required(m_editEmployeeId->text(), "L'ID employé") ||
        !required(m_editReason->toPlainText(), "Le motif"))
        return false;

    // À la création, l'ID dossier vide est généré automatiquement.
    if (!maxLength(m_editRecordId->text(), 20, "L'ID dossier") ||
        !maxLength(m_editAnimalId->text(), 20, "L'ID animal") ||
        !maxLength(m_editAnimalName->text(), 100, "Le nom de l'animal") ||
        !maxLength(m_editOwner->text(), 100, "Le propriétaire") ||
        !maxLength(m_editEmployeeId->text(), 20, "L'ID employé") ||
        !maxLength(m_comboSpecies->currentText(), 50, "L'espèce") ||
        !maxLength(m_editBreed->text(), 100, "La race") ||
        !maxLength(m_editReason->toPlainText(), 500, "Le motif") ||
        !maxLength(m_editDiagnosis->toPlainText(), 500, "Le diagnostic"))
        return false;

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
