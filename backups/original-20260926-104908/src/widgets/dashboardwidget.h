#pragma once
#include <QWidget>
#include <QLabel>

class DashboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit DashboardWidget(QWidget *parent = nullptr);
    void refresh();

private:
    void setupUi();
    QLabel *createStatCard(const QString &title, const QString &value, const QString &color);

    QLabel *m_totalServices;
    QLabel *m_inProgress;
    QLabel *m_done;
    QLabel *m_employees;
};
