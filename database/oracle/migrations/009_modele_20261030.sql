-- Migration du MLD du 30/10/2026. Sauvegarder RAWAND avant execution.
-- Les cinq tables metier portent les noms du MLD. Les vues gardent le contrat
-- des modules Qt existants ; elles ne creent aucune copie des donnees.
-- Les champs complementaires existants sont conserves pour ne pas perdre
-- les informations cliniques et les etats des locaux.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET SERVEROUTPUT ON
SET DEFINE OFF
SET SQLBLANKLINES ON
DECLARE n NUMBER; BEGIN
 IF USER<>'RAWAND' THEN RAISE_APPLICATION_ERROR(-20001,'Schema RAWAND requis'); END IF;
 SELECT COUNT(*) INTO n FROM user_tables WHERE table_name IN ('SERVICES','LOCAUX','STOCK_LOTS','EMPLOYES','MEDICAL_RECORDS');
 IF n<>5 THEN RAISE_APPLICATION_ERROR(-20002,'Migration deja appliquee ou schema inattendu'); END IF;
 SELECT COUNT(*) INTO n FROM (SELECT name FROM services GROUP BY name HAVING COUNT(*)>1);
 IF n>0 THEN RAISE_APPLICATION_ERROR(-20003,'References service en double'); END IF;
 SELECT COUNT(*) INTO n FROM (SELECT animal_id FROM medical_records GROUP BY animal_id HAVING COUNT(*)>1);
 IF n>0 THEN RAISE_APPLICATION_ERROR(-20004,'Identifiants animaux en double'); END IF;
 SELECT COUNT(*) INTO n FROM (SELECT article FROM stock_lots GROUP BY article HAVING COUNT(*)>1);
 IF n>0 THEN RAISE_APPLICATION_ERROR(-20005,'References article en double'); END IF;
END;
/
ALTER TABLE SERVICES RENAME TO SERVICE;
ALTER TABLE SERVICE RENAME COLUMN ID TO ID_SERV;
ALTER TABLE SERVICE RENAME COLUMN NAME TO REF_SERVICE;
ALTER TABLE SERVICE RENAME COLUMN CATEGORY TO CATEGORIE;
ALTER TABLE SERVICE RENAME COLUMN STATUS TO STATUT;
ALTER TABLE SERVICE RENAME COLUMN DATETIME TO DATE_HEUR;
ALTER TABLE SERVICE RENAME COLUMN DURATION TO DUREE;
ALTER TABLE SERVICE RENAME COLUMN PRICE TO TARIF;
ALTER TABLE SERVICE RENAME COLUMN DESCRIPTION TO PRESTATION;
ALTER TABLE LOCAUX RENAME TO LOCAL;
ALTER TABLE LOCAL RENAME COLUMN ID TO ID_LOCAL;
ALTER TABLE LOCAL RENAME COLUMN NAME TO NOM_LOCAL;
ALTER TABLE LOCAL RENAME COLUMN FLOOR TO ETAGE;
ALTER TABLE LOCAL RENAME COLUMN CAPACITY TO CAPACITE;
ALTER TABLE LOCAL RENAME COLUMN SPECIALTY TO SPECIALITE;
ALTER TABLE LOCAL RENAME COLUMN ID_SERVICE TO ID_SERV;
ALTER TABLE STOCK_LOTS RENAME TO STOCK;
ALTER TABLE STOCK RENAME COLUMN CATEGORY TO TYPE_ARTICLE;
ALTER TABLE STOCK RENAME COLUMN QUANTITY TO QUANTITE_DISP;
ALTER TABLE STOCK RENAME COLUMN THRESHOLD TO SEUIL_MIN;
ALTER TABLE STOCK RENAME COLUMN EXPIRY TO DATE_ECHEANCE;
ALTER TABLE EMPLOYES RENAME TO EMPLOYER;
ALTER TABLE EMPLOYER RENAME COLUMN ID TO ID_EMP;
ALTER TABLE EMPLOYER RENAME COLUMN TELEPHONE TO TEL;
ALTER TABLE EMPLOYER RENAME COLUMN CHARGE_HEBDO TO CHARGE;
ALTER TABLE EMPLOYER RENAME COLUMN DISPO_HEBDO TO DISPO_REPOS;
ALTER TABLE EMPLOYER RENAME COLUMN ID_SERVICE TO ID_SERV;
ALTER TABLE MEDICAL_RECORDS RENAME TO MEDICAL;
ALTER TABLE MEDICAL RENAME COLUMN ID TO ID_DOSS;
ALTER TABLE MEDICAL RENAME COLUMN ANIMAL_ID TO ID_ANIMAL;
ALTER TABLE MEDICAL RENAME COLUMN NAME TO NOM_ANIMAL;
ALTER TABLE MEDICAL RENAME COLUMN SPECIES TO TYPE_ANIMAL;
ALTER TABLE MEDICAL RENAME COLUMN BREED TO RACE;
ALTER TABLE MEDICAL RENAME COLUMN SEX TO SEXE;
ALTER TABLE MEDICAL RENAME COLUMN CONSULTATION_DATE TO DATE_CONSULTATION;
ALTER TABLE MEDICAL RENAME COLUMN REASON TO MOTIF;
ALTER TABLE MEDICAL RENAME COLUMN DIAGNOSIS TO DIAGNOSTIC;
ALTER TABLE MEDICAL RENAME COLUMN ID_EMPLOYE TO ID_EMP;

