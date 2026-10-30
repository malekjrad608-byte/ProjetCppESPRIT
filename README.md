# ANIWORLD — version active du 30 octobre 2026

## Ajout depuis Services et modèle Oracle

**Services → Ajouter → Réf. service / Propriétaire** : la liste est relue depuis les propriétaires de Médical à chaque ouverture. L'enregistrement crée une ligne dans `SERVICE`, sans créer de dossier médical. Annuler revient à la liste Services. Le bouton d'ajout de service dans Médical et la redirection vers Médical ont été retirés.

`REF_SERVICE` contient le nom du propriétaire et reste UNIQUE selon la demande : un même nom ne peut donc référencer qu'un service. Le modèle relit le propriétaire du dossier sélectionné avant la sauvegarde. Les anciennes références ont été conservées ; elles restent modifiables sans inventer d'association médicale.

Les cinq tables physiques sont `SERVICE`, `LOCAL`, `STOCK`, `EMPLOYER`, `MEDICAL`. Les quatre FK obligatoires sont :

- `LOCAL.ID_SERV → SERVICE.ID_SERV`
- `STOCK.ID_LOCAL → LOCAL.ID_LOCAL`
- `EMPLOYER.ID_SERV → SERVICE.ID_SERV`
- `MEDICAL.ID_EMP → EMPLOYER.ID_EMP`

Les cinq clés primaires sont générées automatiquement par séquence et trigger. Les contraintes UNIQUE portent sur `REF_SERVICE`, `NOM_LOCAL`, `ARTICLE`, `CIN`, `ID_ANIMAL`. Les cinq tables contiennent uniquement les colonnes demandées, dans leur ordre exact : 8 pour SERVICE, 8 pour LOCAL, 9 pour STOCK, 17 pour EMPLOYER et 12 pour MEDICAL. Les anciennes colonnes supplémentaires ont été retirées ; leur contenu reste dans la sauvegarde Data Pump. Les lignes, identifiants et relations des colonnes conservées sont préservés. `LOCAL.ETAT` est désormais un champ enregistré (Disponible, Occupé, Hors service), choisi dans Qt. `STOCK.ID_ARTICLE` est la clé primaire ; LOT peut se répéter, ARTICLE reste unique.

Dans Médical, **Espèce** désigne le type d’animal (Chien, Chat, Lapin, Oiseau), enregistré dans `MEDICAL.ESPECE`. **Race** reste dans `MEDICAL.RACE`. Le champ de paiement ajouté par erreur a été retiré par la migration `011_medical_espece_animal.sql` ; les espèces et races existantes sont conservées. Stock distingue désormais la référence article de son nom ; Employés propose CIN, date de naissance, adresse, type de contrat et salaire.

Les vues de compatibilité `SERVICES`, `LOCAUX`, `STOCK_LOTS`, `EMPLOYES`, `MEDICAL_RECORDS` permettent aux modules Qt de lire les colonnes historiques sans dupliquer les données. Elles apparaissent sous **Views**, les cinq tables physiques sous **Tables** dans SQL Developer. En mode Oracle, les contrôles associés aux champs supprimés (facturation, détails cliniques supplémentaires, unité stock et suivi du nettoyage) sont masqués ; les vues renvoient des valeurs neutres pour ces anciens champs, sans les stocker.

Migration finale appliquée : `database/oracle/migrations/012_mld_strict.sql`, après 009–011. Ne pas relancer ces migrations sur la base déjà migrée. Sauvegarde complète avant remplacement : `backups/before-strict-mld-20261030/` (sources, exécutable et export Data Pump). `database/oracle/schema.sql` installe le modèle exact dans un schéma vide. Le contrôle en lecture seule est `database/oracle/verifier_modele_strict.sql` ; le test des IDs est `database/oracle/verifier_ids_automatiques.sql` (insertions annulées).

Validation du 30/10 : compilations Release et Debug réussies, exécutable ANIWORLD actualisé. Les sept scénarios ciblés passent (CRUD Oracle, navigation des cinq modules, choix du propriétaire/annulation, médical, stock et deux scénarios atelier). Le contrôle SQL confirme les colonnes exactes, quatre FK actives, zéro orphelin, cinq IDs automatiques et aucun objet invalide. Le test Oracle complémentaire valide les trois états du local, deux articles avec le même lot et la stabilité des IDs stock lors des modifications. Les données de test sont supprimées ou annulées. Les fichiers `connection.cpp/.h` de l’enseignante sont inchangés.

## Alignement Qt / Oracle — 30 octobre 2026

En connexion Oracle, le formulaire Service affiche **Réf. service** : liste non modifiable de propriétaires relus dans MEDICAL. La sélection fournit directement le texte enregistré dans SERVICE.REF_SERVICE ; aucun propriétaire ni dossier médical n'est créé depuis Services. Le modèle relit le propriétaire avant INSERT/UPDATE. Les références existantes restent conservées. La contrainte UNIQUE Oracle continue de refuser deux services portant la même référence.

Les formulaires affichent les IDs automatiques en lecture seule. Employé suit l'ordre des 17 attributs Oracle ; Stock affiche ses 9 colonnes et Local ses 8 colonnes. Service affiche les 8 attributs persistés ; Client, Référent et Facture ne sont plus présentés comme des champs Service en mode Oracle. Les contrôles de nettoyage/occupation chiffrée sans colonne persistée sont masqués. Le lot du stock peut être vide ou répété, tandis que la référence ARTICLE reste unique. Médical distingue Espèce, Race et Propriétaire.

