#include "healthwatch.h"

#include <QSettings>
#include <QMap>
#include <QSet>
#include <algorithm>

namespace {

QString normaliser(const QString &texte)
{
    QString s = texte.toLower();
    const QString from = QString::fromUtf8("éèêëàâäîïôöûùüç");
    const QString to = QStringLiteral("eeeeaaaiioouuuc");
    for (int i = 0; i < from.size(); ++i)
        s.replace(from[i], to[i]);
    for (QChar &c : s)
        if (!c.isLetterOrNumber())
            c = ' ';
    return s.simplified();
}

// vrai si un mot du texte commence par 'mot' (évite "tique" dans "antibiotique", "gale" dans "égale")
bool contientMot(const QStringList &mots, const QString &mot)
{
    for (const QString &m : mots)
        if (m.startsWith(mot))
            return true;
    return false;
}

// Diagnostics qui ne relèvent pas d'une maladie à surveiller
bool aIgnorer(const QString &norm)
{
    static const QStringList phrases = { "bonne sante", "rien a signaler", "pas de probleme" };
    for (const QString &p : phrases)
        if (norm.contains(p))
            return true;
    static const QStringList ignores = { "controle", "routine", "vaccin", "bilan", "suivi", "toilett", "fracture",
                                         "plaie", "blessure", "castration", "sterilisation", "check", "preventif",
                                         "aucun", "normal" };
    const QStringList mots = norm.split(' ', Qt::SkipEmptyParts);
    for (const QString &k : ignores)
        if (contientMot(mots, k))
            return true;
    return false;
}

struct Famille { QStringList mots; QString libelle; };

// Regroupe les écritures différentes d'une même maladie ("gastro enterite", "Gastro-entérite", "diarrhée"...).
// Retourne true si le diagnostic appartient à une famille connue.
bool classer(const QString &diagnostic, QString &cle, QString &libelle)
{
    static const QList<Famille> familles = {
        { { "gastr", "diarrh", "vomis", "enterite" }, "Gastro-entérite" },
        { { "otite" }, "Otite" },
        { { "dermat", "gale", "teigne", "mycose", "eczema" }, "Dermatite / affection cutanée" },
        { { "respir", "toux", "rhume", "bronch", "coryza", "pneumon" }, "Infection respiratoire" },
        { { "parasit", "verm", "puce", "tique" }, "Parasitose" },
        { { "conjonct" }, "Conjonctivite" },
        { { "parvo" }, "Parvovirose" },
        { { "leptospir" }, "Leptospirose" },
    };
    const QString norm = normaliser(diagnostic);
    const QStringList mots = norm.split(' ', Qt::SkipEmptyParts);
    for (const Famille &f : familles)
        for (const QString &m : f.mots)
            if (contientMot(mots, m)) {
                cle = f.libelle;
                libelle = f.libelle;
                return true;
            }
    cle = norm;
    libelle = diagnostic.trimmed();
    return false;
}

struct Cas
{
    QDate date;
    QString idAnimal, nom, espece;
};

QString nomAnimal(const Cas &c) { return QString("%1 (%2)").arg(c.nom, c.idAnimal); }

QString conseilRecidive()
{
    return "Réexaminer l'animal en cherchant une cause de fond (allergie, parasite, alimentation, maladie chronique). "
           "Envisager des examens complémentaires ou un changement de traitement. À valider par le vétérinaire.";
}
QString conseilFoyer()
{
    return "Possible maladie contagieuse : demander aux propriétaires d'isoler les animaux malades, désinfecter la salle "
           "et le matériel, vérifier l'état vaccinal des animaux et prévenir les propriétaires. À valider par le vétérinaire.";
}

} // namespace

