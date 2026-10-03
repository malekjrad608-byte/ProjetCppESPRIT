from pathlib import Path
from copy import deepcopy
from PIL import Image, ImageChops
from docx import Document
from docx.shared import Cm, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_CELL_VERTICAL_ALIGNMENT
from docx.oxml import OxmlElement
from docx.oxml.ns import qn

ROOT=Path(r'C:\Users\malek\Documents\gestion_service')
WORK=ROOT/'tmp/cahier_specification'
SOURCE=ROOT/'output/documents/ANIWORLD_devCore_Cahier_de_specification_v3_compact.docx'
DEST=ROOT/'output/documents/ANIWORLD_DevCore_Cahier_de_specification_v4.docx'
old=Document(SOURCE)
d=Document(SOURCE)
oldtext=[p.text for p in old.paragraphs]
oldtables=[[[c.text for c in r.cells] for r in t.rows] for t in old.tables]
# Keep the verified cover and native Word contents field, rebuild the body compactly.
start=next(p._p for p in d.paragraphs if p.style.name=='Heading 1' and p.text=='1 Contexte général')
body=d.element.body
for e in list(body)[list(body).index(start):]:
    if e.tag!=qn('w:sectPr'):body.remove(e)
sec=d.sections[0]
sec.top_margin=Cm(1.5);sec.bottom_margin=Cm(1.5)
sec.left_margin=Cm(1.7);sec.right_margin=Cm(1.7)
sec.header_distance=Cm(.65);sec.footer_distance=Cm(.65)
for part in d.part.package.parts:
    if hasattr(part,'element'):
        for t in part.element.iter(qn('w:t')):
            if t.text:t.text=t.text.replace('Version 1.3','Version 1.4').replace('devCore','DevCore').replace('27 septembre 2026','1 octobre 2026')
d.core_properties.title='ANIWORLD — Cahier de spécification — Version compacte'
d.core_properties.subject='Client fictif Les Compagnons — Groupe DevCore — Projet C++ 2A'
d.core_properties.author='DevCore'
for name in ['Normal','Body Text']:
    st=d.styles[name];st.font.name='Calibri';st.font.size=Pt(10.5)
    st.paragraph_format.line_spacing=1.02;st.paragraph_format.space_after=Pt(4)
for name,size in [('Heading 1',17),('Heading 2',13),('Heading 3',12),('Heading 4',10.5)]:
    st=d.styles[name];st.font.size=Pt(size);st.font.color.rgb=RGBColor.from_string('225A64')
    st.paragraph_format.space_before=Pt(8);st.paragraph_format.space_after=Pt(4)
    st.paragraph_format.keep_with_next=True;st.paragraph_format.line_spacing=1
for name in ['TOC 1','TOC 2','TOC 3']:
    st=next(s for s in d.styles if s.name.lower()==name.lower());st.font.size=Pt(10)
    st.paragraph_format.space_after=Pt(2);st.paragraph_format.line_spacing=1
d.styles['Caption'].font.size=Pt(9)
d.styles['Caption'].paragraph_format.space_after=Pt(3)

def p(text='',boldlead=None,size=10.5,after=4,style=None):
    x=d.add_paragraph(style=style)
    if x._p.getprevious() is not None and x._p.getprevious().tag==qn('w:tbl'):
        x.paragraph_format.space_before=Pt(3)
    if boldlead and text.startswith(boldlead):x.add_run(boldlead).bold=True;x.add_run(text[len(boldlead):])
    else:x.add_run(text)
    x.paragraph_format.space_after=Pt(after)
    x.paragraph_format.line_spacing=1.02
    for r in x.runs:r.font.size=Pt(size)
    return x
