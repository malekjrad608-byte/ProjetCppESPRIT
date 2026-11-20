from zipfile import ZipFile
from pathlib import Path
import xml.etree.ElementTree as ET
p=Path(r'C:/Users/malek/Downloads/Atelier_Crud-architectureQT.pptx')
ns={'a':'http://schemas.openxmlformats.org/drawingml/2006/main'}
with ZipFile(p) as z:
 names=sorted([n for n in z.namelist() if __import__('re').fullmatch(r'ppt/slides/slide\d+.xml',n)],key=lambda n:int(__import__('re').search(r'(\d+)\.xml',n).group(1)))
 out=[]
 for i,n in enumerate(names,1):
  root=ET.fromstring(z.read(n));texts=[''.join(x.itertext()) for x in root.findall('.//a:t',ns)]
  out.append(f'=== PAGE {i} ===\n'+'\n'.join(texts))
 dest=Path('artifacts/atelier-crud-review');dest.mkdir(parents=True,exist_ok=True)
 (dest/'slides-text.txt').write_text('\n\n'.join(out),encoding='utf-8')
 print('\n\n'.join(out))
