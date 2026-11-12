#include "medicalrecord.h"
#include <QSqlError>
#include <QSqlDatabase>
#include <QMap>
#include <QVector>
#include <QStringList>
#include <QSqlQuery>
#include <QRegularExpression>
#include <QDebug>

namespace {
QString g_derniereErreur;

void memoriserErreur(const char *contexte, const QSqlError &erreur)
{
    g_derniereErreur = QString("%1\n[driver : %2 | base : %3 | code : %4]")
                           .arg(erreur.text().trimmed(), erreur.driverText().trimmed(),
                                erreur.databaseText().trimmed(), erreur.nativeErrorCode());
    if (g_derniereErreur.contains("FK_05") || g_derniereErreur.contains("ORA-02291"))
        g_derniereErreur += QString::fromUtf8("\n\nL'employé choisi n'existe pas dans la table référencée par MEDICAL (clé FK_05). "
                                              "Exécutez sql/integration_fk_employer.sql pour relier MEDICAL.ID_EMPLOYES à EMPLOYER(ID_EMP).");
    qDebug() << contexte << ":" << g_derniereErreur;
}

// valeur SQL NULL typée (évite les erreurs de liaison avec un QVariant vide)
QVariant nullDate()   { return QVariant(QMetaType(QMetaType::QDate)); }
}

MedicalRecord::MedicalRecord(QString idDossier, QDate dateConsultation, QString motif, QString diagnostic,
                             QString idAnimal, QString nomAnimal, QString espece, QString race,
                             int age, double poids, QString proprietaire, QString idEmployes)
{
    this->idDossier = idDossier;
    this->dateConsultation = dateConsultation;
    this->motif = motif;
    this->diagnostic = diagnostic;
    this->idAnimal = idAnimal;
    this->nomAnimal = nomAnimal;
    this->espece = espece;
    this->race = race;
    this->age = age;
    this->poids = poids;
    this->proprietaire = proprietaire;
    this->idEmployes = idEmployes;
}

// ---------------------------------------------------------------- conversions / utilitaires
QVariant MedicalRecord::versNombre(const QString &identifiant)
{
    // On prend le nombre final : "MED-012" -> 12, "ANI-005" -> 5, "7" -> 7. Sans nombre : NULL.
    static const QRegularExpression fin("(\\d+)\\s*$");
    const QRegularExpressionMatch m = fin.match(identifiant);
    if (!m.hasMatch() || m.captured(1).size() > 18)
        return QVariant(QMetaType(QMetaType::LongLong));
    return QVariant(m.captured(1).toLongLong());
}

QString MedicalRecord::versTexte(const QVariant &nombre, const QString &prefixe)
{
    if (!nombre.isValid() || nombre.isNull())
        return QString();
    return QString("%1-%2").arg(prefixe).arg(nombre.toLongLong(), 3, 10, QChar('0'));
}

QString MedicalRecord::derniereErreur()
{
    return g_derniereErreur;
}

bool MedicalRecord::colonneExiste(const QString &colonne)
{
    // Le nom vient du code (SEXE, TRAITEMENT), jamais de l'utilisateur. Test sans lire de données.
    static QMap<QString, bool> cache;
    if (!QSqlDatabase::database().isOpen())
        return false;
    if (cache.contains(colonne))
        return cache.value(colonne);
    QSqlQuery query;
    const bool existe = query.exec(QString("SELECT %1 FROM MEDICAL WHERE 1 = 0").arg(colonne));
    if (!existe)
        qDebug() << "Colonne MEDICAL." << colonne << "absente : champ gardé en mémoire seulement";
    cache.insert(colonne, existe);
    return existe;
}

qlonglong MedicalRecord::prochainIdDossier()
{
    QSqlQuery query;
    if (query.exec("SELECT NVL(MAX(ID_DOSSIER), 0) + 1 FROM MEDICAL") && query.next())
        return query.value(0).toLongLong();
    memoriserErreur("MAX(ID_DOSSIER)", query.lastError());
    return -1;
}

