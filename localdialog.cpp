#include "localdialog.h"
#include "hoverglow.h"
#include "icons.h"
#include "localrepository.h"
#include "theme.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QtMath>

// ===================================================================== outils
namespace {

QPixmap iconTile(const QString &icon, const QColor &bg, const QColor &fg, int size)
{
    const qreal dpr = 2.0;
    QPixmap pm(int(size * dpr), int(size * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(QRectF(0, 0, size, size), size * 0.27, size * 0.27);
    const int is = int(size * 0.5);
    p.drawPixmap(QPointF((size - is) / 2.0, (size - is) / 2.0), Icons::pixmap(icon, fg, is));
    return pm;
}

QString iconForType(const QString &type)
{
    Local l;
    l.setType(type);
    return l.iconName();
}

QLabel *fieldLabel(const QString &text)
{
    auto *l = new QLabel(text);
    l->setObjectName("fieldLabel");
    return l;
}

QPushButton *roleButton(const QString &text, const char *role)
{
    auto *b = new QPushButton(text);
    b->setProperty("role", role);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

// change une propriete utilisee par la feuille de style puis redessine
void setStyleProp(QWidget *w, const char *name, const QString &value)
{
    if (w->property(name).toString() == value)
        return;
    w->setProperty(name, value);
    w->style()->unpolish(w);
    w->style()->polish(w);
}

QColor stateColor(const QString &etat)
{
    if (etat == Local::Occupe)      return QColor(0x5b, 0x76, 0xc9);
    if (etat == Local::ANettoyer)   return QColor(0xd0, 0x5a, 0x55);
    if (etat == Local::HorsService) return QColor(0xa9, 0xb4, 0xb8);
    return QColor(0x4a, 0xa5, 0x7a);
}

} // namespace

// ===================================================================== interrupteur
class ToggleSwitch : public QAbstractButton
{
public:
    explicit ToggleSwitch(QWidget *parent = nullptr) : QAbstractButton(parent)
    {
        setCheckable(true);
        setCursor(Qt::PointingHandCursor);
        setFixedSize(42, 24);
        m_anim = new QVariantAnimation(this);
        m_anim->setDuration(180);
        m_anim->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &v) {
            m_pos = v.toReal();
            update();
        });
        QObject::connect(this, &QAbstractButton::toggled, this, [this](bool on) {
            m_anim->stop();
            m_anim->setStartValue(m_pos);
            m_anim->setEndValue(on ? 1.0 : 0.0);
            m_anim->start();
        });
    }
    void setCheckedNow(bool on)   // sans animation (valeur initiale)
    {
        setChecked(on);
        m_anim->stop();
        m_pos = on ? 1.0 : 0.0;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QColor track = Theme::lerp(QColor(0xcf, 0xdd, 0xe1), QColor(0xd9, 0x97, 0x2f), m_pos);
        if (!isEnabled())
            track = QColor(0xe4, 0xea, 0xec);
        p.setPen(Qt::NoPen);
        p.setBrush(track);
        p.drawRoundedRect(rect(), height() / 2.0, height() / 2.0);
        const qreal d = height() - 6;
        p.setBrush(Qt::white);
        p.drawEllipse(QRectF(3 + (width() - 6 - d) * m_pos, 3, d, d));
    }

private:
    qreal m_pos = 0.0;
    QVariantAnimation *m_anim;
};

// ===================================================================== apercu en direct
class LocalPreview : public QWidget
{
public:
    struct Check { bool ok; QString text; };