Sauvegarde des sources avant cet alignement : `backups/before-qt-oracle-alignment-20261030/sources.zip`.

Validation : Release et Debug compilent ; sept scénarios ciblés passent, puis les deux scénarios Oracle sont revérifiés après la dernière correction visuelle. Le test vérifie aussi un lot vide et les colonnes visibles. ANIWORLD.exe est actualisé et sa connexion QODBC à RAWAND est vérifiée. Captures et résultats : `artifacts/atelier-crud-review/qt-align-*`.

## Historique technique

Les sections datées ci-dessous décrivent les versions antérieures. Pour le schéma et le parcours utilisateur actuels, utiliser la section du 30/10 ci-dessus.

## Requêtes dans les classes C++ — 20/10/2026

Les anciennes fonctions de stockage/planning sont maintenant des méthodes des modèles :

| Ancien namespace | Classe et fichiers actuels |
| --- | --- |
| `ServicePlanning` | `Service`, `src/models/service.h` et `.cpp` |
| `FiveTableStore` (employés et affectations) | `Employe`, `src/employees/employe.h` et `.cpp` |
| `FiveTableStore::invoiceStatus` | `Service` |
| `EmployeeStore` (initialisation/import local) | `Employe` |
| `MedicalStore` | `MedicalRecord`, `src/medical/medicalrecord.h` et `.cpp` |
| `OracleCollection` | SQL propre à `Local` et à `StockRecord`, dans leurs fichiers `.h` / `.cpp` |

Les anciens fichiers ont été retirés de `sources.pri`. `Accounts` et la façade `OracleConnection` sont aussi des classes ; `EmployeeQueries::read` est une méthode privée. Les requêtes techniques de connexion restent dans la façade ; **`connection.h` et `connection.cpp` fournis par l’enseignante restent inchangés**.

La lecture/écriture de collections de locaux et de lots conserve le contrôle de révision contre les modifications concurrentes. Les méthodes de Service et MedicalRecord, les transactions et les affectations gardent leur comportement. Les noms/champs sont ceux du projet, pas une nouvelle table `etudiant`.

Contrôles : `python tools/check_workshop_architecture.py` pour les vues et `python tools/check_model_sql_ownership.py` pour l’appartenance du SQL à des classes dans les sources compilées. Les journaux de cette intervention portent le préfixe `entity-classes-` dans `artifacts/atelier-crud-review`. Sauvegarde avant intervention : `backups/before-entity-classes-20261020/source.zip`.

Validation finale : Release et Debug compilent ; 39 passages de régression réussis, 0 échec, 11 scénarios optionnels ignorés ; les deux scénarios Oracle passent séparément (4 passages avec initialisation/nettoyage). Oracle conserve cinq tables, aucune anomalie de relation et aucun objet invalide. `ANIWORLD/ANIWORLD.exe` a été actualisé et sa connexion QODBC vérifiée.

## Conformité Services à l’atelier — 19/10/2026

- `Service` : attributs privés, constructeur paramétré, getters/setters et appels `ajouter()`, `modifier()`, `supprimer(id)`, `afficher()`.
- `service.cpp` contient les requêtes de Service, avec `prepare`, `bindValue` et `exec`. La transaction Oracle et les contraintes d’affectation sont conservées.
- `ServiceWidget` possède `Service Stmp`. La liste est maintenant un `QTableView` qui reçoit directement `Stmp.afficher()`. Après une écriture réussie, le modèle est renouvelé et la sélection conservée. Les cartes, filtres et tris restent disponibles.
- `ServiceDialog::getService()` construit l’objet depuis les champs du formulaire. Les autres usages de Service passent par ses accesseurs.
- Le Singleton fourni par l’enseignante est inchangé. Aucune migration ni suppression des données Oracle pour cette refactorisation.

Pour la démonstration dans Qt Creator : **Ctrl+K**, ouvrir successivement `service.h`, `service.cpp`, `servicewidget.h`, `servicewidget.cpp`, `connection.cpp`, `main.cpp`. La correspondance des 50 pages et les adaptations sont dans `artifacts/atelier-crud-review/rapport-atelier.html`.

Validation du 19/10 : Release et Debug compilent ; 39 tests de régression passent (11 optionnels ignorés). Les deux scénarios Oracle et les captures passent séparément. La version livrée confirme RAWAND / CDB$ROOT, exactement 5 tables et aucune anomalie de relation.

Sauvegarde de départ : `backups/before-service-atelier-20261019/source.zip`. Vérifications : `strict-service-regression.txt`, `strict-service-oracle.txt` et `strict-service-deployed-check.txt` dans `artifacts/atelier-crud-review`.

## Atelier CRUD — architecture et corrections du 17/10/2026

Le rapport `artifacts/atelier-crud-review/rapport-atelier.html` détaille les 50 pages de `Atelier_Crud-architectureQT.pptx`, les changements et les adaptations à ANIWORLD. La GUI et l'entité Etudiant sont des exemples du cours ; les cinq entités métier restent celles du projet.

