#include "reminderdialog.h"
#include "smsservice.h"
#include "appointmentdialog.h"
#include "lookupdata.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QSettings>
#include <QBrush>
#include <QColor>
#include <QLocale>
#include <QMap>
#include <QSet>

namespace {
enum Col { ColCheck, ColAnimal, ColEspece, ColProprio, ColTel, ColVaccin, ColRappel, ColStatut, ColSms, ColCount };
const int kJoursAvant = 30;
const int kSmsPause = 7;   // pas de nouveau SMS automatique si un SMS est parti depuis moins de 7 jours
}

bool ReminderDialog::autoSendEnabled()
{
    return QSettings("ANIWORLD", "ANIWORLD").value("rappels/envoiAuto", false).toBool();
}

ReminderDialog::ReminderDialog(const QVector<MedicalRecord> &records, QWidget *parent)
    : QDialog(parent), m_records(records)
{
    setWindowTitle("Rappels vaccinaux");
    resize(1000, 560);

    m_sms = new SmsService(this);
    connect(m_sms, &SmsService::finished, this,
            [this](const QString &ref, bool ok, bool simulated, const QString &info) {
        if (ok && !simulated) {
            VaccinationManager::marquerSmsEnvoye(ref, QDate::currentDate());
            reload();
        }
        m_status->setText((ok ? QString::fromUtf8("✅ ") : QString::fromUtf8("❌ ")) + info);
    });

    auto *root = new QVBoxLayout(this);

    auto *title = new QLabel(QString::fromUtf8("💉  Rappels vaccinaux"));
    title->setObjectName("cardTitle");
    root->addWidget(title);

    m_summary = new QLabel();
    root->addWidget(m_summary);

    m_table = new QTableWidget(0, ColCount);
    m_table->setHorizontalHeaderLabels({ "", "Animal", "Espèce", "Propriétaire", "Téléphone (SMS)",
                                         "Vaccin", "Rappel prévu", "Statut", "Dernier SMS" });
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(ColCheck, 34);
    m_table->setColumnWidth(ColAnimal, 120);
    m_table->setColumnWidth(ColEspece, 70);
    m_table->setColumnWidth(ColProprio, 140);
    m_table->setColumnWidth(ColTel, 130);
    m_table->setColumnWidth(ColVaccin, 200);
    m_table->setColumnWidth(ColRappel, 95);
    m_table->setColumnWidth(ColStatut, 130);
    root->addWidget(m_table, 1);

    m_status = new QLabel();
    m_status->setWordWrap(true);
    root->addWidget(m_status);

    const QString mode = m_sms->isConfigured()
        ? QString::fromUtf8("Passerelle SMS : active")
        : QString::fromUtf8("Passerelle SMS non configurée : mode SIMULATION (aucun SMS réel n'est envoyé)");
    auto *modeLabel = new QLabel(mode);
    modeLabel->setObjectName("panelMeta");
    root->addWidget(modeLabel);

    m_autoSend = new QCheckBox("Envoyer automatiquement les SMS de rappel à l'ouverture de l'application "
                               "(vaccins en retard ou à échéance sous 7 jours)");
    m_autoSend->setChecked(autoSendEnabled());
    connect(m_autoSend, &QCheckBox::toggled, this, [](bool on) {
        QSettings("ANIWORLD", "ANIWORLD").setValue("rappels/envoiAuto", on);
    });
    root->addWidget(m_autoSend);

    auto *buttons = new QHBoxLayout();
    auto *btnSend = new QPushButton(QString::fromUtf8("📩  Envoyer SMS (cochés)"));
    auto *btnAll = new QPushButton(QString::fromUtf8("📨  Envoyer à tous"));
    auto *btnRenew = new QPushButton(QString::fromUtf8("✅  Vaccin fait (renouveler)"));
    auto *btnAdd = new QPushButton(QString::fromUtf8("➕  Nouveau vaccin"));
    auto *btnRdv = new QPushButton(QString::fromUtf8("📅  Rendez-vous…"));
    auto *btnClose = new QPushButton("Fermer");
    btnSend->setObjectName("primaryButton");
    for (QPushButton *b : {btnAll, btnRenew, btnAdd, btnRdv, btnClose})
        b->setObjectName("outlineButton");
    buttons->addWidget(btnSend);
    buttons->addWidget(btnAll);
    buttons->addWidget(btnRenew);
    buttons->addWidget(btnAdd);
    buttons->addWidget(btnRdv);
    buttons->addStretch();
    buttons->addWidget(btnClose);
    root->addLayout(buttons);

    connect(btnSend, &QPushButton::clicked, this, &ReminderDialog::onSendSelected);
    connect(btnAll, &QPushButton::clicked, this, &ReminderDialog::onSendAll);
    connect(btnRenew, &QPushButton::clicked, this, &ReminderDialog::onRenew);
    connect(btnAdd, &QPushButton::clicked, this, &ReminderDialog::onAddVaccine);
    connect(btnRdv, &QPushButton::clicked, this, [this]() {   // rendez-vous de consultation (SMS rappel / manqué)
        AppointmentDialog dlg(m_records, this);
        dlg.exec();
    });
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_table, &QTableWidget::itemChanged, this, &ReminderDialog::onItemChanged);

    reload();
}

