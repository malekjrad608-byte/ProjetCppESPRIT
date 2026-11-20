#include "vaccination.h"
#include "rappelsdb.h"
#include "../models/modulesupplements.h"
#include <QJsonObject>
#include <QUuid>
static QJsonObject toJson(const Vaccination &v) { return {{"idVaccin",v.idVaccin},{"idAnimal",v.idAnimal},{"nomAnimal",v.nomAnimal},{"espece",v.espece},{"proprietaire",v.proprietaire},{"telephone",v.telephone},{"nomVaccin",v.nomVaccin},{"dateVaccin",v.dateVaccin.toString(Qt::ISODate)},{"dateRappel",v.dateRappel.toString(Qt::ISODate)},{"dateSms",v.dateSms.toString(Qt::ISODate)}}; }

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMap>
#include <QDebug>
#include <algorithm>

namespace {

// Données de démonstration (mode hors ligne) : gardées en mémoire pour la durée du programme.
QList<Vaccination> &demoStore()
{
    static QList<Vaccination> store;
    return store;
}
bool &demoSeeded()
{
    static bool seeded = false;
    return seeded;
}

bool dbOuverte() { return QSqlDatabase::database().isOpen(); }

} // namespace

// ---------------------------------------------------------------- catalogue
QList<VaccineType> VaccinationManager::catalogue(const QString &espece)
{
    if (espece == "Chien")
        return { {"Rage", 12}, {"CHPPi (Carré, Hépatite, Parvovirose, Parainfluenza)", 12},
                 {"Leptospirose", 12}, {"Toux de chenil", 12} };
    if (espece == "Chat")
        return { {"Rage", 12}, {"Typhus (Panleucopénie)", 12}, {"Coryza", 12},
                 {"Leucose féline (FeLV)", 12} };
    if (espece == "Lapin")
        return { {"Myxomatose", 12}, {"VHD (maladie hémorragique)", 12} };
    return { {"Vaccin annuel", 12} };
}

int VaccinationManager::intervalleParDefaut(const QString &espece, const QString &nomVaccin)
{
    for (const VaccineType &t : catalogue(espece))
        if (t.nom.compare(nomVaccin, Qt::CaseInsensitive) == 0)
            return t.intervalleMois;
    return 12;
}

// ---------------------------------------------------------------- lecture
QList<Vaccination> VaccinationManager::tousLesVaccins()
{
    auto legacy = [&]() {
 QList<Vaccination> result;
 for(const auto &value:ModuleSupplements::read("vaccination")) { auto o=value.toObject(); Vaccination v;
 v.idVaccin=o["idVaccin"].toString();
 v.idAnimal=o["idAnimal"].toString();
 v.nomAnimal=o["nomAnimal"].toString();
 v.espece=o["espece"].toString();
 v.proprietaire=o["proprietaire"].toString();
 v.telephone=o["telephone"].toString();
 v.nomVaccin=o["nomVaccin"].toString();
 v.dateVaccin=QDate::fromString(o["dateVaccin"].toString(),Qt::ISODate);
 v.dateRappel=QDate::fromString(o["dateRappel"].toString(),Qt::ISODate);
 v.dateSms=QDate::fromString(o["dateSms"].toString(),Qt::ISODate);
 result.append(v); } return result;

    };
    if (!OracleConnection::isOracle()) return legacy();
    QList<Vaccination> list;
    for (const RappelsDb::Ligne &l : RappelsDb::lireLignes()) {
        const QStringList en = RappelsDb::entrees(l.vaccins);
        for (int i = 0; i < en.size(); ++i) {
            const QStringList f = en[i].split('|');
            if (f.size() < 3)
                continue;
            Vaccination v;
            v.idVaccin     = QString("%1:%2").arg(l.idDossier).arg(i);
            v.idAnimal     = RappelsDb::idAnimalTexte(l.refAnimal);
            v.nomAnimal    = l.nom;
            v.espece       = l.espece;
            v.proprietaire = l.proprietaire;
            v.telephone    = l.telephone;
            v.nomVaccin    = f[0];
            v.dateVaccin   = QDate::fromString(f[1], Qt::ISODate);
            v.dateRappel   = QDate::fromString(f[2], Qt::ISODate);
            v.dateSms      = f.size() > 3 ? QDate::fromString(f[3], Qt::ISODate) : QDate();
            list.append(v);
        }
    }
    list.append(legacy());
    return list;
}

QList<Vaccination> &VaccinationManager::demo(const QVector<MedicalRecord> &records)
{
    QList<Vaccination> &store = demoStore();
    if (demoSeeded() || records.isEmpty())
        return store;
    demoSeeded() = true;

    // Pour chaque animal connu : 1 ou 2 vaccins avec des échéances variées (en retard / proches / lointaines),
    // calculées par rapport à aujourd'hui pour que la démo montre toujours des alertes.
    static const int offsets[] = { -45, -3, 12, 200, 20, -10, 120, 5 };
    QStringList vus;
    int i = 0;
    for (const MedicalRecord &r : records) {
        if (r.getIdAnimal().isEmpty() || vus.contains(r.getIdAnimal()))
            continue;
        vus << r.getIdAnimal();
        const QList<VaccineType> cat = catalogue(r.getEspece());
        for (int j = 0; j < qMin(2, int(cat.size())); ++j) {
            Vaccination v;
            v.idAnimal = r.getIdAnimal();
            v.nomAnimal = r.getNomAnimal();
            v.espece = r.getEspece();
            v.proprietaire = r.getProprietaire();
            v.nomVaccin = cat[j].nom;
            v.dateRappel = QDate::currentDate().addDays(offsets[(i + j * 3) % 8]);
            v.dateVaccin = v.dateRappel.addMonths(-cat[j].intervalleMois);
            v.idVaccin = prochainId(store);
            store.append(v);
        }
        ++i;
    }
    return store;
}

