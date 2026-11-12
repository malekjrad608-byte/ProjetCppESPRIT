-- =====================================================================
--  ANIWORLD - Integration Employes <-> Dossiers medicaux
--  A executer UNE SEULE FOIS, connecte avec l'utilisateur rawand
--  (SQL Developer : ouvrir ce fichier puis F5).
--
--  Ne cree, ne supprime et ne renomme AUCUNE table.
--  Seule la cle etrangere FK_05 de MEDICAL est redirigee :
--     avant : MEDICAL.ID_EMPLOYES -> EMPLOYES (ancienne table)
--     apres : MEDICAL.ID_EMPLOYES -> EMPLOYER(ID_EMP)  (table du module Employes)
-- =====================================================================

-- 1) Ou pointe FK_05 aujourd'hui ? (lecture seule)
SELECT c.constraint_name, c.table_name, rc.table_name AS table_referencee
FROM user_constraints c
JOIN user_constraints rc ON rc.constraint_name = c.r_constraint_name
WHERE c.constraint_name = 'FK_05';

-- 2) Redirection de FK_05 vers EMPLOYER(ID_EMP)
ALTER TABLE MEDICAL DROP CONSTRAINT FK_05;

-- ENABLE NOVALIDATE : les dossiers deja saisis ne sont pas rejetes,
-- mais tout NOUVEAU dossier doit avoir un employe present dans EMPLOYER.
ALTER TABLE MEDICAL ADD CONSTRAINT FK_05
    FOREIGN KEY (ID_EMPLOYES) REFERENCES EMPLOYER (ID_EMP)
    ENABLE NOVALIDATE;

COMMIT;

-- 3) Verification : la table referencee doit etre EMPLOYER
SELECT c.constraint_name, c.table_name, rc.table_name AS table_referencee
FROM user_constraints c
JOIN user_constraints rc ON rc.constraint_name = c.r_constraint_name
WHERE c.constraint_name = 'FK_05';
