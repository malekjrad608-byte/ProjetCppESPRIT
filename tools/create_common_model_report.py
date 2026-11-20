"""Render the common S5 design from the same catalogue as the Oracle DDL."""
from pathlib import Path
import json, math
from xml.sax.saxutils import escape
from xml.etree.ElementTree import Element, SubElement, ElementTree
from reportlab.pdfgen import canvas
from reportlab.lib.colors import HexColor, white
from reportlab.lib.styles import ParagraphStyle
from reportlab.platypus import Paragraph, Table, TableStyle
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

ROOT=Path(__file__).resolve().parents[1]
model=json.loads((ROOT/'database/conception/modele_commun.json').read_text(encoding='utf-8'))
tables=model['tables']; rels=model['relations']; by={t['name']:t for t in tables}
OUT=ROOT/'output/pdf/ANIWORLD_Modeles_communs_S5.pdf'
pdfmetrics.registerFont(TTFont('UI','C:/Windows/Fonts/segoeui.ttf'))
pdfmetrics.registerFont(TTFont('UIB','C:/Windows/Fonts/segoeuib.ttf'))
pdfmetrics.registerFontFamily('UI',normal='UI',bold='UIB',italic='UI',boldItalic='UIB')
ink=HexColor('#203E4B'); teal=HexColor('#267080'); pale=HexColor('#EDF5F7'); orange=HexColor('#D78824')
c=canvas.Canvas(str(OUT),pagesize=(842,595))
c.setTitle('ANIWORLD - Modeles communs S5 : MCD, MLD et MPD Oracle')
c.setAuthor('ANIWORLD / DevCore')
page=0; y=0
def para(text,x,y,w=754,size=10,color=ink,bold=False):
    p=Paragraph(text,ParagraphStyle('p',fontName='UIB' if bold else 'UI',fontSize=size,leading=size*1.4,textColor=color))
    _,h=p.wrap(w,1000);p.drawOn(c,x,y-h);return y-h
def start(section,title,subtitle=''):
    global page,y
    if page:c.showPage()
    page+=1
    c.setFillColor(teal);c.rect(0,580,842,15,fill=1,stroke=0)
    para('ANIWORLD / DEVCORE',44,559,450,9,teal,True)
    para(section.upper(),570,559,228,9,teal,True)
    para(title,44,530,754,23,ink,True)
    y=488
    if subtitle:y=para(subtitle,44,y,754,9,HexColor('#607985'))-15
    c.setStrokeColor(HexColor('#D5E5E9'));c.line(44,40,798,40)
    para('Conception commune S5 - 14 octobre 2026 - Modèle cible, migration Qt distincte',44,30,690,8,HexColor('#607985'))
    para(str(page),770,30,28,8,teal,True)
def text(s,size=10,bold=False):
    global y
    y=para(s,44,y,754,size,ink,bold)-10
def grid(headers,rows,widths,size=9):
    global y
    styles=[ParagraphStyle('h',fontName='UIB',fontSize=size,leading=size*1.35,textColor=white),ParagraphStyle('v',fontName='UI',fontSize=size,leading=size*1.35,textColor=ink)]
    data=[[Paragraph(escape(str(v)),styles[0 if ri==0 else 1]) for v in row] for ri,row in enumerate([headers]+rows)]
    t=Table(data,colWidths=widths,hAlign='LEFT')
    t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),teal),('ROWBACKGROUNDS',(0,1),(-1,-1),[pale,white]),('VALIGN',(0,0),(-1,-1),'TOP'),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),('TOPPADDING',(0,0),(-1,-1),5),('BOTTOMPADDING',(0,0),(-1,-1),5)]))
    _,h=t.wrap(754,1000)
    if y-h<48: raise ValueError(f'Page {page}: table exceeds page ({y-h})')
    t.drawOn(c,44,y-h);y-=h+12

