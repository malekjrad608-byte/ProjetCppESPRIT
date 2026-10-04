#include "donationwindow.h"
#include "ui_donation.h"

#include <algorithm>

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHash>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPageLayout>
#include <QPrinter>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QVector>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextDocument>
#include <QTextStream>
#include <QVBoxLayout>

namespace {

QString dialogStyle()
{
    return QStringLiteral(
        "QDialog{background:#F4F7F8;}"
        "QLabel{color:#263B42;}"
        "QLabel#dialogKicker{color:#728288;font-size:9pt;font-weight:700;}"
        "QLabel#dialogTitle{color:#225A64;font-size:18pt;font-weight:700;}"
        "QLabel#dialogSubtitle{color:#728288;font-size:9pt;}"
        "QLineEdit,QComboBox,QDateEdit,QDoubleSpinBox{background:#FFFFFF;color:#263B42;border:1px solid #D7E2E5;border-radius:6px;padding:7px 9px;min-height:26px;}"
        "QLineEdit:focus,QComboBox:focus,QDateEdit:focus,QDoubleSpinBox:focus{border:1px solid #397F8D;}"
        "QCheckBox{color:#263B42;spacing:8px;}"
        "QPushButton{background:#FFFFFF;color:#225A64;border:1px solid #D7E2E5;border-radius:6px;padding:8px 14px;min-height:20px;}"
        "QPushButton:hover{background:#E4F0F3;}"
        "QPushButton#primaryButton{background:#225A64;color:#FFFFFF;border:0;font-weight:700;}"
        "QPushButton#dangerButton{color:#AE5751;border-color:#E3C4C1;}"
        "QTableWidget{background:#FFFFFF;color:#263B42;border:1px solid #D7E2E5;gridline-color:#E7EEF0;}"
        "QTableWidget::item{padding:6px;}"
        "QHeaderView::section{background:#F4F7F8;color:#52666D;border:0;border-bottom:1px solid #D7E2E5;padding:8px;font-weight:700;}"
    );
}

QDialog *createDialog(QWidget *parent, const QString &kicker, const QString &title, const QString &subtitle, QSize size)
{
    QDialog *dialog = new QDialog(parent);
    dialog->setModal(true);
    dialog->setStyleSheet(dialogStyle());
    dialog->resize(size);
    dialog->setMinimumSize(size);

    QVBoxLayout *root = new QVBoxLayout(dialog);
    root->setContentsMargins(22, 20, 22, 20);
    root->setSpacing(12);

    QFrame *header = new QFrame(dialog);
    header->setStyleSheet(QStringLiteral("QFrame{background:#FFFFFF;border:1px solid #DDE7E9;border-radius:10px;}"));
    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(18, 14, 18, 14);
    headerLayout->setSpacing(3);

    QLabel *k = new QLabel(kicker, header);
    k->setObjectName(QStringLiteral("dialogKicker"));
    QLabel *t = new QLabel(title, header);
    t->setObjectName(QStringLiteral("dialogTitle"));
    QLabel *s = new QLabel(subtitle, header);
    s->setObjectName(QStringLiteral("dialogSubtitle"));
    s->setWordWrap(true);

    headerLayout->addWidget(k);
    headerLayout->addWidget(t);
    headerLayout->addWidget(s);
    root->addWidget(header);
    return dialog;
}

QFrame *createCardFrame(QWidget *parent)
{
    QFrame *frame = new QFrame(parent);
    frame->setStyleSheet(QStringLiteral("QFrame{background:#FFFFFF;border:1px solid #DDE7E9;border-radius:10px;}"));
    return frame;
}

QString normalizeDonorName(const QString &value)
{
    QString text = value.normalized(QString::NormalizationForm_D).toLower();
    QString result;
    result.reserve(text.size());

    for (const QChar ch : text) {
        if (ch.category() == QChar::Mark_NonSpacing || ch.category() == QChar::Mark_SpacingCombining || ch.category() == QChar::Mark_Enclosing) {
            continue;
        }
        if (ch.isLetterOrNumber()) {
            result.append(ch);
        } else if (!result.isEmpty() && result.back() != QLatin1Char(' ')) {
            result.append(QLatin1Char(' '));
        }
    }

    return result.trimmed();
}

int editDistance(const QString &a, const QString &b)
{
    const int n = a.size();
    const int m = b.size();
    QVector<int> previous(m + 1);
    QVector<int> current(m + 1);

    for (int j = 0; j <= m; ++j) {
        previous[j] = j;
    }

    for (int i = 1; i <= n; ++i) {
        current[0] = i;
        for (int j = 1; j <= m; ++j) {
            const int cost = (a.at(i - 1) == b.at(j - 1)) ? 0 : 1;
            current[j] = qMin(qMin(current[j - 1] + 1, previous[j] + 1), previous[j - 1] + cost);
        }
        previous.swap(current);
    }

    return previous[m];
}

bool donorNamesAreSimilar(const QString &left, const QString &right)
{
    const QString a = normalizeDonorName(left);
    const QString b = normalizeDonorName(right);
    if (a.isEmpty() || b.isEmpty()) {
        return false;
    }

    if (a == b) {
        return true;
    }

    if (editDistance(a, b) <= 2) {
        return true;
    }

    const QStringList aParts = a.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    const QStringList bParts = b.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (aParts.size() >= 2 && bParts.size() >= 2) {
        const QString aLast = aParts.back();
        const QString bLast = bParts.back();
        const QString aFirst = aParts.front();
        const QString bFirst = bParts.front();
        return aLast == bLast && editDistance(aFirst, bFirst) <= 2;
    }

    return false;
}

QString htmlEscape(const QString &value)
{
    QString escaped = value;
    escaped.replace('&', QStringLiteral("&amp;"));
    escaped.replace('<', QStringLiteral("&lt;"));
    escaped.replace('>', QStringLiteral("&gt;"));
    return escaped;
}

} // namespace

