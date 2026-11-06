# Intégration du nouveau Stock

Source fournie : `ANIWORLD_Stock_Ameliore` (dossier Downloads/ProjetCppESPRIT-stock-crud/ProjetCppESPRIT-stock-crud).

Le module remplace la page Stock du projet intégré. Il utilise la navigation et la connexion Oracle communes. Aucun second login, aucune connexion C##RAWAND et aucune nouvelle table ne sont ajoutés.

## Fichiers

- `src/stock/stockwidget.cpp/.h` : page intégrée, filtres, CRUD, mouvements, statistiques, péremption, accès aux nouveaux modules.
- `src/stock/stock.cpp/.h` : classe métier Stock encapsulée et réception des lignes de facture.
- `src/stock/stockrecord.cpp/.h` : persistance Oracle commune, requêtes préparées, verrou et contrôle de concurrence, transaction.
- `src/stock/inventoryfeatures.cpp/.h` : fournisseurs, import PDF contrôlé, achats/prix, alertes, bon de commande PDF.
- `src/stock/stockjournal.cpp/.h` : journal local des mouvements validés.
- `sources.pri` : compilation du module et Qt PrintSupport.

## Compatibilité Oracle

Le module utilise STOCK(ID_ARTICLE, ARTICLE, NOM_ARTICLE, LOT, QUANTITE_DISP, TYPE_ARTICLE, SEUIL_MIN, DATE_ECHEANCE, ID_LOCAL).
ID_ARTICLE est généré par Oracle ; ARTICLE est unique et ID_LOCAL référence LOCAL. Les anciennes colonnes UNITS/DATE_CHEANCE du projet source ne sont pas utilisées.

Une réception exige un local, une référence article, une quantité entière positive et une date valide. Une référence existante doit correspondre au même lot, local et échéance. Toutes les lignes sont enregistrées dans une transaction ; un échec annule la réception. Une erreur de validation conserve le formulaire ouvert. La validation de factures est réservée au mode Oracle.

Le retrait pour péremption conserve l'article avec quantité zéro. Le remplacement crée une nouvelle référence/ID et conserve l'ancien lot à zéro.

## Nouveautés du module fourni

- Factures PDF : extraction du texte via Qt PDF, contrôle/modification des lignes, fournisseur, date, numéro et local avant validation.
- Fournisseurs, historique achats/prix, détection locale des numéros de facture déjà validés.
- Alertes stock/péremption et préparation d'un e-mail dans le client de messagerie (envoi par l'utilisateur).
- Réapprovisionnement selon les sorties locales, le délai fournisseur, la couverture et le seuil minimum ; génération d'un bon PDF.

Fournisseurs, factures/prix et mouvements sont conservés dans `aniworld_features.ini` du dossier de données Qt de l'application ; ils ne sont pas synchronisés entre postes. Le mode démo utilise un fichier séparé. Ce journal local n'est pas une transaction distribuée avec Oracle. Sauvegarder ce fichier avec les données de l'application.

Les PDF texte fonctionnent sans outil externe. Pour les PDF scannés, le code source utilise Poppler (pdftoppm) et Tesseract avec les langues fra/eng dans PATH. Tesseract n'est pas présent dans PATH sur ce poste : la saisie manuelle des lignes reste disponible si l'OCR échoue.

## Guide d'intégration

Classe métier séparée, SQL dans les modèles, connexion commune, navigation centrale, noms de widgets explicites, zones Stock et .gitignore Qt sont appliqués. L'application existante conserve ses vues C++ et fichiers .ui actuels. La conversion des cinq interfaces vers un seul .ui n'est pas réalisée dans ce remplacement du Stock. Aucun push Git n'est effectué.

## Vérification

Compilation Release et tests Qt : navigation intégrée, taille réduite, CRUD, mouvements, péremption, nouveaux dialogues, CRUD Oracle des cinq tables, réception et clés étrangères. Les scripts tools/check_workshop_architecture.py et tools/check_model_sql_ownership.py vérifient l'absence de SQL dans les vues.

Une sauvegarde des sources et de l'exécutable précédent est dans backups/before-new-stock-20261106-085510.
