#include "medicalrecorddialog.h"
#include "treatmentadvisor.h"
#include "fieldvalidator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
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
#include <QCompleter>
#include <QTimer>
#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QMouseEvent>
#include <QCheckBox>
#include <QDateTimeEdit>
#include <QScopeGuard>
#include <QRegularExpression>
#include "smsservice.h"
#include <algorithm>

// ====================================================================
//  CheckableComboBox : sélection multiple (traitements)
// ====================================================================
namespace {
const int RoleLabel = Qt::UserRole + 1;
const int RoleValue = Qt::UserRole + 2;
const int RoleChecked = Qt::UserRole + 3;
}

CheckableComboBox::CheckableComboBox(QWidget *parent) : QComboBox(parent)
{
    m_model = new QStandardItemModel(this);
    setModel(m_model);
    setEditable(true);
    setCompleter(nullptr);
    lineEdit()->setReadOnly(true);
    lineEdit()->setPlaceholderText("Choisir un ou plusieurs traitements");
    lineEdit()->installEventFilter(this);
    view()->viewport()->installEventFilter(this);
}

void CheckableComboBox::clearItems()
{
    m_model->clear();
    updateText();
}

void CheckableComboBox::addCheckItem(const QString &label, const QString &value, bool checked)
{
    auto *item = new QStandardItem;
    item->setData(label, RoleLabel);
    item->setData(value, RoleValue);
    item->setData(checked, RoleChecked);
    item->setText((checked ? QString::fromUtf8("☑  ") : QString::fromUtf8("☐  ")) + label);
    m_model->appendRow(item);
    updateText();
}

QStringList CheckableComboBox::checkedValues() const
{
    QStringList out;
    for (int i = 0; i < m_model->rowCount(); ++i)
        if (m_model->item(i)->data(RoleChecked).toBool())
            out << m_model->item(i)->data(RoleValue).toString();
    return out;
}

void CheckableComboBox::setCheckedValues(const QStringList &values)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStandardItem *it = m_model->item(i);
        const bool on = values.contains(it->data(RoleValue).toString(), Qt::CaseInsensitive);
        it->setData(on, RoleChecked);
        it->setText((on ? QString::fromUtf8("☑  ") : QString::fromUtf8("☐  ")) + it->data(RoleLabel).toString());
    }
    // valeurs qui n'existent plus dans les listes (ancien dossier) : on les garde cochées
    for (const QString &v : values) {
        bool found = false;
        for (int i = 0; i < m_model->rowCount() && !found; ++i)
            found = m_model->item(i)->data(RoleValue).toString().compare(v, Qt::CaseInsensitive) == 0;
        if (!found && !v.trimmed().isEmpty())
            addCheckItem(v.trimmed(), v.trimmed(), true);
    }
    updateText();
}

void CheckableComboBox::updateText()
{
    lineEdit()->setText(checkedValues().join(", "));
    lineEdit()->setCursorPosition(0);
}

bool CheckableComboBox::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == view()->viewport() && event->type() == QEvent::MouseButtonRelease) {
        const QModelIndex idx = view()->indexAt(static_cast<QMouseEvent *>(event)->position().toPoint());
        if (idx.isValid()) {
            QStandardItem *it = m_model->itemFromIndex(idx);
            const bool on = !it->data(RoleChecked).toBool();
            it->setData(on, RoleChecked);
            it->setText((on ? QString::fromUtf8("☑  ") : QString::fromUtf8("☐  ")) + it->data(RoleLabel).toString());
            updateText();
        }
        return true;    // la liste reste ouverte pour cocher plusieurs traitements
    }
    if (obj == lineEdit() && event->type() == QEvent::MouseButtonPress) {
        if (!view()->isVisible())
            showPopup();
        return true;
    }
    return QComboBox::eventFilter(obj, event);
}

// ====================================================================
//  MedicalRecordDialog
// ====================================================================
MedicalRecordDialog::MedicalRecordDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Dossier médical");
    setMinimumSize(700, 600);
    resize(800, 700);

    m_races = LookupData::racesParEspece();
    m_stock = LookupData::traitementsEnStock();
    setupUI();
    fillTreatments();
}