start('01 / Décisions','Une base commune pour les six modules','Services, employés, stock, locaux, dossiers médicaux et adoptions. Authentification et facturation partagées.')
text('<b>Livrables :</b> modèle conceptuel avec cardinalités, modèle relationnel complet, dictionnaire physique Oracle et script DDL exécutable. Le catalogue unique contient <b>23 tables, 33 clés étrangères et 2 vues</b>.')
grid(['Correction du schéma initial','Décision retenue'],[
('RH distinct des employés','Retrait de RH. Conservation d’EMPLOYE, compétences et créneaux pour les affectations.'),
('Stock / article / péremption','ARTICLE décrit un produit ; LOT_STOCK porte la quantité, l’échéance et l’emplacement STOCK.'),
('Animal / dossier / consultation','Un animal possède au plus un dossier longitudinal ; un dossier contient plusieurs consultations.'),
('Adoption / adoptant','Plusieurs demandes sont possibles ; une seule adoption validée non archivée par animal.'),
('Facture manquante','FACTURE et LIGNE_FACTURE gardent le client facturé, les prestations et les prix historiques. Export PDF.'),
('Locaux et personnes partagées','CLIENT, ANIMAL et LOCAL sont référencés par identifiant ; aucune copie de leurs fiches dans chaque module.')
],[220,534],9)
text('<b>Hypothèse à valider avec la professeure :</b> SERVICE représente une prestation planifiée (date, durée, animal, intervenant, local), comme dans le cahier. Un catalogue de prestations réutilisables serait une entité supplémentaire si demandé.',9)

# Conceptual views: collapse the two pure junctions to genuine N:M associations.
verbs={('CLIENT','ANIMAL'):'possède',('ANIMAL','DOSSIER_MEDICAL'):'possède un dossier',('DOSSIER_MEDICAL','CONSULTATION'):'regroupe',('SERVICE','CONSULTATION'):'donne lieu à',('CONSULTATION','FACTURE'):'est facturée par',('CLIENT','FACTURE'):'est facturé',('FACTURE','LIGNE_FACTURE'):'contient',('ARTICLE','LOT_STOCK'):'se décline en',('STOCK','LOT_STOCK'):'héberge',('LOT_STOCK','MOUVEMENT_STOCK'):'est mouvementé',('EMPLOYE','COMPTE'):'dispose de',('CLIENT','ADOPTANT'):'a un profil',('ANIMAL','ADOPTION'):'fait l’objet de',('ADOPTANT','ADOPTION'):'dépose',('ADOPTION','SUIVI_ADOPTION'):'est suivie par'}
assocs=[]
for r in rels:
    if r['child'] in ('EMPLOYE_COMPETENCE','SERVICE_COMPETENCE'):continue
    assocs.append((r['parent'],r['child'],'0,1' if r['unique'] else '0,N','0,1' if r['optional'] else '1,1',verbs.get((r['parent'],r['child']),'est lié à')))
assocs += [('EMPLOYE','COMPETENCE','0,N','0,N','maîtrise'),('SERVICE','COMPETENCE','0,N','0,N','requiert')]
mx=Element('mxfile',host='app.diagrams.net')
for off in range(0,len(assocs),8):
    start('02 / Modèle conceptuel',f'Entités et associations - vue {off//8+1}/4','Notation Merise : la cardinalité près d’une entité compte ses participations à l’association. Une entité répétée désigne toujours la même entité commune.')
    dia=SubElement(mx,'diagram',id=f'mcd-{off}',name=f'MCD - associations {off+1}-{min(off+8,len(assocs))}')
    gm=SubElement(dia,'mxGraphModel',page='1',pageWidth='1000',pageHeight='750');root=SubElement(gm,'root');SubElement(root,'mxCell',id='0');SubElement(root,'mxCell',id='1',parent='0')
    for k,(parent,child,pc,cc,verb) in enumerate(assocs[off:off+8]):
        cy=y-17-k*43
        for x,name in [(44,parent),(594,child)]:
            c.setFillColor(pale);c.setStrokeColor(teal);c.roundRect(x,cy-13,204,28,5,fill=1,stroke=1);para(name,x+8,cy+8,188,9,ink,True)
        c.setStrokeColor(teal);c.line(248,cy,347,cy);c.line(495,cy,594,cy)
        c.setFillColor(white);c.ellipse(347,cy-14,495,cy+14,fill=1,stroke=1)
        para(verb,357,cy+7,130,8,teal)
        para('('+pc+')',275,cy+15,60,9,ink,True);para('('+cc+')',520,cy+15,60,9,ink,True)
        for suffix,name,x,width,style in [('p',parent,40,250,'rounded=1;'),('a',verb,410,180,'ellipse;'),('c',child,720,250,'rounded=1;')]:
            cell=SubElement(root,'mxCell',id=f'{k}{suffix}',value=name,style=style+'html=1;fillColor=#EDF5F7;strokeColor=#267080;fontSize=15;',vertex='1',parent='1');SubElement(cell,'mxGeometry',x=str(x),y=str(50+k*80),width=str(width),height='45',**{'as':'geometry'})
        for suffix,src,dst,card in [('l',f'{k}p',f'{k}a',pc),('r',f'{k}a',f'{k}c',cc)]:
            cell=SubElement(root,'mxCell',id=f'{k}{suffix}',value='('+card+')',source=src,target=dst,edge='1',parent='1',style='endArrow=none;startArrow=none;strokeColor=#267080;fontSize=14;labelBackgroundColor=#FFFFFF;');SubElement(cell,'mxGeometry',relative='1',**{'as':'geometry'})
    para('Les minimums 0 autorisent la préparation progressive des fiches. À l’émission, une facture doit contenir au moins une ligne ; une consultation validée doit avoir sa facture. Ces règles de cycle de vie sont précisées en fin de dossier.',44,91,754,9)
