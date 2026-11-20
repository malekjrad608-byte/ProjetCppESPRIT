-- Test transactionnel des cinq generateurs : les lignes sont annulees.
-- Les sequences avancent normalement meme apres ROLLBACK.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET SERVEROUTPUT ON
SET DEFINE OFF
DECLARE s NUMBER;l NUMBER;e NUMBER;m NUMBER;st NUMBER;
v_ref VARCHAR2(7);
BEGIN
 SELECT MIN(code) INTO v_ref FROM (SELECT 'SRV-'||TO_CHAR(LEVEL-1,'FM000') code FROM dual CONNECT BY LEVEL<=1000) WHERE code NOT IN (SELECT ref_service FROM service);
   INSERT INTO service(proprietaire,ref_service,categorie,statut,date_heur,duree,tarif)
 VALUES('AUTO_TEST_20261030',v_ref,'Consultation','Planifié','2036-01-01T09:00:00',30,10) RETURNING id_serv INTO s;
 INSERT INTO local(nom_local,type,etage,capacite,etat,id_serv)
 VALUES('AUTO_TEST_20261030','Consultation',0,1,'Disponible',s) RETURNING id_local INTO l;
 INSERT INTO employer(nom,prenom,cin,id_serv)
 VALUES('AUTO_TEST_20261030','Test','AUTO_TEST_20261030',s) RETURNING id_emp INTO e;
 INSERT INTO medical(raf_animal,nom_animal,espece,age,poids,proprietaire,date_consultation,id_emp)
 VALUES('AUTO_TEST_20261030','Animal test','Chat',1,3.75,'Test','2036-01-01',e) RETURNING id_doss INTO m;
 INSERT INTO stock(ref_article,nom_article,lot,type_article,quantite_disp,seuil_min,date_echeance,id_local)
 VALUES('AUTO_TEST_20261030','Produit test','AUTO_TEST_20261030','Soins',1,1,'2036-01-01',l) RETURNING id_article INTO st;
 IF s IS NULL OR l IS NULL OR e IS NULL OR m IS NULL OR st IS NULL THEN RAISE_APPLICATION_ERROR(-20001,'ID non genere');END IF;
 DBMS_OUTPUT.PUT_LINE('OK : IDs automatiques des cinq tables, insertion sans aucune cle primaire fournie.');
 ROLLBACK;
 DBMS_OUTPUT.PUT_LINE('OK : toutes les lignes de test annulees.');
EXCEPTION WHEN OTHERS THEN ROLLBACK;RAISE;
END;
/