void MedicalRecordDialog::setupUI()
{
    // Validation en temps réel : chaque champ est enveloppé dans un ValidatedField indépendant
    // (cadre vert/rouge animé, icône ✓/✕, message d'erreur). Les widgets d'origine restent inchangés.
    using VF = ValidatedField;
    auto vf = [this](QWidget *w, VF::Validator v, bool required) {
        auto *f = new VF(w, std::move(v), required);
        m_validated.append(f);
        return f;
    };
    static const QRegularExpression reNom(QString::fromUtf8("^[\\p{L}0-9 '’.\\-]+$"));
    static const QRegularExpression reRef("^[A-Za-z]{0,6}-?\\d{1,9}$");
    static const QRegularExpression reIdDossier("^([A-Za-z]+-)?\\d+$");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollWidget = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setSpacing(20);

    // ---------------- Card 1 - Animal ----------------
    QFrame *animalCard = new QFrame();
    animalCard->setObjectName("dialogCard");
    QVBoxLayout *animalCardLayout = new QVBoxLayout(animalCard);

    QLabel *animalTitle = new QLabel("Informations de l'animal");
    animalTitle->setObjectName("cardTitle");
    animalCardLayout->addWidget(animalTitle);

    QGridLayout *animalGrid = new QGridLayout();
    animalGrid->setSpacing(12);

    animalGrid->addWidget(new QLabel("Réf. animal:"), 0, 0);
    m_editAnimalRef = new QLineEdit();
    m_editAnimalRef->setPlaceholderText("ANI-XXX (auto si nouveau)");
    {
        auto *f = vf(m_editAnimalRef, [this]() -> VF::Result {
            const QString t = m_editAnimalRef->text().trimmed();
            if (t.isEmpty())
                return VF::neutral();                                  // facultatif : générée automatiquement
            if (t.size() > 20)
                return VF::invalid("Maximum 20 caractères");
            if (!reRef.match(t).hasMatch())
                return VF::invalid("Format invalide : attendu ANI-012 ou 12");
            return VF::valid();
        }, false);
        f->watch(m_editAnimalRef, &QLineEdit::textChanged);
        animalGrid->addWidget(f, 0, 1);
    }

    animalGrid->addWidget(new QLabel("Nom:"), 0, 2);
    m_editAnimalName = new QLineEdit();
    {
        auto *f = vf(m_editAnimalName, [this]() -> VF::Result {
            const QString raw = m_editAnimalName->text();
            const QString t = raw.trimmed();
            if (t.isEmpty())
                return VF::invalid(raw.isEmpty() ? "Le nom de l'animal est obligatoire" : "Le nom ne peut pas être vide");
            if (t.size() < 2)
                return VF::invalid("Minimum 2 caractères");
            if (t.size() > 100)
                return VF::invalid("Maximum 100 caractères");
            if (!reNom.match(t).hasMatch())
                return VF::invalid("Caractères non autorisés (lettres, chiffres, espace, - ' .)");
            return VF::valid();
        }, true);
        f->watch(m_editAnimalName, &QLineEdit::textChanged);
        animalGrid->addWidget(f, 0, 3);
    }

    animalGrid->addWidget(new QLabel("Espèce:"), 1, 0);
    m_comboSpecies = new QComboBox();
    m_comboSpecies->addItems(m_races.keys());
    m_comboSpecies->setPlaceholderText("Choisir l'espèce");
    m_comboSpecies->setCurrentIndex(-1);
    {
        auto *f = vf(m_comboSpecies, [this]() -> VF::Result {
            if (m_comboSpecies->currentIndex() < 0)
                return VF::invalid("Choisissez l'espèce");
            if (m_comboSpecies->currentText().size() > 50)
                return VF::invalid("Maximum 50 caractères");
            return VF::valid();
        }, true);
        f->watch(m_comboSpecies, &QComboBox::currentIndexChanged);
        animalGrid->addWidget(f, 1, 1);
    }

    animalGrid->addWidget(new QLabel("Race:"), 1, 2);
    m_comboBreed = new QComboBox();
    m_comboBreed->setPlaceholderText("Choisir d'abord l'espèce");
    m_comboBreed->setEnabled(false);
    {
        m_vfRace = vf(m_comboBreed, [this]() -> VF::Result {
            if (m_comboSpecies->currentIndex() < 0)
                return VF::invalid("Choisissez d'abord l'espèce");
            if (m_comboBreed->currentIndex() < 0)
                return VF::invalid("Choisissez la race");
            if (m_comboBreed->currentText().size() > 100)
                return VF::invalid("Maximum 100 caractères");
            return VF::valid();
        }, true);
        m_vfRace->watch(m_comboBreed, &QComboBox::currentIndexChanged);
        animalGrid->addWidget(m_vfRace, 1, 3);
    }

    animalGrid->addWidget(new QLabel("Âge:"), 2, 0);
    m_spinAge = new QSpinBox();
    m_spinAge->setRange(0, 30);
    m_spinAge->setSuffix(" ans");
    {
        auto *f = vf(m_spinAge, [this]() -> VF::Result {
            return (m_spinAge->value() >= 0 && m_spinAge->value() <= 30) ? VF::valid()
                                                                          : VF::invalid("L'âge doit être entre 0 et 30 ans");
        }, false);
        f->watch(m_spinAge, QOverload<int>::of(&QSpinBox::valueChanged));
        animalGrid->addWidget(f, 2, 1);
    }

    animalGrid->addWidget(new QLabel("Sexe:"), 2, 2);
    m_comboSex = new QComboBox();
    m_comboSex->addItem("Mâle");
    m_comboSex->addItem("Femelle");
    animalGrid->addWidget(m_comboSex, 2, 3);

    animalGrid->addWidget(new QLabel("Propriétaire:"), 3, 0);
    m_comboOwner = new QComboBox();
    for (const LookupItem &o : LookupData::proprietaires())
        m_comboOwner->addItem(o.label, o.id);
    m_comboOwner->setPlaceholderText("Choisir le propriétaire");
    m_comboOwner->setCurrentIndex(-1);
    {
        auto *f = vf(m_comboOwner, [this]() -> VF::Result {
            if (m_comboOwner->currentIndex() < 0)
                return VF::invalid("Choisissez le propriétaire");
            if (m_comboOwner->currentText().trimmed().size() > 100)
                return VF::invalid("Maximum 100 caractères");
            return VF::valid();
        }, true);
        f->watch(m_comboOwner, &QComboBox::currentIndexChanged);
        animalGrid->addWidget(f, 3, 1, 1, 3);
    }

    animalCardLayout->addLayout(animalGrid);
    scrollLayout->addWidget(animalCard);

    connect(m_comboSpecies, &QComboBox::currentIndexChanged, this, &MedicalRecordDialog::onSpeciesChanged);
    connect(m_editAnimalRef, &QLineEdit::editingFinished, this, &MedicalRecordDialog::onAnimalRefEntered);

    // ---------------- Card 2 - Dossier médical ----------------
    QFrame *medicalCard = new QFrame();
    medicalCard->setObjectName("dialogCard");
    QVBoxLayout *medicalCardLayout = new QVBoxLayout(medicalCard);

    QLabel *medicalTitle = new QLabel("Dossier médical");
    medicalTitle->setObjectName("cardTitle");
    medicalCardLayout->addWidget(medicalTitle);

    QGridLayout *medicalGrid = new QGridLayout();
    medicalGrid->setSpacing(12);

    medicalGrid->addWidget(new QLabel("ID dossier:"), 0, 0);
    QLineEdit *recordIdEdit = new QLineEdit();
    recordIdEdit->setReadOnly(true);                    // clé primaire : générée par l'application (MAX+1)
    recordIdEdit->setPlaceholderText("Généré automatiquement");
    m_editRecordId = recordIdEdit;
    {
        auto *f = vf(recordIdEdit, [this]() -> VF::Result {
            const QString t = m_editRecordId->text().trimmed();
            if (!m_editing || t.isEmpty())
                return VF::neutral();                                  // nouveau dossier : numéro attribué à l'enregistrement
            return reIdDossier.match(t).hasMatch() ? VF::valid() : VF::invalid("Identifiant de dossier invalide");
        }, false);
        f->watch(recordIdEdit, &QLineEdit::textChanged);
        medicalGrid->addWidget(f, 0, 1, 1, 3);
    }

    medicalGrid->addWidget(new QLabel("Employé:"), 1, 0);
    m_comboEmployee = new QComboBox();
    for (const LookupItem &e : LookupData::employes())
        m_comboEmployee->addItem(e.label, e.id);       // texte = nom, donnée = ID employé
    m_comboEmployee->setPlaceholderText("Choisir l'employé");
    m_comboEmployee->setCurrentIndex(-1);
    {
        auto *f = vf(m_comboEmployee, [this]() -> VF::Result {
            return m_comboEmployee->currentIndex() < 0 ? VF::invalid("Choisissez l'employé responsable") : VF::valid();
        }, true);
        f->watch(m_comboEmployee, &QComboBox::currentIndexChanged);
        medicalGrid->addWidget(f, 1, 1, 1, 3);
    }

    medicalGrid->addWidget(new QLabel("Date consultation:"), 2, 0);
    m_dateConsultation = new QDateEdit();
    m_dateConsultation->setCalendarPopup(true);          // clic sur la flèche = calendrier
    m_dateConsultation->setDisplayFormat("dd/MM/yyyy");
    m_dateConsultation->setDate(QDate::currentDate());
    {
        auto *f = vf(m_dateConsultation, [this]() -> VF::Result {
            const QDate d = m_dateConsultation->date();
            if (!d.isValid())
                return VF::invalid("Date invalide");
            if (d > QDate::currentDate())
                return VF::invalid("La date ne peut pas être dans le futur");
            if (d < QDate(1990, 1, 1))
                return VF::invalid("Date trop ancienne (avant 1990)");
            return VF::valid();
        }, true);
        f->watch(m_dateConsultation, &QDateEdit::dateChanged);
        medicalGrid->addWidget(f, 2, 1);
    }

    medicalGrid->addWidget(new QLabel("Motif:"), 3, 0);
    m_editReason = new QTextEdit();
    m_editReason->setMaximumHeight(60);
    {
        auto *f = vf(m_editReason, [this]() -> VF::Result {
            const QString raw = m_editReason->toPlainText();
            const QString t = raw.trimmed();
            if (t.isEmpty())
                return VF::invalid(raw.isEmpty() ? "Le motif est obligatoire" : "Le motif ne peut pas être vide");
            if (t.size() < 3)
                return VF::invalid("Minimum 3 caractères");
            if (t.size() > 200)
                return VF::invalid(QString("Maximum 200 caractères (%1 saisis)").arg(t.size()));
            return VF::valid();
        }, true);
        f->watch(m_editReason, &QTextEdit::textChanged);
        medicalGrid->addWidget(f, 3, 1, 1, 3);
    }

    medicalGrid->addWidget(new QLabel("Diagnostic:"), 4, 0);
    m_comboDiagnosis = new QComboBox();
    m_comboDiagnosis->setEditable(true);
    m_comboDiagnosis->setInsertPolicy(QComboBox::NoInsert);
    m_comboDiagnosis->lineEdit()->setPlaceholderText("Choisir un diagnostic ou en saisir un nouveau");
    {
        auto *f = vf(m_comboDiagnosis, [this]() -> VF::Result {
            const QString t = m_comboDiagnosis->currentText().trimmed();
            if (t.isEmpty())
                return VF::neutral();                                  // facultatif
            if (t.size() < 3)
                return VF::invalid("Minimum 3 caractères");
            if (t.size() > 500)
                return VF::invalid(QString("Maximum 500 caractères (%1 saisis)").arg(t.size()));
            return VF::valid();
        }, false);
        f->watch(m_comboDiagnosis, &QComboBox::editTextChanged);
        medicalGrid->addWidget(f, 4, 1, 1, 3);
    }

    medicalGrid->addWidget(new QLabel("Traitement:"), 5, 0);
    m_comboTreatment = new CheckableComboBox();
    medicalGrid->addWidget(m_comboTreatment, 5, 1, 1, 3);

    // Prochain rendez-vous : un SMS de rappel part automatiquement la veille,
    // et un SMS "rendez-vous manqué" le lendemain si l'animal n'est pas venu.
    m_chkRdv = new QCheckBox("Planifier le prochain rendez-vous :");
    medicalGrid->addWidget(m_chkRdv, 6, 0, 1, 2);
    m_dateRdv = new QDateTimeEdit(QDateTime(QDate::currentDate().addDays(14), QTime(10, 0)));
    m_dateRdv->setCalendarPopup(true);
    m_dateRdv->setDisplayFormat("dd/MM/yyyy HH:mm");
    m_dateRdv->setMinimumDateTime(QDateTime::currentDateTime());
    m_dateRdv->setEnabled(false);
    medicalGrid->addWidget(m_dateRdv, 6, 2, 1, 2);

    medicalGrid->addWidget(new QLabel("Téléphone propriétaire:"), 7, 0);
    m_editPhone = new QLineEdit();
    m_editPhone->setPlaceholderText("Ex : 20 123 456 (enregistré avec le rendez-vous)");
    medicalGrid->addWidget(m_editPhone, 7, 1, 1, 3);
    connect(m_chkRdv, &QCheckBox::toggled, m_dateRdv, &QWidget::setEnabled);

    medicalCardLayout->addLayout(medicalGrid);
    scrollLayout->addWidget(medicalCard);

    // ---------------- Card 2b - Assistant diagnostic (état de l'animal + traitements suggérés) ----------------
    QFrame *assistCard = new QFrame();
    assistCard->setObjectName("dialogCard");
    QVBoxLayout *assistLayout = new QVBoxLayout(assistCard);
    QLabel *assistTitle = new QLabel(QString::fromUtf8("💡  Assistant diagnostic"));
    assistTitle->setObjectName("cardTitle");
    assistLayout->addWidget(assistTitle);

    m_stateLabel = new QLabel();
    m_stateLabel->setWordWrap(true);
    m_stateLabel->setObjectName("panelMeta");
    assistLayout->addWidget(m_stateLabel);

    m_suggestBox = new QWidget();
    m_suggestLayout = new QVBoxLayout(m_suggestBox);
    m_suggestLayout->setContentsMargins(0, 0, 0, 0);
    assistLayout->addWidget(m_suggestBox);

    m_btnApplySuggest = new QPushButton(QString::fromUtf8("✔  Ajouter les traitements cochés"));
    m_btnApplySuggest->setObjectName("outlineButton");
    m_btnApplySuggest->hide();
    connect(m_btnApplySuggest, &QPushButton::clicked, this, &MedicalRecordDialog::onApplySuggestions);
    assistLayout->addWidget(m_btnApplySuggest, 0, Qt::AlignLeft);

    QLabel *assistNote = new QLabel("Suggestions basées sur les dossiers déjà enregistrés : le vétérinaire valide toujours.");
    assistNote->setObjectName("panelMeta");
    assistLayout->addWidget(assistNote);
    scrollLayout->addWidget(assistCard);

    // recalcul (avec petite temporisation) quand le diagnostic, l'animal, le poids ou les allergies changent
    m_suggestTimer = new QTimer(this);
    m_suggestTimer->setSingleShot(true);
    m_suggestTimer->setInterval(350);
    connect(m_suggestTimer, &QTimer::timeout, this, &MedicalRecordDialog::refreshSuggestions);
    auto planifier = [this]() { m_suggestTimer->start(); };
    connect(m_comboDiagnosis, &QComboBox::editTextChanged, this, planifier);
    connect(m_comboSpecies, &QComboBox::currentIndexChanged, this, planifier);
    connect(m_editAnimalRef, &QLineEdit::textChanged, this, planifier);
    connect(m_spinAge, QOverload<int>::of(&QSpinBox::valueChanged), this, planifier);

    // ---------------- Card 3 - Santé ----------------
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
    {
        auto *f = vf(m_spinWeight, [this]() -> VF::Result {
            const double v = m_spinWeight->value();
            if (v <= 0.0)
                return VF::neutral();                                  // facultatif : 0 = non renseigné
            if (v > 100.0)
                return VF::invalid("Le poids doit être inférieur ou égal à 100 kg");
            return VF::valid();
        }, false);
        f->watch(m_spinWeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged));
        healthGrid->addWidget(f, 2, 1);
    }
    connect(m_spinWeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { m_suggestTimer->start(); });
    connect(m_editAllergies, &QTextEdit::textChanged, this, [this]() { m_suggestTimer->start(); });

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