// ---------------------------------------------------------------- champs communs INSERT / UPDATE
// Les valeurs sont envoyées au driver QODBC sous forme de texte converti côté Oracle (TO_NUMBER, TO_DATE, TO_CLOB) :
// le driver ODBC d'Oracle gère mal les types BIGINT, TIMESTAMP et CLOB liés directement ("QODBC: unknown error").
namespace {
struct Champ {
    QString nom;          // nom du paramètre (sans :)
    QString colonne;      // colonne Oracle
    QString expr;         // expression SQL avec le paramètre lié
    QVariant valeur;      // valeur liée
    QString litteral;     // même valeur écrite en littéral SQL (repli si la liaison échoue)
};

QString chaineSql(const QString &t)
{
    QString e = t;
    e.replace('\'', "''");
    return "'" + e + "'";
}

QString clobSql(const QString &t)
{
    if (t.isEmpty())
        return "NULL";
    QStringList morceaux;                      // un littéral Oracle est limité à 4000 octets : on découpe
    for (int i = 0; i < t.size(); i += 1000)
        morceaux << "TO_CLOB(" + chaineSql(t.mid(i, 1000)) + ")";
    return morceaux.join(" || ");
}

QString nombreSql(const QVariant &v)
{
    return v.isNull() ? QString("NULL") : QString::number(v.toLongLong());
}

// Colonnes facultatives de MEDICAL : utilisées seulement si elles existent (script ajouter_colonnes_optionnelles.sql)
struct ColOpt {
    const char *nom;                              // paramètre
    const char *colonne;                          // colonne Oracle
    int taille;                                   // VARCHAR2(taille)
    QString (*lire)(const MedicalRecord &);       // valeur du dossier
};
const QVector<ColOpt> &colonnesOptionnelles()
{
    static const QVector<ColOpt> liste = {
        {"sexe",               "SEXE",               10,  [](const MedicalRecord &r) { return r.getSexe(); }},
        {"traitement",         "TRAITEMENT",         500, [](const MedicalRecord &r) { return r.getTraitement(); }},
        {"vaccinations",       "VACCINATIONS",       200, [](const MedicalRecord &r) { return r.getVaccinations(); }},
        {"allergies",          "ALLERGIES",          200, [](const MedicalRecord &r) { return r.getAllergies(); }},
        {"observations",       "OBSERVATIONS",       500, [](const MedicalRecord &r) { return r.getObservations(); }},
        {"statut_vaccination", "STATUT_VACCINATION", 30,  [](const MedicalRecord &r) { return r.getStatutVaccination(); }},
        {"telephone",          "TELEPHONE",          20,  [](const MedicalRecord &r) { return r.getTelephone(); }},
    };
    return liste;
}

QVector<Champ> champsDe(const MedicalRecord &r)
{
    QVector<Champ> c;
    const QVariant idD = MedicalRecord::versNombre(r.getIdDossier());
    const QVariant ref = MedicalRecord::versNombre(r.getIdAnimal());
    const QVariant emp = MedicalRecord::versNombre(r.getIdEmployes());
    const QString date = r.getDateConsultation().isValid() ? r.getDateConsultation().toString("yyyy-MM-dd") : QString();

    c << Champ{"id_dossier", "ID_DOSSIER", "TO_NUMBER(:id_dossier)", idD.isNull() ? QString() : QString::number(idD.toLongLong()), nombreSql(idD)};
    c << Champ{"date_consultation", "DATE_CONSULTATION", "TO_DATE(:date_consultation, 'YYYY-MM-DD')", date,
               date.isEmpty() ? QString("NULL") : "TO_DATE('" + date + "', 'YYYY-MM-DD')"};
    c << Champ{"motif", "MOTIF", ":motif", r.getMotif(), chaineSql(r.getMotif())};
    c << Champ{"diagnostic", "DIAGNOSTIC", "TO_CLOB(:diagnostic)", r.getDiagnostic(), clobSql(r.getDiagnostic())};
    c << Champ{"ref_animal", "REF_ANIMAL", "TO_NUMBER(:ref_animal)", ref.isNull() ? QString() : QString::number(ref.toLongLong()), nombreSql(ref)};
    c << Champ{"nom_animal", "NOM_ANIMAL", ":nom_animal", r.getNomAnimal(), chaineSql(r.getNomAnimal())};
    c << Champ{"espece", "ESPECE", ":espece", r.getEspece(), chaineSql(r.getEspece())};
    c << Champ{"race", "RACE", ":race", r.getRace(), chaineSql(r.getRace())};
    c << Champ{"age", "AGE", ":age", r.getAge(), QString::number(r.getAge())};
    c << Champ{"poids", "POIDS", ":poids", r.getPoids(), QString::number(r.getPoids(), 'f', 2)};
    c << Champ{"proprietaire", "PROPRIETAIRE", ":proprietaire", r.getProprietaire(), chaineSql(r.getProprietaire())};
    c << Champ{"id_employes", "ID_EMPLOYES", "TO_NUMBER(:id_employes)", emp.isNull() ? QString() : QString::number(emp.toLongLong()), nombreSql(emp)};
    for (const ColOpt &o : colonnesOptionnelles()) {
        if (!MedicalRecord::colonneExiste(o.colonne))
            continue;
        const QString v = o.lire(r).left(o.taille);       // jamais plus long que la colonne
        c << Champ{o.nom, o.colonne, QString(":") + o.nom, v, chaineSql(v)};
    }
    return c;
}

// Exécute l'ordre avec paramètres liés ; si le driver refuse, deuxième essai avec des littéraux SQL
// (toutes les valeurs sont échappées : apostrophes doublées).
bool executer(const QString &sqlLie, const QString &sqlLitteral, const QVector<Champ> &champs, const char *contexte)
{
    QSqlQuery query;
    query.prepare(sqlLie);
    for (const Champ &ch : champs)
        query.bindValue(":" + ch.nom, ch.valeur);
    if (query.exec())
        return true;
    const QSqlError premiere = query.lastError();
    qDebug() << contexte << "liaison refusée :" << premiere.text() << "-> essai en littéraux";

    QSqlQuery q2;
    if (q2.exec(sqlLitteral))
        return true;
    memoriserErreur(contexte, q2.lastError().text().trimmed().isEmpty() ? premiere : q2.lastError());
    return false;
}
}

