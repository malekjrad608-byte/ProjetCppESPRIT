#ifndef MEDICALRECORD_H
#define MEDICALRECORD_H
#include <QString>
#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QVariant>

// Classe purement C++ (Modèle) relative à l'entité "Dossier médical"
// -> table MEDICAL de la base Oracle (comme la classe Etudiant de l'atelier)
class MedicalRecord
{
private:
    // Attributs = colonnes de la table MEDICAL
    QString idDossier;
    QDate dateConsultation;
    QString motif, diagnostic;
    QString idAnimal, nomAnimal, espece, race;
    int age;
    double poids;
    QString proprietaire, idEmployes;

    // Informations facultatives : enregistrées dans MEDICAL seulement si les colonnes existent
    // (voir ajouter_colonnes_optionnelles.sql), sinon gardées en mémoire pendant l'exécution
    QString sexe, traitement, vaccinations, allergies, observations, statutVaccination, telephone;

public:
    //Constructeurs
    MedicalRecord(){ age = 0; poids = 0; }
    MedicalRecord(QString idDossier, QDate dateConsultation, QString motif, QString diagnostic,
                  QString idAnimal, QString nomAnimal, QString espece, QString race,
                  int age, double poids, QString proprietaire, QString idEmployes);

    //Getters
    QString getIdDossier() const { return idDossier; }
    QDate getDateConsultation() const { return dateConsultation; }
    QString getMotif() const { return motif; }
    QString getDiagnostic() const { return diagnostic; }
    QString getIdAnimal() const { return idAnimal; }
    QString getNomAnimal() const { return nomAnimal; }
    QString getEspece() const { return espece; }
    QString getRace() const { return race; }
    int getAge() const { return age; }
    double getPoids() const { return poids; }
    QString getProprietaire() const { return proprietaire; }
    QString getIdEmployes() const { return idEmployes; }
    QString getSexe() const { return sexe; }
    QString getTraitement() const { return traitement; }
    QString getVaccinations() const { return vaccinations; }
    QString getAllergies() const { return allergies; }
    QString getObservations() const { return observations; }
    QString getStatutVaccination() const { return statutVaccination; }
    QString getTelephone() const { return telephone; }

    //Setters
    void setIdDossier(QString id){ idDossier = id; }
    void setDateConsultation(QDate d){ dateConsultation = d; }
    void setMotif(QString m){ motif = m; }
    void setDiagnostic(QString d){ diagnostic = d; }
    void setIdAnimal(QString id){ idAnimal = id; }
    void setNomAnimal(QString n){ nomAnimal = n; }
    void setEspece(QString e){ espece = e; }
    void setRace(QString r){ race = r; }
    void setAge(int a){ age = a; }
    void setPoids(double p){ poids = p; }
    void setProprietaire(QString p){ proprietaire = p; }
    void setIdEmployes(QString id){ idEmployes = id; }
    void setSexe(QString s){ sexe = s; }
    void setTraitement(QString t){ traitement = t; }
    void setVaccinations(QString v){ vaccinations = v; }
    void setAllergies(QString a){ allergies = a; }
    void setObservations(QString o){ observations = o; }
    void setStatutVaccination(QString s){ statutVaccination = s; }
    void setTelephone(QString t){ telephone = t; }

    // Fonctionnalités de Base relatives à l'entité MedicalRecord
    bool ajouter();
    QSqlQueryModel * afficher();
    bool supprimer(QString);
    bool modifier();

    // ---- Adaptation à la table MEDICAL réelle d'Oracle -----------------------------------------
    // Dans la base : ID_DOSSIER, REF_ANIMAL et ID_EMPLOYES sont des NUMBER ; dans l'application les
    // identifiants s'affichent "MED-012" / "ANI-005". Conversion faite ici, à la frontière de la base.
    static QVariant versNombre(const QString &identifiant);                    // "MED-012" -> 12 (NULL si pas de nombre)
    static QString versTexte(const QVariant &nombre, const QString &prefixe);  // 12, "MED" -> "MED-012"
    static qlonglong prochainIdDossier();                                      // MAX(ID_DOSSIER)+1, -1 si échec
    // Colonnes facultatives (SEXE, TRAITEMENT, VACCINATIONS, ALLERGIES, OBSERVATIONS, STATUT_VACCINATION, TELEPHONE) :
    // utilisées seulement si elles existent dans la table
    static bool colonneExiste(const QString &colonne);
    // Dernier message d'erreur Oracle renvoyé par ajouter() / modifier() / supprimer() / afficher()
    static QString derniereErreur();
};

#endif // MEDICALRECORD_H
