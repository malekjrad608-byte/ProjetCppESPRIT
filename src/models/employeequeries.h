#pragma once
#include "modelrows.h"

// Employee read model and planning commands. No QWidget dependency.
class EmployeeQueries {
public:
  static ModelRows conflicts();
  static ModelRows consecutive(const QString &today);
  static ModelRows statistics(const QString &start, const QString &finish);
  static ModelRows workloads(const QString &start, const QString &finish);
  static ModelRows workloadRanking(const QString &start, const QString &finish);
  static ModelRows list(const QString &filter, const QString &sort,
                        const QString &job, const QString &status);
  static ModelRows find(int id);
  static ModelRows countAvailable(bool available);
  static ModelRows weeklyCharge(const QString &start, const QString &finish);
  static ModelRows skilled(const QString &skill);
  static ModelRows balance();
  static ModelRows candidates();
  static ModelRows plannedHours(int id, const QString &start,
                                const QString &finish);
  static ModelRows addTask(const QString &title, const QString &skill,
                           const QString &start, const QString &finish,
                           double hours, int employee);
  static ModelRows certificateList();
  static ModelRows certificate(int id);
  static ModelRows planningEmployees();

private:
  static ModelRows read(const QString &sql, const QVariantList &values = {});
  static const QString columns;
  static const QString joinWeek;
  static const QString charge;
};
