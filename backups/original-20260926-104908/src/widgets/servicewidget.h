#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include "../models/service.h"
#include "../models/employee.h"

class ServiceWidget : public QWidget {
    Q_OBJECT
public:
    explicit ServiceWidget(QWidget *parent = nullptr);
    void refresh();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onSearch(const QString &text);
    void onFilterStatus(int idx);

private:
    void setupUi();
    void populateTable(const QList<Service> &services);
    void applyRowColor(int row, const QString &status);
    QColor statusColor(const QString &status);

    QTableWidget *m_table;
    QPushButton  *m_addBtn;
    QPushButton  *m_editBtn;
    QPushButton  *m_deleteBtn;
    QLineEdit    *m_searchEdit;
    QComboBox    *m_statusFilter;
    QLabel       *m_countLabel;

    QList<Service>  m_services;
    QList<Employee> m_employees;
};
