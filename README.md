# ANIWORLD — Gestion Services uniquement

Copie indépendante extraite le 8 novembre 2026 du projet `C:/Users/malek/Documents/gestion_service`.

## Ouvrir

Dans Qt Creator, ouvrir `gestion_service.pro`, sélectionner le kit Qt 6.7.3 MinGW 64 bits, puis compiler/exécuter.
La copie contient la connexion, le login et le tableau de bord Services, le CRUD Services, les filtres/statistiques, le planning et la facture PDF.

La fenêtre propose uniquement Tableau de bord et Services. Aucun module de gestion Stock, Employés, Médical ou Locaux n'est inclus.

## Base de données

La copie utilise la même base Oracle et le même DSN que le projet intégré. Les paramètres sont dans `src/database/connection.h`. Les comptes application Services et Admin sont acceptés par cette copie. Aucune création/migration de table n'est exécutée.

Les modifications de Services portent donc sur la base commune existante. Les propriétaires déjà enregistrés sont lus dans MEDICAL pour le champ Propriétaire ; les employés sont lus pour le planning. Ces lectures indispensables restent dans des classes de modèle, sans copier les CRUD ou les interfaces des autres gestions. Une suppression reste soumise aux clés étrangères Oracle.

## Fichiers principaux

- `src/models/service.h/.cpp` : classe Service, attributs privés, getters/setters, CRUD.
- `src/widgets/servicewidget.h/.cpp` : affichage, boutons, signaux/slots.
- `src/dialogs/servicedialog.h/.cpp` : formulaire Ajouter/Modifier.
- `src/dialogs/serviceplanningdialog.h/.cpp` : planning des services.
- `src/models/invoicemodel.h/.cpp` et `src/ui/invoice.h/.cpp` : facture PDF.
- `src/models/servicereferences.h/.cpp` : lecture des propriétaires pour Services.
- `src/mainwindow.ui` : formulaire Designer Services uniquement.
- `src/mainwindow.h/.cpp` : fenêtre et navigation Services.
- `src/database/connection.h/.cpp` : connexion Oracle.

`build/` et `build_tests/` sont des dossiers générés ; les sources ne dépendent d'aucun fichier du projet original. Les tests de lecture, navigation, formulaire/annulation et export PDF sont dans `tests/service_only_tests.pro`.

La référence service se saisit sous la forme SRV- suivie de trois chiffres (ex. SRV-123), indépendamment du propriétaire. Elle est enregistrée dans SERVICE.REF_SERVICE ; le propriétaire dans SERVICE.PROPRIETAIRE. Les champs affichent une validation visuelle.

## Mise à jour du 20 novembre 2026

Courbes dynamiques (période, catégorie, mesure, actualisation automatique), planning hebdomadaire et liste détaillée. Les services non affectés restent visibles. Les tâches complémentaires des employés sont lues dans le même stockage local ANIWORLD que le projet intégré. Sur ce PC, les dix tâches Démo du 9 au 15 novembre sont donc visibles dans les deux versions. Aucun CRUD ni interface des quatre autres gestions n’est ajouté.
