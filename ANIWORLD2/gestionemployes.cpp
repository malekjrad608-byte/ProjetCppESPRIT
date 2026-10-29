#include "gestionemployes.h"
#include "planning.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDateTime>
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
#include <QSet>

namespace {
QString afficherDateHeure(const QString &value)
{
    QDateTime dt = QDateTime::fromString(value, "yyyy-MM-dd HH:mm:ss");
    if (!dt.isValid()) dt = QDateTime::fromString(value, Qt::ISODate);
    return dt.isValid() ? dt.toString("dd/MM/yyyy HH:mm") : value;
}

// La requete SQL est dans le modele (Planning::conflits) : ici, uniquement la mise en forme.
QStringList conflitsPlanning()
{
    QStringList conflits;
    const QList<Planning::Conflit> liste = Planning::conflits();
    for (const Planning::Conflit &c : liste) {
        conflits << QString("%1 : « %2 » (%3–%4) chevauche « %5 » (%6–%7)")
                        .arg(c.employe, c.titre1,
                             afficherDateHeure(c.debut1),
                             afficherDateHeure(c.fin1),
                             c.titre2,
                             afficherDateHeure(c.debut2),
                             afficherDateHeure(c.fin2));
    }
    return conflits;
}

// La requete SQL est dans le modele (Planning::plagesAPartirDe).
QStringList alertesHeuresConsecutives()
{
    QStringList alertes;
    const QList<Planning::Plage> plages = Planning::plagesAPartirDe(QDate::currentDate());

    int idCourant = -1;
    QString nomCourant;
    QDateTime debutSerie, finSerie;
    auto verifierSerie = [&]() {
        if (debutSerie.isValid() && finSerie.isValid() &&
            debutSerie.secsTo(finSerie) >= 8 * 60 * 60) {
            alertes << QString("⚠ %1 enchaîne au moins 8 heures de planning (%2–%3).")
                           .arg(nomCourant,
                                debutSerie.toString("dd/MM/yyyy HH:mm"),
                                finSerie.toString("dd/MM/yyyy HH:mm"));
        }
    };

    for (const Planning::Plage &p : plages) {
        const int id = p.responsableId;
        const QString nom = p.prenom + " " + p.nom;
        const QDateTime debut = p.debut;
        const QDateTime fin = p.fin;

        if (id != idCourant || !finSerie.isValid() || debut > finSerie.addSecs(30 * 60)) {
            verifierSerie();
            idCourant = id;
            nomCourant = nom;
            debutSerie = debut;
            finSerie = fin;
        } else if (fin > finSerie) {
            finSerie = fin;
        }
    }
    verifierSerie();
    return alertes;
}
}

// ==================== Constructeur ====================
GestionEmployes::GestionEmployes(QWidget *parent) : QWidget(parent)
{
    construireUI();
    chargerEmployes();
}

