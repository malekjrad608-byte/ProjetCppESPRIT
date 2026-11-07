#include "planning.h"
#include "employe.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QtGlobal>

namespace {
const QString FORMAT_SQL = QStringLiteral("yyyy-MM-dd HH:mm:ss");
}

Planning::Planning() {}

// ======================================================================
//  Utilitaires
// ======================================================================
QDate Planning::semaineDebut(const QDate &date)
{
    return date.addDays(1 - date.dayOfWeek());
}

QString Planning::borneSemaine(const QDate &date)
{
    return date.toString("yyyy-MM-dd") + " 00:00:00";
}

// ======================================================================
//  Controle de saisie
// ======================================================================
QString Planning::validerSaisie() const
{
    if (m_titre.trimmed().isEmpty() || m_competence.trimmed().isEmpty())
        return "Saisissez le nom de la tâche et la spécialité requise.";
    if (m_titre.trimmed().size() > 200 || m_competence.trimmed().size() > 200)
        return "La tâche et la spécialité sont limitées à 200 caractères.";
    if (!m_debut.isValid() || !m_fin.isValid() || m_fin <= m_debut)
        return "La fin doit être postérieure au début.";
    if (m_heures < 0.5 || m_heures > 24.0)
        return "La charge de la tâche doit être comprise entre 0,5 h et 24 h.";
    return QString();
}

// ======================================================================
//  Create
// ======================================================================
bool Planning::ajouter(QString *erreur) const
{
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("INSERT INTO planning_affectations "
              "(titre, competence_requise, debut, fin, charge_heures, responsable_id) "
              "VALUES (:titre, :competence, :debut, :fin, :heures, :id)");
    q.bindValue(":titre", m_titre.trimmed());
    q.bindValue(":competence", m_competence.trimmed());
    q.bindValue(":debut", m_debut.toString(FORMAT_SQL));
    q.bindValue(":fin", m_fin.toString(FORMAT_SQL));
    q.bindValue(":heures", m_heures);
    q.bindValue(":id", m_responsableId);
    const bool ok = q.exec();
    if (!ok && erreur) *erreur = q.lastError().text();
    q.finish();
    return ok;
}

// ======================================================================
//  Affectation automatique
// ======================================================================
Planning::Resultat Planning::affecterAutomatiquement(QString &nomResponsable,
                                                     double &tauxPrevu,
                                                     QString &erreur)
{
    const QDate debutSemaine = semaineDebut(m_debut.date());
    const QList<Employe> candidats = Employe::listerDisponibles();

    int meilleurId = -1;
    QString meilleurNom;
    double meilleurTaux = 1e9;

    for (const Employe &emp : candidats) {
        if (!emp.getCompetences().contains(m_competence, Qt::CaseInsensitive) &&
            !emp.getPoste().contains(m_competence, Qt::CaseInsensitive))
            continue;

        if (aConflit(emp.getId(), m_debut, m_fin))
            continue;

        double sumPlanifie = 0.0;
        if (!chargePlanifieeEmploye(emp.getId(), debutSemaine, sumPlanifie))
            continue;

        const double charge = emp.getChargeHebdo() + sumPlanifie;
        const double capacite = emp.getDispoHebdo();
        if (capacite <= 0 || charge + m_heures > capacite)
            continue;

        const double taux = (charge + m_heures) / capacite;
        if (taux < meilleurTaux) {
            meilleurTaux = taux;
            meilleurId = emp.getId();
            meilleurNom = emp.getPrenom() + " " + emp.getNom();
        }
    }

    if (meilleurId < 0)
        return Resultat::AucunResponsable;

    m_responsableId = meilleurId;
    if (!ajouter(&erreur))
        return Resultat::ErreurBase;

    nomResponsable = meilleurNom;
    tauxPrevu = meilleurTaux;
    return Resultat::Ok;
}

// ======================================================================
//  Lectures
// ======================================================================
QList<Planning::Conflit> Planning::conflits()
{
    QList<Conflit> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT e.prenom || ' ' || e.nom, p1.titre, p1.debut, p1.fin, "
              "p2.titre, p2.debut, p2.fin "
              "FROM planning_affectations p1 "
              "JOIN planning_affectations p2 ON p1.responsable_id=p2.responsable_id "
              "AND p1.id<p2.id AND p1.debut<p2.fin AND p2.debut<p1.fin "
              "JOIN employes e ON e.id=p1.responsable_id "
              "ORDER BY p1.debut FETCH FIRST 20 ROWS ONLY");
    if (q.exec()) {
        while (q.next()) {
            Conflit c;
            c.employe = q.value(0).toString();
            c.titre1 = q.value(1).toString();
            c.debut1 = q.value(2).toString();
            c.fin1 = q.value(3).toString();
            c.titre2 = q.value(4).toString();
            c.debut2 = q.value(5).toString();
            c.fin2 = q.value(6).toString();
            liste.append(c);
        }
    }
    q.finish();
    return liste;
}

QList<Planning::Plage> Planning::plagesAPartirDe(const QDate &date)
{
    QList<Plage> liste;
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT p.responsable_id, e.prenom, e.nom, p.debut, p.fin "
              "FROM planning_affectations p JOIN employes e ON e.id=p.responsable_id "
              "WHERE p.fin >= :maintenant ORDER BY p.responsable_id, p.debut");
    q.bindValue(":maintenant", borneSemaine(date));
    if (q.exec()) {
        while (q.next()) {
            Plage p;
            p.responsableId = q.value(0).toInt();
            p.prenom = q.value(1).toString();
            p.nom = q.value(2).toString();
            p.debut = QDateTime::fromString(q.value(3).toString(), FORMAT_SQL);
            p.fin = QDateTime::fromString(q.value(4).toString(), FORMAT_SQL);
            if (p.debut.isValid() && p.fin.isValid())
                liste.append(p);
        }
    }
    q.finish();
    return liste;
}

double Planning::chargePlanifieeSemaine(const QDate &debutSemaine)
{
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT COALESCE(SUM(charge_heures),0) FROM planning_affectations "
              "WHERE debut >= :debut AND debut < :fin");
    q.bindValue(":debut", borneSemaine(debutSemaine));
    q.bindValue(":fin", borneSemaine(debutSemaine.addDays(7)));
    double total = 0.0;
    if (q.exec() && q.next())
        total = q.value(0).toDouble();
    q.finish();
    return total;
}

bool Planning::chargePlanifieeEmploye(int id, const QDate &debutSemaine, double &heures)
{
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT COALESCE(SUM(charge_heures),0) FROM planning_affectations "
              "WHERE responsable_id=:id AND debut >= :debutSemaine AND debut < :finSemaine");
    q.bindValue(":id", id);
    q.bindValue(":debutSemaine", borneSemaine(debutSemaine));
    q.bindValue(":finSemaine", borneSemaine(debutSemaine.addDays(7)));
    bool ok = false;
    if (q.exec() && q.next()) {
        heures = q.value(0).toDouble();
        ok = true;
    }
    q.finish();
    return ok;
}

bool Planning::aConflit(int id, const QDateTime &debut, const QDateTime &fin)
{
    QSqlQuery q;
    q.setForwardOnly(true);
    q.prepare("SELECT COUNT(*) FROM planning_affectations "
              "WHERE responsable_id=:id AND debut < :fin AND :debut < fin");
    q.bindValue(":id", id);
    q.bindValue(":debut", debut.toString(FORMAT_SQL));
    q.bindValue(":fin", fin.toString(FORMAT_SQL));
    bool conflit = false;
    if (q.exec() && q.next())
        conflit = q.value(0).toInt() > 0;
    q.finish();
    return conflit;
}
