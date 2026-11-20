-- Une seule sequence pour l'identifiant du service.
-- SERVICES_BI genere ID avec SERVICES_SEQ et conserve la numerotation facture.
-- Le trigger genere par SQL Developer utilisait une autre sequence (SERVICES_SEQ1).
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET SERVEROUTPUT ON
DECLARE
  n NUMBER;
BEGIN
  IF USER<>'RAWAND' THEN RAISE_APPLICATION_ERROR(-20001,'Schema RAWAND requis'); END IF;
  SELECT COUNT(*) INTO n FROM user_triggers t JOIN user_objects o
    ON o.object_name=t.trigger_name AND o.object_type='TRIGGER'
    WHERE t.trigger_name='SERVICES_BI' AND t.table_name='SERVICES'
      AND t.status='ENABLED' AND o.status='VALID';
  IF n<>1 THEN RAISE_APPLICATION_ERROR(-20002,'Le trigger principal SERVICES_BI doit etre valide et actif'); END IF;
  SELECT COUNT(*) INTO n FROM user_triggers WHERE trigger_name='SERVICES_TRG' AND table_name='SERVICES';
  IF n=1 THEN EXECUTE IMMEDIATE 'ALTER TRIGGER services_trg DISABLE'; END IF;
  DBMS_OUTPUT.PUT_LINE('ID automatique : SERVICES_BI / SERVICES_SEQ. Trigger secondaire desactive.');
END;
/