void GestionEmployes::actualiserStats()
{
    mettreAJourStats();
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

// ==================== STATS PILLS ====================
QWidget *GestionEmployes::construireStatsPills()
{
    QWidget *w = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

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

        QHBoxLayout *cardsL = new QHBoxLayout;
        cardsL->setSpacing(12);

        const QDate debutSemaine = Planning::semaineDebut();
        // Les requetes sont dans le modele : Employe::chargesSemaine()
        const QList<Employe::ChargeEmploye> charges = Employe::chargesSemaine(debutSemaine);
        const Employe::StatsCharge stats = Employe::statistiques(charges);
        const int totalEmp = stats.total;
        const double totalCharge = stats.somme;
        const double avgCharge = stats.moyenne;
        const double maxCharge = stats.maximum;
        const double minCharge = stats.minimum;

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

        QStringList alertes;
        for (const Employe::ChargeEmploye &c : charges) {
            const double taux = c.dispo > 0 ? c.charge / c.dispo * 100.0
                                            : (c.charge > 0 ? 100.0 : 0.0);
            if (taux > 90.0) {
                alertes << QString("⚠ %1 %2 : %3 % de charge (%4 h / %5 h). Rééquilibrage conseillé.")
                               .arg(c.prenom, c.nom)
                               .arg(qRound(taux)).arg(QString::number(c.charge, 'f', 1))
                               .arg(QString::number(c.dispo, 'f', 1));
            }
        }

        alertes.append(alertesHeuresConsecutives());
        const QStringList conflitsDetectes = conflitsPlanning();
        for (const QString &conflit : conflitsDetectes)
            alertes << "⚠ Conflit de planning : " + conflit;
        if (alertes.isEmpty())
            alertes << "✅ Aucun dépassement de 90 %, aucune séquence de 8 h détectée et aucun conflit de planning.";

        QFrame *alertFrame = new QFrame;
        alertFrame->setStyleSheet("QFrame { background: #FFF4E5; border: 1px solid #F1CF98; border-radius: 10px; }");
        QVBoxLayout *alertLayout = new QVBoxLayout(alertFrame);
        alertLayout->setContentsMargins(14, 10, 14, 10);
        QLabel *alertTitle = new QLabel("🔔 Notifications au responsable");
        alertTitle->setStyleSheet("font-weight: bold; color: #8A4B08; background: transparent; border: none;");
        alertLayout->addWidget(alertTitle);
        QLabel *alertText = new QLabel(alertes.join("\n"));
        alertText->setWordWrap(true);
        alertText->setStyleSheet("color: #6D4C22; background: transparent; border: none;");
        alertLayout->addWidget(alertText);
        mainL->addWidget(alertFrame);

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
        table->setMinimumHeight(195);

        table->horizontalHeader()->setMinimumHeight(45);
        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

        int row = 0;
        for (const Employe::ChargeEmploye &c : charges) {
            table->insertRow(row);

            QString nom = c.prenom + " " + c.nom;
            QString poste = c.poste;
            double charge = c.charge;
            double dispo = c.dispo;
            double taux = dispo > 0 ? (charge / dispo * 100.0) : (charge > 0 ? 100.0 : 0.0);

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

        QHBoxLayout *planningActions = new QHBoxLayout;
        QPushButton *btnPlanifier = new QPushButton("🗓 Planifier + affecter automatiquement");
        btnPlanifier->setObjectName("btnPrimary");
        btnPlanifier->setMinimumHeight(42);
        connect(btnPlanifier, &QPushButton::clicked, this, &GestionEmployes::ouvrirPlanification);
        planningActions->addWidget(btnPlanifier);

        QPushButton *btnConflits = new QPushButton("🔎 Détecter les conflits");
        btnConflits->setObjectName("btnSecondary");
        btnConflits->setMinimumHeight(42);
        connect(btnConflits, &QPushButton::clicked, this, &GestionEmployes::detecterConflitsPlanning);
        planningActions->addWidget(btnConflits);
        mainL->addLayout(planningActions);

        QDialogButtonBox *btns = new QDialogButtonBox(QDialogButtonBox::Close);
        btns->button(QDialogButtonBox::Close)->setText("Fermer");
        btns->button(QDialogButtonBox::Close)->setMinimumHeight(42);
        btns->button(QDialogButtonBox::Close)->setMinimumWidth(120);
        connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        mainL->addWidget(btns);

        dlg.exec();
    });
    layout->addWidget(btnStats);

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
    m_comboTri->addItems({"Tri par date", "Tri par charge"});
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
    // Aucune requete ici : tout passe par le modele Employe (architecture Modele-Vue)
    const QList<Employe> liste = Employe::rechercher(filtre, tri, poste, statut);

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
    m_lblDisponibles->setText(QString::number(Employe::compterParDisponibilite(true)));
    m_lblIndisponibles->setText(QString::number(Employe::compterParDisponibilite(false)));

    double total = Employe::chargeTotaleDisponibles();
    total += Planning::chargePlanifieeSemaine(Planning::semaineDebut());

    m_lblChargeTotale->setText(QString::number(qRound(total)));
}

// ==================== FILTRES ====================
void GestionEmployes::onFiltresChange()
{
    QString filtre = m_editRecherche->text().trimmed();
    QString poste = m_comboPoste->currentText();
    QString statut = m_comboStatut->currentText();
    const QString tri = (m_comboTri->currentIndex() == 1) ? "charge" : "date";
    chargerEmployes(filtre, tri, poste, statut);
}

