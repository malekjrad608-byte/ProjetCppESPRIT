#pragma once
#include <QDate>
#include <QString>
#include <QVariant>
#include <QVector>
class StockRecord {
public:
  StockRecord() = default;
  int idArticle=0;
  QString article;
  QString nomArticle;
  QString category;
  QString lot;
  int quantity = 0;
  QString unit;
  int threshold = 1;
  QDate expiry;
  int localId = 0;
  static bool afficher(QVector<StockRecord> *items, qint64 *revision,
                       QString *error);
  static bool enregistrer(const QVector<StockRecord> &items, qint64 *revision,
                          QString *error);
  // Lecture/ecriture SQL propres a cette entite, avec revision de concurrence.
  using Rows = QVector<QVariantList>;
  static bool readRows(Rows *rows, qint64 *revision, QString *error);
  static bool saveRows(const Rows &rows, qint64 *revision, QString *error);

private:
  static bool loadRows(Rows *rows, QString *error);
  static qint64 rowsVersion(const Rows &rows);
};
