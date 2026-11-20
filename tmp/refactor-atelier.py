from pathlib import Path
root=Path(__file__).resolve().parents[1]
def read(p): return (root/p).read_text(encoding='utf-8-sig')
def write(p,s): (root/p).write_text(s,encoding='utf-8')
def replace_block(s,start,end,new):
    a=s.index(start); b=s.index(end,a)
    return s[:a]+new+s[b:]
p='src/employees/gestionemployes.cpp';s=read(p)
s=s.replace('#include <QSqlQuery>','#include "../models/employeequeries.h"').replace('#include <QSqlError>','').replace('#include <QSqlDatabase>','')
s=replace_block(s,'    QSqlQuery q(', '    while (q.next())', '    auto q = EmployeeQueries::conflicts();\n')
s=replace_block(s,'    QSqlQuery q(', '    if (!q.exec()) return alertes;', '    auto q = EmployeeQueries::consecutive(QDate::currentDate().toString("yyyy-MM-dd") + " 00:00:00");\n')
s=replace_block(s,'        const QString jointureChargeSemaine =','        int totalEmp', '        auto qStats = EmployeeQueries::statistics(debutSemaineSql, finSemaineSql);\n')
s=replace_block(s,'        QSqlQuery qCharges(', '        if (qCharges.exec())', '        auto qCharges = EmployeeQueries::workloads(debutSemaineSql, finSemaineSql);\n')
s=replace_block(s,'        QSqlQuery q(', '        int row = 0;', '        auto q = EmployeeQueries::workloadRanking(debutSemaineSql, finSemaineSql);\n\n')
s=replace_block(s,'    QString orderBy =', '    afficherCards(liste);', '    QString error;\n    const auto liste = Employe::afficher(filtre, tri, poste, statut, &error);\n    if(!error.isEmpty()) { m_detailPlaceholder->setText("Chargement impossible : " + error); return; }\n\n')
s=replace_block(s,'    QSqlQuery q(', '    if (q.next()) m_lblDisponibles', '    auto q = EmployeeQueries::countAvailable(true);\n')
s=s.replace('    q.exec("SELECT COUNT(*) FROM employes WHERE disponible=0");','    q = EmployeeQueries::countAvailable(false);')
s=replace_block(s,'    QSqlQuery qCharge(', '    if (qCharge.exec()', '    auto qCharge = EmployeeQueries::weeklyCharge(borneSemaine(debut), borneSemaine(debut.addDays(7)));\n')
s=replace_block(s,'            QSqlQuery q(', '            if (q.exec()', '            auto q = EmployeeQueries::find(id);\n')
# Replace persistence in employee form with an entity command, including SQLite path.
a=s.index('    if(OracleConnection::isOracle()) {\n        QString error;',s.index('bool GestionEmployes::ouvrirDialog'))
b=s.index('    e.setNom(',a)
s=s[:a]+s[b:]
a=s.index('    return true;',s.index('    e.setNom(',s.index('bool GestionEmployes::ouvrirDialog')))
s=s[:a]+'''    e.setServiceId(serviceChoice->currentData().toInt());
    QString error;
    if (!(modification ? e.modifier(&error) : e.ajouter(&error))) {
        QMessageBox::warning(this, "Enregistrement impossible", error);
        return false;
    }
'''+s[a:]
s=replace_block(s,'    QSqlQuery q(', '    if (!q.exec() || !q.next()) return;', '    auto q = EmployeeQueries::find(m_idSelectionne);\n')
s=replace_block(s,'    QSqlQuery q(', '    QString nom;', '    auto q = EmployeeQueries::find(m_idSelectionne);\n')
s=s.replace('nom = q.value(1).toString() + " " + q.value(0).toString();','nom = q.value(2).toString() + " " + q.value(1).toString();')
s=replace_block(s,'    QSqlQuery qDel(', '        return;\n    }', '''    QString error;
    if (!Employe::supprimer(m_idSelectionne, &error)) {
        QMessageBox::warning(this, "Employé conservé", error);
''')
s=replace_block(s,'    QSqlQuery qSelect(', '// ==================== EMP-11', '''    QString error;
    if (!Employe::basculerDisponibilite(m_idSelectionne, &error)) {
        QMessageBox::warning(this, "Modification impossible", error);
        return;
    }
    onFiltresChange();
}

''')
s=replace_block(s,'    QSqlQuery q(', '    QStringList resultats;', '    auto q = EmployeeQueries::skilled(comp.trimmed());\n\n')
s=replace_block(s,'    QSqlQuery q(', '    QStringList surcharges, dispo;', '    auto q = EmployeeQueries::balance();\n\n')
s=replace_block(s,'    QSqlQuery employes(', '    int meilleurId', '    auto employes = EmployeeQueries::candidates();\n')
s=replace_block(s,'        QSqlQuery planifie(', '        if (!planifie.exec()', '        auto planifie = EmployeeQueries::plannedHours(id, semaineDebutSql, semaineFinSql);\n')
s=replace_block(s,'    QSqlQuery insert(', '    if (!insert.exec())', '    auto insert = EmployeeQueries::addTask(nomTache, competence, debutSql, finSql, heures->value(), meilleurId);\n')
s=replace_block(s,'        QSqlQuery qEmployes(', '            while (qEmployes.next())', '        auto qEmployes = EmployeeQueries::certificateList();\n        if (qEmployes.isOk()) {\n')
s=replace_block(s,'        QSqlQuery q(', '        if (!q.exec()', '        auto q = EmployeeQueries::certificate(id);\n')
for name in ['q','qCharges','qCharge','planifie','insert']: s=s.replace(name+'.exec()',name+'.isOk()')
# Inline feedback replaces redundant success dialogs (workshop p48).
import re
s=re.sub(r'\s*QMessageBox::information\(this, "Succès", " Employé (ajouté|modifié|supprimé)"\);','',s)
write(p,s)
# Add source files to qmake.
p='sources.pri';s=read(p);s+='\nSOURCES += $$PWD/src/models/employeequeries.cpp\nHEADERS += $$PWD/src/models/employeequeries.h $$PWD/src/models/modelrows.h\n';write(p,s)
