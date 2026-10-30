# ANIWORLD — Services, Employés, Stock et comptes par module

Application desktop **C++ / Qt Widgets 6.7.3**, version **3.0.0**, avec le logo chien/chat et le nom **ANIWORLD**. Les services, factures, employés, affectations et comptes utilisent une base SQLite locale ; le module Stock fourni reste une interface de démonstration. Les données locales de la version précédente restent compatibles.

## Ouvrir l'application

- Double-cliquer sur **Lancer ANIWORLD.cmd**, ou sur **ANIWORLD/ANIWORLD.exe**.
- La page de connexion s'ouvre en premier. Chaque compte ouvre directement son module ; **Se déconnecter** permet de changer de compte sans effacer les données.
- Le compte **admin** accède à tous les modules. Les comptes **services**, **stock** et **employes** n'affichent que leur espace autorisé (avec le tableau de bord pour Services).
- Pour une présentation avec sept prestations fictives, ouvrir **Demo ANIWORLD.cmd**. Le bandeau « Démo · non enregistré » distingue ce mode : ses données restent en mémoire et disparaissent à la fermeture.
- Pour modifier le projet, ouvrir **gestion_service.pro** dans Qt Creator, choisir le kit **Desktop Qt 6.7.3 MinGW 64-bit**, exécuter qmake puis compiler.

Le dossier `ANIWORLD` contient l'exécutable et ses dépendances Qt. Conserver ce dossier complet lors d'une copie sur un autre PC Windows 64 bits.

## Connexion intégrée

L'interface de `C:/Users/malek/Documents/petsssss` est intégrée dans `src/login/` ; le projet fourni reste intact. Son panneau animé, son logo, le bouton de visibilité du mot de passe et la page de récupération sont conservés. Le réglage des animations est commun à la connexion et aux services.

**Remember username** conserve uniquement l'identifiant. Le mot de passe saisi est effacé à la connexion et à la déconnexion. Une seule fenêtre de gestion est créée, même après plusieurs connexions.

Le bouton **Se déconnecter**, avec une icône de sortie et un survol animé, se trouve en haut à droite des Services, du Stock et du tableau de bord. La déconnexion est immédiate ; une transition de 620 ms affiche « À bientôt ! » avec un cercle doré et un fondu vers le formulaire. Le bouton **Animations** désactive aussi cet effet. Une reconnexion reste possible pendant la transition, et les services enregistrés sont conservés.

Les comptes sont locaux à l'application. Les mots de passe sont conservés sous forme de dérivations PBKDF2-HMAC-SHA256 avec sel aléatoire individuel, jamais en clair dans la base. Les contrôles de navigation et les raccourcis respectent le module du compte connecté. **Mot de passe** permet de changer son mot de passe ; ce changement persiste après fermeture. La simulation de récupération par email/SMS est réservée au mode `--demo`, en mémoire. En mode normal, aucune récupération fictive ne peut créer un compte.

| Espace | Identifiant initial | Mot de passe initial | Ouverture |
|---|---|---|---|
| Services | `services` | `services123` | Services |
| Stock | `stock` | `stock123` | Stock |
| Employés | `employes` | `employes123` | Employés |
| Administration | `admin` | `admin123` | Services, navigation complète |

Les comptes initiaux sont créés une seule fois. Les données appartiennent au centre, avec une séparation d'accès par module dans l'interface ; cette application locale n'est pas une isolation des fichiers Windows entre utilisateurs.

## Employés intégrés

Source : `C:/Users/malek/Downloads/Employessanslogin/Employes/ANIWORLD2`, laissée intacte. Le module est adapté dans `src/employees/`. Les 6 employés et le planning présents dans sa base sont intégrés une seule fois au premier lancement grâce à une copie SQLite cohérente embarquée. Les modifications et suppressions ultérieures sont conservées ; l'import n'est pas rejoué sur un annuaire déjà importé. Sauvegarde préalable de la base Services et des sources : `backups/before-employees-accounts-20261011/`.

Fonctions reprises : ajout, modification, suppression, disponibilité, recherche, filtres, tri par embauche ou charge, statistiques hebdomadaires, notifications de surcharge, planification avec affectation automatique selon compétences/capacité/créneau et détection des conflits. L'attestation de congé utilise le même export PDF atomique que les autres documents. Les employés et leurs affectations sont enregistrés dans `services.sqlite`, dans des tables séparées des consultations.