DonationWindow::DonationWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::DonationWindow)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("ANIWORLD — Gestion des Dons"));
    resize(1440, 900);
    setMinimumSize(1180, 760);

    m_cards = {
        ui->card0, ui->card1, ui->card2, ui->card3,
        ui->card4, ui->card5, ui->card6, ui->card7
    };

    for (QPushButton *card : m_cards) {
        configureCard(card);
    }

    ui->cardsScroll->setWidgetResizable(true);
    ui->cardsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->cardsScroll->setFrameShape(QFrame::NoFrame);
    ui->cardsScroll->setStyleSheet(
        QStringLiteral("QScrollArea{background:#FFFFFF;border:0;}QScrollBar:vertical{width:8px;background:#F4F7F8;}QScrollBar::handle:vertical{background:#C9D8DC;border-radius:4px;min-height:28px;}")
    );
    ui->cardsContent->setStyleSheet(QStringLiteral("QWidget#cardsContent{background:#FFFFFF;}"));

    ui->detailDonor->setReadOnly(true);
    ui->detailAmount->setReadOnly(true);
    ui->detailDate->setReadOnly(true);
    ui->detailPayment->setReadOnly(true);
    ui->detailType->setReadOnly(true);
    ui->detailCampaign->setReadOnly(true);

    for (QLineEdit *field : {ui->detailDonor, ui->detailAmount, ui->detailDate,
                             ui->detailPayment, ui->detailType, ui->detailCampaign}) {
        field->setFocusPolicy(Qt::NoFocus);
        field->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        field->setFixedHeight(32);
    }

    seedDemoData();
    recomputeAnomalies();

    connect(ui->btnDashboard, &QPushButton::clicked, this, [this]() {
        showInfo(QStringLiteral("Tableau de bord"),
                 QStringLiteral("Le prototype est centré sur le module Gestion des Dons. Le bouton Tableau de bord représente le retour vers la navigation ANIWORLD complète."));
    });
    connect(ui->btnNew, &QPushButton::clicked, this, &DonationWindow::openNewDonation);
    connect(ui->btnStatistics, &QPushButton::clicked, this, &DonationWindow::showStatistics);
    connect(ui->btnExportCsv, &QPushButton::clicked, this, &DonationWindow::exportCsv);
    connect(ui->btnExportPdf, &QPushButton::clicked, this, &DonationWindow::exportPdf);
    connect(ui->btnRecurring, &QPushButton::clicked, this, &DonationWindow::showRecurring);
    connect(ui->btnAnomalies, &QPushButton::clicked, this, &DonationWindow::showAnomalies);
    connect(ui->btnConsult, &QPushButton::clicked, this, &DonationWindow::showSelectedDonation);
    connect(ui->btnEdit, &QPushButton::clicked, this, &DonationWindow::editSelectedDonation);
    connect(ui->btnArchive, &QPushButton::clicked, this, &DonationWindow::archiveSelectedDonation);
    connect(ui->btnDelete, &QPushButton::clicked, this, &DonationWindow::deleteSelectedDonation);
    connect(ui->editSearch, &QLineEdit::textChanged, this, &DonationWindow::refreshList);
    connect(ui->comboStatus, &QComboBox::currentTextChanged, this, &DonationWindow::refreshList);
    connect(ui->comboType, &QComboBox::currentTextChanged, this, &DonationWindow::refreshList);
    connect(ui->comboSort, &QComboBox::currentTextChanged, this, &DonationWindow::refreshList);
    connect(ui->btnReset, &QPushButton::clicked, this, &DonationWindow::resetFilters);

    for (QPushButton *card : m_cards) {
        connect(card, &QPushButton::clicked, this, [this, card]() {
            bool ok = false;
            const int index = card->property("recordIndex").toInt(&ok);
            if (ok) {
                updateSelection(index);
            }
        });
    }

    if (!m_records.isEmpty()) {
        updateSelection(0);
    } else {
        refreshList();
    }
}

DonationWindow::~DonationWindow()
{
    delete ui;
}

