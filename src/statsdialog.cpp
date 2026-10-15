#include "statsdialog.h"
#include "hoverglow.h"
#include "icons.h"
#include "theme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSet>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QVariantAnimation>
#include <algorithm>

namespace {

struct TypeRow {
    QString type;
    int rooms = 0;
    int places = 0;
    int occ = 0;
    int pct() const { return places > 0 ? qRound(100.0 * occ / places) : 0; }
};

struct Counter {
    QLabel *label;
    int target;
    QString suffix;
};

// couleur de la barre selon le taux : vert (<50), orange (50-79), rouge (>=80)
QColor rateColor(int pct)
{
    if (pct >= 80)
        return Theme::Red;
    if (pct >= 50)
        return QColor(0xe0, 0x9b, 0x3d);
    return QColor(0x4a, 0xa5, 0x7a);
}

// barre de taux d'occupation (une par ligne du tableau)
class RateBar : public QWidget
{
public:
    explicit RateBar(int pct, QWidget *parent = nullptr) : QWidget(parent), m_pct(pct)
    {
        setMinimumHeight(30);
    }
    void setProgress(qreal v)
    {
        m_p = v;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QColor c = rateColor(m_pct);
        const QRectF track(8, height() / 2.0 - 7, width() - 8 - 64, 14);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0xee, 0xf3, 0xf5));
        p.drawRoundedRect(track, 7, 7);
        const qreal w = track.width() * m_pct / 100.0 * m_p;
        if (w >= 1.0) {
            p.setBrush(c);
            p.drawRoundedRect(QRectF(track.left(), track.top(), w, track.height()), 7, 7);
        }
        p.setPen(c);
        p.setFont(Theme::font(9.5, true));
        p.drawText(QRectF(width() - 58, 0, 50, height()), Qt::AlignVCenter | Qt::AlignRight,
                   QStringLiteral("%1 %").arg(qRound(m_pct * m_p)));
    }

private:
    int m_pct;
    qreal m_p = 1.0;
};

// barre empilee : propre / en cours / a nettoyer
class CleanBar : public QWidget
{
public:
    CleanBar(int clean, int progress, int dirty, QWidget *parent = nullptr)
        : QWidget(parent), m_clean(clean), m_prog(progress), m_dirty(dirty)
    {
        setFixedHeight(26);
    }
    void setProgress(qreal v)
    {
        m_p = v;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r(0, height() / 2.0 - 9, width(), 18);
        QPainterPath path;
        path.addRoundedRect(r, 9, 9);
        p.fillPath(path, QColor(0xee, 0xf3, 0xf5));

        const int total = m_clean + m_prog + m_dirty;
        if (total <= 0)
            return;
        p.save();
        p.setClipPath(path);
        qreal x = r.left();
        const qreal full = r.width() * m_p;
        auto seg = [&](int n, const QColor &c) {
            const qreal w = full * n / total;
            p.fillRect(QRectF(x, r.top(), w, r.height()), c);
            x += w;
        };
        seg(m_clean, QColor(0x4a, 0xa5, 0x7a));
        seg(m_prog, QColor(0xe0, 0x9b, 0x3d));
        seg(m_dirty, Theme::Red);
        p.restore();
    }

private:
    int m_clean, m_prog, m_dirty;
    qreal m_p = 1.0;
};

QWidget *sectionTitle(const QString &icon, const QString &text, const QString &hint)
{
    auto *w = new QWidget;
    auto *h = new QHBoxLayout(w);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(8);
    auto *ic = new QLabel;
    ic->setPixmap(Icons::pixmap(icon, Theme::Teal, 18));
    auto *t = new QLabel(text);
    t->setObjectName("panelTitle");
    auto *s = new QLabel(hint);
    s->setObjectName("sub");
    h->addWidget(ic);
    h->addWidget(t);
    h->addSpacing(6);
    h->addWidget(s);
    h->addStretch();
    return w;
}

} // namespace

