from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
def read(p): return (root/p).read_text(encoding="utf-8-sig")
def write(p,s): (root/p).write_text(s,encoding="utf-8")

# Ignore C++ strings/comments while matching function braces.
def mask(s):
    pattern = r'R"(?P<delimiter>[^\s()\\]{0,16})\(.*?\)(?P=delimiter)"|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*.*?\*/'
    return re.sub(pattern, lambda m: "".join("\n" if c=="\n" else " " for c in m.group()), s, flags=re.S)

def function(s, name):
    m = re.search(r"(?m)^[^\n;{}]*\b"+re.escape(name)+r"\s*\([^;\n]*\)\s*(?:const\s*)?\{",s)
    assert m, name
    start=m.start(); brace=m.end()-1; masked=mask(s); depth=1;i=brace+1
    while depth:
        depth += (masked[i]=="{")-(masked[i]=="}");i+=1
    return s[start:i],start,i

def rename_function(s, old, new):
    return re.sub(r"\b"+re.escape(old)+r"(?=\s*\()",new,s,count=1)

def append(p,code): write(p,read(p)+"\n"+code+"\n")

# Service owns its planning, including its typed result.
planning=read("src/models/serviceplanning.cpp")
code=[]
for name in ("appointments","available","weekStart","end","conflictingServices","inWeek"):
    f,_,_=function(planning,name)
    f=rename_function(f,name,"Service::"+name)
    if name=="appointments": f=f.replace("QList<Appointment> Service::","QList<Service::Appointment> Service::",1)
    code.append(f)
append("src/models/service.cpp","\n".join(code))
h=read("src/models/service.h").replace("#include <QList>","#include <QList>\n#include <QSet>")
decl=read("src/models/serviceplanning.h").split("namespace ServicePlanning {",1)[1].rsplit("}",1)[0]
decl=re.sub(r"(?m)^(QList<Appointment>|bool|QDate|QDateTime|QSet<int>|QList<Service>) ",r"static \1 ",decl)
h=h.replace("  private:", "    // Planning des affectations : requetes et regles dans le modele Service.\n"+decl+"\n  private:",1)
write("src/models/service.h",h)

# FiveTableStore: employee relations belong to Employe, invoice status to Service.
five=read("src/models/fivetablestore.cpp")
for name in ("moveEmployee","employeeService","saveEmployee"):
    f,_,_=function(five,name)
    append("src/employees/employe.cpp",rename_function(f,name,"Employe::"+name).replace("database()","OracleConnection::database()").replace("friendlyError(","OracleConnection::friendlyError("))
f,_,_=function(five,"invoiceStatus")
append("src/models/service.cpp",rename_function(f,"invoiceStatus","Service::invoiceStatus").replace("database()","OracleConnection::database()"))
h=read("src/models/service.h").replace("  private:", "    static QString invoiceStatus(int serviceId);\n\n  private:",1)
write("src/models/service.h",h)

# Schema/import operations are class members too (used only in demo/local mode).
store=read("src/employees/employeestore.cpp")
for name in ("initialize","importInitial"):
    f,_,_=function(store,name)
    append("src/employees/employe.cpp",rename_function(f,name,"Employe::"+name))
h=read("src/employees/employe.h").replace("#include <QList>","#include <QList>\n#include <QSqlDatabase>\n#include <QVariant>")
h=h.replace("private:", """    // Persistance et affectation de l'entite Employe.
    static bool initialize(QSqlDatabase db, QString *error);
    static bool importInitial(QSqlDatabase db, QString *error);
    static bool moveEmployee(int employee, int target, QString *error);
    static bool saveEmployee(int id, int serviceId, const QList<QVariant> &values, QString *error);
    static int employeeService(int employeeId);

private:""",1)
write("src/employees/employe.h",h)
cpp=read("src/employees/employe.cpp")
cpp=cpp.replace('#include <cmath>','#include <cmath>\n#include <QSqlRecord>\n#include <QTemporaryDir>\n#include <QFile>\n#include "../models/service.h"')
write("src/employees/employe.cpp",cpp)

# Medical CRUD is implemented in MedicalRecord itself, no storage namespace.
medical=read("src/medical/medicalstore.cpp")
record=read("src/medical/medicalrecord.cpp")
record=record.split('#include "medicalstore.h"')[0]
helpers=medical[medical.index("namespace {"):medical.index("namespace MedicalStore {")]
includes=medical[:medical.index("namespace {")].replace('#include "medicalstore.h"','#include "medicalrecord.h"')
record=includes+helpers+record
names={"initialize":"initialize","all":"afficher","save":"enregistrer","remove":"supprimer",
       "seedExamples":"seedExamples","importInitial":"importInitial","reportHtml":"reportHtml"}
