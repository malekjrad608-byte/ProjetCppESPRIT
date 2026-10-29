#include "mainwindow.h"
#include "confirmdialog.h"
#include "connection.h"
#include "hoverglow.h"
#include "icons.h"
#include "localrepository.h"
#include "pagelocaux.h"
#include "pawburst.h"
#include "roomcard.h"
#include "spinlogo.h"
#include "theme.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDate>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("ANIWORLD — Gestion du centre"));
    setMinimumSize(1100, 680);

    m_repo = new LocalRepository(this);

    auto *central = new QWidget;
    central->setObjectName("central");
    setCentralWidget(central);

    auto *h = new QHBoxLayout(central);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);
    h->addWidget(buildSidebar());

    auto *right = new QWidget;
    auto *rv = new QVBoxLayout(right);
    rv->setContentsMargins(0, 0, 0, 0);
    rv->setSpacing(0);
    rv->addWidget(buildHeader());
    rv->addWidget(new PageLocaux(m_repo), 1);
    rv->addWidget(buildFooter());
    h->addWidget(right, 1);

    // calque des pattes (animation au clic)
    m_paws = new PawBurst(central);
    m_paws->setGeometry(central->rect());
    m_paws->raise();

    qApp->installEventFilter(this);
}

QWidget *MainWindow::buildSidebar()
{
    auto *side = new QFrame;
    side->setObjectName("sidebar");
    side->setFixedWidth(238);
    auto *v = new QVBoxLayout(side);
    v->setContentsMargins(16, 18, 16, 14);
    v->setSpacing(4);

    // logo + nom
    auto *brand = new QHBoxLayout;
    brand->setSpacing(10);
    brand->addWidget(new SpinLogo(QPixmap(":/images/logo.png"), 50));
    auto *names = new QVBoxLayout;
    names->setSpacing(0);
    auto *title = new QLabel(QStringLiteral("ANIWORLD"));
    title->setObjectName("brandName");
    QFont tf = title->font();
    tf.setLetterSpacing(QFont::AbsoluteSpacing, 2.2);
    title->setFont(tf);
    auto *tag = new QLabel(QStringLiteral("THE OFFICIAL APPLICATION"));
    tag->setObjectName("brandTag");
    QFont gf = tag->font();
    gf.setLetterSpacing(QFont::AbsoluteSpacing, 1.2);
    tag->setFont(gf);
    names->addWidget(title);
    names->addWidget(tag);
    brand->addLayout(names);
    brand->addStretch();
    v->addLayout(brand);
    v->addSpacing(14);

    // carte de contexte
    auto *ctx = new QFrame;
    ctx->setObjectName("ctx");
    auto *cl = new QHBoxLayout(ctx);
    cl->setContentsMargins(14, 12, 14, 12);
    cl->setSpacing(10);
    auto *pawIcon = new QLabel;
    pawIcon->setPixmap(Icons::paw(QColor(0x16, 0x46, 0x50), 22));
    auto *ct = new QVBoxLayout;
    ct->setSpacing(0);
    auto *c1 = new QLabel(QStringLiteral("Centre de soins"));
    c1->setObjectName("ctxTitle");
    auto *c2 = new QLabel(QStringLiteral("Espace de gestion"));
    c2->setObjectName("ctxSub");
    ct->addWidget(c1);
    ct->addWidget(c2);
    cl->addWidget(pawIcon);
    cl->addLayout(ct);
    cl->addStretch();
    v->addWidget(ctx);
    v->addSpacing(16);

    auto *nav = new QLabel(QStringLiteral("NAVIGATION"));
    nav->setObjectName("kicker");
    QFont nf = nav->font();
    nf.setLetterSpacing(QFont::AbsoluteSpacing, 1.6);
    nav->setFont(nf);
    v->addWidget(nav);

    auto addNav = [&](const QString &text, const QString &icon, bool active) {
        auto *b = new QPushButton(text);
        b->setProperty("nav", true);
        b->setCursor(Qt::PointingHandCursor);
        b->setIconSize(QSize(18, 18));
        if (active) {
            b->setCheckable(true);
            b->setChecked(true);
            b->setIcon(Icons::icon(icon, Theme::TealDark, 18));
        } else {
            b->setIcon(Icons::icon(icon, QColor(0x4a, 0x67, 0x70), 18));
            connect(b, &QPushButton::clicked, this, [this, text] {
                QMessageBox::information(this, QStringLiteral("ANIWORLD"),
                    QStringLiteral("« %1 » est le module de ton équipe.\n"
                                   "Il sera relié ici lors de l'intégration.").arg(text));
            });
        }
        v->addWidget(b);
    };
    addNav(QStringLiteral("Tableau de bord"), "rooms", false);
    addNav(QStringLiteral("Services"), "services", false);
    addNav(QStringLiteral("Employés"), "people", false);
    addNav(QStringLiteral("Stock"), "stock", false);
    addNav(QStringLiteral("Locaux"), "rooms", true);
    addNav(QStringLiteral("Dossiers médicaux"), "records", false);

    v->addStretch();
    auto *who = new QLabel(QStringLiteral("Connecté · admin"));
    who->setObjectName("foot");
    auto *slogan = new QLabel(QStringLiteral("Une attention à chaque patte."));
    slogan->setObjectName("slogan");
    auto *centre = new QLabel(QStringLiteral("ANIWORLD · Centre de soins"));
    centre->setObjectName("foot");
    v->addWidget(who);
    v->addSpacing(6);
    v->addWidget(slogan);
    v->addWidget(centre);
    return side;
}