def h(text,level=2):return d.add_heading(text,level)
def page():d.add_page_break()
def table(headers,rows,widths,font=9.5):
    t=d.add_table(rows=1,cols=len(headers));t.alignment=WD_TABLE_ALIGNMENT.CENTER;t.autofit=False
    for col,w in zip(t.columns,widths):col.width=Cm(w)
    pr=t._tbl.tblPr;mar=OxmlElement('w:tblCellMar')
    for edge in ['top','bottom','left','right']:
        v=OxmlElement('w:'+edge);v.set(qn('w:w'),'55' if edge in ['top','bottom'] else '85');v.set(qn('w:type'),'dxa');mar.append(v)
    pr.append(mar)
    borders=OxmlElement('w:tblBorders')
    for edge in ['top','left','bottom','right','insideH','insideV']:
        e=OxmlElement('w:'+edge);e.set(qn('w:val'),'single');e.set(qn('w:sz'),'3');e.set(qn('w:color'),'D9E1E4');borders.append(e)
    pr.append(borders)
    for i,v in enumerate(headers):t.rows[0].cells[i].text=v
    for row in rows:
        for c,text in zip(t.add_row().cells,row):c.text=text
    for ri,row in enumerate(t.rows):
        trpr=row._tr.get_or_add_trPr();trpr.append(OxmlElement('w:cantSplit'))
        if ri==0:trpr.append(OxmlElement('w:tblHeader'))
        for c,w in zip(row.cells,widths):
            c.width=Cm(w);c.vertical_alignment=WD_CELL_VERTICAL_ALIGNMENT.CENTER
            sh=OxmlElement('w:shd');sh.set(qn('w:fill'),'334E57' if ri==0 else ('F1F6F7' if ri%2 else 'FFFFFF'));c._tc.get_or_add_tcPr().append(sh)
            for cp in c.paragraphs:
                cp.paragraph_format.space_after=Pt(0);cp.paragraph_format.space_before=Pt(0);cp.paragraph_format.line_spacing=1
                if ri==0:cp.paragraph_format.keep_with_next=True
                for r in cp.runs:r.font.size=Pt(font);r.bold=ri==0;r.font.color.rgb=RGBColor.from_string('FFFFFF' if ri==0 else '263B42')
    return t
def short(text,label):p(text,boldlead=label,after=4)
def crop_shape(shape,path,width):
    im=Image.open(path);w,h=im.size
    if name_is_app(path):l,t,r,b=ImageChops.difference(im.convert('RGB'),Image.new('RGB',im.size,'white')).getbbox()
    else:l,t,r,b=im.getbbox()
    shape.width=Cm(width);shape.height=Cm(width*(b-t)/(r-l))
    ext=shape._inline.xpath('.//a:xfrm/a:ext')[0];ext.set('cx',str(shape.width));ext.set('cy',str(shape.height))
    blip=shape._inline.xpath('.//a:blip')[0];fill=blip.getparent()
    crop=OxmlElement('a:srcRect')
    for k,v in [('l',l/w),('t',t/h),('r',(w-r)/w),('b',(h-b)/h)]:crop.set(k,str(round(v*100000)))
    fill.insert(1,crop)


def name_is_app(path):return Path(path).name=='Plan de travail 1.png'
def crop_to(shape,path,box,width):
    w,h=Image.open(path).size;l,t,r,b=box
    shape.width=Cm(width);shape.height=Cm(width*(b-t)/(r-l))
    ext=shape._inline.xpath('.//a:xfrm/a:ext')[0];ext.set('cx',str(shape.width));ext.set('cy',str(shape.height))
    fill=shape._inline.xpath('.//a:blip')[0].getparent()
    for x in list(fill.findall(qn('a:srcRect'))):fill.remove(x)
    crop=OxmlElement('a:srcRect')
    for k,v in [('l',l/w),('t',t/h),('r',(w-r)/w),('b',(h-b)/h)]:crop.set(k,str(round(v*100000)))
    fill.insert(1,crop)
coverparas=d.paragraphs
group=coverparas[3];group.clear();group.alignment=WD_ALIGN_PARAGRAPH.CENTER
group.add_run().add_picture(r'C:\Users\malek\Downloads\WhatsApp Image 2026-09-30 at 11.27.16 PM.jpeg',width=Cm(5.0))
for run in coverparas[4].runs:run.text=run.text.replace('devCore','DevCore')
brand=coverparas[5];brand.clear();brand.alignment=WD_ALIGN_PARAGRAPH.CENTER
shape=brand.add_run().add_picture(r'C:\Users\malek\Downloads\Plan de travail 1.png');crop_shape(shape,Path(r'C:\Users\malek\Downloads\Plan de travail 1.png'),2.5)
brand.paragraph_format.space_after=Pt(2);brand.paragraph_format.keep_with_next=True
from docx.text.paragraph import Paragraph
name=OxmlElement('w:p');brand._p.addnext(name);np=Paragraph(name,brand._parent);np.alignment=WD_ALIGN_PARAGRAPH.CENTER
nr=np.add_run('ANIWORLD');nr.bold=True;nr.font.size=Pt(22);nr.font.color.rgb=RGBColor.from_string('263B42');np.paragraph_format.space_after=Pt(8)
coverparas[8].text='Services  •  Employés  •  Stock\nLocaux  •  Dossiers médicaux  •  Adoptions'
coverparas[8].alignment=WD_ALIGN_PARAGRAPH.CENTER
for run in coverparas[8].runs:run.font.size=Pt(10.5)
for cp in d.paragraphs:
    if cp.text.startswith('Les identifiants SER, EMP, STK, LOC et DM'):
        cp.text='Les identifiants SER, EMP, STK, LOC, DM et ADOP relient les exigences aux six modules. Les critères de recette décrivent les résultats attendus lors de la démonstration.'

