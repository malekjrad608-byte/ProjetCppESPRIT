#pragma once
#include <QComboBox>
#include <QSqlQuery>
#include <QSqlError>
#include "../database/oracleconnection.h"

namespace LocalChoice {
inline QVariant sqlValue(int id) {
    return id > 0 ? QVariant(id) : QVariant(QMetaType(QMetaType::Int));
}
inline void populate(QComboBox *box, int selected = 0) {
    box->clear();
    box->addItem(QStringLiteral("Non affecté"), 0);
    if (OracleConnection::isOracle()) {
        QSqlQuery q(QSqlDatabase::database("aniworld-services"));
        if (q.exec("SELECT id,name,out_of_service FROM locaux ORDER BY name")) {
            while (q.next())
                box->addItem(q.value(1).toString() + (q.value(2).toInt() ? QStringLiteral(" (hors service)") : QString()), q.value(0).toInt());
        } else {
            box->setToolTip(QStringLiteral("Locaux indisponibles : ") + q.lastError().text());
            box->setEnabled(false);
        }
    } else {
        box->setEnabled(false);
        box->setToolTip(QStringLiteral("Affectation disponible avec la base Oracle commune."));
    }
    int index = box->findData(selected);
    // Never erase an existing reference when the room list cannot be loaded.
    if (selected > 0 && index < 0) {
        box->addItem(QStringLiteral("Local #%1 (à vérifier)").arg(selected), selected);
        index = box->count() - 1;
    }
    box->setCurrentIndex(qMax(0, index));
}
}
