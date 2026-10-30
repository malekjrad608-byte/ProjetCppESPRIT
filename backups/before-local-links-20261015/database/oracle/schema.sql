-- ANIWORLD / Oracle XE 21c. Schema matching the current Qt application.
-- Install once in the dedicated ANIWORLD_APP schema; no DROP or data replacement.
-- ISO text dates preserve the current UI contract during this migration.
whenever sqlerror exit failure rollback
set define off
CREATE TABLE schema_version(id NUMBER(1) PRIMARY KEY, version NUMBER(5) NOT NULL);
CREATE TABLE integration_migrations(name VARCHAR2(120 CHAR) PRIMARY KEY);
CREATE TABLE accounts(
 username VARCHAR2(120 CHAR) PRIMARY KEY,
 role VARCHAR2(20 CHAR) NOT NULL CHECK(role IN ('admin','services','stock','employes','locaux','medical')),
 salt RAW(24) NOT NULL,password_hash RAW(32) NOT NULL,iterations NUMBER(7) NOT NULL CHECK(iterations BETWEEN 10000 AND 1000000)
);
CREATE UNIQUE INDEX accounts_username_ci ON accounts(LOWER(username));
CREATE TABLE services(
 id NUMBER(10) PRIMARY KEY,name VARCHAR2(120 CHAR) NOT NULL,
 description CLOB,responsible VARCHAR2(250 CHAR),category VARCHAR2(120 CHAR) NOT NULL,
 dateTime VARCHAR2(35 CHAR) NOT NULL,status VARCHAR2(30 CHAR) NOT NULL CHECK(status IN ('Planifié','En cours','Terminé','Annulé')),
 duration NUMBER(4) NOT NULL CHECK(duration BETWEEN 5 AND 1440),price NUMBER(10,2) NOT NULL CHECK(price BETWEEN 0 AND 99999.99),
 clientName VARCHAR2(120 CHAR),invoiceNumber VARCHAR2(60 CHAR) UNIQUE,invoiceDate VARCHAR2(35 CHAR)
);
CREATE INDEX services_date_idx ON services(dateTime);
CREATE TABLE employes(
 id NUMBER(10) PRIMARY KEY,nom VARCHAR2(120 CHAR) NOT NULL,prenom VARCHAR2(120 CHAR) NOT NULL,
 courriel VARCHAR2(250 CHAR),telephone VARCHAR2(60 CHAR),poste VARCHAR2(150 CHAR),date_embauche VARCHAR2(10 CHAR),
 disponible NUMBER(1) DEFAULT 1 CHECK(disponible IN (0,1)),charge_hebdo NUMBER(8,2) DEFAULT 0 CHECK(charge_hebdo>=0),
 dispo_hebdo NUMBER(8,2) DEFAULT 35 CHECK(dispo_hebdo>=0),competences VARCHAR2(2000 CHAR)
);
CREATE TABLE planning_affectations(
 id NUMBER(10) PRIMARY KEY,titre VARCHAR2(250 CHAR) NOT NULL,competence_requise VARCHAR2(250 CHAR) NOT NULL,
 debut VARCHAR2(19 CHAR) NOT NULL,fin VARCHAR2(19 CHAR) NOT NULL,charge_heures NUMBER(8,2) DEFAULT 1 NOT NULL CHECK(charge_heures>0),
 responsable_id NUMBER(10) NOT NULL REFERENCES employes(id) ON DELETE CASCADE,
 created_at VARCHAR2(35 CHAR) DEFAULT TO_CHAR(SYSTIMESTAMP,'YYYY-MM-DD HH24:MI:SS'),CHECK(fin>debut)
);
CREATE INDEX idx_planning_responsable_dates ON planning_affectations(responsable_id,debut,fin);
CREATE TABLE medical_records(
 id NUMBER(10) PRIMARY KEY,animal_id VARCHAR2(120 CHAR) NOT NULL,name VARCHAR2(120 CHAR) NOT NULL,
 species VARCHAR2(30 CHAR) NOT NULL,breed VARCHAR2(120 CHAR),age NUMBER(2) NOT NULL CHECK(age BETWEEN 0 AND 30),
 sex VARCHAR2(20 CHAR) NOT NULL,owner VARCHAR2(120 CHAR) NOT NULL,consultation_date VARCHAR2(10 CHAR) NOT NULL,
 reason CLOB,diagnosis CLOB,treatment CLOB,vaccinations CLOB,allergies CLOB,
 weight NUMBER(6,2) NOT NULL CHECK(weight BETWEEN 0 AND 100),observations CLOB,
 vaccination_status VARCHAR2(30 CHAR) NOT NULL CHECK(vaccination_status IN ('À jour','À renouveler'))
);
CREATE INDEX medical_date_idx ON medical_records(consultation_date);
CREATE SEQUENCE services_seq;
CREATE SEQUENCE employes_seq;
CREATE SEQUENCE planning_affectations_seq;
CREATE SEQUENCE medical_records_seq;
CREATE OR REPLACE TRIGGER services_bi BEFORE INSERT ON services FOR EACH ROW
BEGIN
 IF :NEW.id IS NULL THEN :NEW.id := services_seq.NEXTVAL; END IF;
 IF :NEW.invoiceNumber IS NULL THEN
  :NEW.invoiceNumber := 'FAC-' || SUBSTR(:NEW.dateTime,1,4) || '-' || LPAD(TO_CHAR(:NEW.id),GREATEST(6,LENGTH(TO_CHAR(:NEW.id))),'0');
  :NEW.invoiceDate := TO_CHAR(SYSTIMESTAMP,'YYYY-MM-DD"T"HH24:MI:SS');
 END IF;
