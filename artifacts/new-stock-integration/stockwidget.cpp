#include "stockwidget.h"
#include "inventoryfeatures.h"

#include <QAbstractItemView>
#include <QColor>
#include <QModelIndex>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTableView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QFont>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <algorithm>
#include <functional>

namespace {
const QString kPrimary = QStringLiteral("#397F8D");
const QString kPrimaryDark = QStringLiteral("#225A64");
const QString kInk = QStringLiteral("#263B42");
const QString kMuted = QStringLiteral("#71878D");
const QString kBorder = QStringLiteral("#D8E9EC");

void appendLocalStockMovement(int articleId, const QString &type, int quantity, const QString &reference = QString())
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    QSettings settings(dir + QStringLiteral("/aniworld_features.ini"), QSettings::IniFormat);
    QVector<QVariantMap> rows;
    const int count = settings.beginReadArray("stockMovements");
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        QVariantMap row;
        row.insert("articleId", settings.value("articleId"));
        row.insert("type", settings.value("type"));
        row.insert("quantity", settings.value("quantity"));
        row.insert("date", settings.value("date"));
        row.insert("reference", settings.value("reference"));
        rows.append(row);
    }
    settings.endArray();
    QVariantMap row;
    row.insert("articleId", articleId); row.insert("type", type); row.insert("quantity", quantity);
    row.insert("date", QDate::currentDate().toString(Qt::ISODate)); row.insert("reference", reference);
    rows.append(row);
    settings.beginWriteArray("stockMovements");
    for (int i = 0; i < rows.size(); ++i) {
        settings.setArrayIndex(i);
        const QVariantMap values = rows.at(i);
        for (auto it = values.cbegin(); it != values.cend(); ++it) settings.setValue(it.key(), it.value());
    }
    settings.endArray(); settings.sync();
}

QString prefixForCategory(const QString &category) {
    if (category == "Chiens") return "[CHIEN] ";
    if (category == "Chats") return "[CHAT] ";
    if (category == "Oiseaux") return "[OISEAU] ";
    if (category == "Poissons") return "[POISSON] ";
    if (category == "Rongeurs") return "[HAMSTER] ";
    if (category == "Matériel Clinique") return "[MATÉRIEL] ";
    return QString();
}

QString categoryFromArticle(const QString &article) {
    if (article.startsWith("[CHIEN]")) return "Chiens";
    if (article.startsWith("[CHAT]")) return "Chats";
    if (article.startsWith("[OISEAU]")) return "Oiseaux";
    if (article.startsWith("[POISSON]")) return "Poissons";
    if (article.startsWith("[HAMSTER]")) return "Rongeurs";
    if (article.startsWith("[MATÉRIEL]")) return "Matériel Clinique";
    return "Matériel Clinique";
}

QString articleName(const QString &article) {
    const int endPrefix = article.indexOf("]");
    if (article.startsWith("[") && endPrefix >= 0) {
        return article.mid(endPrefix + 1).trimmed();
    }
    return article.trimmed();
}

QPushButton *makeButton(const QString &text, const QString &background,
                        const QString &foreground, const QString &border = QStringLiteral("none")) {
    QPushButton *button = new QPushButton(text);
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(36);
    button->setStyleSheet(QStringLiteral(
                              "QPushButton { background: %1; color: %2; border: %3; border-radius: 6px; padding: 8px 12px; font-weight: 600; }"
                              "QPushButton:hover { background: %4; }"
                              "QPushButton:disabled { color: #9AAEB2; background: #EDF3F4; border: 1px solid #D8E5E7; }"
                              ).arg(background, foreground, border,
                                   background == "#FFFFFF" ? "#F0F7F8" : (background == "#FCEDEC" ? "#F9DEDC" : kPrimaryDark)));
    return button;
}

class ProductEditDialog : public QDialog {
public:
    ProductEditDialog(const StockRecord &source, const QString &title, QWidget *parent = nullptr)
        : QDialog(parent), record(source) {
        setWindowTitle(title);
        setModal(true);
        setMinimumWidth(470);
        setStyleSheet(QStringLiteral(
                          "QDialog { background: #EAF4F6; color: %1; font-family: 'Segoe UI'; }"
                          "QLineEdit, QComboBox, QSpinBox, QDateEdit { background: white; color: %1; border: 1px solid #C9E0E5; border-radius: 5px; padding: 6px; min-height: 24px; }"
                          "QPushButton { border-radius: 5px; padding: 7px 14px; }"
                          ).arg(kInk));

        QVBoxLayout *layout = new QVBoxLayout(this);
        QFormLayout *form = new QFormLayout;
        form->setSpacing(10);
        form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        categoryBox = new QComboBox;
        categoryBox->addItems({"Chiens", "Chats", "Oiseaux", "Poissons", "Rongeurs", "Matériel Clinique"});
        categoryBox->setCurrentText(source.category);

        articleEdit = new QLineEdit(articleName(source.article));
        articleEdit->setPlaceholderText("Nom de l'article");
        lotEdit = new QLineEdit(source.lot);
        lotEdit->setPlaceholderText("Référence du lot");

        quantitySpin = new QSpinBox;
        quantitySpin->setRange(0, 999999);
        quantitySpin->setValue(source.quantity);
        thresholdSpin = new QSpinBox;
        thresholdSpin->setRange(1, 999999);
        thresholdSpin->setValue(qMax(1, source.threshold));

        unitBox = new QComboBox;
        unitBox->setEditable(true);
        unitBox->addItems({"doses", "sacs", "flacons", "boîtes", "pipettes", "kg", "sprays", "sachets", "unités"});
        unitBox->setCurrentText(source.unit);

        expiryEdit = new QDateEdit(source.expiry.isValid() ? source.expiry : QDate::currentDate().addYears(1));
        expiryEdit->setCalendarPopup(true);
        expiryEdit->setDisplayFormat("dd/MM/yyyy");

        form->addRow("Catégorie", categoryBox);
        form->addRow("Article", articleEdit);
        form->addRow("Lot", lotEdit);
        form->addRow("Quantité disponible", quantitySpin);
        form->addRow("Unité", unitBox);
        form->addRow("Seuil minimum", thresholdSpin);
        form->addRow("Date d'échéance", expiryEdit);
        layout->addLayout(form);

        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
        buttons->button(QDialogButtonBox::Save)->setText("Enregistrer");
        buttons->button(QDialogButtonBox::Save)->setStyleSheet(QStringLiteral("background: %1; color: white; border: none;").arg(kPrimary));
        buttons->button(QDialogButtonBox::Cancel)->setText("Annuler");
        layout->addWidget(buttons);

        connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
            if (articleEdit->text().trimmed().isEmpty() || lotEdit->text().trimmed().isEmpty() || unitBox->currentText().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Champs obligatoires", "Remplissez le nom de l'article, le lot et l'unité.");
                return;
            }
            accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    StockRecord result() const {
        StockRecord updated = record;
        updated.category = categoryBox->currentText();
        updated.article = prefixForCategory(updated.category) + articleEdit->text().trimmed();
        updated.lot = lotEdit->text().trimmed();
        updated.quantity = quantitySpin->value();
        updated.unit = unitBox->currentText().trimmed();
        updated.threshold = thresholdSpin->value();
        updated.expiry = expiryEdit->date();
        return updated;
    }

private:
    StockRecord record;
    QComboBox *categoryBox = nullptr;
    QLineEdit *articleEdit = nullptr;
    QLineEdit *lotEdit = nullptr;
    QSpinBox *quantitySpin = nullptr;
    QComboBox *unitBox = nullptr;
    QSpinBox *thresholdSpin = nullptr;
    QDateEdit *expiryEdit = nullptr;
};

QFrame *makeStatisticCard(const QString &title, const QString &value, const QString &subtitle, const QString &background) {
    QFrame *card = new QFrame;
    card->setStyleSheet(QStringLiteral("QFrame { background: %1; border: 1px solid #D8E9EC; border-radius: 9px; }").arg(background));
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(5);
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(QStringLiteral("color: %1; font-weight: 700; background: transparent; border: none;").arg(kPrimary));
    QLabel *valueLabel = new QLabel(value);
    valueLabel->setStyleSheet(QStringLiteral("font-size: 28px; font-weight: 800; color: %1; background: transparent; border: none;").arg(kPrimaryDark));
    QLabel *subtitleLabel = new QLabel(subtitle);
    subtitleLabel->setStyleSheet("color: #71878D; background: transparent; border: none;");
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    layout->addWidget(subtitleLabel);
    return card;
}
}