StatsDialog::StatsDialog(const QList<Local> &all, QWidget *parent) : QDialog(parent)
{
    setObjectName("statsDialog");
    setWindowTitle(QStringLiteral("Statistiques des locaux"));
    resize(940, 720);
    setMinimumSize(780, 580);

    // ------------------------------------------------------------ calculs
    QSet<int> floors;
    int cap = 0, occ = 0, avail = 0, busy = 0, clean = 0, prog = 0, dirty = 0;
    QMap<QString, TypeRow> byType;
    for (const Local &l : all) {
        floors.insert(l.floor);
        if (l.state() == RoomState::Available) ++avail;
        if (l.state() == RoomState::Occupied) ++busy;
        switch (l.cleaning) {
        case CleaningState::Clean:      ++clean; break;
        case CleaningState::InProgress: ++prog;  break;
        case CleaningState::ToClean:    ++dirty; break;
        }
        TypeRow &r = byType[l.type];
        r.type = l.type;
        r.rooms++;
        if (!l.outOfService) {
            cap += l.capacity;
            occ += l.occupancy;
            r.places += l.capacity;
            r.occ += l.occupancy;
        }
    }
    const int total = all.size();
    const int pct = cap > 0 ? qRound(100.0 * occ / cap) : 0;

    QList<TypeRow> rows = byType.values();
    std::stable_sort(rows.begin(), rows.end(),
                     [](const TypeRow &a, const TypeRow &b) { return a.pct() > b.pct(); });

    QList<Counter> counters;
    QList<RateBar *> bars;

    // ------------------------------------------------------------ mise en page
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 22);
    root->setSpacing(14);

    // titre
    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(12);
    auto *tIcon = new QLabel;
    tIcon->setPixmap(Icons::pixmap("chart", Theme::Teal, 34));
    auto *tCol = new QVBoxLayout;
    tCol->setSpacing(0);
    auto *title = new QLabel(QStringLiteral("Statistiques des locaux"));
    title->setObjectName("statsTitle");
    auto *sub = new QLabel(QStringLiteral("Occupation, état et nettoyage des espaces de la clinique"));
    sub->setObjectName("sub");
    tCol->addWidget(title);
    tCol->addWidget(sub);
    titleRow->addWidget(tIcon);
    titleRow->addLayout(tCol);
    titleRow->addStretch();
    root->addLayout(titleRow);

    // tuiles
    auto makeTile = [&](const QString &name, int value, const QString &suffix, const QString &sub2,
                        const QString &icon, const QColor &bg, const QColor &fg) -> QWidget * {
        auto *f = new QFrame;
        f->setObjectName("statTile");
        f->setStyleSheet(QStringLiteral("#statTile{background:%1;border-radius:14px;}").arg(bg.name()));
        auto *h = new QHBoxLayout(f);
        h->setContentsMargins(16, 14, 16, 14);
        h->setSpacing(14);
        auto *ic = new QLabel;
        ic->setPixmap(Icons::pixmap(icon, fg, 30));
        auto *v = new QVBoxLayout;
        v->setSpacing(1);
        auto *t = new QLabel(name);
        t->setStyleSheet(QStringLiteral("color:%1;font-size:12px;").arg(fg.name()));
        auto *val = new QLabel(QStringLiteral("0") + suffix);
        val->setStyleSheet(QStringLiteral("color:%1;font-size:26px;font-weight:700;")
                               .arg(fg.darker(120).name()));
        auto *s = new QLabel(sub2);
        s->setStyleSheet(QStringLiteral("color:#4a6770;font-size:11px;"));
        v->addWidget(t);
        v->addWidget(val);
        v->addWidget(s);
        h->addWidget(ic);
        h->addLayout(v, 1);
        counters.append(Counter{ val, value, suffix });
        return f;
    };

    auto *tiles = new QHBoxLayout;
    tiles->setSpacing(14);
    tiles->addWidget(makeTile(QStringLiteral("Total"), total, QString(),
        QStringLiteral("locaux sur %1 niveau(x)").arg(floors.size()), "rooms",
        QColor(0xe6, 0xf0, 0xf3), Theme::Teal));
    tiles->addWidget(makeTile(QStringLiteral("Taux d'occupation"), pct, QStringLiteral(" %"),
        QStringLiteral("%1 animaux pour %2 places").arg(occ).arg(cap), "chart",
        Theme::OrangeLight, Theme::OrangeText));
    tiles->addWidget(makeTile(QStringLiteral("Disponibles"), avail, QString(),
        QStringLiteral("locaux libres maintenant"), "records",
        Theme::GreenLight, Theme::Green));
    tiles->addWidget(makeTile(QStringLiteral("À nettoyer"), dirty, QString(),
        QStringLiteral("locaux à nettoyer"), "sparkle",
        Theme::RedLight, Theme::Red));
    root->addLayout(tiles);

    // tableau par type
    root->addWidget(sectionTitle("rooms", QStringLiteral("Occupation par type de local"),
        QStringLiteral("Plus la barre est longue, plus le type de local est rempli")));

    auto *table = new QTableWidget(rows.size(), 5);
    table->setHorizontalHeaderLabels({ "Type de local", "Locaux", "Places", "Occupés", "Taux d'occupation" });
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(42);
    table->horizontalHeader()->setHighlightSections(false);
    table->horizontalHeader()->setMinimumHeight(40);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int c = 1; c <= 3; ++c) {
        table->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Fixed);
        table->setColumnWidth(c, 92);
    }
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    table->setShowGrid(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::NoFocus);
    table->setMinimumHeight(190);
    if (table->horizontalHeaderItem(0))
        table->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    for (int r = 0; r < rows.size(); ++r) {
        const TypeRow &row = rows[r];
        Local tmp;
        tmp.type = row.type;
        auto *name = new QTableWidgetItem(Icons::icon(tmp.iconName(), Theme::Teal, 18), "  " + row.type);
        QFont f = name->font();
        f.setBold(true);
        name->setFont(f);
        table->setItem(r, 0, name);
        const int vals[3] = { row.rooms, row.places, row.occ };
        for (int c = 0; c < 3; ++c) {
            auto *it = new QTableWidgetItem(QString::number(vals[c]));
            it->setTextAlignment(Qt::AlignCenter);
            table->setItem(r, c + 1, it);
        }
        auto *bar = new RateBar(row.pct());
        table->setCellWidget(r, 4, bar);
        bars.append(bar);
    }
    root->addWidget(table, 1);

    // nettoyage
    root->addWidget(sectionTitle("sparkle", QStringLiteral("État du nettoyage"),
        QStringLiteral("Combien de locaux sont propres, en cours ou à nettoyer")));
    auto *cleanBar = new CleanBar(clean, prog, dirty);
    root->addWidget(cleanBar);
    auto *legend = new QLabel(QStringLiteral(
        "<span style='color:#4aa57a'>●</span> Propre <b>%1</b> &nbsp;&nbsp;&nbsp; "
        "<span style='color:#e09b3d'>●</span> En cours <b>%2</b> &nbsp;&nbsp;&nbsp; "
        "<span style='color:#c0504d'>●</span> À nettoyer <b>%3</b>").arg(clean).arg(prog).arg(dirty));
    legend->setTextFormat(Qt::RichText);
    root->addWidget(legend);

    // resume en phrase
    QString busiest = QStringLiteral("aucun local n'est occupé pour le moment");
    if (!rows.isEmpty() && rows.first().pct() > 0)
        busiest = QStringLiteral("le plus rempli : <b>%1</b> (%2 %)").arg(rows.first().type).arg(rows.first().pct());
    auto *summary = new QLabel(QStringLiteral(
        "<b>%1</b> local(aux) occupé(s) sur %2  ·  <b>%3</b> place(s) libre(s)  ·  %4")
        .arg(busy).arg(total).arg(cap - occ).arg(busiest));
    summary->setTextFormat(Qt::RichText);
    summary->setWordWrap(true);
    summary->setStyleSheet(QStringLiteral(
        "background:#eef6f8;border:1px solid #e1ebee;border-radius:12px;padding:12px 16px;color:#1d3b44;"));
    root->addWidget(summary);

    // bouton fermer
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    auto *close = new QPushButton(QStringLiteral("Fermer"));
    close->setProperty("role", "primary");
    close->setCursor(Qt::PointingHandCursor);
    close->setMinimumWidth(150);
    HoverGlow::attach(close);
    connect(close, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(close);
    root->addLayout(btnRow);

    // ------------------------------------------------------------ animation d'ouverture
    auto apply = [counters, bars, cleanBar](qreal v) {
        for (const Counter &c : counters)
            c.label->setText(QString::number(qRound(c.target * v)) + c.suffix);
        for (RateBar *b : bars)
            b->setProgress(v);
        cleanBar->setProgress(v);
    };
    if (Theme::animations) {
        apply(0.0);
        auto *a = new QVariantAnimation(this);
        a->setDuration(900);
        a->setStartValue(0.0);
        a->setEndValue(1.0);
        a->setEasingCurve(QEasingCurve::OutCubic);
        connect(a, &QVariantAnimation::valueChanged, this,
                [apply](const QVariant &v) { apply(v.toReal()); });
        a->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        apply(1.0);
    }
}
