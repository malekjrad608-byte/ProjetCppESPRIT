# ANIWORLD — Gestion des services

Application desktop **C++ / Qt Widgets 6.7.3**, version **2.3.1**, avec le logo chien/chat et le nom **ANIWORLD**. Cette version se concentre sur l'interface et le module de gestion des services. Les données locales de la version précédente restent compatibles.

## Ouvrir l'application

- Double-cliquer sur **Lancer ANIWORLD.cmd**, ou sur **ANIWORLD/ANIWORLD.exe**.
- Pour une présentation avec sept prestations fictives, ouvrir **Demo ANIWORLD.cmd**. Le bandeau « Démo · non enregistré » distingue ce mode : ses données restent en mémoire et disparaissent à la fermeture.
- Pour modifier le projet, ouvrir **gestion_service.pro** dans Qt Creator, choisir le kit **Desktop Qt 6.7.3 MinGW 64-bit**, exécuter qmake puis compiler.

Le dossier `ANIWORLD` contient l'exécutable et ses dépendances Qt. Conserver ce dossier complet lors d'une copie sur un autre PC Windows 64 bits.

## Fonctionnalités

- Logo agrandi : au survol, zoom doux et GIF circulaire turquoise et doré avec une petite patte. Le logo original reste intact. L'animation fonctionne aussi avec le focus clavier ; cliquer sur le logo ouvre le tableau de bord. Elle s'arrête quand le logo perd le survol/focus ou quand la fenêtre est masquée ou désactivée.
- Transitions de 200 ms entre les pages et mise en évidence progressive des cartes au survol, sans déplacement des champs ni des boutons.
- Correction de la superposition Services/Tableau de bord : chaque image de navigation est peinte sur un fond opaque. La transition révèle uniquement la page courante, sans transparence du conteneur ni copie de l'ancienne page.
- Panneaux avec liseré turquoise et doré progressif au survol ; icônes des catégories colorées avec un léger agrandissement au survol des services.
- Boutons avec lumière au survol et onde au clic, indicateur de navigation glissant et apparition progressive des cartes. La sélection d'une prestation, les changements de vue, les formulaires, les statistiques et les messages de confirmation ont une courte transition.
- Tableau de bord : compteurs et barres animés, illustration avec mouvement léger à l'arrivée et au survol, dégradés turquoise et dorés. Les valeurs finales proviennent toujours des services enregistrés.
- Bouton **Animations** dans le bandeau supérieur : désactivation immédiate de tous les effets, y compris le GIF. La préférence est conservée entre les lancements. Les animations sont décoratives et ne retardent aucune action CRUD.
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
- `src/widgets/animatedlogo.*` : logo animé, lecture du GIF avec `QMovie`, zoom et focus clavier.
- `src/widgets/animatedbutton.*` : survol progressif et onde au clic, en conservant le comportement natif de `QPushButton`.
- `src/widgets/animatedpanel.*` : mise en évidence des panneaux par dessin, sans effet d'opacité sur leur contenu.
- `src/ui/motion.*` : préférence commune, apparitions des panneaux, compteurs et barres de progression.
- `resources/brand/logo-hover.gif` : halo animé intégré aux ressources Qt ; aucun fichier à télécharger au lancement.
- `tools/generate_logo_motion.py` : génération du halo à partir de formes géométriques, sans modification du logo. Python/Pillow ne sont nécessaires que pour régénérer ce GIF, pas pour utiliser ou compiler l'application.
- `src/models/serviceanalytics.*` : calculs des indicateurs, indépendants de l'interface.
- `src/ui/servicereport.*` : rapport PDF natif Qt, pagination et en-têtes répétés.
- `src/dialogs/servicedialog.*` : formulaire et consultation.
- `src/database/databasemanager.*` : accès SQLite avec requêtes préparées.
- `resources/theme.qss`, `src/ui/design.*` : styles et composants visuels.
- `tests/service_tests.cpp` : tests Qt Test et captures des widgets.
- `artifacts/` : captures du résultat réel, dont les captures du mode démo.
- `backups/original-20260926-104908/` : sauvegarde complète des sources initiales, ressources et configuration du projet.
- `backups/before-service-upgrade-20261001-104905/` : état du projet avant le nouveau logo et l'amélioration du module Services.
- `backups/before-animation-upgrade-20261002/` : état du projet avant l'agrandissement du logo et les animations.
- `backups/before-full-motion-20261002/` : état du projet avant l'extension des animations à toute l'interface.
- `backups/before-dashboard-transition-fix-20261003/` : état avant la correction de superposition des pages.

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

Les tests vérifient le cycle création/modification/suppression, la persistance après réouverture, les données invalides, les filtres et tris, les formulaires, les compteurs, l'échappement CSV et les rapports PDF. Ils contrôlent aussi la lecture et l'arrêt du GIF, le focus clavier, les transitions rapides entre pages et la stabilité de la disposition pendant les animations. Pour générer les captures, définir `ANIWORLD_CAPTURE_DIR` avec un chemin de sortie avant le lancement des tests. Les avertissements `propagateSizeHints` proviennent du moteur de rendu hors écran utilisé dans les tests.
