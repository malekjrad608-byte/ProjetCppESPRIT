"""Export the five live Oracle tables without changing the source schema.

Uses local Oracle OS authentication. No application passwords are exported.
The optional restore check runs the actual SQL file in a disposable schema.
"""
from pathlib import Path
from datetime import datetime, date
import argparse
import hashlib
import os
import re
import secrets
import subprocess

import oracledb

TABLES = ['SERVICE', 'LOCAL', 'EMPLOYER', 'STOCK', 'MEDICAL']
KEYS = dict(zip(TABLES, ['ID_SERV', 'ID_LOCAL', 'ID_EMP', 'ID_ARTICLE', 'ID_DOSS']))
ORACLE_BIN = Path('C:/app/malek/product/21c/dbhomeXE/bin')


def text_literal(value, clob=False):
    # No physical newlines inside strings: SQL Developer/SQL*Plus can run it with F5.
    parts = []
    for i, line in enumerate(re.split(r'(\r|\n)', value)):
        if line in ('\r', '\n'):
            parts.append('CHR(%d)' % ord(line))
        elif line:
            for offset in range(0, len(line), 400):
                parts.append("'" + line[offset:offset+400].replace("'", "''") + "'")
    if not parts:
        return 'EMPTY_CLOB()' if clob else 'NULL'
    if clob:
        parts[0] = 'TO_CLOB(' + parts[0] + ')'
    return '\n || '.join(parts)


def literal(value, data_type):
    if value is None:
        return 'NULL'
    if data_type in ('CLOB', 'NCLOB'):
        return text_literal(value, True)
    if isinstance(value, str):
        return text_literal(value)
    if isinstance(value, (datetime, date)):
        return "TO_TIMESTAMP('%s','YYYY-MM-DD HH24:MI:SS.FF6')" % value.strftime('%Y-%m-%d %H:%M:%S.%f')
    return str(value)


