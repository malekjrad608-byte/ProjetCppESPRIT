#include "loginvisuals.h"
#include "mainwindow.h"
#include "gestionemployes.h"
#include "employe.h"
#include "connection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QList>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QIcon>
#include <QFrame>
#include <QPixmap>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <QStyle>
#include <QDateEdit>
#include <QFormLayout>
#include <QTextDocument>
#include <QTextCursor>
#include <QPrinter>
#include <QPageSize>
#include <QPageLayout>
#include <QMarginsF>
#include <QFileInfo>
#include <QDir>
#include <QDate>

// ==================== Constructeur ====================
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("ANIWORLD — Gestion du centre");
    resize(1400, 900);

    QWidget *central = new QWidget;
    setCentralWidget(central);

    QHBoxLayout *main = new QHBoxLayout(central);
    main->setContentsMargins(0, 0, 0, 0);
    main->setSpacing(0);

    main->addWidget(creerSidebar());

    QWidget *zone = new QWidget;
    QVBoxLayout *zoneLayout = new QVBoxLayout(zone);
    zoneLayout->setContentsMargins(0, 0, 0, 0);
    zoneLayout->setSpacing(0);

    zoneLayout->addWidget(creerTopBar());

    m_stack = new QStackedWidget;
    m_stack->addWidget(creerPageEmployes());                                    // 0
    m_stack->addWidget(creerPagePlaceholder("Tableau de bord", "📊",
                                            "Vue d'ensemble de l'activité du centre"));                             // 1
    m_stack->addWidget(creerPagePlaceholder("Services", "🐾",
                                            "Planning et gestion des prestations"));                                // 2
    m_stack->addWidget(creerPagePlaceholder("Stock", "📦",
                                            "Inventaire, traçabilité des lots et réapprovisionnement"));            // 3
    m_stack->addWidget(creerPagePlaceholder("Locaux", "🏠",
                                            "Occupation, nettoyage et disponibilité des espaces"));                 // 4
    m_stack->addWidget(creerPagePlaceholder("Dossiers médicaux", "📋",
                                            "Historique clinique et suivi des animaux"));                           // 5

    zoneLayout->addWidget(m_stack, 1);
    main->addWidget(zone, 1);
}

