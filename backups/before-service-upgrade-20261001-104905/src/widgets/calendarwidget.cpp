#include "calendarwidget.h"
#include "../database/databasemanager.h"
#include "../dialogs/scheduledialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QMessageBox>
#include <QTextCharFormat>

CalendarWidget::CalendarWidget(QWidget *parent) : QWidget(parent) {
    setupUi();
    refresh();
}

void CalendarWidget::setupUi() {
    setStyleSheet(R"(
        QWidget { background: #1e1e2e; color: #cdd6f4; font-size: 13px; }
        QCalendarWidget { background: #181825; color: #cdd6f4; border: 1px solid #313244; border-radius: 8px; }
        QCalendarWidget QAbstractItemView {
            background: #181825; color: #cdd6f4;
            selection-background-color: #a6e3a1; selection-color: #1e1e2e;
        }
        QCalendarWidget QToolButton { background: #313244; color: #cdd6f4; border-radius: 4px; padding: 4px; }
        QCalendarWidget QToolButton:hover { background: #45475a; }
        QCalendarWidget QMenu { background: #313244; color: #cdd6f4; }
        QCalendarWidget QSpinBox { background: #313244; color: #cdd6f4; border: 1px solid #45475a; border-radius: 4px; }
        QListWidget { background: #181825; border: 1px solid #313244; border-radius: 8px; }
        QListWidget::item { padding: 10px; border-bottom: 1px solid #313244; }
        QListWidget::item:hover { background: #313244; }
        QPushButton#addBtn { background: #a6e3a1; color: #1e1e2e; border-radius: 6px; padding: 8px 18px; font-weight: bold; }
        QPushButton#addBtn:hover { background: #94e2d5; }
    )");

    auto *root = new QHBoxLayout(this);
    root->setSpacing(20);
    root->setContentsMargins(20,20,20,20);

    // Left: calendar
    auto *leftPanel = new QVBoxLayout;
    auto *calTitle = new QLabel("Calendrier", this);
    QFont tf = calTitle->font(); tf.setPointSize(18); tf.setBold(true);
    calTitle->setFont(tf); calTitle->setStyleSheet("color: #a6e3a1;");
    leftPanel->addWidget(calTitle);

    m_calendar = new QCalendarWidget(this);
    m_calendar->setGridVisible(true);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    QTextCharFormat weekendFmt;
    weekendFmt.setForeground(QColor("#f38ba8"));
    m_calendar->setWeekdayTextFormat(Qt::Saturday, weekendFmt);
    m_calendar->setWeekdayTextFormat(Qt::Sunday, weekendFmt);
    leftPanel->addWidget(m_calendar);

    root->addLayout(leftPanel, 3);

    // Right: shifts for selected day
    auto *rightPanel = new QVBoxLayout;
    m_dateLabel = new QLabel(this);
    QFont dlf = m_dateLabel->font(); dlf.setPointSize(14); dlf.setBold(true);
    m_dateLabel->setFont(dlf);
    m_dateLabel->setStyleSheet("color: #f9e2af;");
    rightPanel->addWidget(m_dateLabel);

    m_shiftList = new QListWidget(this);
    rightPanel->addWidget(m_shiftList);

    m_addBtn = new QPushButton("+ Ajouter un quart", this); m_addBtn->setObjectName("addBtn");
    rightPanel->addWidget(m_addBtn);

    root->addLayout(rightPanel, 2);

    connect(m_calendar, &QCalendarWidget::clicked,   this, &CalendarWidget::onDateSelected);
    connect(m_addBtn,   &QPushButton::clicked,        this, &CalendarWidget::onAddShift);

    onDateSelected(QDate::currentDate());
}

void CalendarWidget::refresh() {
    m_employees = DatabaseManager::instance().getAllEmployees();
    onDateSelected(m_calendar->selectedDate());
}

void CalendarWidget::onDateSelected(const QDate &date) {
    m_dateLabel->setText(date.toString("dddd dd MMMM yyyy"));
    loadShiftsForDate(date);
}

void CalendarWidget::loadShiftsForDate(const QDate &date) {
    m_shiftList->clear();
    auto all = DatabaseManager::instance().getSchedulesForWeek(date);
    for (const auto &sc : all) {
        if (sc.date != date) continue;
        QString badge;
        if (sc.shift == "Matin")      badge = "☀";
        else if (sc.shift == "Nuit")  badge = "🌙";
        else                          badge = "🌤";
        auto *item = new QListWidgetItem(
            QString("%1  %2  —  %3%4").arg(badge).arg(sc.employeeName).arg(sc.shift)
                .arg(sc.note.isEmpty() ? "" : "  (" + sc.note + ")"),
            m_shiftList
        );
        if (sc.shift == "Matin")      item->setForeground(QColor("#f9e2af"));
        else if (sc.shift == "Nuit")  item->setForeground(QColor("#cba6f7"));
        else                          item->setForeground(QColor("#a6e3a1"));
        item->setData(Qt::UserRole, sc.id);
    }
    if (m_shiftList->count() == 0)
        m_shiftList->addItem("Aucun quart planifie pour ce jour.");
}

void CalendarWidget::onAddShift() {
    if (m_employees.isEmpty()) {
        QMessageBox::information(this, "Info", "Ajoutez d'abord des employes.");
        return;
    }
    ScheduleDialog dlg(m_employees, m_calendar->selectedDate(), this);
    if (dlg.exec() == QDialog::Accepted) {
        Schedule s = dlg.getSchedule();
        if (!DatabaseManager::instance().addSchedule(s))
            QMessageBox::critical(this, "Erreur", "Impossible d'ajouter le quart.");
        else loadShiftsForDate(m_calendar->selectedDate());
    }
}