# Page 3: one named fictional client, his complete problem and the expected benefit.
h('1 Contexte général',1)
p('Le groupe DevCore propose ANIWORLD à M. Sami Ben Salem, responsable du centre de soins animaliers « Les Compagnons ». Ce client et son établissement sont fictifs : ils servent de scénario de référence au projet pédagogique.')
h('1.1 Étude de l’existant',2)
p('Dans ce scénario, Les Compagnons accueille des chiens et des chats pour des consultations, du toilettage, de la pension et de l’éducation. M. Ben Salem coordonne une équipe pluridisciplinaire et plusieurs espaces ; le centre accompagne aussi des adoptions. L’accueil organise les rendez-vous ; les employés réalisent les prestations ; le vétérinaire assure le suivi médical.')
p('Le centre utilise un agenda pour les réservations, des fichiers séparés pour les employés et le stock, ainsi que des dossiers papier pour les animaux. Les disponibilités des salles sont souvent communiquées oralement ; les candidatures à l’adoption sont suivies dans des échanges séparés. Une même information doit être recherchée ou recopiée à plusieurs endroits, sans vue commune de l’activité.')
h('1.2 Problématique',2)
p('M. Ben Salem perd du temps à vérifier qui est disponible, quelle salle peut accueillir un animal et quels produits restent en stock. Une absence peut désorganiser les rendez-vous ; un oubli de nettoyage retarde une prestation ; un dossier introuvable complique le suivi vétérinaire. Il manque aussi d’indicateurs fiables pour préparer sa journée, répartir le travail et suivre les demandes d’adoption après le départ d’un animal.')
short('Question centrale. Comment permettre à M. Ben Salem de gérer son centre depuis un seul outil, de limiter les oublis et les erreurs et de consacrer davantage de temps aux animaux et à leurs propriétaires ?', 'Question centrale.')
table(['Difficulté du client','Réponse apportée par ANIWORLD','Bénéfice attendu'],[
    ['Services dispersés dans l’agenda et tarifs à recalculer.','Planning commun, suivi des statuts, recherche de créneaux compatibles et estimation détaillée d’une prestation.','Préparer un rendez-vous plus facilement et limiter les conflits de réservation.'],
    ['Compétences, absences et charge des employés peu visibles.','Annuaire, disponibilités, comparaison des compétences et suggestion de réaffectation vers un profil moins chargé.','Choisir une personne adaptée et mieux répartir les tâches.'],
    ['Produits manquants ou lots oubliés jusqu’à leur péremption.','Traçabilité des entrées et sorties, alertes de seuil, sortie par péremption et proposition de réapprovisionnement.','Anticiper les achats et réduire les ruptures et le gaspillage.'],
    ['Occupation, nettoyage et entretien des espaces mal coordonnés.','Vue des capacités et réservations, validation du nettoyage et planification de la maintenance.','Prévoir un local disponible et prêt pour la prochaine prestation.'],
    ['Historique d’un animal difficile à retrouver et suivis à préparer.','Dossier centralisé avec consultations, allergies, vaccins, rappels à valider et évolution du poids.','Retrouver les informations utiles et faciliter le travail du vétérinaire.'],
    ['Candidatures à l’adoption dispersées et suivi après départ irrégulier.','Dossier de candidature, comparaison animal–foyer, décision validée et suivi à J+7 / J+30.','Organiser les placements et garder une trace des échanges avec les adoptants.'],
],[4.4,8.0,5.2],9.3)
p('Le tableau de bord, la recherche, les tris, les statistiques et les exports donnent au responsable une vue rapide de l’activité. Les comptes par rôle, les sauvegardes et l’historique des opérations soutiennent une utilisation organisée. ANIWORLD doit ainsi rendre le quotidien du client plus simple ; ces bénéfices seront vérifiés lors de la démonstration, sans gain chiffré présumé.',after=0)