// ==================== TOP BAR ====================
QFrame *MainWindow::creerTopBar()
{
    QFrame *bar = new QFrame;
    bar->setObjectName("topBar");
    bar->setFixedHeight(70);

    QHBoxLayout *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(30, 0, 30, 0);
    layout->setSpacing(15);

    QLabel *breadcrumb = new QLabel("Gestion   /   <b>Employés</b>");
    breadcrumb->setObjectName("breadcrumb");
    breadcrumb->setTextFormat(Qt::RichText);
    layout->addWidget(breadcrumb);

    layout->addStretch();

    QPushButton *btnDemo = new QPushButton("Données de démonstration");
    btnDemo->setObjectName("btnDemo");
    btnDemo->setMinimumHeight(40);
    btnDemo->setCursor(Qt::PointingHandCursor);
    connect(btnDemo, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, "Données de démonstration",
                "Ajouter les employés de démonstration manquants ?\n\n"
                "Les employés existants ne seront ni supprimés ni modifiés.",
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
            return;

        struct DemoEmployee {
            QString nom, prenom, email, telephone, poste, date, competences;
            double charge;
        };
        const QList<DemoEmployee> demos = {
            {"Ben Ali", "Sami", "sami@aniworld.tn", "+216 20 111 111", "Vétérinaire", "2020-01-15", "Soins, Chirurgie, Diagnostic", 30},
            {"Trabelsi", "Amine", "amine@aniworld.tn", "+216 20 222 222", "Assistant(e) vétérinaire", "2021-03-10", "Soins, Vaccination", 28},
            {"Mansouri", "Youssef", "youssef@aniworld.tn", "+216 20 333 333", "Auxiliaire vétérinaire", "2022-06-01", "Toilettage, Soins", 20},
            {"Karoui", "Lina", "lina@aniworld.tn", "+216 20 444 444", "Secrétaire / Réceptionniste", "2019-09-20", "Accueil, Réservation", 25},
            {"Gharbi", "Sarra", "sarra@aniworld.tn", "+216 20 555 555", "Agent d'entretien", "2023-02-05", "Nettoyage", 22},
            {"Jelassi", "Nour", "nour@aniworld.tn", "+216 20 666 666", "Aide aux soins animaliers", "2023-07-12", "Soins, Alimentation", 24}
        };

        int ajoutes = 0;
        for (const DemoEmployee &d : demos) {
            // Les requetes sont dans le modele Employe (architecture Modele-Vue)
            if (Employe::existeCourriel(d.email)) continue;

            Employe e;
            e.setNom(d.nom);
            e.setPrenom(d.prenom);
            e.setCourriel(d.email);
            e.setTelephone(d.telephone);
            e.setPoste(d.poste);
            e.setDateEmbauche(QDate::fromString(d.date, "yyyy-MM-dd"));
            e.setDisponible(true);
            e.setChargeHebdo(d.charge);
            e.setDispoHebdo(35);
            e.setCompetences(d.competences);

            QString erreur;
            if (!e.ajouter(&erreur)) {
                QMessageBox::critical(this, "Erreur", "Ajout de démonstration impossible : " + erreur);
                return;
            }
            ++ajoutes;
        }
        QMessageBox::information(this, "Données de démonstration",
            ajoutes == 0 ? "Les employés de démonstration sont déjà présents. Aucune donnée n'a été modifiée."
                         : QString("%1 employé(s) de démonstration ajouté(s). Les données existantes ont été conservées.").arg(ajoutes));
    });
    layout->addWidget(btnDemo);

    QPushButton *btnFichier = new QPushButton("Fichier");
    btnFichier->setObjectName("btnFichier");
    btnFichier->setMinimumHeight(40);
    btnFichier->setMinimumWidth(120);
    btnFichier->setCursor(Qt::PointingHandCursor);

    QMenu *menuFichier = new QMenu(this);
    menuFichier->setStyleSheet(
        "QMenu {"
        "  background-color: #FFFFFF;"
        "  border: 2px solid #D7E9ED;"
        "  border-radius: 12px;"
        "  padding: 8px;"
        "}"
        "QMenu::item {"
        "  padding: 12px 28px 12px 18px;"
        "  border-radius: 8px;"
        "  font-size: 13px;"
        "  color: #1E3A42;"
        "  font-weight: 600;"
        "}"
        "QMenu::item:selected {"
        "  background-color: #EEF7FA;"
        "  color: #1E5A68;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  background-color: #D7E9ED;"
        "  margin: 6px 10px;"
        "}"
        );

    QAction *actEnregistrer = menuFichier->addAction("💾   Enregistrer / vérifier les données");
    connect(actEnregistrer, &QAction::triggered, this, [this]() {
        // Une seule connexion (Singleton) : on lui demande si Oracle repond reellement
        Connection &cnx = Connection::createInstance();
        if (!cnx.isConnected()) {
            QMessageBox::critical(this, "Enregistrement impossible",
                                  "La base de données n'est pas accessible. Aucune confirmation d'enregistrement ne peut être donnée.\n\n" + cnx.lastError());
            return;
        }
        // Oracle (QODBC) : validation automatique (autocommit) apres chaque requete.
        QMessageBox::information(this, "Données enregistrées",
                                 "✅ Connexion Oracle active. Les ajouts, modifications et suppressions sont enregistrés automatiquement dans la base.\n\nBase : " + cnx.databaseName() + " (utilisateur " + cnx.userName() + ")");
    });

    menuFichier->addSeparator();

    QAction *actAttestation = menuFichier->addAction("📝   Générer une attestation de congé (PDF)");
    connect(actAttestation, &QAction::triggered, this, [this]() {
        QDialog dlg(this);
        dlg.setWindowTitle("Attestation de congé");
        dlg.setMinimumWidth(490);
        QVBoxLayout *dialogLayout = new QVBoxLayout(&dlg);
        dialogLayout->setContentsMargins(24, 24, 24, 24);
        dialogLayout->setSpacing(14);
        QLabel *titre = new QLabel("📝 Attestation de congé");
        titre->setStyleSheet("font-size: 20px; font-weight: bold; color: #225A64; background: transparent;");
        dialogLayout->addWidget(titre);
        QFormLayout *form = new QFormLayout;
        form->setSpacing(12);
        QComboBox *employe = new QComboBox;
        const QList<Employe> tousEmployes = Employe::afficherTous();
        for (const Employe &emp : tousEmployes) {
            const QString label = emp.getPrenom() + " " + emp.getNom() + " — " + emp.getPoste();
            employe->addItem(label, emp.getId());
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
        Employe emp;
        if (!Employe::charger(id, emp)) {
            QMessageBox::critical(this, "Erreur", "Impossible de charger les informations de l'employé.");
            return;
        }
        const QString nomComplet = emp.getPrenom() + " " + emp.getNom();
        const QString poste = emp.getPoste();
        const QString dateEmbauche = emp.getDateEmbauche().toString("dd/MM/yyyy");
        QString fichier = QFileDialog::getSaveFileName(this, "Enregistrer l'attestation de congé",
            QString("Attestation_conge_%1.pdf").arg(nomComplet.simplified().replace(' ', '_')),
            "Fichiers PDF (*.pdf)");
        if (fichier.isEmpty()) return;
        if (!fichier.endsWith(".pdf", Qt::CaseInsensitive)) fichier += ".pdf";

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fichier);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setPageMargins(QMarginsF(22, 22, 22, 22), QPageLayout::Millimeter);
        QTextDocument document;
        document.setHtml(QString(
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
                 QDate::currentDate().toString("dd/MM/yyyy")));
        document.print(&printer);
        if (QFileInfo::exists(fichier) && QFileInfo(fichier).size() > 0) {
            QMessageBox::information(this, "Attestation créée", "✅ Attestation de congé PDF créée :\n" + fichier);
        } else {
            QMessageBox::critical(this, "Échec de création", "Le PDF n'a pas pu être créé. Vérifiez les droits d'écriture du dossier.");
        }
    });

    menuFichier->addSeparator();

    QAction *actQuitter = menuFichier->addAction("🚪   Quitter");
    connect(actQuitter, &QAction::triggered, this, &QWidget::close);

    btnFichier->setMenu(menuFichier);
    layout->addWidget(btnFichier);

    // ---- Badge statut Oracle (connecté / non connecté) ----
    m_btnOracle = new QPushButton;
    m_btnOracle->setObjectName("btnOracleStatus");
    m_btnOracle->setMinimumHeight(34);
    m_btnOracle->setCursor(Qt::PointingHandCursor);
    connect(m_btnOracle, &QPushButton::clicked, this, [this]() {
        rafraichirStatutOracle(true);   // clic = retenter la connexion
        if (!Connection::createInstance().isConnected())
            QMessageBox::warning(this, "Oracle non connecté",
                "Impossible de se connecter à Oracle.\n\n" +
                Connection::createInstance().lastError() +
                "\n\nVérifiez que Oracle est démarré et que la source ODBC est correcte.");
    });
    layout->addWidget(m_btnOracle);
    rafraichirStatutOracle();

    QTimer *timerOracle = new QTimer(this);
    connect(timerOracle, &QTimer::timeout, this, [this]() { rafraichirStatutOracle(); });
    timerOracle->start(10000);   // vérification automatique toutes les 10 s

    QPushButton *btnDeconnexion = new QPushButton("Se déconnecter");
    btnDeconnexion->setObjectName("btnDeconnexionTop");
    btnDeconnexion->setIcon(QIcon(":/logout.svg"));
    btnDeconnexion->setIconSize(QSize(15, 15));
    btnDeconnexion->setMinimumHeight(34);
    btnDeconnexion->setCursor(Qt::PointingHandCursor);
    btnDeconnexion->setToolTip("Fermer l’application");
    // L’écran de connexion ayant été supprimé, la déconnexion ferme l’application.
    connect(btnDeconnexion, &QPushButton::clicked, this, &QWidget::close);
    layout->addWidget(btnDeconnexion);


    return bar;
}

