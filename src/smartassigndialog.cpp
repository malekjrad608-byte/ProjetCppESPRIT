#include "smartassigndialog.h"
#include "hoverglow.h"
#include "icons.h"
#include "localrepository.h"
#include "smartassign.h"
#include "theme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVariantAnimation>

namespace {

// barre "Compatibilité xx %"
class ScoreBar : public QWidget
{
public:
    explicit ScoreBar(int score, QWidget *parent = nullptr) : QWidget(parent), m_score(score)
    {
        setFixedHeight(22);
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
        const QColor c = m_score >= 75 ? QColor(0x4a, 0xa5, 0x7a)
                         : m_score >= 50 ? QColor(0xe0, 0x9b, 0x3d)
                                         : Theme::Red;
        const QRectF track(0, height() / 2.0 - 6, width() - 130, 12);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0xee, 0xf3, 0xf5));
        p.drawRoundedRect(track, 6, 6);
        const qreal w = track.width() * m_score / 100.0 * m_p;
        if (w >= 1.0) {
            p.setBrush(c);
            p.drawRoundedRect(QRectF(track.left(), track.top(), w, track.height()), 6, 6);
        }
        p.setPen(c);
        p.setFont(Theme::font(9.5, true));
        p.drawText(QRectF(width() - 122, 0, 122, height()), Qt::AlignVCenter | Qt::AlignRight,
                   QStringLiteral("Compatibilité %1 %").arg(qRound(m_score * m_p)));
    }

private:
    int m_score;
    qreal m_p = 1.0;
};

} // namespace

SmartAssignDialog::SmartAssignDialog(LocalRepository *repo, QWidget *parent)
    : QDialog(parent), m_repo(repo)
{
    setObjectName("statsDialog");
    setWindowTitle(QStringLiteral("Affectation intelligente"));
    resize(820, 700);
    setMinimumSize(700, 560);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 20);
    root->setSpacing(14);

    // titre
    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(12);
    auto *ic = new QLabel;
    ic->setPixmap(Icons::pixmap("sparkle", Theme::Teal, 34));
    auto *col = new QVBoxLayout;
    col->setSpacing(0);
    auto *title = new QLabel(QStringLiteral("Affectation intelligente"));
    title->setObjectName("statsTitle");
    auto *sub = new QLabel(QStringLiteral("Dis-moi quel animal arrive et de quoi il a besoin : je te propose le meilleur local"));
    sub->setObjectName("sub");
    col->addWidget(title);
    col->addWidget(sub);
    titleRow->addWidget(ic);
    titleRow->addLayout(col);
    titleRow->addStretch();
    root->addLayout(titleRow);

    // criteres
    auto *form = new QHBoxLayout;
    form->setSpacing(14);
    auto addField = [&](const QString &label, QWidget *w) {
        auto *box = new QVBoxLayout;
        box->setSpacing(4);
        auto *l = new QLabel(label);
        l->setObjectName("key");
        box->addWidget(l);
        box->addWidget(w);
        form->addLayout(box, 1);
    };
    m_species = new QComboBox;
    m_species->addItems(SmartAssign::species());
    m_need = new QComboBox;
    m_need->addItems(SmartAssign::needs());
    addField(QStringLiteral("Espèce de l'animal"), m_species);
    addField(QStringLiteral("Besoin"), m_need);
    m_urgent = new QCheckBox(QStringLiteral("Urgence"));
    auto *urgBox = new QVBoxLayout;
    urgBox->addSpacing(22);
    urgBox->addWidget(m_urgent);
    form->addLayout(urgBox);
    root->addLayout(form);

    // resultats
    auto *content = new QWidget;
    m_results = new QVBoxLayout(content);
    m_results->setContentsMargins(0, 0, 0, 0);
    m_results->setSpacing(12);
    root->addWidget(content, 1);

    // fermer
    auto *btns = new QHBoxLayout;
    btns->addStretch();
    auto *close = new QPushButton(QStringLiteral("Fermer"));
    close->setProperty("role", "ghost");
    close->setCursor(Qt::PointingHandCursor);
    close->setMinimumWidth(140);
    HoverGlow::attach(close);
    btns->addWidget(close);
    root->addLayout(btns);

    connect(close, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_species, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { rebuild(); });
    connect(m_need, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { rebuild(); });
    connect(m_urgent, &QCheckBox::toggled, this, [this] { rebuild(); });

    rebuild();
}