# Page 4: plan-required five-column table and common scope.
page();h('2 La solution proposée et ses exigences',1)
p('Notre solution est ANIWORLD, une application de bureau qui réunit les six gestions du centre Les Compagnons dans une base commune. M. Ben Salem consulte les priorités depuis le tableau de bord, puis accède au module concerné. Une donnée enregistrée peut être réutilisée dans la planification et le suivi, ce qui limite les ressaisies.')
h('2.1 Exigences fonctionnelles',2)
p('Le tableau reprend les rubriques imposées par le plan. Chaque module comprend le CRUD, le tri, la recherche, l’export et des statistiques graphiques, complétés par deux métiers innovants. Les fiches suivantes précisent les règles et les résultats attendus.')
overview=oldtables[2][1:]+[['Gestion des Adoptions','Adoption','ID, animal, candidat, foyer, date, statut, référent ; validation et suivi.','Référent adoption ; responsable.','CRUD ; tri date/statut ; recherche ; export PDF/CSV ; statistiques par statut.\nADOP-I1 : compatibilité avec le foyer.\nADOP-I2 : suivi après adoption.']]
table(oldtables[2][0],overview,[2.2,1.65,4.05,2.65,7.05],9.2)
h('Périmètre et accès',4)
p('L’accueil planifie les prestations ; les soigneurs suivent leur exécution ; le responsable organise les ressources ; le magasinier gère les stocks ; le référent des locaux valide leur entretien. Le vétérinaire consulte et complète le contenu clinique. Le référent adoption instruit les candidatures ; le responsable valide le placement. L’administrateur gère les comptes, paramètres et sauvegardes ; un rôle médical explicite reste nécessaire pour accéder au dossier clinique.')
p('Les animaux et propriétaires appartiennent aux données de référence du module DM ; les rendez-vous relèvent des Services. La paie, la comptabilité, la vente en ligne, le diagnostic automatique et l’envoi automatique de messages sont hors périmètre. Les propositions métier sont validées par un utilisateur.')
h('Conventions communes',4)
p('CRUD : créer, consulter, modifier et supprimer. Une fiche sans historique peut être supprimée après confirmation ; une fiche liée est archivée. Les observations médicales validées et mouvements de stock se corrigent sans effacer leur historique. Les listes, statistiques et exports partagent les filtres et les droits d’accès.')
p('Chaque objet possède un identifiant, des dates de création et modification et un état actif ou archivé. Affichage : dates jour/mois/année, heures sur 24 heures, montants en TND à trois décimales. La première version utilise C++, Qt Widgets, Qt SQL et une base SQLite locale sur un poste Windows, utilisé successivement par des comptes nominatifs.',after=0)

