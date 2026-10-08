ANIWORLD2 — Integration Employés + Locaux + Services

Modules intégrés dans un seul projet qmake :
- Employés
- Locaux
- Services + Tableau de bord Services

Connexion :
- LINDA / LINDA123 -> ouvre Employés
- EMNA / EMNA123 -> ouvre Locaux
- ADMIN / admin123 -> ouvre Services

Chaque compte arrive directement dans son module au démarrage. La barre latérale reste disponible pour naviguer entre les autres modules.

Le module Services vient du projet gestion_service fourni et comprend :
- création / consultation / modification / suppression des services
- recherche, filtres et tri
- vues cartes et liste
- suivi Planifié -> En cours -> Terminé
- duplication
- statistiques
- export CSV
- export PDF
- tableau de bord avec indicateurs et services du jour
- animations du module Services

Stockage :
- Employés / Locaux : aniworld.db
- Services : SQLite séparée dans le dossier AppLocalData d'ANIWORLD, services.sqlite

Compilation sous Qt Creator :
1. Ouvrir ANIWORLD2.pro
2. Choisir un kit Qt 6.x + MinGW 64 bits compatible
3. Exécuter qmake / CMake selon l'environnement configuré
4. Compiler puis lancer ANIWORLD2


LOGIN COMPTES
- Employés : LINDA / LINDA123
- Locaux   : EMNA / EMNA123
- Services : admin / admin123
- Stock    : SOFIEN / SOF123

MODULE DOSSIERS MÉDICAUX (interface d'origine, ajouté)
- Dossier : dossiers/ (dossierspage.* + dossierspage.ui, recordcard, medicalrecord,
  medicalrecorddialog, medicalrecorddetailsdialog, dossiers.qss)
- Accès : bouton « Dossiers médicaux » de la barre latérale (page directe, sans identifiant)
  ou compte rawand.k / rawand123 au démarrage.
- Le style du module est appliqué uniquement à cette page (n'affecte pas les autres modules).
- Les dossiers sont ceux de l'interface d'origine (données en mémoire, 8 exemples).
