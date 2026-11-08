# Rapport de vérification (analyse statique uniquement : le projet n'a PAS été compilé ni exécuté)

## Fait
- Une seule classe `Connection` (`connection.h/.cpp`). Aucun autre `addDatabase`, `setDatabaseName`, `setUserName`, `setPassword` ni DSN dans le projet.
- Employés (Linda), Locaux (Emna) et Dossiers médicaux (Rawand) utilisent `Connection::createInstance()` ou la connexion Qt par défaut ouverte par elle.
- Liste « Employé » du dossier médical : lue dans la table réellement référencée par la clé étrangère `FK_05` (dictionnaire Oracle `USER_CONSTRAINTS`), donc plus d'`ORA-02291` à cause du choix de l'employé. Aucune table, contrainte ni donnée Oracle modifiée.
- Contrôles par script : toutes les inclusions `#include "..."` existent ; tous les `.cpp/.h/.ui/.qrc` sont dans `ANIWORLD2.pro` et inversement ; tous les fichiers des `.qrc` existent ; aucune classe définie deux fois ; tous les widgets `ui->xxx` de `mainwindow.cpp` et `medicalwindow.cpp` existent dans `mainwindow.ui` ; tous les slots déclarés sont définis ; les `&Classe::membre` des `connect()` existent.
- `.gitignore` : contient toutes les règles du guide (Qt Creator, build/debug/release/bin/obj, .log/.tmp/.bak/.swp/.DS_Store, Makefile*, *.o/*.a/*.so/*.dll/*.exe) plus moc_*, ui_*.h, qrc_*.cpp.

## Décisions de l'équipe nécessaires (rien n'a été modifié sans accord)
1. ~~Un seul fichier `.ui`~~ : **fait** (voir ci-dessous), mais non compilé : à valider par un Rebuild.
2. **Employés : `EMPLOYER` (Linda, `ID_EMP`) ou `EMPLOYES` (clé `FK_05`, `ID_EMPLOYES`)** : deux tables différentes. Options : (a) l'équipe garde `EMPLOYES` comme table commune et Linda s'y adapte ; (b) Linda garde `EMPLOYER` et `FK_05` est recréée vers `EMPLOYER(ID_EMP)` : `ALTER TABLE MEDICAL DROP CONSTRAINT FK_05;` puis `ALTER TABLE MEDICAL ADD CONSTRAINT FK_05 FOREIGN KEY (ID_EMPLOYES) REFERENCES EMPLOYER(ID_EMP);` (les valeurs déjà saisies dans `MEDICAL.ID_EMPLOYES` doivent exister dans `EMPLOYER`). Tant qu'aucune décision n'est prise, le formulaire n'affiche que les employés de la table de `FK_05`.

## Fichier .ui unique (mainwindow.ui)
- `mainwindow.ui` contient `stackedWidget` avec 5 pages : `pageEmployes`, `pageServices`, `pageStock`, `pageLocaux`, `pageDossiers` (ordre 0 à 4, inchangé).
- `pageDossiers` contient l'ancien arbre de `medicalwindow.ui` (racine renommée `medicalCentral`, tous les autres noms de widgets inchangés). Ancien fichier conservé, non compilé : `docs/ancien_ui/medicalwindow_ancien.ui`.
- `MainWindow` utilise `Ui::MainWindow` ; le menu latéral et la bande du haut de Linda restent construits en code et sont insérés dans `layoutPrincipal` / `layoutZone`.
- `MedicalWindow` n'est plus une `QMainWindow` avec son propre `.ui` : c'est un `QWidget` qui reçoit `Ui::MainWindow*` et place `medicalCentral` dans son layout. Ses 131 usages de `ui->` sont inchangés. Même chose pour la mise en page manuelle (`layoutUi`, `resizeEvent`).
- Employés (Linda) et Locaux (Emna) : leurs pages sont ajoutées dans `layoutPageEmployes` et `layoutPageLocaux`, leur code n'a pas changé.
