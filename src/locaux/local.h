#pragma once
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

enum class CleaningState { Clean = 0, ToClean = 1, InProgress = 2 };
enum class RoomState { Available, Occupied, OutOfService, ToClean };

// Entite "Local" (une salle / un espace de la clinique)
class Local {
public:
  Local() = default;
  static const QString Disponible, Occupe, ANettoyer, HorsService;
  static QStringList etats() { return {Disponible,Occupe,ANettoyer,HorsService}; }
  QString getEtat() const { return stateText(); }
  CleaningState getCleaning() const { return cleaning; }
  void setEtat(const QString &v) { outOfService=v==HorsService;occupancy=v==Occupe?qMax(1,occupancy):0;cleaning=v==ANettoyer?CleaningState::ToClean:CleaningState::Clean; }
  bool setOutOfService(bool on) { if(on && occupancy>0)return false;outOfService=on;return true; }
  bool peutRecevoirAnimal() const { return !outOfService && cleaning==CleaningState::Clean && occupancy<capacity; }
  bool ajouterAnimal() { if(!peutRecevoirAnimal())return false;++occupancy;return true; }
  bool retirerAnimal() { if(occupancy<=0)return false;if(--occupancy==0)markToClean();return true; }
  bool signalerANettoyer() { if(outOfService || occupancy>0 || cleaning!=CleaningState::Clean)return false;markToClean();return true; }
  bool terminerNettoyage() { if(outOfService || occupancy>0 || cleaning==CleaningState::Clean)return false;markClean();return true; }
  QString controleSaisie() const;
  static QList<QPair<int,QString>> services();
  int getId() const { return id; }
  void setId(int v) { id=v; }
  QString getName() const { return name; }
  void setName(QString v) { name=v; }
  QString getType() const { return type; }
  void setType(QString v) { type=v; }
  int getFloor() const { return floor; }
  void setFloor(int v) { floor=v; }
  int getCapacity() const { return capacity; }
  void setCapacity(int v) { capacity=v; }
  QString getSpecialty() const { return specialty; }
  void setSpecialty(QString v) { specialty=v; }
  int getOccupancy() const { return occupancy; }
  void setOccupancy(int v) { occupancy=v; }
  int getIdService() const { return serviceId; }
  void setIdService(int v) { serviceId=v; }
  bool getOutOfService() const { return outOfService; }

  static bool afficher(QList<Local> *items, qint64 *revision, QString *error);
  static bool enregistrer(const QList<Local> &items, qint64 *revision,
                          QString *error);
  int id = 0;
  int serviceId = 0;
  QString name;
  QString type;      // Consultation, Chirurgie, ...
  int floor = 0;     // 0 = rez-de-chaussee
  int capacity = 1;  // nombre d'animaux max
  int occupancy = 0; // nombre d'animaux presents
  QString specialty;
  bool outOfService = false;
  CleaningState cleaning = CleaningState::Clean;
  QDateTime lastCleaned;
  QDateTime dirtySince; // depuis quand le local est "a nettoyer"

  RoomState state() const;
  QString stateText() const;
  QString cleaningText() const;
  QString floorText() const;  // "Rez-de-chaussée", "Étage 1"
  QString floorShort() const; // "RDC", "Étage 1"
  QString iconName() const;
  bool isUrgentType() const; // blocs et isolement : nettoyage prioritaire

  // suivi du nettoyage
  void markToClean();          // -> A nettoyer (demarre le chronometre)
  void startCleaning();        // -> En cours
  void markClean();            // -> Propre
  qint64 dirtyMinutes() const; // minutes passees a l'etat "A nettoyer"

  QJsonObject toJson() const;
  static Local fromJson(const QJsonObject &o);
  static QStringList types();
  // Lecture/ecriture SQL propres a cette entite, avec revision de concurrence.
  using Rows = QVector<QVariantList>;
  static bool readRows(Rows *rows, qint64 *revision, QString *error);
  static bool saveRows(const Rows &rows, qint64 *revision, QString *error);

private:
  static bool loadRows(Rows *rows, QString *error);
  static qint64 rowsVersion(const Rows &rows);
};
