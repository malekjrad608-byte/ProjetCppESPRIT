"""Single source for the common conceptual/relational/Oracle S5 design.
This does not migrate or replace the live Qt compatibility schema.
"""
from pathlib import Path
from xml.etree.ElementTree import Element, SubElement, ElementTree
from xml.sax.saxutils import escape
import json

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'database/conception'
OUT.mkdir(parents=True,exist_ok=True)
tables=[]
def col(name,typ='VARCHAR2(120 CHAR)',required=False,default=None,check=None):
    return dict(name=name,type=typ,required=required,default=default,check=check)
def ref(name,parent,optional=False,unique=False):
    c=col(name,'NUMBER(10)',not optional)
    c.update(ref=parent,unique=unique)
    return c
def entity(name,group,desc,fields,pk=None,checks=(),uniques=(),audit=True):
    if pk is None:
        pk=['id_'+name.lower()]
        fields=[dict(name=pk[0],type='NUMBER(10)',required=True,identity=True)]+fields
    if audit:
        fields += [col('cree_le','TIMESTAMP',True,'SYSTIMESTAMP'),col('modifie_le','TIMESTAMP',True,'SYSTIMESTAMP'),col('archive','NUMBER(1)',True,'0','archive IN (0,1)')]
    t=dict(name=name,group=group,description=desc,columns=fields,pk=pk,checks=list(checks),uniques=list(uniques),audit=audit)
    tables.append(t)
    return t

entity('CLIENT','Soins','Personne de référence, propriétaire ou payeur. Une personne peut aussi avoir un profil adoptant.',[
    col('nom',required=True),col('prenom'),col('telephone','VARCHAR2(30 CHAR)'),col('email','VARCHAR2(150 CHAR)'),col('adresse','VARCHAR2(250 CHAR)')])
entity('ANIMAL','Soins','Identité partagée entre Services, Dossiers médicaux et Adoptions. Propriétaire facultatif pour les animaux du centre.',[
    ref('id_client','CLIENT',True),col('nom',required=True),col('espece','VARCHAR2(60 CHAR)',True),col('race'),col('sexe','VARCHAR2(15 CHAR)',True,check="sexe IN ('MALE','FEMELLE','INCONNU')"),col('date_naissance','DATE'),col('identifiant_puce','VARCHAR2(80 CHAR)'),col('besoins_foyer','CLOB'),col('statut_placement','VARCHAR2(20 CHAR)',True,"'AU_CENTRE'","statut_placement IN ('AU_CENTRE','PROPRIETAIRE','ADOPTE')")],uniques=[['identifiant_puce']])
entity('EMPLOYE','Employés','Annuaire et disponibilité hebdomadaire de référence ; la charge est calculée depuis les services planifiés.',[
    col('nom',required=True),col('prenom',required=True),col('email','VARCHAR2(150 CHAR)'),col('telephone','VARCHAR2(30 CHAR)'),col('poste',required=True),col('date_embauche','DATE'),col('heures_hebdo','NUMBER(6,2)',True,'35','heures_hebdo>=0'),col('habilitation_medicale','NUMBER(1)',True,'0','habilitation_medicale IN (0,1)')])
entity('COMPTE','Employés','Authentification applicative. Le compte technique Oracle reste extérieur au modèle métier.',[
    ref('id_employe','EMPLOYE',True,True),col('login','VARCHAR2(80 CHAR)',True),col('role','VARCHAR2(20 CHAR)',True,check="role IN ('ADMIN','SERVICES','EMPLOYES','STOCK','LOCAUX','MEDICAL','ADOPTIONS')"),col('sel','RAW(24)',True),col('empreinte','RAW(32)',True),col('iterations','NUMBER(7)',True,'100000','iterations BETWEEN 10000 AND 1000000')])
