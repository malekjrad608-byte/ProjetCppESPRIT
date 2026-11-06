#pragma once
#include <QString>
class StockJournal {
public:
    static QString filePath();
    static bool mouvement(int id,const QString &type,int quantity,const QString &reference,QString *error);
};
