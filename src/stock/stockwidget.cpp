#include "ui_integrated_pages.h"
#include "../database/oracleconnection.h"
#include "../ui/localchoice.h"

#include "../widgets/animatedbutton.h"
#include <limits>
#include "../ui/design.h"
#include "../ui/invoice.h"
#include <QPainter>
#include <QFileDialog>
#include "stockwidget.h"
#include "inventoryfeatures.h"
#include "stock.h"
#include "stockjournal.h"

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
        if(!source.category.isEmpty() && categoryBox->findText(source.category)<0)categoryBox->addItem(source.category);
        categoryBox->setCurrentText(source.category);
        categoryBox->setObjectName("stockTypeArticle");

        articleEdit = new QLineEdit(OracleConnection::isOracle()?source.article:articleName(source.article));
        articleEdit->setPlaceholderText("Référence unique");
        nameEdit=new QLineEdit(source.nomArticle.isEmpty()?articleName(source.article):source.nomArticle);
        nameEdit->setObjectName("stockDesignation");nameEdit->setMaxLength(250);
        lotEdit = new QLineEdit(source.lot);
        lotEdit->setPlaceholderText("Référence du lot");

        quantitySpin = new QSpinBox;
        quantitySpin->setRange(0, 999999);
        quantitySpin->setValue(source.quantity);
        thresholdSpin = new QSpinBox;
        thresholdSpin->setRange(1, 999999);
        thresholdSpin->setValue(qMax(1, source.threshold));

        unitBox = new QComboBox(this);
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
        if(OracleConnection::isOracle()) {
            auto *id=new QLineEdit(source.idArticle>0?QString::number(source.idArticle):QString("Automatique"),this);
            id->setObjectName("stockIdArticle");id->setReadOnly(true);form->addRow("ID article",id);
        } else form->addRow("Catégorie", categoryBox);
        form->addRow("Référence article (unique)", articleEdit);
        form->addRow("Nom article", nameEdit);
        form->addRow("Lot", lotEdit);
        form->addRow("Quantité disponible", quantitySpin);
        if(OracleConnection::isOracle())form->addRow("Type article",categoryBox);
        if(!OracleConnection::isOracle())form->addRow("Unité", unitBox);else unitBox->hide();
        form->addRow("Seuil minimum", thresholdSpin);
        form->addRow("Date d'échéance", expiryEdit);
        localBox=new QComboBox; localBox->setObjectName("stockLocal"); LocalChoice::populate(localBox,source.localId);
        form->addRow("Local",localBox);
        layout->addLayout(form);

        QDialogButtonBox *buttons = actionBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
        buttons->button(QDialogButtonBox::Save)->setText("Enregistrer");

        buttons->button(QDialogButtonBox::Cancel)->setText("Annuler");
        layout->addWidget(buttons);

        connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
            if(OracleConnection::isOracle() && localBox->currentData().toInt()<=0){QMessageBox::warning(this,"Local obligatoire","Choisissez le local qui contient ce lot.");return;}
            if (articleEdit->text().trimmed().isEmpty() ||
                (!OracleConnection::isOracle() && (lotEdit->text().trimmed().isEmpty() || unitBox->currentText().trimmed().isEmpty()))) {
                QMessageBox::warning(this, "Champs obligatoires", OracleConnection::isOracle()?"Renseignez la référence article.":"Remplissez le nom de l'article, le lot et l'unité.");
                return;
            }
            accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    StockRecord result() const {
        StockRecord updated = record;
        updated.category = categoryBox->currentText();
        updated.article = (OracleConnection::isOracle()?QString():prefixForCategory(updated.category)) + articleEdit->text().trimmed();
        updated.nomArticle = nameEdit->text().trimmed();
        if(updated.nomArticle.isEmpty()) updated.nomArticle=articleEdit->text().trimmed();
        updated.lot = lotEdit->text().trimmed();
        updated.quantity = quantitySpin->value();
        updated.unit = unitBox->currentText().trimmed();
        updated.threshold = thresholdSpin->value();
        updated.expiry = expiryEdit->date();
        updated.localId=localBox->currentData().toInt();
        return updated;
    }

