#include "planning.h"
#include "employe.h"

#include "../models/modulesupplements.h"
#include <QJsonObject>
#include <QUuid>
#include <algorithm>
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
bool Planning::ajouter(QString *erreur) const {
 QString issue=validerSaisie();Employe employee;
 if(issue.isEmpty() && (!Employe::charger(m_responsableId,employee) || !employee.isDisponible()))issue="Choisissez un employé disponible.";
 if(!issue.isEmpty()){if(erreur)*erreur=issue;return false;}
 return ModuleSupplements::update("planning",[&](QJsonArray &rows){
  double hours=employee.getChargeHebdo();const auto week=semaineDebut(m_debut.date());
  for(const auto &value:rows) {const auto o=value.toObject();if(o["employee"].toInt()!=m_responsableId)continue;
   auto start=QDateTime::fromString(o["start"].toString(),Qt::ISODate),end=QDateTime::fromString(o["end"].toString(),Qt::ISODate);
   if(start<m_fin && m_debut<end){if(erreur)*erreur="Ce créneau chevauche une tâche déjà planifiée.";return false;}
   if(start.date()>=week && start.date()<week.addDays(7))hours+=o["hours"].toDouble();
  }
  if(hours+m_heures>employee.getDispoHebdo()){if(erreur)*erreur="La charge dépasse la disponibilité de l'employé.";return false;}
  rows.append(QJsonObject{{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)},
   {"employee",m_responsableId},{"title",m_titre.trimmed()},{"skill",m_competence.trimmed()},
   {"start",m_debut.toString(Qt::ISODate)},{"end",m_fin.toString(Qt::ISODate)},{"hours",m_heures}});return true;
 },erreur);
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
QList<Planning::Conflit> Planning::conflits() {
 QList<Conflit> out;const auto rows=ModuleSupplements::read("planning");
 for(int i=0;i<rows.size();++i)for(int j=i+1;j<rows.size();++j){auto a=rows[i].toObject(),b=rows[j].toObject();
  if(a["employee"]!=b["employee"])continue;
  auto as=QDateTime::fromString(a["start"].toString(),Qt::ISODate),ae=QDateTime::fromString(a["end"].toString(),Qt::ISODate);
  auto bs=QDateTime::fromString(b["start"].toString(),Qt::ISODate),be=QDateTime::fromString(b["end"].toString(),Qt::ISODate);
  Employe e;if(as<be && bs<ae && Employe::charger(a["employee"].toInt(),e))out.append({e.getPrenom()+" "+e.getNom(),a["title"].toString(),a["start"].toString(),a["end"].toString(),b["title"].toString(),b["start"].toString(),b["end"].toString()});
 }return out;
}

QList<Planning::Plage> Planning::plagesAPartirDe(const QDate &date) {
 QList<Plage> out;for(const auto &value:ModuleSupplements::read("planning")) {auto o=value.toObject();Employe e;
  auto start=QDateTime::fromString(o["start"].toString(),Qt::ISODate),end=QDateTime::fromString(o["end"].toString(),Qt::ISODate);
  if(end.date()>=date && Employe::charger(o["employee"].toInt(),e))out.append({e.getId(),e.getPrenom(),e.getNom(),start,end});
 }std::sort(out.begin(),out.end(),[](const Plage &a,const Plage &b){return a.responsableId==b.responsableId?a.debut<b.debut:a.responsableId<b.responsableId;});return out;
}

double Planning::chargePlanifieeSemaine(const QDate &week) {
 double sum=0;for(const auto &e:Employe::afficher()){double hours=0;chargePlanifieeEmploye(e.getId(),week,hours);sum+=hours;}return sum;
}

bool Planning::chargePlanifieeEmploye(int id,const QDate &debutSemaine,double &heures) {
 heures=0;QString error;auto rows=ModuleSupplements::read("planning",&error);if(!error.isEmpty())return false;
 for(const auto &v:rows){auto o=v.toObject();const auto d=QDateTime::fromString(o["start"].toString(),Qt::ISODate).date();
 if(o["employee"].toInt()==id && d>=debutSemaine && d<debutSemaine.addDays(7))heures+=o["hours"].toDouble();}return true;
}

bool Planning::aConflit(int id,const QDateTime &debut,const QDateTime &fin) {
 for(const auto &p:plagesAPartirDe(debut.date()))if(p.responsableId==id && p.debut<fin && debut<p.fin)return true;return false;
}
