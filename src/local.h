#pragma once
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QList>
#include <QPair>

enum class CleaningState { Clean = 0, ToClean = 1, InProgress = 2 };
enum class RoomState { Available, Occupied, OutOfService };

// Entite "Local" (une salle / un espace de la clinique)
class Local
{
private:
    // ---- attributs (prives, comme l'atelier) ----
    int id = 0;
    QString name;
    QString type;          // Consultation, Chirurgie, ...
    int floor = 0;         // 0 = rez-de-chaussee
    int capacity = 1;      // nombre d'animaux max
    int occupancy = 0;     // nombre d'animaux presents
    QString specialty;
    bool outOfService = false;
    CleaningState cleaning = CleaningState::Clean;
    QDateTime lastCleaned;
    QDateTime dirtySince;  // depuis quand le local est "a nettoyer"
    int idService = 0;     // cle etrangere -> SERVICES(ID_SERV), obligatoire

public:
    // ---- constructeurs ----
    Local() {}
    Local(int id, QString name, QString type, int floor, int capacity,
          QString specialty, bool outOfService, int idService)
        : id(id), name(name), type(type), floor(floor), capacity(capacity),
          specialty(specialty), outOfService(outOfService),
          lastCleaned(QDateTime::currentDateTime()), idService(idService) {}

    // ---- getters ----
    int getId() const { return id; }
    QString getName() const { return name; }
    QString getType() const { return type; }
    int getFloor() const { return floor; }
    int getCapacity() const { return capacity; }
    int getOccupancy() const { return occupancy; }
    QString getSpecialty() const { return specialty; }
    bool getOutOfService() const { return outOfService; }
    CleaningState getCleaning() const { return cleaning; }
    QDateTime getLastCleaned() const { return lastCleaned; }
    QDateTime getDirtySince() const { return dirtySince; }
    int getIdService() const { return idService; }

    // ---- setters ----
    void setId(int v) { id = v; }
    void setName(const QString &v) { name = v; }
    void setType(const QString &v) { type = v; }
    void setFloor(int v) { floor = v; }
    void setCapacity(int v) { capacity = v; }
    void setOccupancy(int v) { occupancy = v; }
    void setSpecialty(const QString &v) { specialty = v; }
    void setOutOfService(bool v) { outOfService = v; }
    void setCleaning(CleaningState v) { cleaning = v; }
    void setLastCleaned(const QDateTime &v) { lastCleaned = v; }
    void setDirtySince(const QDateTime &v) { dirtySince = v; }
    void setIdService(int v) { idService = v; }

    // ---- methodes metier ----
    RoomState state() const;
    QString stateText() const;
    QString cleaningText() const;
    QString floorText() const;   // "Rez-de-chaussée", "Étage 1"
    QString floorShort() const;  // "RDC", "Étage 1"
    QString iconName() const;
    bool isUrgentType() const;   // blocs et isolement : nettoyage prioritaire

    // suivi du nettoyage
    void markToClean();          // -> A nettoyer (demarre le chronometre)
    void startCleaning();        // -> En cours
    void markClean();            // -> Propre
    qint64 dirtyMinutes() const; // minutes passees a l'etat "A nettoyer"

    static QStringList types();

    // ---- CRUD (comme l'atelier) : toutes les requetes SQL sont dans la classe de l'entite ----
    QString controleSaisie() const;          // controle de saisie ("" = OK)
    bool ajouter();                          // Create
    QSqlQueryModel *afficher();              // Read
    bool modifier();                         // Update
    bool supprimer(int id);                  // Delete

    static QList<QPair<int, QString>> services();   // (ID_SERV, libelle) pour la liste du formulaire
    static Local fromRecord(const QSqlRecord &r);   // ligne de la table -> objet Local
    static QString derniereErreur();                // dernier message d'erreur SQL
};
