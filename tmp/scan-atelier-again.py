from pathlib import Path
from zipfile import ZipFile
import xml.etree.ElementTree as E
import re
ns={'a':'http://schemas.openxmlformats.org/drawingml/2006/main'}
with ZipFile(r'C:/Users/malek/Downloads/Atelier_Crud-architectureQT.pptx') as z:
    for n in sorted([n for n in z.namelist() if re.fullmatch(r'ppt/notesSlides/notesSlide\d+.xml',n)],key=lambda s:int(re.search(r'(\d+)\.xml',s).group(1))):
        lines=[''.join(x.itertext()) for x in E.fromstring(z.read(n)).findall('.//a:t',ns)]
        meaningful=[l for l in lines if l.strip() and not l.strip().isdigit()]
        if meaningful: print(n,':',' '.join(meaningful))
pri=Path('sources.pri').read_text(encoding='utf-8-sig');files=sorted(set(re.findall(r'\$\$PWD/(src/[\w/]+\.(?:cpp|h))',pri)))
print('COMPILED FILES',len(files))
for f in files:
    s=Path(f).read_text(encoding='utf-8-sig')
    if f.endswith('.cpp') and 'QSqlQuery' in s and 'QSqlQueryModel' not in s.replace('QSqlQueryModel',''):
        if re.search(r'\bQSqlQuery\b',s):print('SQL',f)
    for i,l in enumerate(s.splitlines(),1):
        if 'addDatabase(' in l:print('CONNECTION',f,i,l.strip())
