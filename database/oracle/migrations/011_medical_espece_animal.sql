-- Correction : ESPECE designe l'animal, RACE sa race, aucun paiement.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET DEFINE OFF
DECLARE n NUMBER; BEGIN
 SELECT COUNT(*) INTO n FROM medical WHERE espece IS NOT NULL;
 IF n>0 THEN RAISE_APPLICATION_ERROR(-20001,'Anciennes valeurs de paiement presentes : sauvegarde requise'); END IF;
END;
/
ALTER TABLE medical DROP CONSTRAINT ck_medical_paiement;
ALTER TABLE medical DROP COLUMN espece;
ALTER TABLE medical RENAME COLUMN type_animal TO espece;
CREATE OR REPLACE VIEW medical_records AS
SELECT id_doss id,id_animal animal_id,nom_animal name,espece species,race breed,
 age,sexe sex,proprietaire,date_consultation consultation_date,motif reason,
 diagnostic diagnosis,treatment,vaccinations,allergies,weight,observations,
 vaccination_status,id_emp id_employe FROM medical;
SELECT object_name,object_type FROM user_objects WHERE status<>'VALID';
