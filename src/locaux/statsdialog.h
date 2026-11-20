#pragma once
#include <QDialog>
#include <QList>
#include "local.h"

// Fenetre "Statistiques des locaux" (meme esprit que celle des employes) :
// 4 tuiles colorees, tableau par type avec barres de taux, etat du nettoyage,
// phrase de resume. S'ouvre depuis le bouton "Voir les statistiques".
class StatsDialog : public QDialog
{
public:
    explicit StatsDialog(const QList<Local> &locaux, QWidget *parent = nullptr);
};
