#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QVariantMap>
#include <QDate>

namespace AniRh {

using Record = QVariantMap;
using Records = QList<Record>;

class HrStore : public QObject {
    Q_OBJECT
public:
    explicit HrStore(QObject *parent = nullptr);
    ~HrStore() override;
    bool open(const QString &path);
    QString error() const { return m_error; }
    QString path() const { return m_path; }
    Records rows(const QString &sql, const QVariantList &args = {}) const;
    Record row(const QString &sql, const QVariantList &args = {}) const;
    qint64 scalar(const QString &sql, const QVariantList &args = {}) const;
    Records employees(bool activeOnly = false) const;
    bool saveEmployee(Record data, int id = 0);
    bool setEmployeeActive(int id, bool active);
    bool saveAttendance(Record data, int id = 0);
    bool saveLeave(Record data);
    bool decideLeave(int id, bool approve);
    int usedLeave(int employee, int year) const;
    bool saveTask(Record data, int id = 0);
    bool saveReview(Record data, int id = 0);
    bool removeRecord(const QString &entity, int id);
    bool preparePayroll(const QString &month);
    bool updatePayroll(int id, qint64 bonus, qint64 deductions, const QString &notes);
    bool markPaid(int id);
    bool saveShift(Record data, int id = 0);
    bool copyScheduleWeek(const QDate &monday);
    bool seedDemo();
    static QString today();
signals:
    void changed();
private:
    QSqlDatabase m_db;
    QString m_connection, m_path;
    mutable QString m_error;
    bool fail(const QString &message);
    bool exec(const QString &sql, const QVariantList &args = {});
    bool save(const QString &table, const Record &data, int id);
    bool validateEmployeeDate(int employee, const QDate &date, bool requireActive = true);
};

} // namespace AniRh