for old,new in names.items():
    f,_,_=function(medical,old)
    f=rename_function(f,old,"MedicalRecord::"+new)
    f=re.sub(r"\ball\(", "afficher(",f)
    f=re.sub(r"\bsave\(", "enregistrer(",f)
    record+="\n"+f+"\n"
record+="""
bool MedicalRecord::ajouter(QString *error) {
    if(storageId!=0) { if(error) *error="Ce dossier existe déjà."; return false; }
    return enregistrer(*this,error);
}
bool MedicalRecord::modifier(QString *error) {
    if(storageId<=0) { if(error) *error="Dossier non enregistré."; return false; }
    return enregistrer(*this,error);
}
"""
write("src/medical/medicalrecord.cpp",record)
h=read("src/medical/medicalrecord.h").replace("#include <QVector>","#include <QVector>\n#include <QSqlDatabase>")
h=h.replace("bool ajouter(QString *error);","bool ajouter(QString *error = nullptr);").replace("bool modifier(QString *error);","bool modifier(QString *error = nullptr);")
h=h.replace("afficher(QString *error);","afficher(QString *error = nullptr);").replace("supprimer(qint64 id,QString *error);","supprimer(qint64 id,QString *error = nullptr);")
h=h.replace("    qint64 storageId", """    // Requetes, transactions et import de l'entite medicale.
    static bool initialize(QSqlDatabase db, QString *error);
    static bool enregistrer(MedicalRecord &record, QString *error = nullptr);
    static QVector<MedicalRecord> examples();
    static bool seedExamples(QString *error = nullptr);
    static bool importInitial(QString *error = nullptr);
    static QString reportHtml(const QVector<MedicalRecord> &records, bool demo = false);

    qint64 storageId""")
write("src/medical/medicalrecord.h",h)

# Local and StockRecord own their concrete SQL and concurrency check.
collection=read("src/models/oraclecollection.cpp")
fields={
 "Local":'QStringList{"id","name","type","floor","capacity","occupancy","specialty","out_of_service","cleaning","last_cleaned","dirty_since","id_service"}',
 "StockRecord":'QStringList{"lot","article","category","quantity","unit","threshold","expiry","id_local"}'
}
for cls,path,table in (("Local","src/locaux/local","locaux"),("StockRecord","src/stock/stockrecord","stock_lots")):
    blocks=[]
    for old,new in (("load","loadRows"),("version","rowsVersion"),("read","readRows"),("save","saveRows")):
        f,_,_=function(collection,old)
        f=rename_function(f,old,cls+"::"+new)
        f=f.replace("const QString &name,","")
        f=f.replace("OracleCollection::Rows","Rows")
        f=f.replace('    if(!allowed(name)){*error="Collection inconnue.";return false;}\n',"")
        f=f.replace("columns(name)",fields[cls]).replace("table(name)",'QStringLiteral("'+table+'")')
        f=f.replace('name=="stock"?"Choisissez le local de chaque lot.":"Choisissez le service de chaque local."',
                    '"Choisissez le local de chaque lot."' if cls=="StockRecord" else '"Choisissez le service de chaque local."')
        f=f.replace("load(name,","loadRows(").replace("version(","rowsVersion(")
        blocks.append(f)
    cpp=read(path+".cpp").replace('#include "../models/oraclecollection.h"',"""#include "../database/oracleconnection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMap>
#include <QCryptographicHash>""")
    cpp=cpp.replace("OracleCollection::Rows","Rows").replace('OracleCollection::read("stock",',"readRows(").replace('OracleCollection::read("locaux",',"readRows(")
    cpp=cpp.replace('OracleCollection::save("stock",',"saveRows(").replace('OracleCollection::save("locaux",',"saveRows(")
    write(path+".cpp",cpp+"\n"+"\n".join(blocks)+"\n")
    h=read(path+".h").replace("#include <QString>","#include <QString>\n#include <QVariant>\n#include <QVector>")
    if cls=="Local": h=h.replace("struct Local\n{","class Local\n{\npublic:\n    Local() = default;")
    else: h=h.replace("public:","public:\n    StockRecord() = default;",1)
    i=h.rfind("};")
    h=h[:i]+"""    // Lecture/ecriture SQL propres a cette entite, avec revision de concurrence.
    using Rows = QVector<QVariantList>;
    static bool readRows(Rows *rows, qint64 *revision, QString *error);
    static bool saveRows(const Rows &rows, qint64 *revision, QString *error);
private:
    static bool loadRows(Rows *rows, QString *error);
    static qint64 rowsVersion(const Rows &rows);
"""+h[i:]
    write(path+".h",h)

