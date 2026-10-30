-- Connexion RAWAND / XE dans SQL Developer, puis F5.
set pagesize 100 linesize 200
prompt === Connexion ===
SELECT USER utilisateur,SYS_CONTEXT('USERENV','CON_NAME') conteneur FROM dual;
prompt === Exactement 5 tables ===
SELECT COUNT(*) nombre_tables FROM user_tables;
SELECT table_name FROM user_tables ORDER BY table_name;
prompt === Relations ===
SELECT c.table_name enfant,cc.column_name colonne,p.table_name parent,c.constraint_name,c.status,c.validated
FROM user_constraints c JOIN user_cons_columns cc ON cc.constraint_name=c.constraint_name
JOIN user_constraints p ON p.constraint_name=c.r_constraint_name
WHERE c.constraint_type='R' ORDER BY c.table_name,cc.position;
prompt === Donnees ===
SELECT 'SERVICES' module,COUNT(*) nombre FROM services UNION ALL
SELECT 'EMPLOYES',COUNT(*) FROM employes UNION ALL
SELECT 'LOCAUX',COUNT(*) FROM locaux UNION ALL
SELECT 'STOCK_LOTS',COUNT(*) FROM stock_lots UNION ALL
SELECT 'MEDICAL_RECORDS',COUNT(*) FROM medical_records;
prompt === Services et equipes ===
SELECT s.id service_id,s.name service,e.id employe_id,e.nom,e.prenom FROM services s LEFT JOIN employes e ON e.id_service=s.id ORDER BY s.id,e.id;
prompt === Locaux ===
SELECT l.id,l.name local,s.name service FROM locaux l JOIN services s ON s.id=l.id_service ORDER BY l.id;
prompt === Stock ===
SELECT t.lot,t.article,t.quantity,l.name local FROM stock_lots t JOIN locaux l ON l.id=t.id_local ORDER BY t.lot;
prompt === Dossiers ===
SELECT m.id,m.name animal,e.nom,e.prenom FROM medical_records m JOIN employes e ON e.id=m.id_employe ORDER BY m.id;
prompt === Anomalies : chaque resultat doit valoir 0 ===
SELECT 'Service sans employe' controle,COUNT(*) anomalies FROM services s WHERE NOT EXISTS(SELECT 1 FROM employes e WHERE e.id_service=s.id)
UNION ALL SELECT 'Employe sans service',COUNT(*) FROM employes e WHERE NOT EXISTS(SELECT 1 FROM services s WHERE s.id=e.id_service)
UNION ALL SELECT 'Local sans service',COUNT(*) FROM locaux l WHERE NOT EXISTS(SELECT 1 FROM services s WHERE s.id=l.id_service)
UNION ALL SELECT 'Stock sans local',COUNT(*) FROM stock_lots t WHERE NOT EXISTS(SELECT 1 FROM locaux l WHERE l.id=t.id_local)
UNION ALL SELECT 'Dossier sans employe',COUNT(*) FROM medical_records m WHERE NOT EXISTS(SELECT 1 FROM employes e WHERE e.id=m.id_employe)
UNION ALL SELECT 'Service sans equipe',COUNT(*) FROM services s WHERE NOT EXISTS(SELECT 1 FROM employes e WHERE e.id_service=s.id);
prompt === Objets invalides : 0 ligne attendue ===
SELECT object_name,object_type FROM user_objects WHERE status<>'VALID';
prompt === Planning calcule : vue, pas une table ===
SELECT object_name,object_type FROM user_objects WHERE object_name='PLANNING_AFFECTATIONS';
SELECT * FROM planning_affectations ORDER BY debut,responsable_id;
