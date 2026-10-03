from pathlib import Path
from PIL import Image
from docx import Document
from docx.shared import Cm, Pt, RGBColor
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.enum.text import WD_ALIGN_PARAGRAPH

ROOT=Path(r'C:\Users\malek\Documents\gestion_service')
WORK=ROOT/'tmp/cahier_specification'
SOURCE=ROOT/'output/documents/ANIWORLD_devCore_Cahier_de_specification.docx'
DEST=ROOT/'output/documents/ANIWORLD_devCore_Cahier_de_specification_v2.docx'
DOWNLOADS=Path(r'C:\Users\malek\Downloads')
GROUP=DOWNLOADS/'logo-removebg-preview.png'
SYMBOL=DOWNLOADS/'new logo.png'
WORDMARK=DOWNLOADS/'new name.png'
d=Document(SOURCE)
shapes=list(d.inline_shapes)
assert len(shapes)==8

def setp(p,text):
    if p.runs:
        p.runs[0].text=text
        for r in p.runs[1:]:r.text=''
    else:p.add_run(text)
def find(prefix):
    ps=[p for p in d.paragraphs if p.text.startswith(prefix)]
    assert len(ps)==1,(prefix,len(ps))
    return ps[0]
def replace(prefix,text):setp(find(prefix),text)
def cell(c,t):setp(c.paragraphs[0],t)
def size(shape,width,height):
    shape.width=Cm(width);shape.height=Cm(height)
    ext=shape._inline.xpath('.//a:xfrm/a:ext')[0]
    ext.set('cx',str(shape.width));ext.set('cy',str(shape.height))
def place_asset(shape,path,width,description):
    # Each identity has a distinct relationship, including when the old logos shared a part.
    rid,_=d.part.get_or_add_image(str(path))
    blip=shape._inline.xpath('.//a:blip')[0];blip.set(qn('r:embed'),rid)
    im=Image.open(path);w,h=im.size;l,t,r,b=im.getbbox()
    fill=blip.getparent()
    for old in list(fill.findall(qn('a:srcRect'))):fill.remove(old)
    crop=OxmlElement('a:srcRect')
    for k,v in [('l',l/w),('t',t/h),('r',(w-r)/w),('b',(h-b)/h)]:crop.set(k,str(round(v*100000)))
    fill.insert(1,crop);size(shape,width,width*(b-t)/(r-l))
    shape._inline.docPr.set('descr',description)

for part in d.part.package.parts:
    if hasattr(part,'element'):
        for t in part.element.iter(qn('w:t')):
            if t.text:t.text=t.text.replace('Version 1.1','Version 1.2')
d.core_properties.title='ANIWORLD — Cahier de spécification — Groupe devCore'
d.core_properties.author='devCore'

# Couverture : le symbole du groupe est lié au nom devCore ; l’application possède son logotype séparé.
place_asset(shapes[0],GROUP,1.8,'Logo du groupe devCore : patte anthracite sur un cercle vert')
brand=next(p for p in d.paragraphs if p.text=='ANIWORLD')
assert brand.text=='ANIWORLD'
group=find('Groupe devCore')
brand._p.addprevious(group._p)
group.paragraph_format.space_after=Pt(18)
group.paragraph_format.space_before=Pt(3)
brand.clear()
brand.paragraph_format.space_after=Pt(17)
brand.paragraph_format.space_before=Pt(4)
newbrand=brand.add_run().add_picture(str(WORDMARK))
place_asset(newbrand,WORDMARK,6.4,'Logotype ANIWORLD fourni : nom de l’application')
for p in d.paragraphs:
    if p._p.xpath('.//wp:inline'):
        p.paragraph_format.space_after=Pt(2)
        break

replace('L’identité visuelle d’ANIWORLD associe', 'L’application ANIWORLD utilise le chat orange dans un médaillon bleu et le logotype fourni. Le groupe auteur devCore possède une identité distincte : la patte verte présentée sur la couverture.')
replace('Figure 1  •', 'Figure 1  •  Symbole et logotype de l’application ANIWORLD')
replace('Le nom ANIWORLD évoque', 'Le nom ANIWORLD évoque l’univers des animaux. Le chat orange constitue le symbole de l’application ; le médaillon bleu et les contours sombres le rendent reconnaissable. Le logotype présente le nom ANIWORLD et les signatures du visuel fourni.')
place_asset(shapes[1],SYMBOL,3.15,'Symbole de l’application ANIWORLD : chat orange dans un médaillon bleu')
logo_p=next(p for p in d.paragraphs if any(x is shapes[1]._inline for x in p._p.xpath('.//wp:inline')))
logo_p.alignment=WD_ALIGN_PARAGRAPH.CENTER
logo_p.add_run('     ')
mark=logo_p.add_run().add_picture(str(WORDMARK))
place_asset(mark,WORDMARK,5.8,'Logotype ANIWORLD : nom et signature de l’application')
for row,values in zip(d.tables[13].rows[1:],[
    ['Symbole de l’application','Chat orange dans un médaillon bleu. Utilisé pour les repères de marque et le bandeau du tableau de bord.'],
    ['Logotype ANIWORLD','Nom de l’application avec les éléments graphiques fournis. Affiché dans la navigation commune aux cinq modules.'],
    ['Identité du groupe devCore','Patte anthracite sur fond vert, réservée à la présentation du groupe sur la couverture. Elle ne désigne pas l’application.']]):
    for c,t in zip(row.cells,values):cell(c,t)
