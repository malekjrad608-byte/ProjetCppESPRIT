#include "animaldialog.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QShowEvent>

AnimalDialog::AnimalDialog(QWidget *parent)
    : QDialog(parent)
{
    setup(Animal{}, false, false);
}

AnimalDialog::AnimalDialog(const Animal &animal, bool readOnly, QWidget *parent)
    : QDialog(parent)
{
    setup(animal, animal.id > 0, readOnly);
}

void AnimalDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    Motion::reveal(this, QColor("#F3F7F8"), 180);
}

void AnimalDialog::setup(const Animal &a, bool editing, bool readOnly)
{
    m_animal = a;
    setObjectName("animalDialog");
    setMinimumWidth(550);
    setModal(true);

    const QString title = readOnly ? "Détails de l'animal" : editing ? "Modifier l'animal" : "Nouvel animal";
    setWindowTitle("ANIWORLD — " + title);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(27, 24, 27, 24);
    root->setSpacing(17);

    auto *head = new QHBoxLayout;
    auto *text = new QVBoxLayout;
    text->setSpacing(5);

    text->addWidget(Design::label(editing ? QString("ANIMAL  /  ANI-%1").arg(a.id, 3, 10, QChar('0')) : "NOUVEL ANIMAL", "eyebrow"));
    text->addWidget(Design::label(title, "title"));
    text->addWidget(Design::label(readOnly ? "Toutes les informations de cet animal." : "Enregistrez un nouvel animal dans le système.", "muted"));

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

    m_name = new QLineEdit(a.name);
    m_name->setObjectName("animalName");
    m_name->setMaxLength(100);
    m_name->setPlaceholderText("Ex. Max");
    form->addWidget(Design::label("Nom de l'animal *"), 0, 0, 1, 2);
    form->addWidget(m_name, 1, 0, 1, 2);

    m_species = new QLineEdit(a.species);
    m_species->setObjectName("animalSpecies");
    m_species->setMaxLength(50);
    m_species->setPlaceholderText("Ex. Chien");
    form->addWidget(Design::label("Espèce *"), 2, 0);

    m_breed = new QLineEdit(a.breed);
    m_breed->setObjectName("animalBreed");
    m_breed->setMaxLength(50);
    m_breed->setPlaceholderText("Ex. Labrador");
    form->addWidget(Design::label("Race"), 2, 1);
    form->addWidget(m_species, 3, 0);
    form->addWidget(m_breed, 3, 1);

    m_gender = new QComboBox;
    m_gender->setObjectName("animalGender");
    m_gender->addItems({"Mâle", "Femelle"});
    m_gender->setCurrentText(a.gender);
    form->addWidget(Design::label("Sexe"), 4, 0);

    m_birthDate = new QDateEdit(a.birthDate.isValid() ? a.birthDate : QDate::currentDate().addYears(-1));
    m_birthDate->setObjectName("animalBirthDate");
    m_birthDate->setCalendarPopup(true);
    m_birthDate->setDisplayFormat("dd/MM/yyyy");
    m_birthDate->setMaximumDate(QDate::currentDate());
    form->addWidget(Design::label("Date de naissance"), 4, 1);
    form->addWidget(m_gender, 5, 0);
    form->addWidget(m_birthDate, 5, 1);

    m_color = new QLineEdit(a.color);
    m_color->setObjectName("animalColor");
    m_color->setMaxLength(50);
    m_color->setPlaceholderText("Ex. Noir et blanc");
    form->addWidget(Design::label("Couleur"), 6, 0);

    m_ownerId = new QLineEdit(a.ownerId);
    m_ownerId->setObjectName("animalOwnerId");
    m_ownerId->setMaxLength(100);
    m_ownerId->setPlaceholderText("Nom du propriétaire");
    form->addWidget(Design::label("Propriétaire"), 6, 1);
    form->addWidget(m_color, 7, 0);
    form->addWidget(m_ownerId, 7, 1);

    m_ownerPhone = new QLineEdit(a.ownerPhone);
    m_ownerPhone->setObjectName("animalOwnerPhone");
    m_ownerPhone->setMaxLength(30);
    m_ownerPhone->setPlaceholderText("Ex. 0612345678");
    form->addWidget(Design::label("Téléphone"), 8, 0);

    m_ownerEmail = new QLineEdit(a.ownerEmail);
    m_ownerEmail->setObjectName("animalOwnerEmail");
    m_ownerEmail->setMaxLength(150);
    m_ownerEmail->setPlaceholderText("Ex. email@example.com");
    form->addWidget(Design::label("Email"), 8, 1);
    form->addWidget(m_ownerPhone, 9, 0);
    form->addWidget(m_ownerEmail, 9, 1);

    m_notes = new QTextEdit;
    m_notes->setObjectName("animalNotes");
    m_notes->setAcceptRichText(false);
    m_notes->setPlainText(a.notes);
    m_notes->setPlaceholderText("Notes supplémentaires sur l'animal...");
    m_notes->setFixedHeight(80);
    form->addWidget(Design::label("Notes"), 10, 0, 1, 2);
    form->addWidget(m_notes, 11, 0, 1, 2);

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
    cancel->setObjectName("cancelAnimal");
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    buttons->addWidget(cancel);

    if (!readOnly) {
        auto *save = Design::button(editing ? "Enregistrer" : "Créer", "check", "primary");
        save->setObjectName("saveAnimal");
        save->setDefault(true);
        connect(save, &QPushButton::clicked, this, &AnimalDialog::validateAndAccept);
        buttons->addWidget(save);
    } else {
        m_name->setReadOnly(true);
        m_species->setReadOnly(true);
        m_breed->setReadOnly(true);
        m_gender->setEnabled(false);
        m_birthDate->setReadOnly(true);
        m_color->setReadOnly(true);
        m_ownerId->setReadOnly(true);
        m_ownerPhone->setReadOnly(true);
        m_ownerEmail->setReadOnly(true);
        m_notes->setReadOnly(true);
    }

    root->addLayout(buttons);
    m_name->setFocus();
}

void AnimalDialog::validateAndAccept()
{
    const bool valid = !m_name->text().trimmed().isEmpty() && !m_species->text().trimmed().isEmpty();

    m_name->setProperty("invalid", !valid);
    m_name->style()->unpolish(m_name);
    m_name->style()->polish(m_name);

    if (!valid) {
        m_error->setText("Le nom et l'espèce sont obligatoires.");
        m_error->show();
        m_name->setFocus();
        Motion::reveal(m_error, QColor("#F3F7F8"));
        return;
    }

    accept();
}

Animal AnimalDialog::getAnimal() const
{
    Animal a;
    a.id = m_animal.id;
    a.name = m_name->text().trimmed();
    a.species = m_species->text().trimmed();
    a.breed = m_breed->text().trimmed();
    a.gender = m_gender->currentText();
    a.birthDate = m_birthDate->date();
    a.color = m_color->text().trimmed();
    a.ownerId = m_ownerId->text().trimmed();
    a.ownerPhone = m_ownerPhone->text().trimmed();
    a.ownerEmail = m_ownerEmail->text().trimmed();
    a.notes = m_notes->toPlainText().trimmed();
    a.createdAt = m_animal.createdAt;
    a.updatedAt = QDateTime::currentDateTime();
    return a;
}
