#pragma once
#include <QDialog>
#include <QMap>
#include "../models/service.h"
#include "../models/service.h"
class QComboBox;
class QDateEdit;
class QCheckBox;
class QLabel;
class QTableWidget;
class QPushButton;

class ServicePlanningDialog : public QDialog {
    Q_OBJECT
public:
    explicit ServicePlanningDialog(const QDate &date=QDate::currentDate(),QWidget *parent=nullptr);
    void showEmployee(int employeeId,bool readOnly=false);
signals:
    void servicesChanged();
private:
    void reload();
    void populate();
    void editAssignment();
    QDateEdit *m_date;
    QComboBox *m_employee;
    QCheckBox *m_cancelled;
    QLabel *m_period;
    QLabel *m_summary;
    QLabel *m_message;
    QTableWidget *m_table;
    QPushButton *m_edit;
    QList<Service::Appointment> m_entries;
    bool m_readOnly=false;
    QMap<int,QString> m_names;
    QMap<int,bool> m_available;
    QString m_loadError;
};
