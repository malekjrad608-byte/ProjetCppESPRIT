exec(open(__file__.replace('refactor-service.py','refactor-atelier.py'),encoding='utf-8').read().split("p='src/employees/gestionemployes.cpp'")[0])
p='src/database/databasemanager.cpp';s=read(p)
a=s.index('QList<Service> DatabaseManager::getAllServices()');b=s.index('bool DatabaseManager::addExamples()',a)
block=s[a:b]
block=block.replace('QList<Service> DatabaseManager::getAllServices()', 'QList<Service> Service::afficherListe(QString *error)')
block=block.replace('bool DatabaseManager::save(const Service &s, bool update)', 'bool Service::enregistrer(bool update, int *insertedId, QString *error) const')
block=block.replace('bool DatabaseManager::deleteService(int id)', 'bool Service::supprimer(int id, QString *error)')
block=block.replace('    if (s.name.trimmed().isEmpty()', '    const Service &s=*this;\n    if (s.name.trimmed().isEmpty()')
block=block.replace('QSqlQuery query(m_db)', 'QSqlQuery query(QSqlDatabase::database("aniworld-services"))')
block=block.replace('isOracle()', 'OracleConnection::isOracle()')
block=block.replace('&m_error','error').replace('m_error.clear()','error->clear()').replace('m_error','*error').replace('&m_lastInsertedId','insertedId').replace('m_lastInsertedId','*insertedId')
block=block.replace('bool DatabaseManager::addService(const Service &s) { return save(s,false); }\nbool DatabaseManager::updateService(const Service &s) { return save(s,true); }', '''bool Service::ajouter(QString *error) { int inserted=0; if(!enregistrer(false,&inserted,error))return false; id=inserted; return true; }
bool Service::modifier(QString *error) const { int inserted=0; return enregistrer(true,&inserted,error); }''')
block=block.replace('LocalChoice::sqlValue','nullableId')
headers='''#include "service.h"
#include "serviceplanning.h"
#include "../database/oracleconnection.h"
#include "../database/fivetablestore.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <cmath>
namespace { QVariant nullableId(int id) { return id>0?QVariant(id):QVariant(QMetaType(QMetaType::Int)); } }
'''
write('src/models/service.cpp',headers+block)
s=s[:a]+'''QList<Service> DatabaseManager::getAllServices() { return Service::afficherListe(&m_error); }
bool DatabaseManager::save(const Service &s,bool update) {
    Service entity=s;
    const bool ok=update?entity.modifier(&m_error):entity.ajouter(&m_error);
    if(ok)m_lastInsertedId=entity.id;
    return ok;
}
bool DatabaseManager::addService(const Service &s) { return save(s,false); }
bool DatabaseManager::updateService(const Service &s) { return save(s,true); }
bool DatabaseManager::deleteService(int id) { return Service::supprimer(id,&m_error); }
'''+s[b:]
s=s.replace('#include "../ui/localchoice.h"','')
write(p,s)
p='src/models/service.h';s=read(p).replace('#include <QDateTime>','#include <QDateTime>\n#include <QList>\nclass QSqlQueryModel;\nclass QObject;')
s=s.replace('struct Service {','''class Service {
public:
    Service() = default;
    bool ajouter(QString *error);
    bool modifier(QString *error) const;
    static bool supprimer(int id, QString *error);
    static QList<Service> afficherListe(QString *error);
    static QSqlQueryModel *afficher(QObject *parent = nullptr);
''')
s=s.replace('\n};','\nprivate:\n    bool enregistrer(bool update, int *insertedId, QString *error) const;\n};')
write(p,s)
p='src/models/service.cpp';s=read(p);s+='''
QSqlQueryModel *Service::afficher(QObject *parent) {
    auto *model=new QSqlQueryModel(parent);
    QSqlQuery query(QSqlDatabase::database("aniworld-services"));
    query.prepare("SELECT id,name,category,datetime,price,status FROM services ORDER BY datetime DESC,id DESC");
    query.exec();
    model->setQuery(std::move(query));
    const QStringList titles={"ID","Service","Catégorie","Date / heure","Tarif","Statut"};
    for(int i=0;i<titles.size();++i)model->setHeaderData(i,Qt::Horizontal,titles[i]);
    return model;
}
'''
# Model validation also applies to programmatic calls, not only form widgets.
s=s.replace('    if(s.status!="Annulé"', '''    if(s.duration<5 || s.duration>1440 || !QStringList{"Planifié","En cours","Terminé","Annulé"}.contains(s.status) || s.category.trimmed().isEmpty()) {
        *error="Vérifiez la catégorie, le statut et la durée (5 à 1440 minutes).";return false;
    }
    if(s.status!="Annulé"''')
write(p,s)
