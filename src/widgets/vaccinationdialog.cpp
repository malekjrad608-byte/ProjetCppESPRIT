#include "vaccinationdialog.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QShowEvent>

VaccinationDialog::VaccinationDialog(int animalId, QWidget *parent)
    : QDialog(parent), m_animalId(animalId)
{
    setup(Vaccination{}, false, false);
}

VaccinationDialog::VaccinationDialog(const Vaccination &vaccination, bool readOnly, QWidget *parent)
    : QDialog(parent), m_animalId(vaccination.animalId), m_vaccination(vaccination)
{
    setup(vaccination, vaccination.id > 0, readOnly);
}

void VaccinationDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    Motion::reveal(this, QColor("#F3F7F8"), 180);
}

void VaccinationDialog::setup(const Vaccination &v, bool editing, bool readOnly)
{
    m_vaccination = v;
    setObjectName("vaccinationDialog");
    setMinimumWidth(550);
    setModal(true);

    const QString title = readOnly ? "Détails de la vaccination" : editing ? "Modifier la vaccination" : "Nouvelle vaccination";
    setWindowTitle("ANIWORLD — " + title);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(27, 24, 27, 24);
    root->setSpacing(17);

    auto *head = new QHBoxLayout;
    auto *text = new QVBoxLayout;
    text->setSpacing(5);

    text->addWidget(Design::label(editing ? QString("VACCINATION  /  VAC-%1").arg(v.id, 3, 10, QChar('0')) : "NOUVELLE VACCINATION", "eyebrow"));
    text->addWidget(Design::label(title, "title"));
    text->addWidget(Design::label(readOnly ? "Toutes les informations de cette vaccination." : "Enregistrez une nouvelle vaccination.", "muted"));

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

    m_vaccineName = new QLineEdit(v.vaccineName);
    m_vaccineName->setObjectName("vaccineName");
    m_vaccineName->setMaxLength(100);
    m_vaccineName->setPlaceholderText("Ex. Rage, Parvovirose...");
    form->addWidget(Design::label("Nom du vaccin *"), 0, 0, 1, 2);
    form->addWidget(m_vaccineName, 1, 0, 1, 2);

    m_vaccineType = new QLineEdit(v.vaccineType);
    m_vaccineType->setObjectName("vaccineType");
    m_vaccineType->setMaxLength(100);
    m_vaccineType->setPlaceholderText("Ex. Inactivé, Vivant atténué...");
    form->addWidget(Design::label("Type de vaccin"), 2, 0);

    m_veterinarian = new QLineEdit(v.veterinarian);
    m_veterinarian->setObjectName("vaccineVeterinarian");
    m_veterinarian->setMaxLength(100);
    m_veterinarian->setPlaceholderText("Nom du vétérinaire");
    form->addWidget(Design::label("Vétérinaire"), 2, 1);
    form->addWidget(m_vaccineType, 3, 0);
    form->addWidget(m_veterinarian, 3, 1);

    m_administeredDate = new QDateEdit(v.administeredDate.isValid() ? v.administeredDate : QDate::currentDate());
    m_administeredDate->setObjectName("administeredDate");
    m_administeredDate->setCalendarPopup(true);
    m_administeredDate->setDisplayFormat("dd/MM/yyyy");
    m_administeredDate->setMaximumDate(QDate::currentDate());
    form->addWidget(Design::label("Date d'administration *"), 4, 0);

    m_nextDueDate = new QDateEdit(v.nextDueDate.isValid() ? v.nextDueDate : QDate::currentDate().addYears(1));
    m_nextDueDate->setObjectName("nextDueDate");
    m_nextDueDate->setCalendarPopup(true);
    m_nextDueDate->setDisplayFormat("dd/MM/yyyy");
    m_nextDueDate->setMinimumDate(QDate::currentDate());
    form->addWidget(Design::label("Prochaine dose"), 4, 1);
    form->addWidget(m_administeredDate, 5, 0);
    form->addWidget(m_nextDueDate, 5, 1);

    m_batchNumber = new QLineEdit(v.batchNumber);
    m_batchNumber->setObjectName("batchNumber");
    m_batchNumber->setMaxLength(50);
    m_batchNumber->setPlaceholderText("Numéro de lot");
    form->addWidget(Design::label("Numéro de lot"), 6, 0, 1, 2);
    form->addWidget(m_batchNumber, 7, 0, 1, 2);

    m_notes = new QTextEdit;
    m_notes->setObjectName("vaccineNotes");
    m_notes->setAcceptRichText(false);
    m_notes->setPlainText(v.notes);
    m_notes->setPlaceholderText("Notes sur la vaccination...");
    m_notes->setFixedHeight(80);
    form->addWidget(Design::label("Notes"), 8, 0, 1, 2);
    form->addWidget(m_notes, 9, 0, 1, 2);

    m_reminderSet = new QCheckBox("Activer le rappel automatique");
    m_reminderSet->setObjectName("reminderSet");
    m_reminderSet->setChecked(v.isReminderSet);
    form->addWidget(m_reminderSet, 10, 0, 1, 2);

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
    cancel->setObjectName("cancelVaccination");
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    buttons->addWidget(cancel);

    if (!readOnly) {
        auto *save = Design::button(editing ? "Enregistrer" : "Créer", "check", "primary");
        save->setObjectName("saveVaccination");
        save->setDefault(true);
        connect(save, &QPushButton::clicked, this, &VaccinationDialog::validateAndAccept);
        buttons->addWidget(save);
    } else {
        m_vaccineName->setReadOnly(true);
        m_vaccineType->setReadOnly(true);
        m_veterinarian->setReadOnly(true);
        m_administeredDate->setReadOnly(true);
        m_nextDueDate->setReadOnly(true);
        m_batchNumber->setReadOnly(true);
        m_notes->setReadOnly(true);
        m_reminderSet->setEnabled(false);
    }

    root->addLayout(buttons);
    m_vaccineName->setFocus();
}

void VaccinationDialog::validateAndAccept()
{
    const bool valid = !m_vaccineName->text().trimmed().isEmpty() && m_administeredDate->date().isValid();

    m_vaccineName->setProperty("invalid", !valid);
    m_vaccineName->style()->unpolish(m_vaccineName);
    m_vaccineName->style()->polish(m_vaccineName);

    if (!valid) {
        m_error->setText("Le nom du vaccin et la date d'administration sont obligatoires.");
        m_error->show();
        m_vaccineName->setFocus();
        Motion::reveal(m_error, QColor("#F3F7F8"));
        return;
    }

    accept();
}

Vaccination VaccinationDialog::getVaccination() const
{
    Vaccination v;
    v.id = m_vaccination.id;
    v.animalId = m_animalId;
    v.vaccineName = m_vaccineName->text().trimmed();
    v.vaccineType = m_vaccineType->text().trimmed();
    v.administeredDate = m_administeredDate->date();
    v.nextDueDate = m_nextDueDate->date();
    v.veterinarian = m_veterinarian->text().trimmed();
    v.batchNumber = m_batchNumber->text().trimmed();
    v.notes = m_notes->toPlainText().trimmed();
    v.isReminderSet = m_reminderSet->isChecked();
    v.createdAt = m_vaccination.createdAt;
    v.updatedAt = QDateTime::currentDateTime();
    return v;
}
