#include "schedulewidget.h"
#include "../database/databasemanager.h"
#include "../dialogs/scheduledialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFont>
#include <QToolButton>

static QDate mondayOf(const QDate &d) {
    int dow = d.dayOfWeek(); // 1=Mon
    return d.addDays(1 - dow);
}

ScheduleWidget::ScheduleWidget(QWidget *parent) : QWidget(parent) {
    m_weekStart = mondayOf(QDate::currentDate());
    setupUi();
    refresh();
}

void ScheduleWidget::setupUi() {
    applyStyle();
    auto *root = new QVBoxLayout(this);
    root->setSpacing(16);
    root->setContentsMargins(20,20,20,20);

    // Title + nav
    auto *nav = new QHBoxLayout;
    auto *titleLbl = new QLabel("Planning Hebdomadaire", this);
    QFont tf = titleLbl->font(); tf.setPointSize(18); tf.setBold(true);
    titleLbl->setFont(tf); titleLbl->setStyleSheet("color: #cba6f7;");
    nav->addWidget(titleLbl);
    nav->addStretch();

    m_prevBtn = new QPushButton("< Semaine prec.", this); m_prevBtn->setObjectName("navBtn");
    m_weekLabel = new QLabel(this); m_weekLabel->setAlignment(Qt::AlignCenter);
    m_weekLabel->setMinimumWidth(200);
    m_weekLabel->setStyleSheet("color:#cdd6f4; font-weight:bold; font-size:14px;");
    m_nextBtn = new QPushButton("Semaine suiv. >", this); m_nextBtn->setObjectName("navBtn");
    nav->addWidget(m_prevBtn);
    nav->addWidget(m_weekLabel);
    nav->addWidget(m_nextBtn);
    root->addLayout(nav);

    // Table: rows=employees, cols=days
    m_table = new QTableWidget(this);
    m_table->setRowCount(0);
    m_table->setColumnCount(7);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_table->verticalHeader()->setDefaultSectionSize(80);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectItems);
    root->addWidget(m_table);

    // Action buttons
    auto *actBar = new QHBoxLayout;
    actBar->addStretch();
    m_addBtn    = new QPushButton("+ Ajouter un quart", this); m_addBtn->setObjectName("addBtn");
    m_deleteBtn = new QPushButton("Supprimer", this);           m_deleteBtn->setObjectName("deleteBtn");
    actBar->addWidget(m_addBtn);
    actBar->addWidget(m_deleteBtn);
    root->addLayout(actBar);

    connect(m_prevBtn, &QPushButton::clicked, this, &ScheduleWidget::onPrevWeek);
    connect(m_nextBtn, &QPushButton::clicked, this, &ScheduleWidget::onNextWeek);
    connect(m_addBtn,  &QPushButton::clicked, this, &ScheduleWidget::onAdd);
    connect(m_deleteBtn, &QPushButton::clicked, this, &ScheduleWidget::onDelete);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &ScheduleWidget::onCellDoubleClicked);
}