QWidget *MainWindow::buildHeader()
{
    auto *bar = new QFrame;
    bar->setObjectName("header");
    bar->setFixedHeight(62);
    auto *h = new QHBoxLayout(bar);
    h->setContentsMargins(26, 0, 26, 0);
    h->setSpacing(10);

    auto *crumb = new QLabel(QStringLiteral("Gestion / <span style='color:#1d3b44'>Locaux</span>"));
    crumb->setObjectName("sub");
    crumb->setTextFormat(Qt::RichText);
    h->addWidget(crumb);
    h->addStretch();

    auto *date = new QLabel(QLocale(QLocale::French).toString(QDate::currentDate(),
                                                              QStringLiteral("dddd d MMMM yyyy")));
    date->setObjectName("sub");
    h->addWidget(date);

    auto *save = new QPushButton;
    save->setProperty("role", "ghost");
    save->setFlat(true);
    auto refreshBadge = [this, save]() {
        if (m_repo->usesDatabase()) {
            save->setText(QStringLiteral("●  Base Oracle connectée"));
            save->setToolTip(QString());
            save->setCursor(Qt::ArrowCursor);
        } else {
            save->setText(QStringLiteral("●  Oracle non connecté — cliquer pour réessayer"));
            save->setToolTip(QStringLiteral("Erreur : ") + Connection::createInstance().lastError());
            save->setCursor(Qt::PointingHandCursor);
        }
    };
    refreshBadge();
    connect(save, &QPushButton::clicked, this, [this, refreshBadge]() {
        if (!m_repo->usesDatabase())
            m_repo->reconnect();
        refreshBadge();
    });
    h->addWidget(save);

    auto *anim = new QPushButton(QStringLiteral("Animations : ON"));
    anim->setProperty("role", "ghost");
    anim->setCheckable(true);
    anim->setChecked(true);
    anim->setCursor(Qt::PointingHandCursor);
    HoverGlow::attach(anim);
    connect(anim, &QPushButton::toggled, this, [anim](bool on) {
        Theme::animations = on;
        anim->setText(on ? QStringLiteral("Animations : ON") : QStringLiteral("Animations : OFF"));
    });
    h->addWidget(anim);

    auto *out = new QPushButton(QStringLiteral("Se déconnecter"));
    out->setProperty("role", "ghost");
    out->setCursor(Qt::PointingHandCursor);
    out->setIcon(Icons::icon("logout", Theme::TealDark, 16));
    HoverGlow::attach(out);
    connect(out, &QPushButton::clicked, this, &QWidget::close);
    h->addWidget(out);
    return bar;
}

QWidget *MainWindow::buildFooter()
{
    auto *bar = new QFrame;
    bar->setObjectName("footer");
    bar->setFixedHeight(38);
    auto *h = new QHBoxLayout(bar);
    h->setContentsMargins(26, 0, 26, 0);
    auto *l = new QLabel(QStringLiteral("ANIWORLD · Gestion des locaux"));
    l->setObjectName("foot");
    auto *r = new QLabel(QStringLiteral("Ctrl+N Ajouter   ·   Ctrl+F Rechercher"));
    r->setObjectName("foot");
    h->addWidget(l);
    h->addStretch();
    h->addWidget(r);
    return bar;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    if (m_paws) {
        if (e->type() == QEvent::Resize && obj == centralWidget()) {
            m_paws->setGeometry(centralWidget()->rect());
            m_paws->raise();
        } else if (e->type() == QEvent::MouseButtonPress && Theme::animations) {
            QWidget *w = qobject_cast<QWidget *>(obj);
            if (w && (qobject_cast<QAbstractButton *>(w) || qobject_cast<RoomCard *>(w))) {
                auto *me = static_cast<QMouseEvent *>(e);
                m_paws->spawn(m_paws->mapFromGlobal(me->globalPosition().toPoint()));
            }
        }
    }
    return QMainWindow::eventFilter(obj, e);
}
