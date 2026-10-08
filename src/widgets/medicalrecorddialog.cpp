#include "medicalrecorddialog.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QShowEvent>

MedicalRecordDialog::MedicalRecordDialog(int animalId, QWidget *parent)
    : QDialog(parent), m_animalId(animalId)
{
    setup(MedicalRecord{}, false, false);
}

MedicalRecordDialog::MedicalRecordDialog(const MedicalRecord &record, bool readOnly, QWidget *parent)
    : QDialog(parent), m_animalId(record.animalId), m_record(record)
{
    setup(record, record.id > 0, readOnly);
}

void MedicalRecordDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    Motion::reveal(this, QColor("#F3F7F8"), 180);
}

void MedicalRecordDialog::setup(const MedicalRecord &r, bool editing, bool readOnly)
{
    m_record = r;
    setObjectName("medicalRecordDialog");
    setMinimumWidth(600);
    setModal(true);

    const QString title = readOnly ? "Détails du dossier médical" : editing ? "Modifier le dossier" : "Nouveau dossier médical";
    setWindowTitle("ANIWORLD — " + title);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(27, 24, 27, 24);
    root->setSpacing(17);

    auto *head = new QHBoxLayout;
    auto *text = new QVBoxLayout;
    text->setSpacing(5);

    text->addWidget(Design::label(editing ? QString("DOSSIER  /  MED-%1").arg(r.id, 3, 10, QChar('0')) : "NOUVEAU DOSSIER", "eyebrow"));
    text->addWidget(Design::label(title, "title"));
    text->addWidget(Design::label(readOnly ? "Toutes les informations de ce dossier médical." : "Enregistrez un nouveau dossier médical.", "muted"));

    head->addLayout(text);
    head->addStretch();

    auto *logo = Design::label("");
    logo->setPixmap(Design::brand("logo", QSize(57, 57)));
    head->addWidget(logo);

    root->addLayout(head);

    auto *card = Design::card();
    auto *form = new QGridLayout(card);
    form->setContentsMargins(20, 19, 20, 20);
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(8);
    form->setColumnStretch(0, 1);
    form->setColumnStretch(1, 1);

    m_type = new QLineEdit(r.type);
    m_type->setObjectName("recordType");
    m_type->setMaxLength(100);
    m_type->setPlaceholderText("Ex. Consultation, Vaccination, Chirurgie...");
    form->addWidget(Design::label("Type de dossier *"), 0, 0, 1, 2);
    form->addWidget(m_type, 1, 0, 1, 2);

    m_veterinarian = new QLineEdit(r.veterinarian);
    m_veterinarian->setObjectName("recordVeterinarian");
    m_veterinarian->setMaxLength(100);
    m_veterinarian->setPlaceholderText("Nom du vétérinaire");
    form->addWidget(Design::label("Vétérinaire"), 2, 0);

    m_date = new QDateTimeEdit(r.date.isValid() ? r.date : QDateTime::currentDateTime());
    m_date->setObjectName("recordDate");
    m_date->setCalendarPopup(true);
    m_date->setDisplayFormat("dd/MM/yyyy HH:mm");
    form->addWidget(Design::label("Date et heure *"), 2, 1);
    form->addWidget(m_veterinarian, 3, 0);
    form->addWidget(m_date, 3, 1);

    m_status = new QComboBox;
    m_status->setObjectName("recordStatus");
    m_status->addItems({"En cours", "Terminé", "Annulé"});
    m_status->setCurrentText(r.status);
    form->addWidget(Design::label("Statut"), 4, 0);

    m_cost = new QDoubleSpinBox;
    m_cost->setObjectName("recordCost");
    m_cost->setRange(0, 99999.99);
    m_cost->setDecimals(2);
    m_cost->setSuffix(" DT");
    m_cost->setValue(r.cost);
    form->addWidget(Design::label("Coût"), 4, 1);
    form->addWidget(m_status, 5, 0);
    form->addWidget(m_cost, 5, 1);

    m_description = new QTextEdit;
    m_description->setObjectName("recordDescription");
    m_description->setAcceptRichText(false);
    m_description->setPlainText(r.description);
    m_description->setPlaceholderText("Description de la consultation ou de l'intervention...");
    m_description->setFixedHeight(70);
    form->addWidget(Design::label("Description"), 6, 0, 1, 2);
    form->addWidget(m_description, 7, 0, 1, 2);

    m_diagnosis = new QTextEdit;
    m_diagnosis->setObjectName("recordDiagnosis");
    m_diagnosis->setAcceptRichText(false);
    m_diagnosis->setPlainText(r.diagnosis);
    m_diagnosis->setPlaceholderText("Diagnostic posé...");
    m_diagnosis->setFixedHeight(70);
    form->addWidget(Design::label("Diagnostic"), 8, 0, 1, 2);
    form->addWidget(m_diagnosis, 9, 0, 1, 2);

    m_treatment = new QTextEdit;
    m_treatment->setObjectName("recordTreatment");
    m_treatment->setAcceptRichText(false);
    m_treatment->setPlainText(r.treatment);
    m_treatment->setPlaceholderText("Traitement prescrit...");
    m_treatment->setFixedHeight(70);
    form->addWidget(Design::label("Traitement"), 10, 0, 1, 2);
    form->addWidget(m_treatment, 11, 0, 1, 2);

    m_notes = new QTextEdit;
    m_notes->setObjectName("recordNotes");
    m_notes->setAcceptRichText(false);
    m_notes->setPlainText(r.notes);
    m_notes->setPlaceholderText("Notes supplémentaires...");
    m_notes->setFixedHeight(60);
    form->addWidget(Design::label("Notes"), 12, 0, 1, 2);
    form->addWidget(m_notes, 13, 0, 1, 2);

    root->addWidget(card);

    m_error = Design::label("");
    m_error->setObjectName("validationError");
    m_error->setStyleSheet("color:#B25C4A;font-size:12px;");
    m_error->setWordWrap(true);
    m_error->hide();
    root->addWidget(m_error);

    auto *buttons = new QHBoxLayout;
    buttons->addStretch();

    auto *cancel = Design::button(readOnly ? "Fermer" : "Annuler");
    cancel->setObjectName("cancelRecord");
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    buttons->addWidget(cancel);

    if (!readOnly) {
        auto *save = Design::button(editing ? "Enregistrer" : "Créer", "check", "primary");
        save->setObjectName("saveRecord");
        save->setDefault(true);
        connect(save, &QPushButton::clicked, this, &MedicalRecordDialog::validateAndAccept);
        buttons->addWidget(save);
    } else {
        m_type->setReadOnly(true);
        m_veterinarian->setReadOnly(true);
        m_date->setReadOnly(true);
        m_status->setEnabled(false);
        m_cost->setReadOnly(true);
        m_description->setReadOnly(true);
        m_diagnosis->setReadOnly(true);
        m_treatment->setReadOnly(true);
        m_notes->setReadOnly(true);
    }

    root->addLayout(buttons);
    m_type->setFocus();
}

void MedicalRecordDialog::validateAndAccept()
{
    const bool valid = !m_type->text().trimmed().isEmpty() && m_date->dateTime().isValid();

    m_type->setProperty("invalid", !valid);
    m_type->style()->unpolish(m_type);
    m_type->style()->polish(m_type);

    if (!valid) {
        m_error->setText("Le type et la date sont obligatoires.");
        m_error->show();
        m_type->setFocus();
        Motion::reveal(m_error, QColor("#F3F7F8"));
        return;
    }

    accept();
}

MedicalRecord MedicalRecordDialog::getRecord() const
{
    MedicalRecord r;
    r.id = m_record.id;
    r.animalId = m_animalId;
    r.type = m_type->text().trimmed();
    r.veterinarian = m_veterinarian->text().trimmed();
    r.date = m_date->dateTime();
    r.status = m_status->currentText();
    r.description = m_description->toPlainText().trimmed();
    r.diagnosis = m_diagnosis->toPlainText().trimmed();
    r.treatment = m_treatment->toPlainText().trimmed();
    r.cost = m_cost->value();
    r.notes = m_notes->toPlainText().trimmed();
    r.createdAt = m_record.createdAt;
    r.updatedAt = QDateTime::currentDateTime();
    return r;
}
