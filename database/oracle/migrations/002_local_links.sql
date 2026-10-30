-- Additive, rerunnable migration. Existing rows retain a NULL room assignment.
-- Run in ANIWORLD_APP (or with CURRENT_SCHEMA set to ANIWORLD_APP).
-- No data deletion, no invented assignments, no cascading deletion.
whenever sqlerror exit failure rollback
set define off
DECLARE
  n NUMBER;
  PROCEDURE link_room(t VARCHAR2, fk VARCHAR2, ix VARCHAR2) IS
  BEGIN
    SELECT COUNT(*) INTO n FROM all_tab_columns
      WHERE owner='ANIWORLD_APP' AND table_name=t AND column_name='ID_LOCAL';
    IF n=0 THEN EXECUTE IMMEDIATE 'ALTER TABLE ANIWORLD_APP.'||t||' ADD (id_local NUMBER(10))'; END IF;
    SELECT COUNT(*) INTO n FROM all_constraints WHERE owner='ANIWORLD_APP' AND constraint_name=fk;
    IF n=0 THEN EXECUTE IMMEDIATE 'ALTER TABLE ANIWORLD_APP.'||t||' ADD CONSTRAINT '||fk||' FOREIGN KEY(id_local) REFERENCES ANIWORLD_APP.LOCAUX(id)'; END IF;
    SELECT COUNT(*) INTO n FROM all_indexes WHERE owner='ANIWORLD_APP' AND index_name=ix;
    IF n=0 THEN EXECUTE IMMEDIATE 'CREATE INDEX ANIWORLD_APP.'||ix||' ON ANIWORLD_APP.'||t||'(id_local)'; END IF;
  END;
BEGIN
  IF SYS_CONTEXT('USERENV','CON_NAME') <> 'XEPDB1' OR SYS_CONTEXT('USERENV','CURRENT_SCHEMA') <> 'ANIWORLD_APP' THEN
    RAISE_APPLICATION_ERROR(-20001,'Expected ANIWORLD_APP in XEPDB1');
  END IF;
  link_room('SERVICES','FK_SERVICES_LOCAL','IX_SERVICES_LOCAL');
  link_room('MEDICAL_RECORDS','FK_MEDICAL_LOCAL','IX_MEDICAL_LOCAL');
  link_room('STOCK_LOTS','FK_STOCK_LOCAL','IX_STOCK_LOCAL');
  MERGE INTO ANIWORLD_APP.integration_migrations d USING (SELECT 'local-links-v2' name FROM dual) s
    ON (d.name=s.name) WHEN NOT MATCHED THEN INSERT(name) VALUES(s.name);
END;
/
COMMIT;