- Le SQL a été retiré des vues Employés, Planning, listes de relations et Facture. Les nouveaux modèles `EmployeeQueries`, `RelationModel` et `InvoiceModel` prennent en charge ces opérations.
- `Service` et `Employe` exposent maintenant `ajouter`, `afficher`, `modifier`, `supprimer`. `Service::afficher()` utilise `QSqlQueryModel`. Les cartes et la liste existantes sont alimentées à partir de ses données. `MedicalRecord` expose le CRUD ; `Local` et le nouveau `StockRecord` gèrent la lecture et l'enregistrement de leurs collections avec protection contre les modifications concurrentes.
- Les requêtes de l'application principale utilisent `prepare`, les valeurs liées, puis `exec()` sans texte SQL. Les valeurs utilisateur ne sont pas concaténées dans les requêtes.
- Le formulaire Services est intégré dans un `QStackedWidget`. Les boîtes de succès Employés ont été retirées. En cas d'échec, les formulaires Services et Employés conservent la saisie et affichent l'erreur.
- Validation C++ renforcée dans les modèles : identité/date/courriel/heures des employés, durée/statut/catégorie des services, lots/local obligatoire et cohérence des locaux. Les contraintes Oracle et les cinq tables sont conservées.
- Le Singleton de connexion et l'ouverture avant la fenêtre principale étaient déjà présents ; ils ont été conservés et vérifiés.

Validation : **42 tests de régression réussis, 0 échec, 8 scénarios optionnels ignorés**, puis **6 passages ciblés réussis** (4 scénarios plus initialisation/nettoyage), dont les deux scénarios Oracle à cinq tables. Le contrôle `tools/check_workshop_architecture.py` examine 39 fichiers de vue. Les compilations Release et Debug réussissent. Les fixtures Oracle sont nettoyées après les tests. Rapports dans `artifacts/atelier-crud-review/`.

Sauvegarde des sources avant cette refactorisation : `backups/before-atelier-20261017/source.zip`.

Nouvelle vérification du 17/10 : **39 tests réussis, 0 échec, 11 tests optionnels ignorés**, puis **6 passages ciblés réussis** avec Oracle (4 scénarios et initialisation/nettoyage). Release et Debug recompilés.  le programme principal utilise maintenant **les fichiers originaux `src/database/connection.cpp` et `connection.h` fournis par l’enseignante**, sans réécriture du Singleton. Seuls les trois paramètres ODBC diffèrent du ZIP original. `OracleConnection::open()` appelle `Connection::createInstance().createConnection()`. Tous les modèles partagent cette connexion via `OracleConnection::database()`. `DatabaseManager` reste la façade de démarrage et du mode démo SQLite. Les tests vérifient une seule connexion Oracle ouverte et les retours démo/Oracle. Comparaison des fichiers : `artifacts/atelier-crud-review/strict-singleton-comparison.txt`. Le rapport décrit aussi les différences restantes de déclarations/API avec les exemples Etudiant : il ne s’agit pas d’une copie textuelle de tout le PowerPoint.

Organisation des sources : `src/database` contient `connection` (Singleton original), `databasemanager` (initialisation) et `oracleconnection` (adaptateur et vérification du schéma). Les helpers `fivetablestore` (transactions et relations Services/Employés) et `oraclecollection` (persistance Stock/Locaux et détection des modifications concurrentes) sont dans `src/models`, avec les modèles qui les utilisent. Ce sont des fichiers C++, pas des tables supplémentaires.

Le mode Oracle actif utilise **RAWAND**, **localhost:1521/XE**, **CDB$ROOT**, avec le DSN ODBC 64 bits **Source_Projet2A18**. Lancer `Lancer ANIWORLD.cmd`. L'ancien schéma ANIWORLD_APP/XEPDB1 reste séparé.

## Modèle commun confirmé

| Table | Clé primaire | Relation obligatoire |
|---|---|---|
| SERVICES | ID | Au moins un employé dans son équipe |
| EMPLOYES | ID | ID_SERVICE → SERVICES.ID |
| LOCAUX | ID | ID_SERVICE → SERVICES.ID |
| STOCK_LOTS | LOT | ID_LOCAL → LOCAUX.ID |
| MEDICAL_RECORDS | ID | ID_EMPLOYE → EMPLOYES.ID |

Cardinalités retenues : Service (0,n)–Local (1,1), Local (0,n)–Stock (1,1), Service (1,n)–Employé (1,1), Employé (0,n)–Dossier médical (1,1). La relation Service–Local n'est donc pas plusieurs-à-plusieurs.

`SERVICES.ID_EMPLOYE` désigne un référent de l'équipe. La clé étrangère composée `(ID_EMPLOYE, ID) → EMPLOYES(ID, ID_SERVICE)`, différée jusqu'au COMMIT, garantit qu'un service possède au moins un employé de sa propre équipe. Créer un service et son premier employé se fait dans une seule transaction. Un employé appartient à un seul service dans ce modèle : une réaffectation remplace son rattachement, elle n'ajoute pas une deuxième affectation historique.

## Utilisation dans Qt