QString DonationWindow::money(double amount) const
{
    return QStringLiteral("%1 TND").arg(QString::number(amount, 'f', 3));
}

QString DonationWindow::dateText(const QDate &date) const
{
    return date.isValid() ? date.toString(QStringLiteral("dd/MM/yyyy")) : QStringLiteral("—");
}

void DonationWindow::seedDemoData()
{
    m_records.clear();
    m_records.append({QStringLiteral("DON-001"), QStringLiteral("Ahmed Ben Ali"), QDate(2026,10,2), 150.000,
                      QStringLiteral("Carte bancaire"), QStringLiteral("Ponctuel"), QStringLiteral("Confirmé"),
                      QStringLiteral("Soutien aux animaux"), false, QDate(), QStringLiteral("Affectation libre"), false});
    m_records.append({QStringLiteral("DON-002"), QStringLiteral("Société Nour"), QDate(2026,9,28), 500.000,
                      QStringLiteral("Virement"), QStringLiteral("Campagne"), QStringLiteral("Confirmé"),
                      QStringLiteral("Aide aux soins"), false, QDate(), QStringLiteral("Don entreprise"), false});
    m_records.append({QStringLiteral("DON-003"), QStringLiteral("Sami H."), QDate(2026,9,25), 75.000,
                      QStringLiteral("Espèces"), QStringLiteral("Ponctuel"), QStringLiteral("Confirmé"),
                      QStringLiteral("Soutien aux animaux"), false, QDate(), QString(), false});
    m_records.append({QStringLiteral("DON-004"), QStringLiteral("Nour B."), QDate(2026,9,21), 100.000,
                      QStringLiteral("Carte bancaire"), QStringLiteral("Récurrent"), QStringLiteral("Confirmé"),
                      QStringLiteral("Nourrir les pensionnaires"), true, QDate(2026,10,21), QStringLiteral("Mensuel"), false});
    m_records.append({QStringLiteral("DON-005"), QStringLiteral("Sara Gharbi"), QDate(2026,9,18), 250.000,
                      QStringLiteral("Virement"), QStringLiteral("Ponctuel"), QStringLiteral("En attente"),
                      QStringLiteral("Aide aux soins"), false, QDate(), QStringLiteral("Vérification du paiement"), false});
    m_records.append({QStringLiteral("DON-006"), QStringLiteral("Clinique El Amal"), QDate(2026,9,14), 1200.000,
                      QStringLiteral("Virement"), QStringLiteral("Campagne"), QStringLiteral("Confirmé"),
                      QStringLiteral("Équipement médical"), false, QDate(), QStringLiteral("Don entreprise"), false});
    m_records.append({QStringLiteral("DON-007"), QStringLiteral("Mohamed Ali"), QDate(2026,9,10), 5000.000,
                      QStringLiteral("Virement"), QStringLiteral("Ponctuel"), QStringLiteral("À vérifier"),
                      QStringLiteral("Soutien exceptionnel"), false, QDate(), QStringLiteral("Montant inhabituellement élevé"), true});
    m_records.append({QStringLiteral("DON-008"), QStringLiteral("Mohamed A. Ali"), QDate(2026,9,9), 80.000,
                      QStringLiteral("Carte bancaire"), QStringLiteral("Ponctuel"), QStringLiteral("Confirmé"),
                      QStringLiteral("Soutien aux animaux"), false, QDate(), QStringLiteral("Nom proche d'un autre donateur"), true});
}

bool DonationWindow::recordMatchesFilters(const DonationRecord &record) const
{
    const QString query = ui->editSearch->text().trimmed().toLower();
    const QString status = ui->comboStatus->currentText();
    const QString type = ui->comboType->currentText();

    const bool queryMatches = query.isEmpty()
        || record.id.toLower().contains(query)
        || record.donor.toLower().contains(query)
        || record.campaign.toLower().contains(query);

    const bool statusMatches = status == QStringLiteral("Tous les statuts") || record.status == status;
    const bool typeMatches = type == QStringLiteral("Tous les types") || record.type == type;
    return queryMatches && statusMatches && typeMatches;
}

QList<int> DonationWindow::filteredIndexes() const
{
    QList<int> indexes;
    for (int i = 0; i < m_records.size(); ++i) {
        if (recordMatchesFilters(m_records.at(i))) {
            indexes.append(i);
        }
    }

    const QString sort = ui->comboSort->currentText();
    std::sort(indexes.begin(), indexes.end(), [this, &sort](int left, int right) {
        const DonationRecord &a = m_records.at(left);
        const DonationRecord &b = m_records.at(right);
        if (sort == QStringLiteral("Montant décroissant")) {
            return a.amount > b.amount;
        }
        if (sort == QStringLiteral("Donateur A → Z")) {
            return a.donor.toLower() < b.donor.toLower();
        }
        if (sort == QStringLiteral("Date ancienne → récente")) {
            return a.date < b.date;
        }
        return a.date > b.date;
    });
    return indexes;
}

