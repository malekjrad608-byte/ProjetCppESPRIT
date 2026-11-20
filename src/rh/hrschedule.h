#pragma once
#include "hrstore.h"
#include "hrdesign.h"

namespace AniRh {

class ScheduleWidget : public QWidget {
    Q_OBJECT
public:
    ScheduleWidget(HrStore *store,bool demo,QWidget *parent=nullptr);
    void refresh();
    void createShift();
    void focusSearch() { m_search->setFocus(); }
private:
    HrStore *m_store;
    bool m_demo;
    QDateEdit *m_week;
    QLineEdit *m_search;
    QComboBox *m_department;
    QTableWidget *m_grid;
    QLabel *m_range,*m_summary,*m_empty;
    QDate monday() const;
    void editShift(const Record &record);
    void exportWeek();
    void copyWeek();
};

} // namespace AniRh