ElementTree(mx).write(ROOT/'database/conception/ANIWORLD_MCD_commun.drawio',encoding='utf-8',xml_declaration=True)

for off in range(0,len(tables),4):
    start('03 / Modèle relationnel',f'Relations communes - {off//4+1}/6','PK : clé primaire ; FK : clé étrangère ; UQ : unique ; ? : facultatif. Les noms de colonnes sont identiques au script Oracle.')
    for t in tables[off:off+4]:
        text(t['name'],11,True)
        fields=[]
        for a in t['columns']:
            if a['name'] in ('cree_le','modifie_le','archive'):continue
            tag=(' PK' if a['name'] in t['pk'] else '')+(' FK' if a.get('ref') else '')+(' UQ' if a.get('unique') else '')+(' ?' if not a['required'] else '')
            fields.append(escape(a['name'])+('<b>'+tag+'</b>' if tag else ''))
        text(' ; '.join(fields),9)
        links=[r['field']+' → '+r['parent'] for r in rels if r['child']==t['name']]
        if links:text('Références : '+escape(' ; '.join(links)),8)
        uq=t['uniques']
        if uq:text('Unicité : '+escape(' ; '.join(' + '.join(u) for u in uq)),8)
    para('Audit commun : cree_le, modifie_le, archive sur les entités, sauf EMPLOYE_COMPETENCE, SERVICE_COMPETENCE, LIGNE_FACTURE et MOUVEMENT_STOCK. Les index fonctionnels complémentaires sont détaillés dans le MPD.',44,75,754,8)
    if y<92:raise ValueError(f'MLD overflow page {page} y={y}')

start('04 / Modèle physique','Choix Oracle et règles structurantes','Oracle Database XE 21c / PDB XEPDB1 / schéma de validation ANIWORLD_S5_MODEL')
grid(['Objet','Implémentation'],[
('Identifiants','NUMBER(10), IDENTITY pour les identifiants simples. PK composée pour les deux associations et les lignes de facture.'),
('Textes / dates','VARCHAR2(... CHAR) pour les libellés ; CLOB pour les notes ; DATE pour les jours ; TIMESTAMP pour les rendez-vous et événements.'),
('Montants','NUMBER(12,3) en dinars tunisiens. Total = somme des lignes arrondies à trois décimales. Aucun calcul fiscal n’est présumé.'),
('Authentification','Login unique sans distinction de casse ; sel RAW(24), empreinte RAW(32), nombre d’itérations. Aucun mot de passe en clair dans les tables métier.'),
('Intégrité','33 FK, contraintes CHECK et UNIQUE ; suppression d’un parent référencé refusée. Index de soutien aux FK et aux recherches par date.'),
('Historique','Date de modification par trigger ; archive logique. MOUVEMENT_STOCK refuse UPDATE et DELETE ; correction par mouvement inverse.'),
('Vues','V_TOTAL_FACTURE calcule le total ; V_STOCK_ARTICLE exclut lots archivés, bloqués et périmés. Aucun accès au schéma Qt courant.'),
('Unicités fonctionnelles','LOWER(login), LOWER(email employé), LOWER(nom local). Une adoption VALIDEE et archive=0 par animal via index CASE.')
],[168,586],9)