// ==================== STATUT ORACLE ====================
void MainWindow::rafraichirStatutOracle(bool retenter)
{
    if (!m_btnOracle) return;
    Connection &c = Connection::createInstance();
    bool ok = c.isConnected();
    if (!ok && retenter)
        ok = c.createConnection() && c.isConnected();

    m_btnOracle->setText(ok ? "●  Oracle connecté" : "●  Oracle non connecté");
    m_btnOracle->setToolTip(ok ? "Connexion Oracle active"
                               : "Oracle injoignable — cliquez pour réessayer");
    m_btnOracle->setProperty("connected", ok);
    m_btnOracle->style()->unpolish(m_btnOracle);
    m_btnOracle->style()->polish(m_btnOracle);
}

// ==================== SIDEBAR ====================
QFrame *MainWindow::creerSidebar()
{
    QFrame *sb = new QFrame;
    sb->setObjectName("sidebar");
    sb->setFixedWidth(280);

    QVBoxLayout *l = new QVBoxLayout(sb);
    l->setContentsMargins(0, 20, 0, 15);
    l->setSpacing(5);

    // ==================== LOGO + TITRE (m3a PULSE) ====================
    m_logoWidget = new QWidget;

    QHBoxLayout *headerL = new QHBoxLayout(m_logoWidget);
    headerL->setContentsMargins(20, 0, 15, 0);
    headerL->setSpacing(12);

    // ---- Logo: transparent, no background, shrinks/grows on hover ----
    HoverLogo *logoImage = new HoverLogo(":/logo.png", QSize(56, 56));
    headerL->addWidget(logoImage);

    QVBoxLayout *textL = new QVBoxLayout;
    textL->setSpacing(0);
    textL->setContentsMargins(0, 0, 0, 0);

    QLabel *logo = new QLabel("ANIWORLD");
    logo->setObjectName("logo");
    logo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    textL->addWidget(logo);

    QLabel *sub = new QLabel("THE OFFICIAL APPLICATION");
    sub->setObjectName("logoSubtitle");
    sub->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    textL->addWidget(sub);

    headerL->addLayout(textL);
    headerL->addStretch();

    l->addWidget(m_logoWidget);
    l->addSpacing(15);

    // ---- Card Centre ----
    QFrame *centreCard = new QFrame;
    centreCard->setObjectName("centreCard");
    QVBoxLayout *centreL = new QVBoxLayout(centreCard);
    centreL->setContentsMargins(18, 12, 18, 12);
    centreL->setSpacing(3);

    QLabel *centreTitre = new QLabel("Centre de soins");
    centreTitre->setStyleSheet("font-size: 13px; font-weight: 900; color: #0F3D48;"
                               "background: transparent;");
    centreL->addWidget(centreTitre);

    QLabel *centreSous = new QLabel("Espace de gestion");
    centreSous->setStyleSheet("font-size: 11px; color: #1E5A68; background: transparent;"
                              "font-weight: 600;");
    centreL->addWidget(centreSous);

    l->addWidget(centreCard);
    l->addSpacing(20);

    // ---- Navigation ----
    QLabel *section = new QLabel("NAVIGATION");
    section->setObjectName("sectionTitle");
    l->addWidget(section);

    m_btnTableau = creerBoutonMenu("📊", "Tableau de bord");
    connect(m_btnTableau, &QPushButton::clicked, this, &MainWindow::afficherTableauDeBord);
    l->addWidget(m_btnTableau);

    m_btnServices = creerBoutonMenu("🐾", "Services");
    connect(m_btnServices, &QPushButton::clicked, this, &MainWindow::afficherServices);
    l->addWidget(m_btnServices);

    m_btnEmployes = creerBoutonMenu("👥", "Employés");
    m_btnEmployes->setChecked(true);
    connect(m_btnEmployes, &QPushButton::clicked, this, &MainWindow::afficherEmployes);
    l->addWidget(m_btnEmployes);

    m_btnStock = creerBoutonMenu("📦", "Stock");
    connect(m_btnStock, &QPushButton::clicked, this, &MainWindow::afficherStock);
    l->addWidget(m_btnStock);

    m_btnLocaux = creerBoutonMenu("🏠", "Locaux");
    connect(m_btnLocaux, &QPushButton::clicked, this, &MainWindow::afficherLocaux);
    l->addWidget(m_btnLocaux);

    m_btnDossiers = creerBoutonMenu("📋", "Dossiers médicaux");
    connect(m_btnDossiers, &QPushButton::clicked, this, &MainWindow::afficherDossiers);
    l->addWidget(m_btnDossiers);

    l->addStretch();

    // ---- Footer ----
    QLabel *footer1 = new QLabel("Une attention à chaque patte.");
    footer1->setObjectName("footerText");
    footer1->setAlignment(Qt::AlignCenter);
    l->addWidget(footer1);

    QLabel *footer2 = new QLabel("ANIWORLD · Centre de soins");
    footer2->setObjectName("footerSub");
    footer2->setAlignment(Qt::AlignCenter);
    l->addWidget(footer2);

    return sb;
}

