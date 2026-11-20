#pragma once
#include "../database/oracleconnection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>

// Shared Oracle connection; existing MEDICAL columns only.
class RappelsDb {
public:
    struct Ligne {
        qlonglong idDossier = 0;
        QString refAnimal, nom, espece, proprietaire, telephone, vaccins, rdv;
    };
    static QString &derniereErreur() { static QString error; return error; }
    static bool assurerTables() {
        QSqlQuery q(OracleConnection::database());
        const bool ok = q.exec("SELECT RAPPELS_VACCINS,RAPPELS_RDV,TELEPHONE FROM MEDICAL WHERE 1=0");
        derniereErreur() = ok ? QString() : q.lastError().text();
        return ok;
    }
    static QString idAnimalTexte(const QString &ref) { return ref; }
    static QStringList entrees(const QString &text) { return text.split('~', Qt::SkipEmptyParts); }
    static QString nettoyer(QString text) { return text.replace('|',' ').replace('~',' ').simplified(); }
    static QList<Ligne> lireLignes() {
        QList<Ligne> rows;
        QSqlQuery q(OracleConnection::database());
        if (!q.exec("SELECT ID_DOSS,RAF_ANIMAL,NOM_ANIMAL,ESPECE,PROPRIETAIRE,TELEPHONE,"
                    "RAPPELS_VACCINS,RAPPELS_RDV FROM MEDICAL ORDER BY ID_DOSS")) {
            derniereErreur()=q.lastError().text();return rows;
        }
        while(q.next()) rows.append({q.value(0).toLongLong(),q.value(1).toString(),q.value(2).toString(),
            q.value(3).toString(),q.value(4).toString(),q.value(5).toString(),q.value(6).toString(),q.value(7).toString()});
        derniereErreur().clear();return rows;
    }
    static bool dossierDeAnimal(const QString &ref,Ligne &out) {
        bool found=false;
        for(const auto &row:lireLignes()) if(row.refAnimal.compare(ref,Qt::CaseInsensitive)==0) {out=row;found=true;}
        if(!found && derniereErreur().isEmpty()) derniereErreur()="Aucun dossier médical pour l'animal "+ref;
        return found;
    }
    static bool ecrireColonne(qlonglong id,const QString &column,const QStringList &rows) {
        if(column!="RAPPELS_VACCINS" && column!="RAPPELS_RDV") return false;
        const QString text=rows.join('~');
        if(text.toUtf8().size()>2000) {derniereErreur()="La capacité des rappels de ce dossier est atteinte.";return false;}
        return update(id,column,text);
    }
    static bool ecrireTelephone(qlonglong id,const QString &tel) {
        if(tel.size()>20) {derniereErreur()="Le téléphone est limité à 20 caractères.";return false;}
        return update(id,"TELEPHONE",tel);
    }
    static bool ajouterEntree(const QString &ref,const QString &column,const QString &entry,
                              const QString &phone,QString *id) {
        if(column!="RAPPELS_VACCINS" && column!="RAPPELS_RDV") return false;
        auto db=OracleConnection::database();
        if(!db.transaction()) {derniereErreur()=db.lastError().text();return false;}
        auto fail=[&](const QString &error){db.rollback();derniereErreur()=error;return false;};
        QSqlQuery q(db);
        q.prepare("SELECT ID_DOSS,"+column+" FROM MEDICAL WHERE RAF_ANIMAL=:ref FOR UPDATE WAIT 5");
        q.bindValue(":ref",ref);
        if(!q.exec()) return fail(q.lastError().text());
        if(!q.next()) return fail("Dossier médical introuvable.");
        const auto key=q.value(0).toLongLong();
        auto rows=entrees(q.value(1).toString());q.finish();
        if(column=="RAPPELS_RDV") for(auto &row:rows) {
            auto fields=row.split('|');
            if(fields.size()>1 && fields[1]=="Prévu") {fields[1]="Annulé";row=fields.join('|');}
        }
        rows.append(entry);
        if(!ecrireColonne(key,column,rows)) return fail(derniereErreur());
        if(!phone.trimmed().isEmpty() && !ecrireTelephone(key,phone)) return fail(derniereErreur());
        if(!db.commit()) return fail(db.lastError().text());
        *id=QString("%1:%2").arg(key).arg(rows.size()-1);return true;
    }
private:
    static bool update(qlonglong id,const QString &column,const QString &value) {
        QSqlQuery q(OracleConnection::database());
        q.prepare("UPDATE MEDICAL SET "+column+"=:value WHERE ID_DOSS=:id");
        q.bindValue(":value",value);q.bindValue(":id",QString::number(id));
        if(!q.exec()) {derniereErreur()=q.lastError().text();return false;}
        if(q.numRowsAffected()!=1) {derniereErreur()="Dossier médical introuvable.";return false;}
        derniereErreur().clear();return true;
    }
};