# Pages 5-7: retain every numbered base requirement; compact duplicate prose.
module_data=[
    ('2.1.1 Gestion des Services',3,oldtext[62],
     'SER-I1 Créneaux compatibles. Proposer jusqu’à trois créneaux selon la durée, les disponibilités de l’employé, les réservations du local et sa remise en état. Recontrôler ces contraintes à la confirmation.',
     'SER-I2 Estimation de prestation. Calculer le prix à partir du tarif de base, des options et d’une remise autorisée ; afficher le détail, exporter l’estimation en PDF et conserver les paramètres validés.',
     'Règles. Libellé, animal et durée positive obligatoires ; tarif positif ou nul. Cycle : Planifié, En cours, Terminé ; annulation motivée. Un service terminé conserve ses ressources et consommations.',
     'Recette. Salle réservée de 10 h à 11 h : refuser une demande à 10 h 30. Base de 50 TND + option de 10 TND, remise de 10 % : résultat 54,000 TND.'),
    ('2.1.2 Gestion des Employés',4,oldtext[68],
     'EMP-I1 Adéquation des compétences. Comparer le besoin d’une prestation aux habilitations des employés actifs. Proposer les profils satisfaisant tous les critères et expliquer le choix ; réserver le créneau reste une étape distincte.',
     'EMP-I2 Équilibrage de charge. Calculer le rapport hebdomadaire entre temps planifié et temps disponible hors absences. Au-delà du seuil configuré, suggérer une réaffectation vers un profil habilité moins chargé, à valider par le responsable.',
     'Règles. Courriel unique et contrôlé s’il est renseigné. Une absence bloque l’affectation ; une disponibilité nulle exclut le calcul de charge. Traiter les services futurs avant archivage.',
     'Recette. Exclure un profil sans compétence requise. Pour 24 h planifiées sur 30 h disponibles, afficher 80 %. Refuser une affectation à 15 h pendant une absence de 14 h à 17 h.'),
    ('2.1.3 Gestion du Stock',5,oldtext[75],
     'STK-I1 Sortie selon la péremption. Appliquer FEFO : proposer le lot utilisable dont l’échéance est la plus proche. Exclure les lots expirés, bloqués ou vides et présenter leur quantité avant validation.',
     'STK-I2 Réapprovisionnement. À partir de la consommation moyenne des 30 derniers jours, proposer une quantité couvrant le délai fournisseur et le stock de sécurité, après déduction du disponible. Sans historique, utiliser le seuil manuel ; aucune commande automatique.',
     'Règles. Quantité positive et stock jamais négatif. Sortie atomique, datée et liée à son auteur ; une correction crée un mouvement inverse justifié. Le disponible est calculé par lot.',
     'Recette. Préférer un lot valide expirant le 31/10 à un lot du 31/12. Refuser 6 unités pour 5 disponibles. À 2/jour × 7 jours + 6 de sécurité − 10 disponibles : proposer 10 unités.'),
    ('2.1.4 Gestion des Locaux',6,oldtext[81],
     'LOC-I1 Remise en service après nettoyage. Après une prestation concernée, passer le local à À nettoyer et bloquer la prise en charge suivante. Exiger la validation de la liste de contrôle par un agent habilité, avec auteur et heure.',
     'LOC-I2 Maintenance selon l’usage. Cumuler les heures d’utilisation ou passages, alerter au seuil configuré et planifier une intervention avec indisponibilité. Réinitialiser le compteur après validation de l’intervention.',
     'Règles. Capacité entière positive ; les occupations simultanées ne la dépassent pas. Un local à nettoyer ou en maintenance ne peut accueillir un service. Inclure le nettoyage dans les contrôles de disponibilité.',
     'Recette. Pour une capacité de 2 déjà engagée à 2, refuser une réservation simultanée. Interdire la remise en service sans contrôle validé. Déclencher l’alerte à 100 h pour un seuil de 100 h.'),
    ('2.1.5 Gestion des dossiers médicaux',7,oldtext[88],
     'DM-I1 Rappels vaccinaux. Repérer les échéances saisies par le vétérinaire arrivant dans les 7 jours et les retards. Préparer une liste et un texte de rappel à valider ; aucun calendrier médical n’est calculé automatiquement.',
     'DM-I2 Variation de poids. Comparer les deux dernières pesées valides et signaler une variation absolue dépassant le seuil du vétérinaire. Afficher les mesures et leurs dates pour vérification, sans produire de diagnostic.',
     'Règles. Un dossier actif unique par animal ; poids strictement positif. Conserver auteur et date des observations validées. L’accueil voit l’identité, tandis que le contenu clinique exige une habilitation médicale.',
     'Recette. Refuser l’ouverture et l’export clinique à l’accueil. Avec un seuil de 10 %, passer de 10 kg à 8,5 kg donne −15 % et une alerte. Une seule pesée ne déclenche rien.'),
    ('2.1.6 Gestion des Adoptions',None,
     'Adoption : idAdoption, idAnimal, idCandidat, dateDemande, statut, idRéférent, dateDécision, datePlacement. Candidat : identité, contact, logement, expérience et composition du foyer. Données associées : pièces, rencontre, décision et suivis.',
     'ADOP-I1 Compatibilité avec le foyer. Comparer les besoins renseignés de l’animal aux caractéristiques du candidat : logement, activité, enfants et autres animaux. Afficher les critères concordants, manquants et leur proportion ; le responsable prend la décision.',
     'ADOP-I2 Suivi après adoption. À la date de placement, créer deux actions à J+7 et J+30. Enregistrer contact, réponse et résultat ; signaler les actions en retard au référent, qui décide de la suite à donner.',
     'Règles. Cycle : En étude, Validée, Refusée ou Annulée. Une seule adoption validée et active par animal ; décision et placement datés. Vérifier pièces, rencontre et accord du responsable avant validation. Le référent voit une synthèse autorisée sans accès clinique.',
     'Recette. Pour 3 critères concordants sur 4 évaluables, afficher 75 % et le critère restant. Bloquer une seconde adoption active ; pour un placement le 05/10, créer des suivis les 12/10 et 04/11.'),
]
for i,(heading,ti,attrs,innovation1,innovation2,rules,test) in enumerate(module_data):
    if i in [0,2,4,5]:page()
    hh=h(heading,3)
    if i in [1,3]:hh.paragraph_format.space_before=Pt(12)
    p(attrs,size=10,after=4)
    if ti is None:
        table(['Exigence','Fonctionnalité attendue'],[
            ['ADOP-01 CRUD','Créer une demande ; consulter le dossier ; modifier avant décision ; supprimer un brouillon ou archiver une demande traitée.'],
            ['ADOP-02 Trier','Trier par date de demande, animal, candidat ou statut, dans les deux sens.'],
            ['ADOP-03 Rechercher','Rechercher par dossier, animal ou candidat ; filtrer par espèce, référent, période et statut.'],
            ['ADOP-04 Exporter','Exporter un dossier autorisé en PDF et une liste filtrée en CSV ; limiter les données personnelles au besoin du destinataire.'],
            ['ADOP-05 Statistiques','Présenter les demandes par statut, les placements par période et la part de suivis réalisés à échéance.'],
        ],[3.3,14.3],9.5)
    else:table(oldtables[ti][0],oldtables[ti][1:],[3.3,14.3],9.5)
    short(innovation1,innovation1.split('.')[0]+'.')
    short(innovation2,innovation2.split('.')[0]+'.')
    short(rules,'Règles.');short(test,'Recette.')
    if i==5:
        h('2.1.7 Échanges entre les modules',3)
        table(['Échange','Règle partagée'],[
            ['Services ↔ Employés','Affecter un employé actif, compétent et disponible ; respecter ses absences.'],
            ['Services ↔ Locaux','Vérifier capacité, réservation et temps de remise en état avant confirmation.'],
            ['Services ↔ Stock','Relier chaque consommation au service ; ne pas la déduire une seconde fois à la clôture.'],
            ['Services ↔ DM','Lier la prestation à l’animal. Une prestation clinique peut référencer une consultation ; aucun acte médical automatique.'],
            ['Locaux ↔ Employés','Identifier l’agent ayant effectué ou validé le nettoyage et la maintenance.'],
            ['Adoptions ↔ DM / Employés','Réutiliser l’identité de l’animal, une synthèse autorisée et le référent ; limiter l’accès clinique. Une adoption validée change l’état de placement.'],
        ],[4.2,13.4],9.5)
        p('Parcours de M. Ben Salem. L’accueil retrouve l’animal et crée une prestation. Le responsable confirme l’employé et le local compatibles. L’équipe enregistre le service et ses consommations, puis clôture la prestation et valide le nettoyage. Le vétérinaire complète séparément les éléments cliniques. Le référent suit les candidatures à l’adoption puis les contacts après placement. Le tableau de bord actualise l’activité ; le journal conserve les actions.',boldlead='Parcours de M. Ben Salem.',after=3)
        p('Chaque métier innovant reste porté par son module : créneaux et estimation (Services), compétences et charge (Employés), péremption et achats (Stock), nettoyage et maintenance (Locaux), rappels et poids (DM), compatibilité et suivi après placement (Adoptions).',size=9.5,after=0)

