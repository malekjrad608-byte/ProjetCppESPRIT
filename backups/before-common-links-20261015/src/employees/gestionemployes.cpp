#include "../database/oracleconnection.h"
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
#include <QSqlDatabase>
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
    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
    q.exec("SELECT e.prenom || ' ' || e.nom, p1.titre, p1.debut, p1.fin, "
           "p2.titre, p2.debut, p2.fin "
           "FROM planning_affectations p1 "
           "JOIN planning_affectations p2 ON p1.responsable_id=p2.responsable_id "
           "AND p1.id<p2.id AND p1.debut<p2.fin AND p2.debut<p1.fin "
           "JOIN employes e ON e.id=p1.responsable_id "
           "ORDER BY p1.debut" + QString(OracleConnection::isOracle()?" FETCH FIRST 20 ROWS ONLY":" LIMIT 20"));
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
    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
    q.prepare("SELECT p.responsable_id, e.prenom, e.nom, p.debut, p.fin "
              "FROM planning_affectations p JOIN employes e ON e.id=p.responsable_id "
              "WHERE p.fin >= :maintenant ORDER BY p.responsable_id, p.debut");
    q.bindValue(":maintenant", QDate::currentDate().toString("yyyy-MM-dd") + " 00:00:00");
    if (!q.exec()) return alertes;

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

