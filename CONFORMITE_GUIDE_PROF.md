# Conformité au guide d'intégration (Soumaya Argoubi)

Architecture (même modèle que l'exemple SmartMarket du guide) :

```
ANIWORLD2/
├── mainwindow.h / mainwindow.cpp   → fenêtre principale : navigation + zones « // === MODULE XXX === »
├── connection.h / connection.cpp   → classe de connexion commune à la base (une seule)
├── employe.h / employe.cpp         → classe métier Employé (Linda)
├── local.h / local.cpp             → classe métier Local (Emna)
├── medicalrecord.h / .cpp          → classe métier Dossier médical (Rawand)
├── ANIWORLD2.pro                   → fichier de configuration Qt (une zone par module)
├── resources/                      → icônes, images, styles
├── sql/  docs/                     → scripts SQL et documentation
└── .gitignore
```

| Exigence du guide | Dans ce projet |
|---|---|
| Classe `Connection` commune | `connection.h/.cpp` seule (DSN `Source_Projet2A18`) ; aucun autre `addDatabase` |
| Fenêtre principale + `QStackedWidget` | `mainwindow.*` : Employés 0, Services 1, Stock 2, Locaux 3, Dossiers médicaux 4 |
| Navigation dans la fenêtre principale | slots de navigation dans `mainwindow.cpp` (zone SLOTS GÉNÉRAUX) |
| Zones par module | `// === MODULE EMPLOYE ===`, `// === MODULE LOCAUX ===`, `// === MODULE DOSSIERS MEDICAUX ===` |
| Une classe métier par étudiant | employe / local / medicalrecord (fichiers à plat) |
| `.gitignore` Qt Creator | `.gitignore` |
| Même branche, `git pull` avant `git push` | `integration-local-dossiermedical` |

## Points non conformes à ce jour
- **Un seul `.ui`** : fait (`mainwindow.ui`, `stackedWidget` à 5 pages), à valider par compilation.
- **Employés** : `EMPLOYER` (Linda) ≠ `EMPLOYES` (clé `FK_05`) : décision d'équipe n° 2.
