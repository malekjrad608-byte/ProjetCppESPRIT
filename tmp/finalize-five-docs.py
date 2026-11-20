from pathlib import Path
p=Path('database/oracle/schema.sql')
old=p.read_text(encoding='utf-8-sig')
Path('database/oracle/schema_before_five_tables.sql.historical').write_text(old,encoding='utf-8')
ddl=Path('artifacts/five-tables-ddl-export.sql').read_text(encoding='utf-8-sig').replace('"RAWAND".','')
header='''-- Modele physique commun a cinq tables, export de RAWAND le 16/10/2026.
-- Installation dans un schema VIDE uniquement. Aucune suppression ni migration automatique.
whenever sqlerror exit failure rollback
set define off
set sqlblanklines on
DECLARE n NUMBER; BEGIN SELECT COUNT(*) INTO n FROM user_tables; IF n<>0 THEN RAISE_APPLICATION_ERROR(-20001,'Schema non vide : installation refusee'); END IF; END;
/
CREATE SEQUENCE services_seq;
CREATE SEQUENCE employes_seq;
CREATE SEQUENCE medical_records_seq;
'''
indexes='''
CREATE INDEX ix_employee_service ON employes(id_service);
CREATE INDEX ix_local_service ON locaux(id_service);
CREATE INDEX ix_medical_employe ON medical_records(id_employe);
CREATE INDEX ix_stock_local ON stock_lots(id_local);
CREATE INDEX services_date_idx ON services(datetime);
CREATE INDEX medical_date_idx ON medical_records(consultation_date);
'''
p.write_text(header+ddl+indexes+Path('database/oracle/five_table_rules.sql').read_text(encoding='utf-8-sig'),encoding='utf-8')
p=Path('database/oracle/migrations/005_five_tables.sql')
s=p.read_text(encoding='utf-8-sig').replace('@@../five_table_rules.sql',Path('database/oracle/five_table_rules.sql').read_text(encoding='utf-8-sig'))
p.write_text(s,encoding='utf-8')
p=Path('README.md');s=p.read_text(encoding='utf-8-sig');s=s.replace('# ANIWORLD — Services, Employés, Stock, Locaux, Dossiers médicaux','# Historique des versions précédentes',1)
intro='''# ANIWORLD — version active à cinq tables (16 octobre 2026)

Le mode Oracle actif utilise **RAWAND**, **localhost:1521/XE**, **CDB$ROOT**, avec le DSN ODBC 64 bits **Source_Projet2A18**. Lancer `Lancer ANIWORLD.cmd`. L'ancien schéma ANIWORLD_APP/XEPDB1 reste séparé.

## Modèle commun confirmé

| Table | Clé primaire | Relation obligatoire |
|---|---|---|
| SERVICES | ID | Au moins un employé dans son équipe |
| EMPLOYES | ID | ID_SERVICE → SERVICES.ID |
| LOCAUX | ID | ID_SERVICE → SERVICES.ID |
| STOCK_LOTS | LOT | ID_LOCAL → LOCAUX.ID |
| MEDICAL_RECORDS | ID | ID_EMPLOYE → EMPLOYES.ID |

Cardinalités retenues : Service (0,n)–Local (1,1), Local (0,n)–Stock (1,1), Service (1,n)–Employé (1,1), Employé (0,n)–Dossier médical (1,1). La relation Service–Local n'est donc pas plusieurs-à-plusieurs.

`SERVICES.ID_EMPLOYE` désigne un référent de l'équipe. La clé étrangère composée `(ID_EMPLOYE, ID) → EMPLOYES(ID, ID_SERVICE)`, différée jusqu'au COMMIT, garantit qu'un service possède au moins un employé de sa propre équipe. Créer un service et son premier employé se fait dans une seule transaction. Un employé appartient à un seul service dans ce modèle : une réaffectation remplace son rattachement, elle n'ajoute pas une deuxième affectation historique.

## Utilisation dans Qt

- **Services → Ajouter** : choisir « + Créer le premier employé », saisir son nom et prénom. Pour ajouter d'autres membres, ouvrir **Employés → Ajouter**, puis choisir le service.
- **Locaux** : choisir obligatoirement le service proposé.
- **Stock** : choisir obligatoirement le local du lot.
- **Dossiers médicaux** : choisir obligatoirement l'employé responsable.
- **Planning des employés** : une ligne par membre de l'équipe, avec l'horaire du service. Une réaffectation sur un créneau occupé est refusée. Le service d'origine doit conserver au moins un employé.
- Un élément encore référencé ne peut pas être supprimé ; Oracle protège les liens. Une facture émise est figée dans le service.

Il y a exactement **5 tables**. `PLANNING_AFFECTATIONS` est désormais une **vue en lecture seule**, calculée à partir de SERVICES et EMPLOYES. Elle ne stocke aucune tâche indépendante. Les comptes applicatifs existants ont été conservés dans `%LOCALAPPDATA%/ANIWORLD/ANIWORLD/accounts-local.json` (hachages salés, mêmes identifiants). La facture simple est incluse dans SERVICES ; le client reste un texte dans le service. L'ancien journal de mouvements et les anciennes données annexes sont conservés dans la sauvegarde, sans table supplémentaire active.

## Vérification et sauvegarde

Dans SQL Developer, connexion RAWAND, clic droit **Tables → Refresh**. Exécuter `database/oracle/verifier_cinq_tables.sql` avec **F5** : 5 tables, 0 anomalie de relation, 0 objet invalide attendu. Les tables LOCAUX, STOCK_LOTS et MEDICAL_RECORDS étaient vides avant la migration ; aucune donnée de démonstration n'a été ajoutée. L'employé existant #12 est maintenant rattaché au service existant #5.

Sauvegarde complète avant migration : `backups/before-five-tables-20261016/RAWAND_BEFORE_FIVE_20261016.DMP`, avec journal Data Pump, anciens sources, ancien exécutable et comptes. `database/oracle/schema.sql` contient le modèle physique pour un schéma vide ; `migrations/005_five_tables.sql` documente la migration ponctuelle déjà appliquée. Ne pas rejouer les migrations 001–004 sur ce modèle.

Les sections suivantes sont historiques ; elles ne décrivent pas la version Oracle active à cinq tables. Les modes `--demo` / `--local` restent séparés et conservent leur ancien stockage SQLite/JSON.

---

'''
p.write_text(intro+s,encoding='utf-8')
p=Path('src/database/databasemanager.cpp');s=p.read_text(encoding='utf-8-sig');s=s.replace('m_error=query.lastError().nativeErrorCode().contains("2292") ? QStringLiteral("Ce service est lié à une consultation. Conservez-le ou réaffectez la consultation avant suppression.") : query.lastError().text();','m_error=isOracle()?FiveTableStore::friendlyError(query.lastError().text()):query.lastError().text();');p.write_text(s,encoding='utf-8')