// ==================== UI PRINCIPALE ====================
void GestionEmployes::construireUI()
{
    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(24, 24, 24, 20);
    main->setSpacing(16);

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

    QLabel *titre = new QLabel("Employés");
    titre->setStyleSheet("font-size: 23px; font-weight: bold; color: #225A64;"
                         "background: transparent;");
    auto *titleRow=new QHBoxLayout; auto *titleIcon=Design::label(""); titleIcon->setPixmap(Design::icon("users",QColor("#8268AE")).pixmap(24,24)); titleRow->addWidget(titleIcon); titleRow->addWidget(titre); titleRow->addStretch(); titreL->addLayout(titleRow);

    QLabel *sous = new QLabel("Annuaire, disponibilités et gestion de la charge");
    sous->setStyleSheet("font-size: 13px; color: #7A8B8F; background: transparent;");
    titreL->addWidget(sous);

    layout->addLayout(titreL);
    layout->addStretch();

    QPushButton *btnNouveau = Design::button("Ajouter un employé","plus","primary");
    btnNouveau->setObjectName("addEmployee"); btnNouveau->setProperty("role","primary");
    btnNouveau->setMinimumHeight(36);
    btnNouveau->setMinimumWidth(210);
    btnNouveau->setCursor(Qt::PointingHandCursor);
    connect(btnNouveau, &QPushButton::clicked, this, &GestionEmployes::ajouterEmploye);
    auto *attestation=Design::button("Attestation PDF","export"); attestation->setObjectName("employeeLeavePdf");
    connect(attestation,&QPushButton::clicked,this,&GestionEmployes::exportLeaveCertificate);
    layout->addWidget(attestation);
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

        QLabel *ico = new QLabel; ico->setPixmap(Design::icon(icone,QColor(fg)).pixmap(18,18));
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

        Q_UNUSED(idx);
        return pill;
    };

    QFrame *p1 = creerPill("check", "#E6F4EA", "#2E7D32", m_lblDisponibles, "disponibles", 0);
    QFrame *p2 = creerPill("users", "#FBE9E7", "#AE5751", m_lblIndisponibles, "indisponibles", 1);
    QFrame *p3 = creerPill("clock", "#FDF3E3", "#B8741A", m_lblChargeTotale, "heures planifiées", 2);
    m_lblChargeTotale->setObjectName("employeeTotalHours");

    layout->addWidget(p1);
    layout->addWidget(p2);
    layout->addWidget(p3);

    layout->addStretch();

    //  Bouton stats (m3a pulse animation)
    QPushButton *btnStats = Design::button("Voir les statistiques","grid","");
    btnStats->setObjectName("employeeStatistics"); btnStats->setProperty("role","");
    btnStats->setMinimumHeight(36);
    btnStats->setCursor(Qt::PointingHandCursor);

    connect(btnStats, &QPushButton::clicked, this, [this]() {
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
        const QString jointureChargeSemaine =
            " LEFT JOIN (SELECT responsable_id, SUM(charge_heures) AS plan "
            "FROM planning_affectations WHERE debut >= :debutSemaine AND debut < :finSemaine "
            "GROUP BY responsable_id) p ON p.responsable_id=e.id ";
        const QString chargeEffective = "(COALESCE(e.charge_hebdo,0)+COALESCE(p.plan,0))";
        QSqlQuery qStats(QSqlDatabase::database("aniworld-services"));
        qStats.prepare("SELECT COUNT(*), COALESCE(SUM(" + chargeEffective + "),0), "
                       "COALESCE(AVG(" + chargeEffective + "),0), "
                       "COALESCE(MAX(" + chargeEffective + "),0), "
                       "COALESCE(MIN(" + chargeEffective + "),0) FROM employes e " + jointureChargeSemaine);
        qStats.bindValue(":debutSemaine", debutSemaineSql);
        qStats.bindValue(":finSemaine", finSemaineSql);
        qStats.exec();
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
        QSqlQuery qCharges(QSqlDatabase::database("aniworld-services"));
        qCharges.prepare("SELECT e.prenom, e.nom, e.charge_hebdo, e.dispo_hebdo, "
                         "COALESCE(p.plan,0) FROM employes e " + jointureChargeSemaine);
        qCharges.bindValue(":debutSemaine", debutSemaineSql);
        qCharges.bindValue(":finSemaine", finSemaineSql);
        if (qCharges.exec()) {
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

        QSqlQuery q(QSqlDatabase::database("aniworld-services"));
        q.prepare("SELECT e.nom, e.prenom, e.poste, " + chargeEffective + ", e.dispo_hebdo "
                  "FROM employes e " + jointureChargeSemaine +
                  "ORDER BY " + chargeEffective + " DESC");
        q.bindValue(":debutSemaine", debutSemaineSql);
        q.bindValue(":finSemaine", finSemaineSql);
        q.exec();

        int row = 0;
        while (q.next()) {
            table->insertRow(row);

            QString nom = q.value(1).toString() + " " + q.value(0).toString();
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
        QPushButton *btnPlanifier = Design::button("Planifier et affecter","calendar","primary");
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
    });
    layout->addWidget(btnStats);

    //  MA7INA el animated counters — les valeurs ybanou direct
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
    m_editRecherche->setObjectName("employeeSearch");
    m_editRecherche->addAction(Design::icon("search"),QLineEdit::LeadingPosition);
    m_editRecherche->setPlaceholderText("Rechercher par nom, ID ou poste...");
    m_editRecherche->setMinimumHeight(34);
    connect(m_editRecherche, &QLineEdit::textChanged,
            this, &GestionEmployes::onFiltresChange);
    layout->addWidget(m_editRecherche, 3);

    m_comboPoste = new QComboBox; m_comboPoste->setObjectName("employeePostFilter"); m_comboPoste->setMinimumWidth(110); m_comboPoste->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_comboPoste->setMinimumHeight(34);
    m_comboPoste->addItems({"Tous les postes", "Vétérinaire",
                            "Assistant(e) vétérinaire", "Auxiliaire vétérinaire",
                            "Secrétaire / Réceptionniste", "Agent d'entretien",
                            "Aide aux soins animaliers"});
    connect(m_comboPoste, &QComboBox::currentTextChanged,
            this, &GestionEmployes::onFiltresChange);
    layout->addWidget(m_comboPoste, 1);

    m_comboStatut = new QComboBox; m_comboStatut->setObjectName("employeeStatusFilter"); m_comboStatut->setMinimumWidth(110); m_comboStatut->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_comboStatut->setMinimumHeight(34);
    m_comboStatut->addItems({"Tous", "Disponible", "Non disponible"});
    connect(m_comboStatut, &QComboBox::currentTextChanged,
            this, &GestionEmployes::onFiltresChange);
    layout->addWidget(m_comboStatut, 1);

    m_comboTri = new QComboBox; m_comboTri->setObjectName("employeeSort"); m_comboTri->setMinimumWidth(110); m_comboTri->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_comboTri->setMinimumHeight(34);
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
    m_gridCards->setSpacing(14);
    m_gridCards->setAlignment(Qt::AlignTop);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(m_cardsContainer);
    m_cardsContainer->setAutoFillBackground(false); scroll->viewport()->setAutoFillBackground(false);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }"
                          "QScrollArea > QWidget > QWidget { background: transparent; }");
    return scroll;
}

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
QWidget *GestionEmployes::construireDetailPanel()
{
    QFrame *card = Design::card("employeeDetailPanel");
    card->setMinimumWidth(284);
    card->setMaximumWidth(306);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QLabel *titre = new QLabel("EMPLOYÉ SÉLECTIONNÉ");
    titre->setStyleSheet("font-size: 11px; font-weight: bold; color: #7A8B8F;"
                         "letter-spacing: 1px; padding: 18px 18px 8px 18px;"
                         "background: transparent;");
    layout->addWidget(titre);

    m_detailPlaceholder = new QLabel("Sélectionnez un employé\npour voir ses détails");
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
    contentL->setContentsMargins(18, 5, 18, 12);
    contentL->setSpacing(10);

    m_detailNom = new QLabel; m_detailNom->setObjectName("employeeDetailName"); m_detailNom->setTextFormat(Qt::PlainText);
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
        valeur = new QLabel("-"); valeur->setTextFormat(Qt::PlainText);
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

    m_detailActions=new QWidget;
    auto *actions=new QVBoxLayout(m_detailActions); actions->setContentsMargins(18,8,18,18); actions->setSpacing(8);

    QPushButton *btnModifier = Design::button("Modifier","edit","primary");
    btnModifier->setObjectName("editEmployee"); btnModifier->setProperty("role","primary");
    btnModifier->setMinimumHeight(34);
    btnModifier->setCursor(Qt::PointingHandCursor);
    connect(btnModifier, &QPushButton::clicked, this, &GestionEmployes::modifierEmploye);
    actions->addWidget(btnModifier);

    QPushButton *btnBasculer = Design::button("Basculer disponibilité","check","success");
    btnBasculer->setObjectName("toggleEmployeeAvailability"); btnBasculer->setProperty("role","success");
    btnBasculer->setMinimumHeight(34);
    btnBasculer->setCursor(Qt::PointingHandCursor);
    connect(btnBasculer, &QPushButton::clicked, this, &GestionEmployes::basculerDisponibilite);
    actions->addWidget(btnBasculer);

    QPushButton *btnSupprimer = Design::button("Supprimer","trash","danger");
    btnSupprimer->setObjectName("deleteEmployee"); btnSupprimer->setProperty("role","danger");
    btnSupprimer->setMinimumHeight(34);
    btnSupprimer->setCursor(Qt::PointingHandCursor);
    connect(btnSupprimer, &QPushButton::clicked, this, &GestionEmployes::supprimerEmploye);
    actions->addWidget(btnSupprimer);

    contentL->addStretch();

    scroll->setWidget(m_detailContent);
    m_detailContent->setAutoFillBackground(false); scroll->viewport()->setAutoFillBackground(false);
    layout->addWidget(scroll, 1);
    layout->addWidget(m_detailActions); m_detailActions->hide();

    m_detailContent->hide();

    return card;
}

