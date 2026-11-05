#include "appointment.h"
#include "vaccination.h"

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
    QList<RendezVous> list;
    QSqlQuery query;
    query.prepare("SELECT ID_RDV, ID_ANIMAL, NOM_ANIMAL, PROPRIETAIRE, TEL_PROPRIETAIRE, MOTIF, "
                  "DATE_RDV, STATUT, DATE_SMS_RAPPEL, DATE_SMS_MANQUE FROM RENDEZVOUS");
    if (!query.exec()) {
        qDebug() << "SELECT RENDEZVOUS :" << query.lastError().text();
        return list;
    }
    while (query.next()) {
        RendezVous r;
        r.idRdv        = query.value(0).toString();
        r.idAnimal     = query.value(1).toString();
        r.nomAnimal    = query.value(2).toString();
        r.proprietaire = query.value(3).toString();
        r.telephone    = query.value(4).toString();
        r.motif        = query.value(5).toString();
        r.dateRdv      = query.value(6).toDateTime();
        r.statut       = query.value(7).toString();
        r.smsRappel    = query.value(8).toDate();
        r.smsManque    = query.value(9).toDate();
        list.append(r);
    }
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
    QList<RendezVous> all = dbOuverte() ? lireBase() : demo(records);
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
    // un seul rendez-vous "Prévu" par animal : l'ancien est annulé
    for (const RendezVous &old : pourAnimal(r.idAnimal, records))
        if (old.statut == "Prévu")
            changerStatut(old.idRdv, "Annulé");

    if (!dbOuverte()) {
        r.idRdv = prochainId(demoStore());
        demoStore().append(r);
        return true;
    }

    r.idRdv = prochainId(lireBase());
    QSqlQuery query;
    query.prepare("INSERT INTO RENDEZVOUS (ID_RDV, ID_ANIMAL, NOM_ANIMAL, PROPRIETAIRE, TEL_PROPRIETAIRE, "
                  "MOTIF, DATE_RDV, STATUT) VALUES (:id, :animal, :nom, :proprio, :tel, :motif, :date, :statut)");
    query.bindValue(":id", r.idRdv);
    query.bindValue(":animal", r.idAnimal);
    query.bindValue(":nom", r.nomAnimal);
    query.bindValue(":proprio", r.proprietaire);
    query.bindValue(":tel", r.telephone);
    query.bindValue(":motif", r.motif);
    query.bindValue(":date", r.dateRdv);
    query.bindValue(":statut", r.statut);
    const bool ok = query.exec();
    if (!ok)
        qDebug() << "INSERT RENDEZVOUS :" << query.lastError().text();
    return ok;
}

bool RdvManager::changerStatut(const QString &idRdv, const QString &statut)
{
    if (!dbOuverte()) {
        for (RendezVous &r : demoStore())
            if (r.idRdv == idRdv) { r.statut = statut; return true; }
        return false;
    }
    QSqlQuery query;
    query.prepare("UPDATE RENDEZVOUS SET STATUT = :s WHERE ID_RDV = :id");
    query.bindValue(":s", statut);
    query.bindValue(":id", idRdv);
    return query.exec();
}

bool RdvManager::marquerSms(const QString &idRdv, bool manque, const QDate &date)
{
    if (!dbOuverte()) {
        for (RendezVous &r : demoStore())
            if (r.idRdv == idRdv) { (manque ? r.smsManque : r.smsRappel) = date; return true; }
        return false;
    }
    QSqlQuery query;
    query.prepare(QString("UPDATE RENDEZVOUS SET %1 = :d WHERE ID_RDV = :id")
                      .arg(manque ? "DATE_SMS_MANQUE" : "DATE_SMS_RAPPEL"));
    query.bindValue(":d", date);
    query.bindValue(":id", idRdv);
    return query.exec();
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