ALTER TABLE service ADD CONSTRAINT uq_service_ref UNIQUE(ref_service);
ALTER TABLE local ADD CONSTRAINT uq_local_nom UNIQUE(nom_local);
ALTER TABLE local ADD (etat GENERATED ALWAYS AS
 (CASE WHEN out_of_service=1 THEN 'Hors service' WHEN occupancy>0 THEN 'Occupe' ELSE 'Disponible' END) VIRTUAL);
ALTER TABLE employer ADD (cin VARCHAR2(30 CHAR),date_naiss VARCHAR2(10 CHAR),adresse VARCHAR2(500 CHAR),type_contrat VARCHAR2(80 CHAR),salaire NUMBER(12,2));
ALTER TABLE employer ADD CONSTRAINT uq_employer_cin UNIQUE(cin);
ALTER TABLE employer ADD CONSTRAINT ck_employer_salaire CHECK(salaire>=0);
ALTER TABLE medical ADD (espece VARCHAR2(30 CHAR));
ALTER TABLE medical ADD CONSTRAINT ck_medical_paiement CHECK(espece IN ('Espèces'));
ALTER TABLE medical ADD CONSTRAINT uq_medical_animal UNIQUE(id_animal);
ALTER TABLE stock ADD (id_article NUMBER(10),nom_article VARCHAR2(250 CHAR));
CREATE SEQUENCE stock_seq START WITH 1 INCREMENT BY 1 NOCYCLE;
UPDATE stock SET id_article=stock_seq.NEXTVAL,nom_article=article;
ALTER TABLE stock DROP PRIMARY KEY;
ALTER TABLE stock ADD CONSTRAINT stock_pk PRIMARY KEY(id_article);
ALTER TABLE stock ADD CONSTRAINT uq_stock_article UNIQUE(article);
ALTER TABLE stock ADD CONSTRAINT uq_stock_lot UNIQUE(lot);
ALTER TABLE stock MODIFY nom_article NOT NULL;
DECLARE n NUMBER; BEGIN
 SELECT NVL(MAX(id_local),0)+1 INTO n FROM local;
 EXECUTE IMMEDIATE 'CREATE SEQUENCE local_seq START WITH '||TO_CHAR(n)||' INCREMENT BY 1 NOCYCLE';
END;
/
-- Retirer les anciens triggers qui referencent les colonnes renommees.
BEGIN
 FOR t IN (SELECT trigger_name FROM user_triggers WHERE table_name IN ('SERVICE','LOCAL','STOCK','EMPLOYER','MEDICAL')) LOOP
  EXECUTE IMMEDIATE 'DROP TRIGGER '||DBMS_ASSERT.SIMPLE_SQL_NAME(t.trigger_name);
 END LOOP;
