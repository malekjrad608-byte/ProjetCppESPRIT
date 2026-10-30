#include "stockwidget.h"
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

StockWidget::StockWidget(QWidget *parent) : QWidget(parent), tableModel(new QStandardItemModel(this)) {
    setupUi();

    // FORÇAGE DES COULEURS (Bech n-na7iw l-k7al mta3 l-mode sombre mta3 Windows)
    this->setObjectName("StockView");
    this->setStyleSheet(
        "QWidget#StockView { background-color: #F4F7F8; color: #263B42; font-family: 'Segoe UI'; }"
        "QLineEdit, QComboBox { background-color: #FFFFFF; color: #263B42; border: 1px solid #CBD5D8; padding: 4px; border-radius: 4px; }"
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
        "  background-color: #397F8D;"
        "  color: #FFFFFF;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "  padding: 6px;"
        "  border: 1px solid #225A64;"
        "}"
        );
}

void StockWidget::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // --- En-tête ---
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("<b>Stock</b><br><span style='font-size:12px; color:#397F8D;'>Inventaire, traçabilité des lots et réapprovisionnement</span>");
    titleLabel->setStyleSheet("font-size: 24px; color: #225A64; background: transparent;");

    QPushButton *btnMovement = new QPushButton("+ Enregistrer un mouvement");
    btnMovement->setStyleSheet("background-color: #225A64; color: white; padding: 10px 20px; border-radius: 6px; font-weight: bold; border: none;");
    connect(btnMovement, &QPushButton::clicked, this, &StockWidget::openMovementDialog);

    headerLayout->addWidget(titleLabel);
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
    searchLineEdit->setPlaceholderText("Rechercher un article (ex: Croquettes, Shampooing, Vaccin)...");
    searchLineEdit->setMinimumHeight(35);

    categoryFilter = new QComboBox();
    categoryFilter->addItems({"Toutes les catégories", "Chiens", "Chats", "Oiseaux", "Poissons", "Rongeurs", "Matériel Clinique"});
    categoryFilter->setMinimumHeight(35);

    QPushButton *btnExport = new QPushButton("Exporter l'inventaire");
    btnExport->setStyleSheet("background-color: #FFFFFF; color: #263B42; border: 1px solid #397F8D; padding: 5px 15px; border-radius: 4px; font-weight: bold;");
    btnExport->setMinimumHeight(35);

    toolbarLayout->addWidget(searchLineEdit, 1);
    toolbarLayout->addWidget(categoryFilter);
    toolbarLayout->addWidget(btnExport);
    mainLayout->addLayout(toolbarLayout);

    // --- Tableau de données ---
    stockTable = new QTableView();
    stockTable->setAlternatingRowColors(true);
    stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    stockTable->horizontalHeader()->setStretchLastSection(true);
    stockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    stockTable->verticalHeader()->setVisible(false); // Na7iw l-arkam l-zayda 3al isar

    // Configuration du modèle et des titres (3anawin mta3 l-jadwel)
    tableModel->setColumnCount(6);
    tableModel->setHorizontalHeaderLabels({"ARTICLE (Type / Animal)", "LOT", "DISPONIBLE", "SEUIL", "ÉCHÉANCE", "ÉTAT"});

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

    // Remplissage du tableau avec sécurité des couleurs
    for (const QStringList &rowData : fictiveData) {
        QList<QStandardItem *> rowItems;
        for (int i = 0; i < 6; ++i) {
            QStandardItem *item = new QStandardItem(rowData[i]);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Tableau lecture seule

            // Forcer l-couleur lel ktiba bech ma tetaffected b-mode sombre
            if (i == 5) {
                if (rowData[i] == "Sous le seuil") item->setForeground(QColor("#E8A13F"));
                else if (rowData[i] == "Échéance proche") item->setForeground(QColor("#AE5751"));
                else item->setForeground(QColor("#397F8D"));
            } else {
                item->setForeground(QColor("#263B42")); // Loun ktiba rmedi ghamedh
            }
            rowItems.append(item);
        }
        tableModel->appendRow(rowItems);
    }

    stockTable->setModel(tableModel);
    mainLayout->addWidget(stockTable, 1);

    // --- Panneaux des métiers innovants ---
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    setupActionPanels(actionsLayout);
    mainLayout->addLayout(actionsLayout);
}

