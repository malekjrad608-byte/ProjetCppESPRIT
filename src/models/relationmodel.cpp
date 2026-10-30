#include "../database/oracleconnection.h"
#include "relationmodel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
ModelRows RelationModel::afficher(Entity entity) {
    QString sql;
    switch(entity) {
    case Entity::Employees: sql="SELECT id,TRIM(nom||' '||prenom) FROM employes ORDER BY nom,prenom,id";break;
    case Entity::Services: sql="SELECT id,name||' - '||COALESCE(clientName,'Sans client')||' - '||SUBSTR(datetime,1,10) FROM services ORDER BY datetime DESC,id DESC";break;
    case Entity::Locals: sql="SELECT id,name,out_of_service FROM locaux ORDER BY name";break;
    }
    ModelRows result;
    QSqlQuery query(OracleConnection::database());
    if(!query.prepare(sql) || !query.exec()) { result.error={query.lastError().text(),query.lastError().nativeErrorCode()};return result; }
    while(query.next()) {
        QVariantList row;
        for(int i=0;i<query.record().count();++i)row.append(query.value(i));
        result.rows.append(row);
    }
    return result;
}