# Complete physical dictionary, including defaults and nullability; paginate by rows.
for off in range(0,len(tables),2):
    start('04 / Dictionnaire physique',f'Types et colonnes - {off//2+1}/12','O = obligatoire ; N = NULL accepté. IDENTITY = génération automatique. PK et FK décrites dans le modèle relationnel.')
    top=y
    for side,t in enumerate(tables[off:off+2]):
        x=44+side*387
        py=para(t['name']+' / '+t['group'],x,top,367,10,ink,True)-12
        sty=ParagraphStyle('dic',fontName='UI',fontSize=7.5,leading=10,textColor=ink)
        hs=ParagraphStyle('dich',fontName='UIB',fontSize=7.5,leading=10,textColor=white)
        rows=[['Colonne','Type / requis / défaut']]
        for a in t['columns']:
            if a['name'] in ('cree_le','modifie_le','archive'):continue
            default='IDENTITY' if a.get('identity') else (a.get('default') or '-')
            rows.append([a['name'],a['type']+' / '+('O' if a['required'] else 'N')+' / '+default])
        data=[[Paragraph(escape(str(v)),hs if ri==0 else sty) for v in row] for ri,row in enumerate(rows)]
        tb=Table(data,colWidths=[167,200]);tb.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),teal),('ROWBACKGROUNDS',(0,1),(-1,-1),[pale,white]),('VALIGN',(0,0),(-1,-1),'TOP'),('TOPPADDING',(0,0),(-1,-1),5),('BOTTOMPADDING',(0,0),(-1,-1),5)]))
        _,h=tb.wrap(367,1000);tb.drawOn(c,x,py-h)
        if py-h<95:raise ValueError(f'Physical dictionary overflow page {page}: {py-h}')
    para('Audit (si présent) : cree_le TIMESTAMP NOT NULL DEFAULT SYSTIMESTAMP ; modifie_le identique, actualisé par trigger ; archive NUMBER(1) NOT NULL DEFAULT 0 CHECK (0,1). Liste complète et contraintes exactes : modele_physique_oracle.sql.',44,73,754,8)


start('05 / Règles transactionnelles','Ce que les clés seules ne garantissent pas','À réaliser lors de la migration des services Qt vers ce modèle ; ces workflows ne sont pas exécutés par le DDL livré.')
grid(['Workflow','Contrôle attendu dans une transaction'],[
('Affecter une prestation','Vérifier absence, compétences valides, disponibilité du local et absence de chevauchement. Verrouiller les ressources pour empêcher deux affectations simultanées.'),
('Valider une consultation','Vérifier que SERVICE et DOSSIER_MEDICAL désignent le même animal et que l’intervenant est habilité. Créer automatiquement FACTURE et ses lignes, dans la même transaction.'),
('Émettre une facture','Exiger au moins une ligne, figer l’identité du client et les prix, puis empêcher leur modification. Exporter les valeurs figées en PDF. Ne pas recalculer depuis un tarif ultérieurement modifié.'),
('Mouvement de stock','Verrouiller le lot, refuser une sortie supérieure au disponible, modifier sa quantité et insérer le mouvement ensemble. Le trigger protège l’historique ; il ne met pas à jour la quantité.'),
('Valider / placer un animal','Contrôler les trois validations, actualiser le statut de l’animal et créer les suivis à J+7 / J+30. Verrouiller la fiche animal ; l’index bloque deux validations actives.'),
('Archivage','Exclure les archives des listes actives, conserver les références historiques. Définir la politique de retour d’adoption avant d’archiver une adoption validée.')
],[170,584],9)
text('Les champs equipements, besoins_foyer et autres_animaux restent descriptifs. S’ils doivent être filtrés ou comparés automatiquement, les remplacer par des référentiels et des associations dédiées.',9)

