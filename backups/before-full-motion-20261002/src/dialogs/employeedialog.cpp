#include "employeedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFont>

EmployeeDialog::EmployeeDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Ajouter un Employe");
    setupUi();
}

EmployeeDialog::EmployeeDialog(const Employee &emp, QWidget *parent) : QDialog(parent), m_employeeId(emp.id) {
    setWindowTitle("Modifier l'Employe");
    setupUi();
    loadEmployee(emp);
}

void EmployeeDialog::setupUi() {
    setMinimumWidth(420);
    applyStyle();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    auto *titleLabel = new QLabel(windowTitle(), this);
    QFont f = titleLabel->font(); f.setPointSize(14); f.setBold(true);
    titleLabel->setFont(f); titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);

    auto *grp = new QGroupBox(this); grp->setObjectName("formGroup");
    auto *form = new QFormLayout(grp); form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight);

    m_firstNameEdit = new QLineEdit(this); m_firstNameEdit->setPlaceholderText("Prenom...");
    m_lastNameEdit  = new QLineEdit(this); m_lastNameEdit->setPlaceholderText("Nom...");
    m_posteEdit     = new QLineEdit(this); m_posteEdit->setPlaceholderText("Poste...");
    m_phoneEdit     = new QLineEdit(this); m_phoneEdit->setPlaceholderText("Telephone...");
    m_emailEdit     = new QLineEdit(this); m_emailEdit->setPlaceholderText("Email...");

    form->addRow("Prenom *",    m_firstNameEdit);
    form->addRow("Nom *",       m_lastNameEdit);
    form->addRow("Poste",       m_posteEdit);
    form->addRow("Telephone",   m_phoneEdit);
    form->addRow("Email",       m_emailEdit);
    mainLayout->addWidget(grp);

    auto *btnLayout = new QHBoxLayout;
    m_cancelBtn = new QPushButton("Annuler", this); m_cancelBtn->setObjectName("cancelBtn");
    m_saveBtn   = new QPushButton("Enregistrer", this); m_saveBtn->setObjectName("saveBtn");
    btnLayout->addStretch();
    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_saveBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_saveBtn,   &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void EmployeeDialog::applyStyle() {
    setStyleSheet(R"(
        QDialog { background: #1e1e2e; color: #cdd6f4; }
        QLabel { color: #cdd6f4; }
        QLabel#titleLabel { color: #89b4fa; font-size: 15px; }
        QGroupBox#formGroup { border: 1px solid #313244; border-radius: 8px; padding: 12px; background: #181825; }
        QLineEdit { background: #313244; border: 1px solid #45475a; border-radius: 6px; padding: 6px 10px; color: #cdd6f4; font-size: 13px; }
        QLineEdit:focus { border: 1px solid #89b4fa; }
        QPushButton#saveBtn { background: #a6e3a1; color: #1e1e2e; border-radius: 6px; padding: 8px 24px; font-weight: bold; }
        QPushButton#saveBtn:hover { background: #cdd6f4; }
        QPushButton#cancelBtn { background: #313244; color: #cdd6f4; border-radius: 6px; padding: 8px 24px; }
        QPushButton#cancelBtn:hover { background: #45475a; }
    )");
}

void EmployeeDialog::loadEmployee(const Employee &e) {
    m_firstNameEdit->setText(e.firstName);
    m_lastNameEdit->setText(e.lastName);
    m_posteEdit->setText(e.poste);
    m_phoneEdit->setText(e.phone);
    m_emailEdit->setText(e.email);
}

Employee EmployeeDialog::getEmployee() const {
    Employee e;
    e.id        = m_employeeId;
    e.firstName = m_firstNameEdit->text().trimmed();
    e.lastName  = m_lastNameEdit->text().trimmed();
    e.poste     = m_posteEdit->text().trimmed();
    e.phone     = m_phoneEdit->text().trimmed();
    e.email     = m_emailEdit->text().trimmed();
    return e;
}
