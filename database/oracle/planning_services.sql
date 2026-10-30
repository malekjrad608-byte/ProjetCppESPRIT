-- Connexion RAWAND / XE. Lecture seule, Ctrl+Entree.
-- Le planning utilise directement l'affectation enregistree dans EMPLOYES.ID_SERVICE.
SELECT s.id AS id_service,
       s.name AS service,
       e.id AS id_employe,
       CASE WHEN e.id IS NULL THEN 'Non affecte'
            ELSE TRIM(e.nom || ' ' || e.prenom) END AS employe,
       s.datetime AS debut,
       TO_CHAR(TO_TIMESTAMP(SUBSTR(s.datetime,1,19),'YYYY-MM-DD"T"HH24:MI:SS')
               + NUMTODSINTERVAL(s.duration,'MINUTE'),'DD/MM/YYYY HH24:MI') AS fin,
       s.duration AS duree_minutes,
       s.clientname AS client,
       s.status AS statut
FROM services s
LEFT JOIN employes e ON e.id_service=s.id
ORDER BY s.datetime,s.id;
