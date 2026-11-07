#include "vaccination.h"

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
    QList<Vaccination> list;
    QSqlQuery query;
    query.prepare("SELECT ID_VACCIN, ID_ANIMAL, NOM_ANIMAL, ESPECE, PROPRIETAIRE, TEL_PROPRIETAIRE, "
                  "NOM_VACCIN, DATE_VACCIN, DATE_RAPPEL, DATE_SMS FROM VACCINATION");
    if (!query.exec()) {
        qDebug() << "SELECT VACCINATION :" << query.lastError().text();
        return list;
    }
    while (query.next()) {
        Vaccination v;
        v.idVaccin     = query.value(0).toString();
        v.idAnimal     = query.value(1).toString();
        v.nomAnimal    = query.value(2).toString();
        v.espece       = query.value(3).toString();
        v.proprietaire = query.value(4).toString();
        v.telephone    = query.value(5).toString();
        v.nomVaccin    = query.value(6).toString();
        v.dateVaccin   = query.value(7).toDate();
        v.dateRappel   = query.value(8).toDate();
        v.dateSms      = query.value(9).toDate();
        list.append(v);
    }
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
    const QList<Vaccination> all = dbOuverte() ? tousLesVaccins() : demo(records);

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
    if (!dbOuverte()) {
        v.idVaccin = prochainId(demoStore());
        demoStore().append(v);
        return true;
    }

    v.idVaccin = prochainId(tousLesVaccins());

    QSqlQuery query;
    query.prepare("INSERT INTO VACCINATION (ID_VACCIN, ID_ANIMAL, NOM_ANIMAL, ESPECE, PROPRIETAIRE, "
                  "TEL_PROPRIETAIRE, NOM_VACCIN, DATE_VACCIN, DATE_RAPPEL) "
                  "VALUES (:id, :id_animal, :nom_animal, :espece, :proprietaire, :tel, :vaccin, :date_vaccin, :date_rappel)");
    query.bindValue(":id", v.idVaccin);
    query.bindValue(":id_animal", v.idAnimal);
    query.bindValue(":nom_animal", v.nomAnimal);
    query.bindValue(":espece", v.espece);
    query.bindValue(":proprietaire", v.proprietaire);
    query.bindValue(":tel", v.telephone);
    query.bindValue(":vaccin", v.nomVaccin);
    query.bindValue(":date_vaccin", v.dateVaccin);
    query.bindValue(":date_rappel", v.dateRappel);
    const bool ok = query.exec();
    if (!ok)
        qDebug() << "INSERT VACCINATION :" << query.lastError().text();
    return ok;
}

bool VaccinationManager::enregistrerTelephone(const QString &idVaccin, const QString &tel)
{
    if (!dbOuverte()) {
        for (Vaccination &v : demoStore())
            if (v.idVaccin == idVaccin) { v.telephone = tel; return true; }
        return false;
    }
    QSqlQuery query;
    query.prepare("UPDATE VACCINATION SET TEL_PROPRIETAIRE = :tel WHERE ID_VACCIN = :id");
    query.bindValue(":tel", tel);
    query.bindValue(":id", idVaccin);
    return query.exec();
}

bool VaccinationManager::marquerSmsEnvoye(const QString &idVaccin, const QDate &date)
{
    if (!dbOuverte()) {
        for (Vaccination &v : demoStore())
            if (v.idVaccin == idVaccin) { v.dateSms = date; return true; }
        return false;
    }
    QSqlQuery query;
    query.prepare("UPDATE VACCINATION SET DATE_SMS = :d WHERE ID_VACCIN = :id");
    query.bindValue(":d", date);
    query.bindValue(":id", idVaccin);
    return query.exec();
}
