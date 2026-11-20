# Intégration Employés v3 — 17 novembre 2026

Source : `C:\Users\malek\Downloads\Employer_corrige_v3\Employer\ANIWORLD2`.
Projet intégré : `C:\Users\malek\Documents\gestion_service\gestion_service.pro`.

## Adaptation au projet commun

- Formulaire Oracle v3 avec identité, CIN, naissance, adresse, téléphone, courriel, contrat, salaire et service.
- Classe métier `src/employees/employe.h/.cpp` : validation et accès aux données ; connexion commune conservée.
- Détails complets dans la page Employés du formulaire partagé `src/mainwindow.ui`.
- Évaluation v3 : sauvegarde automatique locale via `EmployeeEvaluation` et `ModuleSupplements`, lecture des anciennes évaluations conservée.
- Planning, statistiques, attestation de congé PDF et droits du projet intégré conservés.
- Le formulaire simplifié du mode démonstration SQLite reste disponible ; le formulaire v3 utilise le schéma Oracle existant.
- Aucun changement du schéma Oracle et aucune importation de la connexion ou du programme principal du module autonome.

## Contrôles demandés

- Toutes les zones restent neutres à l'ouverture. Après saisie : bordure verte et ✓ si valide, rouge et ✕ avec explication sinon.
- Le bouton Ajouter reste désactivé tant que les données obligatoires ne sont pas valides.
- Charge strictement supérieure à zéro, maximum 60 heures par semaine ; vérification aussi dans le modèle avant écriture.
- CIN à huit chiffres et non dupliqué, dates cohérentes, téléphone tunisien à huit chiffres, contrat et service obligatoires.
- Salaire décimal conservé à l'ajout et à la modification.
- Courriel : format Gmail selon le module v3 et recherche des doublons Oracle, contrôlée à nouveau à l'enregistrement. Cette validation ne confirme pas l'existence de la boîte mail. Aucun service externe ne reçoit les adresses.

## Vérifications

- Compilation Release et Debug : réussie.
- Tests Oracle : ajout via le formulaire, modification, conservation du service et du salaire, rejet de la charge zéro, doublons CIN/courriel.
- Tests de validation, persistance des évaluations, navigation, planning et CRUD intégrés : réussis.
- Dernier rapport : `build_tests/employer-v3-final-tests.txt` (7 réussites, 0 échec).
- Vérification SQL dans les modèles et architecture de l'atelier : réussie.
- Captures Windows : `build_tests/employer-v3-windows/` (neutre, invalide, valide et écrans intégrés).
- Les données temporaires des tests Oracle sont supprimées à la fin de chaque test.

Exécutable distribué : `ANIWORLD/ANIWORLD.exe`.