# Page 8: quality, solution, justification and measurable validation.
page();h('2.2 Exigences non fonctionnelles',2)
p('Ces exigences visent une utilisation simple et fiable pour Les Compagnons. Les performances indiquées sont des objectifs de recette, pas des résultats déjà mesurés.')
nf=[
 ['NF-01\nErgonomie','Navigation commune, champs libellés et erreurs près du champ : limiter les manipulations et les erreurs de saisie.','Créer un service en 5 actions au plus après ouverture du formulaire, hors saisie et choix des valeurs.'],
 ['NF-02\nAccessibilité','Parcours clavier, focus visible, contraste et statuts textuels : ne pas dépendre de la couleur seule.','Utiliser les fonctions essentielles au clavier ; vérifier l’affichage à 100 % et 125 %.'],
 ['NF-03\nPerformance','Index sur identifiants et dates, filtres et pagination : limiter le volume traité et affiché.','Windows 64 bits, 8 Go RAM, SSD : 95 % des recherches < 2 s sur 10 000 services ; export de 1 000 lignes < 10 s.'],
 ['NF-04\nAuthentification','Comptes nominatifs, mots de passe hachés avec sel via une bibliothèque adaptée, temporisation et verrouillage.','Aucun mot de passe en clair ; temporisation après 5 échecs ; verrouillage après 15 min d’inactivité.'],
 ['NF-05\nAutorisations','Vérifier le rôle dans les traitements et exports, en plus de l’interface : réserver le contenu clinique aux profils habilités.','Un compte d’accueil ne peut ni ouvrir ni exporter le contenu clinique, même par appel direct.'],
 ['NF-06\nIntégrité','Requêtes préparées, types contrôlés, clés étrangères et transactions : une opération réussit entièrement ou est annulée.','En cas d’identifiant absent, de stock insuffisant ou d’erreur d’écriture, aucune modification partielle.'],
 ['NF-07\nSauvegarde','Copie cohérente de la base et des pièces à la fermeture ; sauvegarde manuelle protégée ; garder les 7 dernières copies valides.','Restaurer en environnement de test en moins de 10 min, avec les mêmes fiches et totaux.'],
 ['NF-08\nConfidentialité','Restreindre fichiers et exports aux comptes Windows autorisés ; exclure le texte clinique des journaux techniques.','Contrôler les droits du dossier de stockage et l’absence de mot de passe ou texte clinique dans les journaux.'],
 ['NF-09\nTraçabilité','Journal horodaté avec auteur, action et objet ; conserver les corrections des observations pour expliquer leur évolution.','Retrouver la version précédente d’une observation validée et son addendum.'],
 ['NF-10\nMaintenabilité','Séparer interface Qt, services métier et accès aux données ; modèles partagés et tests des règles critiques.','Tester stock et affectations sans ouvrir l’interface ; vérifier les conventions des six modules.'],
 ['NF-11\nDéploiement','Exécutable Windows avec dépendances Qt et dossier de données séparé : usage local autonome.','Sur un poste de test, créer une fiche hors connexion, fermer et rouvrir avec les données conservées.'],
]
table(['Qualité','Solution et justification','Vérification attendue'],nf,[2.9,7.25,7.45],9.5)
h('Recette du projet',4)
p('Utiliser une base fictive distincte et consigner, pour chaque exigence, les données, le résultat attendu et le résultat obtenu. Prévoir un cas nominal et un cas de refus ou de limite pour chaque CRUD et métier innovant, y compris une adoption incomplète ou déjà validée. Couvrir conflits d’affectation, absences, stock insuffisant, lots expirés, capacité, archivage et accès refusés.')
p('La démonstration est recevable lorsque les fonctions de base, les douze métiers innovants, les exports et statistiques sont vérifiés, sans anomalie bloquante sur la cohérence des données ou les droits d’accès.',after=0)

