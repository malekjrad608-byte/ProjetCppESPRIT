-- Controle strict des RELATIONS du MLD fourni par l'enseignante.
-- Lecture seule. Ce controle ne certifie pas l'identite des noms/attributs.
-- Tables physiques du modele du 30/10/2026.
WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET SERVEROUTPUT ON
SET LINESIZE 220
SET PAGESIZE 100

SELECT USER AS compte, SYS_CONTEXT('USERENV','CON_NAME') AS base FROM dual;

DECLARE
  n NUMBER;
  PROCEDURE verifier_relation(enfant VARCHAR2, colonne VARCHAR2, parent VARCHAR2, cle_parent VARCHAR2) IS
    relations NUMBER;
    obligatoires NUMBER;
    orphelins NUMBER;
  BEGIN
    SELECT COUNT(*) INTO relations
    FROM user_constraints fk
    JOIN user_cons_columns fc ON fc.constraint_name=fk.constraint_name
    JOIN user_constraints pk ON pk.constraint_name=fk.r_constraint_name
    JOIN user_cons_columns pc ON pc.constraint_name=pk.constraint_name AND pc.position=fc.position
    WHERE fk.constraint_type='R' AND fk.table_name=enfant AND fc.column_name=colonne
      AND fk.r_owner=USER AND pk.table_name=parent AND pc.column_name=cle_parent
      AND pk.constraint_type='P' AND pk.status='ENABLED' AND pk.validated='VALIDATED'
      AND fk.status='ENABLED' AND fk.validated='VALIDATED'
      AND (SELECT COUNT(*) FROM user_cons_columns x WHERE x.constraint_name=fk.constraint_name)=1;
    IF relations<>1 THEN
      RAISE_APPLICATION_ERROR(-20001,'Relation absente ou incorrecte : '||enfant||'.'||colonne||' -> '||parent||'.'||cle_parent);
    END IF;
    SELECT COUNT(*) INTO obligatoires FROM user_tab_columns
    WHERE table_name=enfant AND column_name=colonne AND nullable='N';
    IF obligatoires<>1 THEN
      RAISE_APPLICATION_ERROR(-20002,'Cle etrangere facultative : '||enfant||'.'||colonne);
    END IF;
    SELECT COUNT(*) INTO relations
    FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name
    WHERE c.table_name=enfant AND c.constraint_type IN ('U','P') AND cc.column_name=colonne
      AND (SELECT COUNT(*) FROM user_cons_columns x WHERE x.constraint_name=c.constraint_name)=1;
    IF relations<>0 THEN
      RAISE_APPLICATION_ERROR(-20003,'La cle etrangere doit autoriser plusieurs enfants : '||enfant||'.'||colonne);
    END IF;
    EXECUTE IMMEDIATE 'SELECT COUNT(*) FROM '||DBMS_ASSERT.SIMPLE_SQL_NAME(enfant)||' e WHERE NOT EXISTS (SELECT 1 FROM '
      ||DBMS_ASSERT.SIMPLE_SQL_NAME(parent)||' p WHERE p.'||DBMS_ASSERT.SIMPLE_SQL_NAME(cle_parent)||'=e.'||DBMS_ASSERT.SIMPLE_SQL_NAME(colonne)||')' INTO orphelins;
    IF orphelins<>0 THEN
      RAISE_APPLICATION_ERROR(-20004,'References orphelines : '||enfant);
    END IF;
    DBMS_OUTPUT.PUT_LINE('OK : '||enfant||'.'||colonne||' -> '||parent||'.'||cle_parent||' ; obligatoire ; 0 orphelin');
  END;
BEGIN
  SELECT COUNT(*) INTO n FROM user_tables;
  IF n<>5 THEN RAISE_APPLICATION_ERROR(-20005,'Le schema doit contenir exactement cinq tables'); END IF;
  SELECT COUNT(*) INTO n FROM user_tables
  WHERE table_name IN ('SERVICE','LOCAL','STOCK','EMPLOYER','MEDICAL');
  IF n<>5 THEN RAISE_APPLICATION_ERROR(-20006,'Une gestion manque dans le schema'); END IF;
  SELECT COUNT(*) INTO n FROM user_constraints WHERE constraint_type='R';
  IF n<>4 THEN RAISE_APPLICATION_ERROR(-20007,'Le MLD exige exactement quatre relations entre les cinq tables'); END IF;
  verifier_relation('LOCAL','ID_SERV','SERVICE','ID_SERV');
  verifier_relation('STOCK','ID_LOCAL','LOCAL','ID_LOCAL');
  verifier_relation('EMPLOYER','ID_SERV','SERVICE','ID_SERV');
  verifier_relation('MEDICAL','ID_EMP','EMPLOYER','ID_EMP');
  DBMS_OUTPUT.PUT_LINE('OK : les cinq gestions forment un seul ensemble relie.');
  DBMS_OUTPUT.PUT_LINE('STOCK -> LOCAL -> SERVICE <- EMPLOYER <- MEDICAL');
END;
/

-- Les cinq tables dans une seule jointure. Les NULL indiquent des donnees
-- non encore saisies, pas une absence de cle etrangere dans le schema.
SELECT s.id AS service_id, l.id AS local_id, st.lot AS stock_lot,
       e.id AS employe_id, m.id AS medical_id
FROM services s
LEFT JOIN locaux l ON l.id_service=s.id
LEFT JOIN stock_lots st ON st.id_local=l.id
LEFT JOIN employes e ON e.id_service=s.id
LEFT JOIN medical_records m ON m.id_employe=e.id
ORDER BY s.id,l.id,st.lot,e.id,m.id;
