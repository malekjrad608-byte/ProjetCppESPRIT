#include "gestionemployes.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QTableWidget>
#include <QHeaderView>
#include <QProgressBar>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QInputDialog>
#include <QColor>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>

// ==================== Constructeur ====================
GestionEmployes::GestionEmployes(QWidget *parent) : QWidget(parent)
{
    construireUI();
    chargerEmployes();
}

// ==================== UI PRINCIPALE ====================
void GestionEmployes::construireUI()
{
    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(30, 25, 30, 25);
    main->setSpacing(18);

    main->addWidget(construireHeader());
    main->addWidget(construireStatsPills());
    main->addWidget(construireBarreRecherche());

    QHBoxLayout *corps = new QHBoxLayout;
    corps->setSpacing(20);
    corps->addWidget(construireGrid(), 3);
    corps->addWidget(construireDetailPanel(), 1);
    main->addLayout(corps, 1);
}

// ==================== HEADER ====================
QWidget *GestionEmployes::construireHeader()
{
    QWidget *w = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *titreL = new QVBoxLayout;
    titreL->setSpacing(4);

    QLabel *titre = new QLabel("👥  Employés");
    titre->setStyleSheet("font-size: 28px; font-weight: bold; color: #225A64;"
                         "background: transparent;");
    titreL->addWidget(titre);

    QLabel *sous = new QLabel("Annuaire, disponibilités et gestion de la charge");
    sous->setStyleSheet("font-size: 13px; color: #7A8B8F; background: transparent;");
    titreL->addWidget(sous);

    layout->addLayout(titreL);
    layout->addStretch();

    QPushButton *btnNouveau = new QPushButton("+  Ajouter un employé");
    btnNouveau->setObjectName("btnPrimary");
    btnNouveau->setMinimumHeight(44);
    btnNouveau->setMinimumWidth(210);
    btnNouveau->setCursor(Qt::PointingHandCursor);
    connect(btnNouveau, &QPushButton::clicked, this, &GestionEmployes::ajouterEmploye);
    layout->addWidget(btnNouveau);

    return w;
}