StockWidget::StockWidget(QWidget *parent) : QWidget(parent), tableModel(new QStandardItemModel(this)) {
    setObjectName("StockView");
    setStyleSheet(QStringLiteral(
                      "QWidget#StockView { background: #EAF4F6; color: %1; font-family: 'Segoe UI'; }"
                      "QLabel { color: %1; }"
                      "QLineEdit, QComboBox { background: white; color: %1; border: 1px solid #D0E3E7; border-radius: 6px; padding: 7px 9px; min-height: 22px; }"
                      "QLineEdit:focus, QComboBox:focus { border: 1px solid %2; }"
                      "QTableView { background: white; alternate-background-color: #F8FBFC; color: %1; border: 1px solid #D8E9EC; gridline-color: #E2EEF0; selection-background-color: #D6ECEF; selection-color: %1; outline: 0; }"
                      "QHeaderView::section { background: #397F8D; color: white; font-weight: 700; padding: 10px 8px; border: none; border-right: 1px solid #2E7280; }"
                      "QScrollBar:vertical { background: #EFF6F7; width: 10px; margin: 0; }"
                      "QScrollBar::handle:vertical { background: #B6D3D9; min-height: 25px; border-radius: 4px; }"
                      ).arg(kInk, kPrimary));

    loadDataFromDatabase();
    setupUi();
    refreshTable();
}

void StockWidget::loadDataFromDatabase() {
    items.clear();
    QSqlQuery query("SELECT ID_ARTICLE, ARTICLE, NOM_ARTICLE, LOT, QUANTITE_DISP, UNITS, SEUIL_MIN, DATE_CHEANCE FROM C##RAWAND.STOCK");

    while (query.next()) {
        StockRecord item;
        item.id = query.value(0).toInt();
        item.article = query.value(1).toString();
        item.category = categoryFromArticle(item.article);
        item.lot = query.value(3).toString();
        item.quantity = query.value(4).toInt();
        item.unit = query.value(5).toString();
        item.threshold = query.value(6).toInt();
        item.expiry = query.value(7).toDate();
        items.append(item);
    }
}

