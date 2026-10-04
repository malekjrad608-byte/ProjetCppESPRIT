#include "mainwindow.h"
#include "statsdialog.h"
#include "sallecard.h"
#include "icones.h"
#include "logoanimated.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QToolButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QPixmap>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QScrollArea>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QTableWidget>
#include <QTabWidget>
#include <QSpinBox>
#include <QDateEdit>
#include <QDate>
#include <QLocale>
#include <QTimer>
#include <QEvent>
#include <QStyle>
#include <QSet>
#include <functional>
#include <memory>

// ---------- Réappliquer le style après un changement de propriété ----------
static void repolir(QWidget* w)
{
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

static bool estSalleAvecCages(const Salle* s)
{
    return s && (s->getType() == "Pension" || s->getType() == "Adoption");
}

// Texte du statut d'un séjour (jours restants / dépassement)
static QString libelleSejour(const Cage& c)
{
    if (!c.estOccupee()) return "—";
    const int r = c.joursRestants();
    if (r < 0)  return QString("⚠ Dépassé de %1 j").arg(-r);
    if (r == 0) return "Dernier jour";
    return QString("Dans %1 j").arg(r);
}

// Index des pages du QStackedWidget
enum PageIndex {
    PageDashboard = 0,
    PageServices  = 1,
    PageEmployes  = 2,
    PageStock     = 3,
    PageLocaux    = 4,
    PageDossiers  = 5,
    PagePlan      = 6
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    chargerDonneesDemo();
    setupUI();
    rafraichirCartes();
    rafraichirMap();
}

MainWindow::~MainWindow() {}

// ============================================================
//                    CONSTRUCTION DE L'INTERFACE
// ============================================================
void MainWindow::setupUI()
{
    setWindowTitle("ANIWORLD — Gestion du centre");
    setWindowIcon(QIcon(":/logo.png"));
    resize(1280, 800);
    setMinimumSize(1000, 650);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    root->addWidget(creerSidebar());

    QWidget* droite = new QWidget();
    QVBoxLayout* colDroite = new QVBoxLayout(droite);
    colDroite->setContentsMargins(0, 0, 0, 0);
    colDroite->setSpacing(0);

    colDroite->addWidget(creerTopBar());

    pages = new QStackedWidget();
    pages->setObjectName("pages");
    pages->addWidget(creerPagePlaceholder("Tableau de bord"));   // 0
    pages->addWidget(creerPagePlaceholder("Services"));          // 1
    pages->addWidget(creerPagePlaceholder("Employés"));          // 2
    pages->addWidget(creerPagePlaceholder("Stock"));             // 3
    pages->addWidget(creerPageLocaux());                         // 4
    pages->addWidget(creerPagePlaceholder("Dossiers médicaux")); // 5
    pages->addWidget(creerPagePlan());                           // 6
    pages->setCurrentIndex(PageLocaux);
    colDroite->addWidget(pages, 1);

    colDroite->addWidget(creerFooter());
    root->addWidget(droite, 1);

    connect(searchBar, &QLineEdit::textChanged, this, &MainWindow::rechercheChangee);
    connect(filtreType, &QComboBox::currentTextChanged, this, &MainWindow::rafraichirCartes);
    connect(filtreEtat, &QComboBox::currentTextChanged, this, &MainWindow::rafraichirCartes);
    connect(triCombo, &QComboBox::currentTextChanged, this, &MainWindow::rafraichirCartes);
}

// ---------------------------------------------------------------
// Barre latérale (logo animé, carte "Centre de soins", navigation)
// ---------------------------------------------------------------
QWidget* MainWindow::creerSidebar()
{
    QFrame* side = new QFrame();
    side->setObjectName("sidebar");
    side->setFixedWidth(250);

    QVBoxLayout* l = new QVBoxLayout(side);
    l->setContentsMargins(14, 18, 14, 14);
    l->setSpacing(6);

    // --- Logo animé + nom ---
    QHBoxLayout* brand = new QHBoxLayout();
    brand->setSpacing(8);

    brand->addWidget(new LogoAnime(":/logo.png", 54));

    QVBoxLayout* brandTxt = new QVBoxLayout();
    brandTxt->setSpacing(0);
    QLabel* nom = new QLabel("ANIWORLD");
    nom->setObjectName("brandName");
    QLabel* tag = new QLabel("THE OFFICIAL APPLICATION");
    tag->setObjectName("brandTag");
    brandTxt->addStretch();
    brandTxt->addWidget(nom);
    brandTxt->addWidget(tag);
    brandTxt->addStretch();
    brand->addLayout(brandTxt, 1);

    l->addLayout(brand);
    l->addSpacing(10);

    // --- Carte "Centre de soins" ---
    QFrame* carte = new QFrame();
    carte->setObjectName("centreCard");
    QVBoxLayout* cl = new QVBoxLayout(carte);
    cl->setContentsMargins(14, 12, 14, 12);
    cl->setSpacing(2);
    QLabel* ct = new QLabel("Centre de soins");
    ct->setObjectName("centreTitle");
    QLabel* cs = new QLabel("Espace de gestion");
    cs->setObjectName("centreSub");
    cl->addWidget(ct);
    cl->addWidget(cs);
    l->addWidget(carte);
    l->addSpacing(14);

    // --- Navigation ---
    QLabel* sec = new QLabel("NAVIGATION");
    sec->setObjectName("navSection");
    l->addWidget(sec);

    struct Entree { const char* texte; const char* icone; };
    const Entree entrees[] = {
        {"Tableau de bord",   "dashboard"},
        {"Services",          "calendar"},
        {"Employés",          "users"},
        {"Stock",             "box"},
        {"Locaux",            "rooms"},
        {"Dossiers médicaux", "shield"}
    };

    QButtonGroup* groupe = new QButtonGroup(this);
    groupe->setExclusive(true);

    for (int i = 0; i < 6; ++i) {
        QPushButton* b = new QPushButton(QString(entrees[i].texte));
        b->setObjectName("navBtn");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        b->setIcon(Icones::deuxEtats(entrees[i].icone, QColor("#5F7178"), QColor("#2D6675")));
        b->setIconSize(QSize(18, 18));
        groupe->addButton(b);
        l->addWidget(b);
        if (i == PageLocaux) b->setChecked(true);
        connect(b, &QPushButton::clicked, this, [this, i]() { allerPage(i); });
    }

    l->addStretch(1);

    QLabel* slogan = new QLabel("Une attention à chaque patte.");
    slogan->setObjectName("sideSlogan");
    slogan->setWordWrap(true);
    QLabel* pied = new QLabel("ANIWORLD · Centre de soins");
    pied->setObjectName("sideFoot");
    l->addWidget(slogan);
    l->addWidget(pied);

    return side;
}

// ---------------------------------------------------------------
// Barre du haut
// ---------------------------------------------------------------
QWidget* MainWindow::creerTopBar()
{
    QFrame* bar = new QFrame();
    bar->setObjectName("topBar");
    bar->setFixedHeight(56);

    QHBoxLayout* l = new QHBoxLayout(bar);
    l->setContentsMargins(28, 0, 28, 0);
    l->setSpacing(16);

    QHBoxLayout* crumb = new QHBoxLayout();
    crumb->setSpacing(8);
    QLabel* c1 = new QLabel("Gestion");
    c1->setObjectName("crumb");
    QLabel* sep = new QLabel("/");
    sep->setObjectName("crumb");
    crumbCourant = new QLabel("Locaux");
    crumbCourant->setObjectName("crumbCurrent");
    crumb->addWidget(c1);
    crumb->addWidget(sep);
    crumb->addWidget(crumbCourant);
    l->addLayout(crumb);
    l->addStretch(1);

    QLabel* badge = new QLabel("Données de démonstration");
    badge->setObjectName("badgeDemo");
    l->addWidget(badge);

    QToolButton* btnFichier = new QToolButton();
    btnFichier->setObjectName("ghostBtn");
    btnFichier->setText("Fichier  ▾");
    btnFichier->setPopupMode(QToolButton::InstantPopup);
    btnFichier->setCursor(Qt::PointingHandCursor);

    QMenu* menuFichier = new QMenu(btnFichier);
    QAction* actSauver = menuFichier->addAction("Enregistrer");
    QAction* actCharger = menuFichier->addAction("Charger un CSV…");
    menuFichier->addSeparator();
    QAction* actExportCSV = menuFichier->addAction("Exporter en CSV");
    QAction* actExportPDF = menuFichier->addAction("Exporter en PDF");
    btnFichier->setMenu(menuFichier);

    connect(actSauver, &QAction::triggered, this, &MainWindow::sauvegarderFichier);
    connect(actCharger, &QAction::triggered, this, &MainWindow::chargerFichier);
    connect(actExportCSV, &QAction::triggered, this, &MainWindow::exporterCSV);
    connect(actExportPDF, &QAction::triggered, this, &MainWindow::exporterPDF);
    l->addWidget(btnFichier);

    QLabel* user = new QLabel("Responsable du centre");
    user->setObjectName("userLabel");
    l->addWidget(user);

    return bar;
}

QWidget* MainWindow::creerFooter()
{
    QFrame* f = new QFrame();
    f->setObjectName("footerBar");
    f->setFixedHeight(36);

    QHBoxLayout* l = new QHBoxLayout(f);
    l->setContentsMargins(28, 0, 28, 0);

    QLabel* gauche = new QLabel("ANIWORLD · Gestion du centre");
    gauche->setObjectName("footerLabel");
    QLabel* date = new QLabel(QLocale(QLocale::French).toString(QDate::currentDate(), "d MMMM yyyy"));
    date->setObjectName("footerLabel");

    l->addWidget(gauche);
    l->addStretch(1);
    l->addWidget(date);
    return f;
}

QWidget* MainWindow::creerPagePlaceholder(const QString& titre)
{
    QWidget* p = new QWidget();
    QVBoxLayout* v = new QVBoxLayout(p);
    v->setContentsMargins(28, 24, 28, 12);
    v->setSpacing(4);

    QLabel* t = new QLabel(titre);
    t->setObjectName("pageTitle");
    QLabel* s = new QLabel("Cette section sera disponible prochainement.");
    s->setObjectName("pageSub");
    v->addWidget(t);
    v->addWidget(s);
    v->addStretch(1);
    return p;
}

// ---------------------------------------------------------------
// Page "Locaux"
// ---------------------------------------------------------------
QWidget* MainWindow::creerPageLocaux()
{
    QWidget* page = new QWidget();
    QHBoxLayout* h = new QHBoxLayout(page);
    h->setContentsMargins(28, 24, 28, 12);
    h->setSpacing(20);

    QVBoxLayout* gauche = new QVBoxLayout();
    gauche->setSpacing(14);

    QHBoxLayout* entete = new QHBoxLayout();
    QVBoxLayout* titres = new QVBoxLayout();
    titres->setSpacing(2);
    QLabel* t = new QLabel("Locaux");
    t->setObjectName("pageTitle");
    QLabel* st = new QLabel("Occupation, nettoyage et disponibilité des espaces");
    st->setObjectName("pageSub");
    titres->addWidget(t);
    titres->addWidget(st);
    entete->addLayout(titres);
    entete->addStretch(1);

    QPushButton* btnAjouter = new QPushButton("+ Ajouter un local");
    btnAjouter->setObjectName("primaryBtn");
    btnAjouter->setCursor(Qt::PointingHandCursor);
    entete->addWidget(btnAjouter, 0, Qt::AlignTop);
    gauche->addLayout(entete);

    QHBoxLayout* ligPills = new QHBoxLayout();
    ligPills->setSpacing(10);
    pillDispo = new QLabel();
    pillDispo->setObjectName("pillDispo");
    pillOccupe = new QLabel();
    pillOccupe->setObjectName("pillOccupe");
    pillHors = new QLabel();
    pillHors->setObjectName("pillHors");
    ligPills->addWidget(pillDispo);
    ligPills->addWidget(pillOccupe);
    ligPills->addWidget(pillHors);
    ligPills->addStretch(1);

    QPushButton* btnPlan = new QPushButton("Plan de la clinique");
    btnPlan->setObjectName("outlineBtn");
    btnPlan->setCursor(Qt::PointingHandCursor);
    ligPills->addWidget(btnPlan);
    gauche->addLayout(ligPills);

    QHBoxLayout* filtres = new QHBoxLayout();
    filtres->setSpacing(10);

    searchBar = new QLineEdit();
    searchBar->setPlaceholderText("Rechercher par nom, ID ou type…");
    searchBar->addAction(Icones::creer("search", QColor("#8A989E"), 16), QLineEdit::LeadingPosition);
    filtres->addWidget(searchBar, 3);

    filtreType = new QComboBox();
    filtreType->addItems({"Tous types", "Consultation", "Chirurgie",
                          "Toilettage", "Pension", "Adoption", "Storage"});
    filtres->addWidget(filtreType, 1);

    filtreEtat = new QComboBox();
    filtreEtat->addItem("Tous états", "");
    filtreEtat->addItem("Disponible", "Libre");
    filtreEtat->addItem("Occupée", "Occupée");
    filtreEtat->addItem("Hors service", "Hors service");
    filtres->addWidget(filtreEtat, 1);

    triCombo = new QComboBox();
    triCombo->addItems({"Tri par défaut", "Capacité ↑", "Capacité ↓",
                        "Occupation ↑", "Occupation ↓"});
    filtres->addWidget(triCombo, 1);
    gauche->addLayout(filtres);

    scrollCartes = new QScrollArea();
    scrollCartes->setWidgetResizable(true);
    scrollCartes->setFrameShape(QFrame::NoFrame);
    scrollCartes->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    conteneurCartes = new QWidget();
    conteneurCartes->setObjectName("cardsContainer");
    QVBoxLayout* cv = new QVBoxLayout(conteneurCartes);
    // Marges élargies : laissent la place à l'ombre des cartes au survol
    cv->setContentsMargins(6, 8, 16, 26);
    cv->setSpacing(0);

    grilleCartes = new QGridLayout();
    grilleCartes->setSpacing(16);
    cv->addLayout(grilleCartes);

    lblVide = new QLabel("Aucun local ne correspond à votre recherche.");
    lblVide->setObjectName("emptyLabel");
    lblVide->setAlignment(Qt::AlignCenter);
    lblVide->hide();
    cv->addWidget(lblVide);
    cv->addStretch(1);

    scrollCartes->setWidget(conteneurCartes);
    scrollCartes->viewport()->installEventFilter(this);
    gauche->addWidget(scrollCartes, 1);

    h->addLayout(gauche, 1);
    h->addWidget(creerPanneauDroit());

    connect(btnAjouter, &QPushButton::clicked, this, &MainWindow::ajouterSalle);
    connect(btnPlan, &QPushButton::clicked, this, [this]() { allerPage(PagePlan); });

    return page;
}

// ---------------------------------------------------------------
// Panneau de droite
// ---------------------------------------------------------------
QWidget* MainWindow::creerPanneauDroit()
{
    QScrollArea* zone = new QScrollArea();
    zone->setWidgetResizable(true);
    zone->setFrameShape(QFrame::NoFrame);
    zone->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    zone->setFixedWidth(336);

    QWidget* contenu = new QWidget();
    contenu->setObjectName("cardsContainer");
    QVBoxLayout* v = new QVBoxLayout(contenu);
    v->setContentsMargins(0, 0, 8, 0);
    v->setSpacing(14);

    // ================= Carte 1 : local sélectionné =================
    QFrame* c1 = new QFrame();
    c1->setObjectName("panelCard");
    QVBoxLayout* l1 = new QVBoxLayout(c1);
    l1->setContentsMargins(18, 16, 18, 18);
    l1->setSpacing(8);

    QLabel* kicker = new QLabel("LOCAL SÉLECTIONNÉ");
    kicker->setObjectName("panelKicker");
    l1->addWidget(kicker);

    panTitre = new QLabel("Aucun local sélectionné");
    panTitre->setObjectName("panelTitle");
    panTitre->setWordWrap(true);
    l1->addWidget(panTitre);

    panPill = new QLabel();
    panPill->setObjectName("cardPill");
    panPill->setProperty("etat", "dispo");
    QHBoxLayout* lp = new QHBoxLayout();
    lp->addWidget(panPill);
    lp->addStretch(1);
    l1->addLayout(lp);

    panInfos = new QLabel();
    panInfos->setObjectName("panelInfo");
    panInfos->setWordWrap(true);
    l1->addWidget(panInfos);

    boxRemise = new QWidget();
    QVBoxLayout* lr = new QVBoxLayout(boxRemise);
    lr->setContentsMargins(0, 8, 0, 0);
    lr->setSpacing(8);
    QLabel* rt = new QLabel("Remise en service");
    rt->setObjectName("panelSection");
    lr->addWidget(rt);
    chkSurfaces = new QCheckBox("Surfaces nettoyées");
    chkMateriel = new QCheckBox("Matériel rangé");
    chkControle = new QCheckBox("Contrôle final validé");
    lr->addWidget(chkSurfaces);
    lr->addWidget(chkMateriel);
    lr->addWidget(chkControle);
    btnValider = new QPushButton("Valider la remise en service");
    btnValider->setObjectName("primaryBtn");
    btnValider->setEnabled(false);
    lr->addSpacing(4);
    lr->addWidget(btnValider);
    l1->addWidget(boxRemise);

    auto majValider = [this]() {
        btnValider->setEnabled(chkSurfaces->isChecked()
                               && chkMateriel->isChecked()
                               && chkControle->isChecked());
    };
    connect(chkSurfaces, &QCheckBox::toggled, this, majValider);
    connect(chkMateriel, &QCheckBox::toggled, this, majValider);
    connect(chkControle, &QCheckBox::toggled, this, majValider);
    connect(btnValider, &QPushButton::clicked, this, &MainWindow::validerRemiseEnService);

    l1->addSpacing(6);
    btnAffecter = new QPushButton("Affecter un animal");
    btnAffecter->setObjectName("primaryBtn");
    l1->addWidget(btnAffecter);

    QHBoxLayout* ligne2 = new QHBoxLayout();
    ligne2->setSpacing(8);
    btnLiberer = new QPushButton("Libérer");
    btnLiberer->setObjectName("outlineBtn");
    btnModifier = new QPushButton("Modifier");
    btnModifier->setObjectName("outlineBtn");
    ligne2->addWidget(btnLiberer);
    ligne2->addWidget(btnModifier);
    l1->addLayout(ligne2);

    btnCages = new QPushButton("Gérer les cages");
    btnCages->setObjectName("outlineBtn");
    l1->addWidget(btnCages);

    btnSupprimer = new QPushButton("Supprimer le local");
    btnSupprimer->setObjectName("dangerBtn");
    l1->addWidget(btnSupprimer);

    for (QPushButton* b : {btnAffecter, btnLiberer, btnModifier, btnCages, btnSupprimer, btnValider})
        b->setCursor(Qt::PointingHandCursor);

    connect(btnAffecter, &QPushButton::clicked, this, &MainWindow::affecterAnimal);
    connect(btnLiberer, &QPushButton::clicked, this, &MainWindow::libererSalle);
    connect(btnModifier, &QPushButton::clicked, this, &MainWindow::modifierSalle);
    connect(btnCages, &QPushButton::clicked, this, &MainWindow::ouvrirGestionCages);
    connect(btnSupprimer, &QPushButton::clicked, this, &MainWindow::supprimerSalle);

    v->addWidget(c1);

    // ================= Carte 2 : occupation globale =================
    QFrame* c2 = new QFrame();
    c2->setObjectName("panelCard");
    QVBoxLayout* l2 = new QVBoxLayout(c2);
    l2->setContentsMargins(18, 16, 18, 18);
    l2->setSpacing(8);

    QLabel* t2 = new QLabel("Occupation globale");
    t2->setObjectName("panelSection");
    l2->addWidget(t2);

    lblTaux = new QLabel("0 %");
    lblTaux->setObjectName("panelBig");
    l2->addWidget(lblTaux);

    barTaux = new QProgressBar();
    barTaux->setObjectName("occBar");
    barTaux->setRange(0, 100);
    barTaux->setTextVisible(false);
    l2->addWidget(barTaux);

    lblTauxNote = new QLabel();
    lblTauxNote->setObjectName("panelInfo");
    lblTauxNote->setWordWrap(true);
    l2->addWidget(lblTauxNote);

    QPushButton* btnStats = new QPushButton("Voir les statistiques");
    btnStats->setObjectName("outlineBtn");
    btnStats->setCursor(Qt::PointingHandCursor);
    l2->addSpacing(4);
    l2->addWidget(btnStats);
    connect(btnStats, &QPushButton::clicked, this, &MainWindow::ouvrirStats);

    v->addWidget(c2);
    v->addStretch(1);

    zone->setWidget(contenu);
    return zone;
}

// ---------------------------------------------------------------
// Page "Plan de la clinique"
// ---------------------------------------------------------------
QWidget* MainWindow::creerPagePlan()
{
    QWidget* page = new QWidget();
    QVBoxLayout* v = new QVBoxLayout(page);
    v->setContentsMargins(28, 24, 28, 12);
    v->setSpacing(14);

    QHBoxLayout* h = new QHBoxLayout();
    QVBoxLayout* titres = new QVBoxLayout();
    titres->setSpacing(2);
    QLabel* t = new QLabel("Plan de la clinique");
    t->setObjectName("pageTitle");
    QLabel* st = new QLabel("Vue d'ensemble des locaux — clique sur une salle pour ses détails");
    st->setObjectName("pageSub");
    titres->addWidget(t);
    titres->addWidget(st);
    h->addLayout(titres);
    h->addStretch(1);

    QPushButton* retour = new QPushButton("← Retour aux locaux");
    retour->setObjectName("outlineBtn");
    retour->setCursor(Qt::PointingHandCursor);
    h->addWidget(retour, 0, Qt::AlignTop);
    v->addLayout(h);

    QScrollArea* zone = new QScrollArea();
    zone->setWidgetResizable(true);
    zone->setFrameShape(QFrame::NoFrame);
    mapWidget = new MapWidget();
    zone->setWidget(mapWidget);
    v->addWidget(zone, 1);

    connect(mapWidget, &MapWidget::salleCliquee, this, &MainWindow::surClicSalleMap);
    connect(retour, &QPushButton::clicked, this, [this]() { allerPage(PageLocaux); });

    return page;
}

// ============================================================
//                 NAVIGATION + ÉVÉNEMENTS
// ============================================================
void MainWindow::allerPage(int index)
{
    static const QStringList noms = {
        "Tableau de bord", "Services", "Employés", "Stock",
        "Locaux", "Dossiers médicaux", "Plan de la clinique"
    };
    if (index < 0 || index >= pages->count()) return;

    pages->setCurrentIndex(index);
    if (crumbCourant && index < noms.size())
        crumbCourant->setText(noms[index]);

    if (index == PagePlan) rafraichirMap();
}

bool MainWindow::eventFilter(QObject* obj, QEvent* ev)
{
    if (scrollCartes && obj == scrollCartes->viewport() && ev->type() == QEvent::Resize) {
        QTimer::singleShot(0, this, [this]() { reflowCartes(false); });
    }
    return QMainWindow::eventFilter(obj, ev);
}

// ============================================================
//                    DONNÉES DE DÉMO
// ============================================================
void MainWindow::chargerDonneesDemo()
{
    gestionnaire.ajouter(new SalleConsultation(101, "Consultation 1", 1, "Généraliste"));
    gestionnaire.ajouter(new SalleConsultation(102, "Consultation 2", 1, "Généraliste"));
    gestionnaire.ajouter(new SalleConsultation(103, "Consultation 3", 1, "Dentiste"));
    gestionnaire.ajouter(new SalleConsultation(104, "Consultation 4", 1, "Ophtalmo"));
    gestionnaire.ajouter(new SalleConsultation(105, "Consultation 5", 1, "Généraliste"));
    gestionnaire.ajouter(new SalleChirurgie(201, "Bloc A", 1, "Aseptique"));
    gestionnaire.ajouter(new SalleChirurgie(202, "Bloc B", 1, "Standard"));
    gestionnaire.ajouter(new SalleToilettage(301, "Spa & Bain 1", 2, "Hydro-massage"));
    gestionnaire.ajouter(new SalleToilettage(302, "Spa & Bain 2", 2, "Classique"));
    gestionnaire.ajouter(new SallePension(401, "Pension principale", 20, "Mixte"));
    gestionnaire.ajouter(new SalleAdoption(501, "Grande salle d'adoption", 20, "Exposition adoption"));
    gestionnaire.ajouter(new SalleStorage(601, "Chambre de stockage", 10, "Médicaments et matériel"));
}

// ============================================================
//                 AFFICHAGE : CARTES + PANNEAU
// ============================================================
void MainWindow::rafraichirCartes()
{
    QVector<Salle*> liste;
    QString texte = searchBar->text().trimmed();
    if (texte.isEmpty()) liste = gestionnaire.getToutesLesSalles();
    else liste = gestionnaire.rechercher(texte);

    QVector<Salle*> ordre;
    QString tri = triCombo->currentText();
    if (tri == "Capacité ↑") ordre = gestionnaire.trierParCapacite(true);
    else if (tri == "Capacité ↓") ordre = gestionnaire.trierParCapacite(false);
    else if (tri == "Occupation ↑") ordre = gestionnaire.trierParOccupation(true);
    else if (tri == "Occupation ↓") ordre = gestionnaire.trierParOccupation(false);

    if (!ordre.isEmpty()) {
        QSet<Salle*> gardees;
        for (Salle* s : liste) gardees.insert(s);
        QVector<Salle*> triee;
        for (Salle* s : ordre)
            if (gardees.contains(s)) triee.append(s);
        liste = triee;
    }

    QString type = filtreType->currentText();
    QString etat = filtreEtat->currentData().toString();

    for (SalleCard* c : cartes) {
        grilleCartes->removeWidget(c);
        c->hide();
        c->deleteLater();
    }
    cartes.clear();

    for (Salle* s : liste) {
        if (type != "Tous types" && s->getType() != type) continue;
        if (!etat.isEmpty() && s->getEtat() != etat) continue;

        SalleCard* c = new SalleCard(s, conteneurCartes);
        connect(c, &SalleCard::cliquee, this, &MainWindow::selectionnerSalle);
        if (s->getId() == idSelectionne)
            c->setSelectionnee(true);
        cartes.append(c);
    }

    if (idSelectionne >= 0 && !gestionnaire.rechercherParId(idSelectionne))
        idSelectionne = -1;

    lblVide->setVisible(cartes.isEmpty());

    reflowCartes(true);
    mettreAJourResume();
    mettreAJourPanneau();
}

void MainWindow::reflowCartes(bool forcer)
{
    if (!grilleCartes || !scrollCartes) return;

    int largeur = scrollCartes->viewport()->width();
    int cols = qBound(1, largeur / 300, 3);
    if (!forcer && cols == colonnes) return;
    colonnes = cols;

    for (SalleCard* c : cartes) grilleCartes->removeWidget(c);
    for (int i = 0; i < cartes.size(); ++i) {
        grilleCartes->addWidget(cartes[i], i / cols, i % cols);
        cartes[i]->show();
    }
    for (int c = 0; c < 3; ++c)
        grilleCartes->setColumnStretch(c, c < cols ? 1 : 0);
}

void MainWindow::mettreAJourResume()
{
    Statistiques st = gestionnaire.calculerStatistiques();

    pillDispo->setText(QString("%1 %2").arg(st.sallesLibres)
                           .arg(st.sallesLibres > 1 ? "disponibles" : "disponible"));
    pillOccupe->setText(QString("%1 %2").arg(st.sallesOccupees)
                            .arg(st.sallesOccupees > 1 ? "occupées" : "occupée"));
    pillHors->setText(QString("%1 hors service").arg(st.sallesHorsService));

    lblTaux->setText(QString("%1 %").arg(st.tauxOccupation, 0, 'f', 0));
    barTaux->setValue(int(st.tauxOccupation));
    lblTauxNote->setText(QString("%1 animaux pour %2 places au total")
                             .arg(st.occupationTotale).arg(st.capaciteTotale));
}

void MainWindow::mettreAJourPanneau()
{
    Salle* s = salleSelectionnee();
    bool a = (s != nullptr);

    if (!a) {
        panTitre->setText("Aucun local sélectionné");
        panPill->hide();
        panInfos->setText("Clique sur une carte pour voir ses détails et agir sur le local.");
        boxRemise->hide();
    } else {
        panTitre->setText(s->getNom());

        panPill->setText(SalleCard::libelleEtat(s->getEtat()));
        panPill->setProperty("etat", SalleCard::cleEtat(s->getEtat()));
        repolir(panPill);
        panPill->show();

        QString detail = SalleCard::detailSalle(s);
        QString infos = QString("%1 · ID %2\nCapacité : %3 · Occupation : %4\n%5")
                            .arg(s->getType()).arg(s->getId())
                            .arg(s->getCapacite()).arg(s->getOccupation())
                            .arg(detail);

        // Alerte : séjours dépassés dans les cages
        if (estSalleAvecCages(s)) {
            int depasses = gestionnaire.sejoursDepasses(s->getType() == "Pension");
            if (depasses > 0)
                infos += QString("\n⚠ %1 séjour(s) dépassé(s)").arg(depasses);
        }
        panInfos->setText(infos);

        boxRemise->setVisible(s->getEtat() == "Hors service");
    }

    bool storage = a && s->getType() == "Storage";
    bool avecCages = a && estSalleAvecCages(s);

    // Pension/Adoption : l'occupation vient des cages, les boutons ouvrent la gestion des cages
    btnAffecter->setText(avecCages ? "Occuper une cage" : "Affecter un animal");
    btnLiberer->setText(avecCages ? "Libérer une cage" : "Libérer");

    btnAffecter->setEnabled(a && !storage);
    btnLiberer->setEnabled(a && !storage);
    btnModifier->setEnabled(a && !storage);
    btnSupprimer->setEnabled(a && !storage);
    btnCages->setVisible(avecCages);
}

void MainWindow::selectionnerSalle(int idSalle)
{
    if (idSalle != idSelectionne) {
        chkSurfaces->setChecked(false);
        chkMateriel->setChecked(false);
        chkControle->setChecked(false);
    }
    idSelectionne = idSalle;

    for (SalleCard* c : cartes)
        c->setSelectionnee(c->idSalle() == idSalle);

    mettreAJourPanneau();
}

void MainWindow::validerRemiseEnService()
{
    Salle* s = salleSelectionnee();
    if (!s) return;

    QString nouvelEtat = (s->getCapacite() > 0 && s->getOccupation() >= s->getCapacite())
                             ? "Occupée" : "Libre";

    if (gestionnaire.changerEtat(s->getId(), nouvelEtat)) {
        chkSurfaces->setChecked(false);
        chkMateriel->setChecked(false);
        chkControle->setChecked(false);
        QMessageBox::information(this, "Succès", "Local remis en service ✅");
        rafraichirCartes();
        rafraichirMap();
    }
}

void MainWindow::rafraichirMap()
{
    if (mapWidget)
        mapWidget->setSalles(gestionnaire.getToutesLesSalles());
}

void MainWindow::rechercheChangee() { rafraichirCartes(); }

Salle* MainWindow::salleSelectionnee()
{
    if (idSelectionne < 0) return nullptr;
    return gestionnaire.rechercherParId(idSelectionne);
}

// ==================== Ajouter (avec validation) ====================
void MainWindow::ajouterSalle()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Nouvelle salle");
    QFormLayout* form = new QFormLayout(&dlg);

    // ID proposé : le prochain libre
    int idSuivant = 1;
    for (Salle* x : gestionnaire.getToutesLesSalles())
        idSuivant = qMax(idSuivant, x->getId() + 1);

    QSpinBox* idEdit = new QSpinBox();
    idEdit->setRange(1, 99999);
    idEdit->setValue(idSuivant);

    QLineEdit* nomEdit = new QLineEdit();
    nomEdit->setPlaceholderText("Ex : Consultation 6");

    QComboBox* typeBox = new QComboBox();
    typeBox->addItems({"Consultation", "Chirurgie", "Toilettage",
                       "Pension", "Adoption", "Storage"});

    QSpinBox* capEdit = new QSpinBox();
    capEdit->setRange(1, 200);
    capEdit->setValue(1);

    QLineEdit* extraEdit = new QLineEdit();
    extraEdit->setPlaceholderText("Spécialité / Type / Niveau...");

    // Pension et Adoption ont toujours 20 cages
    connect(typeBox, &QComboBox::currentTextChanged, &dlg, [capEdit](const QString& t) {
        const bool cages = (t == "Pension" || t == "Adoption");
        if (cages) capEdit->setValue(20);
        capEdit->setEnabled(!cages);
    });

    form->addRow("ID :", idEdit);
    form->addRow("Nom :", nomEdit);
    form->addRow("Type :", typeBox);
    form->addRow("Capacité :", capEdit);
    form->addRow("Info :", extraEdit);

    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(btns);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    // On rouvre le formulaire tant que les données sont invalides
    while (true) {
        if (dlg.exec() != QDialog::Accepted) return;

        if (nomEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg, "Erreur", "Le nom ne peut pas être vide ❌");
            continue;
        }
        if (gestionnaire.rechercherParId(idEdit->value())) {
            QMessageBox::warning(&dlg, "Erreur", "Cet ID existe déjà ❌");
            continue;
        }
        break;
    }

    int id = idEdit->value();
    QString nom = nomEdit->text().trimmed();
    QString type = typeBox->currentText();
    int cap = capEdit->value();
    QString extra = extraEdit->text().trimmed();

    Salle* s = nullptr;
    if (type == "Consultation") s = new SalleConsultation(id, nom, cap, extra);
    else if (type == "Chirurgie") s = new SalleChirurgie(id, nom, cap, extra);
    else if (type == "Toilettage") s = new SalleToilettage(id, nom, cap, extra);
    else if (type == "Pension") s = new SallePension(id, nom, cap, extra);
    else if (type == "Adoption") s = new SalleAdoption(id, nom, cap, extra);
    else if (type == "Storage") s = new SalleStorage(id, nom, cap, extra);

    if (s && gestionnaire.ajouter(s)) {
        QMessageBox::information(this, "Succès", "Salle ajoutée ✅");
        rafraichirCartes();
        rafraichirMap();
    } else {
        delete s;
        QMessageBox::warning(this, "Erreur", "Impossible d'ajouter la salle ❌");
    }
}

