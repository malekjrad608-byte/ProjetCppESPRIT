-- Diagnostic en lecture seule : exécuter avec le même compte Oracle que connection.cpp.
SELECT USER AS utilisateur, SYS_CONTEXT('USERENV','CON_NAME') AS conteneur FROM DUAL;

SELECT object_name, object_type, status
FROM user_objects
WHERE object_name IN ('SERVICE','SERVICES','EMPLOYER','EMPLOYES','MEDICAL','SERVICES_SEQ')
ORDER BY object_type, object_name;

SELECT table_name, column_name, data_type
FROM user_tab_columns
WHERE table_name IN ('SERVICE','SERVICES','EMPLOYER','EMPLOYES','MEDICAL')
ORDER BY table_name, column_id;

-- Contrat de lecture du tableau Services (0 ligne, aucune donnée modifiée).
SELECT id, name, description, responsible, category, dateTime,
       status, duration, price, clientName, invoiceNumber, invoiceDate,
       (SELECT MIN(e.id) FROM employes e WHERE e.id_service=services.id) AS employee_id
FROM services WHERE 1=0;
