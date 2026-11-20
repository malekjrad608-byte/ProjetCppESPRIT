#ifndef MEDICALRECORD_H
#define MEDICALRECORD_H

#include <QDate>
#include <QSqlDatabase>
#include <QString>
#include <QVariant>
#include <QRegularExpression>
#include <QVector>

struct AnimalInfo {
  QString animalId;
  QString name;
  QString species;
  QString breed;
  int age = 0;
  QString sex;
  QString owner;
};

struct MedicalInfo {
  QString recordId;
  int localId = 0;
  int serviceId = 0;
  int employeeId = 0;
  QDate consultationDate;
  QString reason;
  QString diagnosis;
  QString treatment;
  QString vaccinations;
  QString allergies;
  double weight = 0;
  QString observations;
  QString vaccinationStatus; // "À jour" or "À renouveler"
};

class MedicalRecord {
private:
  QString telephone;
public:
  QString getTelephone() const { return telephone; }
  void setTelephone(const QString &value) { telephone=value; }
  bool saveSupplement(QString *error=nullptr, bool weightSavedInOracle=false) const;
  MedicalRecord();
  // API de la classe metier importee ; les identifiants Oracle restent generes en base.
  MedicalRecord(QString id, QDate date, QString motif, QString diagnostic,
                QString animalId, QString nom, QString espece, QString race,
                int age, double poids, QString proprietaire, QString employe) {
    setIdDossier(id); medical.consultationDate=date; medical.reason=motif;
    medical.diagnosis=diagnostic; animal={animalId,nom,espece,race,age,QString(),proprietaire};
    medical.weight=poids; medical.employeeId=employe.toInt();
  }
  static QVariant versNombre(const QString &s) {
    auto m=QRegularExpression("(\\d+)$").match(s.trimmed());
    return m.hasMatch()?QVariant(m.captured(1).toLongLong()):QVariant();
  }
  static QString versTexte(const QVariant &v,const QString &prefix) {
    return v.isNull()?QString():QString("%1-%2").arg(prefix).arg(v.toLongLong(),3,10,QChar('0'));
  }
  QString getIdDossier() const { return medical.recordId; }
  void setIdDossier(const QString &v) { medical.recordId=v; storageId=versNombre(v).toLongLong(); }
  QDate getDateConsultation() const { return medical.consultationDate; }
  void setDateConsultation(QDate v) { medical.consultationDate=v; }
  QString getMotif() const { return medical.reason; }
  void setMotif(QString v) { medical.reason=v; }
  QString getDiagnostic() const { return medical.diagnosis; }
  void setDiagnostic(QString v) { medical.diagnosis=v; }
  QString getIdAnimal() const { return animal.animalId; }
  void setIdAnimal(QString v) { animal.animalId=v; }
  QString getNomAnimal() const { return animal.name; }
  void setNomAnimal(QString v) { animal.name=v; }
  QString getEspece() const { return animal.species; }
  void setEspece(QString v) { animal.species=v; }
  QString getRace() const { return animal.breed; }
  void setRace(QString v) { animal.breed=v; }
  int getAge() const { return animal.age; }
  void setAge(int v) { animal.age=v; }
  double getPoids() const { return medical.weight; }
  void setPoids(double v) { medical.weight=v; }
  QString getProprietaire() const { return animal.owner; }
  void setProprietaire(QString v) { animal.owner=v; }
  QString getSexe() const { return animal.sex; }
  void setSexe(QString v) { animal.sex=v; }
  QString getTraitement() const { return medical.treatment; }
  void setTraitement(QString v) { medical.treatment=v; }
  QString getVaccinations() const { return medical.vaccinations; }
  void setVaccinations(QString v) { medical.vaccinations=v; }
  QString getAllergies() const { return medical.allergies; }
  void setAllergies(QString v) { medical.allergies=v; }
  QString getObservations() const { return medical.observations; }
  void setObservations(QString v) { medical.observations=v; }
  QString getStatutVaccination() const { return medical.vaccinationStatus; }
  void setStatutVaccination(QString v) { medical.vaccinationStatus=v; }
  QString getIdEmployes() const { return QString::number(medical.employeeId); }
  void setIdEmployes(QString v) { medical.employeeId=v.toInt(); }

  bool ajouter(QString *error = nullptr);
  bool modifier(QString *error = nullptr);
  static QVector<MedicalRecord> afficher(QString *error = nullptr);
  static bool supprimer(qint64 id, QString *error = nullptr);
  static QString proprietaire(qint64 id, QString *error);
  static QList<QPair<qint64, QString>> proprietaires(QString *error);

  MedicalRecord(const AnimalInfo &animal, const MedicalInfo &medical);

  // Requetes, transactions et import de l'entite medicale.
  static bool initialize(QSqlDatabase db, QString *error);
  static bool enregistrer(MedicalRecord &record, QString *error = nullptr);
  static QVector<MedicalRecord> examples();
  static bool seedExamples(QString *error = nullptr);
  static bool importInitial(QString *error = nullptr);
  static QString reportHtml(const QVector<MedicalRecord> &records,
                            bool demo = false);

  qint64 storageId = 0;
  AnimalInfo animal;
  MedicalInfo medical;

  QString getFullName() const;
  QString getVaccinationStatus() const;

  bool operator==(const MedicalRecord &other) const;
};

#endif // MEDICALRECORD_H
