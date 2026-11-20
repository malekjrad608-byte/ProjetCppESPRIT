-- Same Column Sequence configuration as EMPLOYER, without duplicate generators.
-- Run in RAWAND. Keep the sequence names used by Qt, existing rows and all FKs.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET DEFINE OFF
SET SERVEROUTPUT ON
DECLARE
  v_next NUMBER;
  v_max NUMBER;
  v_increment NUMBER;
BEGIN
  SELECT NVL(MAX(id_serv),0) INTO v_max FROM service;
  SELECT increment_by INTO v_increment FROM all_sequences
    WHERE sequence_owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA') AND sequence_name='SERVICES_SEQ';
  SELECT services_seq.NEXTVAL INTO v_next FROM dual;
  IF v_increment<>1 OR v_next<=v_max THEN
    RAISE_APPLICATION_ERROR(-20001,'Unsafe sequence: services_seq; stop before changing triggers');
  END IF;
  SELECT NVL(MAX(id_local),0) INTO v_max FROM local;
  SELECT increment_by INTO v_increment FROM all_sequences
    WHERE sequence_owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA') AND sequence_name='LOCAL_SEQ';
  SELECT local_seq.NEXTVAL INTO v_next FROM dual;
  IF v_increment<>1 OR v_next<=v_max THEN
    RAISE_APPLICATION_ERROR(-20001,'Unsafe sequence: local_seq; stop before changing triggers');
  END IF;
  SELECT NVL(MAX(id_article),0) INTO v_max FROM stock;
  SELECT increment_by INTO v_increment FROM all_sequences
    WHERE sequence_owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA') AND sequence_name='STOCK_SEQ';
  SELECT stock_seq.NEXTVAL INTO v_next FROM dual;
  IF v_increment<>1 OR v_next<=v_max THEN
    RAISE_APPLICATION_ERROR(-20001,'Unsafe sequence: stock_seq; stop before changing triggers');
  END IF;
  SELECT NVL(MAX(id_doss),0) INTO v_max FROM medical;
  SELECT increment_by INTO v_increment FROM all_sequences
    WHERE sequence_owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA') AND sequence_name='MEDICAL_RECORDS_SEQ';
  SELECT medical_records_seq.NEXTVAL INTO v_next FROM dual;
  IF v_increment<>1 OR v_next<=v_max THEN
    RAISE_APPLICATION_ERROR(-20001,'Unsafe sequence: medical_records_seq; stop before changing triggers');
  END IF;
END;
/

CREATE OR REPLACE TRIGGER service_trg
BEFORE INSERT ON service
FOR EACH ROW
BEGIN
  <<COLUMN_SEQUENCES>>
  BEGIN
    IF INSERTING AND :NEW.id_serv IS NULL THEN
      SELECT services_seq.NEXTVAL INTO :NEW.id_serv FROM SYS.DUAL;
    END IF;
  END COLUMN_SEQUENCES;
END;
/
DECLARE
  v_errors NUMBER;
BEGIN
  SELECT COUNT(*) INTO v_errors FROM all_errors
    WHERE owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA')
      AND name='SERVICE_TRG' AND type='TRIGGER' AND attribute='ERROR';
  IF v_errors>0 THEN RAISE_APPLICATION_ERROR(-20002,'service_trg compilation failed'); END IF;
END;
/
ALTER TRIGGER service_trg ENABLE;
ALTER TRIGGER service_bi DISABLE;

CREATE OR REPLACE TRIGGER local_trg
BEFORE INSERT ON local
FOR EACH ROW
BEGIN
  <<COLUMN_SEQUENCES>>
  BEGIN
    IF INSERTING AND :NEW.id_local IS NULL THEN
      SELECT local_seq.NEXTVAL INTO :NEW.id_local FROM SYS.DUAL;
    END IF;
  END COLUMN_SEQUENCES;
END;
/
DECLARE
  v_errors NUMBER;
BEGIN
  SELECT COUNT(*) INTO v_errors FROM all_errors
    WHERE owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA')
      AND name='LOCAL_TRG' AND type='TRIGGER' AND attribute='ERROR';
  IF v_errors>0 THEN RAISE_APPLICATION_ERROR(-20002,'local_trg compilation failed'); END IF;
END;
/
ALTER TRIGGER local_trg ENABLE;
ALTER TRIGGER local_bi DISABLE;

CREATE OR REPLACE TRIGGER stock_trg
BEFORE INSERT ON stock
FOR EACH ROW
BEGIN
  <<COLUMN_SEQUENCES>>
  BEGIN
    IF INSERTING AND :NEW.id_article IS NULL THEN
      SELECT stock_seq.NEXTVAL INTO :NEW.id_article FROM SYS.DUAL;
    END IF;
  END COLUMN_SEQUENCES;
END;
/
DECLARE
  v_errors NUMBER;
BEGIN
  SELECT COUNT(*) INTO v_errors FROM all_errors
    WHERE owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA')
      AND name='STOCK_TRG' AND type='TRIGGER' AND attribute='ERROR';
  IF v_errors>0 THEN RAISE_APPLICATION_ERROR(-20002,'stock_trg compilation failed'); END IF;
END;
/
ALTER TRIGGER stock_trg ENABLE;
ALTER TRIGGER stock_bi DISABLE;

CREATE OR REPLACE TRIGGER medical_trg
BEFORE INSERT ON medical
FOR EACH ROW
BEGIN
  <<COLUMN_SEQUENCES>>
  BEGIN
    IF INSERTING AND :NEW.id_doss IS NULL THEN
      SELECT medical_records_seq.NEXTVAL INTO :NEW.id_doss FROM SYS.DUAL;
    END IF;
  END COLUMN_SEQUENCES;
END;
/
DECLARE
  v_errors NUMBER;
BEGIN
  SELECT COUNT(*) INTO v_errors FROM all_errors
    WHERE owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA')
      AND name='MEDICAL_TRG' AND type='TRIGGER' AND attribute='ERROR';
  IF v_errors>0 THEN RAISE_APPLICATION_ERROR(-20002,'medical_trg compilation failed'); END IF;
END;
/
ALTER TRIGGER medical_trg ENABLE;
ALTER TRIGGER medical_bi DISABLE;
