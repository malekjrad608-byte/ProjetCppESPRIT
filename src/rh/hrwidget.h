#pragma once
#include "hrstore.h"
#include "hrdesign.h"
#include "hrdashboard.h"
#include "hrdialogs.h"
#include "hrschedule.h"

namespace AniRh {

class HrWidget : public QWidget {
    Q_OBJECT
public:
    HrWidget(HrStore *store,bool demo=false,QWidget *parent=nullptr);
    void navigate(int index);
    void refresh();
    void focusSearch();
    void openCreate();
    void openEdit();
    void exportCurrent();
    void exportSelectedPdf();
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    struct Section {
        QWidget *page=nullptr;
        QLineEdit *search=nullptr;
        QComboBox *filter=nullptr;
        QDateEdit *period=nullptr;
        QTableWidget *table=nullptr;
        QLabel *summary=nullptr,*count=nullptr,*empty=nullptr;
        QPushButton *edit=nullptr,*remove=nullptr,*approve=nullptr,*reject=nullptr,*pdf=nullptr,*paid=nullptr;
        QStringList headers;
    };
    HrStore *m_store;
    bool m_demo;
    QStackedWidget *m_stack=nullptr;
    Dashboard *m_dashboard=nullptr;
    ScheduleWidget *m_schedule=nullptr;
    QTabBar *m_tabs=nullptr;
    QList<Section> m_sections;
    QLabel *m_toast=nullptr;
    QTimer m_toastTimer;
    QString m_lastDay;
    QWidget *buildSection(int index);
    void refreshSection(int index);
    void selectionChanged(int index);
    Record selected() const;
    void removeSelected();
    void decide(bool approve);
    void paySelected();
    void notify(const QString &message);
    void reportError();
};

} // namespace AniRh