// ==================== STATS PILLS (bla animation compteurs) ====================
QWidget *GestionEmployes::construireStatsPills()
{
    QWidget *w = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    // ✅ Bounce-in helper (ken 3la el pills — khallih)
    auto bounceIn = [](QWidget *wid, int delayMs) {
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(wid);
        wid->setGraphicsEffect(eff);
        eff->setOpacity(0);

        QTimer::singleShot(delayMs, [wid, eff]() {
            QPropertyAnimation *a1 = new QPropertyAnimation(eff, "opacity");
            a1->setDuration(400);
            a1->setStartValue(0.0);
            a1->setEndValue(1.0);
            a1->setEasingCurve(QEasingCurve::OutCubic);
            a1->start(QAbstractAnimation::DeleteWhenStopped);
        });
    };

    auto creerPill = [&](const QString &icone, const QString &bg, const QString &fg,
                         QLabel *&valeur, const QString &texte, int idx) {
        QFrame *pill = new QFrame;
        pill->setStyleSheet(QString(
                                "QFrame { background-color: %1; border-radius: 22px; border: none; }")
                                .arg(bg));
        pill->setFixedHeight(44);

        QHBoxLayout *l = new QHBoxLayout(pill);
        l->setContentsMargins(18, 5, 22, 5);
        l->setSpacing(8);

        QLabel *ico = new QLabel(icone);
        ico->setStyleSheet("font-size: 15px; background: transparent; border: none;");
        l->addWidget(ico);

        valeur = new QLabel("0");
        valeur->setStyleSheet(QString(
                                  "font-size: 16px; font-weight: bold; color: %1; "
                                  "background: transparent; border: none;").arg(fg));
        l->addWidget(valeur);

        QLabel *t = new QLabel(texte);
        t->setStyleSheet(QString(
                             "font-size: 12px; color: %1; background: transparent; border: none;")
                             .arg(fg));
        l->addWidget(t);

        bounceIn(pill, idx * 150);
        return pill;
    };

    QFrame *p1 = creerPill("✅", "#E6F4EA", "#2E7D32", m_lblDisponibles, "disponibles", 0);
    QFrame *p2 = creerPill("❌", "#FBE9E7", "#AE5751", m_lblIndisponibles, "indisponibles", 1);
    QFrame *p3 = creerPill("⏱", "#FDF3E3", "#B8741A", m_lblChargeTotale, "heures planifiées", 2);

    layout->addWidget(p1);
    layout->addWidget(p2);
    layout->addWidget(p3);

    layout->addStretch();

    // ✅ Bouton stats (m3a pulse animation)
    QPushButton *btnStats = new QPushButton("📊  Voir les statistiques");
    btnStats->setObjectName("btnSecondary");
    btnStats->setMinimumHeight(44);
    btnStats->setCursor(Qt::PointingHandCursor);

    QGraphicsOpacityEffect *btnEff = new QGraphicsOpacityEffect(btnStats);
    btnStats->setGraphicsEffect(btnEff);
    btnEff->setOpacity(1.0);

    QPropertyAnimation *pulseBtn = new QPropertyAnimation(btnEff, "opacity");
    pulseBtn->setDuration(1800);
    pulseBtn->setStartValue(1.0);
    pulseBtn->setKeyValueAt(0.5, 0.75);
    pulseBtn->setEndValue(1.0);
    pulseBtn->setLoopCount(-1);
    pulseBtn->setEasingCurve(QEasingCurve::InOutSine);
    pulseBtn->start();

    connect(btnStats, &QPushButton::clicked, this, [this]() {
        QDialog dlg(this);
        dlg.setWindowTitle("Statistiques des employés");
        dlg.setMinimumWidth(820);
        dlg.setMinimumHeight(650);
        dlg.setStyleSheet("QDialog { background-color: #F0F4F8; }");

        QVBoxLayout *mainL = new QVBoxLayout(&dlg);
        mainL->setContentsMargins(25, 25, 25, 25);
        mainL->setSpacing(15);

        QLabel *titre = new QLabel("📊  Statistiques des employés");
        titre->setStyleSheet("font-size: 22px; font-weight: bold; color: #225A64;"
                             "background: transparent;");
        mainL->addWidget(titre);

        QLabel *sous = new QLabel("Charge hebdomadaire et répartition par poste");
        sous->setStyleSheet("font-size: 13px; color: #7A8B8F; background: transparent;");
        mainL->addWidget(sous);

        // CARDS RÉSUMÉ
        QHBoxLayout *cardsL = new QHBoxLayout;
        cardsL->setSpacing(12);

        QSqlQuery qStats;
        qStats.exec("SELECT COUNT(*), SUM(charge_hebdo), AVG(charge_hebdo), "
                    "MAX(charge_hebdo), MIN(charge_hebdo) FROM employes");
        int totalEmp = 0;
        double totalCharge = 0, avgCharge = 0, maxCharge = 0, minCharge = 0;
        if (qStats.next()) {
            totalEmp = qStats.value(0).toInt();
            totalCharge = qStats.value(1).toDouble();
            avgCharge = qStats.value(2).toDouble();
            maxCharge = qStats.value(3).toDouble();
            minCharge = qStats.value(4).toDouble();
        }

        auto creerCard = [](const QString &icone, const QString &bg,
                            const QString &fg, const QString &titreCard,
                            const QString &valeur, const QString &sousTitre) {
            QFrame *card = new QFrame;
            card->setStyleSheet(QString(
                                    "QFrame { background-color: %1; border-radius: 14px; }").arg(bg));
            card->setMinimumHeight(90);

            QHBoxLayout *l = new QHBoxLayout(card);
            l->setContentsMargins(15, 12, 15, 12);
            l->setSpacing(12);

            QLabel *ico = new QLabel(icone);
            ico->setStyleSheet("font-size: 28px; background: transparent; border: none;");
            l->addWidget(ico);

            QVBoxLayout *textL = new QVBoxLayout;
            textL->setSpacing(2);

            QLabel *t = new QLabel(titreCard);
            t->setStyleSheet(QString(
                                 "font-size: 11px; color: %1; background: transparent;"
                                 "font-weight: 500; border: none;").arg(fg));
            textL->addWidget(t);

            QLabel *v = new QLabel(valeur);
            v->setStyleSheet(QString(
                                 "font-size: 20px; font-weight: bold; color: %1;"
                                 "background: transparent; border: none;").arg(fg));
            textL->addWidget(v);

            QLabel *s = new QLabel(sousTitre);
            s->setStyleSheet(QString(
                                 "font-size: 10px; color: %1; background: transparent; border: none;")
                                 .arg(fg));
            textL->addWidget(s);

            l->addLayout(textL);
            l->addStretch();
            return card;
        };

        cardsL->addWidget(creerCard("👥", "#E4F0F3", "#225A64",
                                    "Total", QString::number(totalEmp), "employés"));
        cardsL->addWidget(creerCard("⏱", "#FDF3E3", "#B8741A",
                                    "Charge totale", QString::number(totalCharge, 'f', 0) + " h", "par semaine"));
        cardsL->addWidget(creerCard("📈", "#E6F4EA", "#2E7D32",
                                    "Charge moyenne", QString::number(avgCharge, 'f', 1) + " h", "par employé"));
        cardsL->addWidget(creerCard("⚠️", "#FBE9E7", "#AE5751",
                                    "Charge max", QString::number(maxCharge, 'f', 0) + " h", "le plus chargé"));

        mainL->addLayout(cardsL);
        mainL->addSpacing(5);

        QLabel *titreTable = new QLabel("📋  Charge par employé");
        titreTable->setStyleSheet("font-size: 16px; font-weight: bold; color: #225A64;"
                                  "background: transparent;");
        mainL->addWidget(titreTable);

        QTableWidget *table = new QTableWidget;
        table->setColumnCount(5);
        table->setHorizontalHeaderLabels(
            {"Employé", "Poste", "Charge (h)", "Dispo (h)", "Taux de charge"});
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setAlternatingRowColors(true);
        table->verticalHeader()->setVisible(false);
        table->verticalHeader()->setDefaultSectionSize(48);

        table->horizontalHeader()->setMinimumHeight(45);
        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

        QSqlQuery q;
        q.exec("SELECT nom, prenom, poste, charge_hebdo, dispo_hebdo "
               "FROM employes ORDER BY charge_hebdo DESC");

        int row = 0;
        while (q.next()) {
            table->insertRow(row);

            QString nom = q.value(1).toString() + " " + q.value(0).toString();
            QString poste = q.value(2).toString();
            double charge = q.value(3).toDouble();
            double dispo = q.value(4).toDouble();
            double taux = dispo > 0 ? (charge / dispo * 100.0) : 0;

            QTableWidgetItem *nomItem = new QTableWidgetItem(nom);
            nomItem->setForeground(QColor("#225A64"));
            QFont f = nomItem->font();
            f.setBold(true);
            nomItem->setFont(f);
            table->setItem(row, 0, nomItem);

            table->setItem(row, 1, new QTableWidgetItem(poste));

            QTableWidgetItem *chargeItem = new QTableWidgetItem(
                QString::number(charge, 'f', 1) + " h");
            chargeItem->setTextAlignment(Qt::AlignCenter);
            table->setItem(row, 2, chargeItem);

            QTableWidgetItem *dispoItem = new QTableWidgetItem(
                QString::number(dispo, 'f', 1) + " h");
            dispoItem->setTextAlignment(Qt::AlignCenter);
            dispoItem->setForeground(QColor("#7A8B8F"));
            table->setItem(row, 3, dispoItem);

            QWidget *tauxWidget = new QWidget;
            QHBoxLayout *tauxL = new QHBoxLayout(tauxWidget);
            tauxL->setContentsMargins(8, 4, 8, 4);
            tauxL->setSpacing(8);

            QString couleur;
            if (taux > 80) couleur = "#AE5751";
            else if (taux > 60) couleur = "#E8A13F";
            else couleur = "#4A9E5C";

            QProgressBar *bar = new QProgressBar;
            bar->setRange(0, 100);
            bar->setValue(qRound(taux));
            bar->setTextVisible(false);
            bar->setFixedHeight(8);
            bar->setStyleSheet(QString(
                                   "QProgressBar { background-color: #F0F4F8; border-radius: 4px; border: none; }"
                                   "QProgressBar::chunk { background-color: %1; border-radius: 4px; }")
                                   .arg(couleur));
            tauxL->addWidget(bar, 1);

            QLabel *tauxLabel = new QLabel(QString::number(qRound(taux)) + " %");
            tauxLabel->setStyleSheet(QString(
                                         "font-size: 12px; font-weight: bold; color: %1;"
                                         "background: transparent; min-width: 45px; border: none;").arg(couleur));
            tauxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            tauxL->addWidget(tauxLabel);

            table->setCellWidget(row, 4, tauxWidget);
            row++;
        }

        mainL->addWidget(table, 1);

        QFrame *basCard = new QFrame;
        basCard->setStyleSheet("QFrame { background-color: #E4F0F3;"
                               "border-radius: 12px; border: none; }");
        QHBoxLayout *basL = new QHBoxLayout(basCard);
        basL->setContentsMargins(20, 14, 20, 14);

        QLabel *basText = new QLabel(QString(
                                         "⏱  <b>%1 heures</b> planifiées au total / semaine   ·   "
                                         "📊  Moyenne : <b>%2 h</b> par employé   ·   "
                                         "🏆  Max : <b>%3 h</b>   ·   "
                                         "📉  Min : <b>%4 h</b>")
                                         .arg(QString::number(totalCharge, 'f', 0))
                                         .arg(QString::number(avgCharge, 'f', 1))
                                         .arg(QString::number(maxCharge, 'f', 0))
                                         .arg(QString::number(minCharge, 'f', 0)));
        basText->setTextFormat(Qt::RichText);
        basText->setStyleSheet("font-size: 13px; color: #225A64;"
                               "background: transparent; border: none;");
        basL->addWidget(basText);
        basL->addStretch();

        mainL->addWidget(basCard);

        QDialogButtonBox *btns = new QDialogButtonBox(QDialogButtonBox::Close);
        btns->button(QDialogButtonBox::Close)->setText("Fermer");
        btns->button(QDialogButtonBox::Close)->setMinimumHeight(42);
        btns->button(QDialogButtonBox::Close)->setMinimumWidth(120);
        connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        mainL->addWidget(btns);

        dlg.exec();
    });
    layout->addWidget(btnStats);

    // ✅ MA7INA el animated counters — les valeurs ybanou direct
    // (Mise à jour directe men mettreAJourStats())

    return w;
}

