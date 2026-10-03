from pathlib import Path
from zipfile import ZipFile
from hashlib import sha256
from shutil import copy2
import re
from PIL import Image, ImageChops
from docx import Document
from docx.oxml.ns import qn

root=Path(r'C:\Users\malek\Documents\gestion_service')
docx=root/'output/documents/ANIWORLD_devCore_Cahier_de_specification_v3_compact.docx'
pdf=root/'output/pdf/ANIWORLD_devCore_Cahier_de_specification_v3_compact.pdf'
backup=root/'tmp/cahier_specification/before_group_logo_20260928'
backup.mkdir(exist_ok=True)
for path in (docx,pdf):
    assert not (backup/path.name).exists(), 'Backup already exists; stop before overwriting'
    copy2(path,backup/path.name)
logo=Path(r'C:\Users\malek\Downloads\Plan de travail 1.png')
old_logo=Path(r'C:\Users\malek\Downloads\logo-removebg-preview.png').read_bytes()
d=Document(docx)
matches=[]
for shape in d.inline_shapes:
    rid=shape._inline.xpath('.//a:blip')[0].get(qn('r:embed'))
    part=d.part.related_parts[rid]
    if sha256(part.blob).digest()==sha256(old_logo).digest():
        matches.append((rid,str(part.partname).lstrip('/')))
assert len(matches)==1
rid,media=matches[0]

# Fit the supplied square logo in the existing square image box. Only Word's
# whitespace crop is changed; the supplied PNG bytes are embedded unchanged.
im=Image.open(logo).convert('RGB')
w,h=im.size
assert w==h
box=ImageChops.difference(im,Image.new('RGB',im.size,'white')).getbbox()
l,t,r,b=box
side=max(r-l,b-t)
cx=(l+r)/2;cy=(t+b)/2
crop={'l':(cx-side/2)/w,'t':(cy-side/2)/h,'r':(w-cx-side/2)/w,'b':(h-cy-side/2)/h}
new_crop='<a:srcRect '+ ' '.join(f'{key}="{round(value*100000)}"' for key,value in crop.items())+'/>'
with ZipFile(docx) as source:
    xml=source.read('word/document.xml').decode('utf-8')
    shapes=list(re.finditer(r'<wp:inline\b.*?</wp:inline>',xml,re.S))
    target=[m for m in shapes if f'r:embed="{rid}"' in m.group()]
    assert len(target)==1
    shape_xml=target[0].group()
    updated,n=re.subn(r'<a:srcRect\b[^>]*/>',new_crop,shape_xml)
    assert n==1
    updated=updated.replace('Logo du groupe devCore : patte anthracite sur un cercle vert','Logo du groupe devCore')
    xml=xml[:target[0].start()]+updated+xml[target[0].end():]
    temporary=docx.with_suffix('.logo-update.docx')
    with ZipFile(temporary,'w') as output:
        for entry in source.infolist():
            content=logo.read_bytes() if entry.filename==media else (xml.encode('utf-8') if entry.filename=='word/document.xml' else source.read(entry.filename))
            output.writestr(entry,content)
temporary.replace(docx)
with ZipFile(backup/docx.name) as before,ZipFile(docx) as after:
    changed=[n for n in before.namelist() if before.read(n)!=after.read(n)]
    assert set(changed)=={'word/document.xml',media}
    previous_xml=before.read('word/document.xml').decode('utf-8')
    final_xml=after.read('word/document.xml').decode('utf-8')
    assert final_xml.replace(updated,shape_xml,1)==previous_xml
    assert after.read(media)==logo.read_bytes()
print('Only the cover logo, its crop and its alternate description changed. All text and layout preserved.')
print('Backup:',backup)