void ReminderDialog::reload()
{
    m_loading = true;
    m_rows = VaccinationManager::aRappeler(m_records, kJoursAvant);
    m_table->setRowCount(0);

    int late = 0, soon = 0;
    const QLocale fr(QLocale::French);
    for (const Vaccination &v : std::as_const(m_rows)) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        const int d = v.joursRestants();
        const bool isLate = d < 0;
        isLate ? ++late : ++soon;
        const QString statut = isLate ? QString("En retard de %1 j").arg(-d)
                                      : (d == 0 ? QString("Aujourd'hui") : QString("Dans %1 j").arg(d));

        auto *chk = new QTableWidgetItem();
        chk->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        chk->setCheckState(Qt::Unchecked);
        m_table->setItem(row, ColCheck, chk);

        auto ro = [](const QString &t) {
            auto *it = new QTableWidgetItem(t);
            it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            return it;
        };
        m_table->setItem(row, ColAnimal, ro(v.nomAnimal + "  (" + v.idAnimal + ")"));
        m_table->setItem(row, ColEspece, ro(v.espece));
        m_table->setItem(row, ColProprio, ro(v.proprietaire));
        auto *tel = new QTableWidgetItem(v.telephone);          // éditable
        tel->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        m_table->setItem(row, ColTel, tel);
        m_table->setItem(row, ColVaccin, ro(v.nomVaccin));
        m_table->setItem(row, ColRappel, ro(fr.toString(v.dateRappel, "dd/MM/yyyy")));
        m_table->setItem(row, ColStatut, ro(statut));
        m_table->setItem(row, ColSms, ro(v.dateSms.isValid() ? fr.toString(v.dateSms, "dd/MM/yyyy") : QString::fromUtf8("—")));

        const QColor bg = isLate ? QColor("#FDECEA") : QColor("#FFF6E5");
        for (int c = 0; c < ColCount; ++c)
            m_table->item(row, c)->setBackground(QBrush(bg));
        m_table->item(row, ColStatut)->setForeground(QBrush(isLate ? QColor("#B42318") : QColor("#B54708")));
    }

    m_summary->setText(QString::fromUtf8("🔴 %1 en retard   ·   🟠 %2 à renouveler dans les %3 jours   ·   %4 animaux concernés")
                       .arg(late).arg(soon).arg(kJoursAvant)
                       .arg([this]() { QSet<QString> s; for (const auto &v : m_rows) s.insert(v.idAnimal); return s.size(); }()));
    if (m_rows.isEmpty())
        m_status->setText(QString::fromUtf8("✅ Aucun rappel : tous les vaccins sont à jour."));
    m_loading = false;
}

