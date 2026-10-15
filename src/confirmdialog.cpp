#include "confirmdialog.h"
#include "hoverglow.h"
#include "icons.h"
#include "theme.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace Confirm {

bool ask(QWidget *parent, const QString &title, const QString &text,
         const QString &okText, bool danger)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(QStringLiteral("ANIWORLD"));
    dlg.setFixedWidth(460);

    auto *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(26, 24, 26, 20);
    root->setSpacing(18);

    auto *row = new QHBoxLayout;
    row->setSpacing(16);
    auto *ic = new QLabel;
    ic->setFixedSize(52, 52);
    ic->setAlignment(Qt::AlignCenter);
    const QColor fg = danger ? Theme::Red : Theme::Teal;
    const QColor bg = danger ? Theme::RedLight : Theme::TealXL;
    ic->setPixmap(Icons::pixmap(danger ? "trash" : "sparkle", fg, 26));
    ic->setStyleSheet(QStringLiteral("background:%1;border-radius:26px;").arg(bg.name()));

    auto *col = new QVBoxLayout;
    col->setSpacing(6);
    auto *t = new QLabel(title);
    t->setObjectName("statsTitle");
    t->setStyleSheet(QStringLiteral("font-size:18px;"));
    auto *m = new QLabel(text);
    m->setWordWrap(true);
    m->setObjectName("sub");
    col->addWidget(t);
    col->addWidget(m);
    row->addWidget(ic, 0, Qt::AlignTop);
    row->addLayout(col, 1);
    root->addLayout(row);

    auto *btns = new QHBoxLayout;
    btns->setSpacing(10);
    btns->addStretch();
    auto *cancel = new QPushButton(QStringLiteral("Annuler"));
    cancel->setProperty("role", "ghost");
    cancel->setCursor(Qt::PointingHandCursor);
    auto *ok = new QPushButton(okText);
    ok->setProperty("role", danger ? "danger" : "primary");
    ok->setCursor(Qt::PointingHandCursor);
    ok->setDefault(true);
    HoverGlow::attach(cancel);
    HoverGlow::attach(ok);
    btns->addWidget(cancel);
    btns->addWidget(ok);
    root->addLayout(btns);

    QObject::connect(cancel, &QPushButton::clicked, &dlg, &QDialog::reject);
    QObject::connect(ok, &QPushButton::clicked, &dlg, &QDialog::accept);
    return dlg.exec() == QDialog::Accepted;
}

} // namespace Confirm