// ==================== BARRE RECHERCHE ====================
QWidget *GestionEmployes::construireBarreRecherche()
{
    QWidget *w = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    m_editRecherche = new QLineEdit;
    m_editRecherche->setObjectName("searchField");
    m_editRecherche->setPlaceholderText("🔍  Rechercher par nom, ID ou poste...");
    m_editRecherche->setMinimumHeight(46);
    connect(m_editRecherche, &QLineEdit::textChanged,
            this, &GestionEmployes::onFiltresChange);
    layout->addWidget(m_editRecherche, 3);

    m_comboPoste = new QComboBox;
    m_comboPoste->setMinimumHeight(46);
    m_comboPoste->addItems({"Tous les postes", "Vétérinaire",
                            "Assistant(e) vétérinaire", "Auxiliaire vétérinaire",
                            "Secrétaire / Réceptionniste", "Agent d'entretien",
                            "Aide aux soins animaliers"});
    connect(m_comboPoste, &QComboBox::currentTextChanged,
            this, &GestionEmployes::onFiltresChange);
    layout->addWidget(m_comboPoste, 1);

    m_comboStatut = new QComboBox;
    m_comboStatut->setMinimumHeight(46);
    m_comboStatut->addItems({"Tous", "Disponible", "Non disponible"});
    connect(m_comboStatut, &QComboBox::currentTextChanged,
            this, &GestionEmployes::onFiltresChange);
    layout->addWidget(m_comboStatut, 1);

    m_comboTri = new QComboBox;
    m_comboTri->setMinimumHeight(46);
    m_comboTri->addItems({"Tri par défaut", "Tri par nom", "Tri par poste",
                          "Tri par date", "Tri par charge"});
    connect(m_comboTri, &QComboBox::currentTextChanged,
            this, &GestionEmployes::onFiltresChange);
    layout->addWidget(m_comboTri, 1);

    return w;
}

// ==================== GRID ====================
QWidget *GestionEmployes::construireGrid()
{
    m_cardsContainer = new QWidget;
    m_gridCards = new QGridLayout(m_cardsContainer);
    m_gridCards->setContentsMargins(0, 0, 0, 0);
    m_gridCards->setSpacing(18);
    m_gridCards->setAlignment(Qt::AlignTop);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(m_cardsContainer);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }"
                          "QScrollArea > QWidget > QWidget { background: transparent; }");
    return scroll;
}