// ==================== Modifier (nom, capacité, info, état) ====================
void MainWindow::modifierSalle()
{
    Salle* s = salleSelectionnee();
    if (!s) { QMessageBox::warning(this, "Erreur", "Sélectionne une salle"); return; }
    if (s->getType() == "Storage") {
        QMessageBox::warning(this, "Info", "Storage est en lecture seule");
        return;
    }

    const bool cages = estSalleAvecCages(s);

    QDialog dlg(this);
    dlg.setWindowTitle("Modifier le local");
    QFormLayout* form = new QFormLayout(&dlg);

    QLineEdit* nomEdit = new QLineEdit(s->getNom());

    // La capacité ne peut pas descendre sous l'occupation actuelle.
    // Pension/Adoption : 20 cages fixes, capacité non modifiable.
    QSpinBox* capEdit = new QSpinBox();
    capEdit->setRange(qMax(1, s->getOccupation()), 200);
    capEdit->setValue(s->getCapacite());
    capEdit->setEnabled(!cages);

    QLineEdit* infoEdit = new QLineEdit(s->getInfo());

    // Pension/Adoption : "Occupée" découle des cages, donc non proposé
    QComboBox* etatBox = new QComboBox();
    if (cages) etatBox->addItems({"Libre", "Hors service"});
    else etatBox->addItems({"Libre", "Occupée", "Hors service"});
    int idx = etatBox->findText(s->getEtat());
    etatBox->setCurrentIndex(idx >= 0 ? idx : 0);

    form->addRow("Nom :", nomEdit);
    form->addRow("Capacité :", capEdit);
    form->addRow("Info :", infoEdit);
    form->addRow("État :", etatBox);

    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form->addRow(btns);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    while (true) {
        if (dlg.exec() != QDialog::Accepted) return;
        if (nomEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dlg, "Erreur", "Le nom ne peut pas être vide ❌");
            continue;
        }
        break;
    }

    s->setNom(nomEdit->text().trimmed());
    s->setInfo(infoEdit->text().trimmed());
    s->setCapacite(capEdit->value());

    const QString nouvelEtat = etatBox->currentText();
    s->setEtat(nouvelEtat);

    if (cages) {
        // L'occupation et l'état Libre/Occupée sont recalculés depuis les cages
        gestionnaire.synchroniserCages();
    } else if (nouvelEtat == "Occupée" && s->getOccupation() < s->getCapacite()) {
        // "Occupée" = salle pleine : l'occupation suit l'état choisi
        s->setOccupation(s->getCapacite());
    } else if (nouvelEtat == "Libre" && s->getOccupation() >= s->getCapacite()) {
        // "Libre" avec une salle pleine serait incohérent : on la vide
        s->setOccupation(0);
    }

    gestionnaire.sauvegarderAuto();
    rafraichirCartes();
    rafraichirMap();
}

