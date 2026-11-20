-- Appliquer apres sauvegarde Data Pump et fermeture de l'application.
-- MLD : EMPLOYES -> SERVICES ; LOCAUX -> SERVICES ;
--       STOCK_LOTS -> LOCAUX ; MEDICAL_RECORDS -> EMPLOYES.
-- Les affectations existantes sont conservees dans EMPLOYES.ID_SERVICE.
whenever sqlerror exit failure rollback
set define off
DECLARE n NUMBER;
BEGIN
  IF USER <> 'RAWAND' THEN RAISE_APPLICATION_ERROR(-20001,'Schema RAWAND attendu'); END IF;
  SELECT COUNT(*) INTO n FROM services s WHERE NOT EXISTS
    (SELECT 1 FROM employes e WHERE e.id=s.id_employe AND e.id_service=s.id);
  IF n<>0 THEN RAISE_APPLICATION_ERROR(-20002,'Affectations incoherentes : migration arretee'); END IF;
END;
/
ALTER TABLE services DROP CONSTRAINT fk_service_team;
ALTER TABLE employes DROP CONSTRAINT uq_employee_team;
ALTER TABLE services DROP COLUMN id_employe;
BEGIN DBMS_UTILITY.COMPILE_SCHEMA(schema=>USER,compile_all=>FALSE); END;
/
-- Les quatre autres FK et leurs colonnes NOT NULL restent en place.
SELECT table_name,constraint_name,status,validated FROM user_constraints
WHERE constraint_type='R' ORDER BY table_name;
SELECT object_name,object_type FROM user_objects WHERE status='INVALID';