QList<HealthAlert> HealthWatch::analyser(const QVector<MedicalRecord> &records, const QDate &today)
{
    // 1) regrouper les cas par maladie
    QMap<QString, QList<Cas>> parMaladie;
    QMap<QString, QString> libelles;
    for (const MedicalRecord &r : records) {
        if (r.getDiagnostic().trimmed().isEmpty() || !r.getDateConsultation().isValid()
            || r.getDateConsultation() > today)
            continue;
        QString cle, libelle;
        const bool famille = classer(r.getDiagnostic(), cle, libelle);
        if (cle.isEmpty() || (!famille && aIgnorer(cle)))    // diagnostic non pathologique : ignoré
            continue;
        libelles.insert(cle, libelle);
        // la référence est normalisée (espaces, majuscules) pour qu'"ani-001" et "ANI-001" soient le même animal
        parMaladie[cle].append({ r.getDateConsultation(),
                                 (r.getIdAnimal().trimmed().isEmpty() ? r.getIdDossier() : r.getIdAnimal()).trimmed().toUpper(),
                                 r.getNomAnimal(), r.getEspece() });
    }

    QList<HealthAlert> alertes;

    for (auto it = parMaladie.begin(); it != parMaladie.end(); ++it) {
        QList<Cas> cas = it.value();
        std::sort(cas.begin(), cas.end(), [](const Cas &a, const Cas &b) { return a.date < b.date; });
        const QString &cle = it.key();
        const QString libelle = libelles.value(cle);

        // 2) récidive : même animal, même maladie
        QMap<QString, QList<Cas>> parAnimal;
        for (const Cas &c : cas)
            parAnimal[c.idAnimal].append(c);
        for (auto a = parAnimal.begin(); a != parAnimal.end(); ++a) {
            const QList<Cas> &liste = a.value();
            QSet<QDate> jours;                               // plusieurs dossiers le même jour = un seul passage
            QList<QDate> dates;
            for (const Cas &c : liste)
                if (c.date.daysTo(today) >= 0 && c.date.daysTo(today) <= kRecidiveJours && !jours.contains(c.date)) {
                    jours.insert(c.date);
                    dates.append(c.date);
                }
            if (dates.size() < kRecidiveMin)
                continue;
            std::sort(dates.begin(), dates.end());
            const QDate derniere = dates.last();

            HealthAlert al;
            al.type = HealthAlert::Recidive;
            al.level = dates.size() >= 3 ? HealthAlert::Eleve : HealthAlert::Attention;
            al.cle = QString("R|%1|%2|%3|%4").arg(cle, a.key(), derniere.toString(Qt::ISODate)).arg(dates.size());
            al.diagnostic = libelle;
            al.animaux << nomAnimal(liste.last());
            al.idAnimaux << a.key();
            al.especes << liste.last().espece;
            al.dates = dates;
            al.resume = QString("Récidive : %1 — %2 (%3 cas en %4 jour(s))")
                            .arg(liste.last().nom, libelle).arg(dates.size())
                            .arg(int(dates.first().daysTo(derniere)) + 1);
            al.conseil = conseilRecidive();
            alertes.append(al);
        }

        // 3) foyer possible : plusieurs animaux différents sur une courte période
        QList<Cas> meilleure;
        QSet<QString> meilleursAnimaux;
        for (int i = 0; i < cas.size(); ++i) {
            QList<Cas> fenetre;
            QSet<QString> animaux;
            for (const Cas &c : cas)
                if (c.date <= cas[i].date && c.date.daysTo(cas[i].date) <= kFoyerJours) {
                    fenetre.append(c);
                    animaux.insert(c.idAnimal);
                }
            if (animaux.size() > meilleursAnimaux.size()
                || (animaux.size() == meilleursAnimaux.size() && !fenetre.isEmpty()
                    && (meilleure.isEmpty() || fenetre.last().date > meilleure.last().date))) {
                meilleure = fenetre;
                meilleursAnimaux = animaux;
            }
        }
        if (meilleursAnimaux.size() >= kFoyerMin && meilleure.last().date.daysTo(today) <= kActifJours) {
            HealthAlert al;
            al.type = HealthAlert::Foyer;
            al.level = meilleursAnimaux.size() >= kFoyerEleve ? HealthAlert::Eleve : HealthAlert::Attention;
            al.diagnostic = libelle;
            QSet<QString> vus;
            QSet<QDate> jours;
            for (const Cas &c : meilleure) {
                if (!vus.contains(c.idAnimal)) {
                    vus.insert(c.idAnimal);
                    al.animaux << nomAnimal(c);
                    al.idAnimaux << c.idAnimal;
                    if (!al.especes.contains(c.espece))
                        al.especes << c.espece;
                }
                if (!jours.contains(c.date)) {
                    jours.insert(c.date);
                    al.dates.append(c.date);
                }
            }
            std::sort(al.dates.begin(), al.dates.end());
            al.cle = QString("F|%1|%2|%3").arg(cle, meilleure.last().date.toString(Qt::ISODate)).arg(al.animaux.size());
            const int span = qMax(1, int(al.dates.first().daysTo(al.dates.last())) + 1);
            al.resume = QString("Foyer possible : %1 — %2 animaux en %3 jour(s)")
                            .arg(libelle).arg(al.animaux.size()).arg(span);
            al.conseil = conseilFoyer();
            alertes.append(al);
        }
    }

    // les plus graves d'abord, puis les plus récentes
    std::sort(alertes.begin(), alertes.end(), [](const HealthAlert &a, const HealthAlert &b) {
        if (a.level != b.level) return a.level > b.level;
        if (a.type != b.type) return a.type == HealthAlert::Foyer;
        return a.dates.last() > b.dates.last();
    });
    return alertes;
}

QList<HealthAlert> HealthWatch::nonVues(const QList<HealthAlert> &alertes)
{
    QSettings s;
    const QStringList vues = s.value("veille/vues").toStringList();
    QList<HealthAlert> out;
    for (const HealthAlert &a : alertes)
        if (!vues.contains(a.cle))
            out.append(a);
    return out;
}

void HealthWatch::marquerVues(const QList<HealthAlert> &alertes)
{
    QSettings s;
    QStringList vues = s.value("veille/vues").toStringList();
    for (const HealthAlert &a : alertes)
        if (!vues.contains(a.cle))
            vues.append(a.cle);
    while (vues.size() > 300)           // garde la liste courte
        vues.removeFirst();
    s.setValue("veille/vues", vues);
}
