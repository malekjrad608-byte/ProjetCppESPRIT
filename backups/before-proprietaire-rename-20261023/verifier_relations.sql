-- Connexion applicative actuelle : RAWAND / XE. Executer tout le fichier avec F5.
-- Lecture seule : aucun ajout, modification ou suppression.
SET SERVEROUTPUT ON
SET LINESIZE 220
SET PAGESIZE 100

SELECT USER AS compte, SYS_CONTEXT('USERENV','CON_NAME') AS base FROM dual;

SELECT c.table_name,cc.column_name,p.table_name AS table_referencee,
       pc.column_name AS colonne_referencee,c.delete_rule,c.status,c.validated
FROM all_constraints c
JOIN all_cons_columns cc ON cc.owner=c.owner AND cc.constraint_name=c.constraint_name
JOIN all_constraints p ON p.owner=c.r_owner AND p.constraint_name=c.r_constraint_name
JOIN all_cons_columns pc ON pc.owner=p.owner AND pc.constraint_name=p.constraint_name AND pc.position=cc.position
WHERE c.owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA') AND c.constraint_type='R'
ORDER BY c.table_name,cc.position,cc.column_name;

DECLARE
 n NUMBER; total_erreurs NUMBER:=0; condition_join VARCHAR2(4000); condition_null VARCHAR2(4000);
BEGIN
 FOR c IN (SELECT c.table_name,c.constraint_name,p.table_name AS parent_table,c.r_constraint_name
           FROM all_constraints c JOIN all_constraints p ON p.owner=c.r_owner AND p.constraint_name=c.r_constraint_name
           WHERE c.owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA') AND c.constraint_type='R') LOOP
  condition_join:='';condition_null:='';
  FOR col IN (SELECT a.column_name AS child_col,b.column_name AS parent_col
              FROM all_cons_columns a JOIN all_cons_columns b ON b.owner=a.owner AND b.constraint_name=c.r_constraint_name AND b.position=a.position
              WHERE a.owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA') AND a.constraint_name=c.constraint_name ORDER BY a.position) LOOP
   IF condition_join IS NOT NULL THEN condition_join:=condition_join||' AND ';condition_null:=condition_null||' AND '; END IF;
   condition_join:=condition_join||'p.'||DBMS_ASSERT.SIMPLE_SQL_NAME(col.parent_col)||'=d.'||DBMS_ASSERT.SIMPLE_SQL_NAME(col.child_col);
   condition_null:=condition_null||'d.'||DBMS_ASSERT.SIMPLE_SQL_NAME(col.child_col)||' IS NOT NULL';
  END LOOP;
  EXECUTE IMMEDIATE 'SELECT COUNT(*) FROM '||DBMS_ASSERT.SIMPLE_SQL_NAME(c.table_name)||' d WHERE '||condition_null||' AND NOT EXISTS (SELECT 1 FROM '||DBMS_ASSERT.SIMPLE_SQL_NAME(c.parent_table)||' p WHERE '||condition_join||')' INTO n;
  DBMS_OUTPUT.PUT_LINE(c.table_name||' / '||c.constraint_name||' : '||n||' reference(s) orpheline(s)');total_erreurs:=total_erreurs+n;
 END LOOP;
 DBMS_OUTPUT.PUT_LINE('TOTAL REFERENCES ORPHELINES = '||total_erreurs);
END;
/

SELECT 'OBJETS_INVALIDES' AS controle, COUNT(*) AS anomalies FROM user_objects WHERE status<>'VALID'
UNION ALL SELECT 'CONTRAINTES_NON_VALIDEES',COUNT(*) FROM user_constraints WHERE status<>'ENABLED' OR validated<>'VALIDATED'
UNION ALL SELECT 'OCCUPATIONS_INVALIDES',COUNT(*) FROM locaux WHERE capacity<1 OR occupancy<0 OR occupancy>capacity;

SELECT s.id,s.name AS service,s.clientname AS proprietaire,
 e.id AS id_employe,TRIM(e.nom||' '||e.prenom) AS employe,
 s.invoicenumber AS facture,s.invoice_status AS statut_facture
FROM services s
LEFT JOIN employes e ON e.id_service=s.id
ORDER BY s.id,e.id;

SELECT id,name,id_service FROM locaux ORDER BY id;
SELECT lot,article,id_local FROM stock_lots ORDER BY lot;
SELECT id,name,owner,id_employe FROM medical_records ORDER BY id;