void StockWidget::setupUi() {
    QHBoxLayout *windowLayout = new QHBoxLayout(this);
    windowLayout->setContentsMargins(0, 0, 0, 0);
    windowLayout->setSpacing(0);

    QFrame *sidebar = new QFrame;
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet("QFrame#Sidebar { background: #FFFFFF; border-right: 1px solid #D8E9EC; }");
    QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(11, 18, 11, 15);
    sideLayout->setSpacing(10);

    QLabel *brand = new QLabel("🐾  ANIWORLD");
    brand->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 800; color: %1; padding: 2px 5px; background: transparent;").arg(kInk));
    QLabel *brandSub = new QLabel("T H E   O F F I C I A L   A P P L I C A T I O N");
    brandSub->setStyleSheet("font-size: 7px; color: #71878D; padding-left: 30px; background: transparent;");
    sideLayout->addWidget(brand);
    sideLayout->addWidget(brandSub);
    sideLayout->addSpacing(12);

    QFrame *careCard = new QFrame;
    careCard->setStyleSheet("QFrame { background: #8DCAD2; border: none; border-radius: 9px; }");
    QVBoxLayout *careLayout = new QVBoxLayout(careCard);
    careLayout->setContentsMargins(12, 11, 9, 11);
    QLabel *careTitle = new QLabel("◉  Centre de soins");
    careTitle->setStyleSheet("color: #225A64; font-weight: 700; background: transparent;");
    QLabel *careSub = new QLabel("     Espace de gestion");
    careSub->setStyleSheet("color: #346D77; background: transparent;");
    careLayout->addWidget(careTitle);
    careLayout->addWidget(careSub);
    sideLayout->addWidget(careCard);
    sideLayout->addSpacing(10);

    QLabel *navTitle = new QLabel("N A V I G A T I O N");
    navTitle->setStyleSheet("font-size: 9px; font-weight: 700; color: #526F77; padding: 4px 8px; background: transparent;");
    sideLayout->addWidget(navTitle);

    QPushButton *stockNav = makeButton("📦   Stock", "#D5EBEE", kPrimaryDark, "none");
    stockNav->setMinimumHeight(39);
    stockNav->setStyleSheet("QPushButton { text-align: left; background: #CFE6E9; color: #225A64; border: none; border-radius: 6px; padding: 9px 12px; font-weight: 700; } QPushButton:hover { background: #C4E1E5; }");
    stockNav->setEnabled(false);
    sideLayout->addWidget(stockNav);

    QPushButton *statsNav = makeButton("▦   Statistiques", "#FFFFFF", kInk, "1px solid #FFFFFF");
    statsNav->setStyleSheet("QPushButton { text-align: left; background: #FFFFFF; color: #536F76; border: none; border-radius: 6px; padding: 9px 12px; } QPushButton:hover { background: #EDF6F7; color: #225A64; }");
    sideLayout->addWidget(statsNav);

    // Les modules avancés sont affichés sous le tableau principal, et non dans la barre latérale.
    sideLayout->addStretch(1);

    QLabel *connectionLabel = new QLabel("Connecté · Oracle c##rawand");
    connectionLabel->setStyleSheet("color: #71878D; font-size: 10px; background: transparent; padding: 0 5px;");
    sideLayout->addWidget(connectionLabel);
    QPushButton *logoutButton = makeButton("⇥  Se déconnecter", "#EDF6F7", kPrimaryDark, "1px solid #BDD9DE");
    logoutButton->setMinimumHeight(36);
    sideLayout->addWidget(logoutButton);
    QLabel *sideFooter = new QLabel("Une attention à chaque patte.\nANIWORLD · Centre de soins");
    sideFooter->setStyleSheet("color: #71878D; font-size: 9px; padding: 8px 5px 0; background: transparent;");
    sideLayout->addWidget(sideFooter);
    windowLayout->addWidget(sidebar);

    QWidget *mainArea = new QWidget;
    mainArea->setStyleSheet("background: #EAF4F6;");
    QVBoxLayout *mainAreaLayout = new QVBoxLayout(mainArea);
    mainAreaLayout->setContentsMargins(0, 0, 0, 0);
    mainAreaLayout->setSpacing(0);

    QFrame *topBar = new QFrame;
    topBar->setObjectName("TopBar");
    topBar->setStyleSheet("QFrame#TopBar { background: #FFFFFF; border-bottom: 1px solid #D8E9EC; }");
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(26, 10, 24, 10);
    QLabel *breadcrumb = new QLabel("Gestion  /  Stock");
    breadcrumb->setStyleSheet("color: #71878D; background: transparent;");
    topLayout->addWidget(breadcrumb);
    topLayout->addStretch(1);
    QLabel *dateLabel = new QLabel(QLocale(QLocale::French, QLocale::France).toString(QDate::currentDate(), "dddd d MMMM yyyy"));
    dateLabel->setStyleSheet("color: #71878D; background: transparent; padding-right: 8px;");
    topLayout->addWidget(dateLabel);
    QLabel *demoPill = new QLabel("•  Oracle Connecté");
    demoPill->setStyleSheet("background: #DDF3D5; color: #2D6B22; border-radius: 6px; padding: 8px 12px; font-weight: bold;");
    topLayout->addWidget(demoPill);
    mainAreaLayout->addWidget(topBar);

    contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(24, 18, 24, 12);
    contentLayout->setSpacing(14);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    QVBoxLayout *titleTexts = new QVBoxLayout;
    titleTexts->setSpacing(2);
    QLabel *title = new QLabel("📦  Stock");
    title->setStyleSheet(QStringLiteral("font-size: 23px; font-weight: 800; color: %1; background: transparent;").arg(kPrimaryDark));
    QLabel *subtitle = new QLabel("Inventaire, traçabilité des lots et réapprovisionnement");
    subtitle->setStyleSheet("font-size: 11px; color: #71878D; background: transparent;");
    titleTexts->addWidget(title);
    titleTexts->addWidget(subtitle);
    titleLayout->addLayout(titleTexts);
    titleLayout->addStretch(1);

    QPushButton *statsButton = makeButton("▦  Statistiques", "#FFFFFF", kPrimaryDark, "1px solid #BDD9DE");
    QPushButton *editButton = makeButton("Modifier", "#FFFFFF", kPrimaryDark, "1px solid #BDD9DE");
    QPushButton *deleteButton = makeButton("Supprimer", "#FCEDEC", "#B94A48", "1px solid #E8C5C2");
    QPushButton *movementButton = makeButton("＋  Enregistrer un mouvement", kPrimary, "#FFFFFF");
    titleLayout->addWidget(statsButton);
    titleLayout->addWidget(editButton);
    titleLayout->addWidget(deleteButton);
    titleLayout->addWidget(movementButton);
    contentLayout->addLayout(titleLayout);

    QHBoxLayout *toolbar = new QHBoxLayout;
    toolbar->setSpacing(10);
    searchLineEdit = new QLineEdit;
    searchLineEdit->setPlaceholderText("⌕   Rechercher un article, un lot, un état...");
    searchLineEdit->setClearButtonEnabled(true);
    searchLineEdit->setMinimumHeight(37);

    categoryFilter = new QComboBox;
    categoryFilter->addItems({"Toutes les catégories", "Chiens", "Chats", "Oiseaux", "Poissons", "Rongeurs", "Matériel Clinique"});
    categoryFilter->setMinimumHeight(37);
    categoryFilter->setMinimumWidth(150);

    sortFilter = new QComboBox;
    sortFilter->addItem("Tri : par défaut", 0);
    sortFilter->addItem("Quantité : croissante", 1);
    sortFilter->addItem("Quantité : décroissante", 2);
    sortFilter->addItem("Échéance : plus proche", 3);
    sortFilter->addItem("Échéance : plus éloignée", 4);
    sortFilter->setMinimumHeight(37);
    sortFilter->setMinimumWidth(190);

    stateFilter = new QComboBox;
    stateFilter->addItems({"Tous les états", "Disponible", "Sous le seuil", "Échéance proche", "Périmé"});
    stateFilter->setMinimumHeight(37);
    stateFilter->setMinimumWidth(155);

    toolbar->addWidget(searchLineEdit, 1);
    toolbar->addWidget(categoryFilter);
    toolbar->addWidget(sortFilter);
    toolbar->addWidget(stateFilter);
    contentLayout->addLayout(toolbar);

    tableModel->setColumnCount(6);
    tableModel->setHorizontalHeaderLabels({"ARTICLE", "LOT", "DISPONIBLE", "SEUIL", "ÉCHÉANCE", "ÉTAT"});

    stockTable = new QTableView;
    stockTable->setModel(tableModel);
    stockTable->setAlternatingRowColors(true);
    stockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    stockTable->setSelectionMode(QAbstractItemView::SingleSelection);
    stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    stockTable->setShowGrid(true);
    stockTable->verticalHeader()->setVisible(false);
    stockTable->verticalHeader()->setDefaultSectionSize(38);
    stockTable->horizontalHeader()->setMinimumHeight(39);
    stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    stockTable->horizontalHeader()->setStretchLastSection(true);
    stockTable->setColumnWidth(0, 475);
    stockTable->setColumnWidth(1, 125);
    stockTable->setColumnWidth(2, 140);
    stockTable->setColumnWidth(3, 72);
    stockTable->setColumnWidth(4, 130);
    stockTable->setMinimumHeight(300);
    contentLayout->addWidget(stockTable, 1);

    // Accès rapide aux modules avancés, placés sous le tableau du stock.
    QFrame *advancedPanel = new QFrame;
    advancedPanel->setObjectName("AdvancedModulesPanel");
    advancedPanel->setStyleSheet("QFrame#AdvancedModulesPanel { background: #FFFFFF; border: 1px solid #D8E9EC; border-radius: 9px; }");
    QVBoxLayout *advancedPanelLayout = new QVBoxLayout(advancedPanel);
    advancedPanelLayout->setContentsMargins(14, 10, 14, 12);
    advancedPanelLayout->setSpacing(8);

    QHBoxLayout *advancedHeader = new QHBoxLayout;
    QLabel *advancedTitle = new QLabel("Gestion avancée");
    advancedTitle->setStyleSheet(QStringLiteral("font-size: 13px; font-weight: 800; color: %1; background: transparent;").arg(kPrimaryDark));
    QLabel *advancedSubtitle = new QLabel("Factures fournisseurs, alertes et commandes");
    advancedSubtitle->setStyleSheet("font-size: 10px; color: #71878D; background: transparent;");
    advancedHeader->addWidget(advancedTitle);
    advancedHeader->addStretch(1);
    advancedHeader->addWidget(advancedSubtitle);
    advancedPanelLayout->addLayout(advancedHeader);

    QHBoxLayout *advancedButtons = new QHBoxLayout;
    advancedButtons->setSpacing(10);

    QPushButton *purchasesNav = makeButton("▤   Factures d'achat", "#F8FCFC", kPrimaryDark, "1px solid #D8E9EC");
    purchasesNav->setMinimumHeight(42);
    purchasesNav->setStyleSheet("QPushButton { text-align: left; background: #F8FCFC; color: #225A64; border: 1px solid #D8E9EC; border-radius: 7px; padding: 10px 14px; font-weight: 600; } QPushButton:hover { background: #EAF5F6; border-color: #9BC6CE; }");

    alertsNavButton = makeButton("⚠   Alertes de stock", "#F8FCFC", kPrimaryDark, "1px solid #D8E9EC");
    alertsNavButton->setMinimumHeight(42);

    QPushButton *replenishNav = makeButton("↻   Réapprovisionnement", "#F8FCFC", kPrimaryDark, "1px solid #D8E9EC");
    replenishNav->setMinimumHeight(42);
    replenishNav->setStyleSheet("QPushButton { text-align: left; background: #F8FCFC; color: #225A64; border: 1px solid #D8E9EC; border-radius: 7px; padding: 10px 14px; font-weight: 600; } QPushButton:hover { background: #EAF5F6; border-color: #9BC6CE; }");

    advancedButtons->addWidget(purchasesNav, 1);
    advancedButtons->addWidget(alertsNavButton, 1);
    advancedButtons->addWidget(replenishNav, 1);
    advancedPanelLayout->addLayout(advancedButtons);
    contentLayout->addWidget(advancedPanel);

    QFrame *expiryPanel = new QFrame;
    expiryPanel->setStyleSheet("QFrame { background: #FFFFFF; border: 1px solid #D8E9EC; border-radius: 9px; }");
    QHBoxLayout *expiryLayout = new QHBoxLayout(expiryPanel);
    expiryLayout->setContentsMargins(16, 12, 16, 12);
    expiryLayout->setSpacing(12);
    QVBoxLayout *expiryTextLayout = new QVBoxLayout;
    expiryTextLayout->setSpacing(4);
    QLabel *expiryTitle = new QLabel("Gestion des articles périmés");
    expiryTitle->setStyleSheet("color: #B94A48; font-weight: 700; background: transparent;");
    QLabel *expirySub = new QLabel("Consultez les lots périmés ou à échéance proche, puis supprimez-les ou remplacez-les par un nouveau lot.");
    expirySub->setWordWrap(true);
    expirySub->setStyleSheet("color: #71878D; background: transparent;");
    expiryTextLayout->addWidget(expiryTitle);
    expiryTextLayout->addWidget(expirySub);
    expiryLayout->addLayout(expiryTextLayout, 1);
    QPushButton *expiryButton = makeButton("Afficher les articles périmés", kPrimary, "#FFFFFF");
    expiryButton->setMinimumWidth(245);
    expiryLayout->addWidget(expiryButton);
    contentLayout->addWidget(expiryPanel);

    QHBoxLayout *bottomInfo = new QHBoxLayout;
    footerLabel = new QLabel;
    footerLabel->setStyleSheet("color: #71878D; font-size: 10px; background: transparent;");
    QLabel *dataNotice = new QLabel("Base de données Oracle c##rawand synchronisée");
    dataNotice->setStyleSheet("color: #83979C; font-size: 10px; background: transparent;");
    bottomInfo->addWidget(footerLabel);
    bottomInfo->addStretch(1);
    bottomInfo->addWidget(dataNotice);
    contentLayout->addLayout(bottomInfo);
    mainAreaLayout->addWidget(contentWidget, 1);
    windowLayout->addWidget(mainArea, 1);

    connect(statsButton, &QPushButton::clicked, this, &StockWidget::showStatistics);
    connect(statsNav, &QPushButton::clicked, this, &StockWidget::showStatistics);
    connect(editButton, &QPushButton::clicked, this, &StockWidget::editSelectedItem);
    connect(deleteButton, &QPushButton::clicked, this, &StockWidget::deleteSelectedItem);
    connect(movementButton, &QPushButton::clicked, this, &StockWidget::openMovementDialog);
    connect(expiryButton, &QPushButton::clicked, this, &StockWidget::manageExpiryItems);
    connect(purchasesNav, &QPushButton::clicked, this, &StockWidget::openPurchasesModule);
    connect(alertsNavButton, &QPushButton::clicked, this, &StockWidget::openAlertsModule);
    connect(replenishNav, &QPushButton::clicked, this, &StockWidget::openReplenishmentModule);
    connect(logoutButton, &QPushButton::clicked, this, [this]() { emit logoutRequested(); });
    connect(searchLineEdit, &QLineEdit::textChanged, this, &StockWidget::refreshTable);
    connect(categoryFilter, &QComboBox::currentTextChanged, this, &StockWidget::refreshTable);
    connect(sortFilter, qOverload<int>(&QComboBox::currentIndexChanged), this, &StockWidget::refreshTable);
    connect(stateFilter, &QComboBox::currentTextChanged, this, &StockWidget::refreshTable);
}

