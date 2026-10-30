#pragma once
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

enum class CleaningState { Clean = 0, ToClean = 1, InProgress = 2 };
enum class RoomState { Available, Occupied, OutOfService };

// Entite "Local" (une salle / un espace de la clinique)
class Local {
public:
  Local() = default;
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
