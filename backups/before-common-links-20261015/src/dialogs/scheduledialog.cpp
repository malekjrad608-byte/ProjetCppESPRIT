#include "scheduledialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFont>

ScheduleDialog::ScheduleDialog(const QList<Employee> &employees, const QDate &preselectedDate, QWidget *parent)
    : QDialog(parent), m_employees(employees) {
    setWindowTitle("Ajouter un Quart");
    setupUi();
    populateEmployees(employees);
    if (preselectedDate.isValid())
        m_dateEdit->setDate(preselectedDate);
}

ScheduleDialog::ScheduleDialog(const Schedule &s, const QList<Employee> &employees, QWidget *parent)
    : QDialog(parent), m_employees(employees), m_scheduleId(s.id) {
    setWindowTitle("Modifier le Quart");
    setupUi();
    populateEmployees(employees);
    loadSchedule(s);
}

void ScheduleDialog::setupUi() {
    setMinimumWidth(400);
    applyStyle();

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24,24,24,24);

    auto *titleLabel = new QLabel(windowTitle(), this);
    QFont f = titleLabel->font(); f.setPointSize(14); f.setBold(true);
    titleLabel->setFont(f); titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("titleLabel");
    mainLayout->addWidget(titleLabel);

    auto *grp = new QGroupBox(this); grp->setObjectName("formGroup");
    auto *form = new QFormLayout(grp); form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight);

    m_employeeBox = new QComboBox(this);
    m_dateEdit    = new QDateEdit(QDate::currentDate(), this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd/MM/yyyy");
    m_shiftBox    = new QComboBox(this);
    m_shiftBox->addItems({"Matin", "Apres-midi", "Nuit"});
    m_noteEdit    = new QTextEdit(this);
    m_noteEdit->setPlaceholderText("Note...");
    m_noteEdit->setMaximumHeight(70);

    form->addRow("Employe *", m_employeeBox);
    form->addRow("Date *",    m_dateEdit);
    form->addRow("Quart *",   m_shiftBox);
    form->addRow("Note",      m_noteEdit);
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

void ScheduleDialog::applyStyle() {
    setStyleSheet(R"(
        QDialog { background: #1e1e2e; color: #cdd6f4; }
        QLabel { color: #cdd6f4; }
        QLabel#titleLabel { color: #cba6f7; font-size: 15px; }
        QGroupBox#formGroup { border: 1px solid #313244; border-radius: 8px; padding: 12px; background: #181825; }
        QComboBox, QDateEdit, QTextEdit {
            background: #313244; border: 1px solid #45475a; border-radius: 6px;
            padding: 6px 10px; color: #cdd6f4; font-size: 13px;
        }
        QComboBox:focus, QDateEdit:focus, QTextEdit:focus { border: 1px solid #cba6f7; }
        QPushButton#saveBtn { background: #cba6f7; color: #1e1e2e; border-radius: 6px; padding: 8px 24px; font-weight: bold; }
        QPushButton#saveBtn:hover { background: #f5c2e7; }
        QPushButton#cancelBtn { background: #313244; color: #cdd6f4; border-radius: 6px; padding: 8px 24px; }
        QPushButton#cancelBtn:hover { background: #45475a; }
        QComboBox QAbstractItemView { background: #313244; color: #cdd6f4; selection-background-color: #cba6f7; selection-color: #1e1e2e; }
    )");
}

void ScheduleDialog::populateEmployees(const QList<Employee> &employees) {
    for (const auto &e : employees)
        m_employeeBox->addItem(e.fullName(), e.id);
}

void ScheduleDialog::loadSchedule(const Schedule &s) {
    for (int i = 0; i < m_employeeBox->count(); ++i)
        if (m_employeeBox->itemData(i).toInt() == s.employeeId)
            { m_employeeBox->setCurrentIndex(i); break; }
    m_dateEdit->setDate(s.date);
    int idx = m_shiftBox->findText(s.shift);
    if (idx >= 0) m_shiftBox->setCurrentIndex(idx);
    m_noteEdit->setPlainText(s.note);
}

Schedule ScheduleDialog::getSchedule() const {
    Schedule s;
    s.id           = m_scheduleId;
    s.employeeId   = m_employeeBox->currentData().toInt();
    s.employeeName = m_employeeBox->currentText();
    s.date         = m_dateEdit->date();
    s.shift        = m_shiftBox->currentText();
    s.note         = m_noteEdit->toPlainText().trimmed();
    return s;
}