void SmartAssignDialog::rebuild()
{
    if (m_anim) {
        m_anim->stop();
        m_anim->deleteLater();
        m_anim = nullptr;
    }
    while (QLayoutItem *it = m_results->takeAt(0)) {
        if (QWidget *w = it->widget()) {
            w->hide();
            w->deleteLater();
        }
        delete it;
    }

    AssignQuery q;
    q.species = m_species->currentText();
    q.need = m_need->currentText();
    q.urgent = m_urgent->isChecked();
    const QList<AssignSuggestion> list = SmartAssign::suggest(m_repo->all(), q, 3);

    QList<ScoreBar *> bars;

    if (list.isEmpty()) {
        auto *f = new QFrame;
        f->setObjectName("suggestCard");
        f->setStyleSheet(QStringLiteral("#suggestCard{background:#fbe9e8;border-left:5px solid #c0504d;border-radius:10px;}"));
        auto *h = new QHBoxLayout(f);
        h->setContentsMargins(18, 16, 18, 16);
        h->setSpacing(14);
        auto *ic = new QLabel;
        ic->setPixmap(Icons::pixmap("alert", Theme::Red, 28));
        auto *t = new QLabel(QStringLiteral("<b>Aucun local disponible</b><br>%1").arg(SmartAssign::explainNone(m_repo->all(), q)));
        t->setTextFormat(Qt::RichText);
        t->setWordWrap(true);
        t->setStyleSheet(QStringLiteral("color:#c0504d;"));
        h->addWidget(ic);
        h->addWidget(t, 1);
        m_results->addWidget(f);
    }

    int rank = 0;
    for (const AssignSuggestion &s : list) {
        ++rank;
        const bool best = (rank == 1);
        auto *f = new QFrame;
        f->setObjectName("suggestCard");
        f->setStyleSheet(best
            ? QStringLiteral("#suggestCard{background:white;border:2px solid #2f7482;border-radius:14px;}")
            : QStringLiteral("#suggestCard{background:white;border:1px solid #e1ebee;border-radius:14px;}"));
        auto *h = new QHBoxLayout(f);
        h->setContentsMargins(18, 14, 18, 14);
        h->setSpacing(16);

        auto *badge = new QLabel(QString::number(rank));
        badge->setFixedSize(42, 42);
        badge->setAlignment(Qt::AlignCenter);
        badge->setStyleSheet(best
            ? QStringLiteral("background:#2f7482;color:white;border-radius:21px;font-size:18px;font-weight:700;")
            : QStringLiteral("background:#eef6f8;color:#2f7482;border-radius:21px;font-size:18px;font-weight:700;"));
        h->addWidget(badge, 0, Qt::AlignTop);

        auto *col = new QVBoxLayout;
        col->setSpacing(5);

        auto *nameRow = new QHBoxLayout;
        nameRow->setSpacing(10);
        auto *name = new QLabel(s.name);
        name->setStyleSheet(QStringLiteral("font-size:16px;font-weight:700;color:#1f5560;"));
        nameRow->addWidget(name);
        if (best) {
            auto *tag = new QLabel(QStringLiteral("RECOMMANDÉ"));
            tag->setStyleSheet(QStringLiteral("background:#e3f3e9;color:#2e8b57;border-radius:8px;padding:2px 10px;font-size:10px;font-weight:700;"));
            nameRow->addWidget(tag);
        }
        nameRow->addStretch();
        col->addLayout(nameRow);

        auto *where = new QLabel(QStringLiteral("%1  ·  %2").arg(s.type, s.floorText));
        where->setObjectName("sub");
        col->addWidget(where);

        auto *bar = new ScoreBar(s.score);
        bars.append(bar);
        col->addWidget(bar);

        QString chips;
        for (const QString &r : s.reasons)
            chips += QStringLiteral("<span style=\"background-color:#e3f3e9; color:#2e8b57;\">&nbsp;+ %1&nbsp;</span>&nbsp; ").arg(r.toHtmlEscaped());
        for (const QString &w : s.warnings)
            chips += QStringLiteral("<span style=\"background-color:#fff4e2; color:#b97712;\">&nbsp;! %1&nbsp;</span>&nbsp; ").arg(w.toHtmlEscaped());
        auto *why = new QLabel(chips);
        why->setTextFormat(Qt::RichText);
        why->setWordWrap(true);
        col->addWidget(why);
        h->addLayout(col, 1);

        auto *go = new QPushButton(QStringLiteral("Affecter ici"));
        go->setProperty("role", best ? "primary" : "ghost");
        go->setCursor(Qt::PointingHandCursor);
        HoverGlow::attach(go);
        const int id = s.localId;
        connect(go, &QPushButton::clicked, this, [this, id] {
            const Local *cur = m_repo->find(id);
            if (!cur || cur->outOfService || cur->occupancy >= cur->capacity)
                return;
            Local l = *cur;
            l.occupancy++;
            m_assignedId = id;
            m_assignedName = l.name;
            m_repo->update(l);
            accept();
        });
        h->addWidget(go, 0, Qt::AlignVCenter);

        m_results->addWidget(f);
    }
    m_results->addStretch();

    // les barres se remplissent a chaque nouvelle proposition
    auto apply = [bars](qreal v) {
        for (ScoreBar *b : bars)
            b->setProgress(v);
    };
    if (Theme::animations && !bars.isEmpty()) {
        apply(0.0);
        m_anim = new QVariantAnimation(this);
        m_anim->setDuration(700);
        m_anim->setStartValue(0.0);
        m_anim->setEndValue(1.0);
        m_anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(m_anim, &QVariantAnimation::valueChanged, this,
                [apply](const QVariant &v) { apply(v.toReal()); });
        m_anim->start();
    } else {
        apply(1.0);
    }
}