// ---------------------------------------------------------------- données liées
void MedicalRecordDialog::setExistingRecords(const QVector<MedicalRecord> &records)
{
    m_existing = records;

    // Animaux connus (le premier dossier rencontré = le plus récent, la liste est triée par date DESC)
    m_animalsByRef.clear();
    for (const MedicalRecord &r : records)
        if (!r.getIdAnimal().isEmpty() && !m_animalsByRef.contains(r.getIdAnimal()))
            m_animalsByRef.insert(r.getIdAnimal(), r);

    auto *completer = new QCompleter(m_animalsByRef.keys(), m_editAnimalRef);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    connect(completer, QOverload<const QString &>::of(&QCompleter::activated), this,
            [this]() { QTimer::singleShot(0, this, &MedicalRecordDialog::onAnimalRefEntered); });
    m_editAnimalRef->setCompleter(completer);

    // Diagnostics déjà saisis (les plus fréquents d'abord) + quelques diagnostics courants
    const QString current = m_comboDiagnosis->currentText();
    QMap<QString, int> counts;
    QMap<QString, QString> display;
    for (const MedicalRecord &r : records) {
        const QString d = r.getDiagnostic().trimmed();
        if (d.isEmpty()) continue;
        counts[d.toLower()]++;
        if (!display.contains(d.toLower())) display[d.toLower()] = d;
    }
    QList<QPair<int, QString>> sorted;
    for (auto it = counts.cbegin(); it != counts.cend(); ++it)
        sorted.append({ it.value(), display[it.key()] });
    std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) {
        return a.first != b.first ? a.first > b.first : a.second.localeAwareCompare(b.second) < 0; });

    QStringList diag;
    for (const auto &p : sorted) diag << p.second;
    const QStringList common = { "Gastro-entérite", "Otite", "Dermatite", "Infection respiratoire",
                                 "Parasitose", "Plaie / blessure", "Contrôle de routine" };
    for (const QString &c : common)
        if (!counts.contains(c.toLower())) diag << c;

    m_comboDiagnosis->clear();
    m_comboDiagnosis->addItems(diag);
    m_comboDiagnosis->setCurrentIndex(-1);
    m_comboDiagnosis->setEditText(current);

    fillTreatments();
    if (m_suggestTimer)
        m_suggestTimer->start();     // l'historique est maintenant connu
}