L'annuaire, Services et Stock partagent la barre latérale, le fond illustré, les icônes vectorielles, les boutons turquoise/vert/rouge, les cartes sélectionnées en orange et les animations désactivables. Les actions du panneau Employé restent accessibles en bas pendant le défilement des détails. Les anciennes sources `src/rh/` restent hors compilation.


## Stock intégré et guide GUI

Source : `C:/Users/malek/Downloads/stockaniworld/stockaniworld`. Les fichiers d'origine ne sont pas modifiés. `StockWidget` et les 19 articles fictifs fournis sont repris dans `src/stock/`, puis adaptés à la fenêtre ANIWORLD, à ses dimensions et à ses animations.

Le guide `C:/Users/malek/Downloads/Guide Integration GUI (2).pdf` a été lu intégralement. L'intégration reprend son organisation Vue/Contrôleur : **une fenêtre principale, une page par module dans un QStackedWidget, des noms d'objets explicites et une navigation commune**. Les index restent stables : 0 = Tableau de bord, 1 = Services, 2 = Stock, 3 = Employés. Les boutons de navigation s'appellent `navDashboard`, `navServices`, `navStock`, `navEmployees`.

Les deux interfaces fournies sont construites en C++ : cette version conserve des QWidget modulaires et des connexions `connect()` explicites, au lieu de reconstruire l'ensemble dans un fichier Designer `.ui`. Le guide présente aussi les connexions explicites et admet des variantes techniques ; la variante C++ est à présenter au tuteur si une livraison exclusivement `.ui` est exigée. Un seul `main.cpp` et un seul projet qmake sont utilisés ; les anciens points d'entrée et fenêtres vides du module Stock ne sont pas compilés.

Le projet Stock d'origine ne contenait ni persistance ni traitement des boutons (seulement des traces `qDebug`). La version intégrée reste clairement en **démonstration**, même lorsque Services utilise sa sauvegarde locale :

- Inventaire de 19 articles avec tri, recherche insensible aux accents, filtre par catégorie et export PDF des résultats affichés.
- Indicateurs recalculés à partir des quantités et des dates : articles, articles sous le seuil, lots dont l'échéance est dépassée ou arrive dans les 30 prochains jours.
- Fenêtre de simulation des entrées/sorties avec validation, sans quantité négative. Les changements restent en mémoire pendant la session, y compris pendant la navigation ou une déconnexion, puis disparaissent à la fermeture.
- Carte d'échéance prioritaire : propose le lot disponible non périmé dont la date est la plus proche, puis prépare une sortie à valider. Il s'agit d'une démonstration de la priorité par date, pas d'une substitution entre articles.
- Proposition de réapprovisionnement sur les articles sous le seuil, avec l'hypothèse de consommation du prototype explicitement affichée. Aucune commande n'est envoyée.
- **Ctrl+F** recherche dans le module actif ; **Ctrl+N** ouvre un mouvement depuis Stock, un employé depuis Employés ou un service depuis Services/Tableau de bord.

Stock n'écrit pas dans la base des services. La persistance et les opérations CRUD du catalogue de stock ne figuraient pas dans le projet fourni et restent une étape distincte de cette intégration GUI.

## Facturation automatique et nouvelle interface

À chaque création d'un service, SQLite attribue automatiquement un numéro unique `FAC-AAAA-000001` et une date d'émission. Le numéro reste identique après modification et réouverture. Le formulaire demande le **nom du client** ; le détail des prestations reprend le nom, la description, la catégorie, le responsable, la date, la durée et le tarif du service.

Sélectionner un service puis **Facture PDF** pour consulter la facture et l'exporter. Le total à payer correspond au tarif saisi en DT. Le contenu reflète les dernières informations enregistrées du service ; un PDF déjà exporté reste un fichier indépendant. Il ne s'agit pas d'un suivi d'encaissement. Les services annulés ne sont pas facturables.

Les bases précédentes sont migrées sans suppression de services : le client reste vide pour les anciennes consultations et doit être complété avec **Modifier** avant l'export de leur facture. Aucun nom de client n'est inventé.

