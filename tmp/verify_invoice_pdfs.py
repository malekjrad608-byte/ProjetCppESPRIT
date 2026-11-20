from pathlib import Path
from pypdf import PdfReader
import pypdfium2 as pdfium
folder=Path('artifacts/services-upgrade')
for name in ['facture-exemple','facture-longue','stock-inventaire','stock-filtre']:
 path=folder/(name+'.pdf'); reader=PdfReader(path)
 text='\n'.join(p.extract_text() for p in reader.pages)
 print(name, len(reader.pages), 'pages', len(text), 'characters')
 (folder/(name+'.txt')).write_text(text,encoding='utf-8')
 doc=pdfium.PdfDocument(str(path))
 for i in range(len(doc)):
  doc[i].render(scale=1.3).to_pil().save(folder/(name+f'-page-{i+1}.png'))
 if name=='facture-exemple':
  for value in ['50,50','TOTAL','Inès','FAC-2026-000042','Bilan','prévention']: assert value in text, (value,text)
 if name=='stock-filtre':
  assert 'CAT-V-03' in text and 'DOG-V-03' not in text
