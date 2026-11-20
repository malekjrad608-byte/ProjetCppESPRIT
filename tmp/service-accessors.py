from pathlib import Path
import re,sys
log=Path(sys.argv[1]).read_text(encoding='utf-8-sig',errors='replace')
errors={}
for m in re.finditer(r"([^\r\n]+?):(\d+):(\d+): error: '[^']*Service::(\w+)' is private",log):
    path,line,col,name=m.groups(); path=Path(path.replace('\\','/').removeprefix('../'))
    if path.exists():errors.setdefault(path,set()).add((int(line),int(col),name))
def rhs_end(s,pos):
    stack=[];quote=None;escape=False
    for i in range(pos,len(s)):
        c=s[i]
        if quote:
            if escape:escape=False
            elif c=='\\':escape=True
            elif c==quote:quote=None
        elif c in '\"\'':quote=c
        elif c in '([{':stack.append(c)
        elif c in ')]}':
            if not stack:return i
            stack.pop()
        elif c in ';,' and not stack:return i
    raise ValueError('no rhs end')
for p,entries in errors.items():
    s=p.read_text(encoding='utf-8-sig');lines=s.splitlines(keepends=True);offsets=[0]
    for l in lines:offsets.append(offsets[-1]+len(l))
    edits={}
    for line,col,name in entries:
        l=lines[line-1];matches=list(re.finditer(r'(?:(?<=\.)|(?<=->))'+name+r'\b',l))
        if not matches:print('SKIP',p,line,name);continue
        m=min(matches,key=lambda m:abs(m.start()+1-col));a=offsets[line-1]+m.start();b=a+len(name);cap=name[0].upper()+name[1:]
        assign=re.match(r'\s*=(?!=)',s[b:])
        if assign:
            end=rhs_end(s,b+assign.end());edits[(a,b+assign.end())]='set'+cap+'(';edits[(end,end)]=')'
        else:edits[(a,b)]='get'+cap+'()'
    for (a,b),text in sorted(edits.items(),reverse=True):s=s[:a]+text+s[b:]
    p.write_text(s,encoding='utf-8')
    print(p,len(edits))
