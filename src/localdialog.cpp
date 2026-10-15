#include "localdialog.h"
#include "hoverglow.h"
#include "icons.h"
#include "localrepository.h"
#include "theme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {

QLabel *makeError()
{
    auto *e = new QLabel;
    e->setStyleSheet(QStringLiteral("color:#c0504d;font-size:11px;"));
    e->setWordWrap(true);
    e->hide();
    return e;
}

void showError(QLabel *e, const QString &msg)
{
    e->setText(msg);
    e->setVisible(!msg.isEmpty());
}

QWidget *field(const QString &label, QWidget *w, QLabel *err = nullptr)
{
    auto *box = new QWidget;
    auto *v = new QVBoxLayout(box);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(4);
    auto *l = new QLabel(label);
    l->setObjectName("key");
    v->addWidget(l);
    v->addWidget(w);
    if (err)
        v->addWidget(err);
    return box;
}

} // namespace

LocalDialog::LocalDialog(const Local *existing, LocalRepository *repo, QWidget *parent)
    : QDialog(parent), m_repo(repo), m_editing(existing != nullptr)
{
    setWindowTitle(m_editing ? QStringLiteral("Modifier le local") : QStringLiteral("Ajouter un local"));
    setFixedWidth(540);

    if (existing) {
        m_base = *existing;
    } else {
        m_base.lastCleaned = QDateTime::currentDateTime();
    }

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 22);
    root->setSpacing(14);

    // titre
    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(12);
    auto *ic = new QLabel;
    ic->setPixmap(Icons::pixmap(m_editing ? "edit" : "plus", Theme::Teal, 28));
    auto *col = new QVBoxLayout;
    col->setSpacing(0);
    auto *title = new QLabel(m_editing ? QStringLiteral("Modifier le local") : QStringLiteral("Ajouter un local"));
    title->setObjectName("statsTitle");
    auto *sub = new QLabel(m_editing ? QStringLiteral("Change les informations puis enregistre.")
                                     : QStringLiteral("Renseigne les informations du nouveau local."));
    sub->setObjectName("sub");
    col->addWidget(title);
    col->addWidget(sub);
    titleRow->addWidget(ic);
    titleRow->addLayout(col);
    titleRow->addStretch();
    root->addLayout(titleRow);

    // nom
    m_name = new QLineEdit(m_base.name);
    m_name->setMaxLength(30);
    m_name->setPlaceholderText(QStringLiteral("ex. Consultation 6, Bloc C…"));
    m_nameErr = makeError();
    root->addWidget(field(QStringLiteral("Nom du local *"), m_name, m_nameErr));
    connect(m_name, &QLineEdit::textChanged, this, [this] { showError(m_nameErr, QString()); });

    // type + etage
    auto *row1 = new QHBoxLayout;
    row1->setSpacing(14);
    m_type = new QComboBox;
    m_type->addItems(Local::types());
    if (m_editing)
        m_type->setCurrentText(m_base.type);
    m_floor = new QComboBox;
    m_floor->addItems({ "Rez-de-chaussée", "Étage 1", "Étage 2", "Étage 3" });
    m_floor->setCurrentIndex(qBound(0, m_base.floor, 3));
    row1->addWidget(field(QStringLiteral("Type"), m_type), 1);
    row1->addWidget(field(QStringLiteral("Étage"), m_floor), 1);
    root->addLayout(row1);

    // capacite (+ occupation en modification)
    auto *row2 = new QHBoxLayout;
    row2->setSpacing(14);
    m_capacity = new QSpinBox;
    m_capacity->setRange(1, 50);
    m_capacity->setValue(qBound(1, m_base.capacity, 50));
    m_capacity->setSuffix(QStringLiteral(" place(s)"));
    row2->addWidget(field(QStringLiteral("Capacité *"), m_capacity), 1);
    if (m_editing) {
        m_occupancy = new QSpinBox;
        m_occupancy->setRange(0, m_capacity->value());
        m_occupancy->setValue(qMin(m_base.occupancy, m_capacity->value()));
        m_occupancy->setSuffix(QStringLiteral(" animal(aux)"));
        row2->addWidget(field(QStringLiteral("Occupation actuelle"), m_occupancy), 1);
        connect(m_capacity, QOverload<int>::of(&QSpinBox::valueChanged), this,
                [this](int v) { m_occupancy->setMaximum(v); });
    }
    root->addLayout(row2);

    // specialite
    m_specialty = new QLineEdit(m_base.specialty);
    m_specialty->setMaxLength(30);
    m_specialty->setPlaceholderText(QStringLiteral("ex. Généraliste, Dentiste, Aseptique, Chiens…"));
    root->addWidget(field(QStringLiteral("Spécialité / remarque"), m_specialty));

    // hors service
    m_oos = new QCheckBox(QStringLiteral("Hors service (travaux, panne, désinfection…)"));
    m_oos->setChecked(m_base.outOfService);
    m_oosErr = makeError();
    root->addWidget(m_oos);
    root->addWidget(m_oosErr);
    connect(m_oos, &QCheckBox::toggled, this, [this] { showError(m_oosErr, QString()); });

    // boutons
    auto *btns = new QHBoxLayout;
    btns->setSpacing(10);
    btns->addStretch();
    auto *cancel = new QPushButton(QStringLiteral("Annuler"));
    cancel->setProperty("role", "ghost");
    cancel->setCursor(Qt::PointingHandCursor);
    auto *save = new QPushButton(QStringLiteral("Enregistrer"));
    save->setProperty("role", "primary");
    save->setCursor(Qt::PointingHandCursor);
    save->setDefault(true);
    HoverGlow::attach(cancel);
    HoverGlow::attach(save);
    btns->addWidget(cancel);
    btns->addWidget(save);
    root->addLayout(btns);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(save, &QPushButton::clicked, this, &LocalDialog::accept);
}

void LocalDialog::accept()
{
    const QString name = m_name->text().trimmed();
    const int occupancy = m_occupancy ? m_occupancy->value() : m_base.occupancy;
    bool ok = true;

    if (name.size() < 2) {
        showError(m_nameErr, QStringLiteral("Le nom est obligatoire (2 caractères minimum)."));
        ok = false;
    } else if (m_repo->nameExists(name, m_base.id)) {
        showError(m_nameErr, QStringLiteral("Un local porte déjà ce nom."));
        ok = false;
    }

    if (m_oos->isChecked() && occupancy > 0) {
        showError(m_oosErr, QStringLiteral("Libère d'abord les animaux présents avant de mettre le local hors service."));
        ok = false;
    }
    if (!ok)
        return;

    m_result = m_base;
    m_result.name = name;
    m_result.type = m_type->currentText();
    m_result.floor = m_floor->currentIndex();
    m_result.capacity = m_capacity->value();
    m_result.occupancy = qMin(occupancy, m_result.capacity);
    m_result.specialty = m_specialty->text().trimmed();
    m_result.outOfService = m_oos->isChecked();
    QDialog::accept();
}
