#include "localrepository.h"
#include "connection.h"

#include <QSqlError>
#include <QSqlQueryModel>
#include <QSqlRecord>

LocalRepository::LocalRepository(QObject *parent) : QObject(parent)
{
    // la connexion est deja ouverte dans main.cpp
    if (charger() && m_items.isEmpty()) {   // table vide -> premiers locaux
        seed();
        for (Local &l : m_items)
            l.ajouter();
    }
}

bool LocalRepository::usesDatabase() const
{
    return Connection::createInstance().isOpen();
}

QString LocalRepository::lastDbError() const
{
    return Local::derniereErreur();
}

bool LocalRepository::reconnect()
{
    if (!Connection::createInstance().createConnection() || !charger())
        return false;
    emit changed();
    return true;
}

// Read : on utilise Local::afficher() puis on transforme chaque ligne en objet Local
bool LocalRepository::charger()
{
    m_items.clear();
    Local l;
    QSqlQueryModel *model = l.afficher();
    const bool ok = !model->lastError().isValid();
    if (ok) {
        while (model->canFetchMore())
            model->fetchMore();
        for (int i = 0; i < model->rowCount(); ++i)
            m_items.append(Local::fromRecord(model->record(i)));
    }
    delete model;
    return ok;
}

void LocalRepository::seed()
{
    m_items.clear();
    const QList<QPair<int, QString>> sv = Local::services();
    if (sv.isEmpty())
        return;                       // pas de service -> pas de local possible (cle etrangere)
    const int service = sv.first().first;
    int id = 100;
    auto add = [&](const QString &name, const QString &type, int cap,
                   const QString &spec, int floor) {
        Local l;
        l.setId(++id);
        l.setName(name);
        l.setType(type);
        l.setCapacity(cap);
        l.setSpecialty(spec);
        l.setFloor(floor);
        l.setIdService(service);
        l.setLastCleaned(QDateTime::currentDateTime());
        m_items.append(l);
    };
    add("Consultation 1", "Consultation", 1, "Généraliste", 0);
    add("Consultation 2", "Consultation", 1, "Généraliste", 0);
    add("Consultation 3", "Consultation", 1, "Dentiste", 0);
    add("Bloc A", "Chirurgie", 1, "Aseptique", 1);
    add("Bloc B", "Chirurgie", 1, "Aseptique", 1);
    add("Hospitalisation 1", "Hospitalisation", 8, "Soins post-op", 1);
    add("Laboratoire", "Laboratoire", 1, "Analyses", 0);
    add("Radiologie", "Imagerie", 1, "Radio / écho", 0);
    add("Pension A", "Pension", 20, "Chiens", 0);
    add("Isolement", "Isolement", 4, "Maladies contagieuses", 1);

    // quelques etats pour voir les alertes et les couleurs du plan
    const QDateTime now = QDateTime::currentDateTime();
    for (Local &l : m_items) {
        if (l.getName() == "Bloc A") {
            l.setCleaning(CleaningState::ToClean);
            l.setDirtySince(now.addSecs(-95 * 60));
        } else if (l.getName() == "Consultation 3") {
            l.setCleaning(CleaningState::ToClean);
            l.setDirtySince(now.addSecs(-200 * 60));
        } else if (l.getName() == "Consultation 1") {
            l.setOccupancy(1);
        } else if (l.getName() == "Pension A") {
            l.setOccupancy(19);
        } else if (l.getName() == "Hospitalisation 1") {
            l.setOccupancy(8);
        } else if (l.getName() == "Radiologie") {
            l.setOutOfService(true);
        }
    }
}

int LocalRepository::nextId() const
{
    int m = 100;
    for (const Local &l : m_items)
        m = qMax(m, l.getId());
    return m + 1;
}

const Local *LocalRepository::find(int id) const
{
    for (const Local &l : m_items)
        if (l.getId() == id)
            return &l;
    return nullptr;
}

bool LocalRepository::nameExists(const QString &name, int ignoreId) const
{
    for (const Local &l : m_items)
        if (l.getId() != ignoreId && l.getName().compare(name, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}

int LocalRepository::add(Local l)
{
    l.setId(nextId());
    if (!l.ajouter())                 // Create
        return 0;
    charger();                        // actualisation : on relance afficher() (comme l'atelier)
    emit changed();
    return l.getId();
}

bool LocalRepository::update(const Local &l)
{
    Local copie = l;
    if (!copie.modifier())            // Update
        return false;
    charger();                        // actualisation
    emit changed();
    return true;
}

bool LocalRepository::remove(int id)
{
    Local l;
    if (!l.supprimer(id))             // Delete
        return false;
    charger();                        // actualisation
    emit changed();
    return true;
}
