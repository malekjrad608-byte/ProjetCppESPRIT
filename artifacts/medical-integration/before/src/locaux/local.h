#pragma once
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QStringList>

enum class CleaningState { Clean = 0, ToClean = 1, InProgress = 2 };
enum class RoomState { Available, Occupied, OutOfService };

// Entite "Local" (une salle / un espace de la clinique)
struct Local
{
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

    QJsonObject toJson() const;
    static Local fromJson(const QJsonObject &o);
    static QStringList types();
};