void ScheduleWidget::applyStyle() {
    setStyleSheet(R"(
        QWidget { background: #1e1e2e; color: #cdd6f4; font-size: 13px; }
        QTableWidget { background: #181825; border: 1px solid #313244; border-radius: 8px; gridline-color: #313244; }
        QTableWidget::item { padding: 4px; }
        QTableWidget::item:selected { background: #45475a; }
        QHeaderView::section { background: #313244; color: #cba6f7; font-weight: bold; padding: 10px 4px; border: none; border-bottom: 2px solid #cba6f7; }
        QPushButton#addBtn { background: #cba6f7; color: #1e1e2e; border-radius: 6px; padding: 8px 18px; font-weight: bold; }
        QPushButton#addBtn:hover { background: #f5c2e7; }
        QPushButton#deleteBtn { background: #f38ba8; color: #1e1e2e; border-radius: 6px; padding: 8px 18px; font-weight: bold; }
        QPushButton#deleteBtn:hover { background: #eba0ac; }
        QPushButton#navBtn { background: #313244; color: #cdd6f4; border-radius: 6px; padding: 6px 14px; }
        QPushButton#navBtn:hover { background: #45475a; }
    )");
}

void ScheduleWidget::refresh() {
    auto &db = DatabaseManager::instance();
    m_employees = db.getAllEmployees();
    m_schedules = db.getSchedulesForWeek(m_weekStart);
    buildWeekTable();
}

void ScheduleWidget::goToWeek(const QDate &date) {
    m_weekStart = mondayOf(date);
    refresh();
}

void ScheduleWidget::buildWeekTable() {
    // Column headers: day names + dates
    static const char* dayNames[] = {"Lundi","Mardi","Mercredi","Jeudi","Vendredi","Samedi","Dimanche"};
    QStringList headers;
    for (int d = 0; d < 7; ++d)
        headers << QString("%1\n%2").arg(dayNames[d]).arg(m_weekStart.addDays(d).toString("dd/MM"));
    m_table->setHorizontalHeaderLabels(headers);
    m_weekLabel->setText(QString("Sem. du %1 au %2")
        .arg(m_weekStart.toString("dd/MM/yyyy"))
        .arg(m_weekStart.addDays(6).toString("dd/MM/yyyy")));

    // Row headers: employee names
    m_table->setRowCount(m_employees.size());
    QStringList rowHeaders;
    for (const auto &e : m_employees)
        rowHeaders << e.fullName();
    m_table->setVerticalHeaderLabels(rowHeaders);
    m_table->verticalHeader()->setVisible(true);
    m_table->verticalHeader()->setStyleSheet("QHeaderView::section { background:#313244; color:#a6e3a1; font-weight:bold; padding:4px; border: none; border-right: 2px solid #a6e3a1; }");

    // Clear cells
    m_table->clearContents();

    // Fill shifts
    for (const auto &sc : m_schedules) {
        int col = m_weekStart.daysTo(sc.date);
        if (col < 0 || col > 6) continue;
        int row = -1;
        for (int r = 0; r < m_employees.size(); ++r)
            if (m_employees[r].id == sc.employeeId) { row = r; break; }
        if (row < 0) continue;

        auto *it = m_table->item(row, col);
        QString text;
        if (it) text = it->text() + "\n";
        text += shiftBadge(sc.shift, sc.id);
        auto *newItem = new QTableWidgetItem(text);
        newItem->setTextAlignment(Qt::AlignCenter);
        newItem->setData(Qt::UserRole, sc.id);
        newItem->setBackground(shiftBgColor(sc.shift));
        newItem->setForeground(QColor("#1e1e2e"));
        m_table->setItem(row, col, newItem);
    }

    // Highlight today
    int todayCol = m_weekStart.daysTo(QDate::currentDate());
    if (todayCol >= 0 && todayCol <= 6) {
        for (int r = 0; r < m_table->rowCount(); ++r) {
            auto *hdr = m_table->horizontalHeaderItem(todayCol);
            if (hdr) hdr->setForeground(QColor("#f9e2af"));
        }
    }
}

QString ScheduleWidget::shiftBadge(const QString &shift, int id) {
    Q_UNUSED(id)
    if (shift == "Matin")      return "☀ Matin";
    if (shift == "Apres-midi") return "🌤 Après-midi";
    if (shift == "Nuit")       return "🌙 Nuit";
    return shift;
}

QColor ScheduleWidget::shiftBgColor(const QString &shift) {
    if (shift == "Matin")      return QColor(249, 226, 175); // yellow
    if (shift == "Apres-midi") return QColor(166, 227, 161); // green
    if (shift == "Nuit")       return QColor(203, 166, 247); // purple
    return QColor(205, 214, 244);
}

void ScheduleWidget::onPrevWeek() { m_weekStart = m_weekStart.addDays(-7); refresh(); }
void ScheduleWidget::onNextWeek() { m_weekStart = m_weekStart.addDays(7);  refresh(); }

void ScheduleWidget::onAdd() {
    if (m_employees.isEmpty()) {
        QMessageBox::information(this, "Info", "Ajoutez d'abord des employes.");
        return;
    }
    ScheduleDialog dlg(m_employees, m_weekStart, this);
    if (dlg.exec() == QDialog::Accepted) {
        Schedule s = dlg.getSchedule();
        if (!DatabaseManager::instance().addSchedule(s))
            QMessageBox::critical(this, "Erreur", "Impossible d'ajouter le quart.");
        else refresh();
    }
}

void ScheduleWidget::onDelete() {
    auto *it = m_table->currentItem();
    if (!it) { QMessageBox::information(this, "Info", "Selectionnez une cellule a supprimer."); return; }
    int schedId = it->data(Qt::UserRole).toInt();
    if (schedId <= 0) return;
    auto ans = QMessageBox::question(this, "Confirmation", "Supprimer ce quart ?", QMessageBox::Yes | QMessageBox::No);
    if (ans == QMessageBox::Yes) {
        DatabaseManager::instance().deleteSchedule(schedId);
        refresh();
    }
}

void ScheduleWidget::onCellDoubleClicked(int row, int col) {
    QDate cellDate = m_weekStart.addDays(col);
    if (m_employees.isEmpty()) return;
    ScheduleDialog dlg(m_employees, cellDate, this);
    if (col < m_employees.size())
        ; // could preselect employee
    if (dlg.exec() == QDialog::Accepted) {
        Schedule s = dlg.getSchedule();
        DatabaseManager::instance().addSchedule(s);
        refresh();
    }
}
