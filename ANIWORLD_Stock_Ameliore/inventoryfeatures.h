#ifndef INVENTORYFEATURES_H
#define INVENTORYFEATURES_H

#include <QVector>
#include <QWidget>
#include "stockwidget.h"

class InventoryFeatures
{
public:
    // Return true when invoice validation changed the stock and the main view should reload.
    static bool showPurchases(QWidget *parent, const QVector<StockRecord> &items);
    static void showAlerts(QWidget *parent, const QVector<StockRecord> &items);
    static void showSmartReplenishment(QWidget *parent, const QVector<StockRecord> &items);
};

#endif // INVENTORYFEATURES_H
