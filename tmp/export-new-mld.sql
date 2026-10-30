set echo off feedback off heading off pages 0 lines 32767 long 1000000 longchunksize 32767 trimspool on
BEGIN
 DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'SQLTERMINATOR',TRUE);
 DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'SEGMENT_ATTRIBUTES',FALSE);
 DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'EMIT_SCHEMA',FALSE);
 DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'REF_CONSTRAINTS',FALSE);
END;
/
spool C:/Users/malek/Documents/gestion_service/database/oracle/schema.sql
prompt -- Modele du 30/10/2026. Installation dans un schema VIDE uniquement.
prompt whenever sqlerror exit failure rollback
prompt set define off
prompt set sqlblanklines on
prompt DECLARE n NUMBER; BEGIN SELECT COUNT(*) INTO n FROM user_tables; IF n<>0 THEN RAISE_APPLICATION_ERROR(-20001,'Schema non vide'); END IF; END;
prompt /
prompt CREATE SEQUENCE SERVICES_SEQ START WITH 1 INCREMENT BY 1 NOCYCLE;
prompt CREATE SEQUENCE LOCAL_SEQ START WITH 1 INCREMENT BY 1 NOCYCLE;
prompt CREATE SEQUENCE STOCK_SEQ START WITH 1 INCREMENT BY 1 NOCYCLE;
prompt CREATE SEQUENCE EMPLOYES_SEQ START WITH 1 INCREMENT BY 1 NOCYCLE;
prompt CREATE SEQUENCE MEDICAL_RECORDS_SEQ START WITH 1 INCREMENT BY 1 NOCYCLE;
select dbms_metadata.get_ddl('TABLE','SERVICE') from dual;
select dbms_metadata.get_ddl('TABLE','LOCAL') from dual;
select dbms_metadata.get_ddl('TABLE','EMPLOYER') from dual;
select dbms_metadata.get_ddl('TABLE','STOCK') from dual;
select dbms_metadata.get_ddl('TABLE','MEDICAL') from dual;
select dbms_metadata.get_dependent_ddl('REF_CONSTRAINT','LOCAL') from dual;
select dbms_metadata.get_dependent_ddl('REF_CONSTRAINT','STOCK') from dual;
select dbms_metadata.get_dependent_ddl('REF_CONSTRAINT','EMPLOYER') from dual;
select dbms_metadata.get_dependent_ddl('REF_CONSTRAINT','MEDICAL') from dual;
select dbms_metadata.get_ddl('INDEX','LOCAUX_NAME_CI') from dual;
select dbms_metadata.get_ddl('VIEW','SERVICES') from dual;
select dbms_metadata.get_ddl('VIEW','LOCAUX') from dual;
select dbms_metadata.get_ddl('VIEW','STOCK_LOTS') from dual;
select dbms_metadata.get_ddl('VIEW','EMPLOYES') from dual;
select dbms_metadata.get_ddl('VIEW','MEDICAL_RECORDS') from dual;
select dbms_metadata.get_ddl('VIEW','PLANNING_AFFECTATIONS') from dual;
select dbms_metadata.get_ddl('TRIGGER','SERVICE_BI') from dual;
select dbms_metadata.get_ddl('TRIGGER','LOCAL_BI') from dual;
select dbms_metadata.get_ddl('TRIGGER','STOCK_BI') from dual;
select dbms_metadata.get_ddl('TRIGGER','EMPLOYER_BI') from dual;
select dbms_metadata.get_ddl('TRIGGER','MEDICAL_BI') from dual;
select dbms_metadata.get_ddl('TRIGGER','EMPLOYEE_TRANSFER_GUARD') from dual;
spool off
exit
