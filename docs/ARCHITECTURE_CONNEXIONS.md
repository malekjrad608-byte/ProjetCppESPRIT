# Connexion Oracle : classe `Connection` commune (selon le guide d'intégration)

* Une seule classe `Connection` (`connection.h/.cpp`), fichiers de Linda **non modifiés**, connexion Qt par défaut.
* Mêmes paramètres pour toute l'équipe : DSN ODBC `Source_Projet2A18`, utilisateur `rawand`.
* `main.cpp` ouvre la connexion au démarrage ; tous les modules (Employés, Dossiers médicaux, Locaux) l'utilisent.
* Chaque membre garde SA classe métier : Employés (`employe.*`, `planning.*`), Dossiers médicaux (`medicalrecord.*`,
  `vaccination.*`, `appointment.*`, `lookupdata.*`), Locaux (`locaux/src/local.*`).
* Navigation (menu, retour) : dans `mainwindow.cpp`, pas dans les modules.
* `.gitignore` : fichiers Qt Creator, dossiers de compilation, fichiers générés ne sont pas versionnés.

## Git (même branche pour tout le projet)
    git pull origin <branche>      # toujours avant de commencer et avant de pusher
    git add <fichiers modifiés>
    git commit -m "message clair"
    git pull origin <branche>      # récupérer les changements des autres
    git push origin <branche>
