#include "ui_integrated_pages.h"
#include "../database/oracleconnection.h"
#include "employe.h"
#include "../ui/relationchoices.h"
#include "../dialogs/serviceplanningdialog.h"
#include "../models/service.h"
#include "gestionemployes.h"
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
#include "../models/employeequeries.h"

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
#include <QPainter>
#include <QScreen>
#include <QKeyEvent>
#include <QMenu>
#include <QFileDialog>
#include "../ui/design.h"
#include "../ui/motion.h"
#include "../ui/invoice.h"
#include "../widgets/animatedbutton.h"
#include "../widgets/animatedpanel.h"

namespace {
QDate semaineDebut(const QDate &date = QDate::currentDate())
{
    return date.addDays(1 - date.dayOfWeek());
}

QString borneSemaine(const QDate &date)
{
    return date.toString("yyyy-MM-dd") + " 00:00:00";
}

QString afficherDateHeure(const QString &value)
{
    QDateTime dt = QDateTime::fromString(value, "yyyy-MM-dd HH:mm:ss");
    if (!dt.isValid()) dt = QDateTime::fromString(value, Qt::ISODate);
    return dt.isValid() ? dt.toString("dd/MM/yyyy HH:mm") : value;
}

QStringList conflitsPlanning()
{
    QStringList conflits;
    auto q = EmployeeQueries::conflicts();
    while (q.next()) {
        conflits << QString("%1 : « %2 » (%3–%4) chevauche « %5 » (%6–%7)")
                         .arg(q.value(0).toString(), q.value(1).toString(),
                              afficherDateHeure(q.value(2).toString()),
                              afficherDateHeure(q.value(3).toString()),
                              q.value(4).toString(),
                              afficherDateHeure(q.value(5).toString()),
                              afficherDateHeure(q.value(6).toString()));
    }
    return conflits;
}

QStringList alertesHeuresConsecutives()
{
    QStringList alertes;
    auto q = EmployeeQueries::consecutive(QDate::currentDate().toString("yyyy-MM-dd") + " 00:00:00");
    if (!q.isOk()) return alertes;

    int idCourant = -1;
    QString nomCourant;
    QDateTime debutSerie, finSerie;
    auto verifierSerie = [&]() {
        if (debutSerie.isValid() && finSerie.isValid() &&
            debutSerie.secsTo(finSerie) >= 8 * 60 * 60) {
            alertes << QString(" %1 enchaîne au moins 8 heures de planning (%2–%3).")
                           .arg(nomCourant,
                                debutSerie.toString("dd/MM/yyyy HH:mm"),
                                finSerie.toString("dd/MM/yyyy HH:mm"));
        }
    };

    while (q.next()) {
        const int id = q.value(0).toInt();
        const QString nom = q.value(1).toString() + " " + q.value(2).toString();
        QDateTime debut = QDateTime::fromString(q.value(3).toString(), "yyyy-MM-dd HH:mm:ss");
        QDateTime fin = QDateTime::fromString(q.value(4).toString(), "yyyy-MM-dd HH:mm:ss");
        if (!debut.isValid() || !fin.isValid()) continue;

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

// Actualise uniquement les compteurs en haut de page pour éviter de recréer
// les cartes d’employés à chaque retour sur l’onglet.
void GestionEmployes::actualiserStats()
{
    onFiltresChange();
}

void GestionEmployes::paintEvent(QPaintEvent *) { QPainter painter(this); Design::paintWorkspace(painter,rect()); }
void GestionEmployes::focusSearch() { m_editRecherche->setFocus(); m_editRecherche->selectAll(); }

// === MODULE EMPLOYES : LIAISON AU FICHIER MAINWINDOW.UI ===
void GestionEmployes::construireUI() {
    Ui::EmployeesPage ui;ui.setupUi(this);
    m_cardsContainer=ui.Employees_m_cardsContainer;
    m_comboPoste=ui.Employees_m_comboPoste;
    m_comboStatut=ui.Employees_m_comboStatut;
    m_comboTri=ui.Employees_m_comboTri;
    m_detailActions=ui.Employees_m_detailActions;
    m_detailCharge=ui.Employees_m_detailCharge;
    m_detailCompetences=ui.Employees_m_detailCompetences;
    m_detailContent=ui.Employees_m_detailContent;
    m_detailCourriel=ui.Employees_m_detailCourriel;
    m_detailDate=ui.Employees_m_detailDate;
    m_detailDispo=ui.Employees_m_detailDispo;
    m_detailId=ui.Employees_m_detailId;
    m_detailNom=ui.Employees_m_detailNom;
    m_detailPlaceholder=ui.Employees_m_detailPlaceholder;
    m_detailPoste=ui.Employees_m_detailPoste;
    m_detailStatut=ui.Employees_m_detailStatut;
    m_detailTelephone=ui.Employees_m_detailTelephone;
    m_editRecherche=ui.Employees_m_editRecherche;
    m_gridCards=ui.Employees_m_gridCards;
    m_lblChargeTotale=ui.Employees_m_lblChargeTotale;
    m_lblDisponibles=ui.Employees_m_lblDisponibles;
    m_lblIndisponibles=ui.Employees_m_lblIndisponibles;
    m_gridCards->setAlignment(Qt::AlignTop);
    m_detailActions->hide();m_detailContent->hide();
    m_editRecherche->addAction(Design::icon("search"),QLineEdit::LeadingPosition);
    connect(ui.addEmployee,&QPushButton::clicked,this,&GestionEmployes::ajouterEmploye);
    connect(ui.employeeLeavePdf,&QPushButton::clicked,this,&GestionEmployes::exportLeaveCertificate);
    connect(ui.employeeStatistics,&QPushButton::clicked,this,&GestionEmployes::showStatistics);
    connect(ui.editEmployee,&QPushButton::clicked,this,&GestionEmployes::modifierEmploye);
    connect(ui.toggleEmployeeAvailability,&QPushButton::clicked,this,&GestionEmployes::basculerDisponibilite);
    connect(ui.deleteEmployee,&QPushButton::clicked,this,&GestionEmployes::supprimerEmploye);
    connect(m_editRecherche,&QLineEdit::textChanged,this,&GestionEmployes::onFiltresChange);
    for(auto *combo:{m_comboPoste,m_comboStatut,m_comboTri})connect(combo,&QComboBox::currentTextChanged,this,&GestionEmployes::onFiltresChange);
}

void GestionEmployes::showStatistics() {
        QDialog dlg(this);
        dlg.setObjectName("employeeStatisticsDialog");
        dlg.setWindowTitle("Statistiques des employés");
        dlg.setMinimumSize(760,540);
        dlg.resize(QSize(940,780).boundedTo(screen()->availableGeometry().size()-QSize(40,60)));
        dlg.setStyleSheet("QDialog { background-color: #F0F4F8; }");

        auto *dialogLayout=new QVBoxLayout(&dlg); dialogLayout->setContentsMargins(8,8,8,8);
        auto *contentScroll=new QScrollArea; contentScroll->setWidgetResizable(true);
        auto *content=new QWidget; contentScroll->setWidget(content); dialogLayout->addWidget(contentScroll,1);
        QVBoxLayout *mainL = new QVBoxLayout(content); mainL->setSizeConstraint(QLayout::SetMinimumSize);
        mainL->setContentsMargins(25, 25, 25, 25);
        mainL->setSpacing(10);

        QLabel *titre = new QLabel("  Statistiques des employés");
        titre->setStyleSheet("font-size: 22px; font-weight: bold; color: #225A64;"
                             "background: transparent;");
        mainL->addWidget(titre);

        QLabel *sous = new QLabel("Charge hebdomadaire et répartition par poste");
        sous->setStyleSheet("font-size: 13px; color: #7A8B8F; background: transparent;");
        mainL->addWidget(sous);

        // CARDS RÉSUMÉ
        QHBoxLayout *cardsL = new QHBoxLayout;
        cardsL->setSpacing(12);

        const QDate debutSemaine = semaineDebut();
        const QString debutSemaineSql = borneSemaine(debutSemaine);
        const QString finSemaineSql = borneSemaine(debutSemaine.addDays(7));
        auto qStats = EmployeeQueries::statistics(debutSemaineSql, finSemaineSql);
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

            QLabel *ico = new QLabel; ico->setPixmap(Design::icon(icone,QColor(fg)).pixmap(18,18));
            ico->setStyleSheet("font-size: 23px; background: transparent; border: none;");
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

        cardsL->addWidget(creerCard("users", "#E4F0F3", "#225A64",
                                    "Total", QString::number(totalEmp), "employés"));
        cardsL->addWidget(creerCard("clock", "#FDF3E3", "#B8741A",
                                    "Charge totale", QString::number(totalCharge, 'f', 0) + " h", "par semaine"));
        cardsL->addWidget(creerCard("grid", "#E6F4EA", "#2E7D32",
                                    "Charge moyenne", QString::number(avgCharge, 'f', 1) + " h", "par employé"));
        cardsL->addWidget(creerCard("shield", "#FBE9E7", "#AE5751",
                                    "Charge max", QString::number(maxCharge, 'f', 0) + " h", "le plus chargé"));

        mainL->addLayout(cardsL);
        mainL->addSpacing(5);

        // Alertes visuelles : surcharge > 90 %, longues séquences horaires et conflits.
        QStringList alertes;
        auto qCharges = EmployeeQueries::workloads(debutSemaineSql, finSemaineSql);
        if (qCharges.isOk()) {
            while (qCharges.next()) {
                const double charge = qCharges.value(2).toDouble() + qCharges.value(4).toDouble();
                const double capacite = qCharges.value(3).toDouble();
                const double taux = capacite > 0 ? charge / capacite * 100.0 : (charge > 0 ? 100.0 : 0.0);
                if (taux > 90.0) {
                    alertes << QString(" %1 %2 : %3 % de charge (%4 h / %5 h). Rééquilibrage conseillé.")
                                   .arg(qCharges.value(0).toString(), qCharges.value(1).toString())
                                   .arg(qRound(taux)).arg(QString::number(charge, 'f', 1))
                                   .arg(QString::number(capacite, 'f', 1));
                }
            }
        }
        alertes.append(alertesHeuresConsecutives());
        const QStringList conflitsDetectes = conflitsPlanning();
        for (const QString &conflit : conflitsDetectes)
            alertes << " Conflit de planning : " + conflit;
        if (alertes.isEmpty())
            alertes << " Aucun dépassement de 90 %, aucune séquence de 8 h détectée et aucun conflit de planning.";

        QFrame *alertFrame = new QFrame;
        alertFrame->setStyleSheet("QFrame { background: #FFF4E5; border: 1px solid #F1CF98; border-radius: 10px; }");
        QVBoxLayout *alertLayout = new QVBoxLayout(alertFrame);
        alertLayout->setContentsMargins(14, 10, 14, 10);
        QLabel *alertTitle = new QLabel(" Notifications au responsable");
        alertTitle->setStyleSheet("font-weight: bold; color: #8A4B08; background: transparent; border: none;");
        alertLayout->addWidget(alertTitle);
        QLabel *alertText = new QLabel(alertes.join("\n"));
        alertText->setWordWrap(true);
        alertText->setStyleSheet("color: #6D4C22; background: transparent; border: none;");
        alertLayout->addWidget(alertText);
        mainL->addWidget(alertFrame);

        QLabel *titreTable = new QLabel("  Charge par employé");
        titreTable->setStyleSheet("font-size: 16px; font-weight: bold; color: #225A64;"
                                  "background: transparent;");
        mainL->addWidget(titreTable);

        QTableWidget *table = new QTableWidget; table->setObjectName("employeeLoadTable");
        table->setColumnCount(5);
        table->setHorizontalHeaderLabels(
            {"Employé", "Poste", "Charge (h)", "Dispo (h)", "Taux de charge"});
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setFocusPolicy(Qt::NoFocus);
        table->setAlternatingRowColors(true);
        table->verticalHeader()->setVisible(false);
        table->verticalHeader()->setDefaultSectionSize(48);
        // Afficher au minimum 3 employés sans devoir faire défiler le tableau.
        table->setMinimumHeight(235);

        table->horizontalHeader()->setMinimumHeight(45);
        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

        auto q = EmployeeQueries::workloadRanking(debutSemaineSql, finSemaineSql);

        int row = 0;
        while (q.next()) {
            table->insertRow(row);

            QString nom = q.value(2).toString() + " " + q.value(1).toString();
            QString poste = q.value(2).toString();
            double charge = q.value(3).toDouble();
            double dispo = q.value(4).toDouble();
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
                                         "  <b>%1 heures</b> planifiées au total / semaine   ·   "
                                         "  Moyenne : <b>%2 h</b> par employé   ·   "
                                         "  Max : <b>%3 h</b>   ·   "
                                         "  Min : <b>%4 h</b>")
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
        QPushButton *btnPlanifier = Design::button(OracleConnection::isOracle()?"Planning des services":"Planifier et affecter","calendar","primary");
        btnPlanifier->setObjectName("planEmployeeTask"); btnPlanifier->setProperty("role","primary");
        btnPlanifier->setMinimumHeight(42);
        connect(btnPlanifier, &QPushButton::clicked, this, &GestionEmployes::ouvrirPlanification);
        planningActions->addWidget(btnPlanifier);

        QPushButton *btnConflits = Design::button("Détecter les conflits","search","");
        btnConflits->setObjectName("checkEmployeeConflicts"); btnConflits->setProperty("role","");
        btnConflits->setMinimumHeight(42);
        connect(btnConflits, &QPushButton::clicked, this, &GestionEmployes::detecterConflitsPlanning);
        planningActions->addWidget(btnConflits);
        mainL->addLayout(planningActions);

        QDialogButtonBox *btns = new QDialogButtonBox(QDialogButtonBox::Close);
        btns->button(QDialogButtonBox::Close)->setText("Fermer");
        btns->button(QDialogButtonBox::Close)->setMinimumHeight(42);
        btns->button(QDialogButtonBox::Close)->setMinimumWidth(120);
        connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        dialogLayout->addWidget(btns);

        dlg.exec();
    }



// ==================== HEADER ====================


// ==================== STATS PILLS (bla animation compteurs) ====================


// ==================== BARRE RECHERCHE ====================


// ==================== GRID ====================


// ==================== CARD ====================
QWidget *GestionEmployes::creerCard(const Employe &e)
{
    QFrame *card = new AnimatedPanel;
    card->setFocusPolicy(Qt::StrongFocus);
    bool selectionne = (e.getId() == m_idSelectionne);
    card->setObjectName(selectionne ? "cardEmployeSelected" : "cardEmploye");
    card->setCursor(Qt::PointingHandCursor);
    card->setFixedHeight(144);
    card->setProperty("employe_id", e.getId());
    card->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(5);

    QHBoxLayout *topRow = new QHBoxLayout;
    topRow->setSpacing(6);
    topRow->addStretch();

    QLabel *badge = new QLabel(e.isDisponible() ? "● Disponible" : "● Indisponible");
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
    nom->setStyleSheet("font-size: 14px; font-weight: bold; color: #225A64;"
                       "background: transparent;");
    nom->setTextFormat(Qt::PlainText); nom->setWordWrap(true);
    layout->addWidget(nom);

    QLabel *poste = new QLabel(e.getPoste());
    poste->setWordWrap(true); poste->setTextFormat(Qt::PlainText);
    poste->setStyleSheet("font-size: 12px; color: #7A8B8F; background: transparent;");
    layout->addWidget(poste);

    layout->addStretch();

    QHBoxLayout *botRow = new QHBoxLayout;
    botRow->setSpacing(10);

    auto *clock=Design::label(""); clock->setPixmap(Design::icon("clock",QColor("#84979E")).pixmap(12,12)); botRow->addWidget(clock);
    QLabel *charge = new QLabel(QString("%1 h").arg(e.getChargeHebdo(), 0, 'f', 0));
    charge->setStyleSheet("font-size: 11px; color: #263B42; font-weight: 500;"
                          "background: transparent;");
    botRow->addWidget(charge);

    botRow->addStretch();

    if (!e.getCompetences().isEmpty()) {
        QString comp = e.getCompetences();
        if (comp.length() > 22) comp = comp.left(22) + "...";
        QLabel *compLbl = new QLabel(" " + comp);
        compLbl->setStyleSheet("font-size: 10px; color: #7A8B8F; background: transparent;");
        botRow->addWidget(compLbl);
    }

    layout->addLayout(botRow);

    for(auto *label:card->findChildren<QLabel *>()) label->setAttribute(Qt::WA_TransparentForMouseEvents);

    return card;
}

// ==================== DETAIL PANEL ====================


// ==================== CHARGEMENT ====================
void GestionEmployes::chargerEmployes(const QString &filtre, const QString &tri,
                                      const QString &poste, const QString &statut)
{
    QString error;
    const auto liste = Employe::afficher(filtre, tri, poste, statut, &error);
    if(!error.isEmpty()) { m_detailPlaceholder->setText("Chargement impossible : " + error); return; }

    afficherCards(liste);
    bool selected=false; for(const auto &employee:liste) if(employee.getId()==m_idSelectionne) { afficherDetail(employee); selected=true; break; }
    if(!selected) effacerDetail();
    mettreAJourStats();
}

// ==================== AFFICHER CARDS ====================
void GestionEmployes::afficherCards(const QList<Employe> &liste)
{
    QLayoutItem *item;
    while ((item = m_gridCards->takeAt(0)) != nullptr) {
        if (item->widget()) { item->widget()->hide(); item->widget()->deleteLater(); }
        delete item;
    }

    if(liste.isEmpty()) { auto *empty=Design::label("Aucun employé ne correspond à votre recherche.","muted"); empty->setWordWrap(true); m_gridCards->addWidget(empty,0,0,1,2); }
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
    m_detailContent->show(); m_detailActions->show();

    m_detailNom->setText(e.getPrenom() + " " + e.getNom());
    m_detailId->setText(QString("ID Employé #%1").arg(e.getId()));

    if (e.isDisponible()) {
        m_detailStatut->setText("● Disponible");
        m_detailStatut->setStyleSheet(
            "font-size: 12px; color: #2E7D32; background-color: #E6F4EA;"
            "padding: 8px; border-radius: 8px; font-weight: bold; border: none;");
    } else {
        m_detailStatut->setText("● Non disponible");
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
    m_detailContent->hide(); m_detailActions->hide();
    m_detailPlaceholder->show();
}

// ==================== STATS ====================
void GestionEmployes::mettreAJourStats()
{
    auto q = EmployeeQueries::countAvailable(true);
    if (q.next()) m_lblDisponibles->setText(q.value(0).toString());

    q = EmployeeQueries::countAvailable(false);
    if (q.next()) m_lblIndisponibles->setText(q.value(0).toString());

    const QDate debut = semaineDebut();
    auto qCharge = EmployeeQueries::weeklyCharge(borneSemaine(debut), borneSemaine(debut.addDays(7)));
    if (qCharge.isOk() && qCharge.next())
        m_lblChargeTotale->setText(QLocale().toString(qCharge.value(0).toDouble(), 'g', 6));
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

// ==================== EVENT FILTER (Hover + Click) ====================
bool GestionEmployes::eventFilter(QObject *obj, QEvent *event)
{
    QWidget *card = qobject_cast<QWidget*>(obj);
    if (!card || card->property("employe_id").toInt() <= 0)
        return QWidget::eventFilter(obj, event);

    if ((event->type() == QEvent::MouseButtonPress && static_cast<QMouseEvent *>(event)->button()==Qt::LeftButton) ||
        (event->type()==QEvent::KeyPress && (static_cast<QKeyEvent *>(event)->key()==Qt::Key_Return || static_cast<QKeyEvent *>(event)->key()==Qt::Key_Space))) {
        int id = card->property("employe_id").toInt();
        if (id > 0) {
            m_idSelectionne = id;
            onFiltresChange();

            auto q = EmployeeQueries::find(id);
            if (q.isOk() && q.next()) {
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
    dlg.setObjectName("employeeForm");
    dlg.setWindowTitle(modification ? "Modifier l'employé" : "Nouvel employé");
    dlg.setMinimumWidth(490);
    dlg.setStyleSheet("QDialog { background-color: #F0F4F8; }");

    QVBoxLayout *mainL = new QVBoxLayout(&dlg);
    mainL->setContentsMargins(25, 25, 25, 25);
    mainL->setSpacing(10);

    QLabel *titre = new QLabel(modification ? "  Modifier l'employé" : "  Nouvel employé");
    titre->setStyleSheet("font-size: 20px; font-weight: bold; color: #225A64;"
                         "background: transparent;");
    mainL->addWidget(titre);

    QFormLayout *form = new QFormLayout;
    form->setSpacing(8);

    QLineEdit *editNom = new QLineEdit(e.getNom());
    editNom->setObjectName("employeeLastName");
    editNom->setMinimumHeight(34);
    editNom->setPlaceholderText("Nom *");
    form->addRow("Nom *", editNom);

    QLineEdit *editPrenom = new QLineEdit(e.getPrenom());
    editPrenom->setObjectName("employeeFirstName");
    editPrenom->setMinimumHeight(34);
    editPrenom->setPlaceholderText("Prénom *");
    form->addRow("Prénom *", editPrenom);

    QLineEdit *editCourriel = new QLineEdit(e.getCourriel());
    editCourriel->setObjectName("employeeEmail");
    editCourriel->setMinimumHeight(34);
    form->addRow("Courriel", editCourriel);

    QLineEdit *editTel = new QLineEdit(e.getTelephone());
    editTel->setObjectName("employeePhone");
    editTel->setMinimumHeight(34);
    form->addRow("Téléphone", editTel);

    QComboBox *comboPoste = new QComboBox;
    comboPoste->setMinimumHeight(34);
    comboPoste->addItems({"Vétérinaire", "Assistant(e) vétérinaire",
                          "Auxiliaire vétérinaire", "Secrétaire / Réceptionniste",
                          "Agent d'entretien", "Aide aux soins animaliers"});
    if (!e.getPoste().isEmpty()) comboPoste->setCurrentText(e.getPoste());
    form->addRow("Poste", comboPoste);
    auto *serviceChoice=new QComboBox(&dlg);serviceChoice->setObjectName("employeeService");
    RelationChoices::services(serviceChoice,OracleConnection::isOracle()?Employe::employeeService(e.getId()):0);
    if(OracleConnection::isOracle())form->addRow("Service *",serviceChoice);else serviceChoice->hide();

    QDateEdit *dateEmb = new QDateEdit(e.getDateEmbauche().isValid() ?
                                           e.getDateEmbauche() : QDate::currentDate());
    dateEmb->setMinimumHeight(34);
    dateEmb->setCalendarPopup(true);
    dateEmb->setDisplayFormat("dd/MM/yyyy");
    form->addRow("Embauche", dateEmb);

    QDoubleSpinBox *spinCharge = new QDoubleSpinBox;
    spinCharge->setObjectName("employeeWorkload");
    spinCharge->setMinimumHeight(34);
    spinCharge->setRange(0, 60);
    spinCharge->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    spinCharge->setSingleStep(1.0);
    spinCharge->setAccelerated(true);
    spinCharge->setSuffix(" h");
    spinCharge->setValue(e.getChargeHebdo());
    form->addRow("Charge hebdo", spinCharge);

    QDoubleSpinBox *spinDispo = new QDoubleSpinBox;
    spinDispo->setObjectName("employeeCapacity");
    spinDispo->setMinimumHeight(34);
    spinDispo->setRange(0, 60);
    spinDispo->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    spinDispo->setSingleStep(1.0);
    spinDispo->setAccelerated(true);
    spinDispo->setSuffix(" h");
    spinDispo->setValue(e.getDispoHebdo() > 0 ? e.getDispoHebdo() : 35);
    form->addRow("Dispo hebdo", spinDispo);

    QLineEdit *editComp = new QLineEdit(e.getCompetences());
    editComp->setObjectName("employeeSkills");
    editComp->setMinimumHeight(34);
    editComp->setPlaceholderText("Soins, Toilettage...");
    form->addRow("Compétences", editComp);

    QCheckBox *checkDispo = new QCheckBox("Disponible");
    checkDispo->setTristate(false);
    // À la création, l’employé est disponible par défaut (case cochée).
    // En modification, on conserve le statut enregistré.
    checkDispo->setChecked(modification ? e.isDisponible() : true);
    form->addRow("", checkDispo);

    auto *cin=new QLineEdit(e.getCin());cin->setObjectName("employeeCin");cin->setMaxLength(30);
    auto *birth=new QLineEdit(e.getDateNaissance().toString(Qt::ISODate));birth->setObjectName("employeeBirthDate");birth->setPlaceholderText("AAAA-MM-JJ (facultatif)");
    auto *address=new QLineEdit(e.getAdresse());address->setObjectName("employeeAddress");address->setMaxLength(500);
    auto *contract=new QLineEdit(e.getTypeContrat());contract->setObjectName("employeeContract");contract->setMaxLength(80);
    auto *salary=new QDoubleSpinBox;salary->setObjectName("employeeSalary");salary->setRange(-1,99999999);salary->setSpecialValueText("Non renseigné");salary->setSuffix(" DT");salary->setValue(e.getSalaire());
    if(OracleConnection::isOracle()) {form->addRow("CIN (unique)",cin);form->addRow("Date de naissance",birth);form->addRow("Adresse",address);form->addRow("Type de contrat",contract);form->addRow("Salaire",salary);}
    else {cin->hide();birth->hide();address->hide();contract->hide();salary->hide();}
    if(OracleConnection::isOracle()) {
        // Same fields and order as EMPLOYER; the ID is generated by Oracle.
        while(form->rowCount()>0){auto row=form->takeRow(0);if(row.labelItem){delete row.labelItem->widget();delete row.labelItem;}delete row.fieldItem;}
        auto *id=new QLineEdit(e.getId()>0?QString::number(e.getId()):QString("Automatique"),&dlg);
        id->setObjectName("employeeId");id->setReadOnly(true);form->addRow("ID employé",id);
        form->addRow("Nom *",editNom);form->addRow("Prénom *",editPrenom);form->addRow("CIN (unique)",cin);
        form->addRow("Date de naissance",birth);form->addRow("Adresse",address);form->addRow("Téléphone",editTel);
        form->addRow("Poste",comboPoste);form->addRow("Courriel",editCourriel);form->addRow("Compétences",editComp);
        form->addRow("Date d'embauche",dateEmb);form->addRow("Type de contrat",contract);form->addRow("Salaire",salary);
        form->addRow("Charge",spinCharge);form->addRow("Disponible",checkDispo);form->addRow("Disponibilité / repos",spinDispo);
        form->addRow("Service *",serviceChoice);
    }
    auto *profile=new QWidget(&dlg);profile->setLayout(form);
    auto *profileScroll=new QScrollArea(&dlg);profileScroll->setWidgetResizable(true);profileScroll->setFrameShape(QFrame::NoFrame);profileScroll->setWidget(profile);
    mainL->addWidget(profileScroll,1);dlg.resize(620,780);
    mainL->addSpacing(10);

    QDialogButtonBox *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btns->button(QDialogButtonBox::Ok)->setText(modification ? "Modifier" : "Ajouter");
    btns->button(QDialogButtonBox::Ok)->setMinimumHeight(36);
    btns->button(QDialogButtonBox::Ok)->setMinimumWidth(120);
    btns->button(QDialogButtonBox::Cancel)->setText("Annuler");
    btns->button(QDialogButtonBox::Cancel)->setMinimumHeight(36);
    btns->button(QDialogButtonBox::Cancel)->setMinimumWidth(120);
    auto *validation=Design::label("","muted"); validation->setObjectName("employeeValidation"); validation->setWordWrap(true); mainL->addWidget(validation);
    connect(btns, &QDialogButtonBox::accepted, &dlg, [&] {
        if(editNom->text().trimmed().isEmpty() || editPrenom->text().trimmed().isEmpty()) {
            validation->setText("Nom et prénom sont obligatoires."); return;
        }
        if(OracleConnection::isOracle() && serviceChoice->currentData().toInt()<=0){validation->setText("Choisissez le service auquel appartient cet employé.");return;}
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
    if(!birth->text().trimmed().isEmpty() && !QDate::fromString(birth->text().trimmed(),Qt::ISODate).isValid()) {
      validation->setText("Date de naissance attendue : AAAA-MM-JJ.");return;
    }
    e.setCin(cin->text().trimmed());e.setDateNaissance(QDate::fromString(birth->text().trimmed(),Qt::ISODate));
    e.setAdresse(address->text().trimmed());e.setTypeContrat(contract->text().trimmed());e.setSalaire(salary->value());

    e.setServiceId(serviceChoice->currentData().toInt());
    QString error;
    if (!(modification ? e.modifier(&error) : e.ajouter(&error))) {
        validation->setText(error);
        return;
    }
        dlg.accept();
    });
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    mainL->addWidget(btns);

    if (dlg.exec() != QDialog::Accepted) return false;

    return true;
}

// ==================== CRUD ====================
void GestionEmployes::ajouterEmploye()
{
    Employe vide;
    vide.setDispoHebdo(35);
    vide.setDateEmbauche(QDate::currentDate());

    if (ouvrirDialog(vide, false)) {
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

    auto q = EmployeeQueries::find(m_idSelectionne);
    if (!q.isOk() || !q.next()) return;

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
    if(OracleConnection::isOracle()) {
      e.setCin(q.value(11).toString());e.setDateNaissance(QDate::fromString(q.value(12).toString(),Qt::ISODate));
      e.setAdresse(q.value(13).toString());e.setTypeContrat(q.value(14).toString());e.setSalaire(q.value(15).isNull()?-1:q.value(15).toDouble());
    }

    if (ouvrirDialog(e, true)) {
        onFiltresChange();
    }
}

void GestionEmployes::supprimerEmploye()
{
    if (m_idSelectionne <= 0) {
        QMessageBox::warning(this, "Attention",
                             "Sélectionnez un employé dans le grid");
        return;
    }

    auto q = EmployeeQueries::find(m_idSelectionne);
    QString nom;
    if (q.isOk() && q.next())
        nom = q.value(2).toString() + " " + q.value(1).toString();

    if (QMessageBox::question(this, "Confirmation",
                              "Supprimer " + nom + " ?") != QMessageBox::Yes)
        return;

    QString error;
    if (!Employe::supprimer(m_idSelectionne, &error)) {
        QMessageBox::warning(this, "Employé conservé", error);
        return;
    }
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

    QString error;
    if (!Employe::basculerDisponibilite(m_idSelectionne, &error)) {
        QMessageBox::warning(this, "Modification impossible", error);
        return;
    }
    onFiltresChange();
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

    auto q = EmployeeQueries::skilled(comp.trimmed());

    QStringList resultats;
    if (q.isOk()) {
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
                                 " Aucun employé disponible ne possède la compétence « " + comp + " ».");
    } else {
        QMessageBox::information(this, "EMP-11 · Profils proposés",
                                 " Employés habilités pour « " + comp + " » :\n\n" + resultats.join("\n"));
    }
}

// ==================== EMP-12 ====================
void GestionEmployes::verifierEquilibrage()
{
    auto q = EmployeeQueries::balance();

    QStringList surcharges, dispo;
    while (q.next()) {
        QString nom = q.value(0).toString() + " " + q.value(1).toString();
        double charge = q.value(3).toDouble();
        double d = q.value(4).toDouble();
        double taux = charge / d;

        if (taux > 0.8)
            surcharges << QString("  %1 — %2% (%3 h / %4 h)")
                              .arg(nom).arg(qRound(taux * 100)).arg(charge).arg(d);
        else if (taux < 0.5)
            dispo << QString("  %1 — %2% (%3 h / %4 h)")
                         .arg(nom).arg(qRound(taux * 100)).arg(charge).arg(d);
    }

    QString msg;
    msg += surcharges.isEmpty() ? " Aucun employé en surcharge.\n\n"
                                : " Surcharge (>80%) :\n" + surcharges.join("\n") + "\n\n";
    if (!dispo.isEmpty())
        msg += " Moins chargés (<50%) — candidats réaffectation :\n" + dispo.join("\n");

    QMessageBox::information(this, "EMP-12 · Équilibrage", msg);
}

// ==================== PLANIFICATION ET AFFECTATION AUTOMATIQUE ====================
void GestionEmployes::ouvrirPlanification()
{
    if(OracleConnection::isOracle()){ServicePlanningDialog planning(QDate::currentDate(),this);planning.showEmployee(m_idSelectionne>0?m_idSelectionne:0);planning.exec();return;}
    QDialog dlg(this);
    dlg.setObjectName("employeePlanningDialog");
    dlg.setWindowTitle("Planifier une tâche — affectation automatique");
    dlg.setMinimumWidth(490);
    dlg.setStyleSheet("QDialog { background-color: #F0F4F8; }");

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(14);

    QLabel *titre = new QLabel(" Nouvelle tâche à planifier");
    titre->setStyleSheet("font-size: 20px; font-weight: bold; color: #225A64; background: transparent;");
    layout->addWidget(titre);

    QFormLayout *form = new QFormLayout;
    form->setSpacing(8);

    QLineEdit *editTitre = new QLineEdit;
    editTitre->setObjectName("employeeTaskTitle");
    editTitre->setPlaceholderText("Ex. Consultation, soins, accueil...");
    editTitre->setMinimumHeight(34);
    form->addRow("Tâche *", editTitre);

    QLineEdit *editCompetence = new QLineEdit;
    editCompetence->setObjectName("employeeTaskSkill");
    editCompetence->setPlaceholderText("Ex. Soins, Chirurgie, Accueil");
    editCompetence->setMinimumHeight(34);
    form->addRow("Spécialité requise *", editCompetence);

    QDateTime start = QDateTime::currentDateTime().addSecs(3600);
    start.setTime(QTime(start.time().hour(), start.time().minute(), 0));
    QDateTime end = start.addSecs(2 * 3600);
    QDateTimeEdit *dateDebut = new QDateTimeEdit(start);
    dateDebut->setObjectName("employeeTaskStart");
    dateDebut->setCalendarPopup(true);
    dateDebut->setDisplayFormat("dd/MM/yyyy HH:mm");
    dateDebut->setMinimumHeight(34);
    form->addRow("Début", dateDebut);

    QDateTimeEdit *dateFin = new QDateTimeEdit(end);
    dateFin->setObjectName("employeeTaskEnd");
    dateFin->setCalendarPopup(true);
    dateFin->setDisplayFormat("dd/MM/yyyy HH:mm");
    dateFin->setMinimumHeight(34);
    form->addRow("Fin", dateFin);

    QDoubleSpinBox *heures = new QDoubleSpinBox;
    heures->setRange(0.5, 24.0);
    heures->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    heures->setAccelerated(true);
    heures->setSingleStep(0.5);
    heures->setValue(2.0);
    heures->setSuffix(" h");
    heures->setMinimumHeight(34);
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
    buttons->button(QDialogButtonBox::Cancel)->setText("Annuler");
    buttons->button(QDialogButtonBox::Cancel)->setMinimumHeight(42);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);

    if (dlg.exec() != QDialog::Accepted) return;
    const QString nomTache = editTitre->text().trimmed();
    const QString competence = editCompetence->text().trimmed();
    const QDateTime debut = dateDebut->dateTime();
    const QDateTime fin = dateFin->dateTime();
    if (nomTache.isEmpty() || competence.isEmpty()) {
        QMessageBox::warning(this, "Champs obligatoires", "Saisissez le nom de la tâche et la spécialité requise.");
        return;
    }
    if (!debut.isValid() || !fin.isValid() || fin <= debut) {
        QMessageBox::warning(this, "Dates invalides", "La fin doit être postérieure au début.");
        return;
    }

    const QDate debutSemaine = semaineDebut(debut.date());
    const QString semaineDebutSql = borneSemaine(debutSemaine);
    const QString semaineFinSql = borneSemaine(debutSemaine.addDays(7));
    const QString debutSql = debut.toString("yyyy-MM-dd HH:mm:ss");
    const QString finSql = fin.toString("yyyy-MM-dd HH:mm:ss");

    auto employes = EmployeeQueries::candidates();
    int meilleurId = -1;
    QString meilleurNom;
    double meilleurTaux = 1e9;
    while (employes.next()) {
        const int id = employes.value(0).toInt();
        const QString poste = employes.value(3).toString();
        const QString competences = employes.value(4).toString();
        if (!competences.contains(competence, Qt::CaseInsensitive) &&
            !poste.contains(competence, Qt::CaseInsensitive))
            continue;

        QString planningError;
        if(!Service::available(id,debut,fin,0,&planningError))continue;

        auto planifie = EmployeeQueries::plannedHours(id, semaineDebutSql, semaineFinSql);
        if (!planifie.isOk() || !planifie.next()) continue;

        const double charge = employes.value(5).toDouble() + planifie.value(0).toDouble();
        const double capacite = employes.value(6).toDouble();
        const double chargeNouvelle = heures->value();
        if (capacite <= 0 || charge + chargeNouvelle > capacite) continue;
        const double tauxPrevu = (charge + chargeNouvelle) / capacite;
        if (tauxPrevu < meilleurTaux) {
            meilleurTaux = tauxPrevu;
            meilleurId = id;
            meilleurNom = employes.value(2).toString() + " " + employes.value(1).toString();
        }
    }

    if (meilleurId < 0) {
        QMessageBox::warning(this, "Aucun responsable disponible",
                             "Aucune personne disponible ne possède cette spécialité avec une capacité suffisante "
                             "et sans conflit horaire. Essayez une autre date, une autre spécialité ou une charge plus faible.");
        return;
    }

    auto insert = EmployeeQueries::addTask(nomTache, competence, debutSql, finSql, heures->value(), meilleurId);
    if (!insert.isOk()) {
        QMessageBox::critical(this, "Erreur d'enregistrement",
                              insert.lastError().nativeErrorCode().contains("20051")
                              ? "Ce créneau vient d’être réservé. Relancez l’affectation ou choisissez un autre horaire."
                              : "La tâche n'a pas été enregistrée : " + insert.lastError().text());
        return;
    }

    mettreAJourStats();
    QString message = QString("Tâche « %1 » enregistrée et affectée à %2.\n"
                              "Créneau : %3 – %4\nCharge prévue après affectation : %5 %%. ")
                          .arg(nomTache, meilleurNom, debut.toString("dd/MM/yyyy HH:mm"),
                               fin.toString("dd/MM/yyyy HH:mm"))
                          .arg(qRound(meilleurTaux * 100));
    if (meilleurTaux > 0.90)
        message += "\n Attention : charge supérieure à 90 %, rééquilibrage conseillé.";
    QMessageBox::information(this, "Affectation automatique", message);
}

void GestionEmployes::detecterConflitsPlanning()
{
    const QStringList conflits = conflitsPlanning();
    if (conflits.isEmpty()) {
        QMessageBox::information(this, "Vérification du planning",
                                 " Aucun chevauchement détecté entre les tâches affectées.");
        return;
    }
    QMessageBox::warning(this, "Conflits de planification détectés",
                         "Les affectations suivantes se chevauchent :\n\n" + conflits.join("\n\n"));
}


void GestionEmployes::exportLeaveCertificate() {
        QDialog dlg(this);
        dlg.setObjectName("employeeLeaveDialog");
        dlg.setWindowTitle("Attestation de congé");
        dlg.setMinimumWidth(490);
        QVBoxLayout *dialogLayout = new QVBoxLayout(&dlg);
        dialogLayout->setContentsMargins(24, 24, 24, 24);
        dialogLayout->setSpacing(14);
        QLabel *titre = new QLabel(" Attestation de congé");
        titre->setStyleSheet("font-size: 20px; font-weight: bold; color: #225A64; background: transparent;");
        dialogLayout->addWidget(titre);
        QFormLayout *form = new QFormLayout;
        form->setSpacing(12);
        QComboBox *employe = new QComboBox;
        auto qEmployes = EmployeeQueries::certificateList();
        if (qEmployes.isOk()) {
            while (qEmployes.next()) {
                const QString label = qEmployes.value(2).toString() + " " + qEmployes.value(1).toString()
                                      + " — " + qEmployes.value(3).toString();
                employe->addItem(label, qEmployes.value(0).toInt());
            }
        }
        employe->setMinimumHeight(40);
        form->addRow("Employé", employe);
        QDateEdit *dateDebut = new QDateEdit(QDate::currentDate());
        dateDebut->setCalendarPopup(true);
        dateDebut->setDisplayFormat("dd/MM/yyyy");
        dateDebut->setMinimumHeight(40);
        form->addRow("Début du congé", dateDebut);
        QDateEdit *dateFin = new QDateEdit(QDate::currentDate().addDays(7));
        dateFin->setCalendarPopup(true);
        dateFin->setDisplayFormat("dd/MM/yyyy");
        dateFin->setMinimumHeight(40);
        form->addRow("Fin du congé", dateFin);
        QComboBox *typeConge = new QComboBox;
        typeConge->addItems({"Congé annuel", "Congé exceptionnel", "Autre congé"});
        typeConge->setMinimumHeight(40);
        form->addRow("Type de congé", typeConge);
        dialogLayout->addLayout(form);
        QLabel *info = new QLabel("Le document PDF sera créé à l'emplacement choisi.");
        info->setStyleSheet("color: #657B80; background: transparent;");
        dialogLayout->addWidget(info);
        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
        buttons->button(QDialogButtonBox::Save)->setText("Créer le PDF");
        buttons->button(QDialogButtonBox::Save)->setMinimumHeight(42);
        buttons->button(QDialogButtonBox::Cancel)->setText("Annuler");
    buttons->button(QDialogButtonBox::Cancel)->setMinimumHeight(42);
        connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        dialogLayout->addWidget(buttons);
        if (employe->count() == 0) {
            buttons->button(QDialogButtonBox::Save)->setEnabled(false);
            QMessageBox::warning(&dlg, "Aucun employé", "Ajoutez d'abord un employé avant de créer une attestation.");
        }
        if (dlg.exec() != QDialog::Accepted) return;
        if (dateFin->date() < dateDebut->date()) {
            QMessageBox::warning(this, "Dates invalides", "La fin du congé doit être égale ou postérieure au début.");
            return;
        }

        const int id = employe->currentData().toInt();
        auto q = EmployeeQueries::certificate(id);
        if (!q.isOk() || !q.next()) {
            QMessageBox::critical(this, "Erreur", "Impossible de charger les informations de l'employé.");
            return;
        }
        const QString nomComplet = q.value(1).toString() + " " + q.value(0).toString();
        const QString poste = q.value(2).toString();
        const QString dateEmbauche = QDate::fromString(q.value(3).toString(), "yyyy-MM-dd").toString("dd/MM/yyyy");
        QString fichier = QFileDialog::getSaveFileName(this, "Enregistrer l'attestation de congé",
            QString("Attestation_conge_%1.pdf").arg(nomComplet.simplified().replace(' ', '_')),
            "Fichiers PDF (*.pdf)");
        if (fichier.isEmpty()) return;
        if (!fichier.endsWith(".pdf", Qt::CaseInsensitive)) fichier += ".pdf";

        const QString html = QString(
            "<html><body style='font-family:Arial,sans-serif;color:#263B42;'>"
            "<div style='text-align:center;color:#225A64;font-size:13pt;font-weight:bold;'>ANIWORLD — CENTRE DE SOINS</div>"
            "<hr style='border:1px solid #7FB8C4;'>"
            "<h1 style='text-align:center;color:#225A64;margin-top:45px;'>ATTESTATION DE CONGÉ</h1>"
            "<p style='margin-top:35px;font-size:12pt;line-height:1.8;'>"
            "Nous attestons par la présente que <b>%1</b>, occupant le poste de <b>%2</b>"
            "%3, bénéficie d'un congé de type <b>%4</b>, pour la période du <b>%5</b> au <b>%6</b>."
            "</p><p style='margin-top:20px;'>Cette attestation est délivrée pour servir et valoir ce que de droit.</p>"
            "<p style='margin-top:65px;text-align:right;'>Fait le %7<br><br>Signature et cachet du responsable</p>"
            "</body></html>")
            .arg(nomComplet.toHtmlEscaped(), poste.toHtmlEscaped(),
                 dateEmbauche.isEmpty() ? QString() : QString(" (embauché(e) le <b>%1</b>)").arg(dateEmbauche),
                 typeConge->currentText().toHtmlEscaped(),
                 dateDebut->date().toString("dd/MM/yyyy"), dateFin->date().toString("dd/MM/yyyy"),
                 QDate::currentDate().toString("dd/MM/yyyy"));
        QString error;
        if (Invoice::writeDocument(fichier, html, "Attestation de congé", &error)) {
            QMessageBox::information(this, "Attestation créée", " Attestation de congé PDF créée :\n" + fichier);
        } else {
            QMessageBox::critical(this, "Échec de création", error);
        }
}