// ---------------------------------------------------------------- AJOUT (create)
bool MedicalRecord::ajouter()
{
    g_derniereErreur.clear();

    const QVector<Champ> champs = champsDe(*this);
    QStringList cols, lies, lits;
    for (const Champ &ch : champs) {
        cols << ch.colonne;
        lies << ch.expr;
        lits << ch.litteral;
    }
    const QString debut = "INSERT INTO MEDICAL (" + cols.join(", ") + ") VALUES (";
    return executer(debut + lies.join(", ") + ")", debut + lits.join(", ") + ")", champs, "INSERT MEDICAL");
}

// ---------------------------------------------------------------- AFFICHAGE (read)
QSqlQueryModel * MedicalRecord::afficher()
{
    QSqlQueryModel * model = new QSqlQueryModel();

    // Colonnes facultatives : lues seulement si elles existent (sinon NULL ; valeurs gardées en mémoire)
    QString facultatives;
    for (const ColOpt &o : colonnesOptionnelles()) {
        const QString col = QString::fromLatin1(o.colonne);
        facultatives += ", " + (colonneExiste(col) ? col : QString("CAST(NULL AS VARCHAR2(%1)) AS %2").arg(o.taille).arg(col));
    }

    model->setQuery("SELECT ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, "
                    "ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES" + facultatives + " "
                    "FROM MEDICAL ORDER BY DATE_CONSULTATION DESC");
    if (model->lastError().isValid())
        memoriserErreur("SELECT MEDICAL", model->lastError());
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID dossier"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Date consultation"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Motif"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Diagnostic"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Réf. animal"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Nom animal"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Espèce"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Race"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Âge"));
    model->setHeaderData(9, Qt::Horizontal, QObject::tr("Poids"));
    model->setHeaderData(10, Qt::Horizontal, QObject::tr("Propriétaire"));
    model->setHeaderData(11, Qt::Horizontal, QObject::tr("ID employé"));
    model->setHeaderData(12, Qt::Horizontal, QObject::tr("Sexe"));
    model->setHeaderData(13, Qt::Horizontal, QObject::tr("Traitement"));
    model->setHeaderData(14, Qt::Horizontal, QObject::tr("Vaccinations"));
    model->setHeaderData(15, Qt::Horizontal, QObject::tr("Allergies"));
    model->setHeaderData(16, Qt::Horizontal, QObject::tr("Observations"));
    model->setHeaderData(17, Qt::Horizontal, QObject::tr("Statut vaccination"));
    model->setHeaderData(18, Qt::Horizontal, QObject::tr("Téléphone"));

    return model;
}

// ---------------------------------------------------------------- SUPPRESSION (delete)
bool MedicalRecord::supprimer(QString idDossier)
{
    g_derniereErreur.clear();
    QSqlQuery query;

    const QVariant n = versNombre(idDossier);
    query.prepare("DELETE FROM MEDICAL WHERE ID_DOSSIER = TO_NUMBER(:id_dossier)");
    query.bindValue(":id_dossier", n.isNull() ? QString() : QString::number(n.toLongLong()));

    const bool test = query.exec();
    if (!test)
        memoriserErreur("DELETE MEDICAL", query.lastError());
    return test;
}

// ---------------------------------------------------------------- MODIFICATION (update)
bool MedicalRecord::modifier()
{
    g_derniereErreur.clear();

    const QVector<Champ> champs = champsDe(*this);
    QStringList lies, lits;
    QString idLie, idLit;
    for (const Champ &ch : champs) {
        if (ch.nom == "id_dossier") { idLie = ch.expr; idLit = ch.litteral; continue; }
        lies << ch.colonne + " = " + ch.expr;
        lits << ch.colonne + " = " + ch.litteral;
    }
    return executer("UPDATE MEDICAL SET " + lies.join(", ") + " WHERE ID_DOSSIER = " + idLie,
                    "UPDATE MEDICAL SET " + lits.join(", ") + " WHERE ID_DOSSIER = " + idLit,
                    champs, "UPDATE MEDICAL");
}
