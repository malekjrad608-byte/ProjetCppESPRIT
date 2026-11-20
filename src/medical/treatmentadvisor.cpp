#include "treatmentadvisor.h"
#include "vaccination.h"

#include <QMap>
#include <QSet>
#include <QLocale>
#include <QRegularExpression>
#include <algorithm>

namespace {

QSet<QString> tokens(const QString &norm)
{
    static const QSet<QString> stop = { "des", "les", "une", "avec", "sans", "pour", "dans", "sur", "par",
                                        "est", "aux", "que", "qui", "legere", "leger", "debut", "debutante" };
    QSet<QString> out;
    for (const QString &t : norm.split(QRegularExpression("[^a-z0-9]+"), Qt::SkipEmptyParts))
        if (t.size() >= 3 && !stop.contains(t))
            out.insert(t);
    return out;
}

// 0 = aucun rapport ; 1 = diagnostic identique
double similarite(const QString &a, const QString &b)
{
    if (a.isEmpty() || b.isEmpty()) return 0;
    if (a == b) return 1.0;
    const QSet<QString> ta = tokens(a), tb = tokens(b);
    if (ta.isEmpty() || tb.isEmpty()) return 0;
    QSet<QString> inter = ta;
    inter.intersect(tb);
    const double jaccard = double(inter.size()) / double(ta.size() + tb.size() - inter.size());
    const bool contient = (a.size() >= 5 && b.contains(a)) || (b.size() >= 5 && a.contains(b));
    const double s = qMax(jaccard, contient ? 0.7 : 0.0);
    return s >= 0.5 ? s : 0;
}

QStringList decouperTraitements(const QString &texte)
{
    QStringList out;
    for (const QString &p : texte.split(QRegularExpression("[;,]"), Qt::SkipEmptyParts)) {
        const QString t = p.trimmed();
        const QString n = TreatmentAdvisor::normaliser(t);
        if (t.isEmpty() || n.startsWith("aucun") || n.startsWith("pas de"))
            continue;
        out << t;
    }
    return out;
}

QStringList decouperAllergies(const QString &texte)
{
    QStringList out;
    for (const QString &p : texte.split(QRegularExpression("[;,/\\n]"), Qt::SkipEmptyParts)) {
        const QString n = TreatmentAdvisor::normaliser(p.trimmed());
        if (n.size() >= 4 && !n.startsWith("aucun") && !n.startsWith("non "))
            out << n;
    }
    return out;
}

struct KbEntry { const char *motsCles; QStringList traitements; };

// Propositions de base quand l'historique est trop mince (à valider par le vétérinaire)
const QList<KbEntry> &baseConnaissance()
{
    static const QList<KbEntry> kb = {
        { "gastr|diarrh|vomi|digest",       { "Régime digestif", "Probiotique", "Réhydratation" } },
        { "otite|oreille",                  { "Nettoyant auriculaire", "Antibiotique" } },
        { "dermat|allerg|prurit|gale|puce|teigne", { "Antiparasitaire externe", "Anti-inflammatoire", "Shampoing médicamenteux" } },
        { "respir|toux|rhume|bronch|asthm|coryza", { "Antibiotique", "Anti-inflammatoire" } },
        { "parasit|vermi|helminth",           { "Vermifuge", "Antiparasitaire externe" } },
        { "plaie|blessure|abces|morsure",   { "Antiseptique", "Antibiotique", "Anti-inflammatoire" } },
        { "routine|prevent|vaccin|controle",   { "Vaccin antirabique", "Vermifuge", "Antiparasitaire externe" } },
        { "arthrose|boiter|articul",        { "Anti-inflammatoire", "Complément articulaire" } },
    };
    return kb;
}

int stockPour(const QString &nom, const QList<LookupItem> &stock)
{
    const QString n = TreatmentAdvisor::normaliser(nom);
    for (const LookupItem &s : stock) {
        const QString sn = TreatmentAdvisor::normaliser(s.label);
        if (sn == n || sn.contains(n) || n.contains(sn))
            return s.quantity;
    }
    return -1;     // pas un produit suivi en stock (conseil, régime, ...) ou stock inconnu
}

} // namespace

