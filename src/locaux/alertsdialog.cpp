#include "../widgets/animatedbutton.h"
#include "alertsdialog.h"
#include "alerts.h"
#include "hoverglow.h"
#include "icons.h"
#include "theme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

AlertsDialog::AlertsDialog(const QList<Local> &locaux, QWidget *parent)
    : QDialog(parent), m_locaux(locaux)
{
    setObjectName("statsDialog");
    setWindowTitle(QStringLiteral("Alertes automatiques"));
    resize(820, 680);
    setMinimumSize(660, 500);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 20);
    root->setSpacing(14);

    // titre
    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(12);
    auto *ic = new QLabel;
    ic->setPixmap(Icons::pixmap("bell", Theme::Teal, 34));
    auto *col = new QVBoxLayout;
    col->setSpacing(0);
    auto *title = new QLabel(QStringLiteral("Alertes automatiques"));
    title->setObjectName("statsTitle");
    auto *sub = new QLabel(QStringLiteral("ANIWORLD surveille les locaux en continu et te prévient dès qu'un problème apparaît"));
    sub->setObjectName("sub");
    col->addWidget(title);
    col->addWidget(sub);
    titleRow->addWidget(ic);
    titleRow->addLayout(col);
    titleRow->addStretch();
    root->addLayout(titleRow);

    // reglage du seuil
    auto *setRow = new QHBoxLayout;
    setRow->setSpacing(10);
    auto *lbl = new QLabel(QStringLiteral("Un local est « sale trop longtemps » après"));
    m_threshold = new QSpinBox;
    m_threshold->setRange(1, 600);
    m_threshold->setValue(AlertEngine::dirtyThresholdMinutes());
    m_threshold->setSuffix(QStringLiteral(" min"));
    auto *lbl2 = new QLabel(QStringLiteral("(moitié moins pour les blocs et l'isolement)"));
    lbl2->setObjectName("sub");
    setRow->addWidget(lbl);
    setRow->addWidget(m_threshold);
    setRow->addWidget(lbl2);
    setRow->addStretch();
    root->addLayout(setRow);

    m_summary = new QLabel;
    m_summary->setTextFormat(Qt::RichText);
    root->addWidget(m_summary);

    // liste
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *content = new QWidget;
    m_list = new QVBoxLayout(content);
    m_list->setContentsMargins(0, 0, 8, 0);
    m_list->setSpacing(10);
    scroll->setWidget(content);
    root->addWidget(scroll, 1);

    // fermer
    auto *btns = new QHBoxLayout;
    btns->addStretch();
    auto *close = new AnimatedButton(QStringLiteral("Fermer"));
    close->setProperty("role", "primary");
    close->setCursor(Qt::PointingHandCursor);
    close->setMinimumWidth(150);
    HoverGlow::attach(close);
    btns->addWidget(close);
    root->addLayout(btns);

    connect(close, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_threshold, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v) {
        AlertEngine::setDirtyThresholdMinutes(v);
        rebuild();
    });

    rebuild();
}

void AlertsDialog::rebuild()
{
    while (QLayoutItem *it = m_list->takeAt(0)) {
        if (QWidget *w = it->widget()) {
            w->hide();
            w->deleteLater();
        }
        delete it;
    }

    const QList<Alert> alerts = AlertEngine::evaluate(m_locaux);
    int crit = 0, warn = 0, info = 0;
    for (const Alert &a : alerts) {
        if (a.level == Alert::Critical) ++crit;
        else if (a.level == Alert::Warning) ++warn;
        else ++info;
    }
    m_summary->setText(QStringLiteral(
        "<span style='color:#c0504d'>●</span> <b>%1</b> critique(s) &nbsp;&nbsp; "
        "<span style='color:#b97712'>●</span> <b>%2</b> attention &nbsp;&nbsp; "
        "<span style='color:#4a63b8'>●</span> <b>%3</b> information(s)").arg(crit).arg(warn).arg(info));

    if (alerts.isEmpty()) {
        auto *f = new QFrame;
        f->setObjectName("alertCard");
        f->setStyleSheet(QStringLiteral("#alertCard{background:#e3f3e9;border-left:5px solid #2e8b57;border-radius:10px;}"));
        auto *h = new QHBoxLayout(f);
        h->setContentsMargins(18, 16, 18, 16);
        h->setSpacing(14);
        auto *ic = new QLabel;
        ic->setPixmap(Icons::pixmap("check", Theme::Green, 28));
        auto *t = new QLabel(QStringLiteral("<b>Tout va bien</b><br>Aucune alerte pour le moment : aucun local sale trop longtemps, aucun local saturé."));
        t->setTextFormat(Qt::RichText);
        t->setWordWrap(true);
        t->setStyleSheet(QStringLiteral("color:#2e8b57;"));
        h->addWidget(ic);
        h->addWidget(t, 1);
        m_list->addWidget(f);
    }

    for (const Alert &a : alerts) {
        QColor bg, fg;
        QString tag;
        switch (a.level) {
        case Alert::Critical: bg = Theme::RedLight;    fg = Theme::Red;        tag = QStringLiteral("CRITIQUE");  break;
        case Alert::Warning:  bg = Theme::OrangeLight; fg = Theme::OrangeText; tag = QStringLiteral("ATTENTION"); break;
        case Alert::Info:     bg = Theme::BlueLight;   fg = Theme::Blue;       tag = QStringLiteral("INFO");      break;
        }
        auto *f = new QFrame;
        f->setObjectName("alertCard");
        f->setStyleSheet(QStringLiteral("#alertCard{background:%1;border-left:5px solid %2;border-radius:10px;}")
                             .arg(bg.name(), fg.name()));
        auto *h = new QHBoxLayout(f);
        h->setContentsMargins(16, 12, 16, 12);
        h->setSpacing(14);

        auto *ic = new QLabel;
        ic->setPixmap(Icons::pixmap("alert", fg, 26));
        h->addWidget(ic, 0, Qt::AlignTop);

        auto *col = new QVBoxLayout;
        col->setSpacing(2);
        auto *tg = new QLabel(tag);
        tg->setStyleSheet(QStringLiteral("color:%1;font-size:10px;font-weight:700;").arg(fg.name()));
        auto *t = new QLabel(a.title);
        t->setStyleSheet(QStringLiteral("color:%1;font-size:14px;font-weight:700;").arg(fg.darker(120).name()));
        t->setWordWrap(true);
        auto *d = new QLabel(a.detail);
        d->setStyleSheet(QStringLiteral("color:#4a6770;font-size:12px;"));
        d->setWordWrap(true);
        col->addWidget(tg);
        col->addWidget(t);
        col->addWidget(d);
        h->addLayout(col, 1);

        if (a.localId > 0) {
            auto *go = new AnimatedButton(QStringLiteral("Voir le local"));
            go->setProperty("role", "ghost");
            go->setCursor(Qt::PointingHandCursor);
            HoverGlow::attach(go);
            const int id = a.localId;
            connect(go, &QPushButton::clicked, this, [this, id] {
                m_selected = id;
                accept();
            });
            h->addWidget(go, 0, Qt::AlignVCenter);
        }
        m_list->addWidget(f);
    }
    m_list->addStretch();
}
