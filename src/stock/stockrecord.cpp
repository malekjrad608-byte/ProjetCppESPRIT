#include "stockrecord.h"
#include "../database/oracleconnection.h"
#include <QCryptographicHash>
#include <QMap>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
bool StockRecord::afficher(QVector<StockRecord> *items, qint64 *revision,
                           QString *error) {
  Rows rows;
  if (!readRows(&rows, revision, error))
    return false;
  items->clear();
  for (const auto &r : rows) {
    StockRecord item;
    item.idArticle=r.value(9).toInt();
    item.lot = r[0].toString();
    item.article = r[1].toString();
    item.nomArticle = r.value(8, r[1]).toString();
    item.category = r[2].toString();
    item.quantity = r[3].toInt();
    item.unit = r[4].toString();
    item.threshold = r[5].toInt();
    item.expiry = QDate::fromString(r[6].toString(), Qt::ISODate);
    item.localId = r[7].toInt();
    items->append(item);
  }
  return true;
}
bool StockRecord::enregistrer(const QVector<StockRecord> &items,
                              qint64 *revision, QString *error) {
  Rows rows;
  QSet<QString> articles;
  for (const auto &r : items) {
    if ((!OracleConnection::isOracle() && r.lot.trimmed().isEmpty()) || r.article.trimmed().isEmpty() ||
        r.quantity < 0 || r.threshold < 0 || r.localId <= 0 ||
        !r.expiry.isValid() || articles.contains(r.article)) {
      *error = "Vérifiez le lot, la référence article unique, la quantité, le seuil, "
               "l’échéance et le local obligatoire.";
      return false;
    }
    articles.insert(r.article);
    rows.append({r.lot, r.article, r.category, r.quantity, r.unit, r.threshold,
                 r.expiry.toString(Qt::ISODate), r.localId, r.nomArticle.isEmpty() ? r.article : r.nomArticle, r.idArticle});
  }
  return saveRows(rows, revision, error);
}

bool StockRecord::loadRows(Rows *rows, QString *error) {
  QSqlQuery q(OracleConnection::database());
  const auto fields = QStringList{"lot",  "article",   "category", "quantity",
                                  "unit", "threshold", "expiry",   "id_local", "nom_article", "id_article"};
  if (!(q.prepare("SELECT " + fields.join(',') + " FROM " +
                  QStringLiteral("stock_lots") + " ORDER BY " + fields[0]) &&
        q.exec())) {
    *error = q.lastError().text();
    return false;
  }
  rows->clear();
  while (q.next()) {
    QVariantList row;
    for (int i = 0; i < fields.size(); ++i)
      row << q.value(i);
    *rows << row;
  }
  return true;
}
qint64 StockRecord::rowsVersion(const Rows &rows) {
  QMap<QString, QVariantList> ordered;
  for (const auto &row : rows)
    ordered.insert(row.value(9,row[1]).toString(), row);
  QByteArray bytes;
  for (const auto &row : ordered)
    for (const auto &value : row) {
      const auto s = value.toString().toUtf8();
      bytes += QByteArray::number(s.size()) + ':' + s;
    }
  const auto hash = QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
  quint64 result = 0;
  for (int i = 0; i < 8; ++i)
    result = (result << 8) | quint8(hash[i]);
  return qint64((result & 0x7fffffffffffffffULL) | 1);
}
bool StockRecord::readRows(Rows *rows, qint64 *revision, QString *error) {
  if (!loadRows(rows, error))
    return false;
  *revision = rowsVersion(*rows);
  error->clear();
  return true;
}
bool StockRecord::saveRows(const Rows &input, qint64 *revision, QString *error) {
  Rows rows=input;
  auto db=OracleConnection::database();
  if(!db.transaction()){*error=db.lastError().text();return false;}
  auto fail=[&](QString msg){db.rollback();*error=OracleConnection::friendlyError(msg);return false;};
  QSqlQuery q(db);q.prepare("LOCK TABLE stock IN EXCLUSIVE MODE WAIT 5");
  if(!q.exec())return fail(q.lastError().text());
  Rows before;
  if(!loadRows(&before,error))return fail(*error);
  if(rowsVersion(before)!=*revision)return fail("Ces données ont été modifiées par une autre session. Actualisez avant de réessayer.");
  QMap<int,QVariantList> old;QMap<QString,int> articleIds;
  for(const auto &r:before){old.insert(r[9].toInt(),r);articleIds.insert(r[1].toString(),r[9].toInt());}
  QSet<int> kept;QSet<QString> articles;
  for(auto &r:rows){
    if(r.size()==8)r<<r[1];
    if(r.size()==9)r<<0;
    if(r.size()!=10 || r[1].toString().trimmed().isEmpty() || r[7].toInt()<=0 || articles.contains(r[1].toString()))return fail("Référence article unique et local obligatoires.");
    articles.insert(r[1].toString());
    if(r[9].toInt()<=0 && articleIds.contains(r[1].toString()))r[9]=articleIds.value(r[1].toString());
    if(r[9].toInt()>0){if(!old.contains(r[9].toInt()) || kept.contains(r[9].toInt()))return fail("Article introuvable ou identifiant répété.");kept.insert(r[9].toInt());}
  }
  for(auto it=old.cbegin();it!=old.cend();++it)if(!kept.contains(it.key())){
    q.prepare("DELETE FROM stock WHERE id_article=?");q.addBindValue(it.key());if(!q.exec())return fail(q.lastError().text());
  }
  for(const auto &r:rows){
    const bool update=r[9].toInt()>0;
    q.prepare(update
      ?"UPDATE stock SET article=?,nom_article=?,lot=?,quantite_disp=?,type_article=?,seuil_min=?,date_echeance=?,id_local=? WHERE id_article=?"
      :"INSERT INTO stock(article,nom_article,lot,quantite_disp,type_article,seuil_min,date_echeance,id_local) VALUES(?,?,?,?,?,?,?,?)");
    for(int i:{1,8,0,3,2,5,6,7})q.addBindValue(r[i]);
    if(update)q.addBindValue(r[9]);
    if(!q.exec())return fail(q.lastError().text());
  }
  Rows saved;if(!loadRows(&saved,error))return fail(*error);
  if(!db.commit())return fail(db.lastError().text());
  *revision=rowsVersion(saved);error->clear();return true;
}
