#pragma once
#include <QDateTime>
#include <QList>
#include <QSet>
#include <QString>
class QSqlQueryModel;
class QObject;

class Service {
public:
  Service() = default;
  Service(int id, const QString &name, const QString &clientName,
          const QString &category, const QDateTime &dateTime,
          const QString &status, int duration, double price);

  // Atelier : l'interface appelle le modele et consulte son resultat.
  bool ajouter();
  bool modifier();
  bool supprimer(int id);
  QString lastError() const { return m_error; }
  int getId() const { return id; }
  void setId(int value) { id = value; }
  int getLocalId() const { return localId; }
  void setLocalId(int value) { localId = value; }
  int getEmployeeId() const { return employeeId; }
  void setEmployeeId(int value) { employeeId = value; }
  int getClientId() const { return clientId; }
  void setClientId(int value) { clientId = value; }
  QString getNewEmployeeNom() const { return newEmployeeNom; }
  void setNewEmployeeNom(QString value) { newEmployeeNom = value; }
  QString getNewEmployeePrenom() const { return newEmployeePrenom; }
  void setNewEmployeePrenom(QString value) { newEmployeePrenom = value; }
  QString getName() const { return name; }
  void setName(QString value) { name = value; }
  QString getDescription() const { return description; }
  void setDescription(QString value) { description = value; }
  QString getResponsible() const { return responsible; }
  void setResponsible(QString value) { responsible = value; }
  QString getClientName() const { return clientName; }
  void setClientName(QString value) { clientName = value; }
  QString getInvoiceNumber() const { return invoiceNumber; }
  void setInvoiceNumber(QString value) { invoiceNumber = value; }
  QDateTime getInvoiceDate() const { return invoiceDate; }
  void setInvoiceDate(QDateTime value) { invoiceDate = value; }
  QString getCategory() const { return category; }
  void setCategory(QString value) { category = value; }
  QDateTime getDateTime() const { return dateTime; }
  void setDateTime(QDateTime value) { dateTime = value; }
  QString getStatus() const { return status; }
  void setStatus(QString value) { status = value; }
  int getDuration() const { return duration; }
  void setDuration(int value) { duration = value; }
  double getPrice() const { return price; }
  void setPrice(double value) { price = value; }
  bool ajouter(QString *error);
  bool modifier(QString *error) const;
  static bool supprimer(int id, QString *error);
  static QList<Service> afficherListe(QString *error);
  QSqlQueryModel *afficher(QObject *parent = nullptr,
                           const QList<int> *orderedIds = nullptr) const;

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

  bool enregistrerOracle(bool update, int *insertedId, QString *error) const;
  bool enregistrer(bool update, int *insertedId, QString *error) const;
};
