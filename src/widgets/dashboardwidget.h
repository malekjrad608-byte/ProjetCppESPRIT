#pragma once
#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QProgressBar>

class DashboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit DashboardWidget(QWidget *parent=nullptr);
public slots:
    void refresh();
protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
signals:
    void createRequested();
    void servicesRequested();
    void serviceRequested(int id);
private:
    QList<QLabel *> m_values;
    QList<QProgressBar *> m_bars;
    QList<QLabel *> m_categoryCounts;
    QTableWidget *m_table;
    QWidget *m_empty;
    QLabel *m_todayCount;
    QLabel *m_completion;
};