    explicit LocalPreview(QWidget *parent = nullptr) : QWidget(parent) { setFixedSize(290, 430); }
    void setData(const Local &l, const QList<Check> &checks)
    {
        m_local = l;
        m_checks = checks;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        // carte
        const QRectF card = QRectF(rect()).adjusted(1, 1, -1, -1);
        p.setPen(QPen(QColor(0xe1, 0xeb, 0xee), 1));
        p.setBrush(Qt::white);
        p.drawRoundedRect(card, 16, 16);

        const qreal x = 18;
        const qreal w = width() - 36;
        const QColor col = stateColor(m_local.getEtat());

        // icone + nom + type
        p.drawPixmap(QPointF(x, 18), iconTile(m_local.iconName(), QColor(0xee, 0xf6, 0xf8), Theme::Teal, 40));
        QFont nf = Theme::font(13, true);
        p.setFont(nf);
        p.setPen(Theme::Ink);
        const QString name = m_local.getName().trimmed().isEmpty() ? QStringLiteral("Nouveau local")
                                                                    : m_local.getName().trimmed();
        p.drawText(QRectF(x, 66, w, 24), Qt::AlignLeft | Qt::AlignVCenter,
                   QFontMetricsF(nf).elidedText(name, Qt::ElideRight, w));
        p.setFont(Theme::font(9));
        p.setPen(Theme::Muted);
        p.drawText(QRectF(x, 90, w, 18), Qt::AlignLeft | Qt::AlignVCenter,
                   QStringLiteral("%1 · %2 · %3 place(s)").arg(m_local.getType(), m_local.floorShort())
                       .arg(m_local.getCapacity()));

        // badge de l'etat
        {
            const QFont bf = Theme::font(8.5, true);
            QColor bg = col;
            bg.setAlpha(40);
            Theme::drawPill(p, QRectF(x, 114, Theme::pillWidth(m_local.getEtat(), bf, true), 24),
                            m_local.getEtat(), bg, col.darker(125), bf, true);
        }

        // petite scene 2.5D : une dalle + la boite du local
        const QRectF scene(x, 150, w, 150);
        QLinearGradient sg(scene.topLeft(), scene.bottomLeft());
        sg.setColorAt(0, QColor(0xf3, 0xf8, 0xf9));
        sg.setColorAt(1, QColor(0xea, 0xf3, 0xf5));
        p.setPen(Qt::NoPen);
        p.setBrush(sg);
        p.drawRoundedRect(scene, 14, 14);
        drawScene(p, scene, col);

        // controles de saisie
        qreal y = 316;
        for (const Check &c : m_checks) {
            p.setFont(Theme::font(9.5, true));
            p.setPen(c.ok ? Theme::Green : Theme::Red);
            p.drawText(QRectF(x, y, 18, 22), Qt::AlignLeft | Qt::AlignVCenter,
                       c.ok ? QStringLiteral("✓") : QStringLiteral("✗"));
            p.setFont(Theme::font(9));
            p.setPen(c.ok ? QColor(0x4f, 0x6f, 0x78) : Theme::Red);
            p.drawText(QRectF(x + 20, y, w - 20, 22), Qt::AlignLeft | Qt::AlignVCenter, c.text);
            y += 24;
        }
    }

private:
    void drawScene(QPainter &p, const QRectF &scene, const QColor &col)
    {
        // meme projection que le plan 2.5D (vue a 45 degres)
        const qreal k = 30;
        const QPointF o(scene.center().x(), scene.center().y() + 22);
        const qreal c = qCos(M_PI / 4), s = qSin(M_PI / 4);
        auto P = [&](qreal wx, qreal wy, qreal wz) {
            return o + QPointF((wx * c - wy * s) * k, (wx * s + wy * c) * k * 0.5 - wz * k * 0.8);
        };
        auto box = [&](qreal bx, qreal by, qreal bz, qreal bw, qreal bd, qreal bh, const QColor &top) {
            const QPointF b[4] = { { bx, by }, { bx + bw, by }, { bx + bw, by + bd }, { bx, by + bd } };
            const QPointF n[4] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
            p.setPen(QPen(top.darker(150), 0.8));
            for (int i = 0; i < 4; ++i) {
                const qreal nY = n[i].x() * s + n[i].y() * c;
                if (nY <= 0.001)
                    continue;
                const qreal nX = n[i].x() * c - n[i].y() * s;
                QPolygonF f;
                f << P(b[i].x(), b[i].y(), bz) << P(b[(i + 1) % 4].x(), b[(i + 1) % 4].y(), bz)
                  << P(b[(i + 1) % 4].x(), b[(i + 1) % 4].y(), bz + bh) << P(b[i].x(), b[i].y(), bz + bh);
                p.setBrush(top.darker(int(118 + 26 * (nX + 1) / 2)));
                p.drawPolygon(f);
            }
            QPolygonF t;
            for (const QPointF &q : b)
                t << P(q.x(), q.y(), bz + bh);
            p.setBrush(top);
            p.drawPolygon(t);
            return t;
        };
        const qreal bw = m_local.getCapacity() >= 8 ? 2.2 : 1.0;
        box(-1.6, -1.1, 0, 3.2, 2.2, 0.16, QColor(0xe4, 0xef, 0xf2));             // dalle
        const QPolygonF top = box(-bw / 2, -0.5, 0.16, bw, 1.0, 0.9, col);         // local
        p.setPen(QPen(Theme::Orange, 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);
        p.drawPolygon(top);
    }

    Local m_local;
    QList<Check> m_checks;
};

// ===================================================================== formulaire

LocalDialog::LocalDialog(const Local *existing, LocalRepository *repo, QWidget *parent)
    : QDialog(parent), m_repo(repo), m_editing(existing != nullptr)
{
    setObjectName("localForm");
    setWindowTitle(m_editing ? QStringLiteral("Modifier le local") : QStringLiteral("Ajouter un local"));
    setMinimumWidth(900);
    if (existing)
        m_base = *existing;

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ---------------------------------------------------------------- entete
    auto *head = new QFrame;
    head->setObjectName("formHead");
    auto *hh = new QHBoxLayout(head);
    hh->setContentsMargins(28, 20, 28, 20);
    hh->setSpacing(14);
    auto *hi = new QLabel;
    hi->setPixmap(iconTile(m_editing ? "edit" : "plus", QColor(255, 255, 255, 40), Qt::white, 46));
    auto *ht = new QVBoxLayout;
    ht->setSpacing(2);
    auto *title = new QLabel(m_editing ? QStringLiteral("Modifier « %1 »").arg(m_base.getName())
                                       : QStringLiteral("Ajouter un local"));
    title->setObjectName("formTitle");
    auto *sub = new QLabel(QStringLiteral("Les champs marqués * sont obligatoires · l'aperçu se met à jour en direct"));
    sub->setObjectName("formSub");
    ht->addWidget(title);
    ht->addWidget(sub);
    hh->addWidget(hi);
    hh->addLayout(ht, 1);
    root->addWidget(head);

    // ---------------------------------------------------------------- corps : formulaire + apercu
    auto *body = new QHBoxLayout;
    body->setContentsMargins(28, 22, 28, 22);
    body->setSpacing(24);
    auto *form = new QVBoxLayout;
    form->setSpacing(6);

    // nom
    auto *nameHead = new QHBoxLayout;
    nameHead->addWidget(fieldLabel(QStringLiteral("Nom du local *")));
    nameHead->addStretch();
    m_nameCount = new QLabel;
    m_nameCount->setObjectName("fieldCount");
    nameHead->addWidget(m_nameCount);
    form->addLayout(nameHead);
    m_name = new QLineEdit(m_base.getName());
    m_name->setMaxLength(150);
    m_name->setPlaceholderText(QStringLiteral("ex. Consultation 6, Bloc C…"));
    form->addWidget(m_name);
    m_nameMsg = new QLabel;
    m_nameMsg->setWordWrap(true);
    form->addWidget(m_nameMsg);

    // type (boutons avec icones)
    form->addSpacing(6);
    form->addWidget(fieldLabel(QStringLiteral("Type")));
    m_types = new QButtonGroup(this);
    m_types->setExclusive(true);
    auto *typeGrid = new QGridLayout;
    typeGrid->setSpacing(7);
    const QStringList types = Local::types();
    const QString currentType = m_editing ? m_base.getType() : types.first();
    for (int i = 0; i < types.size(); ++i) {
        auto *b = roleButton(types[i], "typechip");
        b->setCheckable(true);
        b->setProperty("type", types[i]);
        b->setIcon(Icons::icon(iconForType(types[i]), Theme::TealDark, 16));
        connect(b, &QPushButton::toggled, b, [b](bool on) {
            b->setIcon(Icons::icon(iconForType(b->property("type").toString()),
                                   on ? QColor(Qt::white) : Theme::TealDark, 16));
        });
        m_types->addButton(b, i);
        typeGrid->addWidget(b, i / 4, i % 4);
        if (types[i] == currentType)
            b->setChecked(true);
    }
    if (!m_types->checkedButton())
        m_types->button(0)->setChecked(true);
    form->addLayout(typeGrid);

    // etage (selecteur segmente)
    form->addSpacing(6);
    form->addWidget(fieldLabel(QStringLiteral("Étage")));
    auto *seg = new QFrame;
    seg->setObjectName("segBar");
    auto *sl = new QHBoxLayout(seg);
    sl->setContentsMargins(3, 3, 3, 3);
    sl->setSpacing(3);
    m_floors = new QButtonGroup(this);
    m_floors->setExclusive(true);
    const QStringList floors = { "RDC", "Étage 1", "Étage 2", "Étage 3" };
    for (int i = 0; i < floors.size(); ++i) {
        auto *b = roleButton(floors[i], "seg");
        b->setCheckable(true);
        m_floors->addButton(b, i);
        sl->addWidget(b, 1);
    }
    m_floors->button(qBound(0, m_base.getFloor(), 3))->setChecked(true);
    form->addWidget(seg);

    // service
    form->addSpacing(6);
    form->addWidget(fieldLabel(QStringLiteral("Service *")));
    m_service = new QComboBox;
    const QList<QPair<int, QString>> services = Local::services();
    for (const auto &sv : services)
        m_service->addItem(sv.second, sv.first);
    if (services.isEmpty())
        m_service->addItem(QStringLiteral("Aucun service : exécutez sql/base_commune_aniworld.sql"), 0);
    const int idx = m_service->findData(m_base.getIdService());
    m_service->setCurrentIndex(idx >= 0 ? idx : 0);
    form->addWidget(m_service);

    // capacite (- / + et raccourcis)
    form->addSpacing(6);
    form->addWidget(fieldLabel(QStringLiteral("Capacité *")));
    auto *capRow = new QHBoxLayout;
    capRow->setSpacing(8);
    m_capMinus = roleButton(QStringLiteral("−"), "step");
    m_capPlus = roleButton(QStringLiteral("+"), "step");
    m_capValue = new QLabel;
    m_capValue->setFixedWidth(54);
    m_capValue->setAlignment(Qt::AlignCenter);
    m_capValue->setStyleSheet(QStringLiteral("font-size:17px;font-weight:700;color:#1d3b44;"));
    capRow->addWidget(m_capMinus);
    capRow->addWidget(m_capValue);
    capRow->addWidget(m_capPlus);
    capRow->addSpacing(8);
    for (int q : { 1, 4, 8, 20 }) {
        auto *b = roleButton(QString::number(q), "quick");
        b->setToolTip(QStringLiteral("%1 place(s)").arg(q));
        connect(b, &QPushButton::clicked, this, [this, q] { setCapacity(q); });
        capRow->addWidget(b);
    }
    capRow->addStretch();
    form->addLayout(capRow);
    m_capMsg = new QLabel;
    m_capMsg->setObjectName("sub");
    form->addWidget(m_capMsg);
    connect(m_capMinus, &QPushButton::clicked, this, [this] { setCapacity(m_capacity - 1); });
    connect(m_capPlus, &QPushButton::clicked, this, [this] { setCapacity(m_capacity + 1); });

    // specialite + suggestions selon le type
    form->addSpacing(4);
    form->addWidget(fieldLabel(QStringLiteral("Spécialité / remarque")));
    m_specialty = new QLineEdit(m_base.getSpecialty());
    m_specialty->setMaxLength(250);
    m_specialty->setPlaceholderText(QStringLiteral("ex. Généraliste, Aseptique, Chiens…"));
    form->addWidget(m_specialty);
    m_suggestRow = new QHBoxLayout;
    m_suggestRow->setSpacing(6);
    form->addLayout(m_suggestRow);

    // hors service (interrupteur)
    form->addSpacing(8);
    auto *sw = new QFrame;
    sw->setObjectName("switchCard");
    auto *swl = new QHBoxLayout(sw);
    swl->setContentsMargins(14, 11, 14, 11);
    swl->setSpacing(12);
    m_oos = new ToggleSwitch;
    m_oos->setCheckedNow(m_base.getOutOfService());
    auto *swt = new QVBoxLayout;
    swt->setSpacing(1);
    auto *swTitle = new QLabel(QStringLiteral("Hors service"));
    swTitle->setStyleSheet(QStringLiteral("font-weight:700;color:#1d3b44;"));
    auto *swSub = new QLabel(QStringLiteral("Travaux, panne ou désinfection : le local ne pourra pas recevoir d'animal"));
    swSub->setObjectName("sub");
    swSub->setWordWrap(true);
    swt->addWidget(swTitle);
    swt->addWidget(swSub);
    swl->addWidget(m_oos, 0, Qt::AlignVCenter);
    swl->addLayout(swt, 1);
    form->addWidget(sw);
    m_oosMsg = new QLabel;
    m_oosMsg->setWordWrap(true);
    m_oosMsg->setStyleSheet(QStringLiteral("color:#b97712;font-size:11px;"));
    form->addWidget(m_oosMsg);
    if (m_base.getOccupancy() > 0 && !m_base.getOutOfService()) {
        m_oos->setEnabled(false);
        m_oosMsg->setText(QStringLiteral("Libère d'abord les %1 animal(aux) présent(s) pour mettre ce local hors service.")
                              .arg(m_base.getOccupancy()));
    }
    m_oosMsg->setVisible(!m_oosMsg->text().isEmpty());
    form->addStretch();

    // apercu
    auto *pv = new QVBoxLayout;
    pv->setSpacing(8);
    auto *pk = new QLabel(QStringLiteral("APERÇU"));
    pk->setObjectName("kicker");
    QFont kf = pk->font();
    kf.setLetterSpacing(QFont::AbsoluteSpacing, 1.6);
    pk->setFont(kf);
    m_preview = new LocalPreview;
    pv->addWidget(pk);
    pv->addWidget(m_preview);
    pv->addStretch();

    body->addLayout(form, 1);
    body->addLayout(pv);
    root->addLayout(body, 1);

    // ---------------------------------------------------------------- pied
    auto *foot = new QFrame;
    foot->setObjectName("formFoot");
    auto *fl = new QHBoxLayout(foot);
    fl->setContentsMargins(28, 14, 28, 14);
    fl->setSpacing(10);
    auto *hint = new QLabel(QStringLiteral("Entrée pour enregistrer · Échap pour annuler"));
    hint->setObjectName("sub");
    auto *cancel = roleButton(QStringLiteral("Annuler"), "ghost");
    m_save = roleButton(m_editing ? QStringLiteral("Enregistrer les modifications")
                                  : QStringLiteral("Enregistrer le local"), "primary");
    m_save->setDefault(true);
    HoverGlow::attach(cancel);
    HoverGlow::attach(m_save);
    fl->addWidget(hint);
    fl->addStretch();
    fl->addWidget(cancel);
    fl->addWidget(m_save);
    root->addWidget(foot);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_save, &QPushButton::clicked, this, &LocalDialog::accept);

