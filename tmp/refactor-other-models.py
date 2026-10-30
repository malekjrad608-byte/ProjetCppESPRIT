from pathlib import Path
def read(p):return Path(p).read_text(encoding='utf-8-sig')
def write(p,s):Path(p).write_text(s,encoding='utf-8')
p='src/ui/invoice.cpp';s=read(p);a=s.index('bool Invoice::loadStored');model=s[a:];s=s[:a]+'''bool Invoice::loadStored(const Service &source,Service *invoice,QString *error) {
    return InvoiceModel::loadStored(source,invoice,error);
}
bool Invoice::writePdf(const QString &path,const Service &source,bool demo,QString *error) {
    return InvoiceModel::emitDocument(source,demo,[&](const Service &s,QString *failure) {
        return writeDocument(path,html(s,demo),"Facture "+s.invoiceNumber,failure);
    },error);
}
''';s=s.replace('#include <QSqlQuery>','#include "../models/invoicemodel.h"').replace('#include <QSqlError>','');write(p,s)
model=model.replace('Invoice::loadStored','InvoiceModel::loadStored').replace('bool Invoice::writePdf(const QString &path, const Service &source, bool demo, QString *error)','bool InvoiceModel::emitDocument(const Service &source, bool demo, const Renderer &render, QString *error)').replace('writeDocument(path,html(s,demo),"Facture "+s.invoiceNumber,error)','render(s,error)')
write('src/models/invoicemodel.cpp','#include "invoicemodel.h"\n#include "../database/oracleconnection.h"\n#include <QSqlQuery>\n#include <QSqlError>\n#include <cmath>\n'+model)
write('src/models/invoicemodel.h','''#pragma once
#include "service.h"
#include <functional>
class InvoiceModel {
public:
    using Renderer=std::function<bool(const Service &,QString *)>;
    static bool loadStored(const Service &source,Service *invoice,QString *error);
    static bool emitDocument(const Service &source,bool demo,const Renderer &render,QString *error);
};
''')
p='src/medical/medicalrecord.h';s=read(p).replace('#include <QDate>','#include <QDate>\n#include <QVector>');s=s.replace('    MedicalRecord();','''    MedicalRecord();
    bool ajouter(QString *error);
    bool modifier(QString *error);
    static QVector<MedicalRecord> afficher(QString *error);
    static bool supprimer(qint64 id,QString *error);
''');write(p,s)
p='src/medical/medicalrecord.cpp';s=read(p);s+='''
#include "medicalstore.h"
bool MedicalRecord::ajouter(QString *error) { if(storageId!=0){*error="Ce dossier existe déjà.";return false;}return MedicalStore::save(*this,error); }
bool MedicalRecord::modifier(QString *error) { if(storageId<=0){*error="Dossier non enregistré.";return false;}return MedicalStore::save(*this,error); }
QVector<MedicalRecord> MedicalRecord::afficher(QString *error) { return MedicalStore::all(error); }
bool MedicalRecord::supprimer(qint64 id,QString *error) { return MedicalStore::remove(id,error); }
''';write(p,s)
p='src/medical/medicalwidget.cpp';s=read(p).replace('MedicalStore::all(&error)','MedicalRecord::afficher(&error)').replace('MedicalStore::remove(','MedicalRecord::supprimer(').replace('MedicalStore::save(record,&error)','(record.storageId>0?record.modifier(&error):record.ajouter(&error))');write(p,s)
# Stock entity and model replace persistence in the QWidget.
p='src/stock/stockwidget.h';s=read(p);a=s.index('struct StockRecord');b=s.index('class StockWidget',a);entity=s[a:b].replace('struct StockRecord {','class StockRecord {\npublic:');entity=entity.replace('\n};','''
    static bool afficher(QVector<StockRecord> *items,qint64 *revision,QString *error);
    static bool enregistrer(const QVector<StockRecord> &items,qint64 *revision,QString *error);
};''');write('src/stock/stockrecord.h','#pragma once\n#include <QDate>\n#include <QString>\n#include <QVector>\n'+entity);s=s[:a]+'#include "stockrecord.h"\n\n'+s[b:];write(p,s)
p='src/stock/stockwidget.cpp';s=read(p);a=s.index('    OracleCollection::Rows rows;');b=s.index('bool StockWidget::persistStock()',a)
s=s[:a]+'''    QString error;
    QVector<StockRecord> loaded;
    if(!StockRecord::afficher(&loaded,&m_revision,&error)) {
        m_storageReady=false;QMessageBox::critical(this,"Stock Oracle",error);return false;
    }
    m_storageReady=true;items=loaded;persistedItems=items;return true;
}
'''+s[b:]
s=s.replace('    QString error;OracleCollection::Rows rows;\n    for(const auto &r:items) rows<<QVariantList{r.lot,r.article,r.category,r.quantity,r.unit,r.threshold,r.expiry.toString(Qt::ISODate),LocalChoice::sqlValue(r.localId)};','    QString error;').replace('OracleCollection::save("stock",rows,&m_revision,&error)','StockRecord::enregistrer(items,&m_revision,&error)').replace('#include "../database/oraclecollection.h"','');write(p,s)
write('src/stock/stockrecord.cpp','''#include "stockrecord.h"
#include "../database/oraclecollection.h"
#include <QSet>
bool StockRecord::afficher(QVector<StockRecord> *items,qint64 *revision,QString *error) {
    OracleCollection::Rows rows;
    if(!OracleCollection::read("stock",&rows,revision,error))return false;
    items->clear();
    for(const auto &r:rows) {
        StockRecord item;item.lot=r[0].toString();item.article=r[1].toString();item.category=r[2].toString();
        item.quantity=r[3].toInt();item.unit=r[4].toString();item.threshold=r[5].toInt();item.expiry=QDate::fromString(r[6].toString(),Qt::ISODate);item.localId=r[7].toInt();
        items->append(item);
    }
    return true;
}
bool StockRecord::enregistrer(const QVector<StockRecord> &items,qint64 *revision,QString *error) {
    OracleCollection::Rows rows;QSet<QString> lots;
    for(const auto &r:items) {
        if(r.lot.trimmed().isEmpty() || r.article.trimmed().isEmpty() || r.quantity<0 || r.threshold<0 || r.localId<=0 || !r.expiry.isValid() || lots.contains(r.lot)) {
            *error="Vérifiez le lot unique, l’article, la quantité, le seuil, l’échéance et le local obligatoire.";return false;
        }
        lots.insert(r.lot);
        rows.append({r.lot,r.article,r.category,r.quantity,r.unit,r.threshold,r.expiry.toString(Qt::ISODate),r.localId});
    }
    return OracleCollection::save("stock",rows,revision,error);
}
''')
p='sources.pri';s=read(p);s+='\nSOURCES += $$PWD/src/models/invoicemodel.cpp $$PWD/src/stock/stockrecord.cpp\nHEADERS += $$PWD/src/models/invoicemodel.h $$PWD/src/stock/stockrecord.h\n';write(p,s)