replace('Conserver les proportions du logo', 'Conserver les proportions, les couleurs et la transparence des visuels fournis. Prévoir une zone libre autour de chaque logo. Dans la navigation, réserver au moins 160 pixels de largeur au logotype ; pour le symbole seul, prévoir au moins 40 pixels.')
replace('Privilégier un fond blanc', 'Privilégier un fond blanc ou très clair. Utiliser le chat comme symbole compact et le logotype complet lorsque le nom doit être visible. Le logo du groupe reste associé au nom devCore sur la couverture.')
replace('La navigation latérale, les couleurs', 'La palette de l’application reprend le bleu du médaillon et l’orange du chat. Une navigation blanche, des titres contrastés et des espacements réguliers structurent les écrans. Chaque module conserve sa vue métier : planning, annuaire, inventaire, cartes des locaux ou historique clinique.')
palette=[
    ['Bleu pétrole','#397F8D','Repères de marque et accent de navigation.'],
    ['Bleu profond','#225A64','Boutons principaux et titres des zones.'],
    ['Anthracite','#263B42','Textes courants et informations essentielles.'],
    ['Gris très clair','#F4F7F8','Fond général de l’application.'],
    ['Blanc','#FFFFFF','Tableaux, formulaires et surfaces de contenu.'],
    ['Bleu pâle','#E4F0F3','Sélection active et surfaces de synthèse.'],
    ['Orange ambré','#E8A13F','Rappel du chat ; encarts de suivi en teinte claire.'],
    ['Rouge brique','#AE5751','Erreurs et actions nécessitant une attention.']]
for row,values in zip(d.tables[14].rows[1:],palette):
    for c,t in zip(row.cells,values):cell(c,t)
    pr=row.cells[1]._tc.get_or_add_tcPr()
    for e in list(pr.findall(qn('w:shd'))):pr.remove(e)
    sh=OxmlElement('w:shd');sh.set(qn('w:fill'),values[1][1:]);pr.append(sh)
    for r in row.cells[1].paragraphs[0].runs:r.font.color.rgb=RGBColor.from_string('FFFFFF' if values[1] in ['#397F8D','#225A64','#263B42','#AE5751'] else '263B42')
replace('La police de l’interface est Segoe UI.', 'La police de l’interface est Segoe UI. Les titres d’écran utilisent 28 pixels, les titres de zone 15 à 17 pixels et le texte courant 13 à 14 pixels. Les mentions secondaires restent à 11 ou 12 pixels. Les tableaux offrent des lignes aérées ; les statuts combinent un libellé et une couleur.')
replace('Le bouton principal est vert profond', 'Le bouton principal est bleu profond avec un texte blanc. Les actions secondaires utilisent un fond blanc et une bordure légère. Une suppression demande confirmation et précise l’objet concerné. Un état vide explique l’absence de données et propose une action utile.')
for i,shape in enumerate(shapes[2:]):
    path=WORK/'gui_aniworld_v2'/f'gui_{i}.png'
    rid,_=d.part.get_or_add_image(str(path));shape._inline.xpath('.//a:blip')[0].set(qn('r:embed'),rid)
    w,h=Image.open(path).size;size(shape,17,17*h/w)
    shape._inline.docPr.set('descr',f'ANIWORLD — Interface {i+1} — Logotype de l’application et données fictives')
assert len(d.inline_shapes)==10
# Preserve the substantive requirements unchanged.
old=Document(SOURCE)
texts=lambda doc:' '.join(t.text or '' for t in doc.element.iter(qn('w:t')))
a,b=texts(old),texts(d)
start='1 Contexte général';end='3 Conception graphique'
assert a.rfind(start)>=0 and b.rfind(start)>=0
assert a[a.rfind(start):a.rfind(end)]==b[b.rfind(start):b.rfind(end)]
d.save(DEST)
print(DEST)
print('Group logo on cover; application symbol and wordmark independently embedded; all requirements preserved.')