- **Services → Ajouter** : choisir « + Créer le premier employé », saisir son nom et prénom. Pour ajouter d'autres membres, ouvrir **Employés → Ajouter**, puis choisir le service.
- **Locaux** : choisir obligatoirement le service proposé.
- **Stock** : choisir obligatoirement le local du lot.
- **Dossiers médicaux** : choisir obligatoirement l'employé responsable.
- **Planning des employés** : une ligne par membre de l'équipe, avec l'horaire du service. Une réaffectation sur un créneau occupé est refusée. Le service d'origine doit conserver au moins un employé.
- Un élément encore référencé ne peut pas être supprimé ; Oracle protège les liens. Une facture émise est figée dans le service.

Il y a exactement **5 tables**. `PLANNING_AFFECTATIONS` est désormais une **vue en lecture seule**, calculée à partir de SERVICES et EMPLOYES. Elle ne stocke aucune tâche indépendante. Les comptes applicatifs existants ont été conservés dans `%LOCALAPPDATA%/ANIWORLD/ANIWORLD/accounts-local.json` (hachages salés, mêmes identifiants). La facture simple est incluse dans SERVICES ; le client reste un texte dans le service. L'ancien journal de mouvements et les anciennes données annexes sont conservés dans la sauvegarde, sans table supplémentaire active.

## Vérification et sauvegarde

Dans SQL Developer, connexion RAWAND, clic droit **Tables → Refresh**. Exécuter `database/oracle/verifier_cinq_tables.sql` avec **F5** : 5 tables, 0 anomalie de relation, 0 objet invalide attendu. Les tables LOCAUX, STOCK_LOTS et MEDICAL_RECORDS étaient vides avant la migration ; aucune donnée de démonstration n'a été ajoutée. L'employé existant #12 est maintenant rattaché au service existant #5.

Sauvegarde complète avant migration : `backups/before-five-tables-20261016/RAWAND_BEFORE_FIVE_20261016.DMP`, avec journal Data Pump, anciens sources, ancien exécutable et comptes. `database/oracle/schema.sql` contient le modèle physique pour un schéma vide ; `migrations/005_five_tables.sql` documente la migration ponctuelle déjà appliquée. Ne pas rejouer les migrations 001–004 sur ce modèle.

Validation du 16/10/2026 : compilation Release et Debug réussie, 37 tests de régression réussis (10 tests optionnels ignorés), deux scénarios Oracle avec CRUD des cinq modules, contraintes, transferts, facturation et ouverture des interfaces réussis. Les données temporaires de test ont été supprimées. L’exécutable livré confirme `Connexion Qt QODBC : RAWAND / CDB$ROOT`. Rapports : `artifacts/five-final-integration-tests.txt`, `artifacts/five-regression-tests.txt`, `artifacts/five-final-database-state.txt`.

Les sections suivantes sont historiques ; elles ne décrivent pas la version Oracle active à cinq tables. Les modes `--demo` / `--local` restent séparés et conservent leur ancien stockage SQLite/JSON.

---

# Historique des versions précédentes

Application desktop **C++ / Qt Widgets 6.7.3**, version **3.0.0**, avec le logo chien/chat et le nom **ANIWORLD**. Depuis le 14 octobre 2026, la configuration de ce poste utilise **Oracle XE 21c / XEPDB1** pour les services, factures, employés, affectations, dossiers médicaux, comptes, locaux et stock. Les données locales précédentes ont été conservées et migrées. Les sections historiques ci-dessous décrivent les étapes antérieures ; SQLite/JSON restent accessibles avec `--local`, et `--demo` reste en mémoire.

## Oracle — installation active du 14 octobre 2026

- Application livrée : `ANIWORLD/ANIWORLD.exe`, pilote `ANIWORLD/sqldrivers/qsqlodbc.dll` inclus.
- Connexion : `localhost:1521/XEPDB1`, utilisateur `ANIWORLD_APP`, DSN utilisateur 64 bits `ANIWORLD_ORACLE`.
- Configuration : `%LOCALAPPDATA%/ANIWORLD/ANIWORLD/oracle.ini`, avec `enabled=true`. Le mot de passe Oracle est protégé par Windows DPAPI dans `oracle-password.dpapi`, hors du code source.
- Les comptes applicatifs existants conservent leurs mots de passe et restrictions. Ils sont distincts du compte Oracle.
- Vérification sans ouvrir l'interface : `ANIWORLD/ANIWORLD.exe --check-oracle`. Le mode Oracle échoue explicitement si la connexion est indisponible ; il ne bascule pas silencieusement vers SQLite.
- Le mode `--local` est indépendant : ses modifications ne sont pas synchronisées vers Oracle après la migration initiale.

**SQL Developer :** le profil `ANIWORLD - XEPDB1` est installé. Si le mot de passe est demandé, exécuter `tools/copy-oracle-password.ps1`, puis le coller soi-même dans le dialogue. Le profil exportable `database/oracle/sql-developer-connection.json` ne contient aucun mot de passe. Il inclut `subtype=oraJDBC`, indispensable à la sélection du créateur de connexion Oracle, et `NoPasswordConnection=TRUE` : malgré son nom, ce paramètre active la demande du mot de passe absent dans `ConnectionPrompt.DefaultPrompter` de cette version. Ouvrir `database/oracle/verification.sql` avec cette connexion et utiliser F5 pour afficher les tables et comptages.

