#include "stockwidget.h"
#include "../ui/design.h"
#include "../ui/invoice.h"
#include "../ui/motion.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QFrame>
#include <QHeaderView>
#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QColor>
#include <QStringList>
#include <QList>
#include <QSortFilterProxyModel>
#include <QDate>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QSaveFile>
#include <QMessageBox>
#include <QTimer>
#include <QPainter>

namespace {
constexpr int ValueRole=Qt::UserRole+1;
constexpr int CategoryRole=Qt::UserRole+2;
QString folded(const QString &text) {
    QString result;
    for(const QChar ch:text.normalized(QString::NormalizationForm_D).toCaseFolded())
        if(ch.category()!=QChar::Mark_NonSpacing) result+=ch;
    return result;
}
QString categoryFor(const QString &article) {
    const QStringList tags={"[CHIEN]","[CHAT]","[OISEAU]","[POISSON]","[HAMSTER]","[MATÉRIEL]"};
    const QStringList names={"Chiens","Chats","Oiseaux","Poissons","Rongeurs","Matériel Clinique"};
    for(int i=0;i<tags.size();++i) if(article.startsWith(tags[i])) return names[i];
    return {};
}

}

class StockFilterModel : public QSortFilterProxyModel {
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    void setSearch(const QString &value) { search=folded(value.trimmed()); invalidateFilter(); }
    void setCategory(const QString &value) { category=value; invalidateFilter(); }
protected:
    bool filterAcceptsRow(int row,const QModelIndex &parent) const override {
        const auto first=sourceModel()->index(row,0,parent);
        if(!category.isEmpty() && first.data(CategoryRole).toString()!=category) return false;
        QString text=first.data(CategoryRole).toString();
        for(int col=0;col<sourceModel()->columnCount();++col) text+=' '+sourceModel()->index(row,col,parent).data().toString();
        return folded(text).contains(search);
    }
private:
    QString search,category;
};

StockWidget::StockWidget(QWidget *parent) : QWidget(parent), tableModel(new QStandardItemModel(this)) {
    setObjectName("stockPage");
    filterModel=new StockFilterModel(this);
    filterModel->setSourceModel(tableModel);
    filterModel->setSortRole(ValueRole);
    setupUi();
    setStyleSheet(
        "QWidget#stockPage { background: #F3F7F8; }"
        "QTableView {"
        "  background-color: #FFFFFF;"
        "  alternate-background-color: #F9FCFC;"
        "  color: #263B42;"
        "  border: 1px solid #E4F0F3;"
        "  gridline-color: #E4F0F3;"
        "  selection-background-color: #E4F0F3;"
        "  selection-color: #263B42;"
        "}"
        "QHeaderView::section {"
        "  background-color: #3C8190;"
        "  color: #FFFFFF;"
        "  font-weight: bold;"
        "  font-size: 10px;"
        "  padding: 10px 6px;"
        "  border: none;"
        "}"
        );
    refresh();
}

void StockWidget::paintEvent(QPaintEvent *) { QPainter p(this); Design::paintWorkspace(p,rect()); }