// ==================== CARD ====================
QWidget *GestionEmployes::creerCard(const Employe &e)
{
    QFrame *card = new QFrame;
    bool selectionne = (e.getId() == m_idSelectionne);
    card->setObjectName(selectionne ? "cardEmployeSelected" : "cardEmploye");
    card->setCursor(Qt::PointingHandCursor);
    card->setMinimumHeight(170);
    card->setProperty("employe_id", e.getId());
    card->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(10);

    QHBoxLayout *topRow = new QHBoxLayout;
    topRow->setSpacing(6);
    topRow->addStretch();

    QLabel *badge = new QLabel(e.isDisponible() ? "✅ Disponible" : "❌ Indisponible");
    if (e.isDisponible())
        badge->setStyleSheet("font-size: 10px; color: #2E7D32; background-color: #E6F4EA;"
                             "padding: 5px 12px; border-radius: 12px; font-weight: bold;"
                             "border: none;");
    else
        badge->setStyleSheet("font-size: 10px; color: #AE5751; background-color: #FBE9E7;"
                             "padding: 5px 12px; border-radius: 12px; font-weight: bold;"
                             "border: none;");
    topRow->addWidget(badge);

    layout->addLayout(topRow);

    QLabel *nom = new QLabel(e.getPrenom() + " " + e.getNom());
    nom->setStyleSheet("font-size: 18px; font-weight: bold; color: #225A64;"
                       "background: transparent;");
    layout->addWidget(nom);

    QLabel *poste = new QLabel(e.getPoste());
    poste->setStyleSheet("font-size: 12px; color: #7A8B8F; background: transparent;");
    layout->addWidget(poste);

    layout->addStretch();

    QHBoxLayout *botRow = new QHBoxLayout;
    botRow->setSpacing(10);

    QLabel *charge = new QLabel(QString("⏱ %1 h").arg(e.getChargeHebdo(), 0, 'f', 0));
    charge->setStyleSheet("font-size: 11px; color: #263B42; font-weight: 500;"
                          "background: transparent;");
    botRow->addWidget(charge);

    botRow->addStretch();

    if (!e.getCompetences().isEmpty()) {
        QString comp = e.getCompetences();
        if (comp.length() > 22) comp = comp.left(22) + "...";
        QLabel *compLbl = new QLabel("🏷 " + comp);
        compLbl->setStyleSheet("font-size: 10px; color: #7A8B8F; background: transparent;");
        botRow->addWidget(compLbl);
    }

    layout->addLayout(botRow);

    return card;
}

// ==================== DETAIL PANEL ====================
QWidget *GestionEmployes::construireDetailPanel()
{
    QFrame *card = new QFrame;
    card->setObjectName("card");
    card->setMinimumWidth(360);
    card->setMaximumWidth(420);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QLabel *titre = new QLabel("EMPLOYÉ SÉLECTIONNÉ");
    titre->setStyleSheet("font-size: 11px; font-weight: bold; color: #7A8B8F;"
                         "letter-spacing: 1px; padding: 22px 25px 8px 25px;"
                         "background: transparent;");
    layout->addWidget(titre);

    m_detailPlaceholder = new QLabel("👈\n\nSélectionnez un employé\npour voir ses détails");
    m_detailPlaceholder->setAlignment(Qt::AlignCenter);
    m_detailPlaceholder->setStyleSheet("font-size: 13px; color: #7A8B8F; padding: 80px 20px;"
                                       "background: transparent;");
    layout->addWidget(m_detailPlaceholder);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    m_detailContent = new QWidget;
    m_detailContent->setObjectName("detailContent");
    m_detailContent->setStyleSheet("QWidget#detailContent { background: transparent; }");

    QVBoxLayout *contentL = new QVBoxLayout(m_detailContent);
    contentL->setContentsMargins(25, 5, 25, 25);
    contentL->setSpacing(10);

    m_detailNom = new QLabel;
    m_detailNom->setStyleSheet("font-size: 22px; font-weight: bold; color: #225A64;"
                               "background: transparent;");
    m_detailNom->setWordWrap(true);
    contentL->addWidget(m_detailNom);

    m_detailId = new QLabel;
    m_detailId->setStyleSheet("font-size: 12px; color: #7A8B8F; background: transparent;");
    contentL->addWidget(m_detailId);

    m_detailStatut = new QLabel;
    m_detailStatut->setAlignment(Qt::AlignCenter);
    contentL->addWidget(m_detailStatut);

    contentL->addSpacing(8);

    QFrame *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color: #E4F0F3; max-height: 1px; border: none;");
    contentL->addWidget(sep);

    contentL->addSpacing(6);

    auto creerRow = [](const QString &label, QLabel *&valeur) {
        QHBoxLayout *row = new QHBoxLayout;
        row->setSpacing(8);
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 12px; color: #7A8B8F; background: transparent;");
        lbl->setFixedWidth(95);
        row->addWidget(lbl);
        valeur = new QLabel("-");
        valeur->setStyleSheet("font-size: 12px; color: #263B42; font-weight: 500;"
                              "background: transparent;");
        valeur->setWordWrap(true);
        row->addWidget(valeur, 1);
        return row;
    };

    contentL->addLayout(creerRow("Poste", m_detailPoste));
    contentL->addLayout(creerRow("Courriel", m_detailCourriel));
    contentL->addLayout(creerRow("Téléphone", m_detailTelephone));
    contentL->addLayout(creerRow("Embauche", m_detailDate));
    contentL->addLayout(creerRow("Charge", m_detailCharge));
    contentL->addLayout(creerRow("Dispo", m_detailDispo));
    contentL->addLayout(creerRow("Compétences", m_detailCompetences));

    contentL->addSpacing(20);

    QPushButton *btnModifier = new QPushButton("✏️  Modifier");
    btnModifier->setObjectName("btnPrimary");
    btnModifier->setMinimumHeight(46);
    btnModifier->setCursor(Qt::PointingHandCursor);
    connect(btnModifier, &QPushButton::clicked, this, &GestionEmployes::modifierEmploye);
    contentL->addWidget(btnModifier);

    QPushButton *btnBasculer = new QPushButton("🔄  Basculer disponibilité");
    btnBasculer->setObjectName("btnSuccess");
    btnBasculer->setMinimumHeight(46);
    btnBasculer->setCursor(Qt::PointingHandCursor);
    connect(btnBasculer, &QPushButton::clicked, this, &GestionEmployes::basculerDisponibilite);
    contentL->addWidget(btnBasculer);

    QPushButton *btnSupprimer = new QPushButton("🗑️  Supprimer");
    btnSupprimer->setObjectName("btnDanger");
    btnSupprimer->setMinimumHeight(46);
    btnSupprimer->setCursor(Qt::PointingHandCursor);
    connect(btnSupprimer, &QPushButton::clicked, this, &GestionEmployes::supprimerEmploye);
    contentL->addWidget(btnSupprimer);

    contentL->addStretch();

    scroll->setWidget(m_detailContent);
    layout->addWidget(scroll, 1);

    m_detailContent->hide();

    return card;
}