# Page 9: ODD and the compact visual identity section.
page();h('2.3 Liste des ODDs touchés',2)
p('Les contributions ci-dessous sont envisagées pour le centre fictif ; elles ne prouvent pas un impact déjà obtenu.',size=10,after=3)
table(['ODD','Lien avec ANIWORLD','Indicateur à suivre'],[
    ['12 · Consommation et production durables','Suivi des lots, péremption et achats proposés pour limiter les produits jetés.','Quantité et valeur des produits périmés ; nombre de ruptures.'],
    ['8 · Travail décent et croissance économique','Visibilité des compétences, absences et charges ; traçabilité du nettoyage.','Conflits d’affectation et écarts de charge entre profils comparables.'],
    ['9 · Industrie, innovation et infrastructure','Centralisation numérique et automatisation des contrôles du centre.','Part des prestations suivies ; nombre de ressaisies évitées.'],
],[4.4,7.0,6.2],9.2)
p('Comparer une période initiale et une période d’utilisation de même activité. Aucun objectif chiffré n’est fixé avant cette mesure. Référence : objectifs 12, 8 et 9 des Nations Unies ; les rapprochements sont l’analyse du projet.',size=9,after=3)
links=p('',size=9,after=4)
for i,(label,url) in enumerate([('ODD 12','https://www.un.org/sustainabledevelopment/fr/sustainable-consumption-production/'),('ODD 8','https://www.un.org/sustainabledevelopment/fr/economic-growth/'),('ODD 9','https://www.un.org/sustainabledevelopment/fr/infrastructure/')]):
    if i:links.add_run('  ·  ')
    hy=OxmlElement('w:hyperlink');hy.set(qn('r:id'),d.part.relate_to(url,'http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink',is_external=True))
    rr=OxmlElement('w:r');rp=OxmlElement('w:rPr');color=OxmlElement('w:color');color.set(qn('w:val'),'225A64');rp.append(color);sz=OxmlElement('w:sz');sz.set(qn('w:val'),'18');rp.append(sz);rr.append(rp);tx=OxmlElement('w:t');tx.text=label;rr.append(tx);hy.append(rr);links._p.append(hy)
h('3 Conception graphique',1);h('3.1 Le logo de l’application',2)
lp=p('',after=2);lp.alignment=WD_ALIGN_PARAGRAPH.CENTER
for name,width in [('Plan de travail 1.png',1.9)]:
    path=Path(r'C:\Users\malek\Downloads')/name
    shape=lp.add_run().add_picture(str(path));crop_shape(shape,path,width);lp.add_run('    ')
p('ANIWORLD utilise le logo chien–chat entouré de mains, de feuilles et d’une croix orange, accompagné du nom ANIWORLD. Le logo bleu et blanc fourni représente le groupe DevCore sur la couverture. Conserver les proportions et les couleurs de chaque identité, avec un espace libre autour.',size=9.5,after=4)
h('3.2 La charte graphique',2)
palette=[['Bleu pétrole','#397F8D','Repères de marque et navigation.'],['Bleu profond','#225A64','Titres et boutons principaux.'],['Anthracite','#263B42','Texte courant et informations essentielles.'],['Gris très clair','#F4F7F8','Fond général.'],['Blanc','#FFFFFF','Tableaux, formulaires et cartes.'],['Bleu pâle','#E4F0F3','Sélection et surfaces de synthèse.'],['Orange ambré','#E8A13F','Croix du logo et encarts de suivi.'],['Rouge brique','#AE5751','Erreurs et actions nécessitant une attention.']]
t=table(['Couleur','Code','Usage'],palette,[4.4,3.0,10.2],9.2)
for row,values in zip(t.rows[1:],palette):
    pr=row.cells[1]._tc.get_or_add_tcPr()
    for sh in list(pr.findall(qn('w:shd'))):pr.remove(sh)
    sh=OxmlElement('w:shd');sh.set(qn('w:fill'),values[1][1:]);pr.append(sh)
    for r in row.cells[1].paragraphs[0].runs:r.font.color.rgb=RGBColor.from_string('FFFFFF' if values[1] in ['#397F8D','#225A64','#263B42','#AE5751'] else '263B42')