void StockWidget::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(24, 20, 24, 16);

    // --- En-tête ---
    QHBoxLayout *headerLayout = new QHBoxLayout();
    auto *titles=new QVBoxLayout; titles->setSpacing(4);
    auto *titleRow=new QHBoxLayout; titleRow->setSpacing(9);
    auto *titleIcon=Design::label(""); titleIcon->setPixmap(Design::icon("stock",QColor("#D69335")).pixmap(24,24));
    titleRow->addWidget(titleIcon); titleRow->addWidget(Design::label("Stock","title")); titleRow->addStretch(); titles->addLayout(titleRow);
    auto *subtitle=Design::label("Inventaire, traçabilité des lots et réapprovisionnement","muted");
    subtitle->setWordWrap(true); titles->addWidget(subtitle);
    auto *btnMovement=Design::button("Enregistrer un mouvement","plus","primary");
    btnMovement->setObjectName("stockMovementButton");
    btnMovement->setToolTip("Simuler une entrée ou une sortie · Ctrl+N");
    connect(btnMovement, &QPushButton::clicked, this, &StockWidget::openMovementDialog);

    headerLayout->addLayout(titles,1);
    headerLayout->addStretch();
    headerLayout->addWidget(btnMovement);
    mainLayout->addLayout(headerLayout);

    // --- Cartes KPI ---
    QHBoxLayout *kpiLayout = new QHBoxLayout();
    setupKpiCards(kpiLayout);
    mainLayout->addLayout(kpiLayout);

    // --- Barre d'outils et recherche ---
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    searchLineEdit = new QLineEdit();
    searchLineEdit->setObjectName("stockSearch");
    searchLineEdit->setPlaceholderText("Rechercher un article, un lot, un état…");
    searchLineEdit->setClearButtonEnabled(true);
    searchLineEdit->setAccessibleName("Rechercher dans le stock");
    searchLineEdit->addAction(Design::icon("search"),QLineEdit::LeadingPosition);

    categoryFilter = new QComboBox();
    categoryFilter->setObjectName("stockCategoryFilter");
    categoryFilter->setAccessibleName("Filtrer le stock par catégorie");
    categoryFilter->addItems({"Toutes les catégories", "Chiens", "Chats", "Oiseaux", "Poissons", "Rongeurs", "Matériel Clinique"});
    auto *btnExport=Design::button("Exporter PDF","export");
    btnExport->setObjectName("stockExportButton");
    connect(btnExport,&QPushButton::clicked,this,&StockWidget::exportInventory);
    connect(searchLineEdit,&QLineEdit::textChanged,this,[this](const QString &text) {
        filterModel->setSearch(text); updateResultCount();
    });
    connect(categoryFilter,&QComboBox::currentIndexChanged,this,[this](int index) {
        filterModel->setCategory(index==0?QString():categoryFilter->currentText()); updateResultCount();
    });

    toolbarLayout->addWidget(searchLineEdit, 1);
    toolbarLayout->addWidget(categoryFilter);
    toolbarLayout->addWidget(btnExport);
    mainLayout->addLayout(toolbarLayout);

    // --- Tableau de données ---
    stockTable = new QTableView();
    stockTable->setObjectName("stockTable");
    stockTable->setAlternatingRowColors(true);
    stockTable->horizontalHeader()->setStretchLastSection(false);
    stockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    stockTable->setSelectionMode(QAbstractItemView::SingleSelection);
    stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    stockTable->setMinimumHeight(150);
    stockTable->setWordWrap(false);
    stockTable->verticalHeader()->setDefaultSectionSize(42);
    stockTable->verticalHeader()->setVisible(false); // Na7iw l-arkam l-zayda 3al isar

    // Configuration du modèle et des titres (3anawin mta3 l-jadwel)
    tableModel->setColumnCount(6);
    tableModel->setHorizontalHeaderLabels({"ARTICLE / ANIMAL", "LOT", "DISPONIBLE", "SEUIL", "ÉCHÉANCE", "ÉTAT"});

    // ==========================================
    // DONNÉES COMPLÈTES (Mekla, Shampooing, Dwa, Tala9i7)
    // ==========================================
    QList<QStringList> fictiveData = {
        // CHIENS
        {"[CHIEN] Croquettes Royal Canin Adulte", "DOG-F-01", "15 sacs", "5", "22/11/2027", "Disponible"},
        {"[CHIEN] Shampooing Poils Longs", "DOG-S-02", "8 flacons", "5", "10/05/2028", "Disponible"},
        {"[CHIEN] Vaccin Rage/Parvovirose", "DOG-V-03", "2 doses", "10", "15/10/2026", "Sous le seuil"},
        {"[CHIEN] Vermifuge comprimés (Dwa)", "DOG-M-04", "45 boîtes", "15", "05/01/2028", "Disponible"},

        // CHATS
        {"[CHAT] Pâtée Saumon (Chat Sterilisé)", "CAT-F-01", "24 boîtes", "10", "30/08/2027", "Disponible"},
        {"[CHAT] Shampooing Sec Mousse", "CAT-S-02", "5 flacons", "5", "14/02/2028", "Disponible"},
        {"[CHAT] Vaccin Typhus/Coryza", "CAT-V-03", "12 doses", "10", "20/12/2026", "Disponible"},
        {"[CHAT] Gouttes anti-puces (Dwa)", "CAT-M-04", "3 pipettes", "8", "01/11/2026", "Sous le seuil"},

        // OISEAUX
        {"[OISEAU] Mélange Graines Canaris", "BIRD-F-01", "2 kg", "5", "10/11/2026", "Sous le seuil"},
        {"[OISEAU] Spray Anti-parasites Plumes", "BIRD-S-02", "7 sprays", "3", "25/09/2028", "Disponible"},
        {"[OISEAU] Complément Vitamines (Dwa)", "BIRD-M-03", "1 flacon", "3", "20/10/2026", "Échéance proche"},

        // POISSONS
        {"[POISSON] Flocons Alimentaires TetraMin", "FISH-F-01", "20 boîtes", "10", "01/01/2029", "Disponible"},
        {"[POISSON] Traitement Eau Anti-chlore", "FISH-M-02", "4 flacons", "5", "15/07/2027", "Sous le seuil"},
        {"[POISSON] Gouttes Anti-fongiques (Dwa)", "FISH-M-03", "12 flacons", "4", "22/03/2028", "Disponible"},

        // HAMSTERS / RONGEURS
        {"[HAMSTER] Mélange complet Rongeurs", "HAM-F-01", "10 sacs", "5", "18/06/2027", "Disponible"},
        {"[HAMSTER] Shampooing poudre", "HAM-S-02", "6 boîtes", "3", "11/04/2028", "Disponible"},
        {"[HAMSTER] Vitamines en gouttes (Dwa)", "HAM-M-03", "15 flacons", "5", "09/09/2027", "Disponible"},

        // MATÉRIEL CLINIQUE
        {"[MATÉRIEL] Compresses stériles", "CP-2606", "30 sachets", "20", "31/10/2026", "Échéance proche"},
        {"[MATÉRIEL] Seringues 5ml", "SER-05", "200 unités", "50", "31/12/2029", "Disponible"}
    };

    // Original sample catalogue from stockaniworld; no real inventory is loaded.
    for (const QStringList &rowData : fictiveData) {
        QList<QStandardItem *> rowItems;
        for (int i = 0; i < 6; ++i) {
            QStandardItem *item = new QStandardItem(rowData[i]);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Tableau lecture seule

            item->setForeground(QColor("#263B42"));
            item->setToolTip(rowData[i]);
            item->setData(folded(rowData[i]),ValueRole);
            if(i==0) item->setData(categoryFor(rowData[i]),CategoryRole);
            if(i==2) item->setData(rowData[i].section(' ',0,0).toInt(),ValueRole);
            if(i==3) item->setData(rowData[i].toInt(),ValueRole);
            if(i==4) item->setData(QDate::fromString(rowData[i],"dd/MM/yyyy"),ValueRole);
            rowItems.append(item);
        }
        tableModel->appendRow(rowItems);
    }

    stockTable->setModel(filterModel);
    stockTable->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    for(int col=1;col<6;++col) stockTable->horizontalHeader()->setSectionResizeMode(col,QHeaderView::Fixed);
    stockTable->setColumnWidth(1,98); stockTable->setColumnWidth(2,95);
    stockTable->setColumnWidth(3,54); stockTable->setColumnWidth(4,97); stockTable->setColumnWidth(5,128);
    stockTable->setSortingEnabled(true); stockTable->sortByColumn(-1,Qt::AscendingOrder);
    mainLayout->addWidget(stockTable, 1);

    // --- Panneaux des métiers innovants ---
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    setupActionPanels(actionsLayout);
    mainLayout->addLayout(actionsLayout);
    auto *footer=new QHBoxLayout;
    resultCount=Design::label("","muted"); resultCount->setObjectName("stockResultCount");
    notice=Design::label("Démonstration · modifications non enregistrées","muted");
    notice->setObjectName("stockNotice"); notice->setWordWrap(true);
    footer->addWidget(resultCount); footer->addStretch(); footer->addWidget(notice);
    mainLayout->addLayout(footer);
}