entity('COMPETENCE','Employés','Référentiel des compétences exigées par les services.',[col('libelle',required=True)],uniques=[['libelle']])
entity('EMPLOYE_COMPETENCE','Employés','Association plusieurs-à-plusieurs entre employés et compétences.',[
    ref('id_employe','EMPLOYE'),ref('id_competence','COMPETENCE'),col('date_validation','DATE'),col('valide_jusquau','DATE')],pk=['id_employe','id_competence'],checks=['valide_jusquau IS NULL OR date_validation IS NULL OR valide_jusquau>=date_validation'],audit=False)
entity('CRENEAU_EMPLOYE','Employés','Disponibilités et absences datées pour contrôler les affectations.',[
    ref('id_employe','EMPLOYE'),col('debut','TIMESTAMP',True),col('fin','TIMESTAMP',True),col('type_creneau','VARCHAR2(20 CHAR)',True,check="type_creneau IN ('DISPONIBLE','ABSENT')"),col('motif','VARCHAR2(250 CHAR)')],checks=['fin>debut'])
entity('LOCAL','Locaux','Local partagé : capacité, équipements et état de remise en service.',[
    col('nom',required=True),col('type_local','VARCHAR2(80 CHAR)',True),col('etage','NUMBER(3)',True,'0'),col('capacite','NUMBER(6)',True,check='capacite>0'),col('surface_m2','NUMBER(8,2)',check='surface_m2>0'),col('equipements','CLOB'),col('etat','VARCHAR2(20 CHAR)',True,"'DISPONIBLE'","etat IN ('DISPONIBLE','A_NETTOYER','NETTOYAGE','MAINTENANCE','HORS_SERVICE')"),col('seuil_entretien_h','NUMBER(8,2)',True,'100','seuil_entretien_h>0')])
entity('INTERVENTION_LOCAL','Locaux','Nettoyage ou maintenance, avec auteur, période et validation du contrôle.',[
    ref('id_local','LOCAL'),ref('id_employe','EMPLOYE'),col('type_intervention','VARCHAR2(20 CHAR)',True,check="type_intervention IN ('NETTOYAGE','MAINTENANCE')"),col('debut','TIMESTAMP',True),col('fin','TIMESTAMP'),col('statut','VARCHAR2(20 CHAR)',True,"'PLANIFIE'","statut IN ('PLANIFIE','EN_COURS','TERMINE','ANNULE')"),col('controle_valide','NUMBER(1)',True,'0','controle_valide IN (0,1)'),col('observations','CLOB')],checks=['fin IS NULL OR fin>=debut',"statut<>'TERMINE' OR (fin IS NOT NULL AND controle_valide=1)"])
entity('SERVICE','Services','Prestation planifiée, conforme au cahier : animal, employé, local, date, durée et tarif. Ce n’est pas un catalogue abstrait.',[
    ref('id_animal','ANIMAL'),ref('id_employe','EMPLOYE',True),ref('id_local','LOCAL',True),col('libelle',required=True),col('categorie','VARCHAR2(80 CHAR)',True),col('debut','TIMESTAMP',True),col('duree_minutes','NUMBER(6)',True,check='duree_minutes>0'),col('tarif_tnd','NUMBER(12,3)',True,'0','tarif_tnd>=0'),col('statut','VARCHAR2(20 CHAR)',True,"'PLANIFIE'","statut IN ('PLANIFIE','EN_COURS','TERMINE','ANNULE')"),col('motif_annulation','VARCHAR2(250 CHAR)'),col('notes','CLOB')],checks=["statut NOT IN ('EN_COURS','TERMINE') OR (id_employe IS NOT NULL AND id_local IS NOT NULL)","statut<>'ANNULE' OR motif_annulation IS NOT NULL"])
entity('SERVICE_COMPETENCE','Services','Compétences requises pour une prestation, sans liste de valeurs dans une cellule.',[
    ref('id_service','SERVICE'),ref('id_competence','COMPETENCE')],pk=['id_service','id_competence'],audit=False)
