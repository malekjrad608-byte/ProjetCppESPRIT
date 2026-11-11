-- ============================================================================
-- Jeu d'essai Veille sanitaire SANS modifier la structure Oracle (la contrainte UNIDM reste en place).
-- Un seul dossier par animal (911 à 914) : 4 chiens avec une gastro-entérite en 8 jours -> FOYER (niveau ATTENTION).
-- Pour passer à ÉLEVÉ (5 animaux) : ajouter un 5e chien avec REF_ANIMAL 915.
-- La détection de RÉCIDIVE (même animal, plusieurs passages) nécessite plusieurs dossiers pour un animal :
-- impossible tant que UNIDM existe -> à démontrer seulement si l'équipe valide son retrait.
-- ============================================================================
INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9004, TRUNC(SYSDATE) - 9, 'Vomissements', 'Gastro-entérite', 911, 'Bella', 'Chien', 'Beagle', 3, 11, 'Propriétaire A', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));
INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9005, TRUNC(SYSDATE) - 7, 'Diarrhée', 'gastro enterite', 912, 'Max', 'Chien', 'Bulldog', 5, 24, 'Propriétaire B', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));
INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9006, TRUNC(SYSDATE) - 5, 'Selles liquides', 'Diarrhée', 913, 'Luna', 'Chien', 'Husky', 2, 20, 'Propriétaire C', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));
INSERT INTO MEDICAL (ID_DOSSIER, DATE_CONSULTATION, MOTIF, DIAGNOSTIC, REF_ANIMAL, NOM_ANIMAL, ESPECE, RACE, AGE, POIDS, PROPRIETAIRE, ID_EMPLOYES)
VALUES (9007, TRUNC(SYSDATE) - 2, 'Ne mange plus', 'Vomissements', 914, 'Oscar', 'Chien', 'Croisé', 6, 18, 'Propriétaire D', (SELECT MIN(ID_EMPLOYES) FROM EMPLOYES));

COMMIT;

-- Nettoyage après la démonstration :
-- DELETE FROM MEDICAL WHERE ID_DOSSIER BETWEEN 9004 AND 9007;
-- COMMIT;