int DonationWindow::selectedIndex() const
{
    if (m_records.isEmpty()) {
        return -1;
    }
    return qBound(0, m_selectedIndex, m_records.size() - 1);
}

void DonationWindow::configureCard(QPushButton *card)
{
    if (!card) {
        return;
    }
    card->setCursor(Qt::PointingHandCursor);
    card->setAutoDefault(false);
    card->setFlat(false);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    card->setMinimumHeight(112);
    card->setMaximumHeight(124);
    card->setProperty("recordIndex", -1);
}

void DonationWindow::ensureCardCount(int count)
{
    QGridLayout *grid = ui->cardsGrid;
    if (!grid) {
        return;
    }

    while (m_cards.size() < count) {
        QPushButton *card = new QPushButton(ui->cardsContent);
        card->setObjectName(QStringLiteral("card%1").arg(m_cards.size()));
        configureCard(card);
        const int cardNumber = m_cards.size();
        const int row = cardNumber / 2;
        const int column = cardNumber % 2;
        grid->addWidget(card, row, column);
        connect(card, &QPushButton::clicked, this, [this, card]() {
            bool ok = false;
            const int index = card->property("recordIndex").toInt(&ok);
            if (ok) {
                updateSelection(index);
            }
        });
        m_cards.append(card);
    }
}

void DonationWindow::applyCardStyle(int index, bool selected)
{
    if (index < 0 || index >= m_cards.size()) {
        return;
    }

    if (selected) {
        m_cards.at(index)->setStyleSheet(QStringLiteral(
            "QPushButton{background:#FFFFFF;color:#263B42;border:2px solid #E8A13F;border-radius:9px;text-align:left;padding:14px;font-size:10pt;}"
            "QPushButton:hover{background:#FFFDF8;}"));
    } else {
        m_cards.at(index)->setStyleSheet(QStringLiteral(
            "QPushButton{background:#FFFFFF;color:#263B42;border:1px solid #DCE6E9;border-radius:9px;text-align:left;padding:14px;font-size:10pt;}"
            "QPushButton:hover{background:#E4F0F3;border-color:#BFD4D8;}"));
    }
}

void DonationWindow::updateDetailPanel(const DonationRecord &r)
{
    ui->detailId->setText(r.id);
    ui->detailDonor->setText(r.donor);
    ui->detailAmount->setText(money(r.amount));
    ui->detailDate->setText(dateText(r.date));
    ui->detailPayment->setText(r.paymentMode);
    ui->detailType->setText(r.type + (r.recurring ? QStringLiteral(" · mensuel") : QString()));
    ui->detailCampaign->setText(r.campaign);
    ui->detailStatus->setText(r.status);
    ui->recurringBadge->setVisible(r.recurring);
    ui->anomalyBadge->setVisible(r.anomaly);
    const bool archived = (r.status == QStringLiteral("Archivé"));
    ui->btnEdit->setEnabled(!archived);
    ui->btnArchive->setEnabled(true);
    ui->btnArchive->setText(archived ? QStringLiteral("Désarchiver") : QStringLiteral("Archiver"));
    ui->btnDelete->setEnabled(!archived);
}

void DonationWindow::updateSelection(int index)
{
    if (index < 0 || index >= m_records.size()) {
        return;
    }

    m_selectedIndex = index;
    updateDetailPanel(m_records.at(index));

    for (int cardIndex = 0; cardIndex < m_cards.size(); ++cardIndex) {
        bool ok = false;
        const int recordIndex = m_cards.at(cardIndex)->property("recordIndex").toInt(&ok);
        if (ok && recordIndex >= 0) {
            applyCardStyle(cardIndex, recordIndex == m_selectedIndex);
        }
    }
}

void DonationWindow::refreshList()
{
    const QList<int> visibleIndexes = filteredIndexes();
    ensureCardCount(visibleIndexes.size());

    for (int cardIndex = 0; cardIndex < m_cards.size(); ++cardIndex) {
        QPushButton *card = m_cards.at(cardIndex);
        if (cardIndex >= visibleIndexes.size()) {
            card->setProperty("recordIndex", -1);
            card->setText(QString());
            card->setVisible(false);
            continue;
        }

        const int recordIndex = visibleIndexes.at(cardIndex);
        const DonationRecord &r = m_records.at(recordIndex);
        card->setProperty("recordIndex", recordIndex);

        QString statusLine = r.status;
        if (r.recurring) {
            statusLine += QStringLiteral(" · récurrent");
        }

        const QString cardText = r.id + QStringLiteral("   ") + r.donor + QStringLiteral("\n")
            + dateText(r.date) + QStringLiteral(" · ") + r.type + QStringLiteral("\n")
            + money(r.amount) + QStringLiteral("\n") + statusLine;
        card->setText(cardText);
        card->setVisible(true);
        applyCardStyle(cardIndex, recordIndex == m_selectedIndex);
    }

    ui->countShown->setText(QStringLiteral("%1 don(s) affiché(s)").arg(visibleIndexes.size()));
    updateStats();
}