void MedicalRecordDialog::fillTreatments()
{
    const QStringList checked = m_comboTreatment->checkedValues();
    m_comboTreatment->clearItems();

    QStringList inStock;
    for (const LookupItem &t : m_stock) {            // 1) disponibles au centre
        m_comboTreatment->addCheckItem(QString("%1   (%2 en stock)").arg(t.label).arg(t.quantity), t.label);
        inStock << t.label.toLower();
    }

    QStringList history;                              // 2) déjà prescrits dans d'anciens dossiers
    for (const MedicalRecord &r : m_existing)
        for (const QString &part : r.getTraitement().split(';', Qt::SkipEmptyParts)) {
            const QString t = part.trimmed();
            if (!t.isEmpty() && !inStock.contains(t.toLower()) && !history.contains(t, Qt::CaseInsensitive))
                history << t;
        }
    history.sort(Qt::CaseInsensitive);
    for (const QString &t : history)
        m_comboTreatment->addCheckItem(QString("%1   (déjà prescrit)").arg(t), t);

    m_comboTreatment->setCheckedValues(checked);
}

// ---------------------------------------------------------------- diagnostic intelligent
void MedicalRecordDialog::refreshSuggestions()
{
    // 1) état de l'animal
    const QStringList etat = TreatmentAdvisor::etatAnimal(
        m_editAnimalRef->text().trimmed(), m_comboSpecies->currentText(), m_spinAge->value(),
        m_spinWeight->value(), m_editAllergies->toPlainText(), m_existing);
    m_stateLabel->setText(etat.join("\n"));

    // 2) traitements suggérés pour le diagnostic saisi
    m_suggestChecks.clear();                                     // les cases sont détruites avec leur layout
    while (QLayoutItem *item = m_suggestLayout->takeAt(0)) {     // vide les anciennes lignes
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }

    const QList<TreatmentSuggestion> list = TreatmentAdvisor::suggerer(
        m_comboDiagnosis->currentText(), m_comboSpecies->currentText(), m_editAnimalRef->text().trimmed(),
        m_editAllergies->toPlainText(), m_existing, m_stock, m_recordId);

    if (list.isEmpty()) {
        auto *hint = new QLabel(m_comboDiagnosis->currentText().trimmed().size() < 3
            ? QString::fromUtf8("Saisissez un diagnostic : les traitements les plus utilisés seront proposés ici.")
            : QString::fromUtf8("Aucune suggestion pour ce diagnostic (pas encore d'historique)."));
        hint->setObjectName("panelMeta");
        m_suggestLayout->addWidget(hint);
        m_btnApplySuggest->hide();
        return;
    }

    for (const TreatmentSuggestion &s : list) {
        auto *chk = new QCheckBox(s.nom);
        QFont f = chk->font();
        f.setBold(true);
        chk->setFont(f);
        chk->setProperty("traitement", s.nom);
        // coché d'office : traitement fréquent, sans allergie, issu de l'historique
        chk->setChecked(!s.allergie && !s.baseConnaissance && s.percent >= 50);
        auto *why = new QLabel(s.raison);
        why->setObjectName("panelMeta");
        why->setWordWrap(true);
        why->setContentsMargins(24, 0, 0, 4);
        if (s.allergie)
            why->setStyleSheet("color:#B42318; font-weight:700;");
        m_suggestLayout->addWidget(chk);
        m_suggestLayout->addWidget(why);
        m_suggestChecks.append(chk);
    }
    m_btnApplySuggest->show();
}

