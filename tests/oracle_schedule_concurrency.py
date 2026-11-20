"""Development-only RAWAND test: two real sessions compete for one employee.

Uses local OS authentication; no credentials. One clearly named fixture employee
is committed for visibility across sessions and removed in finally.
"""
import os
import subprocess
import time

SQLPLUS = r"C:\app\malek\product\21c\dbhomeXE\bin\sqlplus.exe"
ENV = dict(os.environ, NLS_LANG=".AL32UTF8")
PREFIX = """set echo off feedback off heading off pages 0 define off
whenever sqlerror exit failure rollback
connect / as sysdba
alter session set current_schema=RAWAND;
"""

def run(sql):
    p = subprocess.run([SQLPLUS, "-s", "/nolog"], input=PREFIX+sql+"\nexit\n",
                       text=True, encoding="utf-8", errors="replace", env=ENV,
                       capture_output=True, timeout=30, creationflags=subprocess.CREATE_NO_WINDOW)
    return p.returncode, p.stdout+p.stderr

code, out = run("""insert into employes(nom,prenom) values('ScheduleConcurrencyTest','Temporary');
select employes_seq.currval from dual;
commit;""")
assert code == 0, out
employee = int(next(line.strip() for line in out.splitlines() if line.strip().isdigit()))

def task(start, finish):
    return ("insert into planning_affectations(titre,competence_requise,debut,fin,responsable_id) "
            f"values('Concurrency fixture','Test','2035-01-01 {start}','2035-01-01 {finish}',{employee});\n")

try:
    for action in ("commit", "rollback"):
        code, out = run(f"delete from planning_affectations where responsable_id={employee};\ncommit;")
        assert code == 0, out
        first = subprocess.Popen([SQLPLUS, "-s", "/nolog"], stdin=subprocess.PIPE,
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                                 encoding="utf-8", errors="replace", env=ENV,
                                 creationflags=subprocess.CREATE_NO_WINDOW)
        first.stdin.write(PREFIX+task("09:00:00", "10:00:00")+
                          f"prompt FIRST_INSERTED\nexec dbms_session.sleep(4);\n{action};\nexit\n")
        first.stdin.flush()
        seen = ""
        while True:
            line = first.stdout.readline()
            seen += line
            if "FIRST_INSERTED" in line:
                break
            assert line, seen
        started = time.monotonic()
        code, out = run(task("09:30:00", "10:30:00")+"rollback;")
        elapsed = time.monotonic()-started
        first.wait(timeout=15)
        assert first.returncode == 0, seen+first.stdout.read()
        assert elapsed > 2.5, f"Employee lock did not serialize bookings: {elapsed}"
        if action == "commit":
            assert code != 0 and "ORA-20051" in out, out
            print("PASS: concurrent overlap waited for COMMIT, then was rejected (ORA-20051).")
        else:
            assert code == 0, out
            print("PASS: concurrent overlap waited for ROLLBACK, then was accepted and rolled back.")
finally:
    code, out = run(f"delete from planning_affectations where responsable_id={employee};\n"
                    f"delete from employes where id={employee} and nom='ScheduleConcurrencyTest';\ncommit;")
    assert code == 0, out
    print("Temporary employee and tasks removed.")
