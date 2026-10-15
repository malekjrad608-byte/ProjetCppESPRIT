#include "smartassign.h"

#include <algorithm>

namespace SmartAssign {

QStringList species()
{
    return { "Chien", "Chat", "Lapin / NAC", "Oiseau" };
}

QStringList needs()
{
    return { "Consultation générale", "Consultation dentaire", "Consultation ophtalmologie",
             "Chirurgie", "Hospitalisation / post-op", "Soins intensifs",
             "Analyses (laboratoire)", "Radio / échographie", "Pension / garde",
             "Maladie contagieuse (isolement)" };
}

QString typeForNeed(const QString &need)
{
    if (need.startsWith("Consultation")) return QStringLiteral("Consultation");
    if (need == "Chirurgie")             return QStringLiteral("Chirurgie");
    if (need.startsWith("Hospitalisation") || need == "Soins intensifs")
        return QStringLiteral("Hospitalisation");
    if (need.startsWith("Analyses"))     return QStringLiteral("Laboratoire");
    if (need.startsWith("Radio"))        return QStringLiteral("Imagerie");
    if (need.startsWith("Pension"))      return QStringLiteral("Pension");
    if (need.startsWith("Maladie"))      return QStringLiteral("Isolement");
    return QStringLiteral("Consultation");
}

static bool has(const QString &text, const char *word)
{
    return text.contains(QString::fromUtf8(word), Qt::CaseInsensitive);
}

QList<AssignSuggestion> suggest(const QList<Local> &all, const AssignQuery &q, int maxResults)
{
    QList<AssignSuggestion> out;
    const QString type = typeForNeed(q.need);

    for (const Local &l : all) {
        if (l.type != type)
            continue;
        if (l.outOfService || l.occupancy >= l.capacity)
            continue;

        AssignSuggestion s;
        s.localId = l.id;
        s.name = l.name;
        s.type = l.type;
        s.floorText = l.floorText();
        int score = 50;

        // proprete
        switch (l.cleaning) {
        case CleaningState::Clean:
            score += 20;
            s.reasons << QStringLiteral("Local propre");
            break;
        case CleaningState::InProgress:
            score -= 15;
            s.warnings << QStringLiteral("Nettoyage en cours");
            break;
        case CleaningState::ToClean:
            score -= l.isUrgentType() ? 50 : 25;
            s.warnings << (l.isUrgentType() ? QStringLiteral("À nettoyer avant usage (stérilité)")
                                            : QStringLiteral("À nettoyer avant usage"));
            break;
        }

        // places libres
        if (l.capacity == 1) {
            score += 10;
            s.reasons << QStringLiteral("Libre");
        } else {
            const int left = l.capacity - l.occupancy;
            score += (15 * left) / l.capacity;
            s.reasons << QStringLiteral("%1 place(s) libre(s)").arg(left);
        }

        // specialite / equipement
        const QString sp = l.specialty;
        if (q.need == "Consultation générale") {
            if (has(sp, "général")) {
                score += 30;
                s.reasons << QStringLiteral("Spécialité : %1").arg(sp);
            }
        } else if (q.need == "Consultation dentaire") {
            if (has(sp, "dent")) {
                score += 40;
                s.reasons << QStringLiteral("Spécialité : %1").arg(sp);
            } else {
                score -= 20;
                s.warnings << QStringLiteral("Pas équipé pour le dentaire");
            }
        } else if (q.need == "Consultation ophtalmologie") {
            if (has(sp, "ophtalmo")) {
                score += 40;
                s.reasons << QStringLiteral("Spécialité : %1").arg(sp);
            } else {
                score -= 20;
                s.warnings << QStringLiteral("Pas équipé pour l'ophtalmologie");
            }
        } else if (q.need == "Soins intensifs") {
            if (has(sp, "intensif")) {
                score += 40;
                s.reasons << QStringLiteral("Soins intensifs");
            } else {
                score -= 10;
            }
        } else if (q.need == "Hospitalisation / post-op") {
            if (has(sp, "post-op")) {
                score += 20;
                s.reasons << QStringLiteral("Soins post-opératoires");
            }
        } else if (q.need == "Pension / garde") {
            const bool dog = has(sp, "chien");
            const bool cat = has(sp, "chat");
            if (q.species == "Chien" && dog) {
                score += 40;
                s.reasons << QStringLiteral("Dédiée aux chiens");
            } else if (q.species == "Chat" && cat) {
                score += 40;
                s.reasons << QStringLiteral("Dédiée aux chats");
            } else if (dog || cat) {
                score -= 30;
                s.warnings << QStringLiteral("Local dédié à d'autres animaux");
            } else {
                s.warnings << QStringLiteral("Pas de local dédié à cette espèce");
            }
        }

        // urgence : acces rapide
        if (q.urgent && l.floor == 0) {
            score += 10;
            s.reasons << QStringLiteral("Accès rapide (rez-de-chaussée)");
        }

        s.score = qBound(0, score, 100);
        out.append(s);
    }

    std::stable_sort(out.begin(), out.end(),
                     [](const AssignSuggestion &a, const AssignSuggestion &b) { return a.score > b.score; });
    if (out.size() > maxResults)
        out = out.mid(0, maxResults);
    return out;
}

QString explainNone(const QList<Local> &all, const AssignQuery &q)
{
    const QString type = typeForNeed(q.need);
    int rooms = 0, oos = 0, full = 0;
    for (const Local &l : all) {
        if (l.type != type)
            continue;
        ++rooms;
        if (l.outOfService)
            ++oos;
        else if (l.occupancy >= l.capacity)
            ++full;
    }
    if (rooms == 0)
        return QStringLiteral("Aucun local de type « %1 » n'existe dans la clinique.").arg(type);
    if (oos == rooms)
        return QStringLiteral("Tous les locaux « %1 » sont hors service.").arg(type);
    return QStringLiteral("Tous les locaux « %1 » sont complets ou hors service.").arg(type);
}

} // namespace SmartAssign
