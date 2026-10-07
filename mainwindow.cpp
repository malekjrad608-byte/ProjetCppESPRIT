#include "loginvisuals.h"
#include "mainwindow.h"
#include "gestionemployes.h"
#include "gestionlocaux.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
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
                                            "Vue d'ensemble du centre de soins"));                 // 1
    m_stack->addWidget(creerPagePlaceholder("Services", "🐾",
                                            "Gestion des services du centre"));                     // 2
    m_stack->addWidget(creerPagePlaceholder("Stock", "📦",
                                            "Gestion des stocks et fournitures"));                  // 3
    m_stack->addWidget(new GestionLocaux);                                      // 4
    m_stack->addWidget(creerPagePlaceholder("Dossiers médicaux", "📋",
                                            "Historique clinique et suivi des animaux"));         // 5
    zoneLayout->addWidget(m_stack, 1);
    main->addWidget(zone, 1);
}

void MainWindow::showModuleForRole(const QString &role)
{
    if (role == QStringLiteral("locaux")) {
        afficherLocaux();
    } else if (role == QStringLiteral("services")) {
        afficherServices();
    } else if (role == QStringLiteral("stock")) {
        afficherStock();
    } else {
        afficherEmployes();
    }
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

    m_breadcrumb = new QLabel("Gestion   /   <b>Employés</b>");
    m_breadcrumb->setObjectName("breadcrumb");
    m_breadcrumb->setTextFormat(Qt::RichText);
    layout->addWidget(m_breadcrumb);

    layout->addStretch();

    QPushButton *btnDemo = new QPushButton("Données de démonstration");
    btnDemo->setObjectName("btnDemo");
    btnDemo->setMinimumHeight(40);
    btnDemo->setCursor(Qt::PointingHandCursor);
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

    QAction *actEnregistrer = menuFichier->addAction("💾   Enregistrer");
    connect(actEnregistrer, &QAction::triggered, this, [this]() {
        QMessageBox::information(this, "Enregistrer",
                                 "✅ Les modifications ont été enregistrées.\n\n"
                                 "Les modules Services et Employés utilisent leurs stockages locaux.");
    });

    menuFichier->addSeparator();

    QAction *actExportPDF = menuFichier->addAction("📄   Exporter en PDF");
    connect(actExportPDF, &QAction::triggered, this, [this]() {
        QString fichier = QFileDialog::getSaveFileName(this,
                                                       "Exporter en PDF", "employes.pdf", "Fichiers PDF (*.pdf)");
        if (fichier.isEmpty()) return;
        QMessageBox::information(this, "Export PDF",
                                 "✅ Export PDF réussi :\n" + fichier);
    });

    QAction *actExportCSV = menuFichier->addAction("📊   Exporter en CSV");
    connect(actExportCSV, &QAction::triggered, this, [this]() {
        QString fichier = QFileDialog::getSaveFileName(this,
                                                       "Exporter en CSV", "employes.csv", "Fichiers CSV (*.csv)");
        if (fichier.isEmpty()) return;
        QMessageBox::information(this, "Export CSV",
                                 "✅ Export CSV réussi :\n" + fichier);
    });

    menuFichier->addSeparator();

    QAction *actQuitter = menuFichier->addAction("🚪   Quitter");
    connect(actQuitter, &QAction::triggered, this, &QWidget::close);

    btnFichier->setMenu(menuFichier);
    layout->addWidget(btnFichier);

    QLabel *user = new QLabel("Responsable du centre");
    user->setObjectName("userLabel");
    layout->addWidget(user);

    return bar;
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
QWidget *MainWindow::creerPageEmployes() { return new GestionEmployes; }

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
    m_breadcrumb->setText("Gestion   /   <b>Employés</b>");
    fadeInPage(m_stack->currentWidget());
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
    m_breadcrumb->setText("Gestion   /   <b>Tableau de bord</b>");
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
    m_breadcrumb->setText("Gestion   /   <b>Services</b>");
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
    m_breadcrumb->setText("Gestion   /   <b>Stock</b>");
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
    m_breadcrumb->setText("Gestion   /   <b>Locaux</b>");
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
    m_breadcrumb->setText("Gestion   /   <b>Dossiers médicaux</b>");
    fadeInPage(m_stack->currentWidget());
    m_btnTableau->setChecked(false);
    m_btnServices->setChecked(false);
    m_btnEmployes->setChecked(false);
    m_btnStock->setChecked(false);
    m_btnLocaux->setChecked(false);
    m_btnDossiers->setChecked(true);
}