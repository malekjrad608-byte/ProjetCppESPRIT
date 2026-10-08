#pragma once
#include "../models/service.h"
#include <QList>

namespace ServiceReport {
bool writePdf(const QString &path, const QList<Service> &services,
              const QString &scope, QString *error = nullptr);
}