void MedicalRecordDialog::onApplySuggestions()
{
    QStringList values = m_comboTreatment->checkedValues();
    for (QCheckBox *c : std::as_const(m_suggestChecks))
        if (c->isChecked() && !values.contains(c->property("traitement").toString(), Qt::CaseInsensitive))
            values << c->property("traitement").toString();
    m_comboTreatment->setCheckedValues(values);
}

void MedicalRecordDialog::onSpeciesChanged()
{
    // la liste des races est reconstruite : le champ Race repart à l'état neutre
    const auto resetRace = qScopeGuard([this]() { if (m_vfRace) m_vfRace->resetTouched(); });
    const QString species = m_comboSpecies->currentText();
    m_comboBreed->clear();
    if (m_comboSpecies->currentIndex() < 0 || !m_races.contains(species)) {
        m_comboBreed->setEnabled(false);
        return;
    }
    m_comboBreed->addItems(m_races.value(species));
    m_comboBreed->setCurrentIndex(-1);
    m_comboBreed->setPlaceholderText("Choisir la race");
    m_comboBreed->setEnabled(true);
}

void MedicalRecordDialog::selectComboText(QComboBox *combo, const QString &text)
{
    if (text.trimmed().isEmpty()) {
        combo->setCurrentIndex(-1);
        return;
    }
    int idx = combo->findText(text, Qt::MatchFixedString);   // insensible à la casse
    if (idx < 0) {
        combo->addItem(text);
        idx = combo->count() - 1;
    }
    combo->setCurrentIndex(idx);
}