void ReminderDialog::onItemChanged()
{
    if (m_loading)
        return;
    // le téléphone est modifiable : on l'enregistre dès que la cellule change
    for (int r = 0; r < m_rows.size() && r < m_table->rowCount(); ++r) {
        const QString tel = m_table->item(r, ColTel)->text().trimmed();
        if (tel != m_rows[r].telephone) {
            m_rows[r].telephone = tel;
            VaccinationManager::enregistrerTelephone(m_rows[r].idVaccin, tel);
        }
    }
}

void ReminderDialog::sendRows(const QList<int> &rows)
{
    int sent = 0, noPhone = 0;
    for (int r : rows) {
        const Vaccination &v = m_rows[r];
        if (SmsService::normalizePhone(v.telephone).isEmpty()) { ++noPhone; continue; }
        m_sms->send(v.telephone, SmsService::composeReminder(v), v.idVaccin);
        ++sent;
    }
    if (sent == 0 && noPhone > 0)
        m_status->setText(QString::fromUtf8("❌ Aucun SMS lancé : numéro manquant ou invalide (double-clic sur la colonne Téléphone)."));
    else if (noPhone > 0)
        m_status->setText(QString("%1 SMS lancé(s), %2 ignoré(s) (numéro manquant ou invalide).").arg(sent).arg(noPhone));
}

void ReminderDialog::onSendSelected()
{
    QList<int> rows;
    for (int r = 0; r < m_table->rowCount(); ++r)
        if (m_table->item(r, ColCheck)->checkState() == Qt::Checked)
            rows << r;
    if (rows.isEmpty()) {
        QMessageBox::information(this, "Rappels", "Cochez au moins un animal.");
        return;
    }
    sendRows(rows);
}

