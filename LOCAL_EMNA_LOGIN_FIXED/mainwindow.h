#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
class QWidget;
class QEvent;
class SalleCard;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

private slots:
    // --- Fonctions existantes (logique inchangée) ---
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
    void rafraichirCartes();      // (remplace rafraichirTable)
    void rafraichirMap();
    void rechercheChangee();
    void surClicSalleMap(int idSalle);
    void ouvrirGestionCages();

    // --- Nouveautés liées au design ---
    void selectionnerSalle(int idSalle);
    void validerRemiseEnService();
    void allerPage(int index);

private:
    GestionnaireSalles gestionnaire;

    // Navigation
    QStackedWidget* pages = nullptr;
    QLabel* crumbCourant = nullptr;

    // Page "Locaux" : recherche, filtres, cartes
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

    // Panneau de droite : local sélectionné
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

    // Panneau de droite : occupation globale
    QLabel* lblTaux = nullptr;
    QLabel* lblTauxNote = nullptr;
    QProgressBar* barTaux = nullptr;

    // Plan de la clinique
    MapWidget* mapWidget = nullptr;

    // --- Construction de l'interface ---
    void setupUI();
    QWidget* creerSidebar();
    QWidget* creerTopBar();
    QWidget* creerFooter();
    QWidget* creerPageLocaux();
    QWidget* creerPanneauDroit();
    QWidget* creerPagePlan();
    QWidget* creerPagePlaceholder(const QString& titre);

    // --- Mise à jour de l'affichage ---
    void reflowCartes(bool forcer);
    void mettreAJourResume();
    void mettreAJourPanneau();

    // --- Données ---
    void chargerDonneesDemo();
    Salle* salleSelectionnee();
};

#endif // MAINWINDOW_H
