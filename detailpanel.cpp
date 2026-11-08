#include "detailpanel.h"
#include "hoverglow.h"
#include "icons.h"
#include "theme.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>
#include <QVariantAnimation>

// ===================================================================== couleurs par etat
namespace {

struct StateLook {
    QColor main;    // couleur forte (icone, anneau, etape)
    QColor soft;    // fond du badge
    QColor text;    // texte du badge
    QColor head1;   // degrade de l'entete
    QColor head2;
};

StateLook lookFor(const Local &l)
{
    switch (l.state()) {
    case RoomState::Available:
        return { QColor(0x4a, 0xa5, 0x7a), QColor(0xd3, 0xec, 0xdd), QColor(0x2e, 0x8b, 0x57),
                 QColor(0xe3, 0xf3, 0xe9), QColor(0xf7, 0xfb, 0xf9) };
    case RoomState::Occupied:
        return { QColor(0x5b, 0x76, 0xc9), QColor(0xd9, 0xde, 0xf5), QColor(0x4a, 0x63, 0xb8),
                 QColor(0xe6, 0xe9, 0xf8), QColor(0xf8, 0xf9, 0xfd) };
    case RoomState::ToClean:
        return { QColor(0xd0, 0x5a, 0x55), QColor(0xf6, 0xd6, 0xd4), QColor(0xc0, 0x50, 0x4d),
                 QColor(0xfb, 0xe9, 0xe8), QColor(0xfd, 0xf7, 0xf7) };
    case RoomState::OutOfService:
        break;
    }
    return { QColor(0x8f, 0x9d, 0xa2), QColor(0xe4, 0xea, 0xec), QColor(0x5f, 0x72, 0x78),
             QColor(0xee, 0xf2, 0xf3), QColor(0xf8, 0xfa, 0xfb) };
}

QPushButton *makeButton(const QString &text, const QString &icon, const char *role)
{
    auto *b = new QPushButton(text);
    b->setProperty("role", role);
    b->setCursor(Qt::PointingHandCursor);
    if (!icon.isEmpty()) {
        const QString r(role);
        const QColor ic = (r == "primary" || r == "success") ? QColor(Qt::white)
                        : (r == "softdanger") ? Theme::Red : Theme::TealDark;
        b->setIcon(Icons::icon(icon, ic, 16));
        b->setIconSize(QSize(16, 16));
    }
    HoverGlow::attach(b);
    return b;
}

void setRole(QPushButton *b, const char *role)
{
    if (b->property("role").toString() == QLatin1String(role))
        return;
    b->setProperty("role", role);
    b->style()->unpolish(b);
    b->style()->polish(b);
}

// icone du type dans un carre arrondi colore
QPixmap iconTile(const QString &icon, const QColor &bg, int size)
{
    const qreal dpr = 2.0;
    QPixmap pm(int(size * dpr), int(size * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(QRectF(0, 0, size, size), size * 0.26, size * 0.26);
    const int is = int(size * 0.5);
    p.drawPixmap(QPointF((size - is) / 2.0, (size - is) / 2.0), Icons::pixmap(icon, Qt::white, is));
    return pm;
}

} // namespace

// ===================================================================== anneau d'occupation (anime)
class OccupancyRing : public QWidget
{
public:
    explicit OccupancyRing(QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedSize(90, 90);
        m_anim = new QVariantAnimation(this);
        m_anim->setDuration(600);
        m_anim->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            m_shown = v.toReal();
            update();
        });
    }
    void setValue(int occ, int cap, const QColor &color)
    {
        m_occ = occ;
        m_cap = qMax(1, cap);
        m_color = color;
        const qreal target = qreal(occ) / m_cap;
        m_anim->stop();
        if (Theme::animations) {
            m_anim->setStartValue(m_shown);
            m_anim->setEndValue(target);
            m_anim->start();
        } else {
            m_shown = target;
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r(7, 7, width() - 14, height() - 14);
        p.setPen(QPen(QColor(0xe6, 0xee, 0xf0), 10, Qt::SolidLine, Qt::FlatCap));
        p.drawArc(r, 0, 360 * 16);
        if (m_shown > 0.001) {
            p.setPen(QPen(m_color, 10, Qt::SolidLine, Qt::RoundCap));
            p.drawArc(r, 90 * 16, -int(360 * 16 * qMin<qreal>(1.0, m_shown)));
        }
        p.setPen(Theme::Ink);
        p.setFont(Theme::font(m_cap >= 10 ? 12.5 : 14, true));
        p.drawText(QRectF(0, height() / 2.0 - 16, width(), 22), Qt::AlignCenter,
                   QStringLiteral("%1/%2").arg(m_occ).arg(m_cap));
        p.setPen(Theme::Muted);
        p.setFont(Theme::font(7.5));
        p.drawText(QRectF(0, height() / 2.0 + 5, width(), 14), Qt::AlignCenter, QStringLiteral("animaux"));
    }

private:
    int m_occ = 0, m_cap = 1;
    qreal m_shown = 0;
    QColor m_color = Theme::Teal;
    QVariantAnimation *m_anim;
};

// ===================================================================== cycle de vie
class LifeStepper : public QWidget
{
public:
    explicit LifeStepper(QWidget *parent = nullptr) : QWidget(parent) { setFixedHeight(44); }
    // step : 0 Disponible, 1 Occupé, 2 À nettoyer, -1 Hors service
    void setStep(int step, const QColor &color)
    {
        m_step = step;
        m_color = color;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QStringList names = { Local::Disponible, Local::Occupe, Local::ANettoyer };
        const qreal left = 14, right = width() - 14, y = 12;
        const QColor idle(0xcf, 0xdd, 0xe1);
        auto xAt = [&](int i) { return left + (right - left) * i / 2.0; };

        // traits entre les etapes
        for (int i = 0; i < 2; ++i) {
            const bool done = m_step > i;
            p.setPen(QPen(done ? m_color : QColor(0xe1, 0xeb, 0xee), 2.5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(QPointF(xAt(i) + 8, y), QPointF(xAt(i + 1) - 8, y));
        }
        // points + noms
        for (int i = 0; i < 3; ++i) {
            const QPointF c(xAt(i), y);
            const bool current = (i == m_step);
            const bool done = (m_step >= 0 && i <= m_step);
            if (current) {
                QColor halo = m_color;
                halo.setAlpha(55);
                p.setPen(Qt::NoPen);
                p.setBrush(halo);
                p.drawEllipse(c, 11, 11);
            }
            p.setPen(QPen(done ? m_color : idle, 2));
            p.setBrush(done ? m_color : QColor(Qt::white));
            p.drawEllipse(c, 6.5, 6.5);

            QFont f = Theme::font(8, current);
            p.setFont(f);
            p.setPen(current ? m_color.darker(115) : Theme::Muted);
            const Qt::Alignment al = i == 0 ? Qt::AlignLeft : i == 2 ? Qt::AlignRight : Qt::AlignHCenter;
            const qreal w = 110;
            const qreal x = i == 0 ? 0 : i == 2 ? width() - w : c.x() - w / 2;
            p.drawText(QRectF(x, y + 12, w, 18), al | Qt::AlignVCenter, names[i]);
        }
        // hors service : le cycle est en pause
        if (m_step < 0) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 255, 255, 170));
            p.drawRect(rect());
            QFont f = Theme::font(8.5, true);
            p.setFont(f);
            p.setPen(QColor(0x5f, 0x72, 0x78));
            p.drawText(rect(), Qt::AlignCenter, QStringLiteral("Cycle en pause : local hors service"));
        }
    }

private:
    int m_step = 0;
    QColor m_color = Theme::Teal;
};