void StockWidget::openPurchasesModule() {
    if (InventoryFeatures::showPurchases(this, items)) {
        loadDataFromDatabase();
        refreshTable();
    }
}

void StockWidget::openAlertsModule() {
    InventoryFeatures::showAlerts(this, items);
}

void StockWidget::openReplenishmentModule() {
    InventoryFeatures::showSmartReplenishment(this, items);
}

QString StockWidget::statusFor(const StockRecord &item) const {
    const QDate today = QDate::currentDate();
    if (item.expiry.isValid() && item.expiry < today) {
        return "Périmé";
    }
    if (item.expiry.isValid() && item.expiry <= today.addDays(30)) {
        return "Échéance proche";
    }
    if (item.quantity < item.threshold) {
        return "Sous le seuil";
    }
    return "Disponible";
}

int StockWidget::countBelowThreshold() const {
    int count = 0;
    for (const StockRecord &item : items) {
        if (item.quantity < item.threshold) ++count;
    }
    return count;
}

int StockWidget::countExpiryAlerts() const {
    int count = 0;
    const QDate limit = QDate::currentDate().addDays(30);
    for (const StockRecord &item : items) {
        if (item.expiry.isValid() && item.expiry <= limit) ++count;
    }
    return count;
}

void StockWidget::refreshTable() {
    if (!stockTable || !categoryFilter || !searchLineEdit || !sortFilter || !stateFilter) return;

    QVector<int> visible;
    const QString query = searchLineEdit->text().trimmed();
    const QString category = categoryFilter->currentText();
    const QString state = stateFilter->currentText();

    for (int i = 0; i < items.size(); ++i) {
        const StockRecord &item = items.at(i);
        const QString itemState = statusFor(item);
        if (category != "Toutes les catégories" && item.category != category) continue;
        if (state != "Tous les états" && itemState != state) continue;
        const QString haystack = item.article + " " + item.lot + " " + itemState + " " + item.expiry.toString("dd/MM/yyyy");
        if (!query.isEmpty() && !haystack.contains(query, Qt::CaseInsensitive)) continue;
        visible.append(i);
    }

    const int sortMode = sortFilter->currentData().toInt();
    std::stable_sort(visible.begin(), visible.end(), [this, sortMode](int left, int right) {
        const StockRecord &a = items.at(left);
        const StockRecord &b = items.at(right);
        switch (sortMode) {
        case 1: return a.quantity < b.quantity;
        case 2: return a.quantity > b.quantity;
        case 3: return a.expiry < b.expiry;
        case 4: return a.expiry > b.expiry;
        default: return left < right;
        }
    });

    tableModel->removeRows(0, tableModel->rowCount());
    for (const int sourceRow : visible) {
        const StockRecord &record = items.at(sourceRow);
        const QString stateText = statusFor(record);
        QList<QStandardItem *> row;
        QStringList values = {record.article, record.lot,
                              QString::number(record.quantity) + " " + record.unit,
                              QString::number(record.threshold),
                              record.expiry.toString("dd/MM/yyyy"), stateText};
        for (int column = 0; column < values.size(); ++column) {
            QStandardItem *cell = new QStandardItem(values.at(column));
            cell->setEditable(false);
            if (column == 0) cell->setData(sourceRow, Qt::UserRole);
            if (column == 5) {
                if (stateText == "Périmé") cell->setForeground(QColor("#B94A48"));
                else if (stateText == "Sous le seuil") cell->setForeground(QColor("#D47B25"));
                else if (stateText == "Échéance proche") cell->setForeground(QColor("#C87920"));
                else cell->setForeground(QColor(kPrimary));
            } else {
                cell->setForeground(QColor(kInk));
            }
            row.append(cell);
        }
        tableModel->appendRow(row);
    }
    updateFooter();
    updateAlertNavigation();
}

