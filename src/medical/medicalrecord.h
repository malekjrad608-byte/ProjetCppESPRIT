#ifndef MEDICALRECORD_H
#define MEDICALRECORD_H

#include <QDate>
#include <QSqlDatabase>
#include <QString>
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
public:
  MedicalRecord();
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