entity('DOSSIER_MEDICAL','Soins','Un dossier longitudinal unique par animal ; les consultations sont des lignes distinctes.',[
    ref('id_animal','ANIMAL',unique=True),col('date_ouverture','DATE',True,'SYSDATE'),col('antecedents','CLOB'),col('allergies','CLOB'),col('observations','CLOB')])
entity('CONSULTATION','Soins','Acte clinique d’un dossier, associé à une prestation et à son vétérinaire. Poids facultatif mais strictement positif si renseigné.',[
    ref('id_dossier_medical','DOSSIER_MEDICAL'),ref('id_service','SERVICE',unique=True),ref('id_employe','EMPLOYE'),col('date_consultation','TIMESTAMP',True),col('motif','CLOB'),col('diagnostic','CLOB'),col('traitement','CLOB'),col('poids_kg','NUMBER(7,3)',check='poids_kg>0'),col('observations','CLOB'),col('validee','NUMBER(1)',True,'0','validee IN (0,1)')])
entity('VACCINATION','Soins','Événement vaccinal daté ; échéance saisie par le vétérinaire, sans calendrier médical automatique.',[
    ref('id_dossier_medical','DOSSIER_MEDICAL'),ref('id_employe','EMPLOYE'),col('produit','VARCHAR2(150 CHAR)',True),col('date_vaccination','DATE',True),col('prochaine_echeance','DATE'),col('numero_lot','VARCHAR2(100 CHAR)'),col('observations','CLOB')],checks=['prochaine_echeance IS NULL OR prochaine_echeance>=date_vaccination'])
entity('FACTURE','Factures','Au plus une facture par consultation ; créée en brouillon, puis émise après validation. Identité du client figée à l’émission.',[
    ref('id_consultation','CONSULTATION',unique=True),ref('id_client','CLIENT'),col('numero','VARCHAR2(40 CHAR)',True),col('date_emission','TIMESTAMP',True,'SYSTIMESTAMP'),col('nom_client_facture','VARCHAR2(250 CHAR)',True),col('adresse_client_facture','VARCHAR2(500 CHAR)'),col('statut','VARCHAR2(20 CHAR)',True,"'BROUILLON'","statut IN ('BROUILLON','EMISE','PAYEE','ANNULEE')"),col('date_paiement','TIMESTAMP')],checks=["statut<>'PAYEE' OR date_paiement IS NOT NULL",'date_paiement IS NULL OR date_paiement>=date_emission'],uniques=[['numero']])
entity('LIGNE_FACTURE','Factures','Détail historique : désignation, quantité et prix unitaire figés ; aucune dépendance au tarif courant du service.',[
    ref('id_facture','FACTURE'),col('numero_ligne','NUMBER(4)',True,check='numero_ligne>0'),col('designation','VARCHAR2(500 CHAR)',True),col('quantite','NUMBER(10,3)',True,check='quantite>0'),col('prix_unitaire_tnd','NUMBER(12,3)',True,check='prix_unitaire_tnd>=0')],pk=['id_facture','numero_ligne'],audit=False)
entity('STOCK','Stock','Emplacement d’inventaire ; un local peut accueillir plusieurs zones de stock.',[
    ref('id_local','LOCAL',True),col('nom',required=True),col('description','CLOB')],uniques=[['nom']])
entity('ARTICLE','Stock','Référence produit commune à plusieurs lots et emplacements. Quantité et péremption appartiennent au lot.',[
    col('reference','VARCHAR2(80 CHAR)',True),col('designation','VARCHAR2(150 CHAR)',True),col('categorie','VARCHAR2(80 CHAR)',True),col('unite','VARCHAR2(30 CHAR)',True),col('seuil_alerte','NUMBER(12,3)',True,'0','seuil_alerte>=0'),col('prix_unitaire_tnd','NUMBER(12,3)',True,'0','prix_unitaire_tnd>=0'),col('fournisseur','VARCHAR2(150 CHAR)'),col('delai_fournisseur_j','NUMBER(5)',True,'0','delai_fournisseur_j>=0')],uniques=[['reference']])