QList<Vaccination> VaccinationManager::derniers(const QVector<MedicalRecord> &records)
{
    const QList<Vaccination> all = tousLesVaccins();

    QMap<QString, Vaccination> latest;       // clé = animal|vaccin
    for (const Vaccination &v : all) {
        const QString key = v.idAnimal.toLower() + '|' + v.nomVaccin.toLower();
        auto it = latest.find(key);
        if (it == latest.end() || v.dateVaccin > it->dateVaccin
            || (v.dateVaccin == it->dateVaccin && v.idVaccin > it->idVaccin))
            latest.insert(key, v);
    }
    return latest.values();
}

QList<Vaccination> VaccinationManager::aRappeler(const QVector<MedicalRecord> &records, int joursAvant)
{
    QList<Vaccination> out;
    for (const Vaccination &v : derniers(records))
        if (v.etat(joursAvant) != Vaccination::AJour)
            out.append(v);
    std::sort(out.begin(), out.end(), [](const Vaccination &a, const Vaccination &b) {
        return a.dateRappel < b.dateRappel;
    });
    return out;
}

QString VaccinationManager::statutAnimal(const QString &idAnimal, const QVector<MedicalRecord> &records)
{
    bool suivi = false;
    for (const Vaccination &v : derniers(records)) {
        if (v.idAnimal.compare(idAnimal, Qt::CaseInsensitive) != 0)
            continue;
        suivi = true;
        if (v.etat(30) != Vaccination::AJour)
            return "À renouveler";
    }
    return suivi ? QString("À jour") : QString();
}

// ---------------------------------------------------------------- écriture
QString VaccinationManager::prochainId(const QList<Vaccination> &existants)
{
    int maxNum = 0;
    for (const Vaccination &v : existants)
        maxNum = qMax(maxNum, v.idVaccin.section('-', 1).toInt());
    return QString("VAC-%1").arg(maxNum + 1, 4, 10, QChar('0'));
}

bool VaccinationManager::ajouter(Vaccination &v)
{
    if (v.nomVaccin.trimmed().isEmpty() || v.idAnimal.trimmed().isEmpty() || !v.dateVaccin.isValid() || !v.dateRappel.isValid() || v.dateRappel<=v.dateVaccin) return false;
    if (!OracleConnection::isOracle()) {
 if(v.idAnimal.trimmed().isEmpty() || v.nomVaccin.trimmed().isEmpty() || !v.dateVaccin.isValid() || !v.dateRappel.isValid() || v.dateRappel<v.dateVaccin)return false;
 v.idVaccin="VAC-"+QUuid::createUuid().toString(QUuid::WithoutBraces);
 return ModuleSupplements::update("vaccination",[&](QJsonArray &rows){rows.append(toJson(v));return true;});

    }

    const QString entry=QString("%1|%2|%3|").arg(RappelsDb::nettoyer(v.nomVaccin),
        v.dateVaccin.toString(Qt::ISODate),v.dateRappel.toString(Qt::ISODate));
    return RappelsDb::ajouterEntree(v.idAnimal,"RAPPELS_VACCINS",entry,v.telephone,&v.idVaccin);
}

bool VaccinationManager::enregistrerTelephone(const QString &idVaccin, const QString &tel)
{
    if (!OracleConnection::isOracle() || !idVaccin.contains(':')) {
 return ModuleSupplements::update("vaccination",[&](QJsonArray &rows) {
  for(int i=0;i<rows.size();++i) {auto o=rows[i].toObject();if(o["idVaccin"].toString()==idVaccin){o["telephone"]=tel;rows[i]=o;return true;}}return false;
 });

    }

    if (!dbOuverte()) {
        for (Vaccination &v : demoStore())
            if (v.idVaccin == idVaccin) { v.telephone = tel; return true; }
        return false;
    }
    return RappelsDb::assurerTables() && RappelsDb::ecrireTelephone(idVaccin.section(':', 0, 0).toLongLong(), tel);
}

bool VaccinationManager::marquerSmsEnvoye(const QString &idVaccin, const QDate &date)
{
    if (!OracleConnection::isOracle() || !idVaccin.contains(':')) {
 return ModuleSupplements::update("vaccination",[&](QJsonArray &rows) {
  for(int i=0;i<rows.size();++i) {auto o=rows[i].toObject();if(o["idVaccin"].toString()==idVaccin){o["dateSms"]=date.toString(Qt::ISODate);rows[i]=o;return true;}}return false;
 });

    }

    if (!dbOuverte()) {
        for (Vaccination &v : demoStore())
            if (v.idVaccin == idVaccin) { v.dateSms = date; return true; }
        return false;
    }
    const qlonglong dossier = idVaccin.section(':', 0, 0).toLongLong();
    const int idx = idVaccin.section(':', 1, 1).toInt();
    for (const RappelsDb::Ligne &l : RappelsDb::lireLignes()) {
        if (l.idDossier != dossier)
            continue;
        QStringList en = RappelsDb::entrees(l.vaccins);
        if (idx < 0 || idx >= en.size())
            return false;
        QStringList f = en[idx].split('|');
        while (f.size() < 4)
            f << QString();
        f[3] = date.toString(Qt::ISODate);
        en[idx] = f.join('|');
        return RappelsDb::ecrireColonne(dossier, "RAPPELS_VACCINS", en);
    }
    return false;
}
