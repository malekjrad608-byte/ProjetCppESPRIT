WHENEVER SQLERROR EXIT FAILURE ROLLBACK
SET DEFINE OFF
SET SQLBLANKLINES ON
SET SERVEROUTPUT ON
DECLARE n NUMBER; actual VARCHAR2(4000); BEGIN
 SELECT COUNT(*) INTO n FROM user_tables; IF n<>5 THEN RAISE_APPLICATION_ERROR(-20001,'Cinq tables exigees'); END IF;
 SELECT LISTAGG(column_name,',') WITHIN GROUP(ORDER BY column_id) INTO actual FROM user_tab_columns WHERE table_name='SERVICE';
 IF actual<>'ID_SERV,REF_SERVICE,CATEGORIE,STATUT,DATE_HEUR,DUREE,TARIF,PRESTATION' OR actual IS NULL THEN RAISE_APPLICATION_ERROR(-20002,'Colonnes incorrectes SERVICE: '||actual); END IF;
 DBMS_OUTPUT.PUT_LINE('SERVICE('||actual||')');
 SELECT COUNT(*) INTO n FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name WHERE c.table_name='SERVICE' AND c.constraint_type='P' AND cc.column_name='ID_SERV' AND c.status='ENABLED' AND c.validated='VALIDATED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20003,'Cle SERVICE.ID_SERV absente'); END IF;
 SELECT COUNT(*) INTO n FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name WHERE c.table_name='SERVICE' AND c.constraint_type='U' AND cc.column_name='REF_SERVICE' AND c.status='ENABLED' AND c.validated='VALIDATED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20003,'Cle SERVICE.REF_SERVICE absente'); END IF;
 SELECT COUNT(*) INTO n FROM user_triggers WHERE table_name='SERVICE' AND trigger_name='SERVICE_BI' AND status='ENABLED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20004,'Generateur SERVICE absent'); END IF;
 SELECT LISTAGG(column_name,',') WITHIN GROUP(ORDER BY column_id) INTO actual FROM user_tab_columns WHERE table_name='LOCAL';
 IF actual<>'ID_LOCAL,NOM_LOCAL,TYPE,ETAGE,CAPACITE,SPECIALITE,ETAT,ID_SERV' OR actual IS NULL THEN RAISE_APPLICATION_ERROR(-20002,'Colonnes incorrectes LOCAL: '||actual); END IF;
 DBMS_OUTPUT.PUT_LINE('LOCAL('||actual||')');
 SELECT COUNT(*) INTO n FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name WHERE c.table_name='LOCAL' AND c.constraint_type='P' AND cc.column_name='ID_LOCAL' AND c.status='ENABLED' AND c.validated='VALIDATED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20003,'Cle LOCAL.ID_LOCAL absente'); END IF;
 SELECT COUNT(*) INTO n FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name WHERE c.table_name='LOCAL' AND c.constraint_type='U' AND cc.column_name='NOM_LOCAL' AND c.status='ENABLED' AND c.validated='VALIDATED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20003,'Cle LOCAL.NOM_LOCAL absente'); END IF;
 SELECT COUNT(*) INTO n FROM user_triggers WHERE table_name='LOCAL' AND trigger_name='LOCAL_BI' AND status='ENABLED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20004,'Generateur LOCAL absent'); END IF;
 SELECT LISTAGG(column_name,',') WITHIN GROUP(ORDER BY column_id) INTO actual FROM user_tab_columns WHERE table_name='STOCK';
 IF actual<>'ID_ARTICLE,ARTICLE,NOM_ARTICLE,LOT,QUANTITE_DISP,TYPE_ARTICLE,SEUIL_MIN,DATE_ECHEANCE,ID_LOCAL' OR actual IS NULL THEN RAISE_APPLICATION_ERROR(-20002,'Colonnes incorrectes STOCK: '||actual); END IF;
 DBMS_OUTPUT.PUT_LINE('STOCK('||actual||')');
 SELECT COUNT(*) INTO n FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name WHERE c.table_name='STOCK' AND c.constraint_type='P' AND cc.column_name='ID_ARTICLE' AND c.status='ENABLED' AND c.validated='VALIDATED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20003,'Cle STOCK.ID_ARTICLE absente'); END IF;
 SELECT COUNT(*) INTO n FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name WHERE c.table_name='STOCK' AND c.constraint_type='U' AND cc.column_name='ARTICLE' AND c.status='ENABLED' AND c.validated='VALIDATED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20003,'Cle STOCK.ARTICLE absente'); END IF;
 SELECT COUNT(*) INTO n FROM user_triggers WHERE table_name='STOCK' AND trigger_name='STOCK_BI' AND status='ENABLED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20004,'Generateur STOCK absent'); END IF;
 SELECT LISTAGG(column_name,',') WITHIN GROUP(ORDER BY column_id) INTO actual FROM user_tab_columns WHERE table_name='EMPLOYER';
 IF actual<>'ID_EMP,NOM,PRENOM,CIN,DATE_NAISS,ADRESSE,TEL,POSTE,COURRIEL,COMPETENCES,DATE_EMBAUCHE,TYPE_CONTRAT,SALAIRE,CHARGE,DISPONIBLE,DISPO_REPOS,ID_SERV' OR actual IS NULL THEN RAISE_APPLICATION_ERROR(-20002,'Colonnes incorrectes EMPLOYER: '||actual); END IF;
 DBMS_OUTPUT.PUT_LINE('EMPLOYER('||actual||')');
 SELECT COUNT(*) INTO n FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name WHERE c.table_name='EMPLOYER' AND c.constraint_type='P' AND cc.column_name='ID_EMP' AND c.status='ENABLED' AND c.validated='VALIDATED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20003,'Cle EMPLOYER.ID_EMP absente'); END IF;
 SELECT COUNT(*) INTO n FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name WHERE c.table_name='EMPLOYER' AND c.constraint_type='U' AND cc.column_name='CIN' AND c.status='ENABLED' AND c.validated='VALIDATED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20003,'Cle EMPLOYER.CIN absente'); END IF;
 SELECT COUNT(*) INTO n FROM user_triggers WHERE table_name='EMPLOYER' AND trigger_name='EMPLOYER_BI' AND status='ENABLED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20004,'Generateur EMPLOYER absent'); END IF;
 SELECT LISTAGG(column_name,',') WITHIN GROUP(ORDER BY column_id) INTO actual FROM user_tab_columns WHERE table_name='MEDICAL';
 IF actual<>'ID_DOSS,DATE_CONSULTATION,MOTIF,DIAGNOSTIC,ID_ANIMAL,NOM_ANIMAL,ESPECE,RACE,AGE,SEXE,PROPRIETAIRE,ID_EMP' OR actual IS NULL THEN RAISE_APPLICATION_ERROR(-20002,'Colonnes incorrectes MEDICAL: '||actual); END IF;
 DBMS_OUTPUT.PUT_LINE('MEDICAL('||actual||')');
 SELECT COUNT(*) INTO n FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name WHERE c.table_name='MEDICAL' AND c.constraint_type='P' AND cc.column_name='ID_DOSS' AND c.status='ENABLED' AND c.validated='VALIDATED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20003,'Cle MEDICAL.ID_DOSS absente'); END IF;
 SELECT COUNT(*) INTO n FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name WHERE c.table_name='MEDICAL' AND c.constraint_type='U' AND cc.column_name='ID_ANIMAL' AND c.status='ENABLED' AND c.validated='VALIDATED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20003,'Cle MEDICAL.ID_ANIMAL absente'); END IF;
 SELECT COUNT(*) INTO n FROM user_triggers WHERE table_name='MEDICAL' AND trigger_name='MEDICAL_BI' AND status='ENABLED'; IF n<>1 THEN RAISE_APPLICATION_ERROR(-20004,'Generateur MEDICAL absent'); END IF;
 SELECT COUNT(*) INTO n FROM user_objects WHERE status<>'VALID'; IF n<>0 THEN RAISE_APPLICATION_ERROR(-20005,'Objets invalides'); END IF;
 DBMS_OUTPUT.PUT_LINE('OK: colonnes exactes, ordre exact, 5 PK, 5 UNIQUE, 5 generateurs.');
END;
/
@C:/Users/malek/Documents/gestion_service/database/oracle/verifier_mld_cinq_gestions.sql
