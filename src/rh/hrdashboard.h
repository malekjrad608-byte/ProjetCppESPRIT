#pragma once
#include "hrstore.h"
#include "hrdesign.h"

namespace AniRh {

class Dashboard : public QWidget {
    Q_OBJECT
public:
    explicit Dashboard(HrStore *store,QWidget *parent=nullptr);
    void refresh();
signals:
    void navigateRequested(int page);
    void addEmployeeRequested();
private:
    HrStore *m_store;
    QVBoxLayout *m_root;
};

} // namespace AniRh