void StockWidget::setupKpiCards(QHBoxLayout *layout) {
    QStringList titles = {"Articles référencés", "Sous le seuil", "Lots à contrôler"};
    QStringList values = {"19", "4", "2"}; // 19 produits en tout f-liste jdida
    QStringList subtitles = {"Produits actifs", "À réapprovisionner", "Échéances proches"};

    for (int i = 0; i < 3; ++i) {
        QFrame *card = new QFrame();
        card->setStyleSheet("background-color: #FFFFFF; border-radius: 8px; border: 1px solid #E4F0F3;");
        QVBoxLayout *cardLayout = new QVBoxLayout(card);

        QLabel *lblTitle = new QLabel(titles[i]);
        lblTitle->setStyleSheet("color: #397F8D; font-weight: bold; background: transparent;");
        QLabel *lblValue = new QLabel(values[i]);
        lblValue->setStyleSheet("font-size: 28px; color: #225A64; font-weight: bold; background: transparent;");
        QLabel *lblSub = new QLabel(subtitles[i]);
        lblSub->setStyleSheet("color: #7f8c8d; background: transparent;");

        cardLayout->addWidget(lblTitle);
        cardLayout->addWidget(lblValue);
        cardLayout->addWidget(lblSub);
        layout->addWidget(card);
    }
}

void StockWidget::setupActionPanels(QHBoxLayout *layout) {
    // FEFO
    QFrame *fefoPanel = new QFrame();
    fefoPanel->setStyleSheet("background-color: #FFFFFF; border-radius: 8px; border: 1px solid #AE5751;");
    QVBoxLayout *fefoLayout = new QVBoxLayout(fefoPanel);
    fefoLayout->addWidget(new QLabel("<b style='color: #AE5751; background: transparent;'>Sortie conseillée par péremption (FEFO)</b>"));
    fefoLayout->addWidget(new QLabel("<span style='color: #263B42; background: transparent;'>Complément Vitamines (Oiseaux) · lot BIRD-M-03\nÉchéance au 20/10/2026 · 1 flacon disponible</span>"));

    QPushButton *btnFefo = new QPushButton("Préparer la sortie");
    btnFefo->setStyleSheet("border: 1px solid #AE5751; color: #AE5751; border-radius: 4px; padding: 8px; background-color: #FBEBEA; font-weight: bold;");
    connect(btnFefo, &QPushButton::clicked, this, &StockWidget::prepareFefoCheckout);
    fefoLayout->addWidget(btnFefo);
    layout->addWidget(fefoPanel);

    // Réapprovisionnement
    QFrame *reorderPanel = new QFrame();
    reorderPanel->setStyleSheet("background-color: #FFFFFF; border-radius: 8px; border: 1px solid #E8A13F;");
    QVBoxLayout *reorderLayout = new QVBoxLayout(reorderPanel);
    reorderLayout->addWidget(new QLabel("<b style='color: #E8A13F; background: transparent;'>Proposition de réapprovisionnement</b>"));
    reorderLayout->addWidget(new QLabel("<span style='color: #263B42; background: transparent;'>Vaccin Rage/Parvovirose (Chien) · 15 doses conseillées\n3 doses/jour × 5 jours + 2 de sécurité − 2 disponibles</span>"));

    QPushButton *btnReorder = new QPushButton("Vérifier la proposition");
    btnReorder->setStyleSheet("border: 1px solid #E8A13F; color: #D68910; border-radius: 4px; padding: 8px; background-color: #FDF3E5; font-weight: bold;");
    connect(btnReorder, &QPushButton::clicked, this, &StockWidget::verifyReorder);
    reorderLayout->addWidget(btnReorder);
    layout->addWidget(reorderPanel);
}

void StockWidget::openMovementDialog() { qDebug() << "Action : Ouverture de la fenêtre de mouvement de stock."; }
void StockWidget::prepareFefoCheckout() { qDebug() << "Action : Règle FEFO STK-I1 déclenchée pour le lot BIRD-M-03."; }
void StockWidget::verifyReorder() { qDebug() << "Action : Calcul de réapprovisionnement STK-I2 déclenché."; }