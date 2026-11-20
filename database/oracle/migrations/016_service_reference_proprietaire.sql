-- Run as RAWAND. Keep existing IDs and foreign keys. Preserve the former
-- REF_SERVICE text in PROPRIETAIRE before converting old references to codes.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET DEFINE OFF
DECLARE
  n NUMBER;
BEGIN
  SELECT COUNT(*) INTO n FROM service WHERE id_serv NOT BETWEEN 0 AND 999;
  IF n > 0 THEN RAISE_APPLICATION_ERROR(-20001,'Existing IDs need an explicit three-digit mapping'); END IF;
  SELECT COUNT(*) INTO n FROM all_tab_columns
    WHERE owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA') AND table_name='SERVICE' AND column_name='PROPRIETAIRE';
  IF n=0 THEN EXECUTE IMMEDIATE 'ALTER TABLE service ADD (proprietaire VARCHAR2(120 CHAR))'; END IF;
END;
/
LOCK TABLE service IN EXCLUSIVE MODE WAIT 10;
UPDATE service SET proprietaire=ref_service, ref_service='SRV-'||TO_CHAR(id_serv,'FM000')
WHERE proprietaire IS NULL;
COMMIT;
DECLARE
  n NUMBER;
BEGIN
  SELECT COUNT(*) INTO n FROM all_tab_columns WHERE owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA')
    AND table_name='SERVICE' AND column_name='PROPRIETAIRE' AND nullable='Y';
  IF n=1 THEN EXECUTE IMMEDIATE 'ALTER TABLE service MODIFY (proprietaire NOT NULL)'; END IF;
  SELECT COUNT(*) INTO n FROM all_constraints WHERE owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA')
    AND table_name='SERVICE' AND constraint_name='CK_SERVICE_REF_FORMAT';
  IF n=0 THEN EXECUTE IMMEDIATE q'[ALTER TABLE service ADD CONSTRAINT ck_service_ref_format
    CHECK (LENGTH(ref_service)=7 AND REGEXP_LIKE(ref_service,'^SRV-[0-9]{3}$','c'))]'; END IF;
END;
/
CREATE OR REPLACE NONEDITIONABLE VIEW services AS
SELECT s.id_serv id, s.ref_service name, s.prestation description,
       (SELECT MIN(e.nom||' '||e.prenom) FROM employer e WHERE e.id_serv=s.id_serv) responsible,
       s.categorie category, s.date_heur datetime, s.statut status, s.duree duration, s.tarif price,
       s.proprietaire clientname, CAST(NULL AS VARCHAR2(60)) invoicenumber,
       CAST(NULL AS VARCHAR2(35)) invoicedate, 'BROUILLON' invoice_status,
       CAST(NULL AS VARCHAR2(35)) invoice_emitted_at FROM service s;
