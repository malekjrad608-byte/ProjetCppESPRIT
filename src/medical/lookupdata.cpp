#include "lookupdata.h"
#include "medicalrecord.h"
#include "appointment.h"
#include "../employees/employe.h"
#include "../stock/stock.h"
#include "../database/oracleconnection.h"
QList<LookupItem> LookupData::employes() {
 QList<LookupItem> out;for(const auto &e:Employe::afficher())out.append({QString::number(e.getId()),e.getPrenom()+" "+e.getNom(),-1});return out;
}
QList<LookupItem> LookupData::proprietaires() {
 QList<LookupItem> out;QString error;for(const auto &p:MedicalRecord::proprietaires(&error))out.append({p.second,p.second,-1});return out;
}
QList<LookupItem> LookupData::traitementsEnStock() {
 QList<LookupItem> out;QVector<StockRecord> rows;qint64 revision=0;QString error;
 if(Stock::afficher(&rows,&revision,&error))for(const auto &r:rows)if(r.quantity>0)out.append({QString::number(r.idArticle),r.nomArticle,r.quantity});return out;
}
QString LookupData::telephoneProprietaire(const QString &owner) {
 const auto rows=RdvManager::tous({});for(auto i=rows.crbegin();i!=rows.crend();++i)if(i->proprietaire==owner && !i->telephone.isEmpty())return i->telephone;return {};
}
QMap<QString, QStringList> LookupData::racesParEspece()
{
    QMap<QString, QStringList> m;
    m["Chien"]  = { "Berger Allemand", "Golden Retriever", "Labrador", "Bulldog", "Husky",
                    "Caniche", "Rottweiler", "Beagle", "Croisé", "Autre" };
    m["Chat"]   = { "Européen", "Siamois", "Persan", "Maine Coon", "Bengal", "Sphynx",
                    "Angora", "Croisé", "Autre" };
    m["Lapin"]  = { "Nain", "Bélier (Lop)", "Angora", "Rex", "Géant des Flandres", "Autre" };
    m["Oiseau"] = { "Perroquet", "Canari", "Perruche", "Cockatiel", "Inséparable", "Autre" };
    return m;
}
