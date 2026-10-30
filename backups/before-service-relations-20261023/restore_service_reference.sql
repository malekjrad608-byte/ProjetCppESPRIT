whenever sqlerror exit failure rollback
ALTER TABLE services ADD (id_employe NUMBER(10));
UPDATE services SET id_employe=12 WHERE id=5;
UPDATE services SET id_employe=33 WHERE id=30;
COMMIT;
ALTER TABLE services MODIFY (id_employe NOT NULL);
ALTER TABLE employes ADD CONSTRAINT uq_employee_team UNIQUE(id,id_service);
ALTER TABLE services ADD CONSTRAINT fk_service_team FOREIGN KEY(id_employe,id) REFERENCES employes(id,id_service) DEFERRABLE INITIALLY DEFERRED;
