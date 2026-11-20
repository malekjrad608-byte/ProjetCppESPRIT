#include "appointment.h"
#include "vaccination.h"
#include "rappelsdb.h"
#include "../models/modulesupplements.h"
#include <QJsonObject>
#include <QUuid>
static QJsonObject toJson(const RendezVous &v) { return {{"idRdv",v.idRdv},{"idAnimal",v.idAnimal},{"nomAnimal",v.nomAnimal},{"proprietaire",v.proprietaire},{"telephone",v.telephone},{"motif",v.motif},{"dateRdv",v.dateRdv.toString(Qt::ISODate)},{"statut",v.statut},{"smsRappel",v.smsRappel.toString(Qt::ISODate)},{"smsManque",v.smsManque.toString(Qt::ISODate)}}; }

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <algorithm>

namespace {

QList<RendezVous> &demoStore()
{
    static QList<RendezVous> store;
    return store;
}
bool &demoSeeded()
{
    static bool seeded = false;
    return seeded;
}
bool dbOuverte() { return QSqlDatabase::database().isOpen(); }

// vrai si l'animal a une consultation à la date du rendez-vous ou après
bool consulteDepuis(const QString &idAnimal, const QDate &date, const QVector<MedicalRecord> &records)
{
    for (const MedicalRecord &r : records)
        if (r.getIdAnimal().compare(idAnimal, Qt::CaseInsensitive) == 0 && r.getDateConsultation() >= date)
            return true;
    return false;
}

} // namespace

// ---------------------------------------------------------------- lecture
QList<RendezVous> RdvManager::lireBase()
{
    auto legacy = [&]() {
 QList<RendezVous> result;
 for(const auto &value:ModuleSupplements::read("appointment")) { auto o=value.toObject(); RendezVous v;
 v.idRdv=o["idRdv"].toString();
 v.idAnimal=o["idAnimal"].toString();
 v.nomAnimal=o["nomAnimal"].toString();
 v.proprietaire=o["proprietaire"].toString();
 v.telephone=o["telephone"].toString();
 v.motif=o["motif"].toString();
 v.dateRdv=QDateTime::fromString(o["dateRdv"].toString(),Qt::ISODate);
 v.statut=o["statut"].toString();
 v.smsRappel=QDate::fromString(o["smsRappel"].toString(),Qt::ISODate);
 v.smsManque=QDate::fromString(o["smsManque"].toString(),Qt::ISODate);
 result.append(v); } return result;

    };
    if (!OracleConnection::isOracle()) return legacy();
    QList<RendezVous> list;
    for (const RappelsDb::Ligne &l : RappelsDb::lireLignes()) {
        const QStringList en = RappelsDb::entrees(l.rdv);
        for (int i = 0; i < en.size(); ++i) {
            const QStringList f = en[i].split('|');
            if (f.size() < 2)
                continue;
            RendezVous r;
            r.idRdv        = QString("%1:%2").arg(l.idDossier).arg(i);
            r.idAnimal     = RappelsDb::idAnimalTexte(l.refAnimal);
            r.nomAnimal    = l.nom;
            r.proprietaire = l.proprietaire;
            r.telephone    = l.telephone;
            r.dateRdv      = QDateTime::fromString(f[0], Qt::ISODate);
            r.statut       = f[1];
            r.motif        = f.size() > 2 ? f[2] : QString();
            r.smsRappel    = f.size() > 3 ? QDate::fromString(f[3], Qt::ISODate) : QDate();
            r.smsManque    = f.size() > 4 ? QDate::fromString(f[4], Qt::ISODate) : QDate();
            list.append(r);
        }
    }
    list.append(legacy());
    return list;
}

