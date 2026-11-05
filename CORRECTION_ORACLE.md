# Correction "Ajout non effectué"

Cause : l'application écrivait dans des colonnes qui n'existent pas dans la vraie table MEDICAL
(ID_ANIMAL, SEXE, TRAITEMENT) et envoyait du texte ("MED-001") dans des colonnes NUMBER.
Oracle refusait l'INSERT (ORA-00904 / ORA-01722).

Maintenant : REF_ANIMAL, identifiants numériques (affichés MED-012 / ANI-005), MOTIF limité à 200 caractères,
SEXE/TRAITEMENT enregistrés seulement si les colonnes existent (ALTER optionnel dans schema.sql),
et l'erreur Oracle réelle est affichée dans la fenêtre en cas d'échec.
Reconstruire le projet (Rebuild All) avant de lancer.
