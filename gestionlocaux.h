#ifndef GESTIONLOCAUX_H
#define GESTIONLOCAUX_H

#include <QWidget>
#include <QVector>
#include "gestionnairesalles.h"
#include "mapwidget.h"

class QLineEdit;
class QComboBox;
class QLabel;
class QPushButton;
class QCheckBox;
class QProgressBar;
class QScrollArea;
class QGridLayout;
class QStackedWidget;
class QEvent;
class SalleCard;

class GestionLocaux : public QWidget
{
    Q_OBJECT

public:
    explicit GestionLocaux(QWidget *parent = nullptr);
    ~GestionLocaux() override;

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

private slots:
    void ajouterSalle();
    void modifierSalle();
    void supprimerSalle();
    void affecterAnimal();
    void libererSalle();
    void ouvrirStats();
    void exporterCSV();
    void exporterPDF();
    void sauvegarderFichier();
    void chargerFichier();
    void rafraichirCartes();
    void rafraichirMap();
    void rechercheChangee();
    void surClicSalleMap(int idSalle);
    void ouvrirGestionCages();
    void selectionnerSalle(int idSalle);
    void validerRemiseEnService();
    void allerPage(int index);

private:
    GestionnaireSalles gestionnaire;

    // Navigation interne du module Locaux
    QStackedWidget* pages = nullptr;

    // Page Locaux
    QLineEdit* searchBar = nullptr;
    QComboBox* filtreType = nullptr;
    QComboBox* filtreEtat = nullptr;
    QComboBox* triCombo = nullptr;
    QLabel* pillDispo = nullptr;
    QLabel* pillOccupe = nullptr;
    QLabel* pillHors = nullptr;
    QScrollArea* scrollCartes = nullptr;
    QWidget* conteneurCartes = nullptr;
    QGridLayout* grilleCartes = nullptr;
    QLabel* lblVide = nullptr;
    QVector<SalleCard*> cartes;
    int colonnes = 0;
    int idSelectionne = -1;

    // Panneau local sélectionné
    QLabel* panTitre = nullptr;
    QLabel* panPill = nullptr;
    QLabel* panInfos = nullptr;
    QWidget* boxRemise = nullptr;
    QCheckBox* chkSurfaces = nullptr;
    QCheckBox* chkMateriel = nullptr;
    QCheckBox* chkControle = nullptr;
    QPushButton* btnValider = nullptr;
    QPushButton* btnAffecter = nullptr;
    QPushButton* btnLiberer = nullptr;
    QPushButton* btnModifier = nullptr;
    QPushButton* btnSupprimer = nullptr;
    QPushButton* btnCages = nullptr;

    // Occupation globale
    QLabel* lblTaux = nullptr;
    QLabel* lblTauxNote = nullptr;
    QProgressBar* barTaux = nullptr;

    // Plan de la clinique
    MapWidget* mapWidget = nullptr;

    QWidget* creerPageLocaux();
    QWidget* creerPanneauDroit();
    QWidget* creerPagePlan();

    void mettreAJourResume();
    void mettreAJourPanneau();
    void reflowCartes(bool forcer);
    void chargerDonneesDemo();
    Salle* salleSelectionnee();
};

#endif // GESTIONLOCAUX_H
