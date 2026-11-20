"""Plain native shapes: avoid HTML overflow in Lucid's draw.io importer."""
from pathlib import Path
from xml.etree.ElementTree import Element,SubElement,ElementTree,parse,tostring
from urllib.parse import quote
import zlib,base64
import json
ROOT=Path(__file__).resolve().parents[1]
D=ROOT/'database/conception'
data=json.loads((D/'modele_commun.json').read_text(encoding='utf-8'))
tables=data['tables'];rels=data['relations'];by={t['name']:t for t in tables}
out=Element('mxfile',host='app.diagrams.net')
def page(name,w=1440,h=1220):
    d=SubElement(out,'diagram',name=name,id='p'+str(len(out)))
    g=SubElement(d,'mxGraphModel',page='1',pageScale='1',pageWidth=str(w),pageHeight=str(h),grid='1')
    r=SubElement(g,'root');SubElement(r,'mxCell',id='0');SubElement(r,'mxCell',id='1',parent='0');return r
def box(r,id,text,x,y,w,h,color='#EDF5F7',size=14):
    e=SubElement(r,'mxCell',id=id,value=text,style=f'rounded=0;whiteSpace=wrap;html=0;align=left;verticalAlign=top;spacing=14;fillColor={color};strokeColor=#267080;fontColor=#203E4B;fontFamily=Arial;fontSize={size};',vertex='1',parent='1')
    SubElement(e,'mxGeometry',x=str(x),y=str(y),width=str(w),height=str(h),**{'as':'geometry'})
def title(r,text,sub):
    box(r,'title',text,30,20,1380,65,'#CEE4E9',24)
    box(r,'legend',sub,30,95,1380,75,'#FFFFFF',14)
for d in parse(D/'ANIWORLD_MCD_commun.drawio').getroot():out.append(d)
groups=[['CLIENT','ANIMAL','DOSSIER_MEDICAL','CONSULTATION','VACCINATION','SERVICE'],['EMPLOYE','COMPTE','COMPETENCE','EMPLOYE_COMPETENCE','CRENEAU_EMPLOYE','SERVICE_COMPETENCE'],['LOCAL','INTERVENTION_LOCAL','STOCK','ARTICLE','LOT_STOCK','MOUVEMENT_STOCK'],['FACTURE','LIGNE_FACTURE','ADOPTANT','ADOPTION','SUIVI_ADOPTION']]
for mode in ['MLD','MPD']:
    for i,names in enumerate(groups):
        r=page(f'{mode} {i+1} - '+['Soins et services','Employes et competences','Locaux et stock','Factures et adoptions'][i])
        title(r,f'ANIWORLD / {mode} COMMUN / {i+1} sur 4','PK = cle primaire | FK = cle etrangere | UQ = unique | ? = facultatif\nLes references FK relient aussi les tables des autres pages. Modele cible S5 : migration Qt distincte.')
        for j,name in enumerate(names):
            t=by[name];lines=[name,'']
            for a in t['columns']:
                if mode=='MLD' and a['name'] in ('cree_le','modifie_le','archive'):continue
                flags=('PK ' if a['name'] in t['pk'] else '')+('FK ' if a.get('ref') else '')+('UQ ' if a.get('unique') else '')
                s=flags+a['name']+(' ?' if not a['required'] else '')
                if mode=='MPD':s+=' : '+a['type'].replace(' CHAR','')
                if a.get('ref'):s+=' -> '+a['ref']
                lines.append(s)
            if t['uniques']:lines+=['','UQ: '+' ; '.join(' + '.join(u) for u in t['uniques'])]
            if mode=='MLD' and t['audit']:lines+=['','Audit : cree_le, modifie_le, archive']
            box(r,name,'\n'.join(lines),30+(j%3)*470,190+(j//3)*500,440,480,'#F1F7F8',12 if mode=='MPD' else 14)
r=page('Reunion - questions et ameliorations')
title(r,'ANIWORLD / PREPARER LA REUNION','Questions probables et ameliorations proposees a partir du schema initial. Reponses a adapter aux choix valides avec la professeure.')
qa=[('1. Pourquoi retirer RH ?','Le besoin porte sur la gestion des employes et leurs affectations. Conserver EMPLOYE, COMPETENCE et CRENEAU_EMPLOYE ; eviter un module RH redondant.'),('2. Article et stock : quelles cardinalites ?','Un ARTICLE peut avoir plusieurs LOT_STOCK ; chaque lot concerne un seul article et un seul STOCK. Quantite et peremption appartiennent au lot.'),('3. Un animal peut-il avoir plusieurs dossiers ?','Un seul dossier longitudinal par animal, plusieurs consultations dans ce dossier. La FK id_animal est unique dans DOSSIER_MEDICAL.'),('4. Service et consultation : quelle difference ?','SERVICE est la prestation planifiee. CONSULTATION est son acte clinique. Le service et le dossier doivent designer le meme animal.'),('5. Comment generer une facture fiable ?','Creer FACTURE et LIGNE_FACTURE dans la transaction de validation. Figer le client et les prix a l emission ; exporter en PDF.'),('6. Comment empecher deux adoptions actives ?','Plusieurs candidatures possibles, une seule adoption VALIDEE non archivee par animal. Index conditionnel Oracle + transaction de validation.'),('7. Quels controles sont dans Oracle ?','PK, FK, UNIQUE, CHECK, precision monetaire et historique des mouvements. Disponibilites, habilitations et workflows necessitent encore le code transactionnel.'),('8. Est-ce deja le schema utilise par Qt ?','Non. Qt utilise ANIWORLD_APP (11 tables). Le modele cible (23 tables) est teste dans ANIWORLD_S5_MODEL. La migration applicative reste une etape distincte.')]
for j,(q,a) in enumerate(qa):box(r,'q'+str(j),q+'\n\n'+a,30+(j%2)*705,190+(j//2)*245,675,225,'#F1F7F8',17)
ElementTree(out).write(D/'ANIWORLD_Conception_complete_Lucidchart.drawio',encoding='utf-8',xml_declaration=True)
for d in out:
    payload=quote(tostring(d[0],encoding='unicode'),safe="~()*!.'-").encode()
    compressor=zlib.compressobj(wbits=-15)
    d.remove(d[0])
    d.text=base64.b64encode(compressor.compress(payload)+compressor.flush()).decode()
out.set('compressed','true')
ElementTree(out).write(D/'ANIWORLD_Modeles_S5.drawio',encoding='utf-8',xml_declaration=True)
print(f'{len(out)} pages generees')
