from pathlib import Path
for file,fields in [('src/medical/medicalrecorddialog.cpp',['m_local','m_service','m_employee']),('src/locaux/localdialog.cpp',['m_service'])]:
 p=Path(file);s=p.read_text(encoding='utf-8-sig')
 for name in fields:s=s.replace(name+'=new QComboBox;',name+'=new QComboBox(this);')
 p.write_text(s,encoding='utf-8')
p=Path('src/dialogs/servicedialog.cpp');s=p.read_text(encoding='utf-8-sig').replace('Nom du nouvel employé','Nom').replace('Prénom du nouvel employé','Prénom');p.write_text(s,encoding='utf-8')
p=Path('src/employees/gestionemployes.cpp');s=p.read_text(encoding='utf-8-sig');old='qDel.lastError().nativeErrorCode().contains("2292") ? QStringLiteral("Cet employé possède un planning ou des services associés. Son historique est conservé. Vous pouvez le rendre indisponible.") : qDel.lastError().text()';s=s.replace(old,'OracleConnection::isOracle()?FiveTableStore::friendlyError(qDel.lastError().text()):qDel.lastError().text()');s=s.replace('auto *serviceChoice=new QComboBox;','auto *serviceChoice=new QComboBox(&dlg);');p.write_text(s,encoding='utf-8')
p=Path('src/database/fivetablestore.cpp');s=p.read_text(encoding='utf-8-sig');s=s.replace('q.prepare("UPDATE services SET id_employe=? WHERE id=? AND id_employe=?");q.addBindValue(replacement);q.addBindValue(previous);q.addBindValue(employee);','q.prepare("UPDATE services SET id_employe=?,responsible=CASE WHEN invoice_status=\'BROUILLON\' THEN (SELECT TRIM(nom||\' \'||prenom) FROM employes WHERE id=?) ELSE responsible END WHERE id=? AND id_employe=?");q.addBindValue(replacement);q.addBindValue(replacement);q.addBindValue(previous);q.addBindValue(employee);');s=s.replace('if(!q.exec())return fail(q.lastError().text());\n    if(!db.commit())return fail(db.lastError().text());\n    error->clear();return true;','''if(!q.exec())return fail(q.lastError().text());
    if(id>0){
        q.prepare("UPDATE services SET responsible=? WHERE id_employe=? AND invoice_status='BROUILLON'");
        q.addBindValue(values[0].toString()+" "+values[1].toString());q.addBindValue(id);
        if(!q.exec())return fail(q.lastError().text());
    }
    if(!db.commit())return fail(db.lastError().text());
    error->clear();return true;''');p.write_text(s,encoding='utf-8')
