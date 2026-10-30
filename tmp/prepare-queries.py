from pathlib import Path
import re
# Convert the SQL argument of QSqlQuery::exec to an explicit prepare/exec pair.
# Dialog exec() and event loops are not changed.
for path in Path('src').rglob('*.cpp'):
    if 'rh' in path.parts: continue # Separate RH app, outside gestion_service.pro.
    s=path.read_text(encoding='utf-8-sig')
    names=set(re.findall(r'QSqlQuery\s+(\w+)\s*\(',s))
    edits=[]
    for name in names:
        for m in re.finditer(r'\b'+name+r'\.exec\(',s):
            start=m.end();i=start;depth=1;quote=None
            while depth and i<len(s):
                c=s[i]
                if quote:
                    if c=='\\': i+=2;continue
                    if c==quote:quote=None
                elif c in ['"',"'"]:quote=c
                elif c=='(':depth+=1
                elif c==')':depth-=1
                i+=1
            arg=s[start:i-1]
            if arg.strip(): edits.append((m.start(),i,f'({name}.prepare({arg}) && {name}.exec())'))
    for a,b,text in sorted(edits,reverse=True):s=s[:a]+text+s[b:]
    if edits:path.write_text(s,encoding='utf-8')
    if edits:print(path,len(edits))
