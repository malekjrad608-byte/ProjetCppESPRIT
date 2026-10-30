#include "../database/oracleconnection.h"
#include "../database/oraclecollection.h"
#include "../widgets/animatedbutton.h"
#include <limits>
#include "../ui/design.h"
#include "../ui/invoice.h"
#include <QPainter>
#include <QFileDialog>
#include "stockwidget.h"

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
#include <algorithm>
#include <functional>

namespace {
const QString kPrimary = QStringLiteral("#397F8D");
const QString kPrimaryDark = QStringLiteral("#225A64");
const QString kInk = QStringLiteral("#263B42");
const QString kMuted = QStringLiteral("#71878D");
const QString kBorder = QStringLiteral("#D8E9EC");

QString prefixForCategory(const QString &category) {
    if (category == "Chiens") return "[CHIEN] ";
    if (category == "Chats") return "[CHAT] ";
    if (category == "Oiseaux") return "[OISEAU] ";
    if (category == "Poissons") return "[POISSON] ";
    if (category == "Rongeurs") return "[HAMSTER] ";
    if (category == "Matériel Clinique") return "[MATÉRIEL] ";
    return QString();
}

QString articleName(const QString &article) {
    const int endPrefix = article.indexOf("]");
    if (article.startsWith("[") && endPrefix >= 0) {
        return article.mid(endPrefix + 1).trimmed();
    }
    return article.trimmed();
}

QString searchable(QString text) {
    text=text.normalized(QString::NormalizationForm_D).toCaseFolded();
    QString result;
    for(const QChar c:text) if(c.category()!=QChar::Mark_NonSpacing) result+=c;
    return result;
}

QPushButton *makeButton(const QString &text, const QString &background,
                        const QString &foreground, const QString &border = QStringLiteral("none")) {
    Q_UNUSED(foreground); Q_UNUSED(border);
    auto *button = Design::button(text, {}, background == kPrimary ? "primary" : background == "#FCEDEC" ? "danger" : nullptr);
    return button;
}

QDialogButtonBox *actionBox(QDialogButtonBox::StandardButtons actions) {
    auto *box=new QDialogButtonBox(actions);
    for(auto *button:box->buttons()) {
        const auto standard=box->standardButton(button);
        button->setProperty("role",standard==QDialogButtonBox::Save || standard==QDialogButtonBox::Close?"primary":"normal");
    }
    return box;
}

class ProductEditDialog : public QDialog {
public:
    ProductEditDialog(const StockRecord &source, const QString &title, QWidget *parent = nullptr)
        : QDialog(parent), record(source) {
        setWindowTitle(title); setObjectName("stockProductDialog");
        setModal(true);
        setMinimumWidth(470);


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

        articleEdit->setObjectName("stockArticleName");
        lotEdit->setObjectName("stockLot");
        quantitySpin->setObjectName("stockAvailable");
        thresholdSpin->setObjectName("stockThreshold");
        expiryEdit->setObjectName("stockExpiry");
        form->addRow("Catégorie", categoryBox);
        form->addRow("Article", articleEdit);
        form->addRow("Lot", lotEdit);
        form->addRow("Quantité disponible", quantitySpin);
        form->addRow("Unité", unitBox);
        form->addRow("Seuil minimum", thresholdSpin);
        form->addRow("Date d'échéance", expiryEdit);
        layout->addLayout(form);

        QDialogButtonBox *buttons = actionBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
        buttons->button(QDialogButtonBox::Save)->setText("Enregistrer");

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
    loadDemoData();
    setupUi();
    if(OracleConnection::isOracle()) loadOracle();
    refreshTable();
}

bool StockWidget::loadOracle() {
    OracleCollection::Rows rows;QString error;
    if(!OracleCollection::read("stock",&rows,&m_revision,&error)) {
        m_storageReady=false;QMessageBox::critical(this,"Stock Oracle",error);return false;
    }
    m_storageReady=true;
    if(m_revision==0) return persistStock();
    items.clear();
    for(const auto &r:rows) {
        StockRecord item;item.lot=r[0].toString();item.article=r[1].toString();item.category=r[2].toString();
        item.quantity=r[3].toInt();item.unit=r[4].toString();item.threshold=r[5].toInt();item.expiry=QDate::fromString(r[6].toString(),Qt::ISODate);
        items<<item;
    }
    persistedItems=items;return true;
}
bool StockWidget::persistStock() {
    if(!OracleConnection::isOracle()) return true;
    QString error;OracleCollection::Rows rows;
    for(const auto &r:items) rows<<QVariantList{r.lot,r.article,r.category,r.quantity,r.unit,r.threshold,r.expiry.toString(Qt::ISODate)};
    if(!m_storageReady || !OracleCollection::save("stock",rows,&m_revision,&error)) {
        items=persistedItems;refreshTable();
        QMessageBox::critical(this,"Stock non enregistré",error.isEmpty()?"Connexion indisponible. Actualisez le stock.":error);return false;
    }
    persistedItems=items;return true;
}

void StockWidget::loadDemoData() {
    auto add = [this](const QString &article, const QString &category, const QString &lot,
                      int qty, const QString &unit, int threshold, const QString &date) {
        StockRecord item;
        item.article = article;
        item.category = category;
        item.lot = lot;
        item.quantity = qty;
        item.unit = unit;
        item.threshold = threshold;
        item.expiry = QDate::fromString(date, "dd/MM/yyyy");
        items.append(item);
    };

    add("[CHIEN] Croquettes Royal Canin Adulte", "Chiens", "DOG-F-01", 15, "sacs", 5, "22/11/2027");
    add("[CHIEN] Shampooing Poils Longs", "Chiens", "DOG-S-02", 8, "flacons", 5, "10/05/2028");
    add("[CHIEN] Vaccin Rage/Parvovirose", "Chiens", "DOG-V-03", 2, "doses", 10, "15/10/2026");
    add("[CHIEN] Vermifuge comprimés (Dwa)", "Chiens", "DOG-M-04", 45, "boîtes", 15, "05/01/2028");

    add("[CHAT] Pâtée Saumon (Chat Stérilisé)", "Chats", "CAT-F-01", 24, "boîtes", 10, "30/08/2027");
    add("[CHAT] Shampooing Sec Mousse", "Chats", "CAT-S-02", 5, "flacons", 5, "14/02/2028");
    add("[CHAT] Vaccin Typhus/Coryza", "Chats", "CAT-V-03", 12, "doses", 10, "20/12/2026");
    add("[CHAT] Gouttes anti-puces (Dwa)", "Chats", "CAT-M-04", 3, "pipettes", 8, "01/11/2026");

    add("[OISEAU] Mélange Graines Canaris", "Oiseaux", "BIRD-F-01", 2, "kg", 5, "10/11/2026");
    add("[OISEAU] Spray Anti-parasites Plumes", "Oiseaux", "BIRD-S-02", 7, "sprays", 3, "25/09/2028");
    add("[OISEAU] Complément Vitamines (Dwa)", "Oiseaux", "BIRD-M-03", 1, "flacon", 3, "20/10/2026");

    add("[POISSON] Flocons Alimentaires TetraMin", "Poissons", "FISH-F-01", 20, "boîtes", 10, "01/01/2029");
    add("[POISSON] Traitement Eau Anti-chlore", "Poissons", "FISH-M-02", 4, "flacons", 5, "15/07/2027");
    add("[POISSON] Gouttes Anti-fongiques (Dwa)", "Poissons", "FISH-M-03", 12, "flacons", 4, "22/03/2028");

    add("[HAMSTER] Mélange complet Rongeurs", "Rongeurs", "HAM-F-01", 10, "sacs", 5, "18/06/2027");
    add("[HAMSTER] Shampooing poudre", "Rongeurs", "HAM-S-02", 6, "boîtes", 3, "11/04/2028");
    add("[HAMSTER] Vitamines en gouttes (Dwa)", "Rongeurs", "HAM-M-03", 15, "flacons", 5, "09/09/2027");

    add("[MATÉRIEL] Compresses stériles", "Matériel Clinique", "CP-2606", 30, "sachets", 20, "31/10/2026");
    add("[MATÉRIEL] Seringues 5ml", "Matériel Clinique", "SER-05", 200, "unités", 50, "31/12/2029");
}

void StockWidget::setupUi() {
    contentWidget = this;
    auto *contentLayout = new QVBoxLayout(this);
    contentLayout->setContentsMargins(24, 18, 24, 12);
    contentLayout->setSpacing(14);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    QVBoxLayout *titleTexts = new QVBoxLayout;
    titleTexts->setSpacing(2);
    QLabel *title = new QLabel("Stock");
    title->setStyleSheet(QStringLiteral("font-size: 23px; font-weight: 800; color: %1; background: transparent;").arg(kPrimaryDark));
    QLabel *subtitle = new QLabel("Inventaire, traçabilité des lots et réapprovisionnement");
    subtitle->setStyleSheet("font-size: 11px; color: #71878D; background: transparent;");
    titleTexts->addWidget(title);
    titleTexts->addWidget(subtitle);
    titleLayout->addLayout(titleTexts);
    titleLayout->addStretch(1);

    QPushButton *statsButton = makeButton("Statistiques", "#FFFFFF", kPrimaryDark, "1px solid #BDD9DE");
    statsButton->setIcon(Design::icon("grid"));
    QPushButton *editButton = makeButton("Modifier", "#FFFFFF", kPrimaryDark, "1px solid #BDD9DE");
    QPushButton *deleteButton = makeButton("Supprimer", "#FCEDEC", "#B94A48", "1px solid #E8C5C2");
    QPushButton *movementButton = makeButton("Enregistrer un mouvement", kPrimary, "#FFFFFF");
    movementButton->setIcon(Design::icon("plus",Qt::white));
    auto *exportButton=Design::button("Exporter PDF", "export");
    exportButton->setObjectName("stockExportButton");
    connect(exportButton,&QPushButton::clicked,this,&StockWidget::exportInventory);
    titleLayout->addWidget(exportButton);
    titleLayout->addWidget(movementButton);
    contentLayout->addLayout(titleLayout);
    auto *actions=new QHBoxLayout;
    actions->addWidget(statsButton); actions->addStretch();
    actions->addWidget(editButton); actions->addWidget(deleteButton);
    contentLayout->addLayout(actions);

    // Search, category, sort-by-quantity/date and state filters.
    QHBoxLayout *toolbar = new QHBoxLayout;
    toolbar->setSpacing(10);
    searchLineEdit = new QLineEdit;
    searchLineEdit->setPlaceholderText("Rechercher un article, un lot, un état...");
    searchLineEdit->addAction(Design::icon("search"),QLineEdit::LeadingPosition);
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
    // Resolve every table color locally: the Windows palette can otherwise
    // supply a dark AlternateBase even while the surrounding theme is light.
    QPalette stockColors=stockTable->palette();
    stockColors.setColor(QPalette::Base,QColor("#FFFFFF"));
    stockColors.setColor(QPalette::AlternateBase,QColor("#F4F9FA"));
    stockColors.setColor(QPalette::Text,QColor("#263B42"));
    stockColors.setColor(QPalette::Highlight,QColor("#DDEEF1"));
    stockColors.setColor(QPalette::HighlightedText,QColor("#243D45"));
    stockTable->setPalette(stockColors);
    stockTable->setModel(tableModel);
    stockTable->setAlternatingRowColors(true);
    stockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    stockTable->setSelectionMode(QAbstractItemView::SingleSelection);
    stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    stockTable->setShowGrid(true);
    stockTable->verticalHeader()->setVisible(false);
    stockTable->verticalHeader()->setDefaultSectionSize(38);
    stockTable->horizontalHeader()->setMinimumHeight(39);
    stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    stockTable->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    stockTable->setMinimumHeight(230);
    contentLayout->addWidget(stockTable, 1);

    // One action panel replaces the old FEFO and replenishment panels.
    QFrame *expiryPanel = new QFrame;
    expiryPanel->setObjectName("stockExpiryPanel");
    expiryPanel->setStyleSheet("QFrame#stockExpiryPanel { background: #FFFFFF; border: 1px solid #D8E9EC; border-radius: 9px; }");
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
    QLabel *dataNotice = new QLabel(OracleConnection::isOracle()?"Oracle · stock et mouvements enregistrés":"Démo · modifications en mémoire uniquement");
    dataNotice->setStyleSheet("color: #83979C; font-size: 10px; background: transparent;");
    bottomInfo->addWidget(footerLabel);
    bottomInfo->addStretch(1);
    bottomInfo->addWidget(dataNotice);
    contentLayout->addLayout(bottomInfo);


    searchLineEdit->setObjectName("stockSearch");
    categoryFilter->setObjectName("stockCategoryFilter");
    sortFilter->setObjectName("stockSortFilter");
    stateFilter->setObjectName("stockStateFilter");
    stockTable->setObjectName("stockTable");
    movementButton->setObjectName("stockMovementButton");
    statsButton->setObjectName("stockStatisticsButton");
    editButton->setObjectName("stockEditButton");
    deleteButton->setObjectName("stockDeleteButton");
    expiryButton->setObjectName("stockExpiryButton");
    connect(statsButton, &QPushButton::clicked, this, &StockWidget::showStatistics);
    connect(editButton, &QPushButton::clicked, this, &StockWidget::editSelectedItem);
    connect(deleteButton, &QPushButton::clicked, this, &StockWidget::deleteSelectedItem);
    connect(movementButton, &QPushButton::clicked, this, &StockWidget::openMovementDialog);
    connect(expiryButton, &QPushButton::clicked, this, &StockWidget::manageExpiryItems);
    connect(searchLineEdit, &QLineEdit::textChanged, this, &StockWidget::refreshTable);
    connect(categoryFilter, &QComboBox::currentTextChanged, this, &StockWidget::refreshTable);
    connect(sortFilter, qOverload<int>(&QComboBox::currentIndexChanged), this, &StockWidget::refreshTable);
    connect(stateFilter, &QComboBox::currentTextChanged, this, &StockWidget::refreshTable);
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
        if (!query.isEmpty() && !searchable(haystack).contains(searchable(query))) continue;
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
            cell->setEditable(false); cell->setToolTip(values.at(column));
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
}

void StockWidget::updateFooter() {
    if (!footerLabel || !tableModel) return;
    footerLabel->setText(QString("%1 / %2 articles").arg(tableModel->rowCount()).arg(items.size()));
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
    dialog.setWindowTitle("Enregistrer un mouvement de stock"); dialog.setObjectName("stockMovementDialog");
    dialog.setMinimumWidth(500);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QFormLayout *form = new QFormLayout;

    QComboBox *itemBox = new QComboBox;
    for (int i = 0; i < items.size(); ++i) {
        itemBox->addItem(items.at(i).article + " — " + items.at(i).lot, i);
    }

    // Allow the user to create a product directly from this movement window.
    QWidget *articleSelector = new QWidget(&dialog);
    QHBoxLayout *articleSelectorLayout = new QHBoxLayout(articleSelector);
    articleSelectorLayout->setContentsMargins(0, 0, 0, 0);
    QPushButton *addArticleButton = new AnimatedButton("＋ Ajouter un nouvel article", articleSelector);
    addArticleButton->setCursor(Qt::PointingHandCursor);
    articleSelectorLayout->addWidget(itemBox, 1);
    articleSelectorLayout->addWidget(addArticleButton);

    QComboBox *movementType = new QComboBox;
    movementType->addItem("Entrée — ajouter au stock", 0);
    movementType->addItem("Sortie — retirer du stock", 1);
    QSpinBox *quantity = new QSpinBox;
    quantity->setRange(1, 999999);
    quantity->setValue(1);
    itemBox->setObjectName("stockMovementArticle");
    movementType->setObjectName("stockMovementType");
    quantity->setObjectName("stockMovementQuantity");
    addArticleButton->setObjectName("stockAddArticleButton");
    form->addRow("Article / lot", articleSelector);
    form->addRow("Type de mouvement", movementType);
    form->addRow("Quantité", quantity);
    layout->addLayout(form);

    QDialogButtonBox *buttons = actionBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText("Valider le mouvement");

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
        // Append the new product and select it immediately for this movement.
        const int newIndex = items.size();
        items.append(newRecord);
        if(!persistStock()) return;
        itemBox->addItem(newRecord.article + " — " + newRecord.lot, newIndex);
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
        if (!isOutput && items.at(sourceRow).quantity > 999999-amount) {
            QMessageBox::warning(&dialog,"Quantité maximale","Le stock ne peut pas dépasser 999 999 unités par lot."); return;
        }
        if (isOutput) items[sourceRow].quantity -= amount;
        else items[sourceRow].quantity += amount;
        if(!persistStock()) return;
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
        items[index] = dialog.result();
        if(!persistStock()) return;
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
        if (index >= 0 && index < items.size()) items.removeAt(index);
    }
    if(!persistStock()) return;
    refreshTable();
}

void StockWidget::showStatistics() {
    QDialog dialog(this);
    dialog.setWindowTitle("Statistiques du stock"); dialog.setObjectName("stockStatisticsDialog");
    dialog.setMinimumWidth(780);

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

    QDialogButtonBox *buttons = actionBox(QDialogButtonBox::Close);
    buttons->button(QDialogButtonBox::Close)->setText("Fermer");
    root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    dialog.exec();
}

void StockWidget::manageExpiryItems() {
    QDialog dialog(this);
    dialog.setWindowTitle("Gestion des articles périmés"); dialog.setObjectName("stockExpiryDialog");
    dialog.resize(850, 490);

    QVBoxLayout *root = new QVBoxLayout(&dialog);
    QLabel *title = new QLabel("Lots périmés ou à échéance proche");
    title->setStyleSheet(QStringLiteral("font-size: 19px; font-weight: 800; color: %1; background: transparent;").arg(kPrimaryDark));
    QLabel *instruction = new QLabel("Sélectionnez un ou plusieurs lots pour les supprimer. Pour un remplacement, sélectionnez un seul lot et saisissez les informations du nouveau lot.");
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
        table->clearSpans();
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
    QPushButton *deleteButton = makeButton("Supprimer les sélectionnés", "#FCEDEC", "#B94A48", "1px solid #E8C5C2");
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
        if (QMessageBox::question(&dialog, "Confirmer", "Supprimer les lots sélectionnés ?",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
        std::sort(sourceRows.begin(), sourceRows.end(), std::greater<int>());
        for (const int index : sourceRows) {
            if (index >= 0 && index < items.size()) items.removeAt(index);
        }
        if(!persistStock()) {refillTable();return;}
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
            items[index] = editDialog.result();
            if(!persistStock()) {refillTable();return;}
            refreshTable();
            refillTable();
            QMessageBox::information(&dialog, "Lot remplacé", OracleConnection::isOracle()?"Le lot a été remplacé et enregistré dans Oracle.":"Le lot a été remplacé dans la session en cours.");
        }
    });
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();
}

