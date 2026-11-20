#pragma once
#include "hrstore.h"
#include "hrdesign.h"

namespace AniRh {

class EditDialog : public QDialog {
    Q_OBJECT
public:
    enum Kind { Employee, Attendance, Leave, Task, Review, Payroll, Shift };
    EditDialog(HrStore *store,Kind kind,const Record &record={},QWidget *parent=nullptr);
private:
    HrStore *m_store;
    Kind m_kind;
    Record m_record;
    QMap<QString,QWidget *> m_fields;
    QFormLayout *m_form;
    QLabel *m_error;
    QLabel *m_hint=nullptr;
    void field(const QString &key,const QString &caption,QWidget *widget);
    QLineEdit *text(const QString &key,const QString &caption,const QString &placeholder={});
    QComboBox *combo(const QString &key,const QString &caption,const QStringList &items);
    QDateEdit *date(const QString &key,const QString &caption,const QDate &fallback);
    QDoubleSpinBox *amount(const QString &key,const QString &caption);
    QSpinBox *number(const QString &key,const QString &caption,int low,int high,int fallback);
    void multiline(const QString &key,const QString &caption);
    void employeePicker();
    void save();
};
bool exportPayslip(const QString &path,const Record &payroll,bool demo,QString *error=nullptr);

} // namespace AniRh