QString TreatmentAdvisor::normaliser(const QString &texte)
{
    const QString d = texte.trimmed().toLower().normalized(QString::NormalizationForm_D);
    QString out;
    for (const QChar &c : d)
        if (c.category() != QChar::Mark_NonSpacing)
            out.append(c);
    return out.simplified();
}

QList<TreatmentSuggestion> TreatmentAdvisor::suggerer(const QString &diagnostic, const QString &espece,
                                                      const QString &idAnimal, const QString &allergies,
                                                      const QVector<MedicalRecord> &records,
                                                      const QList<LookupItem> &stock,
                                                      const QString &dossierExclu, int maxResultats)
{
    const QString diag = normaliser(diagnostic);
    if (diag.size() < 3)
        return {};

    struct Acc { QString nom; double score = 0; int nb = 0; bool memeAnimal = false; };
    QMap<QString, Acc> acc;
    double poidsTotal = 0;

    for (const MedicalRecord &r : records) {
        if (!dossierExclu.isEmpty() && r.getIdDossier() == dossierExclu)
            continue;
        const double sim = similarite(diag, normaliser(r.getDiagnostic()));
        if (sim <= 0)
            continue;
        const QStringList ts = decouperTraitements(r.getTraitement());
        if (ts.isEmpty())
            continue;
        const bool memeAnimal = !idAnimal.isEmpty() && r.getIdAnimal() == idAnimal;
        const double w = sim
                       * (r.getEspece().compare(espece, Qt::CaseInsensitive) == 0 ? 1.0 : 0.35)
                       * (memeAnimal ? 1.5 : 1.0);
        poidsTotal += w;
        QSet<QString> dejaCompte;
        for (const QString &t : ts) {
            const QString key = normaliser(t);
            if (dejaCompte.contains(key)) continue;
            dejaCompte.insert(key);
            Acc &a = acc[key];
            if (a.nom.isEmpty()) a.nom = t;
            a.score += w;
            a.nb += 1;
            a.memeAnimal = a.memeAnimal || memeAnimal;
        }
    }

    const QStringList allergenes = decouperAllergies(allergies);
    QList<TreatmentSuggestion> out;
    for (auto it = acc.cbegin(); it != acc.cend(); ++it) {
        TreatmentSuggestion s;
        s.nom = it->nom;
        s.score = it->score;
        s.nbCas = it->nb;
        s.percent = poidsTotal > 0 ? qMin(100, qRound(100.0 * it->score / poidsTotal)) : 0;
        s.dejaPourCetAnimal = it->memeAnimal;
        const QString n = it.key();
        for (const QString &al : allergenes)
            if (n.contains(al) || (n.size() >= 4 && al.contains(n))) { s.allergie = true; s.allergene = al; break; }
        s.stock = stockPour(s.nom, stock);

        QStringList parts;
        parts << QString("Prescrit dans %1 cas similaire%2 (%3 %)").arg(s.nbCas).arg(s.nbCas > 1 ? "s" : "").arg(s.percent);
        if (s.dejaPourCetAnimal) parts << "déjà donné à cet animal";
        s.raison = parts.join(" · ");
        out << s;
    }

    // Complément : propositions de base si l'historique est mince
    if (out.size() < 3) {
        for (const KbEntry &e : baseConnaissance()) {
            bool hit = false;
            for (const QString &k : QString(e.motsCles).split('|'))
                if (diag.contains(k)) { hit = true; break; }
            if (!hit) continue;
            for (const QString &t : e.traitements) {
                const QString key = normaliser(t);
                if (acc.contains(key)) continue;
                bool dup = false;
                for (const auto &o : std::as_const(out)) if (normaliser(o.nom) == key) dup = true;
                if (dup) continue;
                TreatmentSuggestion s;
                s.nom = t;
                s.baseConnaissance = true;
                s.stock = stockPour(t, stock);
                for (const QString &al : allergenes)
                    if (key.contains(al) || (key.size() >= 4 && al.contains(key))) { s.allergie = true; s.allergene = al; break; }
                s.raison = "Proposition de base (pas assez d'historique pour ce diagnostic)";
                out << s;
            }
        }
    }

    for (TreatmentSuggestion &s : out) {
        if (s.allergie)
            s.raison = QString::fromUtf8("⚠ Allergie connue (%1) — ").arg(s.allergene) + s.raison;
        if (s.stock == 0)
            s.raison += QString::fromUtf8(" · hors stock");
        else if (s.stock > 0)
            s.raison += QString(" · %1 en stock").arg(s.stock);
    }

    std::sort(out.begin(), out.end(), [](const TreatmentSuggestion &a, const TreatmentSuggestion &b) {
        if (a.allergie != b.allergie) return !a.allergie;                 // allergiques en dernier
        if (a.baseConnaissance != b.baseConnaissance) return !a.baseConnaissance;
        if ((a.stock == 0) != (b.stock == 0)) return a.stock != 0;       // rupture après
        if (a.score != b.score) return a.score > b.score;
        return a.nom.localeAwareCompare(b.nom) < 0;
    });
    if (out.size() > maxResultats)
        out = out.mid(0, maxResultats);
    return out;
}

