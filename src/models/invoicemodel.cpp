#include "invoicemodel.h"
#include "../database/oracleconnection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <cmath>
bool InvoiceModel::loadStored(const Service &source,Service *invoice,QString *error) {
    *invoice=source;
    if(!OracleConnection::isOracle()) return true;
    QSqlQuery q(OracleConnection::database());
    q.prepare("SELECT id,invoiceNumber,COALESCE(invoice_emitted_at,invoiceDate),clientName,name,category,responsible,datetime,duration,price,description,invoice_status FROM services WHERE id=?");
    q.addBindValue(source.getId());
    if(!q.exec() || !q.next()){if(error)*error=q.lastError().isValid()?q.lastError().text():QStringLiteral("Facture enregistrée introuvable.");return false;}
    invoice->setId(q.value(0).toInt());invoice->setInvoiceNumber(q.value(1).toString());invoice->setInvoiceDate(QDateTime::fromString(q.value(2).toString(),Qt::ISODate));
    invoice->setClientName(q.value(3).toString());invoice->setName(q.value(4).toString());invoice->setCategory(q.value(5).toString());invoice->setResponsible(q.value(6).toString());
    invoice->setDateTime(QDateTime::fromString(q.value(7).toString(),Qt::ISODate));invoice->setDuration(q.value(8).toInt());invoice->setPrice(q.value(9).toDouble());invoice->setDescription(q.value(10).toString());
    if(q.value(11).toString()=="ANNULEE")invoice->setStatus("Annulé");
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
        QSqlQuery lock(db);lock.prepare("SELECT id FROM services WHERE id=? FOR UPDATE WAIT 5");lock.addBindValue(source.getId());
        if(!lock.exec() || !lock.next())return abort("Service indisponible : "+lock.lastError().text());
        lock.finish();
        if(!loadStored(source,&s,error)){db.rollback();return false;}
    }
    if (s.getClientName().trimmed().isEmpty() || s.getInvoiceNumber().isEmpty() || !s.getInvoiceDate().isValid() || s.getId()<=0 || !std::isfinite(s.getPrice()) || s.getPrice()<0 || s.getStatus()=="Annulé") {
        return abort(s.getStatus()=="Annulé"?"Un service annulé ne peut pas être facturé.":"Complétez le nom du client et enregistrez le service avant de facturer.");
    }
    if(persisted) {
        QSqlQuery q(db);q.prepare("UPDATE services SET invoice_status='EMISE',invoice_emitted_at=? WHERE id=? AND invoice_status='BROUILLON'");
        q.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd'T'HH:mm:ss"));q.addBindValue(s.getId());
        if(!q.exec())return abort(q.lastError().text());
        if(!loadStored(source,&s,error)){db.rollback();return false;}
    }
    if(!render(s,error)){if(persisted)db.rollback();return false;}
    if(persisted && !db.commit())return abort("PDF créé, mais validation Oracle échouée : "+db.lastError().text());
    return true;
}
