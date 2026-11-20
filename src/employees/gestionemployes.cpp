#include <QJsonArray>
#include <QJsonObject>
#include <memory>
#include <QStandardItemModel>
#include "../ui/formvalidation.h"
#include "planning.h"
#include <QToolButton>
#include <QStackedWidget>
#include <QTextEdit>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QPrinter>
#include <QTextDocument>
#include <QStandardPaths>
#include <QDir>
#include <QRegularExpression>
#include <QDate>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTableWidgetItem>
#include <QSizePolicy>
#include <QPaintEvent>
#include <QVector>
#include <cmath>
#include <algorithm>
#include "employeeevaluation.h"
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
    m_detailCin=ui.Employees_m_detailCin;m_detailCin->setTextFormat(Qt::PlainText);
    m_detailNaissance=ui.Employees_m_detailNaissance;m_detailNaissance->setTextFormat(Qt::PlainText);
    m_detailAdresse=ui.Employees_m_detailAdresse;m_detailAdresse->setTextFormat(Qt::PlainText);
    m_detailService=ui.Employees_m_detailService;m_detailService->setTextFormat(Qt::PlainText);
    m_detailContrat=ui.Employees_m_detailContrat;m_detailContrat->setTextFormat(Qt::PlainText);
    m_detailSalaire=ui.Employees_m_detailSalaire;m_detailSalaire->setTextFormat(Qt::PlainText);

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
    connect(ui.employeeEvaluation,&QPushButton::clicked,this,&GestionEmployes::ouvrirEvaluationEmployes);
    connect(ui.employeeTaskPlanning,&QPushButton::clicked,this,&GestionEmployes::ouvrirPlanificationEquipe);
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

    m_detailCin->setText(e.getCin().isEmpty()?"-":e.getCin());
    m_detailNaissance->setText(e.getDateNaissance().isValid()?e.getDateNaissance().toString("dd/MM/yyyy"):"-");
    m_detailAdresse->setText(e.getAdresse().isEmpty()?"-":e.getAdresse());
    m_detailService->setText(Employe::libelleService(e.getServiceId()));
    m_detailContrat->setText(e.getTypeContrat().isEmpty()?"-":e.getTypeContrat());
    m_detailSalaire->setText(e.getSalaire()<0?"Non renseigné":QString::number(e.getSalaire(),'f',2)+" DT");
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

            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