void StockWidget::updateFooter() {
    if (!footerLabel || !tableModel) return;
    footerLabel->setText(QString("%1 / %2 articles").arg(tableModel->rowCount()).arg(items.size()));
}

void StockWidget::updateAlertNavigation() {
    if (!alertsNavButton) return;
    int activeAlertCount = countExpiryAlerts();
    for (const StockRecord &record : items) {
        const int earlyWarning = qMax(1, record.threshold / 5);
        if (record.quantity <= record.threshold + earlyWarning) ++activeAlertCount;
    }
    alertsNavButton->setText(QString("⚠   Alertes de stock (%1)").arg(activeAlertCount));
    alertsNavButton->setStyleSheet(QString(
        "QPushButton { text-align:left; background:%1; color:%2; border:1px solid %3; border-radius:7px; padding:10px 14px; font-weight:%4; } "
        "QPushButton:hover { background:%5; }")
        .arg(activeAlertCount > 0 ? QString("#FBEAEA") : QString("#F8FCFC"),
             activeAlertCount > 0 ? QString("#B94A48") : QString("#225A64"),
             activeAlertCount > 0 ? QString("#F0CCCA") : QString("#D8E9EC"),
             activeAlertCount > 0 ? QString("700") : QString("600"),
             activeAlertCount > 0 ? QString("#F9DEDC") : QString("#EAF5F6")));
}

QVector<int> StockWidget::selectedSourceRows() const {
    QVector<int> result;
    if (!stockTable || !stockTable->selectionModel()) return result;
    const QModelIndexList selected = stockTable->selectionModel()->selectedRows();
    for (const QModelIndex &index : selected) {
        const QStandardItem *item = tableModel->item(index.row(), 0);
        if (item) result.append(item->data(Qt::UserRole).toInt());
    }
    return result;
}