start('06 / Validation et transition','Conception validée ; migration applicative distincte','Résultat réel du 14 octobre 2026 : 20 contrôles Oracle réussis, aucun objet invalide, aucune donnée de test conservée.')
text('Le script a été exécuté dans <b>ANIWORLD_S5_MODEL</b>, schéma sans authentification. Le schéma courant de Qt, <b>ANIWORLD_APP</b>, conserve ses 11 tables de compatibilité. Les 23 nouvelles tables sont un modèle cible : elles ne reçoivent pas encore les ajouts de l’application.')
grid(['Actuel','Cible et reprise nécessaire'],[
('accounts / employes','COMPTE / EMPLOYE / COMPETENCE : conserver les identités et empreintes, reconstruire les liens et compétences.'),
('services / planning_affectations','SERVICE / SERVICE_COMPETENCE / CRENEAU_EMPLOYE : convertir dates, relier animal et local, vérifier les affectations.'),
('medical_records','CLIENT / ANIMAL / DOSSIER_MEDICAL / CONSULTATION / VACCINATION : dédupliquer les personnes et animaux avec validation humaine.'),
('locaux / stock_lots / stock_movements','LOCAL / INTERVENTION_LOCAL / ARTICLE / STOCK / LOT_STOCK / MOUVEMENT_STOCK : conserver les quantités, lots et mouvements.'),
('Factures intégrées aux services','FACTURE / LIGNE_FACTURE : conserver le numéro, les montants et le client historiques. Créer un brouillon à la validation des nouvelles consultations.'),
('Adoptions prévues au cahier','ADOPTANT / ADOPTION / SUIVI_ADOPTION : modèle prévu ; interface et traitements Qt non intégrés dans cette livraison.')
],[215,539],9)
text('Preuve : artifacts/conception-s5/oracle-validation.txt. Rejouer valider_modele.sql uniquement dans le schéma de validation. Les instructions DDL Oracle valident implicitement les changements de structure : le rollback final annule les données de test, pas les tables créées.',9)

start('07 / Références et livraison','Fichiers à présenter à la séance','Dossier commun reproductible : une source de définition, des diagrammes éditables et le script physique.')
grid(['Fichier','Utilité'],[
('ANIWORLD_MCD_commun.drawio','Quatre vues conceptuelles avec associations et cardinalités Merise.'),
('ANIWORLD_MLD_commun.drawio','Vue globale éditable des 23 relations et des 33 références.'),
('ANIWORLD_MPD_commun.drawio','Vue globale éditable avec les types Oracle.'),
('modele_relationnel.txt / modele_commun.json','Relations textuelles et catalogue structuré des colonnes, clés et contraintes.'),
('modele_physique_oracle.sql','Création des tables, contraintes, index, vues et triggers dans un schéma vide dédié.'),
('valider_modele.sql','Tests positifs et négatifs ; données fictives annulées par rollback.')
],[290,464],9)
text('<b>Sources de conception :</b> cahier ANIWorld DevCore v4 ; export du diagramme initial conservé sous artifacts/oracle-integration/schema-lucid-original.pdf ; demandes de retrait RH, facture par consultation et export PDF uniquement.',9)
text('<b>Références techniques :</b><br/>Oracle 21c CREATE TABLE : https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/CREATE-TABLE.html<br/>Oracle 21c contraintes : https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/constraint.html<br/>Lucidchart ERD : https://help.lucid.co/hc/en-us/articles/16471565238292-Create-an-Entity-Relationship-Diagram-in-Lucidchart',8)
text('<b>Questions à préparer :</b> pourquoi séparer article et lot ? Pourquoi un dossier unique et plusieurs consultations ? Comment empêcher deux adoptions actives ? Quelle différence entre connexion Oracle et migration vers le modèle normalisé ? Pourquoi figer les prix de facture ?',10)
c.save()
print(f'PDF generated: {page} pages / {OUT}')
