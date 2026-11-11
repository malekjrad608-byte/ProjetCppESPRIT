# ANIWORLD — Gestion Services uniquement

Copie indépendante extraite le 8 novembre 2026 du projet `C:/Users/malek/Documents/gestion_service`.

## Ouvrir

Dans Qt Creator, ouvrir `gestion_service.pro`, sélectionner le kit Qt 6.7.3 MinGW 64 bits, puis compiler/exécuter.
La copie contient la connexion, le login et le tableau de bord Services, le CRUD Services, les filtres/statistiques, le planning et la facture PDF.

La fenêtre propose uniquement Tableau de bord et Services. Aucun module de gestion Stock, Employés, Médical ou Locaux n'est inclus.

## Base de données

La copie utilise la même base Oracle et le même DSN que le projet intégré. Les paramètres sont dans `src/database/connection.h`. Les comptes application Services et Admin sont acceptés par cette copie. Aucune création/migration de table n'est exécutée.

Les modifications de Services portent donc sur la base commune existante. Les propriétaires déjà enregistrés sont lus dans MEDICAL pour Réf. service ; les employés sont lus pour le planning. Ces lectures indispensables restent dans des classes de modèle, sans copier les CRUD ou les interfaces des autres gestions. Une suppression reste soumise aux clés étrangères Oracle.

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
