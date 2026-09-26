#include "servicewidget.h"
#include "../database/databasemanager.h"
#include "../dialogs/servicedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFont>
#include <QLabel>
#include <QFrame>

ServiceWidget::ServiceWidget(QWidget *parent) : QWidget(parent) {
    setupUi();
    refresh();
}

void ServiceWidget::setupUi() {
    setStyleSheet(R"(
        QWidget { background: #1e1e2e; color: #cdd6f4; font-size: 13px; }
        QTableWidget {
            background: #181825;
            border: 1px solid #313244;
            border-radius: 8px;
            gridline-color: #313244;
            alternate-background-color: #1e1e2e;
        }
        QTableWidget::item { padding: 8px; }
        QTableWidget::item:selected { background: #313244; color: #89b4fa; }
        QHeaderView::section {
            background: #313244;
            color: #89b4fa;
            font-weight: bold;
            padding: 10px 8px;
            border: none;
            border-bottom: 2px solid #89b4fa;
        }
        QPushButton#addBtn { background: #89b4fa; color: #1e1e2e; border-radius: 6px; padding: 8px 18px; font-weight: bold; }
        QPushButton#addBtn:hover { background: #b4befe; }
        QPushButton#editBtn { background: #f9e2af; color: #1e1e2e; border-radius: 6px; padding: 8px 18px; font-weight: bold; }
        QPushButton#editBtn:hover { background: #fab387; }
        QPushButton#deleteBtn { background: #f38ba8; color: #1e1e2e; border-radius: 6px; padding: 8px 18px; font-weight: bold; }
        QPushButton#deleteBtn:hover { background: #eba0ac; }
        QLineEdit { background: #313244; border: 1px solid #45475a; border-radius: 6px; padding: 6px 12px; color: #cdd6f4; }
        QLineEdit:focus { border: 1px solid #89b4fa; }
        QComboBox { background: #313244; border: 1px solid #45475a; border-radius: 6px; padding: 6px 10px; color: #cdd6f4; }
        QComboBox QAbstractItemView { background: #313244; color: #cdd6f4; selection-background-color: #89b4fa; selection-color: #1e1e2e; }
    )");

    auto *root = new QVBoxLayout(this);
    root->setSpacing(16);
    root->setContentsMargins(20, 20, 20, 20);

    // Header row
    auto *header = new QHBoxLayout;
    auto *titleLbl = new QLabel("Services", this);
    QFont tf = titleLbl->font(); tf.setPointSize(18); tf.setBold(true);
    titleLbl->setFont(tf);
    titleLbl->setStyleSheet("color: #89b4fa;");
    header->addWidget(titleLbl);
    header->addStretch();

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Rechercher...");
    m_searchEdit->setMinimumWidth(200);
    header->addWidget(m_searchEdit);

    m_statusFilter = new QComboBox(this);
    m_statusFilter->addItems({"Tous", "Planifie", "En cours", "Termine", "Annule"});
    m_statusFilter->setMinimumWidth(120);
    header->addWidget(m_statusFilter);

    root->addLayout(header);

    // Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"ID", "Nom du service", "Responsable", "Date / Heure", "Statut", "Description"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(true);
    root->addWidget(m_table);

    // Action buttons + count
    auto *actBar = new QHBoxLayout;
    m_countLabel = new QLabel(this);
    m_countLabel->setStyleSheet("color: #6c7086; font-size: 12px;");
    actBar->addWidget(m_countLabel);
    actBar->addStretch();
    m_addBtn    = new QPushButton("+ Ajouter", this);   m_addBtn->setObjectName("addBtn");
    m_editBtn   = new QPushButton("Modifier", this);    m_editBtn->setObjectName("editBtn");
    m_deleteBtn = new QPushButton("Supprimer", this);   m_deleteBtn->setObjectName("deleteBtn");
    actBar->addWidget(m_addBtn);
    actBar->addWidget(m_editBtn);
    actBar->addWidget(m_deleteBtn);
    root->addLayout(actBar);

    connect(m_addBtn,      &QPushButton::clicked,            this, &ServiceWidget::onAdd);
    connect(m_editBtn,     &QPushButton::clicked,            this, &ServiceWidget::onEdit);
    connect(m_deleteBtn,   &QPushButton::clicked,            this, &ServiceWidget::onDelete);
    connect(m_searchEdit,  &QLineEdit::textChanged,          this, &ServiceWidget::onSearch);
    connect(m_statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ServiceWidget::onFilterStatus);
}

void ServiceWidget::refresh() {
    auto &db = DatabaseManager::instance();
    m_employees = db.getAllEmployees();
    m_services  = db.getAllServices();
    populateTable(m_services);
}

void ServiceWidget::populateTable(const QList<Service> &services) {
    m_table->setRowCount(0);
    for (const auto &s : services) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(s.id)));
        m_table->setItem(row, 1, new QTableWidgetItem(s.name));
        m_table->setItem(row, 2, new QTableWidgetItem(s.responsible));
        m_table->setItem(row, 3, new QTableWidgetItem(s.dateTime.toString("dd/MM/yyyy HH:mm")));
        auto *statusItem = new QTableWidgetItem(s.status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 4, statusItem);
        m_table->setItem(row, 5, new QTableWidgetItem(s.description));
        applyRowColor(row, s.status);
        m_table->setRowHeight(row, 42);
    }
    m_countLabel->setText(QString("%1 service(s)").arg(services.size()));
}

void ServiceWidget::applyRowColor(int row, const QString &status) {
    QColor bg = statusColor(status);
    for (int c = 0; c < m_table->columnCount(); ++c) {
        auto *it = m_table->item(row, c);
        if (it) { it->setBackground(bg); }
    }
}

QColor ServiceWidget::statusColor(const QString &status) {
    if (status == "En cours")  return QColor(249, 226, 175, 40);
    if (status == "Termine")   return QColor(166, 227, 161, 40);
    if (status == "Annule")    return QColor(243, 139, 168, 40);
    return QColor(0,0,0,0); // Planifie
}

void ServiceWidget::onAdd() {
    ServiceDialog dlg(m_employees, this);
    if (dlg.exec() == QDialog::Accepted) {
        Service s = dlg.getService();
        if (s.name.isEmpty()) { QMessageBox::warning(this, "Erreur", "Le nom du service est obligatoire."); return; }
        if (!DatabaseManager::instance().addService(s))
            QMessageBox::critical(this, "Erreur", "Impossible d'ajouter le service.");
        else refresh();
    }
}

void ServiceWidget::onEdit() {
    int row = m_table->currentRow();
    if (row < 0) { QMessageBox::information(this, "Info", "Selectionnez un service a modifier."); return; }
    int id = m_table->item(row, 0)->text().toInt();
    Service current;
    for (const auto &s : m_services)
        if (s.id == id) { current = s; break; }

    ServiceDialog dlg(current, m_employees, this);
    if (dlg.exec() == QDialog::Accepted) {
        Service s = dlg.getService();
        if (s.name.isEmpty()) { QMessageBox::warning(this, "Erreur", "Le nom du service est obligatoire."); return; }
        if (!DatabaseManager::instance().updateService(s))
            QMessageBox::critical(this, "Erreur", "Impossible de modifier le service.");
        else refresh();
    }
}

void ServiceWidget::onDelete() {
    int row = m_table->currentRow();
    if (row < 0) { QMessageBox::information(this, "Info", "Selectionnez un service a supprimer."); return; }
    int id   = m_table->item(row, 0)->text().toInt();
    QString name = m_table->item(row, 1)->text();
    auto ans = QMessageBox::question(this, "Confirmation",
        QString("Supprimer le service \"%1\" ?").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    if (ans == QMessageBox::Yes) {
        if (!DatabaseManager::instance().deleteService(id))
            QMessageBox::critical(this, "Erreur", "Impossible de supprimer le service.");
        else refresh();
    }
}

void ServiceWidget::onSearch(const QString &text) {
    QList<Service> filtered;
    QString filter = m_statusFilter->currentText();
    for (const auto &s : m_services) {
        bool matchText   = text.isEmpty() || s.name.contains(text, Qt::CaseInsensitive) || s.responsible.contains(text, Qt::CaseInsensitive);
        bool matchStatus = (filter == "Tous") || s.status == filter;
        if (matchText && matchStatus) filtered.append(s);
    }
    populateTable(filtered);
}

void ServiceWidget::onFilterStatus(int) {
    onSearch(m_searchEdit->text());
}
