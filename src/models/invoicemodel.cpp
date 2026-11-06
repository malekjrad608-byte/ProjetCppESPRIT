#include "invoicemodel.h"
#include "../database/oracleconnection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <cmath>
bool InvoiceModel::loadStored(const Service &source,Service *invoice,QString *error) {
    *invoice=source;
    if(!OracleConnection::isOracle()) return true;
    QSqlQuery q(OracleConnection::database());
    q.prepare("SELECT id_serv,ref_service,categorie,statut,date_heur,duree,tarif,prestation FROM service WHERE id_serv=?");
    q.addBindValue(source.getId());
    if(!q.exec() || !q.next()){if(error)*error=q.lastError().isValid()?q.lastError().text():QStringLiteral("Service introuvable.");return false;}
    invoice->setId(q.value(0).toInt());invoice->setName(q.value(1).toString());invoice->setClientName(q.value(1).toString());
    invoice->setCategory(q.value(2).toString());invoice->setStatus(q.value(3).toString());
    invoice->setDateTime(QDateTime::fromString(q.value(4).toString(),Qt::ISODate));
    invoice->setDuration(q.value(5).toInt());invoice->setPrice(q.value(6).toDouble());invoice->setDescription(q.value(7).toString());
    invoice->setInvoiceNumber(QString("FAC-%1-%2").arg(invoice->getDateTime().date().year()).arg(invoice->getId(),6,10,QChar('0')));
    invoice->setInvoiceDate(QDateTime::currentDateTime());
    if(error)error->clear();
    return true;
}

bool InvoiceModel::emitDocument(const Service &source, bool demo, const Renderer &render, QString *error) {
    Service s=source;
    auto db=OracleConnection::database();
    const bool persisted=!demo && OracleConnection::isOracle();
    auto abort=[&](const QString &message){if(persisted)db.rollback();if(error)*error=message;return false;};
    if(persisted) {
        if(!db.transaction()){if(error)*error=db.lastError().text();return false;}
        // Same lock order as a service modification, preventing a stale PDF snapshot.
        QSqlQuery lock(db);lock.prepare("SELECT id_serv FROM service WHERE id_serv=? FOR UPDATE WAIT 5");lock.addBindValue(source.getId());
        if(!lock.exec() || !lock.next())return abort("Service indisponible : "+lock.lastError().text());
        lock.finish();
        if(!loadStored(source,&s,error)){db.rollback();return false;}
    }
    if (s.getClientName().trimmed().isEmpty() || s.getInvoiceNumber().isEmpty() || !s.getInvoiceDate().isValid() || s.getId()<=0 || !std::isfinite(s.getPrice()) || s.getPrice()<0 || s.getStatus()=="Annulé") {
        return abort(s.getStatus()=="Annulé"?"Un service annulé ne peut pas être facturé.":"Complétez le nom du client et enregistrez le service avant de facturer.");
    }
    // The exact five-table schema has no invoice columns. Export is read-only:
    // the saved PDF is the snapshot; no table, column or service row is modified.
    if(!render(s,error)){if(persisted)db.rollback();return false;}
    if(persisted && !db.commit())return abort("PDF créé, mais validation Oracle échouée : "+db.lastError().text());
    return true;
}
