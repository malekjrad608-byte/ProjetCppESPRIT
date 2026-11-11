# Intégration du module Services

Le projet à ouvrir est `ANIWORLD2.pro`, avec le kit **Qt 6.7.3 MinGW 64 bits**.
Dans l'application, cliquer sur **Services** dans le menu principal.

## Organisation selon le guide

- `mainwindow.ui` est le seul formulaire compilé. La page Services contient maintenant les widgets Designer du module, à côté des pages Employés, Stock, Locaux et Dossiers médicaux.
- `mainwindow.h/.cpp` gardent la navigation centrale (`afficherServices`) et une zone `MODULE SERVICES` pour l'initialisation du contrôleur.
- `services/models/service.h/.cpp` contiennent la classe métier Service, ses attributs privés, getters/setters, validation et méthodes `ajouter`, `afficher`, `modifier`, `supprimer`.
- `services/widgets/servicewidget.h/.cpp` relient les widgets déjà créés par le formulaire commun à la classe métier. Ils ne créent pas une deuxième fenêtre principale ou un deuxième formulaire Designer.
- `connection.h/.cpp` à la racine restent l'unique propriétaire de la connexion Oracle. `services/database/oracleconnection.*` est seulement un accesseur à cette connexion : aucun nouvel utilisateur, mot de passe ou DSN, aucune ouverture/fermeture indépendante.
- `services/services.pri`, inclus par `ANIWORLD2.pro`, déclare les sources et ressources du module.
- Les noms `ServiceMotion` et `ServiceAnimatedButton`, ainsi que leurs fichiers, évitent les collisions avec les modules existants. Le thème Services s'applique au module et à ses dialogues, sans remplacer le thème global.
- `.gitignore` exclut compilations, fichiers Qt Creator, sauvegardes et résultats de vérification.

Les widgets des quatre autres pages du formulaire ont été comparés à la version avant intégration et conservés. Les sources du projet d'origine sur le Bureau restent inchangées.

## Fonctions intégrées

Ajout, consultation, modification, suppression, recherche, filtres, tri, vue cartes/tableau, changement de statut, planning, statistiques et facture PDF. Les propriétaires proviennent de MEDICAL et les employés du schéma commun. La liste est relue au retour sur Services et après reconnexion via le badge Oracle.

Le module utilise les tables SERVICE, MEDICAL, EMPLOYER et les vues de compatibilité existantes (notamment SERVICES et EMPLOYES), ainsi que SERVICES_SEQ pour les nouveaux identifiants. Aucune migration de schéma n'est exécutée. Les contraintes et clés étrangères Oracle continuent de s'appliquer.

## Vérifications reproductibles

Depuis la racine dans PowerShell :

```powershell
$env:PATH = 'C:\Qt\Tools\mingw1120_64\bin;C:\Qt\6.7.3\mingw_64\bin;' + $env:PATH
qmake ANIWORLD2.pro -o build\Desktop_Qt_6_7_3_MinGW_64_bit_Debug\Makefile -spec win32-g++ 'CONFIG+=debug'
mingw32-make -C build\Desktop_Qt_6_7_3_MinGW_64_bit_Debug -j4

New-Item -ItemType Directory -Force build_tests | Out-Null
qmake tests\service_integration_tests.pro -o build_tests\Makefile -spec win32-g++ 'CONFIG+=debug'
mingw32-make -C build_tests -j4
$env:QT_QPA_PLATFORM = 'offscreen'
.\build_tests\debug\service_integration_tests.exe
Remove-Item Env:QT_QPA_PLATFORM
```

Les tests CRUD et de validation écrivent uniquement dans une base SQLite temporaire en mémoire. Les tests Oracle vérifient les lectures, la connexion unique, la navigation entre les cinq pages, la recherche, l'ouverture/annulation du formulaire, les propriétaires, le planning, les statistiques et la génération d'un PDF temporaire. Ils ne déclenchent aucun ajout, modification ou suppression sur Oracle. La validation CRUD Oracle par écriture n'est donc pas couverte par ces tests.

## Sauvegarde et Git

La sauvegarde des fichiers communs avant modification est dans `tmp/integration/avant-integration-services.zip`. Les journaux de compilation/tests et captures de vérification sont dans `tmp/integration/`.

Ce dossier de travail ne contient pas de dépôt `.git`. Aucun pull, commit ou push n'a été effectué. Pour l'intégration Git de l'équipe, reprendre les fichiers modifiés (`ANIWORLD2.pro`, `mainwindow.ui`, `mainwindow.h/.cpp`, `.gitignore`) et les nouveaux dossiers `services/`, `tests/` et ce document dans le vrai dépôt, puis appliquer la procédure de branche de l'équipe décrite dans le guide. Ne pas versionner les dossiers de compilation ou `tmp/`.

## Résultat local de validation (8 novembre 2026)

Compilation de l'application réussie avec Qt 6.7.3 / MinGW 64 bits. Les tests d'intégration ont obtenu 4 réussites et 0 échec (initialisation, CRUD/validation isolés, Oracle/navigation/formulaires/PDF, nettoyage).

Les pages Designer Employés, Stock, Locaux et Dossiers médicaux ont été comparées structurellement à la sauvegarde : aucune modification. Les noms des widgets/layouts restent uniques dans le formulaire fusionné.

Limite constatée dans le module Médical préexistant : ses requêtes font référence à `ID_EMPLOYES` et `REF_ANIMAL`, absents du schéma Oracle actuel (`ID_EMP` et `ID_ANIMAL` existent). Certains champs médicaux optionnels sont également absents. Ces erreurs apparaissent dans le journal pendant les tests de navigation ; elles ne proviennent pas du module Services. Les sources Médical n'ont pas été modifiées dans cette intégration. La réussite des tests Services ne constitue donc pas une validation complète des CRUD des autres gestions.

## Correctif du crash Services au partage (9 novembre 2026)

Un test sans table Services reproduisait SIGSEGV dans `QHeaderView::setSectionResizeMode`, appelé depuis le constructeur ServiceWidget avec un indice inexistant sur un modèle SQL à zéro colonne. Le debugger a confirmé cette pile d'appels. Une différence de schéma ou de connexion chez un autre membre peut déclencher cette situation ; l'erreur SQL de son poste reste à relever pour identifier l'objet/configuration précis manquant.

La configuration des colonnes est désormais bornée par le nombre de colonnes du modèle et réappliquée après chaque remplacement du modèle. Les erreurs de connexion, de préparation et d'exécution SQL remontent explicitement depuis Service::afficher. L'interface affiche « Services indisponibles » et permet de récupérer après actualisation, sans configurer des colonnes inexistantes ni tenter de charger indéfiniment un modèle en erreur.

Validation : **8 tests réussis, 0 échec**, incluant table manquante, connexion SQLite fermée, connexion QODBC fermée, absence totale de connexion, réparation du schéma temporaire avec la même page encore ouverte, puis nouvel échec. Les vérifications Oracle/navigation/formulaire/PDF restent réussies. Les écritures des tests restent confinées à SQLite en mémoire.

Pour transmettre le projet : lire `LIRE_AVANT_PARTAGE_SERVICES.txt`. L'archive de sources corrigées exclut build/, build_tests/, tmp/ et les réglages Qt Creator propres au poste. Le script `sql/diagnostic_services_lecture_seule.sql` vérifie le contrat de lecture sans modifier Oracle.