L'application propose uniquement des exports **PDF** : factures individuelles, rapport filtré des services et inventaire filtré du stock. La duplication de service reste supprimée. L'ancien module RH reste hors compilation ; il est remplacé par le module Employés demandé le 11 octobre 2026.

La présentation reprend les références fournies : navigation blanche compacte, fond bleu très clair, cartes avec sélection dorée, boutons turquoise et statistiques en fenêtre avec indicateurs pastel et tableau. Le Stock utilise les mêmes composants et couleurs.

## Fonctionnalités Services

- Logo : au survol, zoom doux et GIF circulaire turquoise et doré avec une petite patte. Le logo original reste intact. L'animation fonctionne aussi avec le focus clavier ; cliquer sur le logo ouvre le tableau de bord. Elle s'arrête quand le logo perd le survol/focus ou quand la fenêtre est masquée ou désactivée.
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
- Formulaire : nom, catégorie, date et heure, durée, responsable, client obligatoire, tarif en DT, statut et notes.
- Recherche par nom, ID, client, numéro de facture, catégorie, notes ou responsable, sans distinction de casse ni d'accents ; filtres combinables par catégorie, statut et période ; tri par date, nom, tarif et durée (croissant/décroissant).
- Périodes : toutes les dates, aujourd'hui, aujourd'hui et les six jours suivants, ou services planifiés dont la date est passée (« À replanifier »).
- Statistiques sur les résultats filtrés : volumes, statuts, catégories, heures restant à réaliser et montant des services terminés. Ce montant n'est pas un suivi d'encaissement.
- **Rapport PDF** : aperçu du rapport paginé avec le logo, les filtres et le tri appliqués. Les boutons Précédent/Suivant permettent de vérifier les pages avant d'enregistrer exactement le PDF affiché. **Facture PDF** : facture individuelle avec client et total à payer.
- États vides, validation dans le formulaire, conservation des filtres après modification et navigation au clavier.
- Raccourcis : **Ctrl+N** pour créer, **Ctrl+F** pour rechercher. Double-cliquer sur une prestation pour la consulter.

## Données

Après connexion, l'application ouvre **Services**, sans formulaire MySQL. Le tableau de bord reste accessible dans la navigation.

La version normale enregistre automatiquement les services dans une base SQLite locale :

`%LOCALAPPDATA%/ANIWORLD/ANIWORLD/services.sqlite`

Le chemin exact est aussi visible dans l'infobulle « Sauvegarde locale ». Les données sont conservées après fermeture et réouverture. Le mode normal commence vide ; les exemples du mode démo ne sont jamais ajoutés à cette base.

Le stockage local convient à cette phase d'interface. Les anciennes données MySQL ne sont ni importées ni modifiées. Le script MySQL d'origine et les anciennes sources des autres modules sont conservés ; ils ne sont pas utilisés par cette version.

## Périmètre du projet

La navigation active contient **Tableau de bord**, **Services**, **Stock**. Les futurs modules **Locaux** et **Animaux & dossiers** (animaux et dossiers médicaux réunis) restent désactivés. Stock est intégré dans le périmètre GUI décrit ci-dessus.

La classe `Service`, les calculs `ServiceMetrics` et l'export `ServiceReport` sont séparés des widgets. La future intégration pourra remplacer les points d'accès de `DatabaseManager` et relier le responsable aux employés. Pour l'instant, le responsable reste un nom libre et aucun autre module n'est requis.

Les images originales sont intégrées dans les ressources Qt. Seules leurs marges transparentes sont retirées lors de l'affichage ; les fichiers fournis restent intacts.

## Organisation

- `src/mainwindow.*` : navigation, structure de la fenêtre et raccourcis.
- `src/ui/invoice.*` : facture, échappement du contenu et rendu PDF paginé commun avec le stock.
- `src/stock/stockwidget.*` : interface Stock importée, filtres, simulations et export PDF de démonstration.
- `src/appcontroller.*` : ouverture de la connexion, accès aux services et déconnexion.
- `src/login/loginwindow.*`, `src/login/loginvisuals.*` : interface animée importée et authentification de prototype.
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
- `backups/before-login-integration-20261003/` : état avant l'intégration de la connexion.
- `backups/before-logout-animation-20261003/` : état avant le nouveau bouton et l'animation de déconnexion.
- `backups/before-stock-integration-20261003/` : état avant l'intégration du module Stock.

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

