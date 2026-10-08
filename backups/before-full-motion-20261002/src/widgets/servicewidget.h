#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QScrollArea>
#include <QStackedWidget>
#include <QGridLayout>
#include "../models/service.h"

class ServiceWidget : public QWidget {
    Q_OBJECT
public:
    explicit ServiceWidget(QWidget *parent = nullptr);
    void refresh();
    void focusSearch();
    void viewService(int id);
    static QString csvContent(const QList<Service> &services);
public slots:
    void onAdd();
signals:
    void servicesChanged();
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    void setupDetailPanel(QHBoxLayout *body);
    void filter();
    void populate();
    void rebuildCards();
    void selectService(int id);
    void updateDetail();
    void updateSummary();
    void editSelected();
    void deleteSelected();
    void duplicateSelected();
    void advanceStatus();
    void exportCsv();
    void exportPdf();
    void showStatistics();
    void notify(const QString &message, bool error = false);
    QString filterDescription() const;
    Service selectedService() const;
    QTableWidget *m_table;
    QLineEdit *m_search;
    QComboBox *m_status;
    QComboBox *m_category;
    QComboBox *m_sort;
    QComboBox *m_period;
    QPushButton *m_edit;
    QPushButton *m_delete;
    QPushButton *m_view;
    QPushButton *m_export;
    QPushButton *m_duplicate;
    QPushButton *m_advance;
    QLabel *m_count;
    QLabel *m_notice;
    QList<QLabel *> m_summary;
    QStackedWidget *m_views;
    QScrollArea *m_gridScroll;
    QGridLayout *m_grid;
    QLabel *m_emptyTitle;
    QLabel *m_emptyText;
    QLabel *m_detailName;
    QLabel *m_detailReference;
    QLabel *m_detailNotes;
    QLabel *m_attention;
    QWidget *m_detailContent;
    QWidget *m_emptyDetailSpacer;
    QVBoxLayout *m_statusLayout;
    QList<QLabel *> m_detailValues;
    int m_selectedId = 0;
    int m_gridColumns = 0;
    int m_viewMode = 0;
    QList<Service> m_services;
    QList<Service> m_filtered;
};