// ==================== CHARGEMENT ====================
void GestionEmployes::chargerEmployes(const QString &filtre, const QString &tri,
                                      const QString &poste, const QString &statut)
{
    QString orderBy = "date_embauche DESC, prenom ASC";
    if (tri == "charge") orderBy = "charge_hebdo DESC, prenom ASC";

    QString sql = "SELECT id, nom, prenom, courriel, telephone, poste, "
                  "date_embauche, disponible, charge_hebdo, dispo_hebdo, "
                  "competences FROM employes WHERE 1=1";

    if (!filtre.isEmpty())
        sql += " AND (nom LIKE :f OR prenom LIKE :f OR courriel LIKE :f "
               "OR poste LIKE :f OR " + QString(OracleConnection::isOracle()?"TO_CHAR(id)":"CAST(id AS TEXT)") + " LIKE :f)";
    if (poste != "Tous les postes")
        sql += " AND poste = :poste";
    if (statut == "Disponible")
        sql += " AND disponible = 1";
    else if (statut == "Non disponible")
        sql += " AND disponible = 0";

    sql += " ORDER BY " + orderBy;

    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
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
    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
    q.exec("SELECT COUNT(*) FROM employes WHERE disponible=1");
    if (q.next()) m_lblDisponibles->setText(q.value(0).toString());

    q.exec("SELECT COUNT(*) FROM employes WHERE disponible=0");
    if (q.next()) m_lblIndisponibles->setText(q.value(0).toString());

    const QDate debut = semaineDebut();
    QSqlQuery qCharge(QSqlDatabase::database("aniworld-services"));
    qCharge.prepare(OracleConnection::isOracle()
        ? "SELECT (SELECT COALESCE(SUM(charge_hebdo),0) FROM employes) + (SELECT COALESCE(SUM(charge_heures),0) FROM planning_affectations WHERE debut>=:debut AND debut<:fin) FROM dual"
        : "SELECT COALESCE(SUM(e.charge_hebdo),0) + COALESCE((SELECT SUM(p.charge_heures) FROM planning_affectations p WHERE p.debut>=:debut AND p.debut<:fin),0) FROM employes e");
    qCharge.bindValue(":debut", borneSemaine(debut));
    qCharge.bindValue(":fin", borneSemaine(debut.addDays(7)));
    if (qCharge.exec() && qCharge.next())
        m_lblChargeTotale->setText(QString::number(qRound(qCharge.value(0).toDouble())));
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

            QSqlQuery q(QSqlDatabase::database("aniworld-services"));
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

    mainL->addLayout(form);
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
        dlg.accept();
    });
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    mainL->addWidget(btns);

    if (dlg.exec() != QDialog::Accepted) return false;

    if (editNom->text().trimmed().isEmpty() ||
        editPrenom->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Nom et prénom sont obligatoires");
        return false;
    }

    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
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
        QMessageBox::warning(this, "Erreur", " " + q.lastError().text());
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
        QMessageBox::information(this, "Succès", " Employé ajouté");
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

    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
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
        QMessageBox::information(this, "Succès", " Employé modifié");
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

    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
    q.prepare("SELECT nom, prenom FROM employes WHERE id=:id");
    q.bindValue(":id", m_idSelectionne);
    QString nom;
    if (q.exec() && q.next())
        nom = q.value(1).toString() + " " + q.value(0).toString();

    if (QMessageBox::question(this, "Confirmation",
                              "Supprimer " + nom + " ?") != QMessageBox::Yes)
        return;

    QSqlQuery qDel(QSqlDatabase::database("aniworld-services"));
    qDel.prepare("DELETE FROM employes WHERE id=:id");
    qDel.bindValue(":id", m_idSelectionne);

    if (!qDel.exec()) {
        QMessageBox::warning(this, "Erreur", " " + qDel.lastError().text());
        return;
    }

    QMessageBox::information(this, "Succès", " Employé supprimé");
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

    QSqlQuery qSelect(QSqlDatabase::database("aniworld-services"));
    qSelect.prepare("SELECT disponible FROM employes WHERE id=:id");
    qSelect.bindValue(":id", m_idSelectionne);
    if (!qSelect.exec() || !qSelect.next()) return;

    bool actuel = qSelect.value(0).toInt() == 1;

    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
    q.prepare("UPDATE employes SET disponible=:d WHERE id=:id");
    q.bindValue(":d", actuel ? 0 : 1);
    q.bindValue(":id", m_idSelectionne);

    if (q.exec()) {
        QMessageBox::information(this, "Succès",
                                 QString(" Statut changé : %1").arg(actuel ? "Non disponible" : "Disponible"));
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

    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
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
                                 " Aucun employé disponible ne possède la compétence « " + comp + " ».");
    } else {
        QMessageBox::information(this, "EMP-11 · Profils proposés",
                                 " Employés habilités pour « " + comp + " » :\n\n" + resultats.join("\n"));
    }
}

