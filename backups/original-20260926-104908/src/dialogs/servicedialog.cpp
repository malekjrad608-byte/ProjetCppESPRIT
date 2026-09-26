#include "servicedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFont>

ServiceDialog::ServiceDialog(const QList<Employee> &employees, QWidget *parent)
    : QDialog(parent), m_employees(employees) {
    setupUi();
    populateEmployees(employees);
    setWindowTitle("Ajouter un Service");
    m_dateEdit->setDateTime(QDateTime::currentDateTime());
}

ServiceDialog::ServiceDialog(const Service &service, const QList<Employee> &employees, QWidget *parent)
    : QDialog(parent), m_employees(employees), m_serviceId(service.id) {
    setupUi();
    populateEmployees(employees);
    loadService(service);
    setWindowTitle("Modifier le Service");
}

void ServiceDialog::setupUi() {
    setMinimumWidth(480);
    applyStyle();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // Title
    auto *titleLabel = new QLabel(windowTitle(), this);
    QFont f = titleLabel->font();
    f.setPointSize(14);
    f.setBold(true);
    titleLabel->setFont(f);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);

    // Form
    auto *formGroup = new QGroupBox(this);
    formGroup->setObjectName("formGroup");
    auto *form = new QFormLayout(formGroup);
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Nom du service...");

    m_descEdit = new QTextEdit(this);
    m_descEdit->setPlaceholderText("Description du service...");
    m_descEdit->setMaximumHeight(90);

    m_employeeBox = new QComboBox(this);
    m_employeeBox->addItem("-- Aucun --", 0);

    m_dateEdit = new QDateTimeEdit(this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd/MM/yyyy HH:mm");

    m_statusBox = new QComboBox(this);
    m_statusBox->addItems({"Planifie", "En cours", "Termine", "Annule"});

    form->addRow("Nom *", m_nameEdit);
    form->addRow("Description", m_descEdit);
    form->addRow("Responsable", m_employeeBox);
    form->addRow("Date / Heure", m_dateEdit);
    form->addRow("Statut", m_statusBox);

    mainLayout->addWidget(formGroup);

    // Buttons
    auto *btnLayout = new QHBoxLayout;
    m_cancelBtn = new QPushButton("Annuler", this);
    m_saveBtn   = new QPushButton("Enregistrer", this);
    m_saveBtn->setObjectName("saveBtn");
    m_cancelBtn->setObjectName("cancelBtn");
    btnLayout->addStretch();
    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_saveBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_saveBtn,   &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void ServiceDialog::applyStyle() {
    setStyleSheet(R"(
        QDialog {
            background: #1e1e2e;
            color: #cdd6f4;
        }
        QLabel { color: #cdd6f4; }
        QLabel#titleLabel { color: #89b4fa; font-size: 15px; }
        QGroupBox#formGroup {
            border: 1px solid #313244;
            border-radius: 8px;
            padding: 12px;
            background: #181825;
        }
        QLineEdit, QTextEdit, QComboBox, QDateTimeEdit {
            background: #313244;
            border: 1px solid #45475a;
            border-radius: 6px;
            padding: 6px 10px;
            color: #cdd6f4;
            font-size: 13px;
        }
        QLineEdit:focus, QTextEdit:focus, QComboBox:focus, QDateTimeEdit:focus {
            border: 1px solid #89b4fa;
        }
        QPushButton#saveBtn {
            background: #89b4fa;
            color: #1e1e2e;
            border-radius: 6px;
            padding: 8px 24px;
            font-weight: bold;
        }
        QPushButton#saveBtn:hover { background: #b4befe; }
        QPushButton#cancelBtn {
            background: #313244;
            color: #cdd6f4;
            border-radius: 6px;
            padding: 8px 24px;
        }
        QPushButton#cancelBtn:hover { background: #45475a; }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background: #313244;
            color: #cdd6f4;
            selection-background-color: #89b4fa;
            selection-color: #1e1e2e;
        }
    )");
}

void ServiceDialog::populateEmployees(const QList<Employee> &employees) {
    for (const auto &e : employees)
        m_employeeBox->addItem(e.fullName(), e.id);
}

void ServiceDialog::loadService(const Service &s) {
    m_nameEdit->setText(s.name);
    m_descEdit->setPlainText(s.description);
    m_dateEdit->setDateTime(s.dateTime);
    int idx = m_statusBox->findText(s.status);
    if (idx >= 0) m_statusBox->setCurrentIndex(idx);
    for (int i = 0; i < m_employeeBox->count(); ++i) {
        if (m_employeeBox->itemData(i).toInt() == s.employeeId) {
            m_employeeBox->setCurrentIndex(i);
            break;
        }
    }
}

Service ServiceDialog::getService() const {
    Service s;
    s.id          = m_serviceId;
    s.name        = m_nameEdit->text().trimmed();
    s.description = m_descEdit->toPlainText().trimmed();
    s.employeeId  = m_employeeBox->currentData().toInt();
    s.responsible = m_employeeBox->currentText();
    s.dateTime    = m_dateEdit->dateTime();
    s.status      = m_statusBox->currentText();
    return s;
}
