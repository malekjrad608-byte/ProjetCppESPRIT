-- Transactional check: test rows are rolled back, sequences still advance.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET SERVEROUTPUT ON
DECLARE
  v_service NUMBER;
  v_id1 NUMBER;
  v_id2 NUMBER;
  v_current NUMBER;
  v_max NUMBER;
  v_before NUMBER;
  v_after NUMBER;
  v_unique NUMBER;
BEGIN
  SELECT MIN(id_serv) INTO v_service FROM service;
  IF v_service IS NULL THEN
    RAISE_APPLICATION_ERROR(-20001,'An existing service is required for the FK test');
  END IF;
  SELECT COUNT(*),NVL(MAX(id_emp),0) INTO v_before,v_max FROM employer;
  INSERT INTO employer(nom,prenom,id_serv)
    VALUES('AUTO_ID_CHECK','Temporary',v_service) RETURNING id_emp INTO v_id1;
  SELECT employes_seq.CURRVAL INTO v_current FROM dual;
  IF v_id1 IS NULL OR v_id1<=v_max OR v_current<>v_id1 THEN
    RAISE_APPLICATION_ERROR(-20002,'Automatic ID or Qt CURRVAL mismatch');
  END IF;
  -- The Qt application inserts through the EMPLOYES compatibility view.
  INSERT INTO employes(id,nom,prenom,id_service)
    VALUES(NULL,'AUTO_ID_CHECK','Temporary',v_service);
  SELECT employes_seq.CURRVAL INTO v_id2 FROM dual;
  SELECT COUNT(*) INTO v_after FROM employer
    WHERE id_emp=v_id2 AND nom='AUTO_ID_CHECK';
  IF v_id2<>v_id1+1 OR v_after<>1 THEN
    RAISE_APPLICATION_ERROR(-20003,'View insert or increment-by-one check failed');
  END IF;
  SELECT COUNT(*) INTO v_unique FROM all_constraints c
    JOIN all_cons_columns cc ON cc.owner=c.owner AND cc.constraint_name=c.constraint_name
    WHERE c.owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA') AND c.table_name='EMPLOYER'
      AND c.constraint_type='U' AND c.status='ENABLED' AND c.validated='VALIDATED'
      AND cc.column_name='CIN'
      AND 1=(SELECT COUNT(*) FROM all_cons_columns x
        WHERE x.owner=c.owner AND x.constraint_name=c.constraint_name);
  IF v_unique<>1 THEN RAISE_APPLICATION_ERROR(-20004,'CIN unique constraint missing'); END IF;
  DBMS_OUTPUT.PUT_LINE('PASS: omitted ID='||v_id1||', explicit NULL through Qt view='||v_id2);
  DBMS_OUTPUT.PUT_LINE('PASS: increment +1, Qt CURRVAL and CIN UNIQUE');
  ROLLBACK;
  SELECT COUNT(*) INTO v_after FROM employer;
  IF v_after<>v_before THEN RAISE_APPLICATION_ERROR(-20005,'Row count changed'); END IF;
  DBMS_OUTPUT.PUT_LINE('PASS: test rows rolled back; existing employees='||v_after);
EXCEPTION WHEN OTHERS THEN
  ROLLBACK;
  RAISE;
END;
/