# Infrastructure remains separate from the unchanged teacher Connection singleton.
# Convert the Oracle facade and Accounts to real C++ classes.
for cls,path in (("OracleConnection","src/database/oracleconnection"),("Accounts","src/auth/accounts")):
    h=read(path+".h").replace("namespace "+cls+" {","class "+cls+" {\npublic:")
    h=re.sub(r"(?m)^(QString|bool|QSqlDatabase|qint64|Role|int) ",r"    static \1 ",h)
    h=h.rstrip()[:-1]+"};\n"
    cpp=read(path+".cpp")
    start=cpp.index("namespace "+cls+" {")
    brace=cpp.index("{",start);masked=mask(cpp);depth=1;i=brace+1
    while depth: depth+=(masked[i]=="{")-(masked[i]=="}");i+=1
    body=cpp[brace+1:i-1]
    body=re.sub(r"(?m)^(QString|bool|QSqlDatabase|qint64|Role|int) (\w+)\(",lambda m:("Accounts::Role" if m[1]=="Role" else m[1])+" "+cls+"::"+m[2]+"(",body)
    cpp=cpp[:start]+body+cpp[i:]
    if cls=="OracleConnection":
        h=h.replace("};","    static int nextId(const QString &sequence);\n    static QString friendlyError(const QString &message);\n};")
        for name in ("nextId","friendlyError"):
            f,_,_=function(five,name);cpp+="\n"+rename_function(f,name,cls+"::"+name)+"\n"
    write(path+".h",h);write(path+".cpp",cpp)

# EmployeeQueries' internal SQL executor is also a private class member.
cpp=read("src/models/employeequeries.cpp")
f,a,b=function(cpp,"read")
cpp=cpp[:a]+cpp[b:]
end=cpp.index("\n}\n")+3
f=rename_function(f,"read","EmployeeQueries::read").replace("const QVariantList &values = {}","const QVariantList &values")
cpp=cpp[:end]+"\n"+f+"\n"+cpp[end:]
write("src/models/employeequeries.cpp",cpp)
h=read("src/models/employeequeries.h").replace("};","private:\n    static ModelRows read(const QString &sql, const QVariantList &values = {});\n};")
write("src/models/employeequeries.h",h)

# Rewrite consumers; no compatibility namespaces or old SQL implementation remains.
mapping={
 "ServicePlanning::":"Service::","EmployeeStore::":"Employe::",
 "MedicalStore::all":"MedicalRecord::afficher","MedicalStore::save":"MedicalRecord::enregistrer",
 "MedicalStore::remove":"MedicalRecord::supprimer","MedicalStore::":"MedicalRecord::",
 "FiveTableStore::moveEmployee":"Employe::moveEmployee","FiveTableStore::saveEmployee":"Employe::saveEmployee",
 "FiveTableStore::employeeService":"Employe::employeeService","FiveTableStore::invoiceStatus":"Service::invoiceStatus",
 "FiveTableStore::nextId":"OracleConnection::nextId","FiveTableStore::friendlyError":"OracleConnection::friendlyError",
}
obsolete=[
 "src/models/fivetablestore","src/models/serviceplanning","src/models/oraclecollection",
 "src/employees/employeestore","src/medical/medicalstore"
]
obsolete_files={p+e for p in obsolete for e in (".h",".cpp")}
include_targets={
 "serviceplanning.h":"src/models/service.h","fivetablestore.h":"src/employees/employe.h",
 "employeestore.h":"src/employees/employe.h","medicalstore.h":"src/medical/medicalrecord.h",
 "oraclecollection.h":"src/stock/stockrecord.h"
}
import os
for folder in ("src","tests"):
 for p in (root/folder).rglob("*"):
    if p.suffix not in (".h",".cpp") or str(p.relative_to(root)).replace("\\","/") in obsolete_files: continue
    s=p.read_text(encoding="utf-8-sig")
    for old,new in mapping.items(): s=s.replace(old,new)
    s=s.replace("OracleCollection::Rows","StockRecord::Rows")
    s=s.replace('OracleCollection::read("stock",',"StockRecord::readRows(").replace('OracleCollection::save("stock",',"StockRecord::saveRows(")
    def include(m):
        name=m[1].split("/")[-1]
        return '#include "'+os.path.relpath(root/include_targets[name],p.parent).replace("\\","/")+'"' if name in include_targets else m[0]
    s=re.sub(r'#include "([^"]+)"',include,s)
    p.write_text(s,encoding="utf-8")

# Remove old implementation files from qmake. Keep their backup, not duplicate code.
s=read("sources.pri")
for p in obsolete_files: s=s.replace("$$PWD/"+p,"")
s=re.sub(r"(?m)^(?:SOURCES|HEADERS) \+=\s*$","",s)
s=re.sub(r"(?m)^[ \t]+\\\n","",s)
write("sources.pri",s)
for p in obsolete_files:
    target=(root/p).resolve()
    assert root.resolve() in target.parents
    target.unlink()
print("Moved SQL into entity classes; removed obsolete storage namespace files.")
