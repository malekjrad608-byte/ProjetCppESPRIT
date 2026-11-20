-- Migration supervisee du schema RAWAND sauvegarde le 16/10/2026.
-- Ne pas executer sur un autre jeu de donnees : les preconditions l'interdisent.
whenever sqlerror exit failure rollback
set define off
set serveroutput on
DECLARE n NUMBER;
BEGIN
 IF SYS_CONTEXT('USERENV','CURRENT_SCHEMA')<>'RAWAND' OR SYS_CONTEXT('USERENV','CON_NAME')<>'CDB$ROOT' THEN RAISE_APPLICATION_ERROR(-20001,'Schema inattendu'); END IF;
 SELECT COUNT(*) INTO n FROM services; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20001,'Services modifies : migration a revoir'); END IF;
 SELECT COUNT(*) INTO n FROM services WHERE id=5; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20001,'Service 5 absent'); END IF;
 SELECT COUNT(*) INTO n FROM employes; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20001,'Employes modifies'); END IF;
 SELECT COUNT(*) INTO n FROM employes WHERE id=12; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20001,'Employe 12 absent'); END IF;
 SELECT (SELECT COUNT(*) FROM locaux)+(SELECT COUNT(*) FROM stock_lots)+(SELECT COUNT(*) FROM medical_records)+(SELECT COUNT(*) FROM planning_affectations)+(SELECT COUNT(*) FROM stock_movements) INTO n FROM dual;
 IF n<>0 THEN RAISE_APPLICATION_ERROR(-20001,'Donnees nouvelles : sauvegarder et revoir la migration'); END IF;
 SELECT COUNT(*) INTO n FROM factures WHERE statut<>'BROUILLON' OR id_service IS NULL;
 IF n<>0 THEN RAISE_APPLICATION_ERROR(-20001,'Factures historiques a conserver avant migration'); END IF;
END;
/
DROP TRIGGER services_relations_biu;
DROP TRIGGER services_facture_aiu;
DROP TRIGGER services_schedule_guard;
DROP TRIGGER tasks_schedule_guard;
DROP PACKAGE employee_schedule;
ALTER TABLE services ADD (invoice_status VARCHAR2(12 CHAR) DEFAULT 'BROUILLON' NOT NULL, invoice_emitted_at VARCHAR2(35 CHAR));
ALTER TABLE services ADD CONSTRAINT ck_service_invoice CHECK(invoice_status IN ('BROUILLON','EMISE','ANNULEE'));
UPDATE services s SET invoice_status=(SELECT f.statut FROM factures f WHERE f.id_service=s.id);
ALTER TABLE employes ADD (id_service NUMBER(10));
ALTER TABLE locaux ADD (id_service NUMBER(10));
ALTER TABLE medical_records ADD (id_employe NUMBER(10));
UPDATE employes SET id_service=5 WHERE id=12;
UPDATE services SET id_employe=12,responsible=(SELECT TRIM(nom||' '||prenom) FROM employes WHERE id=12) WHERE id=5;
COMMIT;
ALTER TABLE services DROP CONSTRAINT fk_services_employe;
ALTER TABLE services DROP COLUMN id_local CASCADE CONSTRAINTS;
ALTER TABLE services DROP COLUMN id_client CASCADE CONSTRAINTS;
ALTER TABLE medical_records DROP COLUMN id_local CASCADE CONSTRAINTS;
ALTER TABLE medical_records DROP COLUMN id_service CASCADE CONSTRAINTS;
ALTER TABLE employes MODIFY (id_service NOT NULL);
ALTER TABLE locaux MODIFY (id_service NOT NULL);
ALTER TABLE stock_lots MODIFY (id_local NOT NULL);
ALTER TABLE medical_records MODIFY (id_employe NOT NULL);
ALTER TABLE services MODIFY (id_employe NOT NULL);
ALTER TABLE employes ADD CONSTRAINT uq_employee_team UNIQUE(id,id_service);
ALTER TABLE employes ADD CONSTRAINT fk_employe_service FOREIGN KEY(id_service) REFERENCES services(id) DEFERRABLE INITIALLY DEFERRED;
-- Un referent appartenant a l'equipe garantit au moins un employe par service.
ALTER TABLE services ADD CONSTRAINT fk_service_team FOREIGN KEY(id_employe,id) REFERENCES employes(id,id_service) DEFERRABLE INITIALLY DEFERRED;
ALTER TABLE locaux ADD CONSTRAINT fk_local_service FOREIGN KEY(id_service) REFERENCES services(id);
ALTER TABLE medical_records ADD CONSTRAINT fk_medical_employe FOREIGN KEY(id_employe) REFERENCES employes(id);
CREATE INDEX ix_employee_service ON employes(id_service);
CREATE INDEX ix_local_service ON locaux(id_service);
CREATE INDEX ix_medical_employe ON medical_records(id_employe);
-- Sauvegarde Data Pump complete verifiee avant suppression. Comptes migres en fichier local.
DROP TABLE lignes_facture CASCADE CONSTRAINTS PURGE;
DROP TABLE factures CASCADE CONSTRAINTS PURGE;
DROP TABLE clients CASCADE CONSTRAINTS PURGE;
DROP TABLE planning_affectations CASCADE CONSTRAINTS PURGE;
DROP SEQUENCE planning_affectations_seq;
DROP TABLE stock_movements CASCADE CONSTRAINTS PURGE;
DROP TABLE collection_versions CASCADE CONSTRAINTS PURGE;
DROP TABLE integration_migrations CASCADE CONSTRAINTS PURGE;
DROP TABLE schema_version CASCADE CONSTRAINTS PURGE;
DROP TABLE accounts CASCADE CONSTRAINTS PURGE;
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

PROMPT Migration terminee : 5 tables metier.
SELECT table_name FROM all_tables WHERE owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA') ORDER BY table_name;