void DonationWindow::recomputeAnomalies()
{
    for (int i = 0; i < m_records.size(); ++i) {
        DonationRecord &record = m_records[i];
        QStringList reasons;

        if (record.amount >= 3000.0) {
            reasons << QStringLiteral("Montant inhabituellement élevé");
        }

        for (int j = 0; j < m_records.size(); ++j) {
            if (i == j) {
                continue;
            }
            if (donorNamesAreSimilar(record.donor, m_records.at(j).donor)) {
                reasons << QStringLiteral("Nom très proche de « %1 » (possible doublon)").arg(m_records.at(j).donor);
                break;
            }
        }

        record.anomaly = !reasons.isEmpty();
        record.anomalyReason = reasons.join(QStringLiteral(" · "));
    }
}

void DonationWindow::updateStats()
{
    double confirmedTotal = 0.0;
    int confirmedCount = 0;

    for (const DonationRecord &r : m_records) {
        if (r.status == QStringLiteral("Confirmé")) {
            confirmedTotal += r.amount;
            ++confirmedCount;
        }
    }

    ui->statTotal->setText(money(confirmedTotal));
    ui->statDonors->setText(QString::number(confirmedCount));
    ui->statAverage->setText(confirmedCount > 0 ? money(confirmedTotal / confirmedCount) : QStringLiteral("0.000 TND"));
}

void DonationWindow::resetFilters()
{
    ui->editSearch->clear();
    ui->comboStatus->setCurrentIndex(0);
    ui->comboType->setCurrentIndex(0);
    ui->comboSort->setCurrentIndex(0);
}

void DonationWindow::showInfo(const QString &title, const QString &message)
{
    QMessageBox box(this);
    box.setWindowTitle(title);
    box.setText(message);
    box.setIcon(QMessageBox::Information);
    box.setStyleSheet(QStringLiteral(
        "QMessageBox{background:#F4F7F8;}QLabel{color:#263B42;min-width:360px;}"
        "QPushButton{background:#225A64;color:#FFFFFF;border:0;border-radius:6px;padding:7px 14px;}"));
    box.exec();
}

bool DonationWindow::confirmAction(const QString &title, const QString &message)
{
    QMessageBox box(this);
    box.setWindowTitle(title);
    box.setText(message);
    box.setIcon(QMessageBox::Question);
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    box.setStyleSheet(QStringLiteral(
        "QMessageBox{background:#F4F7F8;}QLabel{color:#263B42;min-width:340px;}"
        "QPushButton{background:#FFFFFF;color:#225A64;border:1px solid #D7E2E5;border-radius:6px;padding:7px 14px;}"));
    return box.exec() == QMessageBox::Yes;
}

void DonationWindow::openNewDonation()
{
    QDialog *dialog = createDialog(this,
        QStringLiteral("NOUVEAU DON"),
        QStringLiteral("Enregistrer un don"),
        QStringLiteral("Créer une nouvelle contribution et conserver les informations essentielles dans l'historique."),
        QSize(620, 580));

    QVBoxLayout *root = qobject_cast<QVBoxLayout *>(dialog->layout());
    QFrame *formFrame = createCardFrame(dialog);
    QFormLayout *form = new QFormLayout(formFrame);
    form->setContentsMargins(18, 16, 18, 16);
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(10);

    QLineEdit *donor = new QLineEdit(formFrame);
    donor->setPlaceholderText(QStringLiteral("Nom du donateur"));
    QDoubleSpinBox *amount = new QDoubleSpinBox(formFrame);
    amount->setRange(0.001, 1000000.0);
    amount->setDecimals(3);
    amount->setSuffix(QStringLiteral(" TND"));
    QDateEdit *date = new QDateEdit(QDate::currentDate(), formFrame);
    date->setCalendarPopup(true);
    QComboBox *mode = new QComboBox(formFrame);
    mode->addItems({QStringLiteral("Carte bancaire"), QStringLiteral("Virement"), QStringLiteral("Espèces")});
    QComboBox *type = new QComboBox(formFrame);
    type->addItems({QStringLiteral("Ponctuel"), QStringLiteral("Récurrent"), QStringLiteral("Campagne")});
    QLineEdit *campaign = new QLineEdit(formFrame);
    campaign->setPlaceholderText(QStringLiteral("Soutien général, soins, équipement..."));
    QCheckBox *recurring = new QCheckBox(QStringLiteral("Prévoir un prochain don mensuel"), formFrame);

    form->addRow(QStringLiteral("Donateur"), donor);
    form->addRow(QStringLiteral("Montant"), amount);
    form->addRow(QStringLiteral("Date"), date);
    form->addRow(QStringLiteral("Mode"), mode);
    form->addRow(QStringLiteral("Type"), type);
    form->addRow(QStringLiteral("Campagne"), campaign);
    form->addRow(QString(), recurring);
    root->addWidget(formFrame, 1);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save, dialog);
    if (QPushButton *saveButton = buttons->button(QDialogButtonBox::Save)) {
        saveButton->setObjectName(QStringLiteral("primaryButton"));
    }
    root->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, dialog,
            [this, dialog, donor, amount, date, mode, type, campaign, recurring]() {
        if (donor->text().trimmed().isEmpty() || amount->value() <= 0.0) {
            showInfo(QStringLiteral("Données incomplètes"),
                     QStringLiteral("Le nom du donateur et un montant strictement positif sont obligatoires."));
            return;
        }

        DonationRecord record;
        record.id = QStringLiteral("DON-%1").arg(m_records.size() + 1, 3, 10, QLatin1Char('0'));
        record.donor = donor->text().trimmed();
        record.date = date->date();
        record.amount = amount->value();
        record.paymentMode = mode->currentText();
        record.type = type->currentText();
        record.status = QStringLiteral("Confirmé");
        record.campaign = campaign->text().trimmed().isEmpty() ? QStringLiteral("Soutien général") : campaign->text().trimmed();
        record.recurring = recurring->isChecked() || record.type == QStringLiteral("Récurrent");
        record.nextDate = record.recurring ? record.date.addMonths(1) : QDate();
        record.note = record.recurring ? QStringLiteral("Prochain don mensuel prévu.") : QString();
        m_records.append(record);
        recomputeAnomalies();
        m_selectedIndex = m_records.size() - 1;
        dialog->accept();
    });

    dialog->exec();
    if (m_selectedIndex >= 0 && m_selectedIndex < m_records.size()) {
        updateSelection(m_selectedIndex);
    }
    refreshList();
}