// ==================== CHARGEMENT ====================
void GestionEmployes::chargerEmployes(const QString &filtre, const QString &tri,
                                      const QString &poste, const QString &statut)
{
    QString orderBy = "prenom ASC, nom ASC";
    if (tri == "nom") orderBy = "prenom ASC, nom ASC";
    else if (tri == "poste") orderBy = "poste ASC, prenom ASC";
    else if (tri == "date") orderBy = "date_embauche DESC, prenom ASC";
    else if (tri == "charge") orderBy = "charge_hebdo DESC, prenom ASC";

    QString sql = "SELECT id, nom, prenom, courriel, telephone, poste, "
                  "date_embauche, disponible, charge_hebdo, dispo_hebdo, "
                  "competences FROM employes WHERE 1=1";

    if (!filtre.isEmpty())
        sql += " AND (nom LIKE :f OR prenom LIKE :f OR courriel LIKE :f "
               "OR poste LIKE :f OR CAST(id AS TEXT) LIKE :f)";
    if (poste != "Tous les postes")
        sql += " AND poste = :poste";
    if (statut == "Disponible")
        sql += " AND disponible = 1";
    else if (statut == "Non disponible")
        sql += " AND disponible = 0";

    sql += " ORDER BY " + orderBy;

    QSqlQuery q;
    q.prepare(sql);
    if (!filtre.isEmpty()) q.bindValue(":f", "%" + filtre + "%");
    if (poste != "Tous les postes") q.bindValue(":poste", poste);

    QList<Employe> liste;
    if (q.exec()) {
        while (q.next()) {
            Employe e;
            e.setId(q.value(0).toInt());
            e.setNom(q.value(1).toString());
            e.setPrenom(q.value(2).toString());
            e.setCourriel(q.value(3).toString());
            e.setTelephone(q.value(4).toString());
            e.setPoste(q.value(5).toString());
            e.setDateEmbauche(QDate::fromString(q.value(6).toString(), "yyyy-MM-dd"));
            e.setDisponible(q.value(7).toInt() == 1);
            e.setChargeHebdo(q.value(8).toDouble());
            e.setDispoHebdo(q.value(9).toDouble());
            e.setCompetences(q.value(10).toString());
            liste.append(e);
        }
    }

    afficherCards(liste);
    mettreAJourStats();
}