p('Typographie : Segoe UI ; titres 28 px, zones 15–17 px, texte 13–14 px, mentions 11–12 px. Navigation commune aux six modules, libellés de statut explicites et focus clavier visible. Les formulaires conservent les saisies en cas d’erreur ; toute suppression exige une confirmation.',boldlead='Typographie :',size=9.5,after=0)

# Pages 10-12: two full-width Qt captures per page and concise explanations.
gui=[
 ('3.3.1 Tableau de bord','Les compteurs, les services du jour et les priorités donnent à M. Ben Salem une vue rapide du centre. Nouveau service ouvre la création ; les données visibles dépendent du rôle.'),
 ('3.3.2 Gestion des Services','La vue en cartes présente les prestations, dates, durées et statuts. Les filtres ouvrent la fiche sélectionnée avec son responsable, son tarif et les actions de consultation, modification et démarrage.'),
 ('3.3.3 Gestion des Employés','Les cartes présentent les employés, leurs disponibilités, compétences et heures planifiées. Le panneau détaille la fiche choisie ; recherche, filtres, modification et statistiques facilitent la répartition du travail.'),
 ('3.3.4 Gestion du Stock','L’inventaire affiche lots, quantités, seuils et échéances. Les panneaux expliquent la sortie FEFO et le réapprovisionnement ; chaque mouvement est validé et tracé.'),
 ('3.3.5 Gestion des Locaux','Les cartes présentent les types, capacités et états des locaux. Le panneau du local sélectionné regroupe affectation, libération, modification et suppression ; la vue d’occupation complète la recherche et les filtres.'),
 ('3.3.6 Gestion des dossiers médicaux','Le vétérinaire retrouve l’animal, les consultations, les pesées et les échéances. Les rappels restent à valider ; le signal de poids demande une vérification clinique.'),
 ('3.3.7 Gestion des Adoptions','Les cartes relient un animal à sa candidature. Le panneau affiche les critères de compatibilité, les étapes de validation et le suivi à J+7 / J+30 ; le responsable confirme le placement.'),
]
gui_paths=[
    WORK/'gui_aniworld_v4/gui_0.png',
    Path(r'C:\Users\malek\Downloads\ChatGPT Image Oct 1, 2026, 11_16_40 AM.png'),
    Path(r'C:\Users\malek\Downloads\WhatsApp Image 2026-09-30 at 9.04.54 PM.jpeg'),
    Path(r'C:\Users\malek\Downloads\5604730f-5391-4456-8296-8146c9a70582.jpg'),
    Path(r'C:\Users\malek\Downloads\WhatsApp Image 2026-09-30 at 9.11.47 PM.jpeg'),
    WORK/'gui_aniworld_v4/gui_5.png',
    WORK/'gui_aniworld_v4/gui_6.png',
]
for i,(title,explanation) in enumerate(gui):
    if i%2==0:page()
    if i==0:
        h('3.3 Les interfaces graphiques GUI',2)
        p('Écrans avec données de démonstration. Services, Employés, Stock et Locaux reprennent les images fournies ; Tableau de bord, Dossiers médicaux et Adoptions sont des maquettes Qt.',size=9,after=2)
    hh=h(title,3);hh.paragraph_format.space_before=Pt(4);hh.paragraph_format.space_after=Pt(2)
    path=gui_paths[i]
    ip=p('',after=1);ip.alignment=WD_ALIGN_PARAGRAPH.CENTER;ip.paragraph_format.line_spacing=1
    ip.paragraph_format.keep_with_next=True
    shape=ip.add_run().add_picture(str(path),width=Cm(16.0))
    if i==4:crop_to(shape,path,(0,24,1600,950),16.0)
    if i==3:crop_to(shape,path,(0,20,1600,857),16.0)
    if i==2:crop_to(shape,path,(0,0,1600,835),16.0)
    shape._inline.docPr.set('descr',title+' — Maquette Qt ANIWORLD, données fictives')
    pp=p(explanation,size=9.5,after=3)

h('Parcours illustré de l’adoption',4)
table(['Étape','Action et résultat attendu'],[
    ['1 · Demande','L’accueil enregistre le candidat et l’animal ; le dossier passe à En étude.'],
    ['2 · Vérification','Le référent vérifie les pièces, la rencontre et les critères de compatibilité.'],
    ['3 · Décision','Le responsable valide ou refuse avec motif ; une adoption validée conserve son historique.'],
    ['4 · Suivi','La date de placement déclenche les contacts à J+7 et J+30 ; chaque réponse est tracée.'],
],[3.5,14.1],9.5)
p('Cette maquette illustre un dossier encore en étude : la validation est inactive tant que la rencontre et l’accord du responsable ne sont pas enregistrés.',size=9.5,after=0)
d.save(DEST)
print(DEST)
print('Rebuilt compact specification; original group and application assets preserved.')
