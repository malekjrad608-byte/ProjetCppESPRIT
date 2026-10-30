#pragma once
#include <QSet>
#include <QString>
#include <QStringList>
#include "local.h"

// Recherche intelligente : une seule barre comprend des mots-cles
//   "chirurgie dispo"   "sale etage 1"   "hors service"   "plein"   "alerte"   "consultation 3"
// Les mots reconnus deviennent des filtres, le reste est cherche dans le nom / type / specialite.
struct SmartQuery
{
    QSet<int> states;        // RoomState
    QSet<int> cleanings;     // CleaningState
    QSet<QString> types;     // types de local (texte d'origine)
    QSet<int> floors;
    bool full = false;       // local complet
    bool alertsOnly = false; // local avec une alerte active
    QStringList terms;       // mots libres (sans accents, minuscules)
    QStringList understood;  // ce qui a ete compris, pour l'affichage

    bool isEmpty() const;
    bool matches(const Local &l, int alertLevel) const;
};

namespace SmartSearch
{
enum class Kw { Available, Occupied, OutOfService, Dirty };

QString normalize(const QString &s);               // minuscules, sans accents
SmartQuery parse(const QString &text);
bool isActive(const SmartQuery &q, Kw kw);
QString toggle(const QString &text, Kw kw);        // ajoute / retire le mot-cle d'une pastille
}