// ==================== EMP-12 ====================
void GestionEmployes::verifierEquilibrage()
{
    QSqlQuery q(QSqlDatabase::database("aniworld-services"));
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

    QSqlQuery employes(QSqlDatabase::database("aniworld-services"));
    employes.exec("SELECT id, nom, prenom, poste, competences, charge_hebdo, dispo_hebdo "
                  "FROM employes WHERE disponible=1");
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

        QSqlQuery conflit(QSqlDatabase::database("aniworld-services"));
        conflit.prepare("SELECT COUNT(*) FROM planning_affectations "
                        "WHERE responsable_id=:id AND debut < :fin AND :debut < fin");
        conflit.bindValue(":id", id);
        conflit.bindValue(":debut", debutSql);
        conflit.bindValue(":fin", finSql);
        if (!conflit.exec() || !conflit.next() || conflit.value(0).toInt() > 0)
            continue;

        QSqlQuery planifie(QSqlDatabase::database("aniworld-services"));
        planifie.prepare("SELECT COALESCE(SUM(charge_heures),0) FROM planning_affectations "
                         "WHERE responsable_id=:id AND debut >= :debutSemaine AND debut < :finSemaine");
        planifie.bindValue(":id", id);
        planifie.bindValue(":debutSemaine", semaineDebutSql);
        planifie.bindValue(":finSemaine", semaineFinSql);
        if (!planifie.exec() || !planifie.next()) continue;

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

    QSqlQuery insert(QSqlDatabase::database("aniworld-services"));
    insert.prepare("INSERT INTO planning_affectations "
                   "(titre, competence_requise, debut, fin, charge_heures, responsable_id) "
                   "VALUES (:titre, :competence, :debut, :fin, :heures, :id)");
    insert.bindValue(":titre", nomTache);
    insert.bindValue(":competence", competence);
    insert.bindValue(":debut", debutSql);
    insert.bindValue(":fin", finSql);
    insert.bindValue(":heures", heures->value());
    insert.bindValue(":id", meilleurId);
    if (!insert.exec()) {
        QMessageBox::critical(this, "Erreur d'enregistrement",
                              "La tâche n'a pas été enregistrée : " + insert.lastError().text());
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
        QSqlQuery qEmployes(QSqlDatabase::database("aniworld-services"));
        if (qEmployes.exec("SELECT id, nom, prenom, poste FROM employes ORDER BY prenom, nom")) {
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
        QSqlQuery q(QSqlDatabase::database("aniworld-services"));
        q.prepare("SELECT nom, prenom, poste, date_embauche FROM employes WHERE id=:id");
        q.bindValue(":id", id);
        if (!q.exec() || !q.next()) {
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