entity('LOT_STOCK','Stock','Lot d’un article dans un stock, avec quantité courante et échéance. Les sorties sont atomiques et journalisées.',[
    ref('id_article','ARTICLE'),ref('id_stock','STOCK'),col('numero_lot','VARCHAR2(100 CHAR)',True),col('quantite','NUMBER(12,3)',True,'0','quantite>=0'),col('date_peremption','DATE'),col('bloque','NUMBER(1)',True,'0','bloque IN (0,1)')],uniques=[['id_article','id_stock','numero_lot']])
entity('MOUVEMENT_STOCK','Stock','Historique immuable des entrées et sorties ; une correction se fait par mouvement inverse motivé.',[
    ref('id_lot_stock','LOT_STOCK'),ref('id_compte','COMPTE'),ref('id_service','SERVICE',True),col('type_mouvement','VARCHAR2(10 CHAR)',True,check="type_mouvement IN ('ENTREE','SORTIE')"),col('quantite','NUMBER(12,3)',True,check='quantite>0'),col('date_mouvement','TIMESTAMP',True,'SYSTIMESTAMP'),col('motif','VARCHAR2(500 CHAR)',True)],audit=False)
entity('ADOPTANT','Adoptions','Profil de candidature d’une personne CLIENT ; identité non dupliquée.',[
    ref('id_client','CLIENT',unique=True),col('logement','VARCHAR2(100 CHAR)',True),col('espace_exterieur','NUMBER(1)',True,'0','espace_exterieur IN (0,1)'),col('nombre_enfants','NUMBER(3)',True,'0','nombre_enfants>=0'),col('autres_animaux','CLOB'),col('experience','CLOB'),col('disponibilite','VARCHAR2(250 CHAR)')])
entity('ADOPTION','Adoptions','Demande pour un animal, instruite par un référent. Une seule demande validée active par animal.',[
    ref('id_animal','ANIMAL'),ref('id_adoptant','ADOPTANT'),ref('id_employe','EMPLOYE'),col('date_demande','DATE',True,'SYSDATE'),col('statut','VARCHAR2(20 CHAR)',True,"'EN_ETUDE'","statut IN ('EN_ETUDE','VALIDEE','REFUSEE','ANNULEE')"),col('date_decision','DATE'),col('date_placement','DATE'),col('pieces_verifiees','NUMBER(1)',True,'0','pieces_verifiees IN (0,1)'),col('rencontre_effectuee','NUMBER(1)',True,'0','rencontre_effectuee IN (0,1)'),col('accord_responsable','NUMBER(1)',True,'0','accord_responsable IN (0,1)'),col('observations','CLOB')],checks=['date_decision IS NULL OR date_decision>=date_demande','date_placement IS NULL OR (date_decision IS NOT NULL AND date_placement>=date_decision)',"statut<>'VALIDEE' OR (date_decision IS NOT NULL AND pieces_verifiees=1 AND rencontre_effectuee=1 AND accord_responsable=1)","date_placement IS NULL OR statut='VALIDEE'"])
entity('SUIVI_ADOPTION','Adoptions','Actions de suivi J+7 et J+30, contact et résultat ; validation humaine.',[
    ref('id_adoption','ADOPTION'),ref('id_compte','COMPTE',True),col('jalon_jours','NUMBER(2)',True,check='jalon_jours IN (7,30)'),col('date_prevue','DATE',True),col('date_contact','TIMESTAMP'),col('statut','VARCHAR2(20 CHAR)',True,"'A_FAIRE'","statut IN ('A_FAIRE','REALISE','ANNULE')"),col('resultat','CLOB')],checks=["statut<>'REALISE' OR date_contact IS NOT NULL"],uniques=[['id_adoption','jalon_jours']])

BY={t['name']:t for t in tables}
relations=[]
for t in tables:
    for c in t['columns']:
        if 'ref' in c:
            relations.append(dict(parent=c['ref'],child=t['name'],field=c['name'],optional=not c['required'],unique=c.get('unique',False)))