void DonationWindow::showStatistics()
{
    QDialog *dialog = createDialog(this,
        QStringLiteral("STATISTIQUES"),
        QStringLiteral("Vue des dons"),
        QStringLiteral("Une synthèse rapide du volume, des dons récurrents et des modes de paiement."),
        QSize(700, 530));
    QVBoxLayout *root = qobject_cast<QVBoxLayout *>(dialog->layout());

    double confirmedTotal = 0.0;
    int confirmedCount = 0;
    int pendingCount = 0;
    int recurringCount = 0;
    QHash<QString, double> byMode;

    for (const DonationRecord &r : m_records) {
        if (r.status == QStringLiteral("Confirmé")) {
            confirmedTotal += r.amount;
            ++confirmedCount;
        }
        if (r.status == QStringLiteral("En attente")) {
            ++pendingCount;
        }
        if (r.recurring) {
            ++recurringCount;
        }
        byMode[r.paymentMode] += r.amount;
    }

    QFrame *summary = createCardFrame(dialog);
    QVBoxLayout *summaryLayout = new QVBoxLayout(summary);
    summaryLayout->setContentsMargins(18, 16, 18, 16);
    summaryLayout->setSpacing(7);

    QLabel *summaryLabel = new QLabel(
        QStringLiteral("Dons confirmés : %1\nMontant confirmé : %2\nDons en attente : %3\nDons récurrents : %4\n\nCarte bancaire : %5\nVirement : %6\nEspèces : %7")
            .arg(confirmedCount)
            .arg(money(confirmedTotal))
            .arg(pendingCount)
            .arg(recurringCount)
            .arg(money(byMode.value(QStringLiteral("Carte bancaire"))))
            .arg(money(byMode.value(QStringLiteral("Virement"))))
            .arg(money(byMode.value(QStringLiteral("Espèces")))),
        dialog);
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet(QStringLiteral("QLabel{color:#263B42;font-size:11pt;line-height:1.3;}"));
    summaryLayout->addWidget(summaryLabel);
    root->addWidget(summary);

    QLabel *goalLabel = new QLabel(
        QStringLiteral("Objectif mensuel : 6 000 TND · réalisé : %1").arg(money(confirmedTotal)),
        dialog);
    goalLabel->setStyleSheet(QStringLiteral("QLabel{background:#E4F0F3;color:#225A64;border-radius:7px;padding:10px 12px;font-weight:700;}"));
    root->addWidget(goalLabel);

    QPushButton *close = new QPushButton(QStringLiteral("Fermer"), dialog);
    root->addWidget(close, 0, Qt::AlignRight);
    connect(close, &QPushButton::clicked, dialog, &QDialog::accept);
    dialog->exec();
    delete dialog;
}

void DonationWindow::exportCsv()
{
    const QString path = QFileDialog::getSaveFileName(this,
        QStringLiteral("Exporter les dons"),
        QStringLiteral("dons_aniworld.csv"),
        QStringLiteral("CSV (*.csv)"));
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        showInfo(QStringLiteral("Export impossible"), QStringLiteral("Le fichier ne peut pas être créé."));
        return;
    }

    QTextStream out(&file);
    out << "ID;Donateur;Date;Montant TND;Mode;Type;Statut;Campagne;Récurrent;Prochain don\n";
    for (int index : filteredIndexes()) {
        const DonationRecord &r = m_records.at(index);
        out << r.id << ';'
            << r.donor << ';'
            << dateText(r.date) << ';'
            << QString::number(r.amount, 'f', 3) << ';'
            << r.paymentMode << ';'
            << r.type << ';'
            << r.status << ';'
            << r.campaign << ';'
            << (r.recurring ? QStringLiteral("Oui") : QStringLiteral("Non")) << ';'
            << dateText(r.nextDate) << '\n';
    }

    file.close();
    showInfo(QStringLiteral("Export terminé"), QStringLiteral("La liste filtrée des dons a été exportée en CSV."));
}

