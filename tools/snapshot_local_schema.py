"""Read-only snapshot of the four affected business tables and live Oracle metadata."""
from pathlib import Path
import subprocess,os,json,sys
ROOT=Path(__file__).resolve().parents[1]
out=ROOT/'artifacts/local-links-20261015'/sys.argv[1];out.mkdir(parents=True,exist_ok=True)
exe=r'C:\app\malek\product\21c\dbhomeXE\bin\sqlplus.exe'
env=dict(os.environ,NLS_LANG='.AL32UTF8')
def query(sql):
    script='set heading off feedback off pagesize 0 linesize 32767 long 10000000 longchunksize 32767 trimspool on\nwhenever sqlerror exit failure rollback\nalter session set container=XEPDB1;\n'+sql+';\nexit\n'
    p=subprocess.run([exe,'-S','/ as sysdba'],input=script.encode(),stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=env)
    if p.returncode: raise RuntimeError(p.stdout.decode('utf-8','replace')+p.stderr.decode('utf-8','replace'))
    return [json.loads(x) for x in p.stdout.decode('utf-8').splitlines() if x.strip()]
for name in ['LOCAUX','SERVICES','MEDICAL_RECORDS','STOCK_LOTS']:
    data=query(f'SELECT JSON_OBJECT(* RETURNING CLOB) FROM ANIWORLD_APP.{name}')
    (out/(name+'.json')).write_text(json.dumps(data,ensure_ascii=False,indent=2),encoding='utf-8')
    print(name,len(data),'rows')
metadata={}
metadata['columns']=query("SELECT JSON_OBJECT(*) FROM (SELECT RTRIM(table_name) table_name,RTRIM(column_name) column_name,column_id,RTRIM(data_type) data_type,char_length,char_used,data_precision,data_scale,nullable FROM dba_tab_columns WHERE owner='ANIWORLD_APP' ORDER BY table_name,column_id)")
metadata['constraints']=query("SELECT JSON_OBJECT(*) FROM (SELECT RTRIM(owner) owner,RTRIM(table_name) table_name,RTRIM(constraint_name) constraint_name,constraint_type,RTRIM(r_owner) r_owner,RTRIM(r_constraint_name) r_constraint_name,delete_rule,status,validated,RTRIM(search_condition_vc) search_condition_vc FROM dba_constraints WHERE owner='ANIWORLD_APP' ORDER BY table_name,constraint_name)")
metadata['constraint_columns']=query("SELECT JSON_OBJECT(*) FROM (SELECT RTRIM(table_name) table_name,RTRIM(constraint_name) constraint_name,RTRIM(column_name) column_name,position FROM dba_cons_columns WHERE owner='ANIWORLD_APP' ORDER BY constraint_name,position)")
(out/'metadata.json').write_text(json.dumps(metadata,ensure_ascii=False,indent=2),encoding='utf-8')
print('Metadata saved')