def ddl():
    lines=['-- ANIWORLD / Modele physique commun cible S5 / Oracle XE 21c',
           '-- A executer dans un SCHEMA VIDE dedie. Ce fichier ne migre pas ANIWORLD_APP.',
           '-- Le schema de compatibilite Qt reste dans database/oracle/schema.sql.',
           'whenever sqlerror exit failure rollback','set define off','set serveroutput on',
           "BEGIN IF SYS_CONTEXT('USERENV','CURRENT_SCHEMA') IN ('ANIWORLD_APP','SYS','SYSTEM') THEN RAISE_APPLICATION_ERROR(-20001,'Utiliser un schema vide dedie au modele S5'); END IF; END;",'/']
    for i,t in enumerate(tables,1):
        defs=[]
        for c in t['columns']:
            s=c['name']+' '+c['type']
            if c.get('identity'): s+=' GENERATED BY DEFAULT ON NULL AS IDENTITY'
            elif c['default'] is not None: s+=' DEFAULT '+c['default']
            if c['required'] and c['name'] not in t['pk']: s+=' NOT NULL'
            defs.append(s)
        defs.append(f"CONSTRAINT pk_{t['name'].lower()} PRIMARY KEY ({','.join(t['pk'])})")
        for j,cols in enumerate(t['uniques']+[[c['name']] for c in t['columns'] if c.get('unique')],1):
            defs.append(f"CONSTRAINT uq_{i}_{j} UNIQUE ({','.join(cols)})")
        checks=t['checks']+[c['check'] for c in t['columns'] if c.get('check')]
        for j,check in enumerate(checks,1): defs.append(f'CONSTRAINT ck_{i}_{j} CHECK ({check})')
        lines.append('CREATE TABLE '+t['name']+' (\n  '+',\n  '.join(defs)+'\n);')
        desc=t['description'].replace("'","''")
        lines.append(f"COMMENT ON TABLE {t['name']} IS '{desc}';")
    for i,r in enumerate(relations,1):
        parentpk=BY[r['parent']]['pk'][0]
        lines.append(f"ALTER TABLE {r['child']} ADD CONSTRAINT fk_s5_{i} FOREIGN KEY ({r['field']}) REFERENCES {r['parent']}({parentpk});")
        # Leading PK/unique columns are indexed already.
        t=BY[r['child']]
        indexed=t['pk'][0]==r['field'] or r['unique'] or any(x[0]==r['field'] for x in t['uniques'])
        if not indexed: lines.append(f"CREATE INDEX ix_s5_fk_{i} ON {r['child']}({r['field']});")
    lines.extend([
        'CREATE UNIQUE INDEX uq_compte_login_ci ON COMPTE(LOWER(login));',
        'CREATE UNIQUE INDEX uq_employe_email_ci ON EMPLOYE(LOWER(email));',
        'CREATE UNIQUE INDEX uq_local_nom_ci ON LOCAL(LOWER(nom));',
        "CREATE UNIQUE INDEX uq_adoption_active ON ADOPTION(CASE WHEN statut='VALIDEE' AND archive=0 THEN id_animal END);",
        'CREATE INDEX ix_service_periode ON SERVICE(debut,id_employe,id_local);',
        'CREATE INDEX ix_lot_echeance ON LOT_STOCK(date_peremption);',
        'CREATE INDEX ix_vaccin_echeance ON VACCINATION(prochaine_echeance);',
        'CREATE VIEW V_TOTAL_FACTURE AS SELECT f.id_facture,f.numero,f.id_consultation,f.id_client,f.statut, NVL(SUM(ROUND(l.quantite*l.prix_unitaire_tnd,3)),0) AS total_tnd FROM FACTURE f LEFT JOIN LIGNE_FACTURE l ON l.id_facture=f.id_facture GROUP BY f.id_facture,f.numero,f.id_consultation,f.id_client,f.statut;',
        'CREATE VIEW V_STOCK_ARTICLE AS SELECT a.id_article,a.reference,a.designation,NVL(SUM(CASE WHEN l.archive=0 AND l.bloque=0 AND (l.date_peremption IS NULL OR l.date_peremption>=TRUNC(SYSDATE)) THEN l.quantite ELSE 0 END),0) AS disponible FROM ARTICLE a LEFT JOIN LOT_STOCK l ON l.id_article=a.id_article GROUP BY a.id_article,a.reference,a.designation;'])
    for i,t in enumerate(tables,1):
        if t['audit']:
            lines.extend([f"CREATE OR REPLACE TRIGGER tr_s5_mod_{i} BEFORE UPDATE ON {t['name']} FOR EACH ROW BEGIN :NEW.modifie_le:=SYSTIMESTAMP; END;",'/'])
    lines.extend(["CREATE OR REPLACE TRIGGER tr_mouvement_immuable BEFORE UPDATE OR DELETE ON MOUVEMENT_STOCK BEGIN RAISE_APPLICATION_ERROR(-20020,'Historique immuable : enregistrer un mouvement inverse motive'); END;",'/'])
    lines+=['-- Regles multi-lignes et workflows : voir dossier de conception, controles transactionnels Qt requis.', 'COMMIT;']
    return '\n\n'.join(lines)+'\n'

