-- Insert twice without IDs. Check +1 and Qt sequence compatibility.
-- Test rows are rolled back; sequence numbers remain consumed normally.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET SERVEROUTPUT ON
SET DEFINE OFF
DECLARE
  s NUMBER; l NUMBER; e NUMBER; m NUMBER; st NUMBER;
  ps NUMBER; pl NUMBER; pe NUMBER; pm NUMBER; pst NUMBER;
  v_current NUMBER;
  v_tag VARCHAR2(30);
v_ref VARCHAR2(7);
BEGIN
  SAVEPOINT auto_check;
  FOR i IN 1..2 LOOP
    v_tag := 'AUTO_'||SUBSTR(RAWTOHEX(SYS_GUID()),1,20);
    SELECT MIN(code) INTO v_ref FROM (SELECT 'SRV-'||TO_CHAR(LEVEL-1,'FM000') code FROM dual CONNECT BY LEVEL<=1000) WHERE code NOT IN (SELECT ref_service FROM service);
   INSERT INTO service(proprietaire,ref_service,categorie,statut,date_heur,duree,tarif)
      VALUES(v_tag,v_ref,'Consultation','En cours','2036-01-01T09:00:00',30,10)
      RETURNING id_serv INTO s;
    SELECT services_seq.CURRVAL INTO v_current FROM dual;
    IF s IS NULL OR s<>v_current THEN RAISE_APPLICATION_ERROR(-20001,'SERVICE failed'); END IF;
    INSERT INTO local(nom_local,type,etage,capacite,etat,id_serv)
      VALUES(v_tag,'Consultation',0,1,'Disponible',s) RETURNING id_local INTO l;
    SELECT local_seq.CURRVAL INTO v_current FROM dual;
    IF l IS NULL OR l<>v_current THEN RAISE_APPLICATION_ERROR(-20002,'LOCAL failed'); END IF;
    INSERT INTO employer(nom,prenom,cin,id_serv)
      VALUES(v_tag,'Test',v_tag,s) RETURNING id_emp INTO e;
    SELECT employes_seq.CURRVAL INTO v_current FROM dual;
    IF e IS NULL OR e<>v_current THEN RAISE_APPLICATION_ERROR(-20003,'EMPLOYER failed'); END IF;
    INSERT INTO medical(raf_animal,nom_animal,espece,age,poids,proprietaire,date_consultation,id_emp)
      VALUES(v_tag,'Animal test','Chat',1,3.75,'Test','2036-01-01',e) RETURNING id_doss INTO m;
    SELECT medical_records_seq.CURRVAL INTO v_current FROM dual;
    IF m IS NULL OR m<>v_current THEN RAISE_APPLICATION_ERROR(-20004,'MEDICAL failed'); END IF;
    INSERT INTO stock(ref_article,nom_article,lot,type_article,quantite_disp,seuil_min,date_echeance,id_local)
      VALUES(v_tag,'Produit test',v_tag,'Soins',1,1,'2036-01-01',l) RETURNING id_article INTO st;
    SELECT stock_seq.CURRVAL INTO v_current FROM dual;
    IF st IS NULL OR st<>v_current THEN RAISE_APPLICATION_ERROR(-20005,'STOCK failed'); END IF;
    IF i=2 AND (s<>ps+1 OR l<>pl+1 OR e<>pe+1 OR m<>pm+1 OR st<>pst+1) THEN
      RAISE_APPLICATION_ERROR(-20006,'Increment +1 failed');
    END IF;
    DBMS_OUTPUT.PUT_LINE('PASS round '||i||': SERVICE='||s||', LOCAL='||l||', EMPLOYER='||e||', MEDICAL='||m||', STOCK='||st);
    ps:=s; pl:=l; pe:=e; pm:=m; pst:=st;
  END LOOP;
  ROLLBACK TO auto_check;
  DBMS_OUTPUT.PUT_LINE('PASS: all five automatic IDs, increment +1, Qt CURRVAL, FK chain; test rows rolled back.');
EXCEPTION WHEN OTHERS THEN
  ROLLBACK TO auto_check;
  RAISE;
END;
/
