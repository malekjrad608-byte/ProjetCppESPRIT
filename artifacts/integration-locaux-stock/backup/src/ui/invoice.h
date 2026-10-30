#pragma once
#include "../models/service.h"

namespace Invoice {
QString html(const Service &service, bool demo=false);
bool writePdf(const QString &path, const Service &service, bool demo=false, QString *error=nullptr);
// Shared native Qt PDF renderer, also used by the filtered stock inventory.
bool writeDocument(const QString &path, const QString &html, const QString &title, QString *error=nullptr);
QString escape(const QString &text);
}