**Schéma et migration :** `database/oracle/schema.sql` crée 11 tables adaptées au code Qt existant. `tools/setup-oracle.ps1`, `tools/install-oracle-schema.ps1` et `--migrate-oracle` ont déjà été exécutés sur ce poste. Ne pas réinstaller le schéma sur les tables existantes. L'import conserve les identifiants, factures et empreintes des comptes ; son marqueur empêche de réimporter les anciennes données. Stock et Locaux deviennent persistants, avec transactions et contrôle de révision ; le stock dispose d'un journal des mouvements. Sauvegarde préalable : `backups/before-oracle-20261014-005853/`.

**Portabilité :** sur un autre poste, il faut installer/configurer Oracle et son pilote ODBC 64 bits, créer le DSN et un secret propre au compte Windows. Le fichier DPAPI de ce poste n'est pas portable. Le gestionnaire utilise le Singleton de Meyers et QtSql, conformément au principe de l'atelier fourni.

**Validation :** connexion de l'exécutable livré réussie ; 2 services, 6 employés, 8 dossiers médicaux, 6 comptes, 14 locaux et 19 lots. Comparaison en lecture seule de 256 valeurs avec SQLite, sans différence ; aucun objet Oracle invalide. Suite habituelle : 35 réussites, 0 échec, 4 tests conditionnels ; ces quatre tests ont également réussi dans les exécutions Oracle et captures séparées. Logs : `artifacts/oracle-integration/`. Vérificateur reproductible : `tools/verify_oracle_migration.pro` (à exécuter avant que les données des deux bases ne divergent).

**Limites du modèle :** cette migration privilégie la compatibilité avec les modules actuels. Elle garde plusieurs dates en texte ISO et les informations de facture dans le service. Elle ne transpose pas intégralement Lucidchart : séparer client, animal, consultation, facture et lignes de facture nécessitera une évolution du modèle et du code. Revue, corrections de cardinalités et 12 questions de réunion : `output/pdf/ANIWORLD_Oracle_revue_et_reunion.pdf`.

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


## Source médicale remplacée — 13 octobre 2026

Source active : `C:/Users/malek/Downloads/Dossier_Medical_ANIWORLD (2)/Dossier_Medical_ANIWORLD`. Les modèles, cartes, formulaires, détails et exemples ont été réimportés depuis ce dossier, puis adaptés au design et aux services communs. Après adaptation, ces neuf fichiers sont identiques à la version intégrée précédente : ce dossier ne fournit pas de nouvelle fonction médicale. Le manifeste `src/medical/source-manifest.json` conserve les empreintes des fichiers fournis et la liste des fichiers repris.

Le module conserve son accès direct **medical / medical123** (ou le mot de passe déjà changé), sa sauvegarde SQLite, l'historique réel et les exports PDF. L'administrateur y accède également. Les sources originales fournies restent intactes. Sauvegarde préalable des sources médicales et de la base : `artifacts/medical-replacement/before/`.
`artifacts/medical-replacement/results.txt` : 6 contrôles réussis, aucun échec (persistance et historique, opérations médicales et PDF, comptes, navigation et restrictions). Les compilations Release et Qt Creator Debug ont réussi ; le nouvel exécutable a été livré et son lancement vérifié. La comparaison avec la sauvegarde confirme que toutes les tables existantes, y compris les dossiers et les comptes, sont inchangées.


## Échange réel de l'écran médical — 14 octobre 2026

La fenêtre et le fichier Qt Designer `mainwindow.cpp/.h/.ui` du dossier `Downloads/Dossier_Medical_ANIWORLD (2)/Dossier_Medical_ANIWORLD` remplacent désormais l'ancien écran reconstruit. Ils sont intégrés sous le nom `MedicalWidget`, avec `medicalwidget.ui` et le style local `resources/medical-source.qss`. La disposition suit la deuxième capture fournie : quatre compteurs, recherche sur une ligne, filtres dessous, cartes en deux colonnes quand la largeur le permet, panneau compact en haut à droite. Le menu Fichier rejoint la barre supérieure commune. Les accès aux autres modules et les mots de passe restent gérés par la connexion commune.

Les huit exemples fournis (Max, Luna, Rocky, Mimi, Bella, Simba, Toby, Coco) sont importés une seule fois en mode normal **uniquement si le registre est vide**. Le marqueur `medical-reference-ui-v1` empêche leur réapparition après suppression. Un registre déjà rempli reste inchangé. Cela remplace le comportement précédent, où les exemples étaient réservés au mode démo. Ces fiches initiales sont des exemples fictifs du projet fourni.

Les ajouts, modifications et suppressions restent enregistrés localement. L'historique lit les consultations réelles ; l'export reste exclusivement PDF. Sauvegarde avant échange et captures : `artifacts/medical-reference-layout/`.


Validation et livraison de cet échange : **38 tests réussis**, puis **5 contrôles Windows réussis** après les corrections des icônes et du menu Fichier. Captures : `artifacts/medical-reference-layout/windows/medical-normal.png`. Journaux : `full-results.txt`, `windows-results.txt`. Les compilations Release et Qt Creator Debug sont à jour. `ANIWORLD/ANIWORLD.exe` a été remplacé et son démarrage vérifié. Les comptes et les données des autres modules ont été comparés à la sauvegarde et conservés ; l'import médical initial a été contrôlé (`delivery-verification.json`).

## Conception commune S5 — 14 octobre 2026