// ===================================================================== panneau

DetailPanel::DetailPanel(QWidget *parent) : QFrame(parent)
{
    setObjectName("panel");
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(20, 18, 20, 18);
    v->setSpacing(10);

    auto *kicker = new QLabel(QStringLiteral("LOCAL SÉLECTIONNÉ"));
    kicker->setObjectName("kicker");
    QFont kf = kicker->font();
    kf.setLetterSpacing(QFont::AbsoluteSpacing, 1.6);
    kicker->setFont(kf);
    v->addWidget(kicker);

    // ---------------------------------------------------------------- aucune selection
    m_empty = new QWidget;
    {
        auto *ev = new QVBoxLayout(m_empty);
        ev->setContentsMargins(0, 18, 0, 18);
        ev->setSpacing(8);
        auto *ic = new QLabel;
        ic->setPixmap(iconTile("rooms", QColor(0xb9, 0xd4, 0xda), 46));
        ic->setAlignment(Qt::AlignCenter);
        auto *t = new QLabel(QStringLiteral("Aucun local sélectionné"));
        t->setObjectName("panelTitle");
        t->setAlignment(Qt::AlignCenter);
        auto *s = new QLabel(QStringLiteral("Clique sur une carte, une ligne de la liste ou une salle du plan pour voir sa fiche."));
        s->setObjectName("sub");
        s->setWordWrap(true);
        s->setAlignment(Qt::AlignCenter);
        ev->addWidget(ic);
        ev->addWidget(t);
        ev->addWidget(s);
    }
    v->addWidget(m_empty);

    // ---------------------------------------------------------------- fiche
    m_content = new QWidget;
    auto *cv = new QVBoxLayout(m_content);
    cv->setContentsMargins(0, 0, 0, 0);
    cv->setSpacing(10);

    // entete colore
    m_head = new QFrame;
    m_head->setObjectName("detailHead");
    auto *hh = new QHBoxLayout(m_head);
    hh->setContentsMargins(14, 14, 14, 14);
    hh->setSpacing(12);
    m_icon = new QLabel;
    m_icon->setFixedSize(46, 46);
    auto *names = new QVBoxLayout;
    names->setSpacing(2);
    m_name = new QLabel;
    m_name->setObjectName("panelName");
    m_name->setWordWrap(true);
    m_sub = new QLabel;
    m_sub->setObjectName("sub");
    names->addWidget(m_name);
    names->addWidget(m_sub);
    m_badge = new QLabel;
    hh->addWidget(m_icon, 0, Qt::AlignTop);
    hh->addLayout(names, 1);
    hh->addWidget(m_badge, 0, Qt::AlignTop);
    cv->addWidget(m_head);

    // occupation
    auto *occRow = new QHBoxLayout;
    occRow->setSpacing(14);
    m_ring = new OccupancyRing;
    auto *occText = new QVBoxLayout;
    occText->setSpacing(2);
    m_occTitle = new QLabel;
    m_occTitle->setStyleSheet(QStringLiteral("font-weight:700;color:#1d3b44;"));
    m_occLine1 = new QLabel;
    m_occLine1->setObjectName("sub");
    m_occLine2 = new QLabel;
    m_occLine2->setObjectName("sub");
    m_occLine2->setWordWrap(true);
    occText->addStretch();
    occText->addWidget(m_occTitle);
    occText->addWidget(m_occLine1);
    occText->addWidget(m_occLine2);
    occText->addStretch();
    occRow->addWidget(m_ring);
    occRow->addLayout(occText, 1);
    cv->addLayout(occRow);

    // cycle de vie
    m_stepper = new LifeStepper;
    cv->addWidget(m_stepper);

    // informations
    auto *sep = new QFrame;
    sep->setFixedHeight(1);
    sep->setStyleSheet(QStringLiteral("background:#eef3f5;"));
    cv->addWidget(sep);
    auto *g = new QGridLayout;
    g->setHorizontalSpacing(16);
    g->setVerticalSpacing(6);
    auto *k1 = new QLabel(QStringLiteral("Spécialité"));
    k1->setObjectName("key");
    auto *k2 = new QLabel(QStringLiteral("Service"));
    k2->setObjectName("key");
    m_specialty = new QLabel;
    m_service = new QLabel;
    m_specialty->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_service->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_service->setWordWrap(true);
    g->addWidget(k1, 0, 0);
    g->addWidget(m_specialty, 0, 1);
    g->addWidget(k2, 1, 0);
    g->addWidget(m_service, 1, 1);
    g->setColumnStretch(1, 1);
    cv->addLayout(g);

    // boutons
    m_btnMain = makeButton(QStringLiteral("Affecter un animal"), "", "primary");
    m_btnFree = makeButton(QStringLiteral("Retirer (−1)"), "", "ghost");
    m_btnClean = makeButton(QStringLiteral("Signaler à nettoyer"), "sparkle", "ghost");
    m_btnEdit = makeButton(QStringLiteral("Modifier"), "edit", "ghost");
    m_btnDelete = makeButton(QStringLiteral("Supprimer"), "trash", "softdanger");
    auto *bl = new QGridLayout;
    bl->setSpacing(8);
    bl->addWidget(m_btnMain, 0, 0, 1, 2);
    bl->addWidget(m_btnFree, 1, 0);
    bl->addWidget(m_btnClean, 1, 1);
    bl->addWidget(m_btnEdit, 2, 0);
    bl->addWidget(m_btnDelete, 2, 1);
    cv->addSpacing(4);
    cv->addLayout(bl);

    v->addWidget(m_content);
    v->addStretch();

    connect(m_btnMain, &QPushButton::clicked, this, [this] {
        if (m_mainCleans)
            emit cleanRequested();      // "Nettoyage terminé"
        else
            emit assignRequested();     // "Affecter un animal"
    });
    connect(m_btnFree, &QPushButton::clicked, this, &DetailPanel::freeRequested);
    connect(m_btnClean, &QPushButton::clicked, this, &DetailPanel::cleanRequested);
    connect(m_btnEdit, &QPushButton::clicked, this, &DetailPanel::editRequested);
    connect(m_btnDelete, &QPushButton::clicked, this, &DetailPanel::deleteRequested);

    setLocal(nullptr);
}

