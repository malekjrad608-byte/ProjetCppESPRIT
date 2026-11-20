#pragma once
#include <QList>
#include <QString>
#include "local.h"

namespace PdfReport {

// Rapport destine au personnel de nettoyage :
// locaux a nettoyer classes par priorite, avec case "Fait" et remarques,
// puis liste des locaux propres. Retourne false (et un message) en cas d'erreur.
bool generate(const QList<Local> &locaux, const QString &path, QString *error = nullptr);

}