QList<RendezVous> &RdvManager::demo(const QVector<MedicalRecord> &records)
{
    QList<RendezVous> &store = demoStore();
    if (demoSeeded() || records.isEmpty())
        return store;
    demoSeeded() = true;

    // Démo hors ligne : un rendez-vous demain, un manqué hier, un dans 10 jours
    static const int offsets[] = { 1, -1, 10 };
    QStringList vus;
    int i = 0;
    for (const MedicalRecord &r : records) {
        if (i >= 3) break;
        if (r.getIdAnimal().isEmpty() || vus.contains(r.getIdAnimal()))
            continue;
        vus << r.getIdAnimal();
        RendezVous v;
        v.idAnimal = r.getIdAnimal();
        v.nomAnimal = r.getNomAnimal();
        v.proprietaire = r.getProprietaire();
        v.motif = "Contrôle";
        v.dateRdv = QDateTime(QDate::currentDate().addDays(offsets[i]), QTime(10, 0));
        v.idRdv = prochainId(store);
        store.append(v);
        ++i;
    }
    return store;
}

QList<RendezVous> RdvManager::tous(const QVector<MedicalRecord> &records)
{
    QList<RendezVous> all = lireBase();
    std::sort(all.begin(), all.end(), [](const RendezVous &a, const RendezVous &b) { return a.dateRdv < b.dateRdv; });
    return all;
}

QList<RendezVous> RdvManager::pourAnimal(const QString &idAnimal, const QVector<MedicalRecord> &records)
{
    QList<RendezVous> out;
    for (const RendezVous &r : tous(records))
        if (r.idAnimal.compare(idAnimal, Qt::CaseInsensitive) == 0)
            out.append(r);
    return out;
}

bool RdvManager::prochain(const QString &idAnimal, const QVector<MedicalRecord> &records, RendezVous &out)
{
    for (const RendezVous &r : pourAnimal(idAnimal, records))
        if (r.statut == "Prévu") { out = r; return true; }
    return false;
}

QString RdvManager::dernierTelephone(const QString &idAnimal, const QVector<MedicalRecord> &records)
{
    const QList<RendezVous> list = pourAnimal(idAnimal, records);
    for (int i = list.size() - 1; i >= 0; --i)
        if (!list[i].telephone.trimmed().isEmpty())
            return list[i].telephone;
    for (const Vaccination &v : VaccinationManager::derniers(records))
        if (v.idAnimal.compare(idAnimal, Qt::CaseInsensitive) == 0 && !v.telephone.trimmed().isEmpty())
            return v.telephone;
    return QString();
}

// ---------------------------------------------------------------- écriture
QString RdvManager::prochainId(const QList<RendezVous> &existants)
{
    int maxNum = 0;
    for (const RendezVous &r : existants)
        maxNum = qMax(maxNum, r.idRdv.section('-', 1).toInt());
    return QString("RDV-%1").arg(maxNum + 1, 4, 10, QChar('0'));
}

bool RdvManager::planifier(RendezVous &r, const QVector<MedicalRecord> &records)
{
    if (r.idAnimal.trimmed().isEmpty() || !r.dateRdv.isValid() || r.dateRdv<=QDateTime::currentDateTime()) return false;
    if (!OracleConnection::isOracle()) {
 Q_UNUSED(records);
 if(r.idAnimal.trimmed().isEmpty() || !r.dateRdv.isValid() || r.dateRdv<=QDateTime::currentDateTime())return false;
 r.idRdv="RDV-"+QUuid::createUuid().toString(QUuid::WithoutBraces);
 return ModuleSupplements::update("appointment",[&](QJsonArray &rows) {
  for(int i=0;i<rows.size();++i) { auto o=rows[i].toObject();
   if(o["idAnimal"].toString().compare(r.idAnimal,Qt::CaseInsensitive)==0 && o["statut"]=="Prévu") { o["statut"]="Annulé";rows[i]=o; }
  }
  rows.append(toJson(r));return true;
 });

    }

    const QString entry=QString("%1|%2|%3||").arg(r.dateRdv.toString(Qt::ISODate),
        RappelsDb::nettoyer(r.statut),RappelsDb::nettoyer(r.motif));
    if (!RappelsDb::ajouterEntree(r.idAnimal,"RAPPELS_RDV",entry,r.telephone,&r.idRdv)) return false;
    for(const auto &old:pourAnimal(r.idAnimal,records))
        if(!old.idRdv.contains(':') && old.statut=="Prévu") changerStatut(old.idRdv,"Annulé");
    return true;
}