void StockWidget::setupKpiCards(QHBoxLayout *layout) {
    QStringList titles = {"Articles référencés", "Sous le seuil", "Lots à contrôler"};
    QStringList subtitles = {"Produits actifs", "À réapprovisionner", "Échéances proches"};

    for (int i = 0; i < 3; ++i) {
        auto *card=Design::card();
        card->setStyleSheet(QString("QFrame[role=card] { background:%1; border:none; border-radius:8px; }").arg(QStringList{"#E4F1F4","#FFF2E0","#FBE9E8"}.at(i)));
        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(16,12,16,12); cardLayout->setSpacing(4);

        QLabel *lblTitle = new QLabel(titles[i]);
        lblTitle->setStyleSheet("color: #397F8D; font-weight: bold; background: transparent;");
        QLabel *lblValue = new QLabel("0");
        lblValue->setObjectName(QString("stockKpi%1").arg(i)); kpiValues.append(lblValue);
        lblValue->setStyleSheet("font-size: 24px; color: #225A64; font-weight: bold; background: transparent;");
        QLabel *lblSub = new QLabel(subtitles[i]);
        lblSub->setStyleSheet("color: #7f8c8d; background: transparent;");

        cardLayout->addWidget(lblTitle);
        cardLayout->addWidget(lblValue);
        cardLayout->addWidget(lblSub);
        layout->addWidget(card);
    }
}

