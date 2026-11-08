#include "espacelocaux.h"
#include "connection.h"
#include "hoverglow.h"
#include "icons.h"
#include "localrepository.h"
#include "pagelocaux.h"
#include "theme.h"

#include <QDate>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QVBoxLayout>

EspaceLocaux::EspaceLocaux(QWidget *parent) : QWidget(parent)
{
    setObjectName("espaceLocaux");
    setAttribute(Qt::WA_StyledBackground, true);

    // style du module Locaux, applique seulement ici (et aux fenetres ouvertes depuis la page)
    QFile qss(QStringLiteral(":/locaux/style.qss"));
    if (qss.open(QIODevice::ReadOnly | QIODevice::Text))
        setStyleSheet(QString::fromUtf8(qss.readAll()));

    m_repo = new LocalRepository(this);

    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);
    v->addWidget(creerBande());
    v->addWidget(new PageLocaux(m_repo), 1);
    v->addWidget(creerPied());
}

QWidget *EspaceLocaux::creerBande()
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

    // statut Oracle (clic = retenter la connexion)
    m_badge = new QPushButton;
    m_badge->setProperty("role", "ghost");
    m_badge->setFlat(true);
    connect(m_badge, &QPushButton::clicked, this, [this] {
        if (!m_repo->usesDatabase())
            m_repo->reconnect();
        else
            m_repo->recharger();
        rafraichirBadge();
    });
    h->addWidget(m_badge);

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
    connect(out, &QPushButton::clicked, this, [this] { window()->close(); });   // comme le reste de l'application
    h->addWidget(out);
    return bar;
}

QWidget *EspaceLocaux::creerPied()
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

void EspaceLocaux::rafraichirBadge()
{
    if (m_repo->usesDatabase()) {
        m_badge->setText(QStringLiteral("●  Base Oracle connectée"));
        m_badge->setToolTip(QStringLiteral("Cliquer pour relire la table LOCAUX"));
        m_badge->setCursor(Qt::PointingHandCursor);
    } else {
        m_badge->setText(QStringLiteral("●  Oracle non connecté — cliquer pour réessayer"));
        m_badge->setToolTip(QStringLiteral("Erreur : ") + Connection::createInstance().lastError());
        m_badge->setCursor(Qt::PointingHandCursor);
    }
}

void EspaceLocaux::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    // la connexion a pu s'ouvrir apres le demarrage : on relit toujours les locaux en arrivant
    m_repo->recharger();
    rafraichirBadge();
}
