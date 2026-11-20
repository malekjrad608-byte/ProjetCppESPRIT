#pragma once
#include <QFrame>
#include "local.h"

class QLabel;
class QPushButton;
class OccupancyRing;
class LifeStepper;

// Panneau de droite : fiche du local selectionne
//  - entete colore selon l'ETAT (icone du type, nom, type · etage · id, badge)
//  - anneau d'occupation anime + phrases d'aide
//  - cycle de vie : Disponible -> Occupé -> À nettoyer
//  - boutons qui s'adaptent a l'etat (une action impossible est grisee avec une explication)
class DetailPanel : public QFrame
{
    Q_OBJECT
public:
    explicit DetailPanel(QWidget *parent = nullptr);
    // nullptr = aucune selection
    void setLocal(const Local *l);

signals:
    void assignRequested();
    void freeRequested();
    void editRequested();
    void cleanRequested();
    void deleteRequested();

private:
    QWidget *m_empty;          // message "clique sur un local"
    QWidget *m_content;        // fiche complete

    QFrame *m_head;
    QLabel *m_icon;
    QLabel *m_name;
    QLabel *m_sub;
    QLabel *m_badge;

    OccupancyRing *m_ring;
    QLabel *m_occTitle;
    QLabel *m_occLine1;
    QLabel *m_occLine2;

    LifeStepper *m_stepper;
    QLabel *m_specialty;
    QLabel *m_service;

    QPushButton *m_btnMain;    // "Affecter un animal" ou "Nettoyage terminé"
    QPushButton *m_btnFree;
    QPushButton *m_btnClean;
    QPushButton *m_btnEdit;
    QPushButton *m_btnDelete;

    // confirmation de suppression affichee DANS le panneau (pas de fenetre)
    QFrame *m_confirm;
    QLabel *m_confirmText;
    bool m_mainCleans = false; // true : le bouton principal termine le nettoyage
};
