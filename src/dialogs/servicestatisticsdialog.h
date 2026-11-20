#pragma once
#include <QDialog>
#include "../models/service.h"
class QDateEdit;class QComboBox;class QLabel;class ServiceCurve;
class ServiceStatisticsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ServiceStatisticsDialog(QWidget *parent=nullptr);
private:
    void reload(bool initial=false);
    void rebuild();
    QList<Service> m_services;
    QDateEdit *m_from,*m_to;
    QComboBox *m_category,*m_metric,*m_group;
    ServiceCurve *m_curve;
    QLabel *m_summary,*m_notice;
};
