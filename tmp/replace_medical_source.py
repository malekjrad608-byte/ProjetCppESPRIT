from pathlib import Path
import shutil, sqlite3, os, json, hashlib

root=Path.cwd()
source=Path(r'C:\Users\malek\Downloads\Dossier_Medical_ANIWORLD (2)\Dossier_Medical_ANIWORLD')
out=root/'artifacts/medical-replacement'
before=out/'before'
before.mkdir(parents=True,exist_ok=True)
shutil.copytree(root/'src/medical',before/'medical',dirs_exist_ok=True)
shutil.copy2(root/'README.md',before/'README.md')
db=Path(os.environ['LOCALAPPDATA'])/'ANIWORLD/ANIWORLD/services.sqlite'
if db.exists():
    with sqlite3.connect(db) as src,sqlite3.connect(before/'services.sqlite') as backup:
        src.backup(backup)

# Re-import this supplied version, reusing only the existing integration adapters.
# Authentication, persistent storage and application navigation remain shared.
script=(root/'tmp/integrate_medical.py').read_text(encoding='utf-8-sig')
start=script.index('def write(')
end=script.index('# Shared authentication and navigation.')
stage=out/'adapted-source'
stage.mkdir(exist_ok=True)
exec('import re\n'+script[start:end],{'source':source,'dest':stage})

def amend(file,old,new):
    p=stage/file;s=p.read_text(encoding='utf8');assert old in s,(file,old)
    p.write_text(s.replace(old,new),encoding='utf8')
amend('medicalrecorddialog.cpp','    m_dateConsultation->setDisplayFormat("dd/MM/yyyy");','    m_dateConsultation->setDisplayFormat("dd/MM/yyyy");\n    m_dateConsultation->setMinimumWidth(150);')
amend('medicalrecorddialog.cpp','    medicalCardLayout->addLayout(medicalGrid);','    medicalCardLayout->addLayout(medicalGrid);\n    medicalGrid->setColumnStretch(1,1);\n    medicalGrid->setColumnStretch(3,1);')
amend('recordcard.cpp','    setMinimumHeight(186);','    setMinimumHeight(170);')

unchanged=[];changed=[]
for p in sorted(stage.iterdir()):
    target=root/'src/medical'/p.name
    (unchanged if target.read_text(encoding='utf8')==p.read_text(encoding='utf8') else changed).append(p.name)
    shutil.copy2(p,target)

def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
manifest={'source':str(source),'date':'2026-10-13','source_sha256':{},'reimported_files':sorted(p.name for p in stage.iterdir()),'behaviorally_unchanged_files':unchanged,'changed_files':changed,
    'integration':['Shared MedicalWidget and common design/hover effects','Existing SQLite records and account passwords preserved','Initial account medical / medical123, opens page 5','PDF-only export; demo examples only in demo mode; real consultation history']}
for p in sorted(source.iterdir()):
    if p.is_file() and p.suffix in ['.cpp','.h','.ui','.pro','.txt','.qrc']:manifest['source_sha256'][p.name]=digest(p)
(root/'src/medical/source-manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
print(json.dumps({'unchanged_after_adaptation':unchanged,'changed_after_adaptation':changed},ensure_ascii=False))