// ==================== EVENT FILTER ====================
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

            Employe emp;
            if (Employe::charger(id, emp))
                afficherDetail(emp);
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
    spinCharge->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    spinCharge->setSingleStep(1.0);
    spinCharge->setAccelerated(true);
    spinCharge->setSuffix(" h");
    spinCharge->setValue(e.getChargeHebdo());
    form->addRow("Charge hebdo", spinCharge);

    QDoubleSpinBox *spinDispo = new QDoubleSpinBox;
    spinDispo->setMinimumHeight(40);
    spinDispo->setRange(0, 60);
    spinDispo->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    spinDispo->setSingleStep(1.0);
    spinDispo->setAccelerated(true);
    spinDispo->setSuffix(" h");
    spinDispo->setValue(e.getDispoHebdo() > 0 ? e.getDispoHebdo() : 35);
    form->addRow("Dispo hebdo", spinDispo);

    QLineEdit *editComp = new QLineEdit(e.getCompetences());
    editComp->setMinimumHeight(40);
    editComp->setPlaceholderText("Soins, Toilettage...");
    form->addRow("Compétences", editComp);

    QCheckBox *checkDispo = new QCheckBox("Disponible");
    checkDispo->setTristate(false);
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
    // Construit un Employe a partir des champs du formulaire (id conserve)
    auto lireFormulaire = [&]() {
        Employe saisie = e;
        saisie.setNom(editNom->text().trimmed());
        saisie.setPrenom(editPrenom->text().trimmed());
        saisie.setCourriel(editCourriel->text().trimmed());
        saisie.setTelephone(editTel->text().trimmed());
        saisie.setPoste(comboPoste->currentText());
        saisie.setDateEmbauche(dateEmb->date());
        saisie.setDisponible(checkDispo->isChecked());
        saisie.setChargeHebdo(spinCharge->value());
        saisie.setDispoHebdo(spinDispo->value());
        saisie.setCompetences(editComp->text().trimmed());
        return saisie;
    };

    // Controle de saisie (C++) : le dialogue reste ouvert tant que la saisie est invalide
    connect(btns, &QDialogButtonBox::accepted, &dlg, [&]() {
        const QString erreurSaisie = lireFormulaire().validerSaisie();
        if (!erreurSaisie.isEmpty()) {
            QMessageBox::warning(&dlg, "Validation", erreurSaisie);
            return;
        }
        dlg.accept();
    });
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    mainL->addWidget(btns);

    if (dlg.exec() != QDialog::Accepted) return false;

    // La requete (INSERT / UPDATE prepare) est dans le modele Employe
    Employe saisie = lireFormulaire();
    QString erreur;
    const bool ok = modification ? saisie.modifier(&erreur) : saisie.ajouter(&erreur);
    if (!ok) {
        QMessageBox::warning(this, "Erreur", "❌ " + erreur);
        return false;
    }

    e = saisie;
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
        QMessageBox::warning(this, "Attention", "Sélectionnez un employé dans le grid");
        return;
    }

    Employe e;
    if (!Employe::charger(m_idSelectionne, e)) return;

    if (ouvrirDialog(e, true)) {
        QMessageBox::information(this, "Succès", "✅ Employé modifié");
        onFiltresChange();
        afficherDetail(e);
    }
}

