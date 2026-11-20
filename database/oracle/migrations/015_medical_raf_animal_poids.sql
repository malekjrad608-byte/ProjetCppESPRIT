-- Run on RAWAND. Renaming the column preserves the existing unique constraint.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
ALTER TABLE RAWAND.MEDICAL ADD (POIDS NUMBER);
ALTER TABLE RAWAND.MEDICAL RENAME COLUMN ID_ANIMAL TO RAF_ANIMAL;

CREATE OR REPLACE NONEDITIONABLE VIEW RAWAND.MEDICAL_RECORDS AS
SELECT id_doss AS id, raf_animal AS animal_id, nom_animal AS name,
       espece AS species, race AS breed, age, CAST(NULL AS VARCHAR2(20 CHAR)) AS sex,
       proprietaire, date_consultation AS consultation_date, motif AS reason,
       diagnostic AS diagnosis, CAST(NULL AS VARCHAR2(4000)) AS treatment,
       CAST(NULL AS VARCHAR2(4000)) AS vaccinations, CAST(NULL AS VARCHAR2(4000)) AS allergies,
       poids AS weight, CAST(NULL AS VARCHAR2(4000)) AS observations,
       CAST(NULL AS VARCHAR2(30)) AS vaccination_status, id_emp AS id_employe
FROM RAWAND.MEDICAL;

ALTER TABLE RAWAND.MEDICAL DROP COLUMN SEXE;
