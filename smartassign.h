#pragma once
#include <QList>
#include <QString>
#include <QStringList>
#include "local.h"

// Affectation intelligente : propose le meilleur local pour un animal.
struct AssignQuery
{
    QString species;     // Chien, Chat, ...
    QString need;        // "Consultation dentaire", "Chirurgie", ...
    bool urgent = false;
};

struct AssignSuggestion
{
    int localId = 0;
    QString name;
    QString type;
    QString floorText;
    int score = 0;               // 0..100
    QStringList reasons;         // points positifs
    QStringList warnings;        // points d'attention
};

namespace SmartAssign {

QStringList species();
QStringList needs();
QString typeForNeed(const QString &need);

// Les meilleurs locaux, du plus au moins adapte
QList<AssignSuggestion> suggest(const QList<Local> &locaux, const AssignQuery &q, int maxResults = 3);

// Pourquoi aucun local ne convient
QString explainNone(const QList<Local> &locaux, const AssignQuery &q);

} // namespace SmartAssign