void GestionEmployes::supprimerEmploye()
{
    if (m_idSelectionne <= 0) {
        QMessageBox::warning(this, "Attention", "Sélectionnez un employé dans le grid");
        return;
    }

    QString nom;
    {
        Employe e;
        if (Employe::charger(m_idSelectionne, e))
            nom = e.getPrenom() + " " + e.getNom();
    }

    if (QMessageBox::question(this, "Confirmation",
                              "Supprimer " + nom + " ?") != QMessageBox::Yes)
        return;

    QString erreur;
    if (!Employe::supprimer(m_idSelectionne, &erreur)) {
        QMessageBox::warning(this, "Erreur", "❌ " + erreur);
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
        QMessageBox::warning(this, "Attention", "Sélectionnez un employé dans le grid");
        return;
    }

    bool nouveauStatut = false;
    QString erreur;
    if (Employe::basculerDisponibilite(m_idSelectionne, &nouveauStatut, &erreur)) {
        QMessageBox::information(this, "Succès",
                                 QString("✅ Statut changé : %1").arg(nouveauStatut ? "Disponible" : "Non disponible"));
        onFiltresChange();
    } else {
        QMessageBox::warning(this, "Erreur", "❌ " + erreur);
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

    QStringList resultats;
    const QList<Employe> trouves = Employe::rechercherParCompetence(comp);
    for (const Employe &e : trouves) {
        const QString nom = e.getNom() + " " + e.getPrenom();
        const double dispoH = e.getDispoHebdo();
        const double taux = dispoH > 0 ? (e.getChargeHebdo() / dispoH * 100.0) : 0;
        resultats << QString("• %1 — %2 (%3% de charge)")
                         .arg(nom, e.getPoste()).arg(qRound(taux));
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
    QStringList surcharges, dispo;
    const QList<Employe> liste = Employe::listerDisponiblesParCharge(true);
    for (const Employe &e : liste) {
        const QString nom = e.getNom() + " " + e.getPrenom();
        const double charge = e.getChargeHebdo();
        const double d = e.getDispoHebdo();
        const double taux = charge / d;

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

// ==================== PLANIFICATION ====================
void GestionEmployes::ouvrirPlanification()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Planifier une tâche — affectation automatique");
    dlg.setMinimumWidth(520);
    dlg.setStyleSheet("QDialog { background-color: #F0F4F8; }");

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(14);

    QLabel *titre = new QLabel("🗓 Nouvelle tâche à planifier");
    titre->setStyleSheet("font-size: 20px; font-weight: bold; color: #225A64; background: transparent;");
    layout->addWidget(titre);

    QFormLayout *form = new QFormLayout;
    form->setSpacing(12);

    QLineEdit *editTitre = new QLineEdit;
    editTitre->setPlaceholderText("Ex. Consultation, soins, accueil...");
    editTitre->setMinimumHeight(40);
    form->addRow("Tâche *", editTitre);

    QLineEdit *editCompetence = new QLineEdit;
    editCompetence->setPlaceholderText("Ex. Soins, Chirurgie, Accueil");
    editCompetence->setMinimumHeight(40);
    form->addRow("Spécialité requise *", editCompetence);

    QDateTime start = QDateTime::currentDateTime().addSecs(3600);
    start.setTime(QTime(start.time().hour(), start.time().minute(), 0));
    QDateTime end = start.addSecs(2 * 3600);
    QDateTimeEdit *dateDebut = new QDateTimeEdit(start);
    dateDebut->setCalendarPopup(true);
    dateDebut->setDisplayFormat("dd/MM/yyyy HH:mm");
    dateDebut->setMinimumHeight(40);
    form->addRow("Début", dateDebut);

    QDateTimeEdit *dateFin = new QDateTimeEdit(end);
    dateFin->setCalendarPopup(true);
    dateFin->setDisplayFormat("dd/MM/yyyy HH:mm");
    dateFin->setMinimumHeight(40);
    form->addRow("Fin", dateFin);

    QDoubleSpinBox *heures = new QDoubleSpinBox;
    heures->setRange(0.5, 24.0);
    heures->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    heures->setAccelerated(true);
    heures->setSingleStep(0.5);
    heures->setValue(2.0);
    heures->setSuffix(" h");
    heures->setMinimumHeight(40);
    form->addRow("Charge de cette tâche", heures);

    layout->addLayout(form);
    QLabel *info = new QLabel("Le système choisira une personne disponible avec la spécialité demandée, "
                              "sans chevauchement horaire et avec la charge prévue la plus faible.");
    info->setWordWrap(true);
    info->setStyleSheet("color: #657B80; background: transparent;");
    layout->addWidget(info);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText("Affecter automatiquement");
    buttons->button(QDialogButtonBox::Ok)->setMinimumHeight(42);
    buttons->button(QDialogButtonBox::Cancel)->setMinimumHeight(42);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);

    if (dlg.exec() != QDialog::Accepted) return;
    Planning tache;
    tache.setTitre(editTitre->text().trimmed());
    tache.setCompetence(editCompetence->text().trimmed());
    tache.setDebut(dateDebut->dateTime());
    tache.setFin(dateFin->dateTime());
    tache.setHeures(heures->value());

    // Controle de saisie (C++)
    const QString erreurSaisie = tache.validerSaisie();
    if (!erreurSaisie.isEmpty()) {
        QMessageBox::warning(this, "Saisie invalide", erreurSaisie);
        return;
    }

    // Choix du responsable + INSERT : tout est dans le modele Planning
    QString meilleurNom, erreur;
    double meilleurTaux = 0.0;
    const Planning::Resultat resultat =
        tache.affecterAutomatiquement(meilleurNom, meilleurTaux, erreur);

    if (resultat == Planning::Resultat::AucunResponsable) {
        QMessageBox::warning(this, "Aucun responsable disponible",
                             "Aucune personne disponible ne possède cette spécialité avec une capacité suffisante "
                             "et sans conflit horaire. Essayez une autre date, une autre spécialité ou une charge plus faible.");
        return;
    }
    if (resultat == Planning::Resultat::ErreurBase) {
        QMessageBox::critical(this, "Erreur d'enregistrement",
                              "La tâche n'a pas été enregistrée : " + erreur);
        return;
    }

    const QString nomTache = tache.getTitre();
    const QDateTime debut = tache.getDebut();
    const QDateTime fin = tache.getFin();

    mettreAJourStats();
    QString message = QString("Tâche « %1 » enregistrée et affectée à %2.\n"
                              "Créneau : %3 – %4\nCharge prévue après affectation : %5 %%. ")
                          .arg(nomTache, meilleurNom, debut.toString("dd/MM/yyyy HH:mm"),
                               fin.toString("dd/MM/yyyy HH:mm"))
                          .arg(qRound(meilleurTaux * 100));
    if (meilleurTaux > 0.90)
        message += "\n⚠ Attention : charge supérieure à 90 %, rééquilibrage conseillé.";
    QMessageBox::information(this, "Affectation automatique", message);
}

void GestionEmployes::detecterConflitsPlanning()
{
    const QStringList conflits = conflitsPlanning();
    if (conflits.isEmpty()) {
        QMessageBox::information(this, "Vérification du planning",
                                 "✅ Aucun chevauchement détecté entre les tâches affectées.");
        return;
    }
    QMessageBox::warning(this, "Conflits de planification détectés",
                         "Les affectations suivantes se chevauchent :\n\n" + conflits.join("\n\n"));
}