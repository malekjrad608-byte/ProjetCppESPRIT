#pragma once
#include "localchoice.h"
#include "../models/relationmodel.h"
namespace RelationChoices {
inline void fill(QComboBox *box,RelationModel::Entity entity,int selected,const QString &empty,const QString &legacy={}) {
    box->clear();box->addItem(empty,0);box->setItemData(0,legacy,Qt::UserRole+1);
    if(!OracleConnection::isOracle()){box->setEnabled(false);return;}
    auto q = RelationModel::afficher(entity);
    if(!q.isOk()){box->setEnabled(false);box->setToolTip(q.lastError().text());}
    else while(q.next()) {box->addItem(q.value(1).toString()+QString(" (#%1)").arg(q.value(0).toInt()),q.value(0).toInt());box->setItemData(box->count()-1,q.value(1),Qt::UserRole+1);}
    int i=box->findData(selected);
    if(i<0 && selected>0){box->addItem(QString("Référence #%1 (à vérifier)").arg(selected),selected);box->setItemData(box->count()-1,legacy,Qt::UserRole+1);i=box->count()-1;}
    box->setCurrentIndex(qMax(0,i));
}
inline void employees(QComboBox *b,int id,const QString &legacy) {
    fill(b,RelationModel::Entity::Employees,id,legacy.isEmpty()?QStringLiteral("Non affecté"):QStringLiteral("Ancien responsable : ")+legacy,legacy);
}
inline void clients(QComboBox *b,int) {b->clear();b->addItem(QStringLiteral("Client saisi dans le service"),0);}
inline void services(QComboBox *b,int id=0) {fill(b,RelationModel::Entity::Services,id,QStringLiteral("Aucun service associé"));}
}
