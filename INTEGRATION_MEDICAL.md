# Integration Medical — 16 novembre 2026

Module fourni : `C:\Users\malek\Downloads\ANIWORLD_projet_seul_formulaire (4)\standalone`.
Projet integre : `gestion_service.pro`.

## Organisation du projet

- Un seul formulaire Designer versionne : `src/mainwindow.ui`, avec la page Medical integree. Les adaptateurs UI sont generes au build.
- La navigation et les droits restent dans MainWindow. Le module utilise la connexion commune du projet.
- Les requetes du dossier restent dans MedicalRecord ; les requetes des rappels sont dans la classe RappelsDb.
- Le projet autonome fourni, sa connexion et ses scripts de creation de base ne sont pas executes ni copies comme nouvelle application.

## Fonctionnalites integrees

- Formulaire avec validation visuelle, telephone du proprietaire, rendez-vous et rappel vaccinal.
- Cartes avec score de risque explicable, veille sanitaire et liste de rappels enrichie.
- Recherche, filtres, statistiques, consultation du dossier, exports PDF et historique conserves.
- Les references animales existantes sont preservees lors de la modification.

## Compatibilite avec la base existante

Les noms reels sont `MEDICAL.ID_DOSS`, `RAF_ANIMAL`, `ID_EMP`, `POIDS` et `TELEPHONE`.
Les rappels utilisent les colonnes existantes `RAPPELS_VACCINS` et `RAPPELS_RDV`.
Les IDs restent generes par Oracle. Aucun changement de schema Oracle n'est requis pour cette integration.
Les anciens complements locaux sont toujours lus ; les champs complementaires non presents dans Oracle restent dans le stockage local existant.
Les ajouts de rappels Oracle utilisent une transaction et refusent un depassement de capacite sans tronquer l'historique.

## Verification

- Build Release et Debug reussis.
- Six scenarios + initialisation/nettoyage : 8 passes, 0 echec (CRUD Oracle des cinq tables, reference/poids medical, persistance, rappels, interfaces).
- Apres correction du placement du bouton Actions et des ressources : scenario UI repasse, 0 echec.
- Verifications architecture : formulaire unique et proprietaire des requetes SQL conformes.
- Captures : `build_tests/medical-captures/`.
- Rapports : `build_tests/medical-integration-tests.txt`, `build_tests/medical-final-ui-tests.txt`.
- Executable distribue : `ANIWORLD/ANIWORLD.exe` ; executable Qt Creator Debug reconstruit.

Les essais de rappels verifient la persistance sans envoyer de SMS.