void StockWidget::setupActionPanels(QHBoxLayout *layout) {
    auto *fefoPanel=Design::card();
    QVBoxLayout *fefoLayout = new QVBoxLayout(fefoPanel);
    fefoLayout->setContentsMargins(15,12,15,12); fefoLayout->setSpacing(7);
    auto *fefoTitle=Design::label("Échéance prioritaire · FEFO");
    fefoTitle->setStyleSheet("color:#AE5751;font-weight:600;"); fefoLayout->addWidget(fefoTitle);
    fefoText=Design::label("","muted"); fefoText->setWordWrap(true); fefoText->setObjectName("stockFefoSummary");
    fefoLayout->addWidget(fefoText,1);
    fefoButton=Design::button("Préparer la sortie","arrow","primary"); fefoButton->setObjectName("stockFefoButton");
    connect(fefoButton,&QPushButton::clicked,this,&StockWidget::prepareFefoCheckout);
    fefoLayout->addWidget(fefoButton); layout->addWidget(fefoPanel,1);

    auto *reorderPanel=Design::card();
    QVBoxLayout *reorderLayout = new QVBoxLayout(reorderPanel);
    reorderLayout->setContentsMargins(15,12,15,12); reorderLayout->setSpacing(7);
    auto *reorderTitle=Design::label("Réapprovisionnement");
    reorderTitle->setStyleSheet("color:#B48332;font-weight:600;"); reorderLayout->addWidget(reorderTitle);
    reorderText=Design::label("","muted"); reorderText->setWordWrap(true); reorderText->setObjectName("stockReorderSummary");
    reorderLayout->addWidget(reorderText,1);
    reorderButton=Design::button("Vérifier la proposition","check","success"); reorderButton->setObjectName("stockReorderButton");
    connect(reorderButton,&QPushButton::clicked,this,&StockWidget::verifyReorder);
    reorderLayout->addWidget(reorderButton); layout->addWidget(reorderPanel,1);
}

void StockWidget::refresh() {
    int low=0,near=0; fefoRow=-1; reorderRow=-1;
    const QDate today=QDate::currentDate();
    for(int row=0;row<tableModel->rowCount();++row) {
        const int quantity=tableModel->item(row,2)->data(ValueRole).toInt();
        const int threshold=tableModel->item(row,3)->data(ValueRole).toInt();
        const QDate expiry=tableModel->item(row,4)->data(ValueRole).toDate();
        const bool isLow=quantity<threshold, expired=expiry<today;
        const bool needsCheck=quantity>0 && expiry<=today.addDays(30);
        if(isLow) { ++low; if(reorderRow<0) reorderRow=row; }
        if(needsCheck) ++near;
        if(quantity>0 && !expired && (fefoRow<0 || expiry<tableModel->item(fefoRow,4)->data(ValueRole).toDate())) fefoRow=row;
        const QString state=expired?"Périmé":(quantity==0?"Rupture":(isLow?"Sous le seuil":(needsCheck?"Échéance proche":"Disponible")));
        auto *status=tableModel->item(row,5); status->setText(state); status->setData(folded(state),ValueRole);
        status->setToolTip(state+(isLow?" · Réapprovisionnement nécessaire":"")+(needsCheck?" · Lot à contrôler":""));
        status->setForeground(QColor(expired || quantity==0?"#AE5751":(isLow || needsCheck?"#AD7A2F":"#397F8D")));
    }
    Motion::countTo(kpiValues[0],tableModel->rowCount()); Motion::countTo(kpiValues[1],low); Motion::countTo(kpiValues[2],near);
    if(fefoRow>=0) {
        fefoText->setText(tableModel->item(fefoRow,0)->text()+"\nLot "+tableModel->item(fefoRow,1)->text()+" · "+tableModel->item(fefoRow,4)->text());
    } else fefoText->setText("Aucun lot disponible non périmé.");
    fefoButton->setEnabled(fefoRow>=0);
    reorderText->setText(reorderRow>=0?QString("%1 article(s) sous le seuil.\nExaminer les quantités à commander.").arg(low):"Tous les articles atteignent leur seuil minimum.");
    reorderButton->setEnabled(reorderRow>=0); updateResultCount();
}