// ==================== Supprimer ====================
void MainWindow::supprimerSalle()
{
    Salle* s = salleSelectionnee();
    if (!s) { QMessageBox::warning(this, "Erreur", "Sélectionne une salle"); return; }
    if (s->getType() == "Storage") {
        QMessageBox::warning(this, "Info", "Storage ne peut pas être supprimé");
        return;
    }
    if (QMessageBox::question(this, "Confirmer", "Supprimer cette salle ?") == QMessageBox::Yes) {
        gestionnaire.supprimer(s->getId());
        idSelectionne = -1;
        rafraichirCartes();
        rafraichirMap();
    }
}

// ==================== Affecter animal ====================
void MainWindow::affecterAnimal()
{
    Salle* s = salleSelectionnee();
    if (!s) { QMessageBox::warning(this, "Erreur", "Sélectionne une salle"); return; }
    if (s->getType() == "Storage") {
        QMessageBox::warning(this, "Info", "Storage n'accueille pas d'animaux");
        return;
    }
    // Pension/Adoption : on passe par les cages pour garder l'occupation cohérente
    if (estSalleAvecCages(s)) {
        ouvrirGestionCages();
        return;
    }
    if (gestionnaire.affecterAnimal(s->getId())) {
        QMessageBox::information(this, "Succès",
                                 QString("Animal affecté à %1 ✅\nOccupation : %2/%3")
                                     .arg(s->getNom()).arg(s->getOccupation()).arg(s->getCapacite()));
        rafraichirCartes();
        rafraichirMap();
    } else {
        QMessageBox::warning(this, "Erreur", "Salle pleine ou indisponible ❌");
    }
}

// ==================== Libérer salle ====================
void MainWindow::libererSalle()
{
    Salle* s = salleSelectionnee();
    if (!s) { QMessageBox::warning(this, "Erreur", "Sélectionne une salle"); return; }
    if (s->getType() == "Storage") {
        QMessageBox::warning(this, "Info", "Storage ne peut pas être libéré");
        return;
    }
    if (estSalleAvecCages(s)) {
        ouvrirGestionCages();
        return;
    }

    if (QMessageBox::question(this, "Libérer la salle",
                              QString("Libérer 1 animal de la salle %1 ?\n\nOccupation actuelle : %2/%3")
                                  .arg(s->getNom()).arg(s->getOccupation()).arg(s->getCapacite()))
        == QMessageBox::Yes) {

        if (gestionnaire.libererSalle(s->getId())) {
            QMessageBox::information(this, "Succès",
                                     QString("✅ Salle %1 libérée !\nOccupation : %2/%3\nÉtat : %4")
                                         .arg(s->getNom()).arg(s->getOccupation())
                                         .arg(s->getCapacite()).arg(s->getEtat()));
            rafraichirCartes();
            rafraichirMap();
        }
    }
}

// ==================== Gestion des cages (Occuper / Libérer / séjours) ====================
void MainWindow::ouvrirGestionCages()
{
    QDialog dlg(this);
    dlg.setWindowTitle("🧺 Gestion des cages");
    dlg.resize(940, 600);
    QVBoxLayout* layout = new QVBoxLayout(&dlg);

    QTabWidget* tabs = new QTabWidget();

    // Construit un onglet complet (tableau + boutons) pour Pension ou Adoption
    auto creerOnglet = [this](bool pension) -> QWidget* {
        QWidget* page = new QWidget();
        QVBoxLayout* lv = new QVBoxLayout(page);

        QLabel* resume = new QLabel();
        resume->setObjectName("pageSub");
        lv->addWidget(resume);

        QTableWidget* table = new QTableWidget(0, 8);
        table->setHorizontalHeaderLabels({"N°", "État", "Animal", "Type",
                                          "Arrivée", "Séjour", "Sortie prévue", "Statut"});
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->verticalHeader()->hide();
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        lv->addWidget(table, 1);

        QHBoxLayout* lb = new QHBoxLayout();
        QPushButton* bOcc = new QPushButton("Occuper la cage sélectionnée");
        bOcc->setObjectName("primaryBtn");
        QPushButton* bLib = new QPushButton("Libérer la cage sélectionnée");
        bLib->setObjectName("outlineBtn");
        bOcc->setCursor(Qt::PointingHandCursor);
        bLib->setCursor(Qt::PointingHandCursor);
        lb->addStretch(1);
        lb->addWidget(bLib);
        lb->addWidget(bOcc);
        lv->addLayout(lb);

        auto cagesDe = [this, pension]() -> QVector<Cage>& {
            return pension ? gestionnaire.getCagesPension() : gestionnaire.getCagesAdoption();
        };

        // Remplit le tableau (couleurs : libre / occupée / séjour dépassé)
        auto remplir = std::make_shared<std::function<void()>>();
        *remplir = [table, resume, cagesDe]() {
            const QVector<Cage>& v = cagesDe();
            table->setRowCount(0);
            int occupees = 0, depasses = 0;
            for (int i = 0; i < v.size(); ++i) {
                const Cage& c = v[i];
                table->insertRow(i);

                QColor fond("#E9F5EC");                       // libre
                if (c.estOccupee()) {
                    ++occupees;
                    if (c.sejourDepasse()) { ++depasses; fond = QColor("#FCE9CF"); }
                    else fond = QColor("#E4E8F6");
                }

                const QStringList cellules = {
                    QString::number(c.getNumero()),
                    c.estOccupee() ? "🔴 Occupée" : "🟢 Libre",
                    c.getNomAnimal(),
                    c.getTypeAnimal(),
                    c.estOccupee() ? c.getDateArrivee().toString("dd/MM/yyyy") : "",
                    c.estOccupee() ? QString("%1 j").arg(c.getNombreJours()) : "",
                    c.estOccupee() ? c.dateSortie().toString("dd/MM/yyyy") : "",
                    libelleSejour(c)
                };
                for (int col = 0; col < cellules.size(); ++col) {
                    QTableWidgetItem* it = new QTableWidgetItem(cellules[col]);
                    it->setBackground(fond);
                    it->setForeground(QColor("#293F45"));
                    table->setItem(i, col, it);
                }
            }
            resume->setText(QString("%1/%2 cages occupées · %3 séjour(s) dépassé(s)")
                                .arg(occupees).arg(v.size()).arg(depasses));
        };
        (*remplir)();

        // ----- Occuper -----
        connect(bOcc, &QPushButton::clicked, page, [this, pension, table, remplir, cagesDe]() {
            int row = table->currentRow();
            if (row < 0) {
                QMessageBox::warning(table, "Erreur", "Sélectionne une cage dans le tableau");
                return;
            }
            const int numero = row + 1;
            if (cagesDe()[row].estOccupee()) {
                QMessageBox::information(table, "Info", "Cette cage est déjà occupée");
                return;
            }

            QDialog d(table->window());
            d.setWindowTitle(QString("Occuper la cage %1").arg(numero));
            QFormLayout* f = new QFormLayout(&d);

            QLineEdit* nomEdit = new QLineEdit();
            nomEdit->setPlaceholderText("Ex : Max");
            QComboBox* typeBox = new QComboBox();
            typeBox->setEditable(true);
            typeBox->addItems({"Chien", "Chat", "Oiseau", "Lapin", "Rongeur", "Reptile"});
            QDateEdit* dateEdit = new QDateEdit(QDate::currentDate());
            dateEdit->setCalendarPopup(true);
            dateEdit->setDisplayFormat("dd/MM/yyyy");
            QSpinBox* joursEdit = new QSpinBox();
            joursEdit->setRange(1, 365);
            joursEdit->setValue(7);
            joursEdit->setSuffix(" jours");
            QLabel* sortie = new QLabel();

            auto majSortie = [dateEdit, joursEdit, sortie]() {
                sortie->setText(dateEdit->date().addDays(joursEdit->value()).toString("dd/MM/yyyy"));
            };
            connect(dateEdit, &QDateEdit::dateChanged, &d, majSortie);
            connect(joursEdit, QOverload<int>::of(&QSpinBox::valueChanged), &d, majSortie);
            majSortie();

            f->addRow("Nom de l'animal :", nomEdit);
            f->addRow("Type :", typeBox);
            f->addRow("Date d'arrivée :", dateEdit);
            f->addRow("Durée du séjour :", joursEdit);
            f->addRow("Sortie prévue :", sortie);

            QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            f->addRow(bb);
            connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
            connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);

            while (true) {
                if (d.exec() != QDialog::Accepted) return;
                if (nomEdit->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&d, "Erreur", "Le nom de l'animal est obligatoire ❌");
                    continue;
                }
                if (typeBox->currentText().trimmed().isEmpty()) {
                    QMessageBox::warning(&d, "Erreur", "Le type d'animal est obligatoire ❌");
                    continue;
                }
                break;
            }

            const QString nom = nomEdit->text().trimmed();
            const QString type = typeBox->currentText().trimmed();
            const bool ok = pension
                                ? gestionnaire.occuperCagePension(numero, nom, type, dateEdit->date(), joursEdit->value())
                                : gestionnaire.occuperCageAdoption(numero, nom, type, dateEdit->date(), joursEdit->value());
            if (!ok) {
                QMessageBox::warning(table, "Erreur", "Impossible d'occuper cette cage ❌");
                return;
            }
            (*remplir)();
            table->selectRow(row);
            rafraichirCartes();
            rafraichirMap();
        });

        // ----- Libérer -----
        connect(bLib, &QPushButton::clicked, page, [this, pension, table, remplir, cagesDe]() {
            int row = table->currentRow();
            if (row < 0) {
                QMessageBox::warning(table, "Erreur", "Sélectionne une cage dans le tableau");
                return;
            }
            const int numero = row + 1;
            const Cage& c = cagesDe()[row];
            if (!c.estOccupee()) {
                QMessageBox::information(table, "Info", "Cette cage est déjà libre");
                return;
            }
            if (QMessageBox::question(table, "Libérer la cage",
                                      QString("Libérer la cage %1 (%2) ?")
                                          .arg(numero).arg(c.getNomAnimal())) != QMessageBox::Yes)
                return;

            if (pension) gestionnaire.libererCagePension(numero);
            else gestionnaire.libererCageAdoption(numero);

            (*remplir)();
            table->selectRow(row);
            rafraichirCartes();
            rafraichirMap();
        });

        return page;
    };

    tabs->addTab(creerOnglet(true), "🐕 Pension");
    tabs->addTab(creerOnglet(false), "🐾 Adoption");

    // Ouvre directement l'onglet de la salle sélectionnée
    Salle* sel = salleSelectionnee();
    if (sel && sel->getType() == "Adoption") tabs->setCurrentIndex(1);

    layout->addWidget(tabs);

    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(btns);

    dlg.exec();

    // Le panneau peut afficher l'alerte "séjours dépassés"
    mettreAJourPanneau();
}

// ==================== Stats ====================
void MainWindow::ouvrirStats()
{
    Statistiques st = gestionnaire.calculerStatistiques();
    StatsDialog dlg(st, this);
    dlg.exec();
}

// ==================== Export ====================
void MainWindow::exporterCSV()
{
    QString chemin = QFileDialog::getSaveFileName(this, "Exporter CSV",
                                                  "salles.csv", "CSV (*.csv)");
    if (chemin.isEmpty()) return;
    if (gestionnaire.sauvegarderCSV(chemin))
        QMessageBox::information(this, "Succès", "Export CSV réussi ✅");
}

void MainWindow::exporterPDF()
{
    QString chemin = QFileDialog::getSaveFileName(this, "Exporter PDF",
                                                  "salles.pdf", "PDF (*.pdf)");
    if (chemin.isEmpty()) return;
    if (gestionnaire.exporterPDF(chemin))
        QMessageBox::information(this, "Succès", "Export PDF réussi ✅");
}

void MainWindow::sauvegarderFichier()
{
    if (gestionnaire.sauvegarderAuto())
        QMessageBox::information(this, "Succès", "Sauvegarde réussie ✅");
    else
        QMessageBox::warning(this, "Erreur", "Échec sauvegarde");
}

void MainWindow::chargerFichier()
{
    QString chemin = QFileDialog::getOpenFileName(this, "Charger CSV",
                                                  "", "CSV (*.csv)");
    if (chemin.isEmpty()) return;
    if (gestionnaire.chargerCSV(chemin)) {
        rafraichirCartes();
        rafraichirMap();
        QMessageBox::information(this, "Succès", "Chargement réussi ✅");
    }
}

// ==================== Clic sur salle dans le plan ====================
void MainWindow::surClicSalleMap(int idSalle)
{
    Salle* s = gestionnaire.rechercherParId(idSalle);
    if (!s) return;
    QMessageBox::information(this, "Détails de la salle",
                             QString("%1\n\nID : %2\nCapacité : %3\nOccupation : %4\nÉtat : %5")
                                 .arg(s->afficherDetails())
                                 .arg(s->getId()).arg(s->getCapacite())
                                 .arg(s->getOccupation()).arg(s->getEtat()));
}
