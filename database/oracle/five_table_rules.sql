-- Objets derives : aucune table supplementaire.
CREATE OR REPLACE VIEW planning_affectations AS
SELECT e.id,s.name titre,NVL(e.competences,'Service') competence_requise,
 REPLACE(SUBSTR(s.datetime,1,19),'T',' ') debut,
 TO_CHAR(TO_TIMESTAMP(SUBSTR(s.datetime,1,19),'YYYY-MM-DD"T"HH24:MI:SS')+NUMTODSINTERVAL(s.duration,'MINUTE'),'YYYY-MM-DD HH24:MI:SS') fin,
 s.duration/60 charge_heures,e.id responsable_id,s.datetime created_at
FROM employes e JOIN services s ON s.id=e.id_service
WHERE s.status<>'Annulé'
WITH READ ONLY;
CREATE OR REPLACE TRIGGER services_invoice_guard
BEFORE UPDATE OR DELETE ON services FOR EACH ROW
DECLARE
 FUNCTION changed(a VARCHAR2,b VARCHAR2) RETURN BOOLEAN IS
 BEGIN RETURN a<>b OR (a IS NULL AND b IS NOT NULL) OR (a IS NOT NULL AND b IS NULL); END;
BEGIN
 IF DELETING AND :OLD.invoice_status IN ('EMISE','ANNULEE') THEN RAISE_APPLICATION_ERROR(-20041,'Conservez le service facture'); END IF;
 IF UPDATING AND :OLD.invoice_status IN ('EMISE','ANNULEE') THEN
  IF :NEW.invoice_status='BROUILLON' OR (:OLD.invoice_status='ANNULEE' AND :NEW.invoice_status<>'ANNULEE') OR
   changed(:NEW.name,:OLD.name) OR changed(:NEW.clientName,:OLD.clientName) OR changed(:NEW.responsible,:OLD.responsible) OR
   changed(:NEW.category,:OLD.category) OR changed(:NEW.datetime,:OLD.datetime) OR :NEW.duration<>:OLD.duration OR :NEW.price<>:OLD.price OR
   changed(:NEW.invoiceNumber,:OLD.invoiceNumber) OR changed(:NEW.invoiceDate,:OLD.invoiceDate) OR changed(:NEW.invoice_emitted_at,:OLD.invoice_emitted_at) OR
   NVL(DBMS_LOB.GETLENGTH(:NEW.description),0)<>NVL(DBMS_LOB.GETLENGTH(:OLD.description),0) OR NVL(DBMS_LOB.COMPARE(:NEW.description,:OLD.description),0)<>0 THEN
    RAISE_APPLICATION_ERROR(-20041,'Informations facturees figees');
  END IF;
 END IF;
 IF UPDATING AND :NEW.status='Annulé' AND :OLD.invoice_status='EMISE' THEN :NEW.invoice_status:='ANNULEE'; END IF;
END;
/
-- Chaque employe n'a qu'un service. Une reaffectation sur un creneau deja occupe est refusee.
CREATE OR REPLACE TRIGGER employee_transfer_guard
BEFORE UPDATE OF id_service ON employes FOR EACH ROW
DECLARE a TIMESTAMP;b TIMESTAMP;da NUMBER;db NUMBER;sa VARCHAR2(30);sb VARCHAR2(30);
BEGIN
 IF :NEW.id_service<>:OLD.id_service THEN
  SELECT TO_TIMESTAMP(SUBSTR(datetime,1,19),'YYYY-MM-DD"T"HH24:MI:SS'),duration,status INTO a,da,sa FROM services WHERE id=:OLD.id_service;
  SELECT TO_TIMESTAMP(SUBSTR(datetime,1,19),'YYYY-MM-DD"T"HH24:MI:SS'),duration,status INTO b,db,sb FROM services WHERE id=:NEW.id_service;
  IF sa<>'Annulé' AND sb<>'Annulé' AND a<b+NUMTODSINTERVAL(db,'MINUTE') AND b<a+NUMTODSINTERVAL(da,'MINUTE') THEN
   RAISE_APPLICATION_ERROR(-20051,'Employe deja occupe sur ce creneau');
  END IF;
 END IF;
END;
/
