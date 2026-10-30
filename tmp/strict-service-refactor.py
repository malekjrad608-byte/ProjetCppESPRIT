from pathlib import Path
p=Path('src/models/service.h');s=p.read_text(encoding='utf-8-sig')
fields=s[s.index('    int id ='):s.index('private:')]
import re
access=[]
for typ,name in re.findall(r'^    (int|double|QString|QDateTime) (\w+)',fields,re.M):
    cap=name[0].upper()+name[1:]
    access += [f'    {typ} get{cap}() const {{ return {name}; }}',f'    void set{cap}({typ} value) {{ {name} = value; }}']
s=s.replace('    Service() = default;', '''    Service() = default;
    Service(int id, const QString &name, const QString &clientName,
            const QString &category, const QDateTime &dateTime,
            const QString &status, int duration, double price);

    // Atelier : l'interface appelle le modele et consulte son resultat.
    bool ajouter();
    bool modifier();
    bool supprimer(int id);
    QString lastError() const { return m_error; }
'''+ '\n'.join(access))
s=s.replace('static QSqlQueryModel *afficher(QObject *parent = nullptr);','QSqlQueryModel *afficher(QObject *parent = nullptr, const QList<int> *orderedIds = nullptr) const;')
s=s.replace('    int id =','private:\n    QString m_error;\n    int id =',1)
s=s.replace('    bool enregistrer(bool','    bool enregistrerOracle(bool update, int *insertedId, QString *error) const;\n    bool enregistrer(bool')
p.write_text(s,encoding='utf-8')
p=Path('src/models/fivetablestore.cpp');s=p.read_text(encoding='utf-8-sig');start=s.index('bool saveService(');end=s.index('bool saveEmployee(',start);body=s[start:end];s=s[:start]+s[end:];s=s.replace('namespace {','namespace FiveTableStore {',1).replace('}\nnamespace FiveTableStore {','',1);p.write_text(s,encoding='utf-8')
p=Path('src/models/fivetablestore.h');s=p.read_text(encoding='utf-8-sig');s=s.replace('bool saveService(const Service &service,bool update,int *insertedId,QString *error);','int nextId(const QString &sequence);\nbool moveEmployee(int employee, int target, QString *error);');p.write_text(s,encoding='utf-8')
p=Path('src/models/service.cpp');s=p.read_text(encoding='utf-8-sig');s=s.replace('QScopedPointer<QSqlQueryModel> model(afficher());','QScopedPointer<QSqlQueryModel> model(Service().afficher());');s=s.replace('FiveTableStore::saveService(s,update,insertedId,error)','enregistrerOracle(update,insertedId,error)')
body=body.replace('bool saveService(const Service &s,bool update,int *insertedId,QString *error) {','bool Service::enregistrerOracle(bool update, int *insertedId, QString *error) const {\n    const Service &s = *this;')
body=body.replace('auto db=database()', 'auto db=OracleConnection::database()').replace('friendlyError(', 'FiveTableStore::friendlyError(').replace('nextId(', 'FiveTableStore::nextId(').replace('moveEmployee(', 'FiveTableStore::moveEmployee(')
s += '\n'+body
s=s.replace('QSqlQueryModel *Service::afficher(QObject *parent) {','QSqlQueryModel *Service::afficher(QObject *parent, const QList<int> *orderedIds) const {')
start=s.index('    query.prepare(OracleConnection::isOracle()?');end=s.index('    query.exec();',start)
s=s[:start]+'''    QString sql = OracleConnection::isOracle()
        ? "SELECT id,name,description,responsible,category,dateTime,status,duration,price,clientName,invoiceNumber,invoiceDate,0,id_employe,0 FROM services"
        : "SELECT id,name,description,responsible,category,dateTime,status,duration,price,clientName,invoiceNumber,invoiceDate,id_local,id_employe,id_client FROM services";
    // La vue transmet uniquement les identifiants filtres, jamais du SQL.
    if (orderedIds) {
        if (orderedIds->isEmpty()) sql += " WHERE 1=0";
        else {
            QStringList placeholders;
            for (int i=0; i<orderedIds->size(); ++i) placeholders << "?";
            sql += " WHERE id IN (" + placeholders.join(',') + ") ORDER BY CASE id";
            for (int i=0; i<orderedIds->size(); ++i) sql += " WHEN ? THEN " + QString::number(i);
            sql += " END";
        }
    } else sql += " ORDER BY dateTime DESC,id DESC";
    query.prepare(sql);
    if (orderedIds) {
        for (int value : *orderedIds) query.addBindValue(value);
        for (int value : *orderedIds) query.addBindValue(value);
    }
'''+s[end:]
s += '''
// Constructeur parametre : memes etapes que Etudiant dans l'atelier.
Service::Service(int id, const QString &name, const QString &clientName,
                 const QString &category, const QDateTime &dateTime,
                 const QString &status, int duration, double price) {
    this->id = id;
    this->name = name;
    this->clientName = clientName;
    this->category = category;
    this->dateTime = dateTime;
    this->status = status;
    this->duration = duration;
    this->price = price;
}

bool Service::ajouter() { return ajouter(&m_error); }
bool Service::modifier() { return modifier(&m_error); }
bool Service::supprimer(int id) { return supprimer(id, &m_error); }
'''
# Named bindings for service writes, matching the workshop.
s=s.replace('query.prepare("DELETE FROM services WHERE id=?"); query.addBindValue(id);','query.prepare("DELETE FROM services WHERE id=:id"); query.bindValue(":id",id);')
for prefix,cols in [('query',['name','description','responsible','category','datetime','status','duration','price','clientName','id_local','id_employe','id_client']),('q',['name','description','responsible','category','datetime','status','duration','price','clientName','id_employe','id'])]:
    a=s.index(f'    {prefix}.prepare(update'); b=s.index(f'    if(!{prefix}.exec())',a)
    chunk=s[a:b]
    # In each SQL string substitute placeholders by columns in order.
    def sqlnames(m):
        text=m.group(0); order=cols+(['id'] if prefix=='query' and text.startswith('"UPDATE') else [])
        it=iter(order);return re.sub(r'\?',lambda _:':'+next(it),text)
    chunk=re.sub(r'"(?:UPDATE|INSERT)[^"\n]+"',sqlnames,chunk)
    it=iter(cols+(['id'] if prefix=='query' else []))
    chunk=chunk.replace(f'{prefix}.addBindValue(',f'{prefix}.bindValue(')
    chunk=re.sub(re.escape(prefix)+r'\.bindValue\(',lambda m:m.group(0)+'":'+next(it)+'",',chunk)
    s=s[:a]+chunk+s[b:]
p.write_text(s,encoding='utf-8')
# Static convenience callers use an actual model instance.
p=Path('tests/service_tests.cpp');s=p.read_text(encoding='utf-8-sig').replace('Service::afficher()', 'Service().afficher()');p.write_text(s,encoding='utf-8')