void StockWidget::updateResultCount() {
    resultCount->setText(QString("%1 / %2 articles%3").arg(filterModel->rowCount()).arg(tableModel->rowCount())
        .arg(filterModel->rowCount()==0?" · Aucun résultat":""));
}
void StockWidget::focusSearch() { searchLineEdit->setFocus(); searchLineEdit->selectAll(); }
void StockWidget::openMovementDialog() {
    const auto selected=filterModel->mapToSource(stockTable->currentIndex());
    showMovementDialog(selected.isValid()?selected.row():0,false);
}
void StockWidget::prepareFefoCheckout() {
    refresh(); if(fefoRow>=0) showMovementDialog(fefoRow,true);
}
void StockWidget::showMovementDialog(int row,bool checkout) {
    QDialog dialog(this); dialog.setObjectName("stockMovementDialog");
    dialog.setWindowTitle("ANIWORLD — Mouvement de stock (démo)"); dialog.setMinimumWidth(580);
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(25,24,25,24); layout->setSpacing(14);
    layout->addWidget(Design::label("Mouvement de stock","title"));
    auto *demo=Design::label("Simulation : les changements disparaissent à la fermeture d'ANIWORLD.","muted");
    demo->setWordWrap(true); layout->addWidget(demo);
    auto *form=new QFormLayout; form->setSpacing(12);
    auto *article=new QComboBox; article->setObjectName("stockMovementArticle");
    for(int i=0;i<tableModel->rowCount();++i) article->addItem(tableModel->item(i,0)->text()+" · "+tableModel->item(i,1)->text(),i);
    article->setCurrentIndex(qBound(0,row,tableModel->rowCount()-1));
    article->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon); article->setMinimumContentsLength(35);
    auto *type=new QComboBox; type->setObjectName("stockMovementType"); type->addItems({"Entrée","Sortie"}); type->setCurrentIndex(checkout?1:0);
    auto *amount=new QSpinBox; amount->setObjectName("stockMovementQuantity"); amount->setRange(1,1000000);
    auto *available=Design::label("","muted");
    auto updateAvailable=[this,article,available] {
        available->setText("Disponible : "+tableModel->item(article->currentData().toInt(),2)->text());
    };
    connect(article,&QComboBox::currentIndexChanged,&dialog,updateAvailable); updateAvailable();
    form->addRow("Article / lot",article); form->addRow("Mouvement",type); form->addRow("Quantité",amount); form->addRow("",available);
    layout->addLayout(form);
    auto *error=Design::label(""); error->setObjectName("stockMovementError"); error->setWordWrap(true);
    error->setStyleSheet("color:#AE5751;"); error->hide(); layout->addWidget(error);
    auto *buttons=new QHBoxLayout; buttons->addStretch();
    auto *cancel=Design::button("Annuler"); auto *save=Design::button("Appliquer en démo","check","primary");
    save->setObjectName("saveStockMovement"); buttons->addWidget(cancel); buttons->addWidget(save); layout->addLayout(buttons);
    connect(cancel,&QPushButton::clicked,&dialog,&QDialog::reject);
    connect(save,&QPushButton::clicked,&dialog,[this,&dialog,article,type,amount,error] {
        const int row=article->currentData().toInt(); auto *cell=tableModel->item(row,2);
        const int before=cell->data(ValueRole).toInt(); const bool outgoing=type->currentIndex()==1;
        if(outgoing && amount->value()>before) { error->setText("La sortie dépasse la quantité disponible."); error->show(); return; }
        if(!outgoing && before>1000000-amount->value()) { error->setText("La quantité maximale de cette démonstration est 1 000 000."); error->show(); return; }
        const int after=before+(outgoing?-amount->value():amount->value());
        const QString unit=cell->text().section(' ',1);
        cell->setData(after,ValueRole); cell->setText(QString::number(after)+' '+unit); cell->setToolTip(cell->text());
        refresh(); notice->setText("Mouvement simulé · non enregistré");
        const QModelIndex visible=filterModel->mapFromSource(tableModel->index(row,0));
        if(visible.isValid()) stockTable->selectRow(visible.row());
        dialog.accept();
    });
    QTimer::singleShot(0,&dialog,[&dialog] { Motion::reveal(&dialog,QColor("#F3F7F8")); });
    dialog.exec();
}