Le modèle cible commun couvre Services, Employés, Stock, Locaux, Dossiers médicaux et Adoptions, avec clients, animaux et factures partagés. Les livrables sont dans `database/conception/` ; le dossier de présentation est `output/pdf/ANIWORLD_Modeles_communs_S5.pdf` (27 pages).

- `ANIWORLD_MCD_commun.drawio` : quatre vues Merise avec cardinalités, incluant les associations N:M de compétences.
- `ANIWORLD_MLD_commun.drawio` et `ANIWORLD_MPD_commun.drawio` : vues globales éditables du modèle relationnel et des types Oracle.
- `modele_relationnel.txt` et `modele_commun.json` : définition des 23 tables et des 33 références.
- `modele_physique_oracle.sql` : DDL à exécuter uniquement dans un schéma vide dédié. Un garde-fou refuse ANIWORLD_APP, SYS et SYSTEM.
- `valider_modele.sql` : jeu de données fictif avec rollback, réservé à ANIWORLD_S5_MODEL.

Validation réelle : schéma sans authentification `ANIWORLD_S5_MODEL`, dans XEPDB1, créé et testé ; **20 contrôles réussis**, 23 tables, 33 FK, 2 vues et 20 triggers, aucun objet invalide. Les données de test ont été annulées. Journaux : `artifacts/conception-s5/oracle-validation.txt` et `oracle-structure.txt`.

**Distinction essentielle :** Qt utilise encore les 11 tables de compatibilité de `ANIWORLD_APP`. La conception commune ne remplace pas automatiquement ces tables ni le code des écrans. Le PDF décrit la reprise des données et les contrôles transactionnels encore à intégrer (affectations, validation clinique, émission de facture, quantité et mouvement de stock, placement et suivi d’adoption). Le module Adoptions appartient au cahier et au modèle cible ; son interface n’est pas intégrée à cette livraison.

Lucidchart : un import du MLD a créé le document `4691305b-37b1-4a1f-a194-2004a104040a`. L’éditeur a ensuite signalé le mode hors ligne et reste bloqué au chargement. Le remplacement du diagramme initial `smart pett` n’a pas été effectué. La version locale finale des fichiers draw.io fait foi ; elle comprend les derniers ajustements de légende et les vues Merise.

Régénération : exécuter `tools/build_common_model.py`, puis `tools/create_common_model_report.py`. Aucun commit ni push n’a été effectué pour cette conception.

### Livraison Lucidchart terminée

Document final : https://lucid.app/lucidchart/d70d0f76-6b8b-424b-9602-31f57283690e/edit?invitationId=inv_53a8b7b0-aca3-4d2e-ad48-9fc43205b6ec

Les **13 pages** sont présentes : 4 vues MCD, 4 pages MLD, 4 pages MPD et 1 page avec 8 questions de réunion et recommandations. Le rendu des trois types de modèle et de la page réunion a été vérifié. Le premier import XML non compressé était incomplet ; l'import final utilise le format draw.io compressé standard (`database/conception/ANIWORLD_Modeles_S5.drawio`). Régénération : `tools/build_lucid_delivery.py`.

Le document original « smart pett » reste disponible. Utiliser le lien ci-dessus pour la version corrigée. Le compte Lucid affiche « You're out of free shapes for this document » : la consultation est disponible, mais l'édition complète est limitée par l'offre gratuite. Aucun essai ni abonnement payant activé. Captures de vérification : `artifacts/conception-s5/lucid-mld-final.png` et `lucid-final-pages.png`.

### Correction des locaux — 15 octobre 2026

Version de référence mise à jour dans Lucidchart : https://lucid.app/lucidchart/cba5e9d9-9692-42d8-8c60-86e6cd5a7792/edit?invitationId=inv_155f0930-667c-49b9-bf80-df79bc3b5956

Les cinq premières pages décrivent le schéma **ANIWORLD_APP réellement utilisé par Qt**, avec les types lus dans le dictionnaire Oracle. Les huit pages marquées **CIBLE** décrivent la conception normalisée commune distincte (sans RH, avec dossier unique par animal, clients, factures et lots). Cette conception ne doit pas être présentée comme une migration complète de l'application. L'ancien diagramme « Smart Pet Manager » est conservé comme source ; utiliser le nouveau lien pour les corrections.

Migration appliquée dans XEPDB1 : `database/oracle/migrations/002_local_links.sql`. Elle ajoute une colonne facultative `ID_LOCAL` et une FK vers `LOCAUX.ID` dans `SERVICES`, `MEDICAL_RECORDS` et `STOCK_LOTS`, avec index. Les trois contraintes sont activées et validées ; aucun objet ANIWORLD_APP invalide. Le script est réexécutable. Aucune ligne ancienne n'a été affectée arbitrairement : les trois nouveaux champs restent NULL jusqu'au choix dans le formulaire Qt. La suppression d'un local référencé est refusée ; utiliser la réaffectation ou l'état hors service.

Les trois formulaires Qt proposent maintenant « Local ». Les valeurs sont conservées lors des lectures et modifications. En mode SQLite/démo, le sélecteur est désactivé : les locaux de ce mode utilisent un stockage JSON indépendant. Les dates ISO VARCHAR2 existantes restent inchangées. L'affectation n'est pas une réservation horaire : elle ne recalcule pas l'occupation et ne bloque pas encore les chevauchements d'horaires.