    // tout changement met a jour les controles et l'apercu
    connect(m_name, &QLineEdit::textChanged, this, [this] { refresh(); });
    connect(m_specialty, &QLineEdit::textChanged, this, [this] { refresh(); });
    connect(m_service, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { refresh(); });
    connect(m_floors, &QButtonGroup::idClicked, this, [this] { refresh(); });
    connect(m_types, &QButtonGroup::idClicked, this, [this] {
        refreshSuggestions();
        refresh();
    });
    connect(m_oos, &QAbstractButton::toggled, this, [this] { refresh(); });

    m_capacity = qBound(1, m_base.getCapacity(), 50);
    refreshSuggestions();
    setCapacity(m_capacity);     // appelle refresh()
    m_name->setFocus();
}

// ===================================================================== logique

Local LocalDialog::buildLocal() const
{
    Local r = m_base;
    r.setName(m_name->text().trimmed());
    if (m_types->checkedButton())
        r.setType(m_types->checkedButton()->property("type").toString());
    r.setFloor(qMax(0, m_floors->checkedId()));
    r.setCapacity(m_capacity);
    r.setSpecialty(m_specialty->text().trimmed());
    r.setIdService(m_service->currentData().toInt());
    r.setOutOfService(m_oos->isChecked());      // "Hors service" <-> "Disponible"
    return r;
}

QString LocalDialog::nameProblem() const
{
    const QString name = m_name->text().trimmed();
    if (name.isEmpty())
        return QStringLiteral("Le nom du local est obligatoire.");
    if (name.size() < 2)
        return QStringLiteral("2 caractères minimum.");
    if (m_repo->nameExists(name, m_base.getId()))
        return QStringLiteral("Un local porte déjà ce nom.");
    return QString();
}

void LocalDialog::setCapacity(int v)
{
    const int minCap = qMax(1, m_base.getOccupancy());   // jamais moins que les animaux presents
    m_capacity = qBound(minCap, v, 50);
    m_capValue->setText(QString::number(m_capacity));
    m_capMinus->setEnabled(m_capacity > minCap);
    m_capPlus->setEnabled(m_capacity < 50);
    m_capMsg->setText(m_base.getOccupancy() > 0
                          ? QStringLiteral("Minimum %1 : animaux présents dans le local").arg(minCap)
                          : QStringLiteral("Entre 1 et 50 places"));
    refresh();
}

void LocalDialog::refreshSuggestions()
{
    while (QLayoutItem *it = m_suggestRow->takeAt(0)) {
        if (it->widget())
            it->widget()->deleteLater();
        delete it;
    }
    const QString type = m_types->checkedButton() ? m_types->checkedButton()->property("type").toString()
                                                  : QString();
    QStringList s;
    if (type == "Consultation")         s = { "Généraliste", "Dentiste", "Dermatologie" };
    else if (type == "Chirurgie")       s = { "Aseptique", "Orthopédie", "Soins intensifs" };
    else if (type == "Hospitalisation") s = { "Soins post-op", "Perfusion", "Convalescence" };
    else if (type == "Pension")         s = { "Chiens", "Chats", "NAC" };
    else if (type == "Laboratoire")     s = { "Analyses", "Hématologie", "Parasitologie" };
    else if (type == "Imagerie")        s = { "Radio / écho", "Scanner", "Échographie" };
    else if (type == "Isolement")       s = { "Maladies contagieuses", "Quarantaine" };
    for (const QString &text : s) {
        auto *b = roleButton(text, "suggest");
        connect(b, &QPushButton::clicked, this, [this, text] { m_specialty->setText(text); });
        m_suggestRow->addWidget(b);
    }
    m_suggestRow->addStretch();
}

void LocalDialog::refresh()
{
    // nom : compteur + message vert / rouge
    const QString problem = nameProblem();
    const bool typed = !m_name->text().trimmed().isEmpty();
    m_nameCount->setText(QStringLiteral("%1 / 150").arg(m_name->text().size()));
    if (problem.isEmpty()) {
        setStyleProp(m_name, "check", "ok");
        m_nameMsg->setStyleSheet(QStringLiteral("color:#2e8b57;font-size:11px;"));
        m_nameMsg->setText(QStringLiteral("✓ Nom disponible"));
    } else if (typed || m_triedSave) {
        setStyleProp(m_name, "check", "err");
        m_nameMsg->setStyleSheet(QStringLiteral("color:#c0504d;font-size:11px;"));
        m_nameMsg->setText(QStringLiteral("✗ ") + problem);
    } else {
        setStyleProp(m_name, "check", "");
        m_nameMsg->setStyleSheet(QStringLiteral("color:#6f8a92;font-size:11px;"));
        m_nameMsg->setText(QStringLiteral("Obligatoire, unique, 2 caractères minimum"));
    }

    // apercu + liste des controles
    const Local l = buildLocal();
    QList<LocalPreview::Check> checks;
    checks.append({ problem.isEmpty(), problem.isEmpty() ? QStringLiteral("Nom unique") : problem });
    checks.append({ m_capacity >= 1 && m_capacity <= 50, QStringLiteral("Capacité entre 1 et 50") });
    checks.append({ l.getIdService() > 0, QStringLiteral("Service choisi") });
    if (m_oos->isChecked())
        checks.append({ m_base.getOccupancy() == 0, QStringLiteral("Local vide (hors service)") });
    m_preview->setData(l, checks);
}

void LocalDialog::accept()
{
    m_triedSave = true;
    refresh();
    if (!nameProblem().isEmpty()) {
        m_name->setFocus();
        return;
    }
    if (m_oos->isChecked() && m_base.getOccupancy() > 0)
        return;

    m_result = buildLocal();
    // controle de saisie final, fait dans la classe de l'entite (comme l'atelier)
    const QString err = m_result.controleSaisie();
    if (!err.isEmpty()) {
        setStyleProp(m_name, "check", "err");
        m_nameMsg->setStyleSheet(QStringLiteral("color:#c0504d;font-size:11px;"));
        m_nameMsg->setText(QStringLiteral("✗ ") + err);
        return;
    }
    QDialog::accept();
}
