#ifndef STATSDIALOG_H
#define STATSDIALOG_H

#include <QDialog>
#include "gestionnairesalles.h"

class StatsDialog : public QDialog
{
    Q_OBJECT

public:
    StatsDialog(const Statistiques& stats, QWidget *parent = nullptr);
};

#endif // STATSDIALOG_H