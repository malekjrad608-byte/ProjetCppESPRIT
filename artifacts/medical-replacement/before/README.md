# ANIWORLD — Services, Employés, Stock, Locaux, Dossiers médicaux

Application desktop **C++ / Qt Widgets 6.7.3**, version **3.0.0**, avec le logo chien/chat et le nom **ANIWORLD**. Les services, factures, employés, affectations, dossiers médicaux et comptes utilisent une base SQLite locale ; les locaux utilisent un fichier JSON local et le Stock fourni conserve ses données en mémoire pendant la session. Les données locales de la version précédente restent compatibles.

## Ouvrir l'application

- Double-cliquer sur **Lancer ANIWORLD.cmd**, ou sur **ANIWORLD/ANIWORLD.exe**.
- La page de connexion s'ouvre en premier. Chaque compte ouvre directement son module ; **Se déconnecter** permet de changer de compte sans effacer les données.
- Le compte **admin** accède à tous les modules. Les comptes **services**, **stock**, **employes** et **locaux** n'affichent que leur espace autorisé (avec le tableau de bord pour Services).
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
| Locaux | `locaux` | `locaux123` | Locaux |
| Dossiers médicaux | `medical` | `medical123` | Dossiers médicaux |
| Administration | `admin` | `admin123` | Services, navigation complète |

Les comptes initiaux sont créés une seule fois. Les données appartiennent au centre, avec une séparation d'accès par module dans l'interface ; cette application locale n'est pas une isolation des fichiers Windows entre utilisateurs.

## Employés intégrés

Source : `C:/Users/malek/Downloads/Employessanslogin/Employes/ANIWORLD2`, laissée intacte. Le module est adapté dans `src/employees/`. Les 6 employés et le planning présents dans sa base sont intégrés une seule fois au premier lancement grâce à une copie SQLite cohérente embarquée. Les modifications et suppressions ultérieures sont conservées ; l'import n'est pas rejoué sur un annuaire déjà importé. Sauvegarde préalable de la base Services et des sources : `backups/before-employees-accounts-20261011/`.

Fonctions reprises : ajout, modification, suppression, disponibilité, recherche, filtres, tri par embauche ou charge, statistiques hebdomadaires, notifications de surcharge, planification avec affectation automatique selon compétences/capacité/créneau et détection des conflits. L'attestation de congé utilise le même export PDF atomique que les autres documents. Les employés et leurs affectations sont enregistrés dans `services.sqlite`, dans des tables séparées des consultations.

L'annuaire, Services et Stock partagent la barre latérale, le fond illustré, les icônes vectorielles, les boutons turquoise/vert/rouge, les cartes sélectionnées en orange et les animations désactivables. Les actions du panneau Employé restent accessibles en bas pendant le défilement des détails. Les anciennes sources `src/rh/` restent hors compilation.


## Stock remplacé et Locaux intégrés — 12 octobre 2026

Sources originales conservées intactes :

- Stock : `C:/Users/malek/Downloads/ANIWORLD_Stock_finale/ANIWORLD_Stock_Modifie`.
- Locaux : `C:/Users/malek/Downloads/locauxF/ANIWORLD_Locaux`.

Le nouveau Stock remplace le précédent dans `src/stock/`. Son `main.cpp`, sa fenêtre de connexion et sa barre latérale séparée ne sont pas compilés : il utilise la connexion, les animations et la navigation communes. Les 19 articles fournis, l'ajout d'article depuis le mouvement, la modification, la suppression, les entrées/sorties, les statistiques et le remplacement/suppression des lots périmés sont repris. Les filtres catégorie/état/recherche se combinent ; le tri porte sur la quantité ou la date d'échéance dans les deux sens. L'export PDF conserve les résultats et l'ordre affichés. L'ancien FEFO et la proposition de réapprovisionnement sont remplacés par la gestion des échéances du nouveau module.

**Le Stock fourni est en mémoire uniquement**, comme son projet d'origine. Les changements subsistent pendant la navigation et les changements de compte mais disparaissent à la fermeture. Le bandeau et le pied de page l'indiquent. Les quantités de sortie sont contrôlées, avec un plafond de 999 999 unités par lot.

Locaux est intégré dans `src/locaux/` : cartes/liste, création/modification/suppression, occupation, nettoyage, recherche intelligente, alertes, affectation assistée, plan interactif, statistiques et rapport de nettoyage PDF. Les boutons et fonds utilisent le design commun turquoise, blanc et orange. Le plan se ferme lorsqu'on quitte la page, notamment au changement de compte.

En mode normal, les locaux sont enregistrés dans `%LOCALAPPDATA%/ANIWORLD/ANIWORLD/locaux_v2.json`. Au premier accès, une copie des locaux existants du projet fourni est importée depuis `resources/data/locaux-initial.json`. Les lancements suivants conservent les modifications et suppressions, même si tous les locaux ont été supprimés. L'écriture JSON est atomique ; un fichier illisible n'est jamais remplacé par des exemples. Le mode `--demo` garde les locaux en mémoire.

