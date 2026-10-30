#include "local.h"

RoomState Local::state() const
{
    if (outOfService)
        return RoomState::OutOfService;
    return occupancy > 0 ? RoomState::Occupied : RoomState::Available;
}

QString Local::stateText() const
{
    switch (state()) {
    case RoomState::Available:    return QStringLiteral("Disponible");
    case RoomState::Occupied:     return QStringLiteral("Occupé");
    case RoomState::OutOfService: return QStringLiteral("Hors service");
    }
    return QString();
}

QString Local::cleaningText() const
{
    switch (cleaning) {
    case CleaningState::Clean:      return QStringLiteral("Propre");
    case CleaningState::ToClean:    return QStringLiteral("À nettoyer");
    case CleaningState::InProgress: return QStringLiteral("En cours");
    }
    return QString();
}

QString Local::floorText() const
{
    return floor == 0 ? QStringLiteral("Rez-de-chaussée")
                      : QStringLiteral("Étage %1").arg(floor);
}

QString Local::floorShort() const
{
    return floor == 0 ? QStringLiteral("RDC") : QStringLiteral("Étage %1").arg(floor);
}

QString Local::iconName() const
{
    if (type == "Consultation")    return QStringLiteral("consult");
    if (type == "Chirurgie")       return QStringLiteral("surgery");
    if (type == "Hospitalisation") return QStringLiteral("bed");
    if (type == "Pension")         return QStringLiteral("home");
    if (type == "Laboratoire")     return QStringLiteral("flask");
    if (type == "Imagerie")        return QStringLiteral("scan");
    if (type == "Isolement")       return QStringLiteral("isolation");
    return QStringLiteral("rooms");
}

bool Local::isUrgentType() const
{
    return type == "Chirurgie" || type == "Isolement";
}

void Local::markToClean()
{
    cleaning = CleaningState::ToClean;
    dirtySince = QDateTime::currentDateTime();
}

void Local::startCleaning()
{
    cleaning = CleaningState::InProgress;
}

void Local::markClean()
{
    cleaning = CleaningState::Clean;
    lastCleaned = QDateTime::currentDateTime();
    dirtySince = QDateTime();
}

qint64 Local::dirtyMinutes() const
{
    if (cleaning != CleaningState::ToClean)
        return 0;
    const QDateTime since = dirtySince.isValid() ? dirtySince : lastCleaned;
    if (!since.isValid())
        return 0;
    return qMax<qint64>(0, since.secsTo(QDateTime::currentDateTime()) / 60);
}

QStringList Local::types()
{
    return { "Consultation", "Chirurgie", "Hospitalisation", "Pension",
             "Laboratoire", "Imagerie", "Isolement" };
}

QJsonObject Local::toJson() const
{
    QJsonObject o;
    o["id"] = id;
    o["name"] = name;
    o["type"] = type;
    o["floor"] = floor;
    o["capacity"] = capacity;
    o["occupancy"] = occupancy;
    o["specialty"] = specialty;
    o["outOfService"] = outOfService;
    o["cleaning"] = static_cast<int>(cleaning);
    o["lastCleaned"] = lastCleaned.toString(Qt::ISODate);
    if (dirtySince.isValid())
        o["dirtySince"] = dirtySince.toString(Qt::ISODate);
    return o;
}

Local Local::fromJson(const QJsonObject &o)
{
    Local l;
    l.id = o["id"].toInt();
    l.name = o["name"].toString();
    l.type = o["type"].toString();
    l.floor = o["floor"].toInt();
    l.capacity = o["capacity"].toInt(1);
    l.occupancy = o["occupancy"].toInt();
    l.specialty = o["specialty"].toString();
    l.outOfService = o["outOfService"].toBool();
    l.cleaning = static_cast<CleaningState>(o["cleaning"].toInt());
    l.lastCleaned = QDateTime::fromString(o["lastCleaned"].toString(), Qt::ISODate);
    if (!l.lastCleaned.isValid())
        l.lastCleaned = QDateTime::currentDateTime();
    l.dirtySince = QDateTime::fromString(o["dirtySince"].toString(), Qt::ISODate);
    return l;
}
