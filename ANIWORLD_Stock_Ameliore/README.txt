ANIWORLD — Gestion du stock

OUVERTURE DU PROJET
1. Ouvrez Qt Creator.
2. Choisissez « Open Project » et sélectionnez stockaniworld.pro.
3. Utilisez un kit Qt 6 Desktop compatible (Widgets, SQL, PrintSupport), puis compilez.
4. La connexion Oracle existante est configurée dans connection.cpp. Vérifiez le DSN ODBC, l'utilisateur et les droits du schéma avant de lancer l'application.

CONNEXION DE DÉMONSTRATION
Nom d'utilisateur : stock
Mot de passe : stock123

FONCTIONNALITÉS
- Interface desktop Qt Widgets avec le thème ANIWORLD turquoise/blanc conservé.
- Les boutons « Factures d'achat », « Alertes de stock » et « Réapprovisionnement » sont regroupés sous le tableau principal, et non plus dans la barre latérale gauche.
- Connexion à Oracle, recherche, filtres par catégorie/état, tri et CRUD des articles.
- Enregistrement des entrées/sorties de stock et statistiques.
- Suivi des lots périmés ou à échéance proche. Le retrait du stock disponible met désormais la quantité à zéro sans effacer l'enregistrement de traçabilité.
- Module Factures d'achat : ouverture PDF, extraction directe du texte via Poppler, OCR via Tesseract pour les scans, lignes détectées modifiables, validation transactionnelle, mise à jour du stock, lien fournisseur, historique de factures et historique de prix.
- Alertes de seuil minimum / stock critique et péremption à 30 jours, avec préparation d'un e-mail destiné au dépôt/local ou au fournisseur dans le client mail configuré.
- Réapprovisionnement indicatif selon le stock, le seuil minimum, le délai fournisseur, une couverture cible et le stock de sécurité. Sans historique des sorties disponible, le module signale que l'estimation repose sur le seuil minimum.
- Génération d'un bon de commande PDF au format A4, dans la même identité visuelle ANIWORLD.

STOCKAGE DES NOUVEAUX MODULES
- Aucun nouveau tableau Oracle / fichier SQL n'est nécessaire pour les fonctionnalités ajoutées.
- Les fournisseurs et l'historique des factures/prix sont stockés localement par Qt dans le dossier de données utilisateur de l'application (aniworld_features.ini).
- La validation d'une facture utilise uniquement la table STOCK déjà présente pour ajouter ou mettre à jour les articles et quantités.
- Les fournisseurs sont à ajouter depuis « Factures d'achat » > « Gérer les fournisseurs » avant d'importer une facture.

DÉPENDANCES POUR LA LECTURE DES PDF
- Poppler doit être installé et pdftotext / pdftoppm accessibles via PATH.
- Tesseract doit être installé et accessible via PATH. Pour les factures scannées, les données de langue fra et eng sont recommandées.
- L'extraction automatique dépend de la mise en page du fournisseur. Toujours contrôler/modifier les lignes extraites avant de valider la facture. Si rien n'est reconnu, vous pouvez saisir les lignes manuellement dans le tableau.

LIMITES / NOTES DE DONNÉES
- Le stock courant est enregistré dans la table Oracle STOCK existante. Les métadonnées d'achat et fournisseurs sont enregistrées localement par Qt, sans créer de tables Oracle.
- Les sorties enregistrées après cette version sont historisées localement par Qt. L'historique ancien n'est pas disponible rétroactivement ; sans sorties récentes, le module de réapprovisionnement se base sur les seuils minimums.
- Le bon de commande PDF est généré localement. L'application prépare les e-mails via le client mail de l'ordinateur, elle n'envoie pas automatiquement sans configuration SMTP dédiée.
