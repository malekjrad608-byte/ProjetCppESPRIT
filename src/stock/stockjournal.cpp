#include "stockjournal.h"
#include "../database/oracleconnection.h"
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QDate>
#include <QVariant>
QString StockJournal::filePath() {
    const QString dir=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir+(OracleConnection::isOracle()?"/aniworld_features.ini":"/aniworld_features_demo.ini");
}
bool StockJournal::mouvement(int id,const QString &type,int quantity,const QString &reference,QString *error) {
    QSettings settings(filePath(),QSettings::IniFormat);
    const int count=settings.beginReadArray("stockMovements");settings.endArray();
    settings.beginWriteArray("stockMovements",count+1);settings.setArrayIndex(count);
    settings.setValue("articleId",id);settings.setValue("type",type);settings.setValue("quantity",quantity);
    settings.setValue("date",QDate::currentDate().toString(Qt::ISODate));settings.setValue("reference",reference);
    settings.endArray();settings.sync();
    if(settings.status()!=QSettings::NoError){*error="Stock enregistré ; le journal local n'a pas pu être écrit.";return false;}
    error->clear();return true;
}