void DonationWindow::exportPdf()
{
    const QString path = QFileDialog::getSaveFileName(this,
        QStringLiteral("Exporter le relevé"),
        QStringLiteral("releve_dons_aniworld.pdf"),
        QStringLiteral("PDF (*.pdf)"));
    if (path.isEmpty()) {
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    printer.setPageMargins(QMarginsF(14, 14, 14, 14), QPageLayout::Millimeter);

    QString html = QStringLiteral(
        "<html><body style='font-family:Segoe UI;color:#263B42'>"
        "<h1 style='color:#225A64'>ANIWORLD · Relevé des dons</h1>"
        "<p style='color:#728288'>Données fictives · Gestion des Dons</p>"
        "<table cellspacing='0' cellpadding='7' width='100%'>"
        "<tr style='background:#E4F0F3'><th align='left'>Donateur</th><th align='left'>Date</th>"
        "<th align='right'>Montant</th><th align='left'>Statut</th></tr>");

    for (int index : filteredIndexes()) {
        const DonationRecord &r = m_records.at(index);
        html += QStringLiteral("<tr><td>%1</td><td>%2</td><td align='right'>%3</td><td>%4</td></tr>")
            .arg(htmlEscape(r.donor),
                 htmlEscape(dateText(r.date)),
                 htmlEscape(money(r.amount)),
                 htmlEscape(r.status));
    }

    html += QStringLiteral("</table></body></html>");

    QTextDocument document;
    document.setHtml(html);
    document.print(&printer);

    showInfo(QStringLiteral("PDF généré"), QStringLiteral("Le relevé des dons a été enregistré en PDF."));
}

void DonationWindow::showRecurring()
{
    QDialog *dialog = createDialog(this,
        QStringLiteral("DONS RÉCURRENTS"),
        QStringLiteral("Contributions à venir"),
        QStringLiteral("Vue des dons mensuels programmés à partir des enregistrements actifs."),
        QSize(720, 500));
    QVBoxLayout *root = qobject_cast<QVBoxLayout *>(dialog->layout());

    QTableWidget *table = new QTableWidget(dialog);
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({QStringLiteral("Donateur"), QStringLiteral("Montant"), QStringLiteral("Prochaine date"), QStringLiteral("Campagne")});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    for (const DonationRecord &r : m_records) {
        if (!r.recurring || r.status == QStringLiteral("Archivé")) {
            continue;
        }
        const int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(r.donor));
        table->setItem(row, 1, new QTableWidgetItem(money(r.amount)));
        table->setItem(row, 2, new QTableWidgetItem(dateText(r.nextDate)));
        table->setItem(row, 3, new QTableWidgetItem(r.campaign));
    }

    root->addWidget(table, 1);
    QPushButton *close = new QPushButton(QStringLiteral("Fermer"), dialog);
    root->addWidget(close, 0, Qt::AlignRight);
    connect(close, &QPushButton::clicked, dialog, &QDialog::accept);
    dialog->exec();
    delete dialog;
}

void DonationWindow::showAnomalies()
{
    QDialog *dialog = createDialog(this,
        QStringLiteral("ANOMALIES"),
        QStringLiteral("Dons à vérifier"),
        QStringLiteral("Les situations inhabituelles sont signalées pour contrôle humain ; aucune correction automatique n'est effectuée."),
        QSize(760, 500));
    QVBoxLayout *root = qobject_cast<QVBoxLayout *>(dialog->layout());

    QTableWidget *table = new QTableWidget(dialog);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({QStringLiteral("Donateur"), QStringLiteral("Montant"), QStringLiteral("Motif")});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    for (const DonationRecord &r : m_records) {
        if (!r.anomaly) {
            continue;
        }
        const int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(r.donor));
        table->setItem(row, 1, new QTableWidgetItem(money(r.amount)));
        table->setItem(row, 2, new QTableWidgetItem(r.anomalyReason));
    }

    root->addWidget(table, 1);
    QPushButton *close = new QPushButton(QStringLiteral("Fermer"), dialog);
    root->addWidget(close, 0, Qt::AlignRight);
    connect(close, &QPushButton::clicked, dialog, &QDialog::accept);
    dialog->exec();
    delete dialog;
}

