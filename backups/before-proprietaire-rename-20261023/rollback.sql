-- Utiliser uniquement avec la restauration de l'ancienne version Qt.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
ALTER TABLE medical_records RENAME COLUMN proprietaire TO owner;
BEGIN DBMS_UTILITY.COMPILE_SCHEMA(schema=>USER,compile_all=>FALSE); END;
/