QStringList TreatmentAdvisor::etatAnimal(const QString &idAnimal, const QString &espece, int age, double poids,
                                         const QString &allergies, const QVector<MedicalRecord> &records)
{
    QStringList lines;
    const QLocale fr(QLocale::French);

    QString id = espece.isEmpty() ? QString("Animal") : espece;
    id += QString(" · %1 an%2").arg(age).arg(age > 1 ? "s" : "");
    if (poids > 0) id += " · " + fr.toString(poids, 'f', 1) + " kg";
    lines << id;

    // historique de cet animal (les dossiers sont triés du plus récent au plus ancien, on re-trie par sécurité)
    QVector<MedicalRecord> hist;
    for (const MedicalRecord &r : records)
        if (!idAnimal.isEmpty() && r.getIdAnimal() == idAnimal)
            hist.append(r);
    std::sort(hist.begin(), hist.end(), [](const MedicalRecord &a, const MedicalRecord &b) {
        return a.getDateConsultation() > b.getDateConsultation(); });

    if (!hist.isEmpty()) {
        const MedicalRecord &last = hist.first();
        lines << QString("%1 consultation%2 · dernière le %3 : %4")
                    .arg(hist.size()).arg(hist.size() > 1 ? "s" : "")
                    .arg(fr.toString(last.getDateConsultation(), "dd/MM/yyyy"),
                         last.getDiagnostic().isEmpty() ? QString("—") : last.getDiagnostic());
        for (const MedicalRecord &r : std::as_const(hist))
            if (r.getPoids() > 0 && poids > 0) {
                const double delta = 100.0 * (poids - r.getPoids()) / r.getPoids();
                if (qAbs(delta) >= 10)
                    lines << QString::fromUtf8("⚠ Poids %1 %2 % depuis le %3")
                                .arg(delta > 0 ? "+" : "−").arg(qRound(qAbs(delta)))
                                .arg(fr.toString(r.getDateConsultation(), "dd/MM/yyyy"));
                break;
            }
    } else {
        lines << "Aucun historique pour cet animal";
    }

    if (!decouperAllergies(allergies).isEmpty())
        lines << QString::fromUtf8("⚠ Allergies : %1").arg(allergies.trimmed());
    if (age >= 8)
        lines << QString::fromUtf8("Animal âgé (≥ 8 ans) : vérifier les doses et la fonction rénale/hépatique");

    const QString vacc = VaccinationManager::statutAnimal(idAnimal, records);
    if (!vacc.isEmpty())
        lines << QString::fromUtf8("💉 Vaccins : %1").arg(vacc);
    return lines;
}