// ==================== DIALOG FORMULAIRE ====================
bool GestionEmployes::ouvrirDialog(Employe &e, bool modification)
{
    if(!OracleConnection::isOracle())return ouvrirDialogDemo(e,modification);
    QDialog dlg(this);dlg.setObjectName("employeeForm");
    using VF=FormValidation::Field;
    struct FieldState {int state=0;QString message;};
    auto states=std::make_shared<QHash<QWidget*,FieldState>>();
    QHash<QWidget*,VF*> feedback;

    dlg.setWindowTitle(modification ? "Modifier l'employé" : "Nouvel employé");
    const QRect zoneFormulaire = QGuiApplication::primaryScreen()
                                     ? QGuiApplication::primaryScreen()->availableGeometry()
                                     : QRect(0, 0, 1280, 800);
    dlg.resize(qMin(1040, zoneFormulaire.width() - 60), qMin(700, zoneFormulaire.height() - 70));
    dlg.setMinimumSize(qMin(900, zoneFormulaire.width() - 30), qMin(610, zoneFormulaire.height() - 30));
    dlg.setStyleSheet("QDialog { background-color: #F0F4F8; }");

    QVBoxLayout *mainL = new QVBoxLayout(&dlg);
    mainL->setContentsMargins(24, 20, 24, 18);
    mainL->setSpacing(12);

    QLabel *titre = new QLabel(modification ? "✏️  Modifier l'employé" : "➕  Nouvel employé");
    titre->setStyleSheet("font-size: 20px; font-weight: bold; color: #225A64;"
                         "background: transparent;");
    mainL->addWidget(titre);
    auto *idLabel=new QLineEdit(modification?QString::number(e.getId()):"Automatique");
    idLabel->setObjectName("employeeId");idLabel->setReadOnly(true);idLabel->hide();idLabel->setParent(&dlg);


    QGridLayout *form = new QGridLayout;
    form->setContentsMargins(0, 4, 0, 4);
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(12);
    form->setColumnMinimumWidth(0, 118);
    form->setColumnMinimumWidth(2, 138);
    form->setColumnStretch(1, 1);
    form->setColumnStretch(3, 1);
    auto ajouterChamp = [&](int ligne, int colonne, const QString &libelle, QWidget *champ) {
        QLabel *label = new QLabel(libelle);
        label->setMinimumWidth(colonne == 0 ? 112 : 132);
        label->setStyleSheet("font-size: 13px; color: #244B56; background: transparent;");
        form->addWidget(label, ligne, colonne, Qt::AlignVCenter);
        auto *validated=FormValidation::bind(champ,[states,champ] {
            const auto value=states->value(champ);
            return value.state<0?VF::invalid(value.message):value.state>0?VF::valid():VF::neutral();
        });
        feedback.insert(champ,validated);
        form->addWidget(validated, ligne, colonne + 1);
    };

    QLineEdit *editNom = new QLineEdit(e.getNom());
    editNom->setObjectName("employeeLastName");
    editNom->setMinimumHeight(38);
    editNom->setMaxLength(100);
    editNom->setPlaceholderText("Nom *");
    ajouterChamp(0, 0, "Nom *", editNom);

    QLineEdit *editPrenom = new QLineEdit(e.getPrenom());
    editPrenom->setObjectName("employeeFirstName");
    editPrenom->setMinimumHeight(38);
    editPrenom->setMaxLength(100);
    editPrenom->setPlaceholderText("Prénom *");
    ajouterChamp(1, 0, "Prénom *", editPrenom);

    QLineEdit *editCin = new QLineEdit(e.getCin());
    editCin->setObjectName("employeeCin");
    editCin->setMinimumHeight(38);
    editCin->setMaxLength(8);
    editCin->setPlaceholderText("8 chiffres *");
    ajouterChamp(2, 0, "CIN *", editCin);

    // Les dates s'affichent toujours comme de vraies dates (jj/MM/aaaa).
    // Naissance : par defaut aujourd'hui => rouge tant que l'utilisateur ne choisit pas
    // une date valide (16 ans minimum).
    const QDate dateMin(1930, 1, 1);
    QDateEdit *dateNaiss = new QDateEdit;
    dateNaiss->setObjectName("employeeBirthDate");
    dateNaiss->setMinimumHeight(38);
    dateNaiss->setCalendarPopup(true);
    dateNaiss->setDisplayFormat("dd/MM/yyyy");
    dateNaiss->setMinimumDate(dateMin);
    dateNaiss->setMaximumDate(QDate::currentDate());
    dateNaiss->setDate(e.getDateNaissance().isValid() ? e.getDateNaissance() : QDate::currentDate());
    ajouterChamp(3, 0, "Date de naissance *", dateNaiss);

    QLineEdit *editAdresse = new QLineEdit(e.getAdresse());
    editAdresse->setObjectName("employeeAddress");
    editAdresse->setMinimumHeight(38);
    editAdresse->setMaxLength(250);
    ajouterChamp(4, 0, "Adresse", editAdresse);

    QLineEdit *editTel = new QLineEdit(e.getTelephone());
    editTel->setObjectName("employeePhone");
    editTel->setMinimumHeight(38);
    editTel->setMaxLength(20);
    editTel->setPlaceholderText("+216 20 000 000");
    ajouterChamp(5, 0, modification ? "Tél" : "Tél *", editTel);

    QLineEdit *editCourriel = new QLineEdit(e.getCourriel());
    editCourriel->setObjectName("employeeEmail");
    editCourriel->setMinimumHeight(38);
    editCourriel->setMaxLength(150);
    editCourriel->setPlaceholderText("nom.prenom@gmail.com");
    ajouterChamp(6, 0, modification ? "Courriel" : "Courriel *", editCourriel);

    QComboBox *comboPoste = new QComboBox;
    comboPoste->setObjectName("employeeJob");
    comboPoste->setMinimumHeight(38);
    comboPoste->addItem("— Choisir un poste —", QString());
    comboPoste->addItems({"Vétérinaire", "Assistant(e) vétérinaire",
                          "Auxiliaire vétérinaire", "Secrétaire / Réceptionniste",
                          "Agent d'entretien", "Aide aux soins animaliers"});
    if (!e.getPoste().isEmpty()) {
        if (comboPoste->findText(e.getPoste()) < 0)
            comboPoste->addItem(e.getPoste());
        comboPoste->setCurrentText(e.getPoste());
    } else {
        comboPoste->setCurrentIndex(0);
    }
    ajouterChamp(0, 2, "Poste *", comboPoste);

    // Service obligatoire (EMPLOYER.ID_SERV est NOT NULL). La liste vient de la
    // table SERVICE (ex. « SRV-005 — Soins & bien-être »).
    QComboBox *comboService = new QComboBox;
    comboService->setObjectName("employeeService");
    comboService->setMinimumHeight(38);
    comboService->addItem("— Choisir un service —", 0);
    const QList<Employe::ServiceInfo> services = Employe::listerServices();
    for (const Employe::ServiceInfo &sv : services) {
        const int index = comboService->count();
        comboService->addItem(sv.libelle, sv.id);
        if (sv.id <= 0) {
            if (QStandardItemModel *model = qobject_cast<QStandardItemModel*>(comboService->model())) {
                if (QStandardItem *item = model->item(index)) {
                    item->setEnabled(false);
                    item->setToolTip("Cette catégorie n'a pas encore de ligne correspondante dans SERVICE.");
                }
            }
        }
    }
    if (e.getServiceId() > 0) {
        int idx = comboService->findData(e.getServiceId());
        if (idx < 0) {
            comboService->addItem(Employe::libelleService(e.getServiceId()), e.getServiceId());
            idx = comboService->count() - 1;
        }
        comboService->setCurrentIndex(idx);
    } else {
        comboService->setCurrentIndex(0);
    }
    ajouterChamp(6, 2, "Service *", comboService);

    QComboBox *comboContrat = new QComboBox;
    comboContrat->setObjectName("employeeContract");
    comboContrat->setMinimumHeight(38);
    comboContrat->addItem("— Choisir un type de contrat —", QString());
    comboContrat->addItems({"CDI", "CDD", "CIVP", "Stage", "Contractuel"});
    if (!e.getTypeContrat().isEmpty()) {
        if (comboContrat->findText(e.getTypeContrat()) < 0)
            comboContrat->addItem(e.getTypeContrat());
        comboContrat->setCurrentText(e.getTypeContrat());
    } else {
        comboContrat->setCurrentIndex(0);
    }
    ajouterChamp(2, 2, "Type de contrat *", comboContrat);

    QDateEdit *dateEmb = new QDateEdit;
    dateEmb->setObjectName("employeeHireDate");
    dateEmb->setMinimumHeight(38);
    dateEmb->setCalendarPopup(true);
    dateEmb->setDisplayFormat("dd/MM/yyyy");
    dateEmb->setMinimumDate(dateMin);
    dateEmb->setMaximumDate(QDate::currentDate());
    dateEmb->setDate(e.getDateEmbauche().isValid() ? e.getDateEmbauche() : QDate::currentDate());
    ajouterChamp(1, 2, "Date d'embauche *", dateEmb);

    QLineEdit *editSalaire = new QLineEdit(e.getSalaire() >= 0
                                               ? QString::number(e.getSalaire(), 'f', 2) : QString());
    editSalaire->setObjectName("employeeSalary");
    editSalaire->setMinimumHeight(38);
    editSalaire->setPlaceholderText("Ex. : 1500");
    ajouterChamp(3, 2, modification ? "Salaire (DT)" : "Salaire (DT) *", editSalaire);

    QDoubleSpinBox *spinCharge = new QDoubleSpinBox;
    spinCharge->setObjectName("employeeWorkload");
    spinCharge->setMinimumHeight(38);
    spinCharge->setRange(0, 60);
    spinCharge->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    spinCharge->setSingleStep(1.0);
    spinCharge->setAccelerated(true);
    spinCharge->setSuffix(" h");
    spinCharge->setValue(e.getChargeHebdo());
    ajouterChamp(4, 2, "Charge (h / semaine)", spinCharge);

    // DISPO_REPOS est un NUMBER(8,2) dans Oracle : heures disponibles par semaine.
    QDoubleSpinBox *spinDispo = new QDoubleSpinBox;
    spinDispo->setObjectName("employeeCapacity");
    spinDispo->setMinimumHeight(38);
    spinDispo->setRange(1, 60);
    spinDispo->setSingleStep(1.0);
    spinDispo->setSuffix(" h");
    spinDispo->setValue(e.getDispoHebdo());          // 35 h par defaut
    ajouterChamp(5, 2, "Disponibilité (h / semaine)", spinDispo);

    QLineEdit *editComp = new QLineEdit(e.getCompetences());
    editComp->setObjectName("employeeSkills");
    editComp->setMinimumHeight(38);
    editComp->setMaxLength(300);
    editComp->setPlaceholderText("Soins, Toilettage...");
    ajouterChamp(7, 0, "Compétences", editComp);

    QCheckBox *checkDispo = new QCheckBox("Disponible");
    checkDispo->setTristate(false);
    checkDispo->setChecked(modification ? e.isDisponible() : true);
    form->addWidget(checkDispo, 7, 3, Qt::AlignLeft | Qt::AlignVCenter);

    // Le formulaire est long : il defile dans la fenetre au lieu de depasser l'ecran.
    QWidget *formContent = new QWidget;
    formContent->setStyleSheet("background: transparent;");
    formContent->setLayout(form);
    QScrollArea *formScroll = new QScrollArea;
    formScroll->setWidgetResizable(true);
    formScroll->setFrameShape(QFrame::NoFrame);
    formScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    formScroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    formScroll->setWidget(formContent);
    formScroll->setMinimumHeight(470);
    mainL->addWidget(formScroll, 1);
    mainL->addSpacing(4);

    QDialogButtonBox *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btns->button(QDialogButtonBox::Ok)->setText(modification ? "Modifier" : "Ajouter");
    btns->button(QDialogButtonBox::Ok)->setMinimumHeight(46);
    btns->button(QDialogButtonBox::Ok)->setMinimumWidth(120);
    btns->button(QDialogButtonBox::Cancel)->setText("Annuler");
    btns->button(QDialogButtonBox::Cancel)->setMinimumHeight(46);
    btns->button(QDialogButtonBox::Cancel)->setMinimumWidth(120);
    // Construit un Employe a partir des champs du formulaire (id conserve)
    auto lireFormulaire = [&]() {
        Employe saisie = e;
        saisie.setNom(editNom->text().trimmed());
        saisie.setPrenom(editPrenom->text().trimmed());
        saisie.setCin(editCin->text().trimmed());
        saisie.setDateNaissance(dateNaiss->date());
        saisie.setAdresse(editAdresse->text().trimmed());
        saisie.setTelephone(Employe::telephoneValide(editTel->text())
                          ? Employe::normaliserTelephone(editTel->text().trimmed())
                          : editTel->text().trimmed());
        saisie.setCourriel(editCourriel->text().trimmed());
        saisie.setPoste(comboPoste->currentIndex() <= 0 ? QString() : comboPoste->currentText());
        saisie.setServiceId(comboService->currentData().toInt());
        saisie.setTypeContrat(comboContrat->currentIndex() <= 0 ? QString() : comboContrat->currentText());
        saisie.setDateEmbauche(dateEmb->date());
        QString salaireTexte = editSalaire->text().trimmed();
        salaireTexte.replace(',', '.');
        saisie.setSalaire(salaireTexte.isEmpty()
                              ? (modification ? e.getSalaire() : -1.0)
                              : salaireTexte.toDouble());
        saisie.setChargeHebdo(spinCharge->value());
        saisie.setDispoHebdo(spinDispo->value());
        saisie.setDisponible(checkDispo->isChecked());
        saisie.setCompetences(editComp->text().trimmed());
        return saisie;
    };

    // Validation visuelle en temps reel : rouge = invalide, vert = valide,
    // bordure normale = champ facultatif encore vide.
    QHash<QWidget*, int> etatsChamps;
    QPushButton *btnValider = btns->button(QDialogButtonBox::Ok);
    auto actualiserBouton = [&]() {
        bool toutValide = true;
        for (auto it = etatsChamps.constBegin(); it != etatsChamps.constEnd(); ++it)
            if (it.value() < 0) { toutValide = false; break; }
        btnValider->setEnabled(toutValide);
        btnValider->setToolTip(toutValide ? QString()
                                          : QStringLiteral("Remplissez correctement tous les champs obligatoires."));
    };
    auto definirEtatChamp = [&](QWidget *champ, int etat, const QString &message = QString()) {
        if (!champ) return;
        etatsChamps.insert(champ, etat);
        actualiserBouton();
        champ->setToolTip(message);
        (*states)[champ]={etat,message};
        if(auto *field=feedback.value(champ))field->revalidate();
    };

    const QRegularExpression reCin(QStringLiteral("^[0-9]{8}$"));
    const QRegularExpression reSalaire(QStringLiteral("^[0-9]+([.,][0-9]{1,2})?$"));
    const QRegularExpression reNombre(QStringLiteral("^[0-9]+([.,][0-9]+)?$"));
    const int idExclu = modification ? e.getId() : 0;

    // Nom / prenom : lettres => vert, chiffres ou symboles => rouge.
    auto actualiserNom = [&]() {
        const bool ok = Employe::nomValide(editNom->text());
        definirEtatChamp(editNom, ok ? 1 : -1,
                         ok ? QString() : QStringLiteral("Le nom doit contenir uniquement des lettres."));
    };
    auto actualiserPrenom = [&]() {
        const bool ok = Employe::nomValide(editPrenom->text());
        definirEtatChamp(editPrenom, ok ? 1 : -1,
                         ok ? QString() : QStringLiteral("Le prénom doit contenir uniquement des lettres."));
    };
    auto actualiserCin = [&]() {
        const QString valeur = editCin->text().trimmed();
        if (!reCin.match(valeur).hasMatch()) {
            definirEtatChamp(editCin, -1, QStringLiteral("Le CIN doit contenir exactement 8 chiffres."));
            return;
        }
        bool requeteOk = false;
        const bool libre = Employe::cinDisponible(valeur, idExclu, &requeteOk);
        if (!requeteOk) {
            definirEtatChamp(editCin, -1, QStringLiteral("Impossible de vérifier le CIN dans Oracle."));
        } else if (!libre) {
            definirEtatChamp(editCin, -1, QStringLiteral("Ce CIN est déjà utilisé par un autre employé."));
        } else {
            definirEtatChamp(editCin, 1);
        }
    };
    auto actualiserNaissance = [&]() {
        const QDate date = dateNaiss->date();
        const bool ok = date.isValid() && date.year() >= 1930 &&
                        date.addYears(16) <= QDate::currentDate();
        definirEtatChamp(dateNaiss, ok ? 1 : -1,
                         ok ? QString() : QStringLiteral("Choisissez une date valide : l'employé doit avoir au moins 16 ans."));
    };
    auto actualiserAdresse = [&]() {
        const QString valeur = editAdresse->text().trimmed();
        if (valeur.isEmpty()) definirEtatChamp(editAdresse, 0);
        else definirEtatChamp(editAdresse, valeur.size() <= 250 ? 1 : -1,
                              valeur.size() <= 250 ? QString() : QStringLiteral("250 caractères maximum."));
    };
    auto actualiserTelephone = [&]() {
        const QString valeur = editTel->text().trimmed();
        if (valeur.isEmpty() && modification) {
            definirEtatChamp(editTel, 0);
            return;
        }
        const bool ok = Employe::telephoneValide(valeur);
        definirEtatChamp(editTel, ok ? 1 : -1,
                         ok ? QString() : QStringLiteral("Le téléphone doit contenir exactement 8 chiffres."));
    };
    auto actualiserCourriel = [&]() {
        const QString valeur = editCourriel->text().trimmed();
        if (valeur.isEmpty() && modification) {
            definirEtatChamp(editCourriel, 0);
            return;
        }
        if (!Employe::gmailValide(valeur)) {
            definirEtatChamp(editCourriel, -1,
                             QStringLiteral("Saisissez une adresse Gmail valide (ex. nom.prenom@gmail.com)."));
            return;
        }
        bool requeteOk = false;
        const bool libre = Employe::courrielDisponible(valeur, idExclu, &requeteOk);
        if (!requeteOk)
            definirEtatChamp(editCourriel, -1, QStringLiteral("Impossible de vérifier le courriel dans Oracle."));
        else if (!libre)
            definirEtatChamp(editCourriel, -1, QStringLiteral("Ce courriel est déjà utilisé par un autre employé."));
        else
            definirEtatChamp(editCourriel, 1, "Format Gmail correct et courriel non utilisé. L'existence de la boîte mail n'est pas vérifiée.");
    };
    auto actualiserPoste = [&]() {
        const bool ok = comboPoste->currentIndex() > 0;
        definirEtatChamp(comboPoste, ok ? 1 : -1,
                         ok ? QString() : QStringLiteral("Choisissez un poste."));
    };
    auto actualiserDateEmbauche = [&]() {
        const QDate naissance = dateNaiss->date();
        const QDate embauche = dateEmb->date();
        const bool naissanceOk = naissance.isValid() && naissance.year() >= 1930 &&
                                 naissance.addYears(16) <= QDate::currentDate();
        const bool ok = naissanceOk && embauche.isValid() &&
                        embauche >= naissance.addYears(16) && embauche <= QDate::currentDate();
        definirEtatChamp(dateEmb, ok ? 1 : -1,
                         ok ? QString() : QStringLiteral("Choisissez une date d'embauche cohérente avec la date de naissance."));
    };
    auto actualiserContrat = [&]() {
        const bool ok = comboContrat->currentIndex() > 0;
        definirEtatChamp(comboContrat, ok ? 1 : -1,
                         ok ? QString() : QStringLiteral("Choisissez un type de contrat."));
    };
    auto actualiserSalaire = [&]() {
        const QString valeur = editSalaire->text().trimmed();
        if (valeur.isEmpty() && modification) {
            definirEtatChamp(editSalaire, 0);
            return;
        }
        QString normalisee = valeur;
        normalisee.replace(QLatin1Char(','), QLatin1Char('.'));
        bool conversionOk = false;
        const double montant = normalisee.toDouble(&conversionOk);
        const bool ok = reSalaire.match(valeur).hasMatch() && conversionOk &&
                        montant >= 0.0 && montant <= 1000000.0;
        definirEtatChamp(editSalaire, ok ? 1 : -1,
                         ok ? QString() : QStringLiteral("Salaire requis : nombre entre 0 et 1 000 000 DT."));
    };
    auto actualiserCharge = [&]() {
        const bool ok=spinCharge->hasAcceptableInput() && spinCharge->value()>0;
        definirEtatChamp(spinCharge, ok?1:-1, "La charge doit être supérieure à 0 et ne pas dépasser 60 heures.");
    };
    auto actualiserRepos = [&]() {
        definirEtatChamp(spinDispo, spinDispo->hasAcceptableInput()?1:-1, "Saisissez entre 1 et 60 heures.");
    };
    auto actualiserService = [&]() {
        const bool ok = comboService->currentData().toInt() > 0;
        definirEtatChamp(comboService, ok ? 1 : -1,
                         ok ? QString() : QStringLiteral("Choisissez un service (obligatoire)."));
    };
    auto actualiserCompetences = [&]() {
        const QString valeur = editComp->text().trimmed();
        if (valeur.isEmpty()) definirEtatChamp(editComp, 0);
        else definirEtatChamp(editComp, valeur.size() <= 300 ? 1 : -1,
                              valeur.size() <= 300 ? QString() : QStringLiteral("300 caractères maximum."));
    };

    connect(editNom, &QLineEdit::textChanged, &dlg, actualiserNom);
    connect(editPrenom, &QLineEdit::textChanged, &dlg, actualiserPrenom);
    connect(editCin, &QLineEdit::textChanged, &dlg, actualiserCin);
    connect(dateNaiss, &QDateEdit::dateChanged, &dlg, [&]() {
        actualiserNaissance();
        actualiserDateEmbauche();
    });
    connect(editAdresse, &QLineEdit::textChanged, &dlg, actualiserAdresse);
    connect(editTel, &QLineEdit::textChanged, &dlg, actualiserTelephone);
    connect(editCourriel, &QLineEdit::textChanged, &dlg, actualiserCourriel);
    connect(comboPoste, qOverload<int>(&QComboBox::currentIndexChanged), &dlg, actualiserPoste);
    connect(dateEmb, &QDateEdit::dateChanged, &dlg, actualiserDateEmbauche);
    connect(comboContrat, qOverload<int>(&QComboBox::currentIndexChanged), &dlg, actualiserContrat);
    connect(editSalaire, &QLineEdit::textChanged, &dlg, actualiserSalaire);
    connect(spinCharge, qOverload<double>(&QDoubleSpinBox::valueChanged), &dlg, actualiserCharge);
    connect(spinDispo, qOverload<double>(&QDoubleSpinBox::valueChanged), &dlg, actualiserRepos);
    connect(comboService, qOverload<int>(&QComboBox::currentIndexChanged), &dlg, actualiserService);
    connect(editComp, &QLineEdit::textChanged, &dlg, actualiserCompetences);

    // Calcule la validité sans colorer les champs avant la première saisie.
    actualiserNom();
    actualiserPrenom();
    actualiserCin();
    actualiserNaissance();
    actualiserAdresse();
    actualiserTelephone();
    actualiserCourriel();
    actualiserPoste();
    actualiserDateEmbauche();
    actualiserContrat();
    actualiserSalaire();
    actualiserCharge();
    actualiserRepos();
    actualiserService();
    actualiserCompetences();

    for(auto *field:feedback)field->resetTouched();

    actualiserBouton();

    // Controle de saisie (C++) : le dialogue reste ouvert tant que la saisie est invalide
    connect(btns, &QDialogButtonBox::accepted, &dlg, [&]() {
        actualiserCin();
        actualiserCourriel();
        actualiserBouton();
        FormValidation::validate(&dlg);
        if (!btnValider->isEnabled()) return;
        const QString erreurSaisie = lireFormulaire().validerSaisie(!modification, !modification);
        if (!erreurSaisie.isEmpty()) {
            QMessageBox::warning(&dlg, "Validation", erreurSaisie);
            return;
        }
        Employe saisie=lireFormulaire();QString erreur;
        if(!(modification?saisie.modifier(&erreur):saisie.ajouter(&erreur))) {
            QMessageBox::warning(&dlg,"Enregistrement impossible",erreur);return;
        }
        e=saisie;dlg.accept();
    });
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    mainL->addWidget(btns);

    if (dlg.exec() != QDialog::Accepted) return false;

    return true;
}

