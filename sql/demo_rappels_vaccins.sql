-- Jeu d'essai Rappels vaccinaux : AUCUNE nouvelle table. Les vaccins sont dans MEDICAL.RAPPELS_VACCINS
-- (colonne créée automatiquement par l'application au 1er lancement ; sinon exécuter la ligne ALTER ci-dessous).
-- ALTER TABLE MEDICAL ADD (RAPPELS_VACCINS VARCHAR2(2000));
-- Chaque animal reçoit 2 vaccins : un EN RETARD (rappel il y a 15 jours) et un À RENOUVELER (dans 10 jours).
UPDATE MEDICAL
SET RAPPELS_VACCINS = 'Rage|' || TO_CHAR(TRUNC(SYSDATE) - 380, 'YYYY-MM-DD') || '|' || TO_CHAR(TRUNC(SYSDATE) - 15, 'YYYY-MM-DD')
                    || '|~Leptospirose|' || TO_CHAR(TRUNC(SYSDATE) - 355, 'YYYY-MM-DD') || '|' || TO_CHAR(TRUNC(SYSDATE) + 10, 'YYYY-MM-DD') || '|'
WHERE REF_ANIMAL IS NOT NULL;
COMMIT;
-- Annuler : UPDATE MEDICAL SET RAPPELS_VACCINS = NULL; COMMIT;
