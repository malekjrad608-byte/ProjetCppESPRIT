#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "../models/service.h"

class ServiceWidget : public QWidget {
    Q_OBJECT
public:
    explicit ServiceWidget(QWidget *parent=nullptr);
    void refresh();
    void focusSearch();
    void viewService(int id);
    static QString csvContent(const QList<Service> &services);
public slots:
    void onAdd();
signals:
    void servicesChanged();
private:
    void filter();
    void populate();
    void editSelected();
    void deleteSelected();
    void selectionChanged();
    void exportCsv();
    void notify(const QString &message);
    int selectedId() const;
    QTableWidget *m_table;
    QLineEdit *m_search;
    QComboBox *m_status;
    QComboBox *m_category;
    QComboBox *m_sort;
    QPushButton *m_edit;
    QPushButton *m_delete;
    QPushButton *m_view;
    QPushButton *m_export;
    QLabel *m_count;
    QLabel *m_notice;
    QWidget *m_empty;
    QLabel *m_emptyTitle;
    QLabel *m_emptyText;
    QList<Service> m_services;
    QList<Service> m_filtered;
};