END;
/
CREATE OR REPLACE TRIGGER service_bi BEFORE INSERT ON service FOR EACH ROW
BEGIN
 IF :NEW.id_serv IS NULL THEN :NEW.id_serv:=services_seq.NEXTVAL; END IF;
 
END;
/
CREATE OR REPLACE TRIGGER local_bi BEFORE INSERT ON local FOR EACH ROW
BEGIN
 IF :NEW.id_local IS NULL THEN :NEW.id_local:=local_seq.NEXTVAL; END IF;
 
END;
/
CREATE OR REPLACE TRIGGER stock_bi BEFORE INSERT ON stock FOR EACH ROW
BEGIN
 IF :NEW.id_article IS NULL THEN :NEW.id_article:=stock_seq.NEXTVAL; END IF;
 IF :NEW.nom_article IS NULL THEN :NEW.nom_article:=:NEW.article; END IF;
END;
/
CREATE OR REPLACE TRIGGER employer_bi BEFORE INSERT ON employer FOR EACH ROW
BEGIN
 IF :NEW.id_emp IS NULL THEN :NEW.id_emp:=employes_seq.NEXTVAL; END IF;
 
END;
/
CREATE OR REPLACE TRIGGER medical_bi BEFORE INSERT ON medical FOR EACH ROW
BEGIN
 IF :NEW.id_doss IS NULL THEN :NEW.id_doss:=medical_records_seq.NEXTVAL; END IF;
 
END;
/

CREATE OR REPLACE VIEW services AS
SELECT id_serv id,ref_service name,prestation description,responsible,
 categorie category,date_heur datetime,statut status,duree duration,tarif price,
 ref_service clientname,CAST(NULL AS VARCHAR2(60)) invoicenumber,
 CAST(NULL AS VARCHAR2(35)) invoicedate,'BROUILLON' invoice_status,
 CAST(NULL AS VARCHAR2(35)) invoice_emitted_at FROM service;
CREATE OR REPLACE VIEW locaux AS
SELECT id_local id,nom_local name,type,etage floor,capacite capacity,occupancy,
 specialite specialty,out_of_service,cleaning,last_cleaned,dirty_since,id_serv id_service,etat FROM local;
CREATE OR REPLACE VIEW stock_lots AS
SELECT lot,article,type_article category,quantite_disp quantity,unit,seuil_min threshold,
 date_echeance expiry,id_local,id_article,nom_article FROM stock;
CREATE OR REPLACE VIEW employes AS
SELECT id_emp id,nom,prenom,courriel,tel telephone,poste,date_embauche,disponible,
 charge charge_hebdo,dispo_repos dispo_hebdo,competences,id_serv id_service,
 cin,date_naiss,adresse,type_contrat,salaire FROM employer;
CREATE OR REPLACE VIEW medical_records AS
SELECT id_doss id,id_animal animal_id,nom_animal name,type_animal species,race breed,
 age,sexe sex,proprietaire,date_consultation consultation_date,motif reason,
 diagnostic diagnosis,treatment,vaccinations,allergies,weight,observations,
 vaccination_status,id_emp id_employe,espece FROM medical;
CREATE OR REPLACE VIEW planning_affectations AS
SELECT e.id_emp id,s.ref_service titre,NVL(e.competences,'Service') competence_requise,
 REPLACE(SUBSTR(s.date_heur,1,19),'T',' ') debut,
 TO_CHAR(TO_TIMESTAMP(SUBSTR(s.date_heur,1,19),'YYYY-MM-DD"T"HH24:MI:SS')+NUMTODSINTERVAL(s.duree,'MINUTE'),'YYYY-MM-DD HH24:MI:SS') fin,
 s.duree/60 charge_heures,e.id_emp responsable_id,s.date_heur created_at
FROM employer e JOIN service s ON s.id_serv=e.id_serv WHERE s.statut<>'Annulé' WITH READ ONLY;
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
BEGIN DBMS_UTILITY.COMPILE_SCHEMA(schema=>USER,compile_all=>FALSE); END;
/
SELECT table_name FROM user_tables ORDER BY table_name;
SELECT object_name,object_type FROM user_objects WHERE status<>'VALID';
