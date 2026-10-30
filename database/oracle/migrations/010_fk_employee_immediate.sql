-- Plus de cycle SERVICE -> EMPLOYER : verifier la FK des la requete.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
ALTER TABLE employer MODIFY CONSTRAINT fk_employe_service INITIALLY IMMEDIATE;
CREATE OR REPLACE TRIGGER employee_transfer_guard
BEFORE UPDATE OF id_serv ON employer FOR EACH ROW
DECLARE a TIMESTAMP;b TIMESTAMP;da NUMBER;db NUMBER;sa VARCHAR2(30);sb VARCHAR2(30);
BEGIN
 IF :NEW.id_serv<>:OLD.id_serv THEN
  SELECT TO_TIMESTAMP(SUBSTR(date_heur,1,19),'YYYY-MM-DD"T"HH24:MI:SS'),duree,statut INTO a,da,sa FROM service WHERE id_serv=:OLD.id_serv;
  SELECT TO_TIMESTAMP(SUBSTR(date_heur,1,19),'YYYY-MM-DD"T"HH24:MI:SS'),duree,statut INTO b,db,sb FROM service WHERE id_serv=:NEW.id_serv;
  IF sa<>'Annulé' AND sb<>'Annulé' AND a<b+NUMTODSINTERVAL(db,'MINUTE') AND b<a+NUMTODSINTERVAL(da,'MINUTE') THEN
   RAISE_APPLICATION_ERROR(-20051,'Employe deja occupe sur ce creneau');
  END IF;
 END IF;
EXCEPTION WHEN NO_DATA_FOUND THEN
 RAISE_APPLICATION_ERROR(-20052,'Service de destination introuvable');
END;
/
