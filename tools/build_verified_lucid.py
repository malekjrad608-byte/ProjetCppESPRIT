"""Generate native Lucid-importable diagrams from the verified Oracle dictionary.
The operational schema and the separately tested normalized target are labelled explicitly.
No business rows, passwords or personal data are included.
"""
from pathlib import Path
from xml.etree.ElementTree import Element,SubElement,ElementTree,parse,tostring
from urllib.parse import quote
import json,zlib,base64
ROOT=Path(__file__).resolve().parents[1]
D=ROOT/'database/conception'
m=json.loads((ROOT/'artifacts/local-links-20261015/after/metadata.json').read_text(encoding='utf-8'))
out=Element('mxfile',host='app.diagrams.net')
def page(name,w=1600,h=1250):
    d=SubElement(out,'diagram',name=name,id='actual'+str(len(out)))
    g=SubElement(d,'mxGraphModel',page='1',pageScale='1',pageWidth=str(w),pageHeight=str(h))
    r=SubElement(g,'root');SubElement(r,'mxCell',id='0');SubElement(r,'mxCell',id='1',parent='0');return r
def box(r,id,text,x,y,w,h,color='#F4F9FA',size=16):
    e=SubElement(r,'mxCell',id=id,value=text,style=f'rounded=1;whiteSpace=wrap;html=0;align=left;verticalAlign=top;spacing=18;fillColor={color};strokeColor=#267080;fontColor=#203E4B;fontFamily=Arial;fontSize={size};',vertex='1',parent='1')
    SubElement(e,'mxGeometry',x=str(x),y=str(y),width=str(w),height=str(h),**{'as':'geometry'})
def title(r,text,sub):
    box(r,'title',text,30,20,1540,65,'#CEE4E9',25)
    box(r,'legend',sub,30,100,1540,95,'#FFFFFF',16)
def edge(r,id,source,target,label):
    y={'SERVICES':0.15,'MEDICAL_RECORDS':0.5,'STOCK_LOTS':0.85}[target]
    e=SubElement(r,'mxCell',id=id,value=label,style=f'edgeStyle=none;rounded=0;html=0;exitX=1;exitY={y};entryX=0;entryY=0.5;startArrow=ERzeroToOne;endArrow=ERzeroToMany;strokeColor=#267080;fontSize=17;labelBackgroundColor=#FFFFFF;',edge='1',parent='1',source=source,target=target)
    SubElement(e,'mxGeometry',relative='1',**{'as':'geometry'})
def columns(t):return [a for a in m['columns'] if a['TABLE_NAME']==t]
def body(t):
    cs={c['CONSTRAINT_NAME']:c for c in m['constraints'] if c['TABLE_NAME']==t}
    pk=set();fk=set();uq=set()
    for c in m['constraint_columns']:
        if c['CONSTRAINT_NAME'] in cs:
            k=cs[c['CONSTRAINT_NAME']]['CONSTRAINT_TYPE']
            if k=='P':pk.add(c['COLUMN_NAME'])
            if k=='R':fk.add(c['COLUMN_NAME'])
            if k=='U':uq.add(c['COLUMN_NAME'])
    lines=[t,'']
    for a in columns(t):
        name=a['COLUMN_NAME'];typ=a['DATA_TYPE']
        if typ in ('VARCHAR2','CHAR','RAW'):typ+=f"({a['CHAR_LENGTH'] or (24 if name=='SALT' else 32)}"+(' CHAR)' if a['CHAR_USED']=='C' else ')')
        if typ=='NUMBER' and a['DATA_PRECISION'] is not None:typ+=f"({a['DATA_PRECISION']},{a['DATA_SCALE'] or 0})"
        flags=('PK ' if name in pk else '')+('FK ' if name in fk else '')+('UQ ' if name in uq else '')
        lines.append(flags+name+' : '+typ+(' ?' if a['NULLABLE']=='Y' else ''))
    return '\n'.join(lines)