void DetailPanel::setLocal(const Local *l)
{
    const bool has = (l != nullptr);
    m_empty->setVisible(!has);
    m_content->setVisible(has);
    if (!has)
        return;

    const StateLook look = lookFor(*l);
    const QString etat = l->getEtat();
    const int occ = l->getOccupancy();
    const int cap = l->getCapacity();
    const int freePlaces = qMax(0, cap - occ);

    // ---- entete
    m_head->setStyleSheet(QStringLiteral(
        "#detailHead{border-radius:14px;background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2);}")
        .arg(look.head1.name(), look.head2.name()));
    m_icon->setPixmap(iconTile(l->iconName(), look.main, 46));
    m_name->setText(l->getName());
    m_sub->setText(QStringLiteral("%1 · %2 · #%3").arg(l->getType(), l->floorShort()).arg(l->getId()));
    m_badge->setText(QStringLiteral("●  ") + etat);
    m_badge->setStyleSheet(QStringLiteral(
        "background:%1;color:%2;border-radius:9px;padding:4px 10px;font-weight:600;")
        .arg(look.soft.name(), look.text.name()));

    // ---- occupation
    m_ring->setValue(occ, cap, look.main);
    const int pct = qRound(100.0 * occ / qMax(1, cap));
    if (etat == Local::ANettoyer) {
        m_occTitle->setText(QStringLiteral("Nettoyage nécessaire"));
        m_occLine1->setText(l->isUrgentType() ? QStringLiteral("Local sensible : prioritaire")
                                              : QStringLiteral("À faire avant le prochain animal"));
        m_occLine2->setText(QStringLiteral("Aucun animal tant qu'il est sale"));
    } else if (etat == Local::HorsService) {
        m_occTitle->setText(QStringLiteral("Local hors service"));
        m_occLine1->setText(QStringLiteral("Travaux, panne ou désinfection"));
        m_occLine2->setText(QStringLiteral("Remise en service : bouton Modifier"));
    } else {
        m_occTitle->setText(QStringLiteral("Occupation %1 %").arg(pct));
        m_occLine1->setText(freePlaces == 1 ? QStringLiteral("1 place libre sur %1").arg(cap)
                                            : QStringLiteral("%1 places libres sur %2").arg(freePlaces).arg(cap));
        m_occLine2->setText(freePlaces == 0 ? QStringLiteral("Complet")
                            : occ == 0      ? QStringLiteral("Local prêt à accueillir")
                                            : QStringLiteral("Peut encore accueillir"));
    }

    // ---- cycle de vie
    const int step = etat == Local::Disponible ? 0 : etat == Local::Occupe ? 1
                   : etat == Local::ANettoyer  ? 2 : -1;
    m_stepper->setStep(step, look.main);

    // ---- informations
    m_specialty->setText(l->getSpecialty().isEmpty() ? QStringLiteral("—") : l->getSpecialty());
    QString service = QStringLiteral("—");
    for (const auto &sv : Local::services())
        if (sv.first == l->getIdService())
            service = sv.second;
    m_service->setText(service);

    // ---- boutons : chaque action n'est possible que si elle a un sens
    m_mainCleans = (etat == Local::ANettoyer);
    if (m_mainCleans) {
        m_btnMain->setText(QStringLiteral("Nettoyage terminé"));
        m_btnMain->setIcon(Icons::icon("check", Qt::white, 16));
        setRole(m_btnMain, "success");
        m_btnMain->setEnabled(true);
        m_btnMain->setToolTip(QStringLiteral("Le local redevient disponible"));
    } else {
        m_btnMain->setText(QStringLiteral("Affecter un animal"));
        m_btnMain->setIcon(QIcon(Icons::paw(Qt::white, 16)));
        setRole(m_btnMain, "primary");
        m_btnMain->setEnabled(l->peutRecevoirAnimal());
        m_btnMain->setToolTip(l->peutRecevoirAnimal() ? QString()
            : etat == Local::HorsService ? QStringLiteral("Le local est hors service")
                                         : QStringLiteral("Le local est complet"));
    }

    m_btnFree->setEnabled(occ > 0);
    m_btnFree->setToolTip(occ > 0 ? QStringLiteral("Retire un animal ; quand le local est vide il passe « À nettoyer »")
                                  : QStringLiteral("Aucun animal dans ce local"));

    m_btnClean->setEnabled(etat == Local::Disponible);
    m_btnClean->setToolTip(etat == Local::Disponible ? QStringLiteral("Le local passe « À nettoyer »")
                                                     : QStringLiteral("Possible seulement quand le local est vide et disponible"));
}
