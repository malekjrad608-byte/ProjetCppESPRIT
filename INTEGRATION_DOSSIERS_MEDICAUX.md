# Intégration : Dossiers médicaux dans ANIWORLD2 (projet de Linda)

Base du projet : ANIWORLD2 (Employés). Le module « Dossiers médicaux » est la page du menu latéral « Dossiers médicaux ».

## Ce qui a changé
- `mainwindow.cpp/.h` (Linda) : la page « Dossiers médicaux » ouvre le module (créé au premier clic).
- `main.cpp` : ajout de setApplicationName / setOrganizationName("ANIWORLD") (réglages SMS et alertes).
- `ANIWORLD2.pro` : fichiers du module, `QT += svg network`, `medicalwindow.ui`, `medical.qrc`.
- Ancienne fenêtre médicale `mainwindow.*` renommée `medicalwindow.*` (classe `MedicalWindow`), sans menu latéral,
  sans bouton de déconnexion ni badge Oracle (ils existent déjà dans la fenêtre principale). Son style reste limité à cette page.
- Ressources du module sous le préfixe `:/med/` (`medical.qrc`) pour ne pas entrer en conflit avec `resources.qrc`.
- Connexion Oracle : celle de Linda (`connection.cpp`) ; l'ancienne `Connection` du module n'est plus utilisée.
- Écran de connexion du module (`logindialog`, `loginvisuals`) non inclus.
- `lookupdata.cpp` : la liste « Employé » du formulaire lit la table EMPLOYES du module Employés (colonnes ID, NOM, PRENOM).
  Les employés créés dans « Gestion des employés » apparaissent directement, sous la forme « NOM PRENOM ».

## Ce que le module n'a PAS changé
Aucune modification du module Employés (employe.*, gestionemployes.*, planning.*, connection.*), ni de la base.

## Points à vérifier au premier lancement
1. Le badge Oracle doit afficher « Oracle connecté ». Sinon lire le message d'erreur : le code de Linda exige une colonne `ID`
   dans la table EMPLOYES (une ancienne table avec `ID_EMPLOYES` est refusée).
2. Table MEDICAL : `ID_EMPLOYES` doit contenir l'`ID` d'un employé existant (choisi dans la liste).
3. Tables VACCINATION et RENDEZVOUS : à créer avec `schema.sql` pour les rappels et rendez-vous.
