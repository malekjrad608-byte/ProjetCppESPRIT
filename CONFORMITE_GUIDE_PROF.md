# Conformité au guide d'intégration (Soumaya Argoubi)

| Exigence du guide | Dans ce projet |
|---|---|
| Une classe `Connection` commune à toute l'équipe | `connection.h/.cpp` (singleton, DSN `Source_Projet2A18`, même utilisateur pour tous). Locaux et Dossiers médicaux l'utilisent sans en créer d'autre |
| Une seule fenêtre principale avec `QStackedWidget` | `mainwindow.cpp` : Employés 0 (Linda), Services 1, Stock 2, Locaux 3, Dossiers médicaux 4 |
| Une classe métier par étudiant, dans son propre dossier | Locaux (Emna) : `locaux/src/local.*`, `localrepository.*` ; Dossiers médicaux (Rawand) : `medicalrecord.*`, `vaccination.*`, `appointment.*`, `healthwatch.*` |
| Navigation dans la fenêtre principale uniquement | boutons du menu dans `mainwindow.cpp` |
| `.gitignore` Qt Creator | `.gitignore` (build/, *.user, moc_*, ui_*, qrc_*) |
| `git pull` avant `git push`, même branche | `integration-medicaux-employes-locale` |

Locaux : version la plus récente de la branche `gestion_local` (commit 46e682d).
