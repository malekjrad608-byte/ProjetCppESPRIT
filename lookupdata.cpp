#include "lookupdata.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// ------------------------------------------------------------------
// A ADAPTER aux tables des autres modules (une seule ligne à changer par requête)
// ------------------------------------------------------------------
// Colonnes attendues : 1) identifiant  2) texte affiché  [3) quantité pour le stock]
static const char *SQL_EMPLOYES =
    "SELECT ID_EMPLOYES, NVL(TRIM(NOM || ' ' || PRENOM), 'Employé n° ' || ID_EMPLOYES) "
    "FROM EMPLOYES ORDER BY NOM, PRENOM, ID_EMPLOYES";

static const char *SQL_PROPRIETAIRES =
    "SELECT ID_CLIENT, NOM || ' ' || PRENOM FROM CLIENTS ORDER BY NOM, PRENOM";

static const char *SQL_STOCK =
    "SELECT ID_ARTICLE, NVL(TRIM(NOM_ARTICLE), 'Article n° ' || ID_ARTICLE), QUANTITE_DISP "
    "FROM STOCK WHERE QUANTITE_DISP > 0 ORDER BY NOM_ARTICLE, ID_ARTICLE";

// Téléphone du propriétaire (module Clients) : :p = identifiant ou nom complet. A adapter.
static const char *SQL_TEL_PROPRIETAIRE =
    "SELECT TELEPHONE FROM CLIENTS WHERE ID_CLIENT = :p OR NOM || ' ' || PRENOM = :p";

// Exécute une requête de lecture (sans paramètre) et remplit la liste.
// Retourne false si la requête échoue (table absente, pas de connexion...).
static bool lire(const char *sql, QList<LookupItem> &out, bool avecQuantite = false)
{
    if (!QSqlDatabase::database().isOpen())
        return false;

    QSqlQuery query;
    query.prepare(QString::fromUtf8(sql));
    if (!query.exec()) {
        qDebug() << "LookupData :" << query.lastError().text();
        return false;
    }
    while (query.next()) {
        LookupItem it;
        it.id = query.value(0).toString();
        it.label = query.value(1).toString().trimmed();
        if (avecQuantite)
            it.quantity = query.value(2).toInt();
        if (it.label.isEmpty())
            it.label = it.id;           // jamais de ligne ignorée : on garde l'ID réel
        if (!it.id.isEmpty())
            out.append(it);
    }
    return true;
}

QList<LookupItem> LookupData::employes()
{
    QList<LookupItem> list;
    if (QSqlDatabase::database().isOpen()) {
        lire(SQL_EMPLOYES, list);       // en ligne : seulement les vrais employés (ID_EMPLOYES est une clé réelle)
        return list;
    }
    // Données d'exemple (mode hors ligne uniquement)
    return { {"EMP-001", "Dr. Ben Ali Sami", -1}, {"EMP-002", "Dr. Trabelsi Salma", -1},
             {"EMP-003", "Gharbi Yassine", -1} };
}

QList<LookupItem> LookupData::proprietaires()
{
    QList<LookupItem> list;
    if (lire(SQL_PROPRIETAIRES, list) && !list.isEmpty())
        return list;
    return { {"CLI-001", "Mansour Ahmed", -1}, {"CLI-002", "Jlassi Mariem", -1},
             {"CLI-003", "Khelifi Omar", -1} };
}

QList<LookupItem> LookupData::traitementsEnStock()
{
    QList<LookupItem> list;
    if (QSqlDatabase::database().isOpen()) {
        lire(SQL_STOCK, list, true);    // en ligne : uniquement le vrai stock (ID_ARTICLE, NOM_ARTICLE, QUANTITE_DISP)
        return list;
    }
    return { {"P-001", "Vaccin antirabique", 20}, {"P-002", "Vermifuge", 35},
             {"P-003", "Antibiotique", 12}, {"P-004", "Anti-inflammatoire", 18},
             {"P-005", "Antiparasitaire externe", 25} };
}

QString LookupData::telephoneProprietaire(const QString &proprietaire)
{
    if (proprietaire.trimmed().isEmpty() || !QSqlDatabase::database().isOpen())
        return QString();
    QSqlQuery query;
    query.prepare(QString::fromUtf8(SQL_TEL_PROPRIETAIRE));
    query.bindValue(":p", proprietaire.trimmed());
    if (!query.exec()) {
        qDebug() << "LookupData (téléphone) :" << query.lastError().text();
        return QString();
    }
    return query.next() ? query.value(0).toString().trimmed() : QString();
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
