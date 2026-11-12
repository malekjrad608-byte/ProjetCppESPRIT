#include "lookupdata.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>

// ------------------------------------------------------------------
// A ADAPTER aux tables des autres modules (une seule ligne à changer par requête)
// ------------------------------------------------------------------
// Colonnes attendues : 1) identifiant  2) texte affiché  [3) quantité pour le stock]
// Employés du module « Gestion des employés » : table EMPLOYER (ID_EMP, NOM, PRENOM).
// Si elle n'existe pas, on retombe sur l'ancienne table EMPLOYES (ID ou ID_EMPLOYES).
// Les employés affichés dans le formulaire sont donc ceux créés dans « Gestion des employés ».
static const char *SQL_EMPLOYER =
    "SELECT ID_EMP, NVL(TRIM(NOM || ' ' || PRENOM), 'Employé n° ' || ID_EMP) FROM EMPLOYER ORDER BY NOM, PRENOM, ID_EMP";
static const char *SQL_EMPLOYES_ID =
    "SELECT ID, NVL(TRIM(NOM || ' ' || PRENOM), 'Employé n° ' || ID) FROM EMPLOYES ORDER BY NOM, PRENOM, ID";
static const char *SQL_EMPLOYES_ID_EMPLOYES =
    "SELECT ID_EMPLOYES, NVL(TRIM(NOM || ' ' || PRENOM), 'Employé n° ' || ID_EMPLOYES) FROM EMPLOYES ORDER BY NOM, PRENOM, ID_EMPLOYES";

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

// Table et colonne réellement référencées par la clé étrangère FK_05 de MEDICAL (lues dans le dictionnaire
// Oracle : aucune table ni contrainte n'est modifiée). Le formulaire propose ainsi exactement les employés
// que la base accepte, ce qui évite l'erreur ORA-02291.
static bool cibleCleEmployes(QString &table, QString &colonne)
{
    QSqlQuery q;
    q.prepare("SELECT rcc.table_name, rcc.column_name "
              "FROM user_constraints c "
              "JOIN user_constraints rc ON rc.constraint_name = c.r_constraint_name "
              "JOIN user_cons_columns rcc ON rcc.constraint_name = rc.constraint_name "
              "WHERE c.constraint_name = 'FK_05' AND c.table_name = 'MEDICAL'");
    if (!q.exec() || !q.next())
        return false;
    table = q.value(0).toString().trimmed().toUpper();
    colonne = q.value(1).toString().trimmed().toUpper();
    const QRegularExpression ident("^[A-Z][A-Z0-9_$#]*$");
    return ident.match(table).hasMatch() && ident.match(colonne).hasMatch();
}

QList<LookupItem> LookupData::employes()
{
    QList<LookupItem> list;
    if (QSqlDatabase::database().isOpen()) {
        // 1) table référencée par FK_05 (ex. EMPLOYES) : c'est la seule qui accepte l'insertion dans MEDICAL
        QString table, colonne;
        if (cibleCleEmployes(table, colonne)) {
            const QString sql = QString("SELECT %1, NVL(TRIM(NOM || ' ' || PRENOM), 'Employé n° ' || %1) "
                                        "FROM %2 ORDER BY NOM, PRENOM, %1").arg(colonne, table);
            // On ne s'arrete que si la table de FK_05 contient vraiment des employes ;
            // sinon (table vide) on passe aux autres tables pour ne pas afficher une liste vide.
            if (lire(sql.toUtf8().constData(), list) && !list.isEmpty())
                return list;
            qDebug() << "LookupData : table" << table << "(cle FK_05) vide ou illisible, essai de EMPLOYER";
            list.clear();
        }
        // 2) FK_05 introuvable : on essaie les tables connues, la première qui contient des employés
        const char *sources[] = { SQL_EMPLOYER, SQL_EMPLOYES_ID, SQL_EMPLOYES_ID_EMPLOYES };
        for (const char *sql : sources) {
            list.clear();
            if (lire(sql, list) && !list.isEmpty())
                break;
        }
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