Validation : test `oracleLocalLinks` via le pilote QODBC de Qt (ajout/relecture/modification, affectation vide, plusieurs services par local, rejet des trois références inexistantes, protection de suppression, conservation du local d'un lot, présence des sélecteurs). Données fictives annulées ou supprimées après test. Régression : **35 tests réussis, 0 échec, 5 tests optionnels ignorés** ; test Oracle ciblé : **3 réussis, 0 échec** (initialisation et nettoyage inclus). Comparaison de toutes les anciennes valeurs : 14 locaux, 3 services, 8 fiches médicales et 19 lots conservés.

Sources et ancien exécutable sauvegardés dans `backups/before-local-links-20261015/`. Journaux, états Oracle avant/après et captures dans `artifacts/local-links-20261015/` (ces sauvegardes métier sont locales, ne pas publier). Release et Debug recompilés ; `ANIWORLD/ANIWORLD.exe` mis à jour. Aucun commit/push effectué.

Diagramme éditable local : `database/conception/ANIWORLD_Oracle_Qt_corrige_20261015.drawio`. Régénération : `tools/snapshot_local_schema.py after`, puis `tools/build_verified_lucid.py`. Le fichier exporte uniquement la structure, sans données métier ni secrets. La limite de formes de l'offre gratuite Lucidchart reste applicable à l'édition.

### Planning des employés dans Services — 16 octobre 2026

Le bouton supérieur « Rapport PDF » et son aperçu sont retirés de Services. La facture PDF individuelle reste accessible. « Planning des employés » ouvre une vue hebdomadaire des services, avec date, début/fin, employé affecté, service, client et statut. La navigation par semaine, le choix de date, le filtre par employé (y compris les employés sans affectation) et « Services non affectés » permettent de consulter les créneaux. Les services annulés sont masqués par défaut, avec une option pour les afficher.

Le planning lit directement `SERVICES.ID_EMPLOYE`, `DATETIME` et `DURATION` avec `EMPLOYES`, ainsi que les tâches de `PLANNING_AFFECTATIONS`. « Affecter / modifier le service » ouvre le formulaire existant ; le champ « Employé affecté » et le créneau se sauvegardent dans RAWAND via QODBC. « Voir son emploi du temps » dans le formulaire ouvre la semaine et l'employé sélectionnés, en consultation. Les tâches se gèrent dans Employés. Les services qui traversent minuit sont inclus dans la semaine concernée ; les heures du récapitulatif sont limitées à la semaine affichée.

Une affectation qui chevauche un service ou une tâche du même employé est refusée dans le formulaire et lors de l'enregistrement. Le message précise le rendez-vous et ses dates. Les créneaux contigus sont acceptés ; les services annulés ne bloquent pas. Modifier un service exclut son propre créneau du contrôle. L'affectation automatique des tâches dans Employés tient aussi compte des services. L'indisponibilité générale reste visible. En mode local/démo, la consultation est disponible ; l'édition des affectations utilise Oracle.

Migration additive `database/oracle/migrations/004_employee_schedule.sql`, appliquée à RAWAND : deux triggers composés et le package `EMPLOYEE_SCHEDULE` contrôlent les écritures des services et des tâches. Le verrou de la ligne employé sérialise les réservations concurrentes ; le contrôle après instruction évite les tables mutantes. Aucun rendez-vous existant n'est effacé. Le marqueur `employee-schedule-v4` est requis par cette version de Qt. Une erreur ORA-20051 indique un chevauchement refusé. Les dates sont interprétées comme heures locales de l'application.

Validation du blocage : `artifacts/schedule-guards-focused.txt` (Qt/QODBC, créations et modifications, tâches, annulations, limites et aperçu par employé) et `artifacts/schedule-guards-concurrency.txt` (deux sessions Oracle, attente de COMMIT/ROLLBACK). Les essais Qt sont annulés par rollback ; la fixture temporaire du test concurrent est retirée. Captures dans `artifacts/schedule-guards/`.

Régression après ajout du blocage : **37 réussis, 0 échec, 9 tests optionnels ignorés** (`artifacts/schedule-guards-regression.txt`). Les 6 scénarios ciblés, dont 3 Oracle, passent également (8 succès avec initialisation/nettoyage). Release et Debug normal reconstruits ; `ANIWORLD/ANIWORLD.exe` mis à jour et connexion QODBC RAWAND vérifiée. L'audit confirme que le service existant est conservé, qu'aucune donnée de test ne subsiste et qu'aucun objet Oracle n'est invalide.

Vérification : `artifacts/service-planning-focused.txt` (limites de semaine, filtres, réaffectation/changement d'heure via QODBC avec rollback, facture PDF conservée) et `artifacts/service-planning-regression.txt` (36 réussis, 0 échec, 8 tests optionnels ignorés). Captures dans `artifacts/service-planning/`. Requête de consultation : `database/oracle/planning_services.sql`. Exécutable normal : `Lancer ANIWORLD.cmd`. Si l'ancien Debug tourne dans Qt Creator, l'arrêter puis reconstruire ; le Debug compilé pendant son utilisation se trouve dans `debug/gestion_service_updated.exe`.

### Connexion active — RAWAND, 16 octobre 2026

La configuration actuelle de Qt utilise **QODBC**, DSN **Source_Projet2A18**, utilisateur **rawand**, service **//localhost:1521/XE** (conteneur CDB$ROOT). Pour la présentation de l'atelier demandée, les trois valeurs de connexion sont maintenant écrites directement dans `src/database/oracleconnection.cpp` : `setDatabaseName`, `setUserName`, `setPassword`, puis `open`. Qt ne lit plus le fichier DPAPI pour cette connexion ; les outils d'administration conservent leur configuration locale. Validation après ce changement : `artifacts/rawand-atelier-connection.txt`, connexion RAWAND réussie. Release et Debug recompilés, exécutable livré mis à jour.

Le schéma RAWAND a été initialisé avec les 14 tables applicatives **sans reprise des données anciennes**. Les six comptes applicatifs initiaux sont recréés (`admin/admin123`, puis nom du module suivi de `123`) ; ils sont distincts du compte Oracle `rawand`. Le marqueur `fresh-oracle-v1` bloque l'import SQLite et les collections Stock/Locaux démarrent vides sans importer les exemples. L'ancien schéma n'est plus utilisé par la configuration active et n'a pas été effacé.

Release, Debug et `ANIWORLD/ANIWORLD.exe` sont recompilés. Validation : `artifacts/rawand-connection.txt` confirme RAWAND / CDB$ROOT ; `artifacts/rawand-qt-tests.txt` valide les six connexions applicatives, les cinq écrans, l'absence de données importées, l'interdiction d'import et les relations/écritures Qt avec rollback. Audit : `artifacts/rawand-audit.txt`, aucun objet invalide, contrainte désactivée ou référence orpheline. Les commandes SQL pour les données actuelles doivent utiliser les noms sans préfixe, ou `RAWAND`, et non `ANIWORLD_APP`.

### Relations communes opérationnelles — migration 003, 15 octobre 2026

`database/oracle/migrations/003_common_links.sql` est appliquée à **ANIWORLD_APP / XEPDB1**. Le schéma utilisé par Qt compte désormais **14 tables et 10 FK**, actives et validées. Les anciennes mentions des 11 tables décrivent l'état antérieur. Le modèle normalisé de 23 tables dans ANIWORLD_S5_MODEL reste distinct.

- Services : sélection d'un employé (`ID_EMPLOYE`) et d'un client enregistré (`ID_CLIENT`). « Nouveau client » crée une identité distincte ; aucun rapprochement automatique par nom.
- Consultations médicales : sélection facultative du service (`ID_SERVICE`). Un service peut avoir plusieurs consultations ; sa suppression est refusée tant qu'une consultation le référence.
- `CLIENTS`, `FACTURES`, `LIGNES_FACTURE` : facture brouillon et ligne de prestation créées automatiquement avec le service. L'export PDF valide l'émission et fige les informations facturées. Une réexportation utilise l'instantané enregistré. Une facture ne constitue pas une preuve de paiement. Le formulaire actuel prévoit une prestation par facture.
- La suppression d'un service conserve sa facture (référence historique et instantanés conservés, lien `ID_SERVICE` mis à NULL). La suppression d'un employé référencé par un planning ou un service est refusée ; utiliser l'indisponibilité pour conserver l'historique. Le planning n'est plus supprimé en cascade.
- Les liens historiques inconnus restent NULL. Les nouveaux sélecteurs relationnels sont disponibles en mode Oracle. Le mode SQLite/démo conserve son périmètre local ; il ne simule pas les nouvelles tables de facturation Oracle.

Vérification en lecture seule : ouvrir `database/oracle/verifier_relations.sql` dans SQL Developer sur ANIWORLD_APP puis **F5**. Il affiche chaque relation, les références orphelines, les contraintes invalides, la cohérence des totaux et les associations enregistrées. Description du modèle actuel : `database/conception/Modele_relationnel_Qt_003.txt`. Les pages Lucidchart livrées pour la migration 002 n'ont pas été remplacées lors de cette livraison.

Sauvegarde préalable : `backups/before-common-links-20261015/`. Contrôles Qt/QODBC : `artifacts/common-links-20261015/oracle-final.txt` ; audit Oracle : `oracle-audit.txt` ; comparaison des données anciennes : `data-preservation.json`. Les essais de facturation vérifient les instantanés, le rendu PDF et le refus des modifications après émission dans une transaction annulée ; aucune facture fictive émise n'est conservée en base. Exécutable livré : `ANIWORLD/ANIWORLD.exe`. Si Qt Creator garde une ancienne version ouverte, l'arrêter avant de reconstruire Debug. Aucun commit/push.

Validation finale : **35 tests de régression réussis, 0 échec, 6 tests optionnels ignorés** ; les deux scénarios Oracle ciblés passent (4 succès avec initialisation/nettoyage). `ANIWORLD.exe --check-oracle` termine avec le code 0 sur ANIWORLD_APP / XEPDB1. Les 14 locaux, 3 services, 8 consultations et 19 lots présents avant migration ont conservé toutes leurs anciennes valeurs. Le binaire Debug ouvert étant verrouillé par Windows, le nouveau Debug est compilé dans `debug/gestion_service_updated.exe` ; le lancement normal à jour utilise `Lancer ANIWORLD.cmd`.
