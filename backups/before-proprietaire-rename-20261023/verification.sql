-- Open in SQL Developer with the ANIWORLD - XEPDB1 connection, then run F5.
SELECT USER AS utilisateur, SYS_CONTEXT('USERENV','CON_NAME') AS base FROM dual;
SELECT table_name FROM user_tables ORDER BY table_name;
SELECT 'SERVICES' AS module,COUNT(*) AS lignes FROM services
UNION ALL SELECT 'EMPLOYES',COUNT(*) FROM employes
UNION ALL SELECT 'PLANNING',COUNT(*) FROM planning_affectations
UNION ALL SELECT 'DOSSIERS MEDICAUX',COUNT(*) FROM medical_records
UNION ALL SELECT 'COMPTES',COUNT(*) FROM accounts
UNION ALL SELECT 'LOCAUX',COUNT(*) FROM locaux
UNION ALL SELECT 'LOTS DE STOCK',COUNT(*) FROM stock_lots;
SELECT id,name,clientName,price,invoiceNumber FROM services ORDER BY id;
SELECT id,animal_id,name,owner,consultation_date FROM medical_records ORDER BY id;
SELECT lot,article,quantity,expiry FROM stock_lots ORDER BY lot;
SELECT id,lot,quantity_before,quantity_after,action,changed_at,changed_by FROM stock_movements ORDER BY id;
SELECT object_name,object_type,status FROM user_objects WHERE status<>'VALID';