r=page('01 - Relations Oracle utilisees par Qt')
title(r,'ANIWORLD / ORACLE ET QT / RELATIONS EFFECTIVES','ANIWORLD_APP dans XEPDB1 - verifie le 15/10/2026. Une FK facultative permet une ligne non affectee.\nUn local peut accueillir plusieurs lignes au fil du temps. Ce diagramme ne constitue pas un controle de conflits horaires.')
box(r,'LOCAUX','LOCAUX\n\nPK ID\nNAME, TYPE, FLOOR\nCAPACITY, OCCUPANCY\nSPECIALTY, OUT_OF_SERVICE\nCLEANING\nLAST_CLEANED, DIRTY_SINCE',40,425,490,350,'#CEE4E9',22)
box(r,'SERVICES','SERVICES\n\nPK ID\nFK ID_LOCAL ? -> LOCAUX.ID\nClient et facture : CLIENTNAME,\nINVOICENUMBER, INVOICEDATE, PRICE',960,230,580,235,size=21)
box(r,'MEDICAL_RECORDS','MEDICAL_RECORDS\n\nPK ID\nFK ID_LOCAL ? -> LOCAUX.ID\nUne ligne = une consultation avec\nles informations de l animal et du dossier.',960,570,580,235,size=21)
box(r,'STOCK_LOTS','STOCK_LOTS\n\nPK LOT\nFK ID_LOCAL ? -> LOCAUX.ID\nQuantite et peremption propres au lot.',960,910,580,220,size=21)
for i,t in enumerate(['SERVICES','MEDICAL_RECORDS','STOCK_LOTS']):edge(r,'e'+str(i),'LOCAUX',t,'0..1 local / 0..N lignes')
box(r,'rules','Suppression du local refusee si une ligne le reference.\nAucune suppression en cascade.\nAnciens enregistrements : ID_LOCAL = NULL tant que le local n est pas choisi dans Qt.',40,920,650,210,'#FFF3DF',18)
groups=[('02 - MPD Locaux et Services',['LOCAUX','SERVICES']),('03 - MPD Medical et Stock',['MEDICAL_RECORDS','STOCK_LOTS','STOCK_MOVEMENTS']),('04 - MPD Employes et Comptes',['EMPLOYES','PLANNING_AFFECTATIONS','ACCOUNTS'])]
for name,tables in groups:
    r=page(name)
    title(r,'ANIWORLD_APP / '+name[9:].upper(),'Structure physique lue dans Oracle. PK = cle primaire ; FK = cle etrangere ; UQ = unique ; ? = NULL autorise.\nLes dates VARCHAR2 sont au format ISO pour compatibilite avec Qt ; leur conversion en TIMESTAMP exige une migration applicative.')
    width=740 if len(tables)==2 else 490
    for i,t in enumerate(tables):box(r,t,body(t),30+i*(width+25),220,width,730,size=20 if len(tables)==2 else 17)
    notes={
        'LOCAUX':'LOCAUX : CAPACITY >= 1 ; 0 <= OCCUPANCY <= CAPACITY ; OUT_OF_SERVICE = 0/1.\nCLEANING : 0 propre, 1 a nettoyer, 2 nettoyage en cours. Nom unique sans distinction de casse.\nSERVICES : ID_LOCAL -> LOCAUX.ID. Numerotation de facture unique generee a l insertion.',
        'MEDICAL_RECORDS':'MEDICAL_RECORDS.ID_LOCAL et STOCK_LOTS.ID_LOCAL referencent LOCAUX.ID.\nSTOCK_MOVEMENTS conserve un instantane d audit (lot, article, quantites avant/apres), meme apres suppression du lot.\nIl n a donc pas de FK de suppression vers STOCK_LOTS. Le dossier longitudinal normalise figure dans les pages CIBLE.',
        'EMPLOYES':'PLANNING_AFFECTATIONS.RESPONSABLE_ID -> EMPLOYES.ID (FK obligatoire ; suppression en cascade existante).\nUn employe peut avoir 0..N affectations ; une affectation a exactement 1 responsable.\nACCOUNTS contient les comptes applicatifs par module ; aucun module RH separe dans cette implementation.'
    }
    box(r,'notes',notes[tables[0]],30,985,1540,210,'#FFF3DF',19)
r=page('05 - Regles et tables techniques')
title(r,'ANIWORLD / REGLES ET PERIMETRE','Les pages 01 a 05 decrivent ANIWORLD_APP utilise par Qt. Les pages CIBLE suivantes decrivent ANIWORLD_S5_MODEL, distinct et non utilise par Qt.')
for i,t in enumerate(['SCHEMA_VERSION','INTEGRATION_MIGRATIONS','COLLECTION_VERSIONS']):box(r,t,body(t),30+i*515,220,490,250,size=19)
box(r,'changes','CORRECTIONS APPLIQUEES\n\n1. LOCAUX represente les 11 champs reels : occupation, specialite, nettoyage et dates inclus.\n2. Trois FK ID_LOCAL existent maintenant dans SERVICES, MEDICAL_RECORDS et STOCK_LOTS.\n3. Qt propose le choix du local dans ces trois formulaires. La valeur vide reste NULL.\n4. La relation vers les consultations est 0..N, et non une seule consultation par local.\n5. La conception commune retire RH, rend le dossier unique par animal, et distingue clients, factures et lots.\n6. Les pages CIBLE ne pretendent pas que cette normalisation complete est deja utilisee dans Qt.',30,510,1540,360,'#CEE4E9',22)
box(r,'limits','QUESTIONS A PREPARER\n\nPourquoi distinguer le schema execute et la conception cible ? Comment une FK protege-t-elle les donnees ?\nPourquoi le local est-il facultatif pour les anciennes lignes ? Comment conserver l historique d un lot supprime ?\nLes conflits horaires et la synchronisation de l occupation ne sont pas garantis par une FK : ils demandent des regles de reservation.',30,905,1540,270,'#FFF3DF',21)
# The common model already has no RH, one dossier per animal, clients, invoices and lots.
# Keep its explicit target status; do not present these tables as the operational Qt schema.
for d in parse(D/'ANIWORLD_Conception_complete_Lucidchart.drawio').getroot():
    if d.attrib['name'].startswith(('MCD','MPD')):
        d.set('name','CIBLE - '+d.attrib['name']);d.set('id','target'+str(len(out)))
        for cell in d.iter('mxCell'):
            if cell.get('id')=='title':cell.set('value','CIBLE / '+cell.get('value',''))
        out.append(d)
for d in out:
    raw=quote(tostring(d[0],encoding='unicode'),safe="~()*!.'-").encode()
    c=zlib.compressobj(wbits=-15);d.remove(d[0]);d.text=base64.b64encode(c.compress(raw)+c.flush()).decode()
out.set('compressed','true')
dest=D/'ANIWORLD_Oracle_Qt_corrige_20261015.drawio'
ElementTree(out).write(dest,encoding='utf-8',xml_declaration=True)
print(len(out),'pages generated:',dest)
