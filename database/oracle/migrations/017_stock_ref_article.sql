-- Run as RAWAND (or with CURRENT_SCHEMA=RAWAND).
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
ALTER TABLE STOCK RENAME COLUMN ARTICLE TO REF_ARTICLE;

-- Preserve the interface used by existing Qt readers.
CREATE OR REPLACE VIEW STOCK_LOTS AS
SELECT lot,ref_article article,type_article category,quantite_disp quantity,
 UNISTR('unit\00E9') unit,seuil_min threshold,date_echeance expiry,
 id_local,id_article,nom_article FROM stock;

ALTER TRIGGER STOCK_BI COMPILE;
ALTER TRIGGER STOCK_TRG COMPILE;
