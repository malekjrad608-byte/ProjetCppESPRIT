#include "cage.h"

Cage::Cage(int numero)
    : numero(numero), occupee(false),
    nomAnimal(""), typeAnimal(""),
    dateArrivee(QDate::currentDate()), nombreJours(0) {}

void Cage::occuper(const QString& nom, const QString& type,
                   const QDate& date, int jours) {
    occupee = true;
    nomAnimal = nom;
    typeAnimal = type;
    dateArrivee = date;
    nombreJours = jours;
}

void Cage::liberer() {
    occupee = false;
    nomAnimal = "";
    typeAnimal = "";
    dateArrivee = QDate::currentDate();
    nombreJours = 0;
}

QString Cage::toCSV() const {
    if (!occupee) return QString::number(numero) + ";libre;;;;";
    return QString("%1;occupee;%2;%3;%4;%5")
        .arg(numero)
        .arg(nomAnimal)
        .arg(typeAnimal)
        .arg(dateArrivee.toString("yyyy-MM-dd"))
        .arg(nombreJours);
}