QString MedicalRecordDialog::nextAnimalRef() const
{
    int maxNum = 0;
    for (auto it = m_animalsByRef.cbegin(); it != m_animalsByRef.cend(); ++it)
        maxNum = qMax(maxNum, it.key().section('-', 1).toInt());
    return QString("ANI-%1").arg(maxNum + 1, 3, 10, QChar('0'));
}

// Saisie / choix d'une référence connue : on remplit automatiquement la fiche de l'animal
void MedicalRecordDialog::onAnimalRefEntered()
{
    if (m_editing)
        return;
    const QString ref = m_editAnimalRef->text().trimmed();
    for (auto it = m_animalsByRef.cbegin(); it != m_animalsByRef.cend(); ++it) {
        if (it.key().compare(ref, Qt::CaseInsensitive) != 0)
            continue;
        const MedicalRecord &a = it.value();
        m_editAnimalRef->setText(it.key());
        m_editAnimalName->setText(a.getNomAnimal());
        selectComboText(m_comboSpecies, a.getEspece());   // remplit aussi la liste des races
        selectComboText(m_comboBreed, a.getRace());
        m_spinAge->setValue(a.getAge());
        if (!a.getSexe().isEmpty())
            m_comboSex->setCurrentText(a.getSexe());
        selectComboText(m_comboOwner, a.getProprietaire());
        m_editPhone->setText(RdvManager::dernierTelephone(it.key(), m_existing));
        return;
    }
}

