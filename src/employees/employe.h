#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QDate>
#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <QVariant>

class Employe {
public:
  Employe();
  // Atelier CRUD: the view calls the entity; all persistence stays in the
  // model.
  bool ajouter(QString *error);
  bool modifier(QString *error) const;
  static bool supprimer(int id, QString *error);
  static QList<Employe> afficher(const QString &filter = {},
                                 const QString &sort = "date",
                                 const QString &job = "Tous les postes",
                                 const QString &status = "Tous",
                                 QString *error = nullptr);
  static bool basculerDisponibilite(int id, QString *error);
  int getServiceId() const { return m_serviceId; }
  void setServiceId(int id) { m_serviceId = id; }

  int getId() const { return m_id; }
  QString getNom() const { return m_nom; }
  QString getPrenom() const { return m_prenom; }
  QString getCourriel() const { return m_courriel; }
  QString getTelephone() const { return m_telephone; }
  QString getPoste() const { return m_poste; }
  QDate getDateEmbauche() const { return m_dateEmbauche; }
  bool isDisponible() const { return m_disponible; }
  double getChargeHebdo() const { return m_chargeHebdo; }
  double getDispoHebdo() const { return m_dispoHebdo; }
  QString getCompetences() const { return m_competences; }
  QString getCin() const { return m_cin; }
  QDate getDateNaissance() const { return m_dateNaissance; }
  QString getAdresse() const { return m_adresse; }
  QString getTypeContrat() const { return m_typeContrat; }
  double getSalaire() const { return m_salaire; }
  void setCin(const QString &v) { m_cin=v; }
  void setDateNaissance(const QDate &v) { m_dateNaissance=v; }
  void setAdresse(const QString &v) { m_adresse=v; }
  void setTypeContrat(const QString &v) { m_typeContrat=v; }
  void setSalaire(double v) { m_salaire=v; }

  void setId(int id) { m_id = id; }
  void setNom(const QString &n) { m_nom = n; }
  void setPrenom(const QString &p) { m_prenom = p; }
  void setCourriel(const QString &c) { m_courriel = c; }
  void setTelephone(const QString &t) { m_telephone = t; }
  void setPoste(const QString &p) { m_poste = p; }
  void setDateEmbauche(const QDate &d) { m_dateEmbauche = d; }
  void setDisponible(bool d) { m_disponible = d; }
  void setChargeHebdo(double c) { m_chargeHebdo = c; }
  void setDispoHebdo(double d) { m_dispoHebdo = d; }
  void setCompetences(const QString &c) { m_competences = c; }

  double tauxCharge() const;
  bool depasseSeuil(double seuil = 0.8) const;

  // Persistance et affectation de l'entite Employe.
  static bool initialize(QSqlDatabase db, QString *error);
  static bool importInitial(QSqlDatabase db, QString *error);
  static bool moveEmployee(int employee, int target, QString *error);
  static bool saveEmployee(int id, int serviceId, const QList<QVariant> &values,
                           QString *error);
  static int employeeService(int employeeId);

private:
  bool enregistrer(bool update, QString *error) const;
  int m_serviceId = 0;
  int m_id = 0;
  QString m_nom;
  QString m_prenom;
  QString m_courriel;
  QString m_telephone;
  QString m_poste;
  QDate m_dateEmbauche;
  bool m_disponible = true;
  double m_chargeHebdo = 0.0;
  double m_dispoHebdo = 35.0;
  QString m_competences;
  QString m_cin,m_adresse,m_typeContrat;
  QDate m_dateNaissance;
  double m_salaire=-1; // Valeur inconnue pour les profils anciens.
};

#endif