OUT.joinpath('modele_commun.json').write_text(json.dumps(dict(tables=tables,relations=relations),ensure_ascii=False,indent=2),encoding='utf-8')
OUT.joinpath('modele_physique_oracle.sql').write_text(ddl(),encoding='utf-8')

# Draw.io XML, native editable table cards and real connector endpoints.
# Numeric cardinality labels avoid ambiguity when Lucid imports line-end markers.
positions={
 'CLIENT':(50,150),'ANIMAL':(510,150),'DOSSIER_MEDICAL':(970,150),'CONSULTATION':(1430,150),'VACCINATION':(1890,150),
 'EMPLOYE':(50,850),'COMPTE':(510,850),'SERVICE':(970,850),'FACTURE':(1430,850),'LIGNE_FACTURE':(1890,850),
 'COMPETENCE':(50,1550),'EMPLOYE_COMPETENCE':(510,1550),'SERVICE_COMPETENCE':(970,1550),'ADOPTANT':(1430,1550),'ADOPTION':(1890,1550),
 'CRENEAU_EMPLOYE':(50,2250),'LOCAL':(510,2250),'INTERVENTION_LOCAL':(970,2250),'SUIVI_ADOPTION':(1430,2250),'STOCK':(1890,2250),
 'ARTICLE':(510,2950),'LOT_STOCK':(970,2950),'MOUVEMENT_STOCK':(1430,2950),
}
colors={'Soins':'#247B91','Employés':'#6E5D9A','Services':'#277666','Locaux':'#BC7A2C','Factures':'#337AA8','Stock':'#B87537','Adoptions':'#56815A'}
def make_drawio(mode='MLD'):
    mx=Element('mxfile',host='app.diagrams.net',version='24.7.17')
    dia=SubElement(mx,'diagram',name='ANIWORLD - '+mode+' commun corrige',id='aniworld-s5-'+mode)
    model=SubElement(dia,'mxGraphModel',dx='2400',dy='3650',grid='1',gridSize='10',page='1',pageScale='1',pageWidth='2400',pageHeight='3650')
    root=SubElement(model,'root');SubElement(root,'mxCell',id='0');SubElement(root,'mxCell',id='1',parent='0')
    for t in tables:
        color=colors[t['group']]
        rows=[]
        for c in t['columns']:
            if mode!='MPD' and c['name'] in ('cree_le','modifie_le','archive'): continue
            if mode=='MCD' and 'ref' in c: continue
            tag=('PK ' if c['name'] in t['pk'] else '')+('FK ' if 'ref' in c else '')+('UQ ' if c.get('unique') or [c['name']] in t['uniques'] else '')
            label=tag+c['name']+(' *' if c['required'] else '')
            if mode=='MPD': label+=' : '+c['type'].replace(' CHAR','')
            rows.append('<div style="padding:3px 10px;border-bottom:1px solid #e1ebee;text-align:left">'+escape(label)+'</div>')
        html=f'<div style="background:{color};color:white;padding:10px;font-size:17px"><b>{t["name"]}</b></div>'+''.join(rows)
        h=46+len(rows)*26
        cell=SubElement(root,'mxCell',id=t['name'],value=html,style='rounded=1;whiteSpace=wrap;html=1;align=left;verticalAlign=top;spacing=0;overflow=fill;fillColor=#FFFFFF;strokeColor='+color+';fontFamily=Arial;fontSize=14;',vertex='1',parent='1')
        x,y=positions[t['name']];SubElement(cell,'mxGeometry',x=str(x),y=str(y),width='365',height=str(h),**{'as':'geometry'})
    for i,r in enumerate(relations):
        # Labels specify how many parent/child records may be associated.
        source_card='0..1' if r['optional'] else '1'
        target_card='0..1' if r['unique'] else '0..N'
        label=r['parent']+' ['+source_card+'] - ['+target_card+'] '+r['child']
        cell=SubElement(root,'mxCell',id='rel'+str(i),value=escape(label),source=r['parent'],target=r['child'],edge='1',parent='1',style='edgeStyle=orthogonalEdgeStyle;rounded=0;html=1;strokeColor=#81949C;strokeWidth=1.5;endArrow=none;startArrow=none;fontSize=11;labelBackgroundColor=#FFFFFF;')
        SubElement(cell,'mxGeometry',relative='1',**{'as':'geometry'})
    title=SubElement(root,'mxCell',id='title',value=f'ANIWORLD | {mode} COMMUN CORRIGÉ | 6 MODULES | 14 OCTOBRE 2026',style='text;html=1;fontSize=28;fontStyle=1;fontColor=#174C5B;',vertex='1',parent='1')
    SubElement(title,'mxGeometry',x='40',y='20',width='2300',height='55',**{'as':'geometry'})
    legend=SubElement(root,'mxCell',id='legend',value='PK : clé primaire | FK : clé étrangère | UQ : unique | * : obligatoire. Cardinalités près du parent = parents par enfant, et réciproquement. Audit commun : cree_le, modifie_le, archive (sauf associations, lignes, mouvements). Modèle cible S5 ; migration Qt distincte.',style='text;html=1;fontSize=16;fontColor=#4C6974;',vertex='1',parent='1')
    SubElement(legend,'mxGeometry',x='40',y='80',width='2300',height='45',**{'as':'geometry'})
    ElementTree(mx).write(OUT/f'ANIWORLD_{mode}_commun.drawio',encoding='utf-8',xml_declaration=True)
for mode in ['MLD','MPD']: make_drawio(mode)
# MCD Merise views are generated with the PDF by create_common_model_report.py.

reltext=['ANIWORLD - MODELE RELATIONNEL COMMUN S5','PK = cle primaire ; # = cle etrangere ; ? = nullable ; UQ = unique.','']
for t in tables:
    fields=[]
    for c in t['columns']:
        suffix=(' [PK]' if c['name'] in t['pk'] else '')+(' [UQ]' if c.get('unique') else '')+('?' if not c['required'] else '')
        fields.append(('#' if 'ref' in c else '')+c['name']+suffix)
    reltext += [t['name']+'('+', '.join(fields)+')',t['description']]
    if t['uniques']: reltext.append('  UNIQUE : '+' ; '.join(' + '.join(u) for u in t['uniques']))
    for r in relations:
        if r['child']==t['name']: reltext.append('  '+r['field']+' -> '+r['parent']+'('+BY[r['parent']]['pk'][0]+')')
    reltext.append('')
OUT.joinpath('modele_relationnel.txt').write_text('\n'.join(reltext),encoding='utf-8')
print(f'{len(tables)} tables, {len(relations)} relations, MLD/MPD draw.io et SQL generes.')
