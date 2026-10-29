#include "medicalrecord.h"
#include <QSqlError>
#include <QDebug>

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

// ---------------------------------------------------------------- AJOUT (create)
bool MedicalRecord::ajouter()
{
    QSqlQuery query;

    //prepare() prend la requête en paramètre pour la préparer à l'exécution.
    query.prepare("INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, ID_ANIMAL, "
                  "NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES) "
                  "VALUES (:id_dossier, :date_consultation, :motif, :diagnostic, :id_animal, "
                  ":nom_animal, :espece, :race, :age, :poids, :proprietaire, :id_employes)");

    //Création des variables liées
    query.bindValue(":id_dossier", idDossier);
    query.bindValue(":date_consultation", dateConsultation);
    query.bindValue(":motif", motif);
    query.bindValue(":diagnostic", diagnostic);
    query.bindValue(":id_animal", idAnimal);
    query.bindValue(":nom_animal", nomAnimal);
    query.bindValue(":espece", espece);
    query.bindValue(":race", race);
    query.bindValue(":age", age);
    query.bindValue(":poids", poids);
    query.bindValue(":proprietaire", proprietaire);
    query.bindValue(":id_employes", idEmployes);

    bool test = query.exec(); //exec() envoie la requête pour l'exécuter
    if (!test)
        qDebug() << "INSERT MEDICAL :" << query.lastError().text();
    return test;
}

// ---------------------------------------------------------------- AFFICHAGE (read)
QSqlQueryModel * MedicalRecord::afficher()
{
    QSqlQueryModel * model = new QSqlQueryModel();

    model->setQuery("SELECT ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, ID_ANIMAL, NOM_ANIMAL, "
                    "ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES "
                    "FROM MEDICAL ORDER BY DATE_CONSULTATION DESC");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID dossier"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Date consultation"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Motif"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Diagnostic"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("ID animal"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Nom animal"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Espèce"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Race"));
    model->setHeaderData(8, Qt::Horizontal, QObject::tr("Âge"));
    model->setHeaderData(9, Qt::Horizontal, QObject::tr("Poids"));
    model->setHeaderData(10, Qt::Horizontal, QObject::tr("Propriétaire"));
    model->setHeaderData(11, Qt::Horizontal, QObject::tr("ID employé"));

    return model;
}

// ---------------------------------------------------------------- SUPPRESSION (delete)
bool MedicalRecord::supprimer(QString idDossier)
{
    QSqlQuery query;

    query.prepare("DELETE FROM MEDICAL WHERE ID_DOSSIER = :id_dossier");
    query.bindValue(":id_dossier", idDossier);

    return query.exec();
}

// ---------------------------------------------------------------- MODIFICATION (update)
bool MedicalRecord::modifier()
{
    QSqlQuery query;

    query.prepare("UPDATE MEDICAL SET DATE_CONSULTATION = :date_consultation, MOTIF = :motif, "
                  "DIAGNOSTIC = :diagnostic, ID_ANIMAL = :id_animal, NOM_ANIMAL = :nom_animal, "
                  "ESPECE = :espece, RACE = :race, AGE = :age, POIDS = :poids, "
                  "PROPRIETAIRE = :proprietaire, ID_EMPLOYES = :id_employes "
                  "WHERE ID_DOSSIER = :id_dossier");

    //Création des variables liées
    query.bindValue(":id_dossier", idDossier);
    query.bindValue(":date_consultation", dateConsultation);
    query.bindValue(":motif", motif);
    query.bindValue(":diagnostic", diagnostic);
    query.bindValue(":id_animal", idAnimal);
    query.bindValue(":nom_animal", nomAnimal);
    query.bindValue(":espece", espece);
    query.bindValue(":race", race);
    query.bindValue(":age", age);
    query.bindValue(":poids", poids);
    query.bindValue(":proprietaire", proprietaire);
    query.bindValue(":id_employes", idEmployes);

    return query.exec();
}