void ReminderDialog::onSendAll()
{
    if (m_rows.isEmpty())
        return;
    if (QMessageBox::question(this, "Confirmation",
            QString("Envoyer un SMS de rappel pour les %1 vaccins listés ?").arg(m_rows.size()),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;
    QList<int> rows;
    for (int r = 0; r < m_rows.size(); ++r)
        rows << r;
    sendRows(rows);
}

// Le vaccin vient d'être fait : on crée la ligne suivante (même intervalle que le cycle précédent)
void ReminderDialog::onRenew()
{
    const int r = m_table->currentRow();
    if (r < 0 || r >= m_rows.size()) {
        QMessageBox::information(this, "Rappels", "Sélectionnez d'abord une ligne.");
        return;
    }
    const Vaccination &old = m_rows[r];
    int mois = 0;
    if (old.dateVaccin.isValid() && old.dateRappel.isValid()) {
        const int jours = old.dateVaccin.daysTo(old.dateRappel);
        mois = qRound(jours / 30.4);
    }
    if (mois <= 0)
        mois = VaccinationManager::intervalleParDefaut(old.espece, old.nomVaccin);

    if (QMessageBox::question(this, "Confirmation",
            QString("Confirmer que %1 a reçu le vaccin « %2 » aujourd'hui ?\nProchain rappel dans %3 mois.")
                .arg(old.nomAnimal, old.nomVaccin).arg(mois),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    Vaccination n = old;
    n.dateVaccin = QDate::currentDate();
    n.dateRappel = n.dateVaccin.addMonths(mois);
    n.dateSms = QDate();
    if (VaccinationManager::ajouter(n)) {
        m_status->setText(QString::fromUtf8("✅ Vaccin enregistré. Prochain rappel : %1")
                          .arg(QLocale(QLocale::French).toString(n.dateRappel, "dd/MM/yyyy")));
        reload();
    } else {
        QMessageBox::critical(this, "Erreur", "Enregistrement du vaccin impossible.");
    }
}

void ReminderDialog::onAddVaccine()
{
    // animaux connus (un par référence)
    QMap<QString, MedicalRecord> animaux;
    for (const MedicalRecord &r : std::as_const(m_records))
        if (!r.getIdAnimal().isEmpty() && !animaux.contains(r.getIdAnimal()))
            animaux.insert(r.getIdAnimal(), r);
    if (animaux.isEmpty()) {
        QMessageBox::information(this, "Rappels", "Aucun animal : créez d'abord un dossier médical.");
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("Nouveau vaccin");
    auto *form = new QFormLayout(&dlg);

    auto *comboAnimal = new QComboBox();
    for (auto it = animaux.cbegin(); it != animaux.cend(); ++it)
        comboAnimal->addItem(QString("%1 — %2").arg(it.key(), it.value().getNomAnimal()), it.key());
    auto *comboVaccin = new QComboBox();
    comboVaccin->setEditable(true);
    auto *dateVaccin = new QDateEdit(QDate::currentDate());
    dateVaccin->setCalendarPopup(true);
    dateVaccin->setDisplayFormat("dd/MM/yyyy");
    auto *dateRappel = new QDateEdit();
    dateRappel->setCalendarPopup(true);
    dateRappel->setDisplayFormat("dd/MM/yyyy");
    auto *tel = new QLineEdit();
    tel->setPlaceholderText("Ex : 20 123 456");

    auto currentRecord = [&]() { return animaux.value(comboAnimal->currentData().toString()); };
    auto updateRappel = [&]() {
        const MedicalRecord r = currentRecord();
        dateRappel->setDate(dateVaccin->date().addMonths(
            VaccinationManager::intervalleParDefaut(r.getEspece(), comboVaccin->currentText())));
    };
    auto updateAnimal = [&]() {
        const MedicalRecord r = currentRecord();
        comboVaccin->clear();
        for (const VaccineType &t : VaccinationManager::catalogue(r.getEspece()))
            comboVaccin->addItem(t.nom);
        tel->setText(LookupData::telephoneProprietaire(r.getProprietaire()));
        updateRappel();
    };
    QObject::connect(comboAnimal, &QComboBox::currentIndexChanged, &dlg, updateAnimal);
    QObject::connect(comboVaccin, &QComboBox::currentTextChanged, &dlg, updateRappel);
    QObject::connect(dateVaccin, &QDateEdit::dateChanged, &dlg, updateRappel);
    updateAnimal();

    form->addRow("Animal :", comboAnimal);
    form->addRow("Vaccin :", comboVaccin);
    form->addRow("Date du vaccin :", dateVaccin);
    form->addRow("Prochain rappel :", dateRappel);
    form->addRow("Téléphone propriétaire :", tel);
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted)
        return;
    if (comboVaccin->currentText().trimmed().isEmpty() || dateRappel->date() <= dateVaccin->date()) {
        QMessageBox::warning(this, "Nouveau vaccin", "Vaccin manquant, ou rappel avant la date du vaccin.");
        return;
    }

    const MedicalRecord r = currentRecord();
    Vaccination v;
    v.idAnimal = r.getIdAnimal();
    v.nomAnimal = r.getNomAnimal();
    v.espece = r.getEspece();
    v.proprietaire = r.getProprietaire();
    v.telephone = tel->text().trimmed();
    v.nomVaccin = comboVaccin->currentText().trimmed();
    v.dateVaccin = dateVaccin->date();
    v.dateRappel = dateRappel->date();
    if (VaccinationManager::ajouter(v)) {
        m_status->setText(QString::fromUtf8("✅ Vaccin ajouté pour %1.").arg(v.nomAnimal));
        reload();
    } else {
        QMessageBox::critical(this, "Erreur", "Ajout du vaccin impossible.");
    }
}

// ---------------------------------------------------------------- envoi automatique (démarrage)
int ReminderDialog::envoyerAutomatiquement(const QVector<MedicalRecord> &records, SmsService *sms)
{
    if (!autoSendEnabled() || !sms->isConfigured())
        return 0;       // en simulation on n'envoie rien automatiquement
    int n = 0;
    const QDate today = QDate::currentDate();
    for (const Vaccination &v : VaccinationManager::aRappeler(records, 7)) {
        if (v.dateSms.isValid() && v.dateSms.daysTo(today) < kSmsPause)
            continue;
        if (SmsService::normalizePhone(v.telephone).isEmpty())
            continue;
        sms->send(v.telephone, SmsService::composeReminder(v), v.idVaccin);
        ++n;
    }
    return n;
}