// modifie le champ 'champ' (1 = statut, 3 = SMS rappel, 4 = SMS manqué) du rendez-vous 'idRdv'
static bool majChampRdv(const QString &idRdv, int champ, const QString &valeur)
{
    const qlonglong dossier = idRdv.section(':', 0, 0).toLongLong();
    const int idx = idRdv.section(':', 1, 1).toInt();
    for (const RappelsDb::Ligne &l : RappelsDb::lireLignes()) {
        if (l.idDossier != dossier)
            continue;
        QStringList en = RappelsDb::entrees(l.rdv);
        if (idx < 0 || idx >= en.size())
            return false;
        QStringList f = en[idx].split('|');
        while (f.size() < 5)
            f << QString();
        f[champ] = valeur;
        en[idx] = f.join('|');
        return RappelsDb::ecrireColonne(dossier, "RAPPELS_RDV", en);
    }
    return false;
}

bool RdvManager::changerStatut(const QString &idRdv, const QString &statut)
{
    if (!OracleConnection::isOracle() || !idRdv.contains(':')) {
 return ModuleSupplements::update("appointment",[&](QJsonArray &rows) {
  for(int i=0;i<rows.size();++i) {auto o=rows[i].toObject();if(o["idRdv"].toString()==idRdv){o["statut"]=statut;rows[i]=o;return true;}}return false;
 });

    }

    if (!dbOuverte()) {
        for (RendezVous &r : demoStore())
            if (r.idRdv == idRdv) { r.statut = statut; return true; }
        return false;
    }
    return majChampRdv(idRdv, 1, statut);
}

bool RdvManager::marquerSms(const QString &idRdv, bool manque, const QDate &date)
{
    if (!OracleConnection::isOracle() || !idRdv.contains(':')) {
 return ModuleSupplements::update("appointment",[&](QJsonArray &rows) {
  for(int i=0;i<rows.size();++i) {auto o=rows[i].toObject();if(o["idRdv"].toString()==idRdv){o[manque ? "smsManque" : "smsRappel"]=date.toString(Qt::ISODate);rows[i]=o;return true;}}return false;
 });

    }

    if (!dbOuverte()) {
        for (RendezVous &r : demoStore())
            if (r.idRdv == idRdv) { (manque ? r.smsManque : r.smsRappel) = date; return true; }
        return false;
    }
    return majChampRdv(idRdv, manque ? 4 : 3, date.toString(Qt::ISODate));
}

// ---------------------------------------------------------------- logique automatique
void RdvManager::majStatuts(const QVector<MedicalRecord> &records, const QDateTime &now)
{
    for (const RendezVous &r : tous(records)) {
        if (r.statut != "Prévu")
            continue;
        if (consulteDepuis(r.idAnimal, r.dateRdv.date(), records))
            changerStatut(r.idRdv, "Venu");                 // l'animal est venu
        else if (now.date() > r.dateRdv.date())
            changerStatut(r.idRdv, "Manqué");               // la date est dépassée sans consultation
    }
}

QList<RdvSmsAction> RdvManager::aEnvoyer(const QVector<MedicalRecord> &records, const QDateTime &now)
{
    majStatuts(records, now);

    QList<RdvSmsAction> out;
    for (const RendezVous &r : tous(records)) {
        const int jours = now.date().daysTo(r.dateRdv.date());
        if (r.statut == "Prévu" && !r.smsRappel.isValid() && jours >= 0 && jours <= 1 && now <= r.dateRdv)
            out.append({ r, false });                       // rappel : la veille ou le jour même
        else if (r.statut == "Manqué" && !r.smsManque.isValid() && jours >= -14)
            out.append({ r, true });                        // rendez-vous manqué (pas plus de 14 jours après)
    }
    return out;
}