END;
/
CREATE OR REPLACE TRIGGER employes_bi BEFORE INSERT ON employes FOR EACH ROW
BEGIN IF :NEW.id IS NULL THEN :NEW.id := employes_seq.NEXTVAL; END IF; END;
/
CREATE OR REPLACE TRIGGER planning_affectations_bi BEFORE INSERT ON planning_affectations FOR EACH ROW
BEGIN IF :NEW.id IS NULL THEN :NEW.id := planning_affectations_seq.NEXTVAL; END IF; END;
/
CREATE OR REPLACE TRIGGER medical_records_bi BEFORE INSERT ON medical_records FOR EACH ROW
BEGIN IF :NEW.id IS NULL THEN :NEW.id := medical_records_seq.NEXTVAL; END IF; END;
/
CREATE TABLE collection_versions(name VARCHAR2(30) PRIMARY KEY,revision NUMBER(10) DEFAULT 0 NOT NULL);
INSERT INTO collection_versions VALUES('locaux',0);
INSERT INTO collection_versions VALUES('stock',0);
CREATE TABLE locaux(
 id NUMBER(10) PRIMARY KEY,name VARCHAR2(150 CHAR) NOT NULL,type VARCHAR2(100 CHAR) NOT NULL,
 floor NUMBER(3) NOT NULL,capacity NUMBER(6) NOT NULL CHECK(capacity>=1),occupancy NUMBER(6) NOT NULL,
 specialty VARCHAR2(250 CHAR),out_of_service NUMBER(1) NOT NULL CHECK(out_of_service IN (0,1)),
 cleaning NUMBER(1) NOT NULL CHECK(cleaning IN (0,1,2)),last_cleaned VARCHAR2(35 CHAR),dirty_since VARCHAR2(35 CHAR),
 CHECK(occupancy BETWEEN 0 AND capacity)
);
CREATE UNIQUE INDEX locaux_name_ci ON locaux(LOWER(name));
CREATE TABLE stock_lots(
 lot VARCHAR2(120 CHAR) PRIMARY KEY,article VARCHAR2(250 CHAR) NOT NULL,category VARCHAR2(100 CHAR) NOT NULL,
 quantity NUMBER(6) NOT NULL CHECK(quantity BETWEEN 0 AND 999999),unit VARCHAR2(60 CHAR) NOT NULL,
 threshold NUMBER(6) NOT NULL CHECK(threshold BETWEEN 1 AND 999999),expiry VARCHAR2(10 CHAR) NOT NULL
);
CREATE TABLE stock_movements(
 id NUMBER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,lot VARCHAR2(120 CHAR) NOT NULL,
 article VARCHAR2(250 CHAR) NOT NULL,quantity_before NUMBER(6) NOT NULL,quantity_after NUMBER(6) NOT NULL,
 action VARCHAR2(40 CHAR) NOT NULL,changed_at TIMESTAMP DEFAULT SYSTIMESTAMP NOT NULL,
 changed_by VARCHAR2(120 CHAR)
);
INSERT INTO schema_version VALUES(1,1);
COMMIT;