def rows(cursor, owner, table):
    result = cursor.execute(f'SELECT * FROM "{owner}"."{table}" ORDER BY "{KEYS[table]}"').fetchall()
    return [tuple(v.read() if hasattr(v, 'read') else v for v in row) for row in result]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--verify', action='store_true')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    oracledb.init_oracle_client(lib_dir=str(ORACLE_BIN))
    db = oracledb.connect(mode=oracledb.AUTH_MODE_SYSDBA)
    q = db.cursor()
    for option in ['SEGMENT_ATTRIBUTES', 'STORAGE', 'TABLESPACE', 'REF_CONSTRAINTS']:
        q.execute('begin DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,:p,false);end;', p=option)
    q.execute("begin DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,'SQLTERMINATOR',true);end;")

    def ddl(kind, name):
        value = q.execute('select DBMS_METADATA.GET_DDL(:k,:n,\'RAWAND\') from dual', k=kind, n=name).fetchone()[0].read()
        value = value.replace('"RAWAND".', '')
        value = re.sub(r'\bRAWAND\s*\.', '', value, flags=re.IGNORECASE)
        return value.strip() + '\n'

    sections = ["""-- ANIWORLD : sauvegarde complete des 5 gestions
-- Export depuis RAWAND; import dans un schema utilisateur VIDE.
-- SQL Developer : ouvrir ce fichier UTF-8, selectionner la connexion, puis F5.
-- Ne pas executer dans SYS/SYSTEM ou dans la base d'origine deja remplie.
-- Aucun DROP, aucun mot de passe et aucune creation d'utilisateur.
SET DEFINE OFF
SET SQLBLANKLINES ON
SET SERVEROUTPUT ON
WHENEVER SQLERROR EXIT SQL.SQLCODE ROLLBACK
ALTER SESSION SET NLS_NUMERIC_CHARACTERS = '.,';
BEGIN
  IF SYS_CONTEXT('USERENV','SESSION_USER') IN ('SYS','SYSTEM') THEN
    RAISE_APPLICATION_ERROR(-20070,'Choisir un schema utilisateur vide, pas SYS/SYSTEM.');
  END IF;
  FOR t IN (SELECT table_name FROM user_tables
            WHERE table_name IN ('SERVICE','LOCAL','EMPLOYER','STOCK','MEDICAL')) LOOP
    RAISE_APPLICATION_ERROR(-20071,'Table deja presente : ' || t.table_name || '. Import arrete.');
  END LOOP;
END;
/
"""]
    objects = {}
    for kind in ['SEQUENCE', 'VIEW', 'TRIGGER']:
        objects[kind] = [r[0] for r in q.execute('select object_name from all_objects where owner=\'RAWAND\' and object_type=:k order by object_name', k=kind)]
    columns = {}
    for table in TABLES:
        columns[table] = q.execute("select column_name,data_type from all_tab_columns where owner='RAWAND' and table_name=:t order by column_id", t=table).fetchall()
    for name in objects['SEQUENCE']:
        sections.append(ddl('SEQUENCE', name))
    for table in TABLES:
        sections.append(ddl('TABLE', table))

    # A consistent read-only snapshot of all five tables.
    db.commit()
    q.execute('SET TRANSACTION READ ONLY')
    snapshot = {table: rows(q, 'RAWAND', table) for table in TABLES}
    db.rollback()
    for table in TABLES:
        sections.append(f'-- Donnees {table} : {len(snapshot[table])} lignes\n')
        names = ','.join('"' + c[0] + '"' for c in columns[table])
        for row in snapshot[table]:
            values = ',\n  '.join(literal(v, c[1]) for v, c in zip(row, columns[table]))
            sections.append(f'INSERT INTO "{table}" ({names}) VALUES (\n  {values}\n);\n')
    sections.append('COMMIT;\n')
    # Foreign keys are installed after the complete data set.
    foreign_keys = [r[0] for r in q.execute("select constraint_name from all_constraints where owner='RAWAND' and constraint_type='R' order by constraint_name")]
    for name in foreign_keys:
        sections.append(ddl('REF_CONSTRAINT', name))
    indexes = [r[0] for r in q.execute("""select index_name from all_indexes i where owner='RAWAND'
        and not exists(select 1 from all_constraints c where c.owner=i.owner and c.index_name=i.index_name)
        order by index_name""")]
    for name in indexes:
        sections.append(ddl('INDEX', name))
    for kind in ['VIEW', 'TRIGGER']:
        for name in objects[kind]:
            sections.append(ddl(kind, name))
    sections.append("""PROMPT === Verification des donnees importees ===
SELECT 'SERVICE' AS TABLE_NAME, COUNT(*) AS NB_LIGNES FROM SERVICE
UNION ALL SELECT 'LOCAL', COUNT(*) FROM LOCAL
UNION ALL SELECT 'EMPLOYER', COUNT(*) FROM EMPLOYER
UNION ALL SELECT 'STOCK', COUNT(*) FROM STOCK
UNION ALL SELECT 'MEDICAL', COUNT(*) FROM MEDICAL;
SELECT constraint_name, table_name, status, validated FROM user_constraints WHERE constraint_type='R';
SELECT trigger_name, status FROM user_triggers ORDER BY trigger_name;
PROMPT === Import ANIWORLD termine ===
""")
    target = args.output / 'ANIWORLD_5_GESTIONS.sql'
    target.write_text('\n'.join(sections), encoding='utf-8')
    report = ['Export Oracle ANIWORLD - ' + datetime.now().isoformat(timespec='seconds'),
              'Source : RAWAND', 'Encodage : UTF-8', 'Tables et lignes :']
    report += [f'  {t}: {len(snapshot[t])}' for t in TABLES]
    report += [f'Cles etrangeres: {len(foreign_keys)}', f'Sequences: {len(objects["SEQUENCE"])}',
               f'Triggers: {len(objects["TRIGGER"])} (etat ENABLED/DISABLED preserve)',
               f'Vues de compatibilite Qt: {len(objects["VIEW"])}']
    if args.verify:
        user = 'C##ANI_EXPORT_' + secrets.token_hex(4).upper()
        password = 'Verify' + secrets.token_hex(10)
        created = False
        try:
            q.execute(f'CREATE USER {user} IDENTIFIED BY "{password}" DEFAULT TABLESPACE USERS QUOTA 100M ON USERS')
            created = True
            q.execute(f'GRANT CREATE SESSION,CREATE TABLE,CREATE VIEW,CREATE TRIGGER,CREATE SEQUENCE TO {user}')
            env = dict(os.environ, NLS_LANG='.AL32UTF8')
            commands = f'connect {user}/"{password}"@localhost:1521/xe\n@"{target}"\nexit\n'
            run = subprocess.run([str(ORACLE_BIN/'sqlplus.exe'), '-L', '-s', '/nolog'], input=commands,
                                 text=True, encoding='utf-8', errors='replace', capture_output=True, env=env, timeout=120)
            (args.output/'verification_import.log').write_text(run.stdout+run.stderr, encoding='utf-8')
            if run.returncode or re.search(r'ORA-\d+|SP2-\d+', run.stdout+run.stderr):
                raise RuntimeError('Import SQLPlus failed; see verification_import.log')
            for table in TABLES:
                assert rows(q, user, table) == snapshot[table], f'Data mismatch: {table}'
            invalid = q.execute("select object_name,object_type from all_objects where owner=:o and status<>'VALID'",o=user).fetchall()
            assert not invalid, f'Invalid restored objects: {invalid}'
            assert q.execute("select count(*) from all_constraints where owner=:o and constraint_type='R' and status='ENABLED' and validated='VALIDATED'",o=user).fetchone()[0] == 4
            source_states = q.execute("select trigger_name,status from all_triggers where owner='RAWAND' order by trigger_name").fetchall()
            assert source_states == q.execute('select trigger_name,status from all_triggers where owner=:o order by trigger_name',o=user).fetchall()
            # Exercise each restored auto-ID trigger without retaining the test rows.
            for table in TABLES:
                row = list(snapshot[table][0])
                for i,(column,_) in enumerate(columns[table]):
                    if column == KEYS[table]: row[i] = None
                    elif column in ('REF_SERVICE','NOM_LOCAL','CIN','REF_ARTICLE','RAF_ANIMAL'):
                        row[i] = 'SRV-999' if column=='REF_SERVICE' else 'EXPORT-CHECK-' + table
                if table=='SERVICE':
                    used={r[1] for r in snapshot[table]}
                    row[1]=next(f'SRV-{n:03d}' for n in range(1000) if f'SRV-{n:03d}' not in used)
                out=q.var(oracledb.DB_TYPE_NUMBER)
                names=','.join('"'+c[0]+'"' for c in columns[table])
                q.execute(f'INSERT INTO "{user}"."{table}" ({names}) VALUES ('+','.join(':'+str(i+1) for i in range(len(row)))+f') RETURNING "{KEYS[table]}" INTO :out',row+[out])
                old_max=max(r[0] for r in snapshot[table])
                assert out.getvalue()[0] > old_max, f'Auto ID not above existing IDs: {table}'
                db.rollback()
            report += ['VERIFICATION REUSSIE : import reel via SQLPlus dans un schema temporaire.',
                       'Toutes les lignes et valeurs sont identiques (y compris CLOB et accents).',
                       '4 cles etrangeres actives et validees; tous les objets sont VALID.',
                       'Insertion sans ID reussie pour les 5 tables; nouveaux IDs superieurs aux IDs existants.']
        finally:
            db.rollback()
            if created:
                q.execute(f'DROP USER {user} CASCADE')
    report.append('SHA256 SQL: ' + hashlib.sha256(target.read_bytes()).hexdigest())
    (args.output/'VERIFICATION.txt').write_text('\n'.join(report)+'\n',encoding='utf-8')
    print('\n'.join(report))
    db.close()


if __name__ == '__main__':
    main()
