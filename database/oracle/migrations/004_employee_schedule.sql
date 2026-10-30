-- Shared employee timetable. Additive; preserves all appointments and identities.
whenever sqlerror exit failure rollback
set define off
BEGIN
 IF NOT ((SYS_CONTEXT('USERENV','CURRENT_SCHEMA')='RAWAND' AND SYS_CONTEXT('USERENV','CON_NAME')='CDB$ROOT') OR
         (SYS_CONTEXT('USERENV','CURRENT_SCHEMA')='ANIWORLD_APP' AND SYS_CONTEXT('USERENV','CON_NAME')='XEPDB1')) THEN
  RAISE_APPLICATION_ERROR(-20001,'Schema applicatif non autorise');
 END IF;
END;
/
CREATE OR REPLACE PACKAGE employee_schedule AS
 PROCEDURE lock_employee(employee NUMBER);
 PROCEDURE assert_free(employee NUMBER, kind VARCHAR2, booking NUMBER, starts TIMESTAMP, ends TIMESTAMP);
END;
/
CREATE OR REPLACE PACKAGE BODY employee_schedule AS
 PROCEDURE lock_employee(employee NUMBER) IS locked_id NUMBER;
 BEGIN
  IF employee IS NOT NULL THEN
   SELECT id INTO locked_id FROM employes WHERE id=employee FOR UPDATE;
  END IF;
 END;
 PROCEDURE assert_free(employee NUMBER, kind VARCHAR2, booking NUMBER, starts TIMESTAMP, ends TIMESTAMP) IS
  n NUMBER;
 BEGIN
  IF employee IS NULL THEN RETURN; END IF;
  IF starts IS NULL OR ends IS NULL OR ends<=starts THEN
   RAISE_APPLICATION_ERROR(-20052,'Creneau employe invalide');
  END IF;
  SELECT COUNT(*) INTO n FROM (
   SELECT 'S' typ,id,
    TO_TIMESTAMP(REPLACE(SUBSTR(datetime,1,19),'T',' '),'YYYY-MM-DD HH24:MI:SS') d,
    TO_TIMESTAMP(REPLACE(SUBSTR(datetime,1,19),'T',' '),'YYYY-MM-DD HH24:MI:SS')+NUMTODSINTERVAL(duration,'MINUTE') f
   FROM services WHERE id_employe=employee AND status<>'Annulé'
   UNION ALL
   SELECT 'T',id,TO_TIMESTAMP(debut,'YYYY-MM-DD HH24:MI:SS'),TO_TIMESTAMP(fin,'YYYY-MM-DD HH24:MI:SS')
   FROM planning_affectations WHERE responsable_id=employee
  ) WHERE NOT (typ=kind AND id=booking) AND d<ends AND starts<f;
  IF n>0 THEN RAISE_APPLICATION_ERROR(-20051,'Employe deja occupe : conflit entre services ou taches'); END IF;
 END;
END;
/
CREATE OR REPLACE TRIGGER services_schedule_guard
FOR INSERT OR UPDATE OF id_employe,datetime,duration,status ON services
COMPOUND TRIGGER
 TYPE booking_type IS RECORD(employee NUMBER,id NUMBER,d TIMESTAMP,f TIMESTAMP);
 TYPE booking_list IS TABLE OF booking_type INDEX BY PLS_INTEGER;
 bookings booking_list; n PLS_INTEGER:=0;
 BEFORE EACH ROW IS
 BEGIN
  IF :NEW.id_employe IS NOT NULL AND :NEW.status<>'Annulé' THEN
   employee_schedule.lock_employee(:NEW.id_employe);
  END IF;
 END BEFORE EACH ROW;
 AFTER EACH ROW IS
 BEGIN
  IF :NEW.id_employe IS NOT NULL AND :NEW.status<>'Annulé' THEN
   n:=n+1;bookings(n).employee:=:NEW.id_employe;bookings(n).id:=:NEW.id;
   bookings(n).d:=TO_TIMESTAMP(REPLACE(SUBSTR(:NEW.datetime,1,19),'T',' '),'YYYY-MM-DD HH24:MI:SS');
   bookings(n).f:=bookings(n).d+NUMTODSINTERVAL(:NEW.duration,'MINUTE');
  END IF;
 END AFTER EACH ROW;
 AFTER STATEMENT IS
 BEGIN
  FOR i IN 1..n LOOP
   employee_schedule.assert_free(bookings(i).employee,'S',bookings(i).id,bookings(i).d,bookings(i).f);
  END LOOP;
 END AFTER STATEMENT;
END;
/
CREATE OR REPLACE TRIGGER tasks_schedule_guard
FOR INSERT OR UPDATE OF responsable_id,debut,fin ON planning_affectations
COMPOUND TRIGGER
 TYPE booking_type IS RECORD(employee NUMBER,id NUMBER,d TIMESTAMP,f TIMESTAMP);
 TYPE booking_list IS TABLE OF booking_type INDEX BY PLS_INTEGER;
 bookings booking_list; n PLS_INTEGER:=0;
 BEFORE EACH ROW IS
 BEGIN
  employee_schedule.lock_employee(:NEW.responsable_id);
 END BEFORE EACH ROW;
 AFTER EACH ROW IS
 BEGIN
  n:=n+1;bookings(n).employee:=:NEW.responsable_id;bookings(n).id:=:NEW.id;
  bookings(n).d:=TO_TIMESTAMP(:NEW.debut,'YYYY-MM-DD HH24:MI:SS');
  bookings(n).f:=TO_TIMESTAMP(:NEW.fin,'YYYY-MM-DD HH24:MI:SS');
 END AFTER EACH ROW;
 AFTER STATEMENT IS
 BEGIN
  FOR i IN 1..n LOOP
   employee_schedule.assert_free(bookings(i).employee,'T',bookings(i).id,bookings(i).d,bookings(i).f);
  END LOOP;
 END AFTER STATEMENT;
END;
/
DECLARE n NUMBER;
BEGIN
 SELECT COUNT(*) INTO n FROM all_errors WHERE owner=SYS_CONTEXT('USERENV','CURRENT_SCHEMA')
 AND name IN ('EMPLOYEE_SCHEDULE','SERVICES_SCHEDULE_GUARD','TASKS_SCHEDULE_GUARD');
 IF n>0 THEN RAISE_APPLICATION_ERROR(-20053,'Compilation planning invalide : consulter ALL_ERRORS'); END IF;
 MERGE INTO integration_migrations t USING (SELECT 'employee-schedule-v4' name FROM dual) s ON (t.name=s.name)
 WHEN NOT MATCHED THEN INSERT(name) VALUES(s.name);
END;
/
COMMIT;