Une seule fenêtre et un seul `main.cpp` : 0 = Tableau de bord, 1 = Services, 2 = Stock, 3 = Employés, 4 = Locaux. Les comptes existants et mots de passe modifiés sont conservés lors de la migration ; seul le compte manquant `locaux` est ajouté. Les raccourcis Ctrl+N / Ctrl+F sont routés vers le module autorisé.

Sauvegarde avant intégration : `artifacts/integration-locaux-stock/backup/` (sources, ressources, tests, base SQLite et JSON d'origine).

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

Après connexion, chaque compte ouvre directement son module. Le tableau de bord est accessible aux comptes Services et Administration.

La version normale enregistre automatiquement les services dans une base SQLite locale :

`%LOCALAPPDATA%/ANIWORLD/ANIWORLD/services.sqlite`

Le chemin exact est aussi visible dans l'infobulle « Sauvegarde locale ». Les données sont conservées après fermeture et réouverture. Le mode normal commence vide ; les exemples du mode démo ne sont jamais ajoutés à cette base.

Le stockage local convient à cette phase d'interface. Les anciennes données MySQL ne sont ni importées ni modifiées. Le script MySQL d'origine et les anciennes sources des autres modules sont conservés ; ils ne sont pas utilisés par cette version.

## Périmètre du projet

La navigation active contient **Tableau de bord**, **Services**, **Stock**, **Employés** et **Locaux**. Seul le futur module **Animaux & dossiers** reste désactivé. Chaque compte voit ses modules autorisés.

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

Les tests Stock vérifient les filtres combinés, les tris numériques et chronologiques, le PDF filtré, les mouvements valides et invalides, les modifications après tri, la suppression, les échéances, les statistiques et la navigation intégrée. Les tests Locaux couvrent la persistance, les échecs d’écriture, les fichiers invalides, le nettoyage, la recherche, l’affectation assistée, le PDF, l’ajout et le plan. Captures actuelles : `artifacts/integration-locaux-stock/`.

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

### Validation du remplacement Stock et de l'intégration Locaux

34 tests réussis, aucun échec : `artifacts/integration-locaux-stock/full-results.txt`. Les contrôles couvrent aussi la migration de l'ancienne table de comptes sans réinitialisation du mot de passe Stock, les cinq rôles, les changements de compte et la fermeture du plan. Vérification complémentaire à 125 % : 6 contrôles réussis (`dpi125-results.txt`). Les PDF de nettoyage ont été rendus et inspectés sur leurs deux pages. Les compilations Release et Qt Creator Debug sont à jour ; `ANIWORLD/ANIWORLD.exe` et les dépendances Qt SVG ont été déployés et le lancement Windows du dossier livré a été vérifié en mode démonstration.


## Dossiers médicaux intégrés — 13 octobre 2026

Source : `C:/Users/malek/Downloads/med/med/dossier medical R`, conservée intacte. La version du module décrite par son README et son CMake est adaptée dans `src/medical/`, sans son deuxième login ni sa fenêtre de navigation autonome. Les anciens fichiers Services présents dans le même dossier ne remplacent pas les Services de l'application.

Le compte **medical / medical123** ouvre directement Dossiers médicaux. L'administrateur a également accès au module. Les comptes déjà existants et leurs mots de passe modifiés sont conservés pendant la migration.

Fonctions : fiches animaux et propriétaires, consultations (motif, diagnostic, traitement), vaccinations, allergies, poids, observations, ajout/modification/suppression, statut vaccinal, recherche, filtres par espèce/statut, tri par date/nom/propriétaire et statistiques. Les dossiers sont enregistrés dans la table `medical_records` de la base SQLite commune. Leur numéro automatique ne change pas après modification et n'est pas réutilisé après suppression. L'historique affiche uniquement les consultations enregistrées avec le même identifiant animal ; aucune visite antérieure n'est inventée.

Le mode normal démarre avec un registre vide. Les huit exemples du projet fourni sont chargés uniquement en mode `--demo`, sans sauvegarde à la fermeture. Le fond, les cartes, les boutons et le survol orange reprennent le style commun. Les formulaires et les détails défilent pour rester accessibles sur les petites fenêtres.

**Exporter PDF** affiche le contenu des dossiers filtrés, dans l'ordre du tri choisi, avant enregistrement. **PDF** dans le panneau sélectionné exporte uniquement ce dossier. Aucun export CSV. Les notes et les caractères spéciaux sont échappés dans les documents ; le PDF porte la mention démonstration en mode démo.

Sauvegarde préalable des sources et de la base locale : `artifacts/medical-integration/before/`. Captures et contrôles de cette intégration : `artifacts/medical-integration/`.


Validation de l'intégration médicale : **37 tests réussis, aucun échec** (`artifacts/medical-integration/full-results.txt`). Contrôles Windows à 125 % : **4 contrôles réussis**, dont le CRUD médical et la non-régression des couleurs Stock (`windows125-results.txt`). Le PDF a été rendu et inspecté. Release, Qt Creator Debug et `ANIWORLD/ANIWORLD.exe` sont à jour ; le lancement du dossier livré et la migration réelle ont été vérifiés. Les cinq comptes existants (empreintes et sels compris) ainsi que les anciennes tables ont été comparés à la sauvegarde et sont inchangés ; seul le compte médical et sa table vide ont été ajoutés (`live-migration-check.json`).