void StockWidget::openMovementDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Enregistrer un mouvement de stock");
    dialog.setMinimumWidth(500);
    dialog.setStyleSheet(QStringLiteral(
                             "QDialog { background: #EAF4F6; color: %1; font-family: 'Segoe UI'; }"
                             "QComboBox, QSpinBox { background: white; border: 1px solid #C9E0E5; border-radius: 5px; padding: 6px; }"
                             "QPushButton { border-radius: 5px; padding: 7px 10px; }"
                             ).arg(kInk));
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QFormLayout *form = new QFormLayout;

    QComboBox *itemBox = new QComboBox;
    for (int i = 0; i < items.size(); ++i) {
        itemBox->addItem(items.at(i).article + " — " + items.at(i).lot, i);
    }

    QWidget *articleSelector = new QWidget(&dialog);
    QHBoxLayout *articleSelectorLayout = new QHBoxLayout(articleSelector);
    articleSelectorLayout->setContentsMargins(0, 0, 0, 0);
    QPushButton *addArticleButton = new QPushButton("＋ Ajouter un nouvel article", articleSelector);
    addArticleButton->setCursor(Qt::PointingHandCursor);
    addArticleButton->setStyleSheet(QStringLiteral(
                                        "QPushButton { background: white; color: %1; border: 1px solid #9BC6CE; border-radius: 5px; padding: 7px 10px; }"
                                        "QPushButton:hover { background: #DDEFF2; }").arg(kPrimaryDark));
    articleSelectorLayout->addWidget(itemBox, 1);
    articleSelectorLayout->addWidget(addArticleButton);

    QComboBox *movementType = new QComboBox;
    movementType->addItem("Entrée — ajouter au stock", 0);
    movementType->addItem("Sortie — retirer du stock", 1);
    QSpinBox *quantity = new QSpinBox;
    quantity->setRange(1, 999999);
    quantity->setValue(1);
    form->addRow("Article / lot", articleSelector);
    form->addRow("Type de mouvement", movementType);
    form->addRow("Quantité", quantity);
    layout->addLayout(form);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText("Valider le mouvement");
    buttons->button(QDialogButtonBox::Save)->setStyleSheet(QStringLiteral("background: %1; color: white; border: none; padding: 7px 12px;").arg(kPrimary));
    buttons->button(QDialogButtonBox::Cancel)->setText("Annuler");
    layout->addWidget(buttons);

    connect(addArticleButton, &QPushButton::clicked, &dialog, [this, &dialog, itemBox]() {
        StockRecord emptyRecord;
        emptyRecord.category = "Matériel Clinique";
        emptyRecord.quantity = 0;
        emptyRecord.unit = "unités";
        emptyRecord.threshold = 1;
        emptyRecord.expiry = QDate::currentDate().addYears(1);
        ProductEditDialog addDialog(emptyRecord, "Ajouter un nouvel article", &dialog);
        if (addDialog.exec() != QDialog::Accepted) return;

        const StockRecord newRecord = addDialog.result();

        QSqlQuery queryMax;
        queryMax.exec("SELECT NVL(MAX(ID_ARTICLE), 0) + 1 FROM C##RAWAND.STOCK");
        int nextId = 1;
        if (queryMax.next()) {
            nextId = queryMax.value(0).toInt();
        }

        // Requête d'insertion sans ID_LOCAL pour éviter toute contrainte d'intégrité
        QSqlQuery q;
        q.prepare("INSERT INTO C##RAWAND.STOCK (ID_ARTICLE, ARTICLE, NOM_ARTICLE, LOT, QUANTITE_DISP, UNITS, SEUIL_MIN, DATE_CHEANCE) "
                  "VALUES (:id, :art, :nom, :lot, :qty, :unit, :seuil, TO_DATE(:ech, 'YYYY-MM-DD'))");
        q.bindValue(":id", nextId);
        q.bindValue(":art", newRecord.article);
        q.bindValue(":nom", articleName(newRecord.article));
        q.bindValue(":lot", newRecord.lot);
        q.bindValue(":qty", newRecord.quantity);
        q.bindValue(":unit", newRecord.unit);
        q.bindValue(":seuil", newRecord.threshold);
        q.bindValue(":ech", newRecord.expiry.toString("yyyy-MM-dd"));

        if(!q.exec()) {
            QString err = q.lastError().text();
            qDebug() << "Erreur ajout STOCK Oracle:" << err;
            QMessageBox::critical(&dialog, "Erreur Oracle", "Échec de l'insertion :\n" + err);
        } else {
            QSqlQuery commitQ;
            commitQ.exec("COMMIT");
            QMessageBox::information(&dialog, "Succès", "Article ajouté avec succès dans Oracle !");
        }

        loadDataFromDatabase();
        itemBox->clear();
        for (int i = 0; i < items.size(); ++i) {
            itemBox->addItem(items.at(i).article + " — " + items.at(i).lot, i);
        }
        itemBox->setCurrentIndex(itemBox->count() - 1);
        refreshTable();
    });

    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        const int sourceRow = itemBox->currentData().toInt();
        const int amount = quantity->value();
        const bool isOutput = movementType->currentData().toInt() == 1;
        if (sourceRow < 0 || sourceRow >= items.size()) {
            QMessageBox::warning(&dialog, "Article manquant", "Ajoutez ou sélectionnez un article avant de valider le mouvement.");
            return;
        }
        if (isOutput && amount > items.at(sourceRow).quantity) {
            QMessageBox::warning(&dialog, "Stock insuffisant", "La quantité de sortie dépasse la quantité disponible.");
            return;
        }

        int newQty = items.at(sourceRow).quantity;
        if (isOutput) newQty -= amount;
        else newQty += amount;

        QSqlQuery q;
        q.prepare("UPDATE C##RAWAND.STOCK SET QUANTITE_DISP = :qty WHERE ID_ARTICLE = :id");
        q.bindValue(":qty", newQty);
        q.bindValue(":id", items.at(sourceRow).id);

        if(!q.exec()) {
            QString err = q.lastError().text();
            qDebug() << "Erreur mvt STOCK Oracle:" << err;
            QMessageBox::critical(&dialog, "Erreur Oracle", "Échec de la mise à jour :\n" + err);
            return;
        }
        QSqlQuery commitQ;
        if (!commitQ.exec("COMMIT")) {
            QMessageBox::critical(&dialog, "Erreur Oracle", "Le mouvement n'a pas pu être validé :\n" + commitQ.lastError().text());
            return;
        }

        // Journalisation locale gérée par Qt : aucun tableau Oracle supplémentaire n'est requis.
        const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(appDataDir);
        QSettings localHistory(appDataDir + QStringLiteral("/aniworld_features.ini"), QSettings::IniFormat);
        QVector<QVariantMap> movementRows;
        const int movementCount = localHistory.beginReadArray("stockMovements");
        for (int i = 0; i < movementCount; ++i) {
            localHistory.setArrayIndex(i);
            QVariantMap row;
            row.insert("articleId", localHistory.value("articleId"));
            row.insert("type", localHistory.value("type"));
            row.insert("quantity", localHistory.value("quantity"));
            row.insert("date", localHistory.value("date"));
            movementRows.append(row);
        }
        localHistory.endArray();
        QVariantMap newMovement;
        newMovement.insert("articleId", items.at(sourceRow).id);
        newMovement.insert("type", isOutput ? QStringLiteral("SORTIE") : QStringLiteral("ENTREE"));
        newMovement.insert("quantity", amount);
        newMovement.insert("date", QDate::currentDate().toString(Qt::ISODate));
        movementRows.append(newMovement);
        localHistory.beginWriteArray("stockMovements");
        for (int i = 0; i < movementRows.size(); ++i) {
            localHistory.setArrayIndex(i);
            const QVariantMap row = movementRows.at(i);
            for (auto it = row.cbegin(); it != row.cend(); ++it) localHistory.setValue(it.key(), it.value());
        }
        localHistory.endArray();
        localHistory.sync();

        loadDataFromDatabase();
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) refreshTable();
}

