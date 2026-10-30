#pragma once
#include <QDateTime>
#include <QList>
#include <QSet>
#include <QString>
class QSqlQueryModel;
class QObject;

class Service {
private:
  // Attributs encapsules : accessibles via les getters et setters publics.
  QString m_error;
  int id = 0;
  int localId = 0;
  int employeeId = 0;
  int clientId = 0;
  QString newEmployeeNom;
  QString newEmployeePrenom;
  QString name;
  QString description;
  QString responsible;
  QString clientName;
  QString invoiceNumber;
  QDateTime invoiceDate;
  QString category = QStringLiteral("Soins & bien-être");
  QDateTime dateTime = QDateTime::currentDateTime();
  QString status = QStringLiteral("Planifié");
  int duration = 30;
  double price = 0;

public:
  // Constructeurs publics pour creer un Service depuis la vue.
  Service() = default;
  Service(int id, const QString &name, const QString &clientName,
          const QString &category, const QDateTime &dateTime,
          const QString &status, int duration, double price);

  // Getters : lecture des attributs prives.
  int getId() const { return id; }
  int getLocalId() const { return localId; }
  int getEmployeeId() const { return employeeId; }
  int getClientId() const { return clientId; }
  QString getNewEmployeeNom() const { return newEmployeeNom; }
  QString getNewEmployeePrenom() const { return newEmployeePrenom; }
  QString getName() const { return name; }
  QString getDescription() const { return description; }
  QString getResponsible() const { return responsible; }
  QString getClientName() const { return clientName; }
  QString getInvoiceNumber() const { return invoiceNumber; }
  QDateTime getInvoiceDate() const { return invoiceDate; }
  QString getCategory() const { return category; }
  QDateTime getDateTime() const { return dateTime; }
  QString getStatus() const { return status; }
  int getDuration() const { return duration; }
  double getPrice() const { return price; }
  QString lastError() const { return m_error; }

  // Setters : modification des attributs prives.
  void setId(int value) { id = value; }
  void setLocalId(int value) { localId = value; }
  void setEmployeeId(int value) { employeeId = value; }
  void setClientId(int value) { clientId = value; }
  void setNewEmployeeNom(QString value) { newEmployeeNom = value; }
  void setNewEmployeePrenom(QString value) { newEmployeePrenom = value; }
  void setName(QString value) { name = value; }
  void setDescription(QString value) { description = value; }
  void setResponsible(QString value) { responsible = value; }
  void setClientName(QString value) { clientName = value; }
  void setInvoiceNumber(QString value) { invoiceNumber = value; }
  void setInvoiceDate(QDateTime value) { invoiceDate = value; }
  void setCategory(QString value) { category = value; }
  void setDateTime(QDateTime value) { dateTime = value; }
  void setStatus(QString value) { status = value; }
  void setDuration(int value) { duration = value; }
  void setPrice(double value) { price = value; }

  // CRUD : ajouter, afficher, supprimer, modifier.
  bool ajouter();
  QSqlQueryModel *afficher(QObject *parent = nullptr,
                           const QList<int> *orderedIds = nullptr) const;
  bool supprimer(int id);
  bool modifier();

  bool ajouter(QString *error);
  bool modifier(QString *error) const;
  static bool supprimer(int id, QString *error);
  static QList<Service> afficherListe(QString *error);

  // Planning des affectations : requetes et regles dans le modele Service.

  struct Appointment {
    int serviceId =
        0; // Zero denotes an independent task, which is edited in Employés.
    int employeeId = 0;
    QString title, client, status;
    QDateTime start, finish;
  };
  static QList<Appointment> appointments(QString *error);
  // False also means that availability could not be checked: fail closed.
  static bool available(int employee, const QDateTime &start,
                        const QDateTime &finish, int excludedService,
                        QString *error);
  static QDate weekStart(const QDate &date);
  static QDateTime end(const Service &service);
  static QSet<int> conflictingServices(const QList<Service> &services);
  static QList<Service> inWeek(const QList<Service> &services,
                               const QDate &date, int employee = 0,
                               bool cancelled = false);

  static QString invoiceStatus(int serviceId);

private:
  bool enregistrerOracle(bool update, int *insertedId, QString *error) const;
  bool enregistrer(bool update, int *insertedId, QString *error) const;
};
