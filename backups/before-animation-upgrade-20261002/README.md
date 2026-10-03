# ANIWORLD — Gestion des services

Application desktop **C++ / Qt Widgets 6.7.3**, version **2.1**, avec le nouveau logo chien/chat et le nom **ANIWORLD**. Cette version se concentre sur l'interface et le module de gestion des services. Les données locales de la version précédente restent compatibles.

## Ouvrir l'application

- Double-cliquer sur **Lancer ANIWORLD.cmd**, ou sur **ANIWORLD/ANIWORLD.exe**.
- Pour une présentation avec sept prestations fictives, ouvrir **Demo ANIWORLD.cmd**. Le bandeau « Démo · non enregistré » distingue ce mode : ses données restent en mémoire et disparaissent à la fermeture.
- Pour modifier le projet, ouvrir **gestion_service.pro** dans Qt Creator, choisir le kit **Desktop Qt 6.7.3 MinGW 64-bit**, exécuter qmake puis compiler.

Le dossier `ANIWORLD` contient l'exécutable et ses dépendances Qt. Conserver ce dossier complet lors d'une copie sur un autre PC Windows 64 bits.

## Fonctionnalités

- Tableau de bord : total, services planifiés, en cours et terminés, prestations du jour, répartition par catégorie.
- Services : création, consultation, modification et suppression avec confirmation.
- Deux vues : cartes avec sélection dorée, ou liste. Le panneau de détail garde les actions accessibles, même sur une fenêtre de 1100 × 700.
- Suivi rapide : **Démarrer le service**, puis **Marquer comme terminé**, avec sauvegarde immédiate.
- Duplication : reprend les informations dans un formulaire à valider, avec un nouvel identifiant, un statut planifié et une date proposée le lendemain. L'original reste inchangé.
- Formulaire : nom, catégorie, date et heure, durée, responsable, tarif en DT, statut et notes.
- Recherche par nom, ID, catégorie, notes ou responsable, sans distinction de casse ni d'accents ; filtres combinables par catégorie, statut et période ; tri par date, nom ou tarif.
- Périodes : toutes les dates, aujourd'hui, aujourd'hui et les six jours suivants, ou services planifiés dont la date est passée (« À replanifier »).
- Statistiques sur les résultats filtrés : volumes, statuts, catégories, heures restant à réaliser et montant des services terminés. Ce montant n'est pas un suivi d'encaissement.
- Menu **Exporter** : rapport PDF paginé avec le logo et les filtres appliqués, ou CSV UTF-8 des résultats affichés.
- États vides, validation dans le formulaire, conservation des filtres après modification et navigation au clavier.
- Raccourcis : **Ctrl+N** pour créer, **Ctrl+F** pour rechercher. Double-cliquer sur une prestation pour la consulter.

## Données

L'application ouvre directement **Services** : aucun formulaire MySQL ni mot de passe au lancement. Le tableau de bord reste accessible dans la navigation.

La version normale enregistre automatiquement les services dans une base SQLite locale :

`%LOCALAPPDATA%/ANIWORLD/ANIWORLD/services.sqlite`

Le chemin exact est aussi visible dans l'infobulle « Sauvegarde locale ». Les données sont conservées après fermeture et réouverture. Le mode normal commence vide ; les exemples du mode démo ne sont jamais ajoutés à cette base.

Le stockage local convient à cette phase d'interface. Les anciennes données MySQL ne sont ni importées ni modifiées. Le script MySQL d'origine et les anciennes sources des autres modules sont conservés ; ils ne sont pas utilisés par cette version.

## Périmètre du projet

La navigation reprend les modules des captures de référence : **Employés**, **Stock**, **Locaux**, et **Animaux & dossiers** (animaux et dossiers médicaux réunis). Ces modules sont désactivés : aucune intégration avec les projets des coéquipiers n'a été faite. Seuls le tableau de bord des services et la gestion des services sont fonctionnels.

La classe `Service`, les calculs `ServiceMetrics` et l'export `ServiceReport` sont séparés des widgets. La future intégration pourra remplacer les points d'accès de `DatabaseManager` et relier le responsable aux employés. Pour l'instant, le responsable reste un nom libre et aucun autre module n'est requis.

Les images originales sont intégrées dans les ressources Qt. Seules leurs marges transparentes sont retirées lors de l'affichage ; les fichiers fournis restent intacts.

## Organisation

- `src/mainwindow.*` : navigation, structure de la fenêtre et raccourcis.
- `src/widgets/dashboardwidget.*` : tableau de bord.
- `src/widgets/servicewidget.*` : liste, filtres, actions et export.
- `src/widgets/servicecard.*` : cartes sélectionnables et accessibles au clavier.
- `src/models/serviceanalytics.*` : calculs des indicateurs, indépendants de l'interface.
- `src/ui/servicereport.*` : rapport PDF natif Qt, pagination et en-têtes répétés.
- `src/dialogs/servicedialog.*` : formulaire et consultation.
- `src/database/databasemanager.*` : accès SQLite avec requêtes préparées.
- `resources/theme.qss`, `src/ui/design.*` : styles et composants visuels.
- `tests/service_tests.cpp` : tests Qt Test et captures des widgets.
- `artifacts/` : captures du résultat réel, dont les captures du mode démo.
- `backups/original-20260926-104908/` : sauvegarde complète des sources initiales, ressources et configuration du projet.
- `backups/before-service-upgrade-20261001-104905/` : état du projet avant le nouveau logo et l'amélioration du module Services.

## Vérification

Le projet et les tests se compilent avec le kit Qt installé. Dans un terminal configuré avec Qt et MinGW :

```powershell
mkdir build_tests
cd build_tests
qmake ../tests/service_tests.pro "CONFIG+=release"
mingw32-make -j4
$env:QT_QPA_PLATFORM = "offscreen"
./release/service_tests.exe
```

Les tests vérifient le cycle création/modification/suppression, la persistance après réouverture, les données invalides, les filtres et tris, les formulaires, les compteurs et l'échappement CSV. Pour générer les captures, définir `ANIWORLD_CAPTURE_DIR` avec un chemin de sortie avant le lancement des tests. Les avertissements `propagateSizeHints` proviennent du moteur de rendu hors écran utilisé dans les tests.