QPushButton *MainWindow::creerBoutonMenu(const QString &icone, const QString &texte)
{
    QPushButton *b = new QPushButton(icone + "    " + texte);
    b->setObjectName("menuButton");
    b->setCheckable(true);
    b->setCursor(Qt::PointingHandCursor);
    b->setMinimumHeight(44);
    return b;
}

// ==================== PAGE EMPLOYÉS ====================
QWidget *MainWindow::creerPageEmployes()
{
    m_gestionEmployes = new GestionEmployes;
    return m_gestionEmployes;
}

// ==================== PAGE PLACEHOLDER ====================
QWidget *MainWindow::creerPagePlaceholder(const QString &titre, const QString &icone,
                                          const QString &description)
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(30, 25, 30, 25);
    layout->setSpacing(15);

    QHBoxLayout *headerL = new QHBoxLayout;
    QVBoxLayout *titreL = new QVBoxLayout;
    titreL->setSpacing(4);

    QLabel *lblTitre = new QLabel(icone + "  " + titre);
    lblTitre->setStyleSheet("font-size: 30px; font-weight: 900; color: #1E5A68;"
                            "background: transparent;");
    titreL->addWidget(lblTitre);

    QLabel *lblSous = new QLabel(description);
    lblSous->setStyleSheet("font-size: 13px; color: #7A8B8F; background: transparent;");
    titreL->addWidget(lblSous);

    headerL->addLayout(titreL);
    headerL->addStretch();
    layout->addLayout(headerL);

    QFrame *card = new QFrame;
    card->setObjectName("card");
    QVBoxLayout *cardL = new QVBoxLayout(card);
    cardL->setContentsMargins(50, 80, 50, 80);
    cardL->setSpacing(15);

    QLabel *ico = new QLabel(icone);
    ico->setAlignment(Qt::AlignCenter);
    ico->setStyleSheet("font-size: 90px; background: transparent;");
    cardL->addWidget(ico);

    QLabel *msg = new QLabel("Module en cours de développement");
    msg->setAlignment(Qt::AlignCenter);
    msg->setStyleSheet("font-size: 22px; font-weight: 900; color: #1E5A68;"
                       "background: transparent;");
    cardL->addWidget(msg);

    QLabel *sub = new QLabel("Cette fonctionnalité sera disponible prochainement.");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("font-size: 13px; color: #7A8B8F; background: transparent;");
    cardL->addWidget(sub);

    cardL->addStretch();
    layout->addWidget(card, 1);

    return page;
}

