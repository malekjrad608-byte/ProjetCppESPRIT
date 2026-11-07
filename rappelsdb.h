#ifndef RAPPELSDB_H
#define RAPPELSDB_H

// Rappels vaccinaux et rendez-vous stockés DANS la table MEDICAL (aucune nouvelle table).
// Deux colonnes texte optionnelles sont ajoutées à MEDICAL au premier accès si elles manquent :
//   RAPPELS_VACCINS : "vaccin|date_vaccin|date_rappel|date_sms~..."      (plusieurs vaccins par dossier)
//   RAPPELS_RDV     : "date_heure|statut|motif|sms_rappel|sms_manque~..." (plusieurs rendez-vous)
// Le numéro de téléphone du propriétaire reste dans MEDICAL.TELEPHONE.
#include <QString>
#include <QStringList>
#include <QDate>
#include <QDateTime>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace RappelsDb {

inline QString &derniereErreur() { static QString e; return e; }

inline QString lit(const QString &s)
{
    if (s.trimmed().isEmpty())
        return QStringLiteral("NULL");
    QString t = s;
    t.replace('\'', QStringLiteral("''"));
    return QStringLiteral("'") + t + QStringLiteral("'");
}

inline bool exec(const QString &sql)
{
    QSqlQuery q;
    if (q.exec(sql)) {
        derniereErreur().clear();
        return true;
    }
    derniereErreur() = q.lastError().text() + QStringLiteral(" [") + q.lastError().nativeErrorCode() + QStringLiteral("]");
    qDebug() << "SQL rappels :" << derniereErreur() << "\n" << sql;
    return false;
}

inline bool colonneExiste(const QString &col)
{
    QSqlQuery q;
    return q.exec(QStringLiteral("SELECT %1 FROM MEDICAL WHERE 1 = 0").arg(col));
}

// Ajoute les colonnes manquantes à MEDICAL (ALTER TABLE ... ADD, sans toucher aux colonnes existantes).
inline bool assurerTables()
{
    static bool fait = false;
    if (fait)
        return true;
    if (!QSqlDatabase::database().isOpen())
        return true;
    bool ok = true;
    if (!colonneExiste("RAPPELS_VACCINS"))
        ok = exec("ALTER TABLE MEDICAL ADD (RAPPELS_VACCINS VARCHAR2(2000))") && ok;
    if (!colonneExiste("RAPPELS_RDV"))
        ok = exec("ALTER TABLE MEDICAL ADD (RAPPELS_RDV VARCHAR2(2000))") && ok;
    if (!colonneExiste("TELEPHONE"))
        ok = exec("ALTER TABLE MEDICAL ADD (TELEPHONE VARCHAR2(20))") && ok;
    fait = ok;
    return ok;
}

struct Ligne {
    qlonglong idDossier = 0;
    qlonglong refAnimal = 0;
    QString nom, espece, proprietaire, telephone, vaccins, rdv;
};

inline QString idAnimalTexte(qlonglong ref) { return QString("ANI-%1").arg(ref, 3, 10, QChar('0')); }

inline QList<Ligne> lireLignes()
{
    QList<Ligne> out;
    if (!assurerTables())
        return out;
    QSqlQuery q;
    if (!q.exec("SELECT ID_DOSSIER, REF_ANIMAL, NOM_ANIMAL, ESPECE, PROPRIETAIRE, TELEPHONE, RAPPELS_VACCINS, RAPPELS_RDV "
                "FROM MEDICAL WHERE REF_ANIMAL IS NOT NULL ORDER BY ID_DOSSIER")) {
        derniereErreur() = q.lastError().text();
        qDebug() << "SELECT rappels :" << derniereErreur();
        return out;
    }
    while (q.next()) {
        Ligne l;
        l.idDossier = q.value(0).toLongLong();
        l.refAnimal = q.value(1).toLongLong();
        l.nom = q.value(2).toString();
        l.espece = q.value(3).toString();
        l.proprietaire = q.value(4).toString();
        l.telephone = q.value(5).toString();
        l.vaccins = q.value(6).toString();
        l.rdv = q.value(7).toString();
        out << l;
    }
    return out;
}

inline QStringList entrees(const QString &texte)
{
    return texte.trimmed().isEmpty() ? QStringList() : texte.split('~', Qt::SkipEmptyParts);
}

inline QString nettoyer(QString s) { s.replace('|', ' ').replace('~', ' ').replace('\'', QChar(0x2019)); return s.simplified(); }

// Dossier le plus récent de l'animal (celui qui porte les rappels). Renvoie false si l'animal n'a pas de dossier.
inline bool dossierDeAnimal(const QString &idAnimal, Ligne &out)
{
    bool found = false;
    const QString num = idAnimal.section('-', 1);
    for (const Ligne &l : lireLignes())
        if (idAnimalTexte(l.refAnimal).compare(idAnimal, Qt::CaseInsensitive) == 0 || (!num.isEmpty() && l.refAnimal == num.toLongLong())) {
            out = l;
            found = true;      // lignes triées par ID_DOSSIER : on garde le dernier
        }
    if (!found)
        derniereErreur() = QStringLiteral("Aucun dossier médical pour l'animal %1.").arg(idAnimal);
    return found;
}

inline bool ecrireColonne(qlonglong idDossier, const QString &colonne, QStringList lignes)
{
    QString texte = lignes.join('~');
    while (texte.size() > 1900 && !lignes.isEmpty()) {      // garde les entrées les plus récentes
        lignes.removeFirst();
        texte = lignes.join('~');
    }
    return exec(QString("UPDATE MEDICAL SET %1 = %2 WHERE ID_DOSSIER = %3").arg(colonne, lit(texte)).arg(idDossier));
}

inline bool ecrireTelephone(qlonglong idDossier, const QString &tel)
{
    return exec(QString("UPDATE MEDICAL SET TELEPHONE = %1 WHERE ID_DOSSIER = %2").arg(lit(tel.left(20))).arg(idDossier));
}

} // namespace RappelsDb

#endif // RAPPELSDB_H