void StockWidget::editSelectedItem() {
    const QVector<int> selected = selectedSourceRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Modifier un article", "Sélectionnez d'abord une ligne du tableau.");
        return;
    }
    if (selected.size() != 1) {
        QMessageBox::information(this, "Modifier un article", "Sélectionnez un seul article à modifier.");
        return;
    }
    const int index = selected.first();
    if (index < 0 || index >= items.size()) return;

    ProductEditDialog dialog(items.at(index), "Modifier un article", this);
    if (dialog.exec() == QDialog::Accepted) {
        StockRecord updated = dialog.result();

        QSqlQuery q;
        q.prepare("UPDATE C##RAWAND.STOCK SET ARTICLE=:art, NOM_ARTICLE=:nom, LOT=:lot, QUANTITE_DISP=:qty, "
                  "UNITS=:unit, SEUIL_MIN=:seuil, DATE_CHEANCE=TO_DATE(:ech, 'YYYY-MM-DD') WHERE ID_ARTICLE=:id");
        q.bindValue(":art", updated.article);
        q.bindValue(":nom", articleName(updated.article));
        q.bindValue(":lot", updated.lot);
        q.bindValue(":qty", updated.quantity);
        q.bindValue(":unit", updated.unit);
        q.bindValue(":seuil", updated.threshold);
        q.bindValue(":ech", updated.expiry.toString("yyyy-MM-dd"));
        q.bindValue(":id", updated.id);

        if(!q.exec()) {
            QString err = q.lastError().text();
            QMessageBox::critical(this, "Erreur Oracle", "Échec de la modification :\n" + err);
        } else {
            QSqlQuery commitQ;
            commitQ.exec("COMMIT");
        }

        loadDataFromDatabase();
        refreshTable();
    }
}

void StockWidget::deleteSelectedItem() {
    QVector<int> selected = selectedSourceRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Supprimer un article", "Sélectionnez d'abord une ligne du tableau.");
        return;
    }
    const auto answer = QMessageBox::question(this, "Confirmer la suppression",
                                              selected.size() == 1 ? "Supprimer cet article du stock ?" : "Supprimer les articles sélectionnés ?",
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    std::sort(selected.begin(), selected.end(), std::greater<int>());
    for (const int index : selected) {
        if (index >= 0 && index < items.size()) {
            QSqlQuery q;
            q.prepare("DELETE FROM C##RAWAND.STOCK WHERE ID_ARTICLE = :id");
            q.bindValue(":id", items.at(index).id);
            if(q.exec()) {
                QSqlQuery commitQ;
                commitQ.exec("COMMIT");
            } else {
                QString err = q.lastError().text();
                QMessageBox::critical(this, "Erreur Oracle", "Échec de la suppression :\n" + err);
            }
        }
    }
    loadDataFromDatabase();
    refreshTable();
}

void StockWidget::showStatistics() {
    QDialog dialog(this);
    dialog.setWindowTitle("Statistiques du stock");
    dialog.setMinimumWidth(780);
    dialog.setStyleSheet("QDialog { background: #EAF4F6; font-family: 'Segoe UI'; }");
    QVBoxLayout *root = new QVBoxLayout(&dialog);
    QLabel *title = new QLabel("Statistiques du stock");
    title->setStyleSheet(QStringLiteral("font-size: 22px; color: %1; font-weight: 800; background: transparent;").arg(kPrimaryDark));
    QLabel *subtitle = new QLabel("Vue d'ensemble des articles et des alertes de stock.");
    subtitle->setStyleSheet("color: #71878D; background: transparent;");
    root->addWidget(title);
    root->addWidget(subtitle);

    QHBoxLayout *cards = new QHBoxLayout;
    cards->setSpacing(12);
    cards->addWidget(makeStatisticCard("Articles référencés", QString::number(items.size()), "Produits actifs", "#E4F1F3"));
    cards->addWidget(makeStatisticCard("Sous le seuil", QString::number(countBelowThreshold()), "À réapprovisionner", "#FFF3E2"));
    cards->addWidget(makeStatisticCard("Lots à contrôler", QString::number(countExpiryAlerts()), "Échéances proches ou dépassées", "#FBEAEA"));
    root->addLayout(cards);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    buttons->button(QDialogButtonBox::Close)->setText("Fermer");
    root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    dialog.exec();
}

