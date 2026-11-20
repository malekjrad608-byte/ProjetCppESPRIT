# Intégration ANIWORLD — 7 novembre 2026

## Sources intégrées

- Stock v4 : `C:/Users/malek/Downloads/stock crud v2/ANIWORLD_Stock_Ameliore_QtOnly_v4/ANIWORLD_Stock_Ameliore`.
- Médical, Employés, Locaux : `C:/Users/malek/Downloads/ProjetCppESPRIT-integration-medicaux-employes-locale/ProjetCppESPRIT-integration-medicaux-employes-locale`.
- Guide : `C:/Users/malek/Downloads/GUIDE_Integration _couche_Métier (2).pdf`.

Projet à ouvrir : `gestion_service.pro` dans `C:/Users/malek/Documents/gestion_service`.

## Organisation commune

`src/mainwindow.ui` est le formulaire Designer commun aux cinq gestions. Il contient ServicePage, StockPage, EmployeesPage, LocauxPage et MedicalPage. `tools/generate_integrated_ui.py` produit automatiquement les classes Ui de chaque page au lancement de qmake/make. Les fenêtres complémentaires (formulaires, statistiques, aperçu PDF) restent créées en C++.

La navigation et les droits restent dans MainWindow. Chaque module conserve sa classe métier, ses méthodes CRUD et ses signaux/slots. `sources.pri` regroupe les sources par module. Les requêtes SQL sont dans les modèles, avec paramètres liés. La connexion Oracle est unique : `src/database/connection.h` contient les paramètres privés et `connection.cpp` ouvre le DSN commun. Les paramètres récupérés de la configuration existante n'ont pas été remplacés par ceux des projets sources.

## Modules

- **Stock** : recherche, tri, CRUD, mouvements, péremption, statistiques, fournisseurs, achats/import PDF, bon de commande PDF réimportable, alertes et préparation d'une notification au local. La réception valide toutes les lignes dans une seule transaction Oracle. Les articles existants gardent leur ID, local, type et date de péremption ; une nouvelle référence exige un local. Le retrait garde une quantité zéro. Le remplacement crée une nouvelle référence.
- **Services** : le bouton Facture est disponible avec Oracle. Le document utilise le propriétaire/référence et le tarif relus depuis SERVICE. Le PDF est un document de facturation ; aucun nouveau champ de paiement ou table de facture n'est créé.
- **Locaux** : formulaire intégré dans la page, gestion d'occupation et de nettoyage, affectation intelligente, plan de la clinique, statistiques, rapport PDF et alertes. Le choix du service utilise les IDs de SERVICE.
- **Médical** : dossiers, propriétaire saisissable, espèces/races, recherche, statistiques, fiche animal PDF, rendez-vous, vaccinations/rappels, veille sanitaire et suggestions. Le responsable est choisi dans EMPLOYER. Les envois manuels de SMS nécessitent une passerelle configurée ; aucun message n'est envoyé par la vérification de l'intégration.
- **Employés** : CRUD des attributs existants, statistiques, planning de services conservé, nouvelle planification de tâches avec contrôle des conflits et de la capacité, évaluations avec historique et PDF.

## Oracle conservé

Aucun changement du schéma Oracle n'est requis par cette intégration. Les cinq tables utilisées restent SERVICE, LOCAL, STOCK, EMPLOYER et MEDICAL. Les relations sont LOCAL.ID_SERV → SERVICE.ID_SERV, STOCK.ID_LOCAL → LOCAL.ID_LOCAL, EMPLOYER.ID_SERV → SERVICE.ID_SERV et MEDICAL.ID_EMP → EMPLOYER.ID_EMP. Les identifiants restent générés par Oracle et les contraintes UNIQUE sont conservées.

Le formulaire Services lit les propriétaires existants dans MEDICAL pour la référence. Il reste dans Services lors de l'ajout et de l'annulation.

Les fonctions supplémentaires des projets fournis demandaient des tables/attributs absents de ce schéma. Pour respecter la consigne de ne pas modifier Oracle :

- rendez-vous, suivi vaccinal, compléments médicaux, occupation/nettoyage détaillé, tâches et évaluations sont conservés dans des fichiers JSON du dossier de données Qt, sous `modules/<identité de connexion>/` ; écriture atomique et verrou de fichier ;
- fournisseurs, factures d'achat/prix et mouvements Stock restent dans `aniworld_features.ini` ;
- ces données complémentaires sont locales au poste : elles ne sont pas synchronisées avec les autres PC. Sauvegarder le dossier de données de l'application avec la base.

L'état Oracle d'un local reste Disponible, Occupé ou Hors service. Le détail À nettoyer appartient au complément local.

## Compilation et vérification

Qt 6.7.3 MinGW 64 bits, Python 3 pour la génération du formulaire, modules Qt Widgets, SQL, Network, SVG, PDF et PrintSupport. Compiler `gestion_service.pro` avec qmake puis mingw32-make. Les PDF texte sont lus par Qt PDF ; les scans nécessitent Poppler/Tesseract ou une saisie manuelle.

Contrôles : `python tools/check_workshop_architecture.py`, tests Qt dans `tests/service_tests.pro`, tests des cinq tables via `ANIWORLD_FIVE_TESTS=1`. Les essais Oracle utilisent des enregistrements dédiés et les nettoient ; les tests de contraintes provoquent volontairement des refus de doublon et de clé étrangère.

Sauvegarde des sources avant la vérification finale : `backups/before-final-check-20261107-134413/sources.zip`. Rapports de vérification : `artifacts/new-stock-integration/`. Aucun pull, reset ni push Git effectué par cette finalisation.
