-- Run as RAWAND, or after ALTER SESSION SET CURRENT_SCHEMA=RAWAND.
-- SQL Developer: Identity Column > Column Sequence.
-- Preserve EMPLOYES_SEQ: Qt reads its CURRVAL after inserting an employee.
-- EMPLOYER_TRG is the only enabled ID generator; keep the older trigger
-- disabled for recovery. Do not reset either sequence or change existing IDs.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET DEFINE OFF
SET SERVEROUTPUT ON

DECLARE
  v_next NUMBER;
  v_max NUMBER;
  v_increment NUMBER;
BEGIN
  SELECT MAX(id_emp) INTO v_max FROM employer;
  SELECT increment_by INTO v_increment FROM all_sequences
    WHERE sequence_owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA')
      AND sequence_name='EMPLOYES_SEQ';
  IF v_increment<>1 THEN
    RAISE_APPLICATION_ERROR(-20001,'EMPLOYES_SEQ must increment by one');
  END IF;
  SELECT employes_seq.NEXTVAL INTO v_next FROM dual;
  IF v_next<=NVL(v_max,0) THEN
    RAISE_APPLICATION_ERROR(-20002,'Sequence is behind existing IDs; stop before changing triggers');
  END IF;
END;
/

CREATE OR REPLACE TRIGGER employer_trg
BEFORE INSERT ON employer
FOR EACH ROW
BEGIN
  <<COLUMN_SEQUENCES>>
  BEGIN
    IF INSERTING AND :NEW.id_emp IS NULL THEN
      SELECT employes_seq.NEXTVAL INTO :NEW.id_emp FROM SYS.DUAL;
    END IF;
  END COLUMN_SEQUENCES;
END;
/

DECLARE
  v_errors NUMBER;
BEGIN
  SELECT COUNT(*) INTO v_errors FROM all_errors
    WHERE owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA')
      AND name='EMPLOYER_TRG' AND type='TRIGGER' AND attribute='ERROR';
  IF v_errors>0 THEN
    RAISE_APPLICATION_ERROR(-20003,'EMPLOYER_TRG compilation failed');
  END IF;
END;
/
ALTER TRIGGER employer_trg ENABLE;
ALTER TRIGGER employer_bi DISABLE;