void StockWidget::refresh() { if(OracleConnection::isOracle()) loadOracle(); refreshTable(); }
void StockWidget::focusSearch() { searchLineEdit->setFocus(); searchLineEdit->selectAll(); }
void StockWidget::paintEvent(QPaintEvent *) { QPainter p(this); Design::paintWorkspace(p,rect()); }
bool StockWidget::exportInventoryPdf(const QString &path, QString *error) const {
    QString html="<html><body style='font-family:Segoe UI; color:#34545e; font-size:9pt;'><h1 style='color:#225b66;'>ANIWORLD - Inventaire du stock</h1>";
    html+="<p>"+QString(OracleConnection::isOracle()?"Inventaire enregistré dans Oracle":"DÉMONSTRATION - DONNÉES FICTIVES")+"<br/>Édité le "+QDate::currentDate().toString("dd/MM/yyyy")+"<br/>"+Invoice::escape(categoryFilter->currentText())+" · Recherche : "+Invoice::escape(searchLineEdit->text())+"</p>";
    html+="<table width='100%' cellspacing='0' cellpadding='7'><thead><tr bgcolor='#367e8b'>";
    for(int col=0;col<tableModel->columnCount();++col) html+="<th style='color:white;'>"+Invoice::escape(tableModel->headerData(col,Qt::Horizontal).toString())+"</th>";
    html+="</tr></thead>";
    for(int row=0;row<tableModel->rowCount();++row) {
        html+=row%2?"<tr bgcolor='#edf5f6'>":"<tr>";
        for(int col=0;col<tableModel->columnCount();++col) html+="<td>"+Invoice::escape(tableModel->index(row,col).data().toString())+"</td>";
        html+="</tr>";
    }
    html+="</table><p>"+QString::number(tableModel->rowCount())+" article(s) affiché(s).</p></body></html>";
    return Invoice::writeDocument(path,html,"Inventaire du stock",error);
}
void StockWidget::exportInventory() {
    QString path=QFileDialog::getSaveFileName(this,"Exporter les articles affichés", "ANIWORLD-stock.pdf","Document PDF (*.pdf)");
    if(path.isEmpty()) return;
    if(!path.endsWith(".pdf",Qt::CaseInsensitive)) path+=".pdf";
    QString error;
    if(!exportInventoryPdf(path,&error)) { QMessageBox::warning(this,"Export impossible",error); return; }
    footerLabel->setText("Inventaire affiché exporté en PDF · démo");
}