void StockWidget::verifyReorder() {
    refresh(); if(reorderRow<0) return;
    QDialog dialog(this); dialog.setObjectName("stockReorderDialog"); dialog.setWindowTitle("ANIWORLD — Réapprovisionnement (démo)");
    dialog.setMinimumWidth(560);
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(25,24,25,24); layout->setSpacing(14);
    layout->addWidget(Design::label("Proposition de réapprovisionnement","section"));
    auto *note=Design::label("Hypothèse de démonstration : 3 unités/jour pendant 5 jours, plus 2 unités de sécurité. Aucune commande n'est envoyée.","muted");
    note->setWordWrap(true); layout->addWidget(note);
    auto *article=new QComboBox; article->setObjectName("stockReorderArticle");
    for(int row=0;row<tableModel->rowCount();++row)
        if(tableModel->item(row,2)->data(ValueRole).toInt()<tableModel->item(row,3)->data(ValueRole).toInt())
            article->addItem(tableModel->item(row,0)->text(),row);
    article->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon); article->setMinimumContentsLength(35);
    layout->addWidget(article);
    auto *proposal=Design::label(""); proposal->setObjectName("stockReorderQuantity"); proposal->setWordWrap(true);
    auto update=[this,article,proposal] {
        const int row=article->currentData().toInt(),available=tableModel->item(row,2)->data(ValueRole).toInt();
        const int threshold=tableModel->item(row,3)->data(ValueRole).toInt();
        proposal->setText(QString("Disponible : %1 · seuil : %2\nQuantité proposée : %3\nmax(seuil, 3 × 5 + 2) − disponible").arg(available).arg(threshold).arg(qMax(0,qMax(threshold,17)-available)));
    };
    connect(article,&QComboBox::currentIndexChanged,&dialog,update); update(); layout->addWidget(proposal);
    auto *close=Design::button("Fermer",{},"primary"); connect(close,&QPushButton::clicked,&dialog,&QDialog::accept); layout->addWidget(close,0,Qt::AlignRight);
    QTimer::singleShot(0,&dialog,[&dialog] { Motion::reveal(&dialog,QColor("#F3F7F8")); }); dialog.exec();
}

bool StockWidget::exportInventoryPdf(const QString &path, QString *error) const {
    QString html="<html><body style='font-family:Segoe UI; color:#34545e; font-size:9pt;'><h1 style='color:#225b66;'>ANIWORLD - Inventaire du stock</h1>";
    html+="<p>DÉMONSTRATION - DONNÉES FICTIVES<br/>Édité le "+QDate::currentDate().toString("dd/MM/yyyy")+"<br/>"+Invoice::escape(categoryFilter->currentText())+" · Recherche : "+Invoice::escape(searchLineEdit->text())+"</p>";
    html+="<table width='100%' cellspacing='0' cellpadding='7'><thead><tr bgcolor='#367e8b'>";
    for(int col=0;col<tableModel->columnCount();++col) html+="<th style='color:white;'>"+Invoice::escape(tableModel->headerData(col,Qt::Horizontal).toString())+"</th>";
    html+="</tr></thead>";
    for(int row=0;row<filterModel->rowCount();++row) {
        html+=row%2?"<tr bgcolor='#edf5f6'>":"<tr>";
        for(int col=0;col<tableModel->columnCount();++col) html+="<td>"+Invoice::escape(filterModel->index(row,col).data().toString())+"</td>";
        html+="</tr>";
    }
    html+="</table><p>"+QString::number(filterModel->rowCount())+" article(s) affiché(s).</p></body></html>";
    return Invoice::writeDocument(path,html,"Inventaire du stock",error);
}
void StockWidget::exportInventory() {
    QString path=QFileDialog::getSaveFileName(this,"Exporter les articles affichés", "ANIWORLD-stock-demo.pdf","Document PDF (*.pdf)");
    if(path.isEmpty()) return;
    if(!path.endsWith(".pdf",Qt::CaseInsensitive)) path+=".pdf";
    QString error;
    if(!exportInventoryPdf(path,&error)) { QMessageBox::warning(this,"Export impossible",error); return; }
    notice->setText("Inventaire affiché exporté en PDF · démo");
}