// ==================== AFFICHER CARDS ====================
void GestionEmployes::afficherCards(const QList<Employe> &liste)
{
    QLayoutItem *item;
    while ((item = m_gridCards->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    int row = 0, col = 0;
    const int COLS = 2;

    for (const Employe &e : liste) {
        QWidget *card = creerCard(e);
        m_gridCards->addWidget(card, row, col);

        col++;
        if (col >= COLS) { col = 0; row++; }
    }
}

// ==================== AFFICHER DETAIL ====================
void GestionEmployes::afficherDetail(const Employe &e)
{
    m_detailPlaceholder->hide();
    m_detailContent->show();

    m_detailNom->setText(e.getPrenom() + " " + e.getNom());
    m_detailId->setText(QString("ID Employé #%1").arg(e.getId()));

    if (e.isDisponible()) {
        m_detailStatut->setText("✅  Disponible");
        m_detailStatut->setStyleSheet(
            "font-size: 12px; color: #2E7D32; background-color: #E6F4EA;"
            "padding: 8px; border-radius: 8px; font-weight: bold; border: none;");
    } else {
        m_detailStatut->setText("❌  Non disponible");
        m_detailStatut->setStyleSheet(
            "font-size: 12px; color: #AE5751; background-color: #FBE9E7;"
            "padding: 8px; border-radius: 8px; font-weight: bold; border: none;");
    }

    m_detailPoste->setText(e.getPoste());
    m_detailCourriel->setText(e.getCourriel().isEmpty() ? "-" : e.getCourriel());
    m_detailTelephone->setText(e.getTelephone().isEmpty() ? "-" : e.getTelephone());
    m_detailDate->setText(e.getDateEmbauche().toString("dd/MM/yyyy"));

    double taux = e.tauxCharge() * 100;
    m_detailCharge->setText(QString("%1 h (%2%)")
                                .arg(e.getChargeHebdo(), 0, 'f', 0).arg(qRound(taux)));
    m_detailDispo->setText(QString("%1 h").arg(e.getDispoHebdo(), 0, 'f', 0));
    m_detailCompetences->setText(e.getCompetences().isEmpty() ? "-" : e.getCompetences());
}

void GestionEmployes::effacerDetail()
{
    m_idSelectionne = -1;
    m_detailContent->hide();
    m_detailPlaceholder->show();
}

// ==================== STATS ====================
void GestionEmployes::mettreAJourStats()
{
    QSqlQuery q;
    q.exec("SELECT COUNT(*) FROM employes WHERE disponible=1");
    if (q.next()) m_lblDisponibles->setText(q.value(0).toString());

    q.exec("SELECT COUNT(*) FROM employes WHERE disponible=0");
    if (q.next()) m_lblIndisponibles->setText(q.value(0).toString());

    q.exec("SELECT SUM(charge_hebdo) FROM employes WHERE disponible=1");
    if (q.next()) {
        double total = q.value(0).toDouble();
        m_lblChargeTotale->setText(QString::number(qRound(total)));
    }
}

// ==================== FILTRES ====================
void GestionEmployes::onFiltresChange()
{
    QString filtre = m_editRecherche->text().trimmed();
    QString poste = m_comboPoste->currentText();
    QString statut = m_comboStatut->currentText();
    QString tri = "nom";
    int idx = m_comboTri->currentIndex();
    if (idx == 1) tri = "nom";
    else if (idx == 2) tri = "poste";
    else if (idx == 3) tri = "date";
    else if (idx == 4) tri = "charge";

    chargerEmployes(filtre, tri, poste, statut);
}

// ==================== EVENT FILTER (Hover + Click) ====================
bool GestionEmployes::eventFilter(QObject *obj, QEvent *event)
{
    QWidget *card = qobject_cast<QWidget*>(obj);
    if (!card || card->property("employe_id").toInt() <= 0)
        return QWidget::eventFilter(obj, event);

    if (event->type() == QEvent::Enter) {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
        shadow->setBlurRadius(0);
        shadow->setColor(QColor(57, 127, 141, 150));
        shadow->setOffset(0, 6);
        card->setGraphicsEffect(shadow);

        QPropertyAnimation *anim = new QPropertyAnimation(shadow, "blurRadius");
        anim->setDuration(300);
        anim->setStartValue(0.0);
        anim->setEndValue(30.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start(QPropertyAnimation::DeleteWhenStopped);
        return true;
    }

    if (event->type() == QEvent::Leave) {
        QGraphicsDropShadowEffect *shadow =
            qobject_cast<QGraphicsDropShadowEffect*>(card->graphicsEffect());
        if (shadow) {
            QPropertyAnimation *anim = new QPropertyAnimation(shadow, "blurRadius");
            anim->setDuration(300);
            anim->setStartValue(shadow->blurRadius());
            anim->setEndValue(0.0);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, &QPropertyAnimation::finished, [card]() {
                card->setGraphicsEffect(nullptr);
            });
            anim->start(QPropertyAnimation::DeleteWhenStopped);
        }
        return true;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        int id = card->property("employe_id").toInt();
        if (id > 0) {
            m_idSelectionne = id;
            onFiltresChange();

            QSqlQuery q;
            q.prepare("SELECT id, nom, prenom, courriel, telephone, poste, "
                      "date_embauche, disponible, charge_hebdo, dispo_hebdo, "
                      "competences FROM employes WHERE id=:id");
            q.bindValue(":id", id);
            if (q.exec() && q.next()) {
                Employe emp;
                emp.setId(q.value(0).toInt());
                emp.setNom(q.value(1).toString());
                emp.setPrenom(q.value(2).toString());
                emp.setCourriel(q.value(3).toString());
                emp.setTelephone(q.value(4).toString());
                emp.setPoste(q.value(5).toString());
                emp.setDateEmbauche(QDate::fromString(q.value(6).toString(), "yyyy-MM-dd"));
                emp.setDisponible(q.value(7).toInt() == 1);
                emp.setChargeHebdo(q.value(8).toDouble());
                emp.setDispoHebdo(q.value(9).toDouble());
                emp.setCompetences(q.value(10).toString());
                afficherDetail(emp);
            }
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

// ==================== DIALOG FORMULAIRE ====================
bool GestionEmployes::ouvrirDialog(Employe &e, bool modification)
{
    QDialog dlg(this);
    dlg.setWindowTitle(modification ? "Modifier l'employé" : "Nouvel employé");
    dlg.setMinimumWidth(520);
    dlg.setStyleSheet("QDialog { background-color: #F0F4F8; }");

    QVBoxLayout *mainL = new QVBoxLayout(&dlg);
    mainL->setContentsMargins(25, 25, 25, 25);
    mainL->setSpacing(15);

    QLabel *titre = new QLabel(modification ? "✏️  Modifier l'employé" : "➕  Nouvel employé");
    titre->setStyleSheet("font-size: 20px; font-weight: bold; color: #225A64;"
                         "background: transparent;");
    mainL->addWidget(titre);

    QFormLayout *form = new QFormLayout;
    form->setSpacing(12);

    QLineEdit *editNom = new QLineEdit(e.getNom());
    editNom->setMinimumHeight(40);
    editNom->setPlaceholderText("Nom *");
    form->addRow("Nom *", editNom);

    QLineEdit *editPrenom = new QLineEdit(e.getPrenom());
    editPrenom->setMinimumHeight(40);
    editPrenom->setPlaceholderText("Prénom *");
    form->addRow("Prénom *", editPrenom);

    QLineEdit *editCourriel = new QLineEdit(e.getCourriel());
    editCourriel->setMinimumHeight(40);
    form->addRow("Courriel", editCourriel);

    QLineEdit *editTel = new QLineEdit(e.getTelephone());
    editTel->setMinimumHeight(40);
    form->addRow("Téléphone", editTel);

    QComboBox *comboPoste = new QComboBox;
    comboPoste->setMinimumHeight(40);
    comboPoste->addItems({"Vétérinaire", "Assistant(e) vétérinaire",
                          "Auxiliaire vétérinaire", "Secrétaire / Réceptionniste",
                          "Agent d'entretien", "Aide aux soins animaliers"});
    if (!e.getPoste().isEmpty()) comboPoste->setCurrentText(e.getPoste());
    form->addRow("Poste", comboPoste);

    QDateEdit *dateEmb = new QDateEdit(e.getDateEmbauche().isValid() ?
                                           e.getDateEmbauche() : QDate::currentDate());
    dateEmb->setMinimumHeight(40);
    dateEmb->setCalendarPopup(true);
    dateEmb->setDisplayFormat("dd/MM/yyyy");
    form->addRow("Embauche", dateEmb);

    QDoubleSpinBox *spinCharge = new QDoubleSpinBox;
    spinCharge->setMinimumHeight(40);
    spinCharge->setRange(0, 60);
    spinCharge->setSuffix(" h");
    spinCharge->setValue(e.getChargeHebdo());
    form->addRow("Charge hebdo", spinCharge);

    QDoubleSpinBox *spinDispo = new QDoubleSpinBox;
    spinDispo->setMinimumHeight(40);
    spinDispo->setRange(0, 60);
    spinDispo->setSuffix(" h");
    spinDispo->setValue(e.getDispoHebdo() > 0 ? e.getDispoHebdo() : 35);
    form->addRow("Dispo hebdo", spinDispo);

    QLineEdit *editComp = new QLineEdit(e.getCompetences());
    editComp->setMinimumHeight(40);
    editComp->setPlaceholderText("Soins, Toilettage...");
    form->addRow("Compétences", editComp);

    QCheckBox *checkDispo = new QCheckBox("Disponible");
    checkDispo->setChecked(modification ? e.isDisponible() : true);
    form->addRow("", checkDispo);

    mainL->addLayout(form);
    mainL->addSpacing(10);

    QDialogButtonBox *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btns->button(QDialogButtonBox::Ok)->setText(modification ? "Modifier" : "Ajouter");
    btns->button(QDialogButtonBox::Ok)->setMinimumHeight(44);
    btns->button(QDialogButtonBox::Ok)->setMinimumWidth(120);
    btns->button(QDialogButtonBox::Cancel)->setMinimumHeight(44);
    btns->button(QDialogButtonBox::Cancel)->setMinimumWidth(120);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    mainL->addWidget(btns);

    if (dlg.exec() != QDialog::Accepted) return false;

    if (editNom->text().trimmed().isEmpty() ||
        editPrenom->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Nom et prénom sont obligatoires");
        return false;
    }

    QSqlQuery q;
    if (modification) {
        q.prepare("UPDATE employes SET nom=:nom, prenom=:prenom, courriel=:courriel, "
                  "telephone=:tel, poste=:poste, date_embauche=:date, disponible=:dispo, "
                  "charge_hebdo=:ch, dispo_hebdo=:dh, competences=:comp WHERE id=:id");
        q.bindValue(":id", e.getId());
    } else {
        q.prepare("INSERT INTO employes (nom, prenom, courriel, telephone, poste, "
                  "date_embauche, disponible, charge_hebdo, dispo_hebdo, competences) "
                  "VALUES (:nom, :prenom, :courriel, :tel, :poste, :date, :dispo, "
                  ":ch, :dh, :comp)");
    }

    q.bindValue(":nom", editNom->text().trimmed());
    q.bindValue(":prenom", editPrenom->text().trimmed());
    q.bindValue(":courriel", editCourriel->text().trimmed());
    q.bindValue(":tel", editTel->text().trimmed());
    q.bindValue(":poste", comboPoste->currentText());
    q.bindValue(":date", dateEmb->date().toString("yyyy-MM-dd"));
    q.bindValue(":dispo", checkDispo->isChecked() ? 1 : 0);
    q.bindValue(":ch", spinCharge->value());
    q.bindValue(":dh", spinDispo->value());
    q.bindValue(":comp", editComp->text().trimmed());

    if (!q.exec()) {
        QMessageBox::warning(this, "Erreur", "❌ " + q.lastError().text());
        return false;
    }

    e.setNom(editNom->text().trimmed());
    e.setPrenom(editPrenom->text().trimmed());
    e.setCourriel(editCourriel->text().trimmed());
    e.setTelephone(editTel->text().trimmed());
    e.setPoste(comboPoste->currentText());
    e.setDateEmbauche(dateEmb->date());
    e.setDisponible(checkDispo->isChecked());
    e.setChargeHebdo(spinCharge->value());
    e.setDispoHebdo(spinDispo->value());
    e.setCompetences(editComp->text().trimmed());

    return true;
}

// ==================== CRUD ====================
void GestionEmployes::ajouterEmploye()
{
    Employe vide;
    vide.setDispoHebdo(35);
    vide.setDateEmbauche(QDate::currentDate());

    if (ouvrirDialog(vide, false)) {
        QMessageBox::information(this, "Succès", "✅ Employé ajouté");
        onFiltresChange();
    }
}

void GestionEmployes::modifierEmploye()
{
    if (m_idSelectionne <= 0) {
        QMessageBox::warning(this, "Attention",
                             "Sélectionnez un employé dans le grid");
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT id, nom, prenom, courriel, telephone, poste, "
              "date_embauche, disponible, charge_hebdo, dispo_hebdo, "
              "competences FROM employes WHERE id=:id");
    q.bindValue(":id", m_idSelectionne);
    if (!q.exec() || !q.next()) return;

    Employe e;
    e.setId(q.value(0).toInt());
    e.setNom(q.value(1).toString());
    e.setPrenom(q.value(2).toString());
    e.setCourriel(q.value(3).toString());
    e.setTelephone(q.value(4).toString());
    e.setPoste(q.value(5).toString());
    e.setDateEmbauche(QDate::fromString(q.value(6).toString(), "yyyy-MM-dd"));
    e.setDisponible(q.value(7).toInt() == 1);
    e.setChargeHebdo(q.value(8).toDouble());
    e.setDispoHebdo(q.value(9).toDouble());
    e.setCompetences(q.value(10).toString());

    if (ouvrirDialog(e, true)) {
        QMessageBox::information(this, "Succès", "✅ Employé modifié");
        onFiltresChange();
        afficherDetail(e);
    }
}

void GestionEmployes::supprimerEmploye()
{
    if (m_idSelectionne <= 0) {
        QMessageBox::warning(this, "Attention",
                             "Sélectionnez un employé dans le grid");
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT nom, prenom FROM employes WHERE id=:id");
    q.bindValue(":id", m_idSelectionne);
    QString nom;
    if (q.exec() && q.next())
        nom = q.value(1).toString() + " " + q.value(0).toString();

    if (QMessageBox::question(this, "Confirmation",
                              "Supprimer " + nom + " ?") != QMessageBox::Yes)
        return;

    QSqlQuery qDel;
    qDel.prepare("DELETE FROM employes WHERE id=:id");
    qDel.bindValue(":id", m_idSelectionne);

    if (!qDel.exec()) {
        QMessageBox::warning(this, "Erreur", "❌ " + qDel.lastError().text());
        return;
    }

    QMessageBox::information(this, "Succès", "✅ Employé supprimé");
    effacerDetail();
    onFiltresChange();
}

// ==================== BASCULER ====================
void GestionEmployes::basculerDisponibilite()
{
    if (m_idSelectionne <= 0) {
        QMessageBox::warning(this, "Attention",
                             "Sélectionnez un employé dans le grid");
        return;
    }

    QSqlQuery qSelect;
    qSelect.prepare("SELECT disponible FROM employes WHERE id=:id");
    qSelect.bindValue(":id", m_idSelectionne);
    if (!qSelect.exec() || !qSelect.next()) return;

    bool actuel = qSelect.value(0).toInt() == 1;

    QSqlQuery q;
    q.prepare("UPDATE employes SET disponible=:d WHERE id=:id");
    q.bindValue(":d", actuel ? 0 : 1);
    q.bindValue(":id", m_idSelectionne);

    if (q.exec()) {
        QMessageBox::information(this, "Succès",
                                 QString("✅ Statut changé : %1").arg(actuel ? "Non disponible" : "Disponible"));
        onFiltresChange();
    }
}

// ==================== EMP-11 ====================
void GestionEmployes::verifierAdequation()
{
    bool ok;
    QString comp = QInputDialog::getText(this,
                                         "EMP-11 · Adéquation des compétences",
                                         "Compétence requise (ex: Soins, Toilettage, Accueil) :",
                                         QLineEdit::Normal, "", &ok);

    if (!ok || comp.trimmed().isEmpty()) return;

    QSqlQuery q;
    q.prepare("SELECT nom, prenom, poste, charge_hebdo, dispo_hebdo "
              "FROM employes WHERE disponible=1 "
              "AND competences LIKE :c "
              "ORDER BY (charge_hebdo * 1.0 / dispo_hebdo) ASC");
    q.bindValue(":c", "%" + comp.trimmed() + "%");

    QStringList resultats;
    if (q.exec()) {
        while (q.next()) {
            QString nom = q.value(0).toString() + " " + q.value(1).toString();
            QString poste = q.value(2).toString();
            double charge = q.value(3).toDouble();
            double dispo = q.value(4).toDouble();
            double taux = dispo > 0 ? (charge / dispo * 100.0) : 0;
            resultats << QString("• %1 — %2 (%3% de charge)")
                             .arg(nom, poste).arg(qRound(taux));
        }
    }

    if (resultats.isEmpty()) {
        QMessageBox::information(this, "EMP-11 · Résultat",
                                 "❌ Aucun employé disponible ne possède la compétence « " + comp + " ».");
    } else {
        QMessageBox::information(this, "EMP-11 · Profils proposés",
                                 "✅ Employés habilités pour « " + comp + " » :\n\n" + resultats.join("\n"));
    }
}

// ==================== EMP-12 ====================
void GestionEmployes::verifierEquilibrage()
{
    QSqlQuery q;
    q.exec("SELECT nom, prenom, poste, charge_hebdo, dispo_hebdo "
           "FROM employes WHERE disponible=1 AND dispo_hebdo > 0 "
           "ORDER BY (charge_hebdo * 1.0 / dispo_hebdo) DESC");

    QStringList surcharges, dispo;
    while (q.next()) {
        QString nom = q.value(0).toString() + " " + q.value(1).toString();
        double charge = q.value(3).toDouble();
        double d = q.value(4).toDouble();
        double taux = charge / d;

        if (taux > 0.8)
            surcharges << QString("⚠️  %1 — %2% (%3 h / %4 h)")
                              .arg(nom).arg(qRound(taux * 100)).arg(charge).arg(d);
        else if (taux < 0.5)
            dispo << QString("💡  %1 — %2% (%3 h / %4 h)")
                         .arg(nom).arg(qRound(taux * 100)).arg(charge).arg(d);
    }

    QString msg;
    msg += surcharges.isEmpty() ? "✅ Aucun employé en surcharge.\n\n"
                                : "🔴 Surcharge (>80%) :\n" + surcharges.join("\n") + "\n\n";
    if (!dispo.isEmpty())
        msg += "🟢 Moins chargés (<50%) — candidats réaffectation :\n" + dispo.join("\n");

    QMessageBox::information(this, "EMP-12 · Équilibrage", msg);
}