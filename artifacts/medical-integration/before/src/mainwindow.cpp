#include "locaux/pagelocaux.h"
#include "locaux/localrepository.h"
#include <QFileInfo>
#include "mainwindow.h"
#include "ui/design.h"
#include "database/databasemanager.h"
#include "widgets/animatedlogo.h"
#include "ui/motion.h"
#include "stock/stockwidget.h"
#include "employees/gestionemployes.h"
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScreen>
#include <QApplication>
#include <QTimer>
#include <QShortcut>
#include <QLocale>
#include <QPainter>
#include <QPainterPath>
#include <QStandardPaths>
#include <QDir>

namespace {
class SidebarSurface : public QFrame {
public:
    SidebarSurface():m_artworkSpace(new QWidget(this)) {
        setObjectName("sidebar");
        m_artworkSpace->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    QWidget *artworkSpace() const { return m_artworkSpace; }
protected:
    void paintEvent(QPaintEvent *event) override {
        QFrame::paintEvent(event);
        QPainter painter(this); Design::paintSidebarArt(painter,m_artworkSpace->geometry());
    }
private:
    QWidget *m_artworkSpace;
};

// Every navigation frame starts with a solid surface. Applying an opacity
// effect to the entire stack left old page pixels in the Windows backing store.
class PageSurface : public QStackedWidget {
public:
    PageSurface() {
        setObjectName("pageStack");
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAutoFillBackground(true);
        QPalette colors=palette(); colors.setColor(QPalette::Window,QColor("#F3F7F8")); setPalette(colors);
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        { QPainter painter(this); painter.fillRect(rect(),QColor("#F3F7F8")); }
        QStackedWidget::paintEvent(event);
    }
};
}

MainWindow::MainWindow(bool demo,QWidget *parent):QMainWindow(parent),m_demo(demo) {
    setWindowTitle(demo?"ANIWORLD — Démonstration":"ANIWORLD — Gestion du centre");
    setMinimumSize(1100,700);
    const QSize available=QApplication::primaryScreen()->availableGeometry().size();
    resize(qMin(1400,available.width()-40),qMin(900,available.height()-60));
    auto *central=new QWidget; setCentralWidget(central);
    auto *root=new QHBoxLayout(central); root->setContentsMargins(0,0,0,0); root->setSpacing(0);
    auto *side=new SidebarSurface; side->setFixedWidth(224);
    auto *sidebar=new QVBoxLayout(side); sidebar->setContentsMargins(12,12,12,14); sidebar->setSpacing(3);
    auto *brandRow=new QHBoxLayout; brandRow->setSpacing(7);
    auto *brandLogo=new AnimatedLogo; brandLogo->setFixedSize(42,42); brandRow->addWidget(brandLogo);
    connect(brandLogo,&QAbstractButton::clicked,this,[this] { navigate(Accounts::allows(m_role,0)?0:Accounts::homePage(m_role)); });
    auto *brandText=new QVBoxLayout; brandText->setSpacing(2); brandText->addStretch();
    auto *name=Design::label("ANIWORLD"); name->setStyleSheet("font-size:17px;font-weight:700;color:#263E46;letter-spacing:1px;"); brandText->addWidget(name);
    auto *tagline=Design::label("THE OFFICIAL APPLICATION"); tagline->setStyleSheet("font-size:7px;letter-spacing:0.8px;color:#87989E;"); brandText->addWidget(tagline);
    brandText->addStretch();
    brandRow->addLayout(brandText); sidebar->addLayout(brandRow); sidebar->addSpacing(12);
    auto *workspace=Design::card("workspace"); auto *wl=new QHBoxLayout(workspace); wl->setContentsMargins(12,11,10,11);
    auto *paw=Design::label(""); paw->setPixmap(Design::icon("paw").pixmap(23,23)); paw->setFixedSize(30,30); wl->addWidget(paw);
    auto *wt=new QVBoxLayout; wt->setSpacing(3); wt->addWidget(Design::label("Centre de soins",nullptr)); wt->addWidget(Design::label("Espace de gestion","muted")); wl->addLayout(wt); sidebar->addWidget(workspace);
    sidebar->addSpacing(16); sidebar->addWidget(Design::label("  NAVIGATION","eyebrow")); sidebar->addSpacing(5);
    const QStringList labels={"Tableau de bord","Services","Stock","Employés","Locaux"}; const QStringList icons={"grid","services","stock","users","home"};
    const QStringList names={"navDashboard","navServices","navStock","navEmployees","navLocaux"};
    for(int i=0;i<labels.size();++i) {
        auto *button=Design::button("  "+labels[i],icons[i],"nav"); button->setCheckable(true); button->setObjectName(names[i]);
        m_navigation.append(button); sidebar->addWidget(button); connect(button,&QPushButton::clicked,this,[this,i]{navigate(i);});
    }
    sidebar->addSpacing(16); sidebar->addWidget(Design::label("  PROCHAINS MODULES","eyebrow")); sidebar->addSpacing(5);
    const QStringList future={"Animaux & dossiers"};
    const QStringList fi={"shield"};
    for(int i=0;i<future.size();++i) {
        auto *button=Design::button("  "+future[i],fi[i],"nav"); button->setEnabled(false);
        button->setToolTip(i==1?"Animaux et dossiers médicaux réunis dans un futur module.":"Intégration prévue ultérieurement."); sidebar->addWidget(button);
    }
    sidebar->addWidget(side->artworkSpace(),1);
    m_sessionUser=Design::label("","muted"); m_sessionUser->setObjectName("signedInUser");
    m_sessionUser->setStyleSheet("color:#6F858B;font-size:11px;padding:5px 3px;");
    m_sessionUser->hide(); sidebar->addWidget(m_sessionUser);
    m_passwordButton=Design::button("Mot de passe","shield"); m_passwordButton->setObjectName("changePassword");
    m_passwordButton->hide(); sidebar->addWidget(m_passwordButton);
    connect(m_passwordButton,&QPushButton::clicked,this,[this] {
        if(m_role==Accounts::Role::None) return;
        QDialog dialog(this); dialog.setObjectName("passwordDialog"); dialog.setWindowTitle("Changer mon mot de passe"); dialog.setMinimumWidth(430);
        auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(24,24,24,24); layout->setSpacing(14);
        layout->addWidget(Design::label("Changer mon mot de passe","section"));
        auto *form=new QFormLayout; auto *current=new QLineEdit; auto *replacement=new QLineEdit; auto *confirm=new QLineEdit;
        current->setObjectName("currentPassword"); replacement->setObjectName("newPassword"); confirm->setObjectName("confirmPassword");
        for(auto *field:{current,replacement,confirm}) { field->setEchoMode(QLineEdit::Password); field->setMaxLength(128); }
        form->addRow("Mot de passe actuel",current); form->addRow("Nouveau mot de passe",replacement); form->addRow("Confirmation",confirm); layout->addLayout(form);
        auto *message=Design::label("8 caractères minimum.","muted"); message->setWordWrap(true); layout->addWidget(message);
        auto *buttons=new QHBoxLayout; buttons->addStretch(); auto *cancel=Design::button("Fermer"); auto *save=Design::button("Enregistrer","check","primary"); save->setObjectName("savePassword");
        buttons->addWidget(cancel); buttons->addWidget(save); layout->addLayout(buttons);
        connect(cancel,&QPushButton::clicked,&dialog,&QDialog::reject);
        connect(save,&QPushButton::clicked,&dialog,[&] {
            if(replacement->text()!=confirm->text()) { message->setText("Les mots de passe ne correspondent pas."); return; }
            QString error;
            if(!Accounts::changePassword(m_username,current->text(),replacement->text(),&error)) { message->setText(error); return; }
            current->clear(); replacement->clear(); confirm->clear(); message->setText("Votre mot de passe a été enregistré.");
        });
        dialog.exec();
    });
    auto *mission=Design::label("Une attention à chaque patte."); mission->setStyleSheet("font-size:11px;font-weight:600;color:#53717B;padding:12px 3px;"); sidebar->addWidget(mission);
    sidebar->addWidget(Design::label("ANIWORLD · Centre de soins","muted")); root->addWidget(side);
    m_navigationMarker=new QFrame(side); m_navigationMarker->setObjectName("navigationMarker");
    m_navigationMarker->setFixedSize(3,22); m_navigationMarker->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_navigationMarker->setStyleSheet("background:#377E8E;border-radius:1px;"); m_navigationMarker->hide();
    m_markerMotion=new QPropertyAnimation(m_navigationMarker,"pos",this);
    m_markerMotion->setDuration(220); m_markerMotion->setEasingCurve(QEasingCurve::OutCubic);
    auto *right=new QVBoxLayout; right->setContentsMargins(0,0,0,0); right->setSpacing(0); root->addLayout(right,1);
    auto *top= new QFrame; top->setObjectName("topbar"); top->setFixedHeight(52);
    auto *topLayout=new QHBoxLayout(top); topLayout->setContentsMargins(28,0,28,0);
    m_breadcrumb=Design::label("Espace de gestion   /   Tableau de bord","muted"); topLayout->addWidget(m_breadcrumb); topLayout->addStretch();
    m_headerDate=Design::label("","muted");
    auto updateDate=[this]{ m_headerDate->setText(QLocale().toString(QDate::currentDate(),"dddd d MMMM yyyy")); };
    updateDate(); auto *timer=new QTimer(this); connect(timer,&QTimer::timeout,this,updateDate); timer->start(60000);
    topLayout->addWidget(m_headerDate); topLayout->addSpacing(8);
    auto *mode=Design::label(demo?"●  Démo · non enregistré":"●  Sauvegarde locale");
    m_storageBadge=mode; mode->setObjectName("storageBadge");
    mode->setStyleSheet("background:#E6F0F3;color:#3B7481;border-radius:7px;padding:8px 11px;font-size:11px;");
    mode->setToolTip(demo?"Données fictives. Les modifications disparaissent à la fermeture.":DatabaseManager::instance().storagePath());
    topLayout->addWidget(mode,0,Qt::AlignVCenter);
    auto *motionToggle=Design::button("Animations","motion","toggle"); motionToggle->setObjectName("motionToggle");
    motionToggle->setCheckable(true); motionToggle->setChecked(Motion::instance().enabled());
    motionToggle->setAccessibleName("Activer ou désactiver les animations");
    motionToggle->setToolTip("Activer ou désactiver les animations. Votre préférence est conservée.");
    connect(motionToggle,&QPushButton::clicked,this,[](bool checked) { Motion::instance().setEnabled(checked,true); });
    topLayout->addWidget(motionToggle,0,Qt::AlignVCenter);
    m_logoutButton=Design::button("Se déconnecter","logout");
    m_logoutButton->setObjectName("logoutButton");
    m_logoutButton->setAccessibleName("Se déconnecter et revenir à la connexion");
    m_logoutButton->setToolTip("Revenir à la connexion. Les données enregistrées sont conservées.");
    m_logoutButton->setStyleSheet(
        "QPushButton { color:#2B6973;background:#EAF3F3;border:1px solid #B7D2D4;padding:10px 12px; }"
        "QPushButton:hover { background:#D7EBEB;border-color:#397E89; }"
        "QPushButton:pressed { background:#C5E0E1; }"
        "QPushButton:focus { border:1px solid #397E89; }");
    m_logoutButton->hide(); topLayout->addWidget(m_logoutButton,0,Qt::AlignVCenter);
    connect(m_logoutButton,&QPushButton::clicked,this,&MainWindow::logoutRequested);
    m_headerDate->setVisible(width()>=1280);
    right->addWidget(top);
    m_stack=new PageSurface; right->addWidget(m_stack,1);
    connect(&Motion::instance(),&Motion::changed,this,[this,motionToggle](bool enabled) {
        motionToggle->setChecked(enabled);
        if(!enabled) { m_markerMotion->stop(); positionNavigationMarker(false); }
    });
    m_dashboard=new DashboardWidget; auto *scroll=new QScrollArea; scroll->setWidgetResizable(true); scroll->setWidget(m_dashboard); scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_services=new ServiceWidget; m_services->setProperty("demoMode",demo); m_stack->addWidget(scroll); m_stack->addWidget(m_services);
    m_stock=new StockWidget; m_stack->addWidget(m_stock);
    m_employees=new GestionEmployes; m_employees->setObjectName("employeesPage"); m_stack->addWidget(m_employees);
    const QString dbPath=DatabaseManager::instance().storagePath();
    const QString locauxPath=(demo || dbPath.isEmpty() || dbPath==":memory:")?":memory:":QFileInfo(dbPath).absolutePath()+"/locaux_v2.json";
    auto *locauxRepo=new LocalRepository(this,locauxPath);
    m_locaux=new PageLocaux(locauxRepo); m_locaux->setProperty("storagePath",locauxPath); m_stack->addWidget(m_locaux);
    QPalette pageColors=m_stack->palette(); pageColors.setColor(QPalette::Base,QColor("#F3F7F8"));
    for(auto *page:{static_cast<QWidget *>(m_services),static_cast<QWidget *>(m_dashboard),static_cast<QWidget *>(m_stock),scroll->viewport()}) {
        page->setPalette(pageColors); page->setAutoFillBackground(true);
    }
    connect(m_dashboard,&DashboardWidget::createRequested,this,[this]{if(Accounts::allows(m_role,1)) {navigate(1);m_services->onAdd();}});
    connect(m_dashboard,&DashboardWidget::servicesRequested,this,[this]{navigate(1);});
    connect(m_dashboard,&DashboardWidget::serviceRequested,this,[this](int id){if(Accounts::allows(m_role,1)) {navigate(1);m_services->viewService(id);}});
    connect(m_services,&ServiceWidget::servicesChanged,m_dashboard,&DashboardWidget::refresh);
    auto *addShortcut=new QShortcut(QKeySequence("Ctrl+N"),this); connect(addShortcut,&QShortcut::activated,this,[this]{
        if(!Accounts::allows(m_role,m_stack->currentIndex())) return;
        if(m_stack->currentIndex()==2) m_stock->openMovementDialog();
        else if(m_stack->currentIndex()==3) m_employees->ajouterEmploye();
        else if(m_stack->currentIndex()==4) m_locaux->addLocal();
        else { navigate(1);m_services->onAdd(); }
    });
    auto *searchShortcut=new QShortcut(QKeySequence("Ctrl+F"),this); connect(searchShortcut,&QShortcut::activated,this,[this]{
        if(!Accounts::allows(m_role,m_stack->currentIndex())) return;
        if(m_stack->currentIndex()==2) m_stock->focusSearch();
        else if(m_stack->currentIndex()==3) m_employees->focusSearch();
        else if(m_stack->currentIndex()==4) m_locaux->focusSearch();
        else { navigate(1);m_services->focusSearch(); }
    });
    navigate(1);
}
void MainWindow::setSignedInUser(const QString &username) {
    const bool signedIn=!username.isEmpty();
    m_sessionUser->setText(m_sessionUser->fontMetrics().elidedText("Connecté · "+username,Qt::ElideRight,202));
    m_sessionUser->setToolTip(username);
    m_sessionUser->setVisible(signedIn);
    m_logoutButton->setVisible(signedIn);
}
void MainWindow::setSession(const QString &username,Accounts::Role role) {
    m_username=username.trimmed(); m_role=role;
    setSignedInUser(m_username);
    m_sessionUser->setToolTip(m_username+" · "+Accounts::label(role));
    m_passwordButton->setVisible(role!=Accounts::Role::None);
    for(int i=0;i<m_navigation.size();++i) {
        const bool allowed=Accounts::allows(role,i);
        m_navigation[i]->setVisible(allowed); m_navigation[i]->setEnabled(allowed);
        m_stack->widget(i)->setEnabled(allowed);
    }
    if(role!=Accounts::Role::None) navigate(Accounts::homePage(role));
    else m_navigationMarker->hide();
}
void MainWindow::navigate(int page) {
    if(page<0 || page>=m_stack->count() || !Accounts::allows(m_role,page)) return;
    const bool changed=m_stack->currentIndex()!=page;
    m_stack->setCurrentIndex(page);
    for(int i=0;i<m_navigation.size();++i) m_navigation[i]->setChecked(i==page);
    const QStringList pages={"Tableau de bord","Services","Stock","Employés","Locaux"};
    m_breadcrumb->setText("Gestion / "+pages.value(page));
    m_storageBadge->setText(page==2?"●  Stock · démo":(m_demo?"●  Démo · non enregistré":"●  Sauvegarde locale"));
    m_storageBadge->setToolTip(page==2?"Interface Stock intégrée : articles fictifs, mouvements conservés uniquement pendant cette session.":
        (m_demo?"Données fictives. Les modifications disparaissent à la fermeture.":DatabaseManager::instance().storagePath()));
    if(page==0) m_dashboard->refresh();
    else if(page==1) m_services->refresh();
    else if(page==2) m_stock->refresh();
    else if(page==3) m_employees->actualiserStats();
    else if(page==4) { m_locaux->refreshPage(); m_storageBadge->setToolTip(m_locaux->property("storagePath").toString()); }
    positionNavigationMarker(changed);
    if(changed && isVisible() && Motion::instance().enabled()) {
        Motion::reveal(m_stack,QColor("#F3F7F8"),200);
    }
}
void MainWindow::positionNavigationMarker(bool animated) {
    QTimer::singleShot(0,this,[this,animated] {
        if(m_role==Accounts::Role::None) { m_navigationMarker->hide(); return; }
        auto *button=m_navigation.value(m_stack->currentIndex()); if(!button) return;
        const QPoint destination(button->x()+2,button->y()+(button->height()-m_navigationMarker->height())/2);
        m_markerMotion->stop();
        if(animated && isVisible() && m_navigationMarker->isVisible() && Motion::instance().enabled()) {
            m_markerMotion->setStartValue(m_navigationMarker->pos()); m_markerMotion->setEndValue(destination); m_markerMotion->start();
        } else m_navigationMarker->move(destination);
        m_navigationMarker->show(); m_navigationMarker->raise();
    });
}
void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event); positionNavigationMarker(false);
    if(m_headerDate) m_headerDate->setVisible(width()>=1280);
}