// ---------------------------------------------------------------- modèle <-> formulaire
void MedicalRecordDialog::setMedicalRecord(const MedicalRecord& record)
{
    m_editing = true;
    m_recordId = record.getIdDossier();      // clé primaire conservée pour l'UPDATE
    m_editRecordId->setText(record.getIdDossier());

    m_editAnimalRef->setText(record.getIdAnimal());
    m_editAnimalName->setText(record.getNomAnimal());
    selectComboText(m_comboSpecies, record.getEspece());
    selectComboText(m_comboBreed, record.getRace());
    m_spinAge->setValue(record.getAge());
    if (!record.getSexe().isEmpty())
        m_comboSex->setCurrentText(record.getSexe());
    selectComboText(m_comboOwner, record.getProprietaire());

    int e = m_comboEmployee->findData(record.getIdEmployes());
    if (e < 0 && !record.getIdEmployes().isEmpty()) {
        m_comboEmployee->addItem(record.getIdEmployes(), record.getIdEmployes());
        e = m_comboEmployee->count() - 1;
    }
    m_comboEmployee->setCurrentIndex(e);

    m_dateConsultation->setDate(record.getDateConsultation().isValid() ? record.getDateConsultation()
                                                                         : QDate::currentDate());
    m_editReason->setText(record.getMotif());
    m_comboDiagnosis->setEditText(record.getDiagnostic());

    QStringList ts;
    for (const QString &p : record.getTraitement().split(';', Qt::SkipEmptyParts))
        if (!p.trimmed().isEmpty()) ts << p.trimmed();
    m_comboTreatment->setCheckedValues(ts);

    m_editVaccinations->setText(record.getVaccinations());
    m_editAllergies->setText(record.getAllergies());
    m_spinWeight->setValue(record.getPoids());
    m_comboVaccinationStatus->setCurrentText(record.getStatutVaccination());
    m_editObservations->setText(record.getObservations());
    m_editPhone->setText(RdvManager::dernierTelephone(record.getIdAnimal(), m_existing));
}

