-- Renommage demande : conserver les valeurs et les relations existantes.
-- Appliquer avec la version Qt qui utilise PROPRIETAIRE pour Oracle.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET SERVEROUTPUT ON
DECLARE
  ancien NUMBER;
  nouveau NUMBER;
BEGIN
  IF USER<>'RAWAND' THEN RAISE_APPLICATION_ERROR(-20001,'Schema RAWAND requis'); END IF;
  SELECT COUNT(*) INTO ancien FROM user_tab_columns WHERE table_name='MEDICAL_RECORDS' AND column_name='OWNER';
  SELECT COUNT(*) INTO nouveau FROM user_tab_columns WHERE table_name='MEDICAL_RECORDS' AND column_name='PROPRIETAIRE';
  IF ancien=1 AND nouveau=0 THEN
    EXECUTE IMMEDIATE 'ALTER TABLE medical_records RENAME COLUMN owner TO proprietaire';
    DBMS_OUTPUT.PUT_LINE('OWNER renomme en PROPRIETAIRE ; valeurs conservees.');
  ELSIF ancien=0 AND nouveau=1 THEN
    DBMS_OUTPUT.PUT_LINE('PROPRIETAIRE existe deja.');
  ELSE
    RAISE_APPLICATION_ERROR(-20002,'Colonnes inattendues : aucun renommage effectue');
  END IF;
END;
/
BEGIN DBMS_UTILITY.COMPILE_SCHEMA(schema=>USER,compile_all=>FALSE); END;
/
SELECT column_name,data_type,nullable FROM user_tab_columns
WHERE table_name='MEDICAL_RECORDS' AND column_name IN ('OWNER','PROPRIETAIRE');