bool GestionEmployes::ouvrirDialogDemo(Employe &e, bool modification)
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

    QLineEdit *editPrenom = new QLineEdit(e.getPrenom());
    editPrenom->setObjectName("employeeFirstName");
    editPrenom->setMinimumHeight(34);
    editPrenom->setPlaceholderText("Prénom *");

    QLineEdit *editCourriel = new QLineEdit(e.getCourriel());
    editCourriel->setObjectName("employeeEmail");
    editCourriel->setMinimumHeight(34);

    QLineEdit *editTel = new QLineEdit(e.getTelephone());
    editTel->setObjectName("employeePhone");
    editTel->setMinimumHeight(34);

    QComboBox *comboPoste = new QComboBox;
    comboPoste->setMinimumHeight(34);
    comboPoste->addItems({"Vétérinaire", "Assistant(e) vétérinaire",
                          "Auxiliaire vétérinaire", "Secrétaire / Réceptionniste",
                          "Agent d'entretien", "Aide aux soins animaliers"});
    if (!e.getPoste().isEmpty()) comboPoste->setCurrentText(e.getPoste());
    auto *serviceChoice=new QComboBox(&dlg);serviceChoice->setObjectName("employeeService");
    RelationChoices::services(serviceChoice,OracleConnection::isOracle()?Employe::employeeService(e.getId()):0);

    QDateEdit *dateEmb = new QDateEdit(e.getDateEmbauche().isValid() ?
                                           e.getDateEmbauche() : QDate::currentDate());
    dateEmb->setMinimumHeight(34);
    dateEmb->setCalendarPopup(true);
    dateEmb->setDisplayFormat("dd/MM/yyyy");

    QDoubleSpinBox *spinCharge = new QDoubleSpinBox;
    spinCharge->setObjectName("employeeWorkload");
    spinCharge->setMinimumHeight(34);
    spinCharge->setRange(0, 60);
    spinCharge->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    spinCharge->setSingleStep(1.0);
    spinCharge->setAccelerated(true);
    spinCharge->setSuffix(" h");
    spinCharge->setValue(e.getChargeHebdo());

    QDoubleSpinBox *spinDispo = new QDoubleSpinBox;
    spinDispo->setObjectName("employeeCapacity");
    spinDispo->setMinimumHeight(34);
    spinDispo->setRange(0, 60);
    spinDispo->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    spinDispo->setSingleStep(1.0);
    spinDispo->setAccelerated(true);
    spinDispo->setSuffix(" h");
    spinDispo->setValue(e.getDispoHebdo() > 0 ? e.getDispoHebdo() : 35);

    QLineEdit *editComp = new QLineEdit(e.getCompetences());
    editComp->setObjectName("employeeSkills");
    editComp->setMinimumHeight(34);
    editComp->setPlaceholderText("Soins, Toilettage...");

    QCheckBox *checkDispo = new QCheckBox("Disponible");
    checkDispo->setTristate(false);
    // À la création, l’employé est disponible par défaut (case cochée).
    // En modification, on conserve le statut enregistré.
    checkDispo->setChecked(modification ? e.isDisponible() : true);

    auto *cin=new QLineEdit(e.getCin());cin->setObjectName("employeeCin");cin->setMaxLength(30);
    auto *birth=new QLineEdit(e.getDateNaissance().toString(Qt::ISODate));birth->setObjectName("employeeBirthDate");birth->setPlaceholderText("AAAA-MM-JJ (facultatif)");
    auto *address=new QLineEdit(e.getAdresse());address->setObjectName("employeeAddress");address->setMaxLength(500);
    auto *contract=new QLineEdit(e.getTypeContrat());contract->setObjectName("employeeContract");contract->setMaxLength(80);
    auto *salary=new QDoubleSpinBox;salary->setObjectName("employeeSalary");salary->setRange(-1,99999999);salary->setSpecialValueText("Non renseigné");salary->setSuffix(" DT");salary->setValue(e.getSalaire());

    namespace FV = FormValidation;
    const bool oracle = OracleConnection::isOracle();
    if (oracle) {
        auto *id = new QLineEdit(modification ? QString::number(e.getId()) : "Automatique", &dlg);
        id->setObjectName("employeeId");
        id->setReadOnly(true);
        form->addRow("ID employé", id);
    }
    auto nameProblem = [](const QString &value) -> QString {
        static const QRegularExpression letter(R"(\p{L})");
        return value.contains(letter) ? QString() : QString("Le nom doit contenir une lettre.");
    };
    form->addRow("Nom *", FV::text(editNom, 120, true, nameProblem));
    form->addRow("Prénom *", FV::text(editPrenom, 120, true, nameProblem));
    if (oracle) {
        form->addRow("CIN (unique)", FV::text(cin, 30, false, [id=e.getId()](const QString &value) -> QString {
            auto result = EmployeeQueries::cinInUse(value, id);
            if (!result.isOk() || !result.next()) return "Vérification du CIN indisponible.";
            return result.value(0).toInt() ? QString("Ce CIN appartient déjà à un employé.") : QString();
        }));
        form->addRow("Date de naissance", FV::text(birth, 10, false, [](const QString &value) -> QString {
            const auto date = QDate::fromString(value, Qt::ISODate);
            return date.isValid() && date.toString(Qt::ISODate) == value ? QString() : QString("Date attendue au format AAAA-MM-JJ.");
        }));
        form->addRow("Adresse", FV::text(address, 500, false));
    } else {
        for (QWidget *w : QList<QWidget *>{cin, birth, address, contract, salary, serviceChoice}) {
            w->setParent(&dlg);
            w->hide();
        }
    }
    form->addRow("Téléphone", FV::text(editTel, 60, false, [](const QString &value) -> QString {
        static const QRegularExpression phone(R"(^\+?(?=[\d ()./-]*\d)[\d ()./-]+$)");
        return phone.match(value).hasMatch() ? QString() : QString("Saisissez un numéro de téléphone, avec son indicatif si nécessaire.");
    }));
    form->addRow("Poste", FV::choice(comboPoste));
    form->addRow("Courriel", FV::text(editCourriel, 250, false, [](const QString &value) -> QString {
        static const QRegularExpression email(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");
        return email.match(value).hasMatch() ? QString() : QString("Saisissez une adresse courriel valide.");
    }));
    form->addRow("Compétences", FV::text(editComp, 2000, false));
    form->addRow("Date d'embauche", FV::spin(dateEmb, "Complétez la date d’embauche."));
    if (oracle) {
        form->addRow("Type de contrat", FV::text(contract, 80, false));
        form->addRow("Salaire", FV::bind(salary, [salary] {
            if (salary->text() == salary->specialValueText()) return FV::Field::neutral();
            if (!salary->hasAcceptableInput() || salary->value() < 0)
                return FV::Field::invalid("Le salaire doit être positif ou nul.");
            return FV::Field::valid();
        }, false));
    }
    form->addRow("Charge hebdo", FV::bind(spinCharge,[spinCharge] {
        return spinCharge->hasAcceptableInput() && spinCharge->value()>0
            ? FV::Field::valid() : FV::Field::invalid("La charge doit être supérieure à 0 et ne pas dépasser 60 heures.");
    }));
    form->addRow("Disponible", FV::bind(checkDispo, [] { return FV::Field::valid(); }));
    form->addRow("Disponibilité / repos", FV::spin(spinDispo, "Saisissez entre 0 et 60 heures."));
    if (oracle) form->addRow("Service *", FV::choice(serviceChoice, true));
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
        validation->clear();
        if (!FV::validate(profile)) return;
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

    Employe e;
    if (!Employe::charger(m_idSelectionne, e)) return;

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

void GestionEmployes::ouvrirEvaluationEmployes()
{
    QDialog dlg(this);dlg.setObjectName("employeeEvaluationDialog");
    dlg.setWindowTitle("Performance & Évaluation des employés");
    const QRect zoneEvaluation = QGuiApplication::primaryScreen()
                                     ? QGuiApplication::primaryScreen()->availableGeometry()
                                     : QRect(0, 0, 1280, 800);
    dlg.resize(qMin(1080, zoneEvaluation.width() - 70), qMin(760, zoneEvaluation.height() - 70));
    dlg.setMinimumSize(qMin(900, zoneEvaluation.width() - 30), qMin(620, zoneEvaluation.height() - 30));
    dlg.setStyleSheet("QDialog { background-color: #F0F4F8; }");

    QVBoxLayout *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(24, 22, 24, 20);
    root->setSpacing(12);

    QLabel *titre = new QLabel("⭐  Performance & Évaluation des employés");
    titre->setStyleSheet("font-size: 22px; font-weight: bold; color: #225A64; background: transparent;");
    root->addWidget(titre);

    QLabel *sousTitre = new QLabel(
        "Sélectionnez un employé pour consulter ou remplir son évaluation professionnelle.");
    sousTitre->setWordWrap(true);
    sousTitre->setStyleSheet("font-size: 13px; color: #71858B; background: transparent;");
    root->addWidget(sousTitre);

    QStackedWidget *pages = new QStackedWidget(&dlg);
    root->addWidget(pages, 1);

    // -------------------- PAGE 1 : indicateurs + liste + recherche --------------------
    QWidget *pageListe = new QWidget;
    QVBoxLayout *listeLayout = new QVBoxLayout(pageListe);
    listeLayout->setContentsMargins(0, 4, 0, 4);
    listeLayout->setSpacing(12);

    // Reprendre les quatre indicateurs de la fenetre Statistiques et les garder
    // au-dessus de la recherche, comme dans l'interface precedente.
    const QList<Employe> employes = Employe::afficherTous();
    const QList<Employe::ChargeEmploye> charges = Employe::chargesSemaine(Planning::semaineDebut());
    const Employe::StatsCharge stats = Employe::statistiques(charges);

    QHBoxLayout *cardsLayout = new QHBoxLayout;
    cardsLayout->setSpacing(10);
    auto creerCarteIndicateur = [](const QString &icone, const QString &fond,
                                   const QString &couleur, const QString &titreCarte,
                                   const QString &valeur, const QString &sousTitre) {
        QFrame *card = new QFrame;
        card->setMinimumHeight(82);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        card->setStyleSheet(QString("QFrame { background-color: %1; border: none; border-radius: 12px; }").arg(fond));
        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(12, 9, 12, 9);
        cardLayout->setSpacing(9);
        QLabel *iconLabel = new QLabel(icone);
        iconLabel->setStyleSheet("font-size: 24px; background: transparent; border: none;");
        cardLayout->addWidget(iconLabel);
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(1);
        QLabel *titleLabel = new QLabel(titreCarte);
        titleLabel->setStyleSheet(QString("font-size: 10px; color: %1; background: transparent; border: none;").arg(couleur));
        QLabel *valueLabel = new QLabel(valeur);
        valueLabel->setStyleSheet(QString("font-size: 19px; font-weight: bold; color: %1; background: transparent; border: none;").arg(couleur));
        QLabel *subtitleLabel = new QLabel(sousTitre);
        subtitleLabel->setStyleSheet(QString("font-size: 9px; color: %1; background: transparent; border: none;").arg(couleur));
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(valueLabel);
        textLayout->addWidget(subtitleLabel);
        cardLayout->addLayout(textLayout, 1);
        return card;
    };
    cardsLayout->addWidget(creerCarteIndicateur(
        "👥", "#E4F0F3", "#225A64", "Nombre des employés",
        QString::number(employes.size()), "employés enregistrés"));
    cardsLayout->addWidget(creerCarteIndicateur(
        "⏱", "#FDF3E3", "#B8741A", "Charge totale",
        QString::number(stats.somme, 'f', 0) + " h", "par semaine"));
    cardsLayout->addWidget(creerCarteIndicateur(
        "📈", "#E6F4EA", "#2E7D32", "Charge moyenne",
        QString::number(stats.moyenne, 'f', 1) + " h", "par employé"));
    cardsLayout->addWidget(creerCarteIndicateur(
        "⚠️", "#FBE9E7", "#AE5751", "Charge max",
        QString::number(stats.maximum, 'f', 0) + " h", "le plus chargé"));
    listeLayout->addLayout(cardsLayout);

    QFrame *searchFrame = new QFrame;
    searchFrame->setStyleSheet("QFrame { background: white; border: 1px solid #D7E9ED; border-radius: 12px; }");
    QHBoxLayout *searchLayout = new QHBoxLayout(searchFrame);
    searchLayout->setContentsMargins(14, 8, 14, 8);
    QLabel *searchIcon = new QLabel("🔍");
    searchLayout->addWidget(searchIcon);
    QLineEdit *searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("Rechercher un employé par son nom ou prénom...");
    searchEdit->setMinimumHeight(38);
    searchEdit->setStyleSheet("QLineEdit { border: none; background: transparent; color: #244B56; font-size: 13px; }");
    searchLayout->addWidget(searchEdit, 1);
    listeLayout->addWidget(searchFrame);

    QLabel *listeInfo = new QLabel("Cliquez sur un employé pour ouvrir sa fiche d'évaluation.");
    listeInfo->setStyleSheet("font-size: 12px; color: #71858B; background: transparent;");
    listeLayout->addWidget(listeInfo);

    QTableWidget *employeesTable = new QTableWidget;employeesTable->setObjectName("employeeEvaluationTable");
    employeesTable->setColumnCount(3);
    employeesTable->setHorizontalHeaderLabels({"Employé", "Poste", "Courriel"});
    employeesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    employeesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    employeesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    employeesTable->setAlternatingRowColors(true);
    employeesTable->setShowGrid(false);
    employeesTable->verticalHeader()->setVisible(false);
    employeesTable->verticalHeader()->setDefaultSectionSize(52);
    employeesTable->horizontalHeader()->setMinimumHeight(42);
    employeesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    employeesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    employeesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    employeesTable->setStyleSheet(
        "QTableWidget { background: white; border: 1px solid #D7E9ED; border-radius: 12px; "
        "color: #244B56; selection-background-color: #D9EEF1; selection-color: #174B55; }"
        "QHeaderView::section { background: #2F7F8D; color: white; padding: 11px; border: none; font-weight: bold; }");

    employeesTable->setRowCount(employes.size());
    for (int row = 0; row < employes.size(); ++row) {
        const Employe &e = employes.at(row);
        QTableWidgetItem *nom = new QTableWidgetItem((e.getPrenom() + " " + e.getNom()).trimmed());
        nom->setData(Qt::UserRole, e.getId());
        QFont nomFont = nom->font();
        nomFont.setBold(true);
        nom->setFont(nomFont);
        employeesTable->setItem(row, 0, nom);
        employeesTable->setItem(row, 1, new QTableWidgetItem(e.getPoste()));
        employeesTable->setItem(row, 2, new QTableWidgetItem(e.getCourriel()));
    }
    if (employes.isEmpty()) {
        employeesTable->setRowCount(1);
        QTableWidgetItem *empty = new QTableWidgetItem("Aucun employé enregistré.");
        empty->setTextAlignment(Qt::AlignCenter);
        employeesTable->setItem(0, 0, empty);
        employeesTable->setSpan(0, 0, 1, 3);
        employeesTable->setEnabled(false);
    }
    listeLayout->addWidget(employeesTable, 1);
    pages->addWidget(pageListe);

    // -------------------- PAGE 2 : fiche d'evaluation --------------------
    QWidget *pageFiche = new QWidget;
    QVBoxLayout *ficheLayout = new QVBoxLayout(pageFiche);
    ficheLayout->setContentsMargins(0, 4, 0, 4);
    ficheLayout->setSpacing(10);

    QPushButton *btnRetour = new QPushButton("←  Retour à la liste des employés");
    btnRetour->setObjectName("btnSecondary");
    btnRetour->setCursor(Qt::PointingHandCursor);
    btnRetour->setMinimumHeight(36);
    ficheLayout->addWidget(btnRetour, 0, Qt::AlignLeft);

    QFrame *identiteFrame = new QFrame;
    identiteFrame->setStyleSheet("QFrame { background: white; border: 1px solid #D7E9ED; border-radius: 12px; }");
    QHBoxLayout *identiteLayout = new QHBoxLayout(identiteFrame);
    identiteLayout->setContentsMargins(16, 12, 16, 12);
    QLabel *avatar = new QLabel("👤");
    avatar->setStyleSheet("font-size: 30px; background: transparent; border: none;");
    identiteLayout->addWidget(avatar);
    QVBoxLayout *identiteText = new QVBoxLayout;
    QLabel *employeeName = new QLabel;
    employeeName->setStyleSheet("font-size: 18px; font-weight: bold; color: #225A64; background: transparent; border: none;");
    QLabel *employeeInfo = new QLabel;
    employeeInfo->setStyleSheet("font-size: 12px; color: #71858B; background: transparent; border: none;");
    identiteText->addWidget(employeeName);
    identiteText->addWidget(employeeInfo);
    identiteLayout->addLayout(identiteText, 1);
    QLabel *globalScore = new QLabel("Note globale : -- / 5");
    globalScore->setAlignment(Qt::AlignCenter);
    globalScore->setMinimumWidth(160);
    globalScore->setStyleSheet("QLabel { background: #FFF5D9; color: #9A6A12; border-radius: 10px; padding: 10px; font-size: 14px; font-weight: bold; }");
    identiteLayout->addWidget(globalScore);
    ficheLayout->addWidget(identiteFrame);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    QWidget *formContent = new QWidget;
    QVBoxLayout *formLayout = new QVBoxLayout(formContent);
    formLayout->setContentsMargins(0, 0, 6, 0);
    formLayout->setSpacing(12);

    QFrame *criteriaFrame = new QFrame;
    criteriaFrame->setStyleSheet("QFrame { background: white; border: 1px solid #D7E9ED; border-radius: 12px; }");
    QVBoxLayout *criteriaLayout = new QVBoxLayout(criteriaFrame);
    criteriaLayout->setContentsMargins(16, 14, 16, 14);
    QLabel *criteriaTitle = new QLabel("⭐  Critères d'évaluation");
    criteriaTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #225A64; background: transparent; border: none;");
    criteriaLayout->addWidget(criteriaTitle);
    QLabel *criteriaHelp = new QLabel(
        "Le responsable attribue une note de 1 à 5 pour chaque critère : 1 = Insuffisant, 5 = Excellent.");
    criteriaHelp->setWordWrap(true);
    criteriaHelp->setStyleSheet("font-size: 11px; color: #71858B; background: transparent; border: none;");
    criteriaLayout->addWidget(criteriaHelp);

    const QStringList criteriaNames = {
        "Discipline & ponctualité",
        "Comportement & communication",
        "Compétences professionnelles",
        "Qualité du travail",
        "Hygiène & sécurité",
        "Responsabilité & travail en équipe"
    };
    const QStringList tooltips = {
        "Respect des horaires, ponctualité et présence.",
        "Respect des collègues, des clients et qualité de communication.",
        "Compétences techniques liées au poste occupé.",
        "Précision, sérieux et qualité des tâches réalisées.",
        "Respect des règles d'hygiène, de sécurité et des protocoles.",
        "Autonomie, respect des responsabilités et coopération avec l'équipe."
    };
    QList<QList<QToolButton*>> ratingStarButtons;
    QList<QLabel*> ratingValueLabels;
    QList<QLabel*> criterionHints;
    QList<int> ratingValues;
    const QStringList ratingNames = {"", "Insuffisant", "À améliorer", "Satisfaisant", "Très bien", "Excellent"};
    for (int i = 0; i < criteriaNames.size(); ++i) {
        QFrame *line = new QFrame;
        line->setStyleSheet("QFrame { background: #F7FAFB; border: none; border-radius: 8px; }");
        QHBoxLayout *lineLayout = new QHBoxLayout(line);
        lineLayout->setContentsMargins(10, 7, 10, 7);
        QVBoxLayout *nameLayout = new QVBoxLayout;
        QLabel *criterion = new QLabel(criteriaNames.at(i));
        criterion->setStyleSheet("font-size: 13px; font-weight: 600; color: #315B65; background: transparent;");
        QLabel *hint = new QLabel(tooltips.at(i));
        hint->setWordWrap(true);
        criterionHints.append(hint);
        hint->setStyleSheet("font-size: 10px; color: #829297; background: transparent;");
        nameLayout->addWidget(criterion);
        nameLayout->addWidget(hint);
        lineLayout->addLayout(nameLayout, 1);

        QHBoxLayout *starsLayout = new QHBoxLayout;
        starsLayout->setSpacing(1);
        QList<QToolButton*> rowStars;
        for (int star = 1; star <= 5; ++star) {
            QToolButton *starButton = new QToolButton;
            starButton->setObjectName(QString("employeeRating_%1_%2").arg(i).arg(star));
            starButton->setText("★");
            starButton->setToolTip(QString::number(star) + "/5");
            starButton->setCursor(Qt::PointingHandCursor);
            starButton->setFixedSize(29, 32);
            starButton->setStyleSheet("QToolButton { color: #B8C4C8; font-size: 23px; border: none; background: transparent; padding: 0; }");
            starsLayout->addWidget(starButton);
            rowStars.append(starButton);
        }
        QWidget *starsWidget = new QWidget;
        starsWidget->setStyleSheet("background: transparent;");
        starsWidget->setLayout(starsLayout);
        lineLayout->addWidget(starsWidget, 0, Qt::AlignVCenter);

        QLabel *valueLabel = new QLabel("Non noté");
        valueLabel->setMinimumWidth(105);
        valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        valueLabel->setStyleSheet("font-size: 10px; color: #829297; background: transparent;");
        lineLayout->addWidget(valueLabel);
        criteriaLayout->addWidget(line);
        ratingStarButtons.append(rowStars);
        ratingValueLabels.append(valueLabel);
        ratingValues.append(0);
    }
    auto actualiserEtoiles = [&](int criterionIndex, int value) {
        if (criterionIndex < 0 || criterionIndex >= ratingStarButtons.size()) return;
        ratingValues[criterionIndex] = value;
        for (int j = 0; j < ratingStarButtons.at(criterionIndex).size(); ++j) {
            const bool selected = (j + 1) <= value;
            ratingStarButtons[criterionIndex].at(j)->setStyleSheet(
                selected
                    ? "QToolButton { color: #E6A52E; font-size: 23px; border: none; background: transparent; padding: 0; }"
                    : "QToolButton { color: #B8C4C8; font-size: 23px; border: none; background: transparent; padding: 0; }");
        }
        ratingValueLabels.at(criterionIndex)->setText(value > 0
            ? QString("%1/5 · %2").arg(value).arg(ratingNames.at(value))
            : "Non noté");
        ratingValueLabels.at(criterionIndex)->setStyleSheet(value > 0
            ? "font-size: 10px; color: #9A6A12; font-weight: bold; background: transparent;"
            : "font-size: 10px; color: #829297; background: transparent;");
    };
    formLayout->addWidget(criteriaFrame);

    QFrame *commentsFrame = new QFrame;
    commentsFrame->setStyleSheet("QFrame { background: white; border: 1px solid #D7E9ED; border-radius: 12px; }");
    QVBoxLayout *commentsLayout = new QVBoxLayout(commentsFrame);
    commentsLayout->setContentsMargins(16, 14, 16, 14);
    QLabel *commentsTitle = new QLabel("📝  Avis du responsable & plan d'amélioration");
    commentsTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #225A64; background: transparent; border: none;");
    commentsLayout->addWidget(commentsTitle);

    auto addTextField = [&](const QString &label, const QString &placeholder, int height) {
        QLabel *fieldLabel = new QLabel(label);
        fieldLabel->setStyleSheet("font-size: 12px; font-weight: 600; color: #315B65; background: transparent; border: none;");
        QTextEdit *field = new QTextEdit;
        field->setPlaceholderText(placeholder);
        field->setFixedHeight(height);
        field->setStyleSheet("QTextEdit { background: #F7FAFB; border: 1px solid #DCE9EC; border-radius: 8px; padding: 7px; color: #244B56; }");
        commentsLayout->addWidget(fieldLabel);
        commentsLayout->addWidget(field);
        return field;
    };
    QTextEdit *pointsForts = addTextField("Points forts", "Ex. : ponctuel, sérieux, bonne prise en charge des animaux...", 68);
    QTextEdit *axesAmelioration = addTextField("Axes d'amélioration", "Ex. : améliorer le suivi des dossiers ou la communication...", 68);
    QTextEdit *commentaires = addTextField("Commentaire général du responsable", "Écrire ici l'avis et les observations concernant cet employé...", 82);
    commentaires->setObjectName("employeeEvaluationComment");
    QTextEdit *objectifs = addTextField("Objectifs / recommandations", "Ex. : suivre une formation, améliorer une compétence, respecter un objectif...", 68);
    commentsLayout->addWidget(new QLabel("Les notes et commentaires sont sauvegardés automatiquement et peuvent être modifiés à tout moment."));
    formLayout->addWidget(commentsFrame);

    QFrame *metaFrame = new QFrame;
    metaFrame->setStyleSheet("QFrame { background: white; border: 1px solid #D7E9ED; border-radius: 12px; }");
    QFormLayout *metaLayout = new QFormLayout(metaFrame);
    metaLayout->setContentsMargins(16, 12, 16, 12);
    QDateEdit *dateEvaluation = new QDateEdit(QDate::currentDate());
    dateEvaluation->setCalendarPopup(true);
    dateEvaluation->setDisplayFormat("dd/MM/yyyy");
    dateEvaluation->setMinimumHeight(34);
    QLineEdit *evaluateur = new QLineEdit;
    evaluateur->setPlaceholderText("Nom du responsable / évaluateur");
    evaluateur->setMinimumHeight(34);
    metaLayout->addRow("Date de l'évaluation", dateEvaluation);
    metaLayout->addRow("Évaluateur", evaluateur);
    formLayout->addWidget(metaFrame);

    QLabel *reviewStatus = new QLabel("Sélectionnez les critères et saisissez les observations du responsable.");
    reviewStatus->setWordWrap(true);
    reviewStatus->setStyleSheet("QLabel { background: #E9F3F5; color: #315B65; border-radius: 8px; padding: 9px; }");
    formLayout->addWidget(reviewStatus);

    scroll->setWidget(formContent);
    ficheLayout->addWidget(scroll, 1);
    pages->addWidget(pageFiche);

    // -------------------- Barre d'actions --------------------
    QHBoxLayout *actionsLayout = new QHBoxLayout;
    actionsLayout->setSpacing(10);
    root->addLayout(actionsLayout);

    QPushButton *btnSave = new QPushButton("💾  Enregistrer");
    btnSave->setObjectName("btnPrimary");
    btnSave->setMinimumHeight(42);
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setVisible(false);
    actionsLayout->addWidget(btnSave);

    QPushButton *btnPdf = new QPushButton("📄  Exporter PDF");
    btnPdf->setObjectName("btnSecondary");
    btnPdf->setMinimumHeight(42);
    btnPdf->setCursor(Qt::PointingHandCursor);
    btnPdf->setVisible(false);
    actionsLayout->addWidget(btnPdf);

    QPushButton *btnGmail = new QPushButton("✉️  Envoyer par Gmail");
    btnGmail->setObjectName("btnPrimary");
    btnGmail->setMinimumHeight(42);
    btnGmail->setCursor(Qt::PointingHandCursor);
    btnGmail->setVisible(false);
    actionsLayout->addWidget(btnGmail);

    actionsLayout->addStretch();
    QPushButton *btnFermer = new QPushButton("Fermer");
    btnFermer->setObjectName("btnSecondary");
    btnFermer->setMinimumHeight(42);
    btnFermer->setMinimumWidth(125);
    btnFermer->setCursor(Qt::PointingHandCursor);
    actionsLayout->addWidget(btnFermer);

    Employe employeSelectionne;
    bool hasEmployee = false;
    bool loadingReview = false;
    bool reviewModified = false;

    auto calculerNote = [&]() -> double {
        int total = 0;
        int count = 0;
        for (int value : ratingValues) {
            if (value > 0) { total += value; ++count; }
        }
        if (count == 0) return 0.0;
        return static_cast<double>(total) / count;
    };
    auto actualiserNote = [&]() {
        int count = 0;
        for (int value : ratingValues)
            if (value > 0) ++count;
        if (count == ratingValues.size())
            globalScore->setText(QString("Note globale : %1 / 5").arg(calculerNote(), 0, 'f', 2));
        else
            globalScore->setText(QString("%1 / 6 critères notés").arg(count));
    };
    auto rendreActionsDisponibles = [&]() {
        btnSave->setEnabled(hasEmployee);
        btnPdf->setEnabled(hasEmployee);
        btnGmail->setEnabled(hasEmployee);
        if (employeSelectionne.getCourriel().trimmed().isEmpty())
            btnGmail->setToolTip("Aucune adresse courriel n'est enregistrée : cliquez pour voir le message d'avertissement.");
        else
            btnGmail->setToolTip("Ouvre Gmail avec le destinataire et l'évaluation affichée préremplis.");
    };

    auto sauvegarderEvaluation = [&](bool afficherConfirmation) -> bool {
        if (!hasEmployee || employeSelectionne.getId() <= 0)
            return false;

        QJsonObject fiche;
        QJsonArray notes;
        for (int valeur : ratingValues)
            notes.append(valeur);
        fiche.insert("notes", notes);
        fiche.insert("pointsForts", pointsForts->toPlainText());
        fiche.insert("axesAmelioration", axesAmelioration->toPlainText());
        fiche.insert("commentaires", commentaires->toPlainText());
        fiche.insert("objectifs", objectifs->toPlainText());
        fiche.insert("dateEvaluation", dateEvaluation->date().toString(Qt::ISODate));
        fiche.insert("evaluateur", evaluateur->text().trimmed());
        fiche.insert("noteGlobale", calculerNote());
        fiche.insert("updatedAt", QDateTime::currentDateTime().toString(Qt::ISODate));

        QString erreur;
        if(!EmployeeEvaluation::enregistrerFiche(employeSelectionne.getId(),fiche,&erreur)) {
            reviewStatus->setText("Enregistrement impossible : "+erreur);
            if(afficherConfirmation)QMessageBox::critical(&dlg,"Erreur d'enregistrement",erreur);
            return false;
        }

        reviewModified = false;
        reviewStatus->setText("✅ Évaluation enregistrée. Les changements suivants remplaceront cette version.");
        if (afficherConfirmation)
            QMessageBox::information(&dlg, "Évaluation enregistrée",
                                     "L'évaluation de " + employeSelectionne.getPrenom() + " " +
                                     employeSelectionne.getNom() + " a été sauvegardée. Vous la retrouverez en rouvrant sa fiche.");
        return true;
    };

    auto marquerModifie = [&]() {
        if (loadingReview || !hasEmployee) return;
        reviewModified = true;
        actualiserNote();
        // Sauvegarde automatique à chaque changement, en plus du bouton.
        // Ainsi, fermer la fenêtre ou revenir à la liste ne fait pas perdre la note.
        sauvegarderEvaluation(false);
    };
    for (int i = 0; i < ratingStarButtons.size(); ++i) {
        for (int j = 0; j < ratingStarButtons.at(i).size(); ++j) {
            connect(ratingStarButtons.at(i).at(j), &QToolButton::clicked, &dlg, [&, i, score = j + 1]() {
                actualiserEtoiles(i, score);
                marquerModifie();
            });
        }
    }
    connect(pointsForts, &QTextEdit::textChanged, &dlg, marquerModifie);
    connect(axesAmelioration, &QTextEdit::textChanged, &dlg, marquerModifie);
    connect(commentaires, &QTextEdit::textChanged, &dlg, marquerModifie);
    connect(objectifs, &QTextEdit::textChanged, &dlg, marquerModifie);
    connect(dateEvaluation, &QDateEdit::dateChanged, &dlg, marquerModifie);
    connect(evaluateur, &QLineEdit::textChanged, &dlg, marquerModifie);

    auto construireTexteReview = [&]() {
        QString text;
        text += "ÉVALUATION PROFESSIONNELLE\n";
        text += "Employé : " + (employeSelectionne.getPrenom() + " " + employeSelectionne.getNom()).trimmed() + "\n";
        text += "Poste : " + employeSelectionne.getPoste() + "\n";
        text += "Date : " + dateEvaluation->date().toString("dd/MM/yyyy") + "\n";
        text += "Évaluateur : " + evaluateur->text().trimmed() + "\n\n";
        for (int i = 0; i < ratingValues.size(); ++i) {
            const int score = ratingValues.at(i);
            text += criteriaNames.at(i) + " : " + (score > 0 ? QString::number(score) + "/5" : "Non évalué") + "\n";
        }
        text += "\nNote globale : " + QString::number(calculerNote(), 'f', 2) + "/5\n";
        text += "\nPoints forts :\n" + pointsForts->toPlainText().trimmed() + "\n";
        text += "\nAxes d'amélioration :\n" + axesAmelioration->toPlainText().trimmed() + "\n";
        text += "\nCommentaire du responsable :\n" + commentaires->toPlainText().trimmed() + "\n";
        text += "\nObjectifs / recommandations :\n" + objectifs->toPlainText().trimmed() + "\n";
        return text;
    };

    auto construireHtmlReview = [&]() {
        QString html = "<html><head><meta charset='utf-8'><style>"
                       "body{font-family:Arial,sans-serif;color:#244B56;} h1{color:#225A64;} "
                       ".meta{color:#607D86;} table{border-collapse:collapse;width:100%;} "
                       "td,th{border:1px solid #D7E9ED;padding:9px;text-align:left;} "
                       "th{background:#2F7F8D;color:white;} .box{background:#F3F8F9;padding:10px;margin:10px 0;}"
                       "</style></head><body>";
        html += "<h1>Évaluation professionnelle</h1>";
        html += "<p class='meta'><b>Employé :</b> " + (employeSelectionne.getPrenom() + " " + employeSelectionne.getNom()).trimmed().toHtmlEscaped() +
                "<br><b>Poste :</b> " + employeSelectionne.getPoste().toHtmlEscaped() +
                "<br><b>Date :</b> " + dateEvaluation->date().toString("dd/MM/yyyy") +
                "<br><b>Évaluateur :</b> " + evaluateur->text().toHtmlEscaped() + "</p>";
        html += "<table><tr><th>Critère</th><th>Note</th></tr>";
        for (int i = 0; i < ratingValues.size(); ++i) {
            const int score = ratingValues.at(i);
            html += "<tr><td>" + criteriaNames.at(i).toHtmlEscaped() + "</td><td>" +
                    (score > 0 ? QString::number(score) + "/5" : "Non évalué") + "</td></tr>";
        }
        html += "</table><p><b>Note globale :</b> " + QString::number(calculerNote(), 'f', 2) + "/5</p>";
        auto bloc = [&](const QString &label, const QString &value) {
            return "<div class='box'><b>" + label.toHtmlEscaped() + "</b><p>" +
                   value.toHtmlEscaped().replace("\n", "<br>") + "</p></div>";
        };
        html += bloc("Points forts", pointsForts->toPlainText());
        html += bloc("Axes d'amélioration", axesAmelioration->toPlainText());
        html += bloc("Commentaire du responsable", commentaires->toPlainText());
        html += bloc("Objectifs / recommandations", objectifs->toPlainText());
        html += "</body></html>";
        return html;
    };

    auto reinitialiserFiche = [&]() {
        loadingReview = true;
        for (int i = 0; i < ratingValues.size(); ++i) actualiserEtoiles(i, 0);
        pointsForts->clear();
        axesAmelioration->clear();
        commentaires->clear();
        objectifs->clear();
        evaluateur->clear();
        dateEvaluation->setDate(QDate::currentDate());
        reviewModified = false;

        QString erreurLecture;
        const auto fiche=EmployeeEvaluation::fiche(employeSelectionne.getId(),&erreurLecture);
        if (!erreurLecture.isEmpty()) {
            reviewStatus->setText("⚠ Impossible de charger l'évaluation sauvegardée : " + erreurLecture);
        } else {
            if (!fiche.isEmpty()) {
                const QJsonArray notes = fiche.value("notes").toArray();
                for (int i = 0; i < ratingValues.size(); ++i) {
                    const int valeur = i < notes.size() ? qBound(0, notes.at(i).toInt(), 5) : 0;
                    actualiserEtoiles(i, valeur);
                }
                pointsForts->setPlainText(fiche.value("pointsForts").toString());
                axesAmelioration->setPlainText(fiche.value("axesAmelioration").toString());
                commentaires->setPlainText(fiche.value("commentaires").toString());
                objectifs->setPlainText(fiche.value("objectifs").toString());
                const QDate dateStockee = QDate::fromString(fiche.value("dateEvaluation").toString(), Qt::ISODate);
                if (dateStockee.isValid()) dateEvaluation->setDate(dateStockee);
                evaluateur->setText(fiche.value("evaluateur").toString());
                reviewStatus->setText("✅ Évaluation précédente chargée. Toute modification remplacera automatiquement cette version.");
            } else {
                reviewStatus->setText("Nouvelle fiche : saisissez les notes et commentaires. Les changements seront enregistrés automatiquement.");
            }
        }
        loadingReview = false;
        actualiserNote();
        rendreActionsDisponibles();
    };

    auto choisirEmploye = [&](int row) {
        if (row < 0 || !employeesTable->item(row, 0)) return;
        const int id = employeesTable->item(row, 0)->data(Qt::UserRole).toInt();
        if (id <= 0) return;
        if (!Employe::charger(id, employeSelectionne)) {
            QMessageBox::warning(&dlg, "Employé introuvable", "Impossible de charger les informations de cet employé.");
            return;
        }
        hasEmployee = true;
        employeeName->setText((employeSelectionne.getPrenom() + " " + employeSelectionne.getNom()).trimmed());
        employeeInfo->setText(employeSelectionne.getPoste() + "  ·  " + employeSelectionne.getCourriel());
        const QString poste = employeSelectionne.getPoste().toLower();
        QString competenceHint = "Compétences techniques et savoir-faire nécessaires pour le poste occupé.";
        if (poste.contains("vétérinaire") || poste.contains("veterinaire"))
            competenceHint = "Consultation, diagnostic, soins, suivi des dossiers médicaux et respect des protocoles vétérinaires.";
        else if (poste.contains("assistant") || poste.contains("auxiliaire"))
            competenceHint = "Préparation du matériel, aide aux soins, manipulation sécurisée des animaux et assistance au vétérinaire.";
        else if (poste.contains("réception") || poste.contains("reception") || poste.contains("secrétaire") || poste.contains("secretaire"))
            competenceHint = "Accueil des clients, gestion des rendez-vous et dossiers, communication et confidentialité.";
        else if (poste.contains("entretien"))
            competenceHint = "Nettoyage, désinfection, organisation des espaces et respect des protocoles d'hygiène.";
        criterionHints.at(2)->setText(competenceHint);
        reinitialiserFiche();
        pages->setCurrentWidget(pageFiche);
        btnSave->setVisible(true);
        btnPdf->setVisible(true);
        btnGmail->setVisible(true);
        rendreActionsDisponibles();
    };

    connect(searchEdit, &QLineEdit::textChanged, &dlg, [employeesTable](const QString &text) {
        const QString needle = text.trimmed();
        for (int row = 0; row < employeesTable->rowCount(); ++row) {
            const QString name = employeesTable->item(row, 0) ? employeesTable->item(row, 0)->text() : QString();
            employeesTable->setRowHidden(row, !name.contains(needle, Qt::CaseInsensitive));
        }
    });
    connect(employeesTable, &QTableWidget::cellClicked, &dlg,
            [&](int row, int) { choisirEmploye(row); });
    connect(btnRetour, &QPushButton::clicked, &dlg, [&]() {
        pages->setCurrentWidget(pageListe);
        btnSave->setVisible(false);
        btnPdf->setVisible(false);
        btnGmail->setVisible(false);
        btnPdf->setEnabled(false);
        btnGmail->setEnabled(false);
        employeesTable->clearSelection();
        searchEdit->setFocus();
    });

    connect(btnSave, &QPushButton::clicked, &dlg, [&]() {
        sauvegarderEvaluation(true);
    });

    connect(btnPdf, &QPushButton::clicked, &dlg, [&]() {
        if (!hasEmployee) return;
        if (reviewModified && !sauvegarderEvaluation(false)) return;
        QString defaultName = "Evaluation_" + employeSelectionne.getPrenom() + "_" + employeSelectionne.getNom() + ".pdf";
        defaultName.replace(QRegularExpression("[^A-Za-z0-9_.-]"), "_");
        const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + defaultName;
        QString path = QFileDialog::getSaveFileName(&dlg, "Exporter l'évaluation en PDF", defaultPath, "Fichier PDF (*.pdf)");
        if (path.isEmpty()) return;
        if (!path.endsWith(".pdf", Qt::CaseInsensitive)) path += ".pdf";
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(path);
        QTextDocument document;
        document.setHtml(construireHtmlReview());
        document.print(&printer);
        const QFileInfo pdfInfo(path);
        if (!pdfInfo.exists() || pdfInfo.size() <= 0) {
            QMessageBox::critical(&dlg, "Erreur PDF",
                                  "Le fichier PDF n'a pas pu être créé. Vérifiez les droits d'accès au dossier choisi.");
            return;
        }
        QMessageBox::information(&dlg, "PDF créé",
                                 "Le rapport d'évaluation a été exporté :\n" + path);
    });

    connect(btnGmail, &QPushButton::clicked, &dlg, [&]() {
        if (!hasEmployee) return;
        if (reviewModified && !sauvegarderEvaluation(false)) return;
        const QString destinataire = employeSelectionne.getCourriel().trimmed();
        if (destinataire.isEmpty()) {
            QMessageBox::warning(&dlg, "Adresse manquante", "Aucune adresse courriel n'est enregistrée pour cet employé.");
            return;
        }
        QUrl url("https://mail.google.com/mail/");
        QUrlQuery query;
        query.addQueryItem("view", "cm");
        query.addQueryItem("fs", "1");
        query.addQueryItem("to", destinataire);
        query.addQueryItem("su", "Évaluation professionnelle - " +
                           (employeSelectionne.getPrenom() + " " + employeSelectionne.getNom()).trimmed());
        query.addQueryItem("body", "Bonjour " + employeSelectionne.getPrenom() + ",\n\n" +
                           construireTexteReview() +
                           "\n\nCordialement,\nResponsable du centre\n\n" +
                           "Remarque : vous pouvez joindre le PDF exporté depuis ANIWORLD avant d'envoyer ce message.");
        url.setQuery(query);
        if (!QDesktopServices::openUrl(url)) {
            QMessageBox::warning(&dlg, "Gmail", "Impossible d'ouvrir Gmail dans le navigateur. Vérifiez la connexion Internet et l'application associée aux liens web.");
            return;
        }
        QMessageBox::information(&dlg, "Gmail ouvert",
                                 "Gmail a été ouvert avec le destinataire, le sujet et le contenu de l'évaluation préremplis.\n\nVérifiez le message puis cliquez sur Envoyer dans Gmail. Pour joindre le PDF, exportez-le avec le bouton PDF puis ajoutez-le au message.");
    });

    connect(btnFermer, &QPushButton::clicked, &dlg, &QDialog::accept);
    dlg.exec();
}

void GestionEmployes::ouvrirPlanificationEquipe()
{
    QDialog dlg(this);dlg.setObjectName("employeeTeamPlanningDialog");
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