MedicalRecord MedicalRecordDialog::getMedicalRecord() const
{
    QString idAnimal = m_editAnimalRef->text().trimmed();
    if (idAnimal.isEmpty())
        idAnimal = nextAnimalRef();           // nouvel animal : référence générée automatiquement
    else if (const QVariant n = MedicalRecord::versNombre(idAnimal); !n.isNull())
        idAnimal = MedicalRecord::versTexte(n, "ANI");     // "12" et "ani-12" deviennent "ANI-012"

    const QString idDossier = m_recordId;     // vide à la création : généré par la fenêtre principale
    const QDate dateConsultation = m_dateConsultation->date();
    const QString motif = m_editReason->toPlainText().trimmed();
    const QString diagnostic = m_comboDiagnosis->currentText().trimmed();
    const QString nomAnimal = m_editAnimalName->text().trimmed();
    const QString espece = m_comboSpecies->currentText();
    const QString race = m_comboBreed->currentIndex() >= 0 ? m_comboBreed->currentText() : QString();
    const int age = m_spinAge->value();
    const double poids = m_spinWeight->value();
    const QString proprietaire = m_comboOwner->currentIndex() >= 0 ? m_comboOwner->currentText() : QString();
    const QString idEmployes = m_comboEmployee->currentData().toString();

    MedicalRecord M(idDossier, dateConsultation, motif, diagnostic, idAnimal, nomAnimal,
                    espece, race, age, poids, proprietaire, idEmployes);

    M.setSexe(m_comboSex->currentText());
    M.setTraitement(m_comboTreatment->checkedValues().join("; "));
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
    auto chosen = [this](const QComboBox *c, const QString &label) {
        if (c->currentIndex() >= 0)
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

    if (!required(m_editAnimalName->text(), "Le nom de l'animal") ||
        !chosen(m_comboSpecies, "L'espèce") ||
        !chosen(m_comboBreed, "La race") ||
        !chosen(m_comboOwner, "Le propriétaire") ||
        !chosen(m_comboEmployee, "L'employé") ||
        !required(m_editReason->toPlainText(), "Le motif"))
        return false;

    // REF_ANIMAL est un NUMBER dans la base : la référence doit contenir un numéro (ANI-012 ou 12)
    if (!m_editAnimalRef->text().trimmed().isEmpty()
        && MedicalRecord::versNombre(m_editAnimalRef->text()).isNull()) {
        QMessageBox::warning(this, "Saisie invalide",
                             "La référence animal doit se terminer par un numéro (ex : ANI-012 ou 12).");
        return false;
    }

    // La référence animal vide est générée automatiquement à l'enregistrement.
    if (!m_editPhone->text().trimmed().isEmpty() && SmsService::normalizePhone(m_editPhone->text()).isEmpty()) {
        QMessageBox::warning(this, "Saisie invalide", "Téléphone invalide (8 chiffres, ou +216 suivi de 8 chiffres).");
        return false;
    }
    if (m_chkRdv->isChecked()) {
        if (m_dateRdv->dateTime() <= QDateTime::currentDateTime()) {
            QMessageBox::warning(this, "Rendez-vous", "La date du rendez-vous doit être dans le futur.");
            return false;
        }
        if (SmsService::normalizePhone(m_editPhone->text()).isEmpty()) {
            QMessageBox::warning(this, "Rendez-vous",
                "Le téléphone du propriétaire est obligatoire pour envoyer le rappel SMS du rendez-vous.");
            return false;
        }
    }

    if (!maxLength(m_editAnimalRef->text(), 20, "La référence animal") ||
        !maxLength(m_editAnimalName->text(), 100, "Le nom de l'animal") ||
        !maxLength(m_comboOwner->currentText(), 100, "Le propriétaire") ||
        !maxLength(m_comboSpecies->currentText(), 50, "L'espèce") ||
        !maxLength(m_comboBreed->currentText(), 100, "La race") ||
        !maxLength(m_editReason->toPlainText(), 200, "Le motif") ||
        !maxLength(m_comboDiagnosis->currentText(), 500, "Le diagnostic") ||
        !maxLength(m_comboTreatment->checkedValues().join("; "), 500, "La liste des traitements"))
        return false;

    return true;
}

void MedicalRecordDialog::onSaveClicked()
{
    // 1) contrôle visuel de tous les champs (les erreurs s'affichent en rouge sous chaque champ)
    QWidget *premier = nullptr;
    for (ValidatedField *f : std::as_const(m_validated))
        if (!f->validateNow() && !premier)
            premier = f->field();
    if (premier) {
        premier->setFocus();
        return;
    }
    // 2) contrôles complémentaires (téléphone, rendez-vous...)
    if (validateForm()) {
        accept();
    }
}

void MedicalRecordDialog::onCancelClicked()
{
    reject();
}

// ---------------------------------------------------------------- rendez-vous
void MedicalRecordDialog::setRendezVous(const RendezVous &rdv)
{
    if (!rdv.dateRdv.isValid())
        return;
    m_dateRdv->clearMinimumDateTime();             // un rendez-vous existant peut déjà être dans le passé
    m_dateRdv->setDateTime(rdv.dateRdv);
    m_chkRdv->setChecked(true);
    if (!rdv.telephone.isEmpty())
        m_editPhone->setText(rdv.telephone);
}

bool MedicalRecordDialog::rdvPlanifie() const { return m_chkRdv->isChecked(); }
QDateTime MedicalRecordDialog::rdvDate() const { return m_dateRdv->dateTime(); }
QString MedicalRecordDialog::telephone() const { return m_editPhone->text().trimmed(); }