void StockWidget::manageExpiryItems() {
    QDialog dialog(this);
    dialog.setWindowTitle("Gestion des articles périmés");
    dialog.resize(850, 490);
    dialog.setStyleSheet(QStringLiteral(
                             "QDialog { background: #EAF4F6; color: %1; font-family: 'Segoe UI'; }"
                             "QTableWidget { background: white; alternate-background-color: #F8FBFC; color: %1; border: 1px solid #D8E9EC; gridline-color: #E2EEF0; selection-background-color: #D6ECEF; }"
                             "QHeaderView::section { background: #397F8D; color: white; font-weight: 700; padding: 8px; border: none; }"
                             ).arg(kInk));
    QVBoxLayout *root = new QVBoxLayout(&dialog);
    QLabel *title = new QLabel("Lots périmés ou à échéance proche");
    title->setStyleSheet(QStringLiteral("font-size: 19px; font-weight: 800; color: %1; background: transparent;").arg(kPrimaryDark));
    QLabel *instruction = new QLabel("Retirez les lots périmés ou à échéance proche du stock disponible sans effacer leur traçabilité. Pour un remplacement, l'ancien lot sera mis à zéro et un nouveau lot distinct sera créé.");
    instruction->setWordWrap(true);
    instruction->setStyleSheet("color: #71878D; background: transparent;");
    root->addWidget(title);
    root->addWidget(instruction);

    QTableWidget *table = new QTableWidget;
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"ARTICLE", "LOT", "ÉCHÉANCE", "ÉTAT"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    root->addWidget(table, 1);

    auto refillTable = [&]() {
        table->setRowCount(0);
        const QDate limit = QDate::currentDate().addDays(30);
        int count = 0;
        for (int i = 0; i < items.size(); ++i) {
            const StockRecord &item = items.at(i);
            if (!item.expiry.isValid() || item.expiry > limit) continue;
            const int row = table->rowCount();
            table->insertRow(row);
            QStringList values = {item.article, item.lot, item.expiry.toString("dd/MM/yyyy"), statusFor(item)};
            for (int col = 0; col < values.size(); ++col) {
                QTableWidgetItem *cell = new QTableWidgetItem(values.at(col));
                if (col == 0) cell->setData(Qt::UserRole, i);
                if (col == 3) cell->setForeground(QColor(statusFor(item) == "Périmé" ? "#B94A48" : "#C87920"));
                table->setItem(row, col, cell);
            }
            ++count;
        }
        if (count == 0) {
            table->setRowCount(1);
            QTableWidgetItem *empty = new QTableWidgetItem("Aucun article périmé ou à échéance proche.");
            empty->setTextAlignment(Qt::AlignCenter);
            empty->setFlags(empty->flags() & ~Qt::ItemIsSelectable);
            table->setItem(0, 0, empty);
            table->setSpan(0, 0, 1, 4);
        }
    };
    refillTable();

    QHBoxLayout *buttonsRow = new QHBoxLayout;
    buttonsRow->addStretch(1);
    QPushButton *deleteButton = makeButton("Retirer du stock disponible", "#FCEDEC", "#B94A48", "1px solid #E8C5C2");
    QPushButton *replaceButton = makeButton("Remplacer par un nouveau lot", kPrimary, "#FFFFFF");
    QPushButton *closeButton = makeButton("Fermer", "#FFFFFF", kInk, "1px solid #BDD9DE");
    buttonsRow->addWidget(deleteButton);
    buttonsRow->addWidget(replaceButton);
    buttonsRow->addWidget(closeButton);
    root->addLayout(buttonsRow);

    connect(deleteButton, &QPushButton::clicked, &dialog, [&]() {
        QVector<int> sourceRows;
        const QModelIndexList selectedRows = table->selectionModel()->selectedRows();
        for (const QModelIndex &modelIndex : selectedRows) {
            QTableWidgetItem *firstCell = table->item(modelIndex.row(), 0);
            if (firstCell && firstCell->data(Qt::UserRole).isValid()) {
                const int sourceRow = firstCell->data(Qt::UserRole).toInt();
                if (!sourceRows.contains(sourceRow)) sourceRows.append(sourceRow);
            }
        }
        if (sourceRows.isEmpty()) {
            QMessageBox::information(&dialog, "Aucune sélection", "Sélectionnez un lot dans la liste.");
            return;
        }
        if (QMessageBox::question(&dialog, "Confirmer le retrait",
                                  "Mettre à zéro la quantité disponible des lots sélectionnés ? Les enregistrements seront conservés pour garder la traçabilité.",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;

        QSqlDatabase db = QSqlDatabase::database();
        if (!db.transaction()) {
            QMessageBox::critical(&dialog, "Erreur Oracle", "Impossible de démarrer la transaction :\n" + db.lastError().text());
            return;
        }
        bool failed = false;
        QString error;
        for (const int index : sourceRows) {
            if (index < 0 || index >= items.size()) continue;
            const StockRecord item = items.at(index);
            QSqlQuery q;
            q.prepare("UPDATE C##RAWAND.STOCK SET QUANTITE_DISP=0 WHERE ID_ARTICLE=:id");
            q.bindValue(":id", item.id);
            if (!q.exec()) { failed = true; error = q.lastError().text(); break; }
            if (item.quantity > 0) appendLocalStockMovement(item.id, QStringLiteral("PERTE_PEREMPTION"), item.quantity, item.lot);
        }
        if (failed) {
            db.rollback();
            QMessageBox::critical(&dialog, "Erreur Oracle", "Le retrait n'a pas été appliqué :\n" + error);
            return;
        }
        if (!db.commit()) {
            db.rollback();
            QMessageBox::critical(&dialog, "Erreur Oracle", "Impossible de valider le retrait :\n" + db.lastError().text());
            return;
        }
        loadDataFromDatabase();
        refreshTable();
        refillTable();
    });

    connect(replaceButton, &QPushButton::clicked, &dialog, [&]() {
        const QModelIndexList selectedRows = table->selectionModel()->selectedRows();
        QVector<int> sourceRows;
        for (const QModelIndex &modelIndex : selectedRows) {
            QTableWidgetItem *firstCell = table->item(modelIndex.row(), 0);
            if (firstCell && firstCell->data(Qt::UserRole).isValid()) {
                const int sourceRow = firstCell->data(Qt::UserRole).toInt();
                if (!sourceRows.contains(sourceRow)) sourceRows.append(sourceRow);
            }
        }
        if (sourceRows.size() != 1) {
            QMessageBox::information(&dialog, "Choisir un lot", "Sélectionnez exactement un lot à remplacer.");
            return;
        }
        const int index = sourceRows.first();
        if (index < 0 || index >= items.size()) return;
        StockRecord proposed = items.at(index);
        proposed.lot = "NOUVEAU-LOT";
        proposed.quantity = qMax(1, proposed.threshold);
        proposed.expiry = QDate::currentDate().addYears(1);
        ProductEditDialog editDialog(proposed, "Créer le nouveau lot", &dialog);

        if (editDialog.exec() == QDialog::Accepted) {
            StockRecord updated = editDialog.result();
            QSqlDatabase db = QSqlDatabase::database();
            if (!db.transaction()) {
                QMessageBox::critical(&dialog, "Erreur Oracle", "Impossible de démarrer la transaction :\n" + db.lastError().text());
                return;
            }

            QSqlQuery retireOld;
            retireOld.prepare("UPDATE C##RAWAND.STOCK SET QUANTITE_DISP=0 WHERE ID_ARTICLE=:id");
            retireOld.bindValue(":id", items.at(index).id);
            if (!retireOld.exec()) {
                db.rollback();
                QMessageBox::critical(&dialog, "Erreur Oracle", "Impossible de retirer l'ancien lot :\n" + retireOld.lastError().text());
                return;
            }

            QSqlQuery queryMax;
            if (!queryMax.exec("SELECT NVL(MAX(ID_ARTICLE),0)+1 FROM C##RAWAND.STOCK") || !queryMax.next()) {
                db.rollback();
                QMessageBox::critical(&dialog, "Erreur Oracle", "Impossible de générer l'identifiant du nouveau lot :\n" + queryMax.lastError().text());
                return;
            }
            const int newId = queryMax.value(0).toInt();
            QSqlQuery insertNew;
            insertNew.prepare("INSERT INTO C##RAWAND.STOCK (ID_ARTICLE, ARTICLE, NOM_ARTICLE, LOT, QUANTITE_DISP, UNITS, SEUIL_MIN, DATE_CHEANCE) "
                              "VALUES (:id,:art,:nom,:lot,:qty,:unit,:seuil,TO_DATE(:ech,'YYYY-MM-DD'))");
            insertNew.bindValue(":id", newId);
            insertNew.bindValue(":art", updated.article);
            insertNew.bindValue(":nom", articleName(updated.article));
            insertNew.bindValue(":lot", updated.lot);
            insertNew.bindValue(":qty", updated.quantity);
            insertNew.bindValue(":unit", updated.unit);
            insertNew.bindValue(":seuil", updated.threshold);
            insertNew.bindValue(":ech", updated.expiry.toString("yyyy-MM-dd"));
            if (!insertNew.exec()) {
                db.rollback();
                QMessageBox::critical(&dialog, "Erreur Oracle", "Impossible de créer le nouveau lot :\n" + insertNew.lastError().text());
                return;
            }

            const bool historySaved = true;
            if (items.at(index).quantity > 0)
                appendLocalStockMovement(items.at(index).id, QStringLiteral("PERTE_PEREMPTION"), items.at(index).quantity, items.at(index).lot);
            if (!db.commit()) {
                db.rollback();
                QMessageBox::critical(&dialog, "Erreur Oracle", "Impossible de valider le remplacement :\n" + db.lastError().text());
                return;
            }

            loadDataFromDatabase();
            refreshTable();
            refillTable();
            QMessageBox::information(&dialog, "Lot remplacé", historySaved
                ? "L'ancien lot a été retiré du stock disponible et un nouveau lot a été créé."
                : "Le nouveau lot a été créé et l'ancien retiré du stock.");
        }
    });
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}