Les tests vérifient le cycle création/modification/suppression, la persistance après réouverture, les données invalides, les filtres et tris, les formulaires, les compteurs, la migration des factures, leur numérotation et leur persistance et les rapports PDF. Ils contrôlent aussi la lecture et l'arrêt du GIF, le focus clavier, les transitions rapides entre pages et la stabilité de la disposition pendant les animations. Pour générer les captures, définir `ANIWORLD_CAPTURE_DIR` avec un chemin de sortie avant le lancement des tests. Les avertissements `propagateSizeHints` proviennent du moteur de rendu hors écran utilisé dans les tests.

Les tests de connexion couvrent les saisies invalides, l'ouverture de Services après validation, l'ajout d'une prestation après connexion, la conservation des données à la déconnexion, la reconnexion sans duplication de fenêtre, la mémorisation de l'identifiant seul et la récupération simulée. Les captures `artifacts/login*.png` montrent l'interface intégrée.

Les tests Stock vérifient les filtres combinés, le tri numérique, le PDF filtré, les mouvements valides et invalides, l'annulation, la priorité par date, la proposition de réapprovisionnement, la navigation intégrée, la déconnexion depuis Stock et l'absence de superposition avec Services/Tableau de bord. Captures : `artifacts/stock-integrated.png`, `artifacts/stock-small.png`, `artifacts/stock-movement.png` et `artifacts/stock-reorder.png`.

Validation de la version 3.0 : 26 tests réussis, aucun échec ; vérifications supplémentaires des écrans à 125 % réussies. Captures et PDF de contrôle : `artifacts/services-upgrade/`. Journaux : `build_tests/upgrade-results.txt` et `build_tests/upgrade-dpi125.txt`.

### Style de référence et survol orange

Le formulaire Service utilise une colonne de libellés et une colonne de champs. Le panneau sélectionné présente les actions pleine largeur : Modifier (turquoise), démarrer/terminer (vert), Supprimer (rouge), avec les documents au-dessus. Le Stock reprend les mêmes boutons et couleurs.

Les boutons, cartes, panneaux et champs affichent un halo orange progressif au survol (160–180 ms), avec retour doux à la sortie. Le focus clavier des boutons et champs bénéficie aussi de ce repère. Les effets ne changent ni la disposition ni les zones cliquables ; le bouton Animations les désactive. Les lignes de tableau, menus et barres de défilement utilisent également une couleur de survol orange.

Vérification : suite de régression réussie, puis tests ciblés du survol, de la saisie, des formulaires, du CRUD et du Stock réussis (`build_tests/orange-results.txt`, `build_tests/orange-final.txt`). Captures : `artifacts/reference-orange/`, dont `orange-hover.png`.

### Fond illustré et aperçu du rapport

Services, Stock et Tableau de bord partagent un fond vectoriel avec dégradés turquoise, courbes fines, touches dorées et empreintes discrètes. La barre latérale réserve ces motifs à son espace libre pour garder la navigation lisible.

Le bouton Rapport PDF ouvre un aperçu du véritable document, paginé, avant son enregistrement. Il conserve les services filtrés dans leur ordre affiché ; les factures individuelles restent accessibles depuis le service sélectionné. Le module Qt PDF est requis pour cet aperçu.

Validation : 28 tests réussis, dont le rapport filtré sur plusieurs pages, le tri par tarif, la navigation de l'aperçu et la concordance du fichier enregistré (`build_tests/background-results.txt`). Captures : `artifacts/background-report/`.

### Validation de l'intégration Employés et comptes

32 tests réussis, aucun échec (`build_tests/employees-full.txt`). Les nouveaux tests couvrent les quatre comptes, le rejet des mots de passe incorrects, les restrictions de navigation, les changements de compte, le changement et la persistance du mot de passe, l'import unique sans réapparition des employés supprimés, le CRUD Employés, les affectations et conflits, la suppression des tâches liées et l'attestation PDF. Vérification complémentaire à 125 % : `build_tests/employees-dpi125.txt`. Captures : `artifacts/employees-accounts/` et son sous-dossier `dpi125/`.