void DonationWindow::showSelectedDonation()
{
    const int index = selectedIndex();
    if (index < 0) {
        showInfo(QStringLiteral("Aucun don"), QStringLiteral("Aucun don n'est disponible à consulter."));
        return;
    }

    const DonationRecord &r = m_records.at(index);
    showInfo(QStringLiteral("Détail du don"),
             QStringLiteral("%1\n\nDonateur : %2\nMontant : %3\nDate : %4\nMode : %5\nType : %6\nStatut : %7\nCampagne : %8")
                 .arg(r.id,
                      r.donor,
                      money(r.amount),
                      dateText(r.date),
                      r.paymentMode,
                      r.type,
                      r.status,
                      r.campaign));
}

void DonationWindow::editSelectedDonation()
{
    const int index = selectedIndex();
    if (index < 0) {
        return;
    }

    const DonationRecord &record = m_records[index];
    if (record.status == QStringLiteral("Archivé")) {
        showInfo(QStringLiteral("Modification impossible"), QStringLiteral("Un don archivé ne peut plus être modifié."));
        return;
    }

    QDialog *dialog = createDialog(this,
        QStringLiteral("MODIFIER LE DON"),
        QStringLiteral("Modifier le dossier"),
        QStringLiteral("Les modifications concernent les informations de gestion du don."),
        QSize(620, 420));
    QVBoxLayout *root = qobject_cast<QVBoxLayout *>(dialog->layout());

    QFrame *formFrame = createCardFrame(dialog);
    QFormLayout *form = new QFormLayout(formFrame);
    form->setContentsMargins(18, 16, 18, 16);
    form->setVerticalSpacing(10);
    form->setHorizontalSpacing(18);

    QLineEdit *donor = new QLineEdit(record.donor, formFrame);
    QDoubleSpinBox *amount = new QDoubleSpinBox(formFrame);
    amount->setRange(0.001, 1000000.0);
    amount->setDecimals(3);
    amount->setSuffix(QStringLiteral(" TND"));
    amount->setValue(record.amount);
    QLineEdit *campaign = new QLineEdit(record.campaign, formFrame);

    form->addRow(QStringLiteral("Donateur"), donor);
    form->addRow(QStringLiteral("Montant"), amount);
    form->addRow(QStringLiteral("Campagne"), campaign);
    root->addWidget(formFrame, 1);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save, dialog);
    if (QPushButton *saveButton = buttons->button(QDialogButtonBox::Save)) {
        saveButton->setObjectName(QStringLiteral("primaryButton"));
    }
    root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, dialog, [this, dialog, index, donor, amount, campaign]() {
        if (donor->text().trimmed().isEmpty() || amount->value() <= 0.0) {
            showInfo(QStringLiteral("Données incomplètes"),
                     QStringLiteral("Le nom du donateur et un montant strictement positif sont obligatoires."));
            return;
        }
        if (index >= 0 && index < m_records.size()) {
            DonationRecord &edited = m_records[index];
            edited.donor = donor->text().trimmed();
            edited.amount = amount->value();
            edited.campaign = campaign->text().trimmed().isEmpty() ? QStringLiteral("Soutien général") : campaign->text().trimmed();
            recomputeAnomalies();
        }
        dialog->accept();
    });

    dialog->exec();
    delete dialog;
    updateSelection(index);
    refreshList();
}

void DonationWindow::archiveSelectedDonation()
{
    const int index = selectedIndex();
    if (index < 0 || index >= m_records.size()) {
        return;
    }

    const DonationRecord record = m_records.at(index);
    if (record.status == QStringLiteral("Archivé")) {
        if (!confirmAction(QStringLiteral("Désarchiver le don"),
                           QStringLiteral("Désarchiver %1 et le remettre dans les dons actifs ?").arg(record.id))) {
            return;
        }
        m_records[index].status = QStringLiteral("Confirmé");
    } else {
        if (!confirmAction(QStringLiteral("Archiver le don"),
                           QStringLiteral("Archiver %1 ? Le dossier restera conservé dans l'historique.").arg(record.id))) {
            return;
        }
        m_records[index].status = QStringLiteral("Archivé");
    }

    updateSelection(index);
    refreshList();
}
void DonationWindow::deleteSelectedDonation()
{
    const int index = selectedIndex();
    if (index < 0) {
        return;
    }

    const QString id = m_records.at(index).id;
    if (!confirmAction(QStringLiteral("Supprimer le don"), QStringLiteral("Supprimer %1 ?").arg(id))) {
        return;
    }

    m_records.removeAt(index);
    recomputeAnomalies();
    if (m_records.isEmpty()) {
        m_selectedIndex = -1;
        for (QPushButton *card : m_cards) {
            card->setVisible(false);
        }
        ui->detailId->clear();
        ui->detailDonor->clear();
        ui->detailAmount->clear();
        ui->detailDate->clear();
        ui->detailPayment->clear();
        ui->detailType->clear();
        ui->detailCampaign->clear();
        refreshList();
        return;
    }

    m_selectedIndex = qMin(index, m_records.size() - 1);
    updateSelection(m_selectedIndex);
    refreshList();
}