private:
    StockRecord record;
    QComboBox *categoryBox = nullptr;
    QComboBox *localBox = nullptr;
    QLineEdit *articleEdit = nullptr;
    QLineEdit *nameEdit = nullptr;
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
    QString error;
    QVector<StockRecord> loaded;
    if(!Stock::afficher(&loaded,&m_revision,&error)) {
        m_storageReady=false;QMessageBox::critical(this,"Stock Oracle",error);return false;
    }
    m_storageReady=true;items=loaded;persistedItems=items;return true;
}
bool StockWidget::persistStock() {
    if(!OracleConnection::isOracle()) return true;
    QString error;
    if(!m_storageReady || !Stock::enregistrer(items,&m_revision,&error)) {
        items=persistedItems;refreshTable();
        QMessageBox::critical(this,"Stock non enregistré",error.isEmpty()?"Connexion indisponible. Actualisez le stock.":error);return false;
    }
    if(!Stock::afficher(&items,&m_revision,&error)) {m_storageReady=false;return false;}
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

// === MODULE STOCK : INTERFACE ET SIGNAUX / SLOTS ===
void StockWidget::setupUi() {
    Ui::StockPage ui;ui.setupUi(this);contentWidget=this;
    alertsNavButton=ui.Stock_alertsNavButton;
    categoryFilter=ui.Stock_categoryFilter;
    footerLabel=ui.Stock_footerLabel;
    searchLineEdit=ui.Stock_searchLineEdit;
    sortFilter=ui.Stock_sortFilter;
    stateFilter=ui.Stock_stateFilter;
    stockTable=ui.Stock_stockTable;
    for(int i=0;i<sortFilter->count();++i)sortFilter->setItemData(i,i);
    tableModel->setHorizontalHeaderLabels(OracleConnection::isOracle()
      ?QStringList{"ID ARTICLE","ARTICLE","NOM ARTICLE","LOT","QUANTITÉ DISP.","TYPE ARTICLE","SEUIL MIN.","DATE ÉCHÉANCE","ID LOCAL"}
      :QStringList{"ARTICLE","LOT","DISPONIBLE","SEUIL","ÉCHÉANCE","ÉTAT"});
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
    stockTable->setColumnWidth(0, OracleConnection::isOracle()?90:300);
    stockTable->setColumnWidth(1, 125);
    stockTable->setColumnWidth(2, 140);
    stockTable->setColumnWidth(3, 72);
    stockTable->setColumnWidth(4, 130);
    stockTable->setMinimumHeight(150);

    connect(ui.stockMovementButton,&QPushButton::clicked,this,&StockWidget::openMovementDialog);
    connect(ui.stockExportButton,&QPushButton::clicked,this,&StockWidget::exportInventory);
    connect(ui.stockStatisticsButton,&QPushButton::clicked,this,&StockWidget::showStatistics);
    connect(ui.stockEditButton,&QPushButton::clicked,this,&StockWidget::editSelectedItem);
    connect(ui.stockDeleteButton,&QPushButton::clicked,this,&StockWidget::deleteSelectedItem);
    connect(ui.stockExpiryButton,&QPushButton::clicked,this,&StockWidget::manageExpiryItems);
    connect(ui.stockPurchasesButton,&QPushButton::clicked,this,&StockWidget::openPurchasesModule);
    connect(alertsNavButton,&QPushButton::clicked,this,&StockWidget::openAlertsModule);
    connect(ui.stockReplenishmentButton,&QPushButton::clicked,this,&StockWidget::openReplenishmentModule);
    connect(searchLineEdit,&QLineEdit::textChanged,this,&StockWidget::refreshTable);
    for(auto *combo:{categoryFilter,sortFilter,stateFilter})connect(combo,&QComboBox::currentIndexChanged,this,&StockWidget::refreshTable);
}

void StockWidget::openPurchasesModule() {
    if (InventoryFeatures::showPurchases(this, items)) {
        refresh();
        refreshTable();
    }
}

void StockWidget::openAlertsModule() {
    if(OracleConnection::isOracle() && !loadOracle())return;
    refreshTable();
    InventoryFeatures::showAlerts(this, items);
    refresh();
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
    if (item.quantity <= item.threshold) {
        return "Sous le seuil";
    }
    return "Disponible";
}

int StockWidget::countBelowThreshold() const {
    int count = 0;
    for (const StockRecord &item : items) {
        if (item.quantity <= item.threshold) ++count;
    }
    return count;
}

int StockWidget::countExpiryAlerts() const {
    int count = 0;
    const QDate limit = QDate::currentDate().addDays(30);
    for (const StockRecord &item : items) {
        if (item.quantity > 0 && item.expiry.isValid() && item.expiry <= limit) ++count;
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
        const QString haystack = item.article + " " + item.nomArticle + " " + item.lot + " " + itemState + " " + item.expiry.toString("dd/MM/yyyy");
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
        const QString designation=record.nomArticle.isEmpty() || record.nomArticle==record.article || record.nomArticle==articleName(record.article)?record.article:record.article+" — "+record.nomArticle;
        QStringList values = {designation, record.lot,
                              QString::number(record.quantity) + " " + record.unit,
                              QString::number(record.threshold),
                              record.expiry.toString("dd/MM/yyyy"), stateText};
        if(OracleConnection::isOracle())values={QString::number(record.idArticle),record.article,record.nomArticle,record.lot,
            QString::number(record.quantity),record.category,QString::number(record.threshold),record.expiry.toString("dd/MM/yyyy"),QString::number(record.localId)};
        for (int column = 0; column < values.size(); ++column) {
            QStandardItem *cell = new QStandardItem(values.at(column));
            cell->setEditable(false); cell->setToolTip(values.at(column));
            if (column == 0) cell->setData(sourceRow, Qt::UserRole);
            if (column == 5 && !OracleConnection::isOracle()) {
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
        items.append(newRecord);
        if(!persistStock()) return;
        itemBox->clear();int selected=0;
        for(int i=0;i<items.size();++i){itemBox->addItem(items[i].article+" — "+items[i].lot,i);if(items[i].article==newRecord.article)selected=i;}
        itemBox->setCurrentIndex(selected);
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
        const auto changed=items[sourceRow];
        if (isOutput) items[sourceRow].quantity -= amount;
        else items[sourceRow].quantity += amount;
        if(!persistStock()) return;
        QString journalError;
        if(!StockJournal::mouvement(changed.idArticle,isOutput?"SORTIE":"ENTREE",amount,changed.article,&journalError))QMessageBox::warning(&dialog,"Journal du stock",journalError);
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
        if (QMessageBox::question(&dialog, "Confirmer", "Mettre à zéro le disponible des lots sélectionnés en conservant leur traçabilité ?",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
        QVector<StockRecord> retired;
        for (const int index : sourceRows) {
            if (index >= 0 && index < items.size()){retired<<items[index];items[index].quantity=0;}
        }
        if(!persistStock()) {refillTable();return;}
        QString journalError;
        for(const auto &r:retired)if(r.quantity>0 && !StockJournal::mouvement(r.idArticle,"PERTE_PEREMPTION",r.quantity,r.article,&journalError))QMessageBox::warning(&dialog,"Journal du stock",journalError);
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
        proposed.idArticle=0;
        proposed.article=proposed.article.left(210)+"-"+QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
        proposed.lot = "NOUVEAU-LOT";
        proposed.quantity = qMax(1, proposed.threshold);
        proposed.expiry = QDate::currentDate().addYears(1);
        ProductEditDialog editDialog(proposed, "Créer le nouveau lot", &dialog);
        if (editDialog.exec() == QDialog::Accepted) {
            auto replacement=editDialog.result();replacement.idArticle=0;
            for(const auto &r:items)if(r.article==replacement.article){QMessageBox::warning(&dialog,"Référence unique","Le nouveau lot doit avoir une référence article distincte.");return;}
            const auto previous=items[index];items[index].quantity=0;items.append(replacement);
            if(!persistStock()) {refillTable();return;}
            QString journalError;
            if(previous.quantity>0 && !StockJournal::mouvement(previous.idArticle,"PERTE_PEREMPTION",previous.quantity,previous.article,&journalError))QMessageBox::warning(&dialog,"Journal du stock",journalError);
            refreshTable();
            refillTable();
            QMessageBox::information(&dialog, "Lot remplacé", "L’ancien lot est conservé avec une quantité nulle. Le nouveau lot possède une référence et un ID distincts.");
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

void StockWidget::updateAlertNavigation() {
    if (!alertsNavButton) return;
    int activeAlertCount = countExpiryAlerts();
    for (const StockRecord &record : items) {
        if (record.quantity <= record.threshold) ++activeAlertCount;
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

