#include "smartsearch.h"

#include <QChar>

namespace SmartSearch {

QString normalize(const QString &s)
{
    const QString d = s.toLower().normalized(QString::NormalizationForm_D);
    QString out;
    out.reserve(d.size());
    for (const QChar c : d) {
        if (c.category() == QChar::Mark_NonSpacing)
            continue;
        out += (c.isLetterOrNumber() ? c : QChar(' '));
    }
    return out.simplified();
}

// le mot tape est un debut du mot-cle (ou sa forme au pluriel) - au moins 4 lettres
static bool kw(const QString &t, const char *word)
{
    const QString w = QString::fromLatin1(word);
    return t.size() >= 4 && (w.startsWith(t) || t.startsWith(w));
}

static bool isNumber(const QString &t)
{
    if (t.isEmpty())
        return false;
    for (const QChar c : t)
        if (!c.isDigit())
            return false;
    return true;
}

SmartQuery parse(const QString &text)
{
    SmartQuery q;
    const QStringList tk = normalize(text).split(' ', Qt::SkipEmptyParts);
    static const QStringList stop = { "a", "en", "de", "du", "le", "la", "les", "et", "des", "un", "une" };

    QStringList normTypes;
    const QStringList types = Local::types();
    for (const QString &t : types)
        normTypes << normalize(t);

    for (int i = 0; i < tk.size(); ++i) {
        const QString &t = tk[i];
        if (stop.contains(t))
            continue;

        if (t == "hs" || t == "hors") {
            q.states.insert(int(RoomState::OutOfService));
            q.understood << QStringLiteral("Hors service");
            if (t == "hors" && i + 1 < tk.size() && tk[i + 1] == "service")
                ++i;
        } else if (t == "rdc") {
            q.floors.insert(0);
            q.understood << QStringLiteral("RDC");
        } else if (t == "etage") {
            if (i + 1 < tk.size() && isNumber(tk[i + 1])) {
                q.floors.insert(tk[i + 1].toInt());
                q.understood << QStringLiteral("Étage %1").arg(tk[i + 1].toInt());
                ++i;
            }
        } else if (kw(t, "disponible") || kw(t, "libre")) {
            q.states.insert(int(RoomState::Available));
            q.understood << QStringLiteral("Disponible");
        } else if (kw(t, "occupe")) {
            q.states.insert(int(RoomState::Occupied));
            q.understood << QStringLiteral("Occupé");
        } else if (kw(t, "sale") || kw(t, "nettoyer")) {
            q.cleanings.insert(int(CleaningState::ToClean));
            q.understood << QStringLiteral("À nettoyer");
        } else if (kw(t, "propre")) {
            q.cleanings.insert(int(CleaningState::Clean));
            q.understood << QStringLiteral("Propre");
        } else if (t == "cours" || t == "encours") {
            q.cleanings.insert(int(CleaningState::InProgress));
            q.understood << QStringLiteral("Nettoyage en cours");
        } else if (kw(t, "plein") || kw(t, "sature") || kw(t, "complet")) {
            q.full = true;
            q.understood << QStringLiteral("Complet");
        } else if (kw(t, "alerte")) {
            q.alertsOnly = true;
            q.understood << QStringLiteral("Avec alerte");
        } else {
            bool isType = false;
            if (t.size() >= 3) {
                for (int k = 0; k < types.size(); ++k) {
                    if (normTypes[k].startsWith(t)) {
                        q.types.insert(types[k]);
                        q.understood << types[k];
                        isType = true;
                    }
                }
            }
            if (!isType) {
                q.terms << t;
                q.understood << QStringLiteral("« %1 »").arg(t);
            }
        }
    }
    q.understood.removeDuplicates();
    return q;
}

bool isActive(const SmartQuery &q, Kw k)
{
    switch (k) {
    case Kw::Available:    return q.states.contains(int(RoomState::Available));
    case Kw::Occupied:    return q.states.contains(int(RoomState::Occupied));
    case Kw::OutOfService:return q.states.contains(int(RoomState::OutOfService));
    case Kw::Dirty:       return q.cleanings.contains(int(CleaningState::ToClean));
    }
    return false;
}

QString toggle(const QString &text, Kw k)
{
    QStringList words = text.split(' ', Qt::SkipEmptyParts);
    if (isActive(parse(text), k)) {
        // retire les mots qui, seuls, activent cette pastille
        QStringList kept;
        for (int i = 0; i < words.size(); ++i) {
            if (isActive(parse(words[i]), k)) {
                if (normalize(words[i]) == "hors" && i + 1 < words.size() && normalize(words[i + 1]) == "service")
                    ++i;
                continue;
            }
            kept << words[i];
        }
        return kept.join(' ');
    }
    static const char *const add[] = { "dispo", "occupe", "hs", "sale" };
    words << QString::fromLatin1(add[int(k)]);
    return words.join(' ');
}

} // namespace SmartSearch

bool SmartQuery::isEmpty() const
{
    return states.isEmpty() && cleanings.isEmpty() && types.isEmpty() && floors.isEmpty()
           && !full && !alertsOnly && terms.isEmpty();
}

bool SmartQuery::matches(const Local &l, int alertLevel) const
{
    if (!states.isEmpty() && !states.contains(int(l.state())))
        return false;
    if (!cleanings.isEmpty() && !cleanings.contains(int(l.getCleaning())))
        return false;
    if (!types.isEmpty() && !types.contains(l.getType()))
        return false;
    if (!floors.isEmpty() && !floors.contains(l.getFloor()))
        return false;
    if (full && !(!l.getOutOfService() && l.getCapacity() > 0 && l.getOccupancy() >= l.getCapacity()))
        return false;
    if (alertsOnly && alertLevel <= 0)
        return false;

    if (!terms.isEmpty()) {
        const QString hay = SmartSearch::normalize(l.getName() + ' ' + l.getType() + ' ' + l.getSpecialty());
        for (const QString &t : terms) {
            bool numeric = !t.isEmpty();
            for (const QChar c : t)
                if (!c.isDigit())
                    numeric = false;
            const bool ok = hay.contains(t) || (numeric && QString::number(l.getId()) == t);
            if (!ok)
                return false;
        }
    }
    return true;
}