// ==================== FADE-IN PAGE ====================
void MainWindow::fadeInPage(QWidget *page)
{
    if (!page) return;

    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(page);
    page->setGraphicsEffect(eff);

    QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity");
    anim->setDuration(400);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, &QPropertyAnimation::finished, [page]() {
        page->setGraphicsEffect(nullptr);
    });

    anim->start(QPropertyAnimation::DeleteWhenStopped);
}

// ==================== SLOTS ====================
void MainWindow::afficherEmployes()
{
    m_stack->setCurrentIndex(0);
    if (m_gestionEmployes)
        m_gestionEmployes->actualiserStats();
    m_btnTableau->setChecked(false);
    m_btnServices->setChecked(false);
    m_btnEmployes->setChecked(true);
    m_btnStock->setChecked(false);
    m_btnLocaux->setChecked(false);
    m_btnDossiers->setChecked(false);
}

void MainWindow::afficherTableauDeBord()
{
    m_stack->setCurrentIndex(1);
    fadeInPage(m_stack->currentWidget());
    m_btnTableau->setChecked(true);
    m_btnServices->setChecked(false);
    m_btnEmployes->setChecked(false);
    m_btnStock->setChecked(false);
    m_btnLocaux->setChecked(false);
    m_btnDossiers->setChecked(false);
}

void MainWindow::afficherServices()
{
    m_stack->setCurrentIndex(2);
    fadeInPage(m_stack->currentWidget());
    m_btnTableau->setChecked(false);
    m_btnServices->setChecked(true);
    m_btnEmployes->setChecked(false);
    m_btnStock->setChecked(false);
    m_btnLocaux->setChecked(false);
    m_btnDossiers->setChecked(false);
}

void MainWindow::afficherStock()
{
    m_stack->setCurrentIndex(3);
    fadeInPage(m_stack->currentWidget());
    m_btnTableau->setChecked(false);
    m_btnServices->setChecked(false);
    m_btnEmployes->setChecked(false);
    m_btnStock->setChecked(true);
    m_btnLocaux->setChecked(false);
    m_btnDossiers->setChecked(false);
}

void MainWindow::afficherLocaux()
{
    m_stack->setCurrentIndex(4);
    fadeInPage(m_stack->currentWidget());
    m_btnTableau->setChecked(false);
    m_btnServices->setChecked(false);
    m_btnEmployes->setChecked(false);
    m_btnStock->setChecked(false);
    m_btnLocaux->setChecked(true);
    m_btnDossiers->setChecked(false);
}

void MainWindow::afficherDossiers()
{
    m_stack->setCurrentIndex(5);
    fadeInPage(m_stack->currentWidget());
    m_btnTableau->setChecked(false);
    m_btnServices->setChecked(false);
    m_btnEmployes->setChecked(false);
    m_btnStock->setChecked(false);
    m_btnLocaux->setChecked(false);
    m_btnDossiers->setChecked(true);
}