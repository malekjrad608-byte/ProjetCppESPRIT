-- ============================================================================
-- Jeu d'essai pour la démonstration de la Veille sanitaire.
-- Utilise UNIQUEMENT la table MEDICAL réelle (aucune table / colonne créée).
-- Dossiers 9001 à 9009 (affichés MED-9001...), animaux 901 et 911 à 915 (ANI-901...).
-- Prérequis : la table EMPLOYES contient au moins un employé (ID_EMPLOYES est NOT NULL).
-- Les dates sont relatives à SYSDATE : le script fonctionne tel quel à n'importe quelle date.
-- Résultat attendu dans l'application :
--   * Récidive : Rex - Otite, 3 passages en 70 jours            -> niveau ÉLEVÉ
--   * Foyer    : Gastro-entérite chez 4 chiens en 8 jours       -> niveau ATTENTION
--     (écritures différentes regroupées automatiquement)
--   * Décommenter le dossier 9009 -> le foyer passe à ÉLEVÉ (5 animaux)
-- ============================================================================

INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9001, TRUNC(SYSDATE) - 70, 'Oreille qui gratte', 'Otite', 901, 'Rex', 'Chien', 'Labrador', 4, 30, 'Propriétaire Test', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));
INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9002, TRUNC(SYSDATE) - 35, 'Secoue la tête', 'Otite externe', 901, 'Rex', 'Chien', 'Labrador', 4, 31, 'Propriétaire Test', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));
INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9003, TRUNC(SYSDATE) - 3, 'Oreille rouge', 'otite', 901, 'Rex', 'Chien', 'Labrador', 4, 31, 'Propriétaire Test', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));

INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9004, TRUNC(SYSDATE) - 9, 'Vomissements', 'Gastro-entérite', 911, 'Bella', 'Chien', 'Beagle', 3, 11, 'Propriétaire A', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));
INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9005, TRUNC(SYSDATE) - 7, 'Diarrhée', 'gastro enterite', 912, 'Max', 'Chien', 'Bulldog', 5, 24, 'Propriétaire B', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));
INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9006, TRUNC(SYSDATE) - 5, 'Selles liquides', 'Diarrhée', 913, 'Luna', 'Chien', 'Husky', 2, 20, 'Propriétaire C', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));
INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9007, TRUNC(SYSDATE) - 2, 'Ne mange plus', 'Vomissements', 914, 'Oscar', 'Chien', 'Croisé', 6, 18, 'Propriétaire D', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));

INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9008, TRUNC(SYSDATE) - 1, 'Contrôle annuel', 'Contrôle de routine', 901, 'Rex', 'Chien', 'Labrador', 4, 31, 'Propriétaire Test', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));

-- INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
-- VALUES (9009, TRUNC(SYSDATE) - 1, 'Gastrite', 'Gastrite', 915, 'Nala', 'Chien', 'Caniche', 1, 6, 'Propriétaire E', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));

COMMIT;

-- Nettoyage après la démonstration :
-- DELETE FROM MEDICAL WHERE ID_DOSSIER BETWEEN 9001 AND 9009;
-- COMMIT;
