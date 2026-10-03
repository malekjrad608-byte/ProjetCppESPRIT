from pathlib import Path
from docx import Document
from docx.shared import Cm, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_CELL_VERTICAL_ALIGNMENT
from docx.oxml import OxmlElement
from docx.oxml.ns import qn

ROOT = Path(r'C:\Users\malek\Documents\gestion_service')
WORK = ROOT / 'tmp' / 'cahier_specification'
OUT = ROOT / 'output' / 'documents'
OUT.mkdir(parents=True, exist_ok=True)
LOGO = Path(r'C:\Users\malek\Downloads\logo-removebg-preview.png')
doc = Document()
sec = doc.sections[0]
sec.page_width, sec.page_height = Cm(21), Cm(29.7)
sec.top_margin, sec.bottom_margin = Cm(1.9), Cm(1.8)
sec.left_margin, sec.right_margin = Cm(2), Cm(2)
sec.header_distance, sec.footer_distance = Cm(0.8), Cm(0.8)
sec.different_first_page_header_footer = True
GREEN='246756'; INK='333333'; MUTED='68766F'; SAGE='9FC8AA'
styles=doc.styles
for name in ['Normal','Body Text','Title','Subtitle','Heading 1','Heading 2','Heading 3','Heading 4','Caption']:
    s=styles[name]; s.font.name='Calibri'; s.font.color.rgb=RGBColor.from_string(INK)
    s.paragraph_format.widow_control=True
styles['Normal'].font.size=Pt(11)
styles['Normal'].paragraph_format.line_spacing=1.10
styles['Normal'].paragraph_format.space_after=Pt(7)
for name,size in [('Heading 1',22),('Heading 2',16),('Heading 3',15),('Heading 4',11.5)]:
    s=styles[name]; s.font.size=Pt(size);s.font.bold=True;s.font.color.rgb=RGBColor.from_string(GREEN)
    s.paragraph_format.space_before=Pt(12);s.paragraph_format.space_after=Pt(8);s.paragraph_format.keep_with_next=True
styles['Title'].font.size=Pt(30);styles['Title'].font.bold=True;styles['Title'].font.color.rgb=RGBColor(0,0,0)
styles['Title'].paragraph_format.space_after=Pt(12)
styles['Subtitle'].font.size=Pt(16);styles['Subtitle'].font.color.rgb=RGBColor.from_string(INK)
styles['Subtitle'].font.italic=False
for style in styles:
    for border in list(style.element.iter(qn('w:pBdr'))):
        border.getparent().remove(border)
styles['Caption'].font.size=Pt(9);styles['Caption'].font.italic=False;styles['Caption'].font.color.rgb=RGBColor.from_string(MUTED)
styles['Caption'].paragraph_format.space_after=Pt(9)
for name in ['TOC 1','TOC 2','TOC 3']:
    if name not in styles: styles.add_style(name,1)
    s=styles[name];s.font.name='Calibri';s.font.size=Pt(10.5);s.font.color.rgb=RGBColor.from_string(INK);s.paragraph_format.space_after=Pt(3)
    s.paragraph_format.line_spacing=1.05
styles['TOC 1'].font.bold=True
doc.core_properties.title='Cahier de spécification Pattia'
doc.core_properties.subject='Application C++ et Qt de gestion d’un centre de soins animaliers'
doc.core_properties.author='Équipe projet Pattia'
doc.core_properties.keywords='Pattia, Qt, C++, services, employés, stock, locaux, dossiers médicaux'
doc.core_properties.comments=''
lang=OxmlElement('w:lang');lang.set(qn('w:val'),'fr-FR');styles['Normal'].element.get_or_add_rPr().append(lang)

def field(p,code):
    r=p.add_run();f=OxmlElement('w:fldSimple');f.set(qn('w:instr'),code);rr=OxmlElement('w:r');tt=OxmlElement('w:t');tt.text='1';rr.append(tt);f.append(rr);r._r.addnext(f)

hp=sec.header.paragraphs[0];hp.text='PATTIA   /   CAHIER DE SPÉCIFICATION';hp.style='Caption';hp.paragraph_format.space_after=Pt(0)
fp=sec.footer.paragraphs[0];fp.paragraph_format.space_after=Pt(0)
r=fp.add_run('Projet C++ 2A   •   Version 1.0');r.font.size=Pt(9);r.font.color.rgb=RGBColor.from_string(MUTED)
fp.paragraph_format.tab_stops.add_tab_stop(Cm(13.8))
fp.add_run('\t');field(fp,'PAGE');fp.add_run(' / ');field(fp,'NUMPAGES')

def p(text='',style=None,boldlead=None):
    x=doc.add_paragraph(style=style)
    if boldlead and text.startswith(boldlead):x.add_run(boldlead).bold=True;x.add_run(text[len(boldlead):])
    else:x.add_run(text)
    return x
def h(text,level=2):return doc.add_heading(text,level)
def new():doc.add_page_break()
def table(headers,rows,widths=None,font=10.2):
    t=doc.add_table(rows=1,cols=len(headers));t.alignment=WD_TABLE_ALIGNMENT.CENTER;t.autofit=False
    if widths:
        for c,w in zip(t.columns,widths):c.width=Cm(w)
    pr=t._tbl.tblPr
    borders=OxmlElement('w:tblBorders')
    for edge in ['top','left','bottom','right','insideH','insideV']:
        b=OxmlElement('w:'+edge);b.set(qn('w:val'),'single');b.set(qn('w:sz'),'4');b.set(qn('w:color'),'D9D9D9');borders.append(b)
    pr.append(borders)
    mar=OxmlElement('w:tblCellMar')
    for edge in ['top','bottom','left','right']:
        m=OxmlElement('w:'+edge);m.set(qn('w:w'),'90' if edge in ('top','bottom') else '105');m.set(qn('w:type'),'dxa');mar.append(m)
    pr.append(mar)
    for i,v in enumerate(headers):t.rows[0].cells[i].text=v
    for row in rows:
        cells=t.add_row().cells
        for i,v in enumerate(row):cells[i].text=str(v)
    for ri,row in enumerate(t.rows):
        trpr=row._tr.get_or_add_trPr();nosplit=OxmlElement('w:cantSplit');trpr.append(nosplit)
        if ri==0:
            rep=OxmlElement('w:tblHeader');trpr.append(rep)
        for ci,c in enumerate(row.cells):
            if widths:c.width=Cm(widths[ci])
            c.vertical_alignment=WD_CELL_VERTICAL_ALIGNMENT.CENTER
            sh=OxmlElement('w:shd');sh.set(qn('w:fill'),'3B3B3B' if ri==0 else ('F4F7F4' if ri%2 else 'FFFFFF'));c._tc.get_or_add_tcPr().append(sh)
            for cp in c.paragraphs:
                cp.paragraph_format.space_after=Pt(0);cp.paragraph_format.space_before=Pt(0);cp.paragraph_format.line_spacing=1.05
                for r in cp.runs:r.font.size=Pt(font);r.font.bold=(ri==0);r.font.color.rgb=RGBColor.from_string('FFFFFF' if ri==0 else INK)
    p('').paragraph_format.space_after=Pt(0)
    return t
def pic(path,width=17,caption=None):
    x=p();x.alignment=WD_ALIGN_PARAGRAPH.CENTER;x.paragraph_format.keep_with_next=True;x.add_run().add_picture(str(path),width=Cm(width))
    if caption:p(caption,'Caption')
def link(text,url):
    x=p();hyp=OxmlElement('w:hyperlink');rid=doc.part.relate_to(url,'http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink',is_external=True);hyp.set(qn('r:id'),rid)
    r=OxmlElement('w:r');rp=OxmlElement('w:rPr');col=OxmlElement('w:color');col.set(qn('w:val'),GREEN);rp.append(col);r.append(rp);tx=OxmlElement('w:t');tx.text=text;r.append(tx);hyp.append(r);x._p.append(hyp)
    x.paragraph_format.space_after=Pt(4)

# Couverture
p('PROJET C++ 2A', 'Subtitle').alignment=WD_ALIGN_PARAGRAPH.CENTER
p('Année universitaire 2025/2026', 'Caption').alignment=WD_ALIGN_PARAGRAPH.CENTER
p('').paragraph_format.space_after=Pt(18)
pic(LOGO,4.1)
x=p('PATTIA');x.alignment=WD_ALIGN_PARAGRAPH.CENTER;x.runs[0].font.size=Pt(34);x.runs[0].font.bold=True;x.runs[0].font.color.rgb=RGBColor.from_string(GREEN)
x=p('Cahier de spécification','Title');x.alignment=WD_ALIGN_PARAGRAPH.CENTER
x=p('Application de gestion\nd’un centre de soins animaliers','Subtitle');x.alignment=WD_ALIGN_PARAGRAPH.CENTER
x=p('Services  •  Employés  •  Stock\nLocaux  •  Dossiers médicaux');x.alignment=WD_ALIGN_PARAGRAPH.CENTER;x.paragraph_format.space_after=Pt(25)
table(['Identification du projet','Informations'],[
 ('Établissement','À compléter'),('Classe et groupe','À compléter'),('Réalisé par','À compléter'),('Encadré par','À compléter'),('Version et date','Version 1.0  |  26 septembre 2026')],[5,12],11)
p('Document de référence pour la conception, le développement et la validation du projet.', 'Caption').alignment=WD_ALIGN_PARAGRAPH.CENTER

# Sommaire
new();p('Sommaire','Title')
p('Pattia regroupe les activités et les ressources d’un centre de soins animaliers. Ce cahier précise le périmètre fonctionnel, les règles de gestion, les qualités attendues et les interfaces de l’application C++ et Qt.')
toc=p();field(toc,'TOC \\o "1-3" \\h \\z \\u')
p('Les identifiants SER, EMP, STK, LOC et DM permettent de relier les exigences aux cinq modules. Les critères de recette décrivent les résultats attendus lors de la démonstration.','Caption')

# Page contexte
new();h('1 Contexte général',1)
p('Le projet consiste à concevoir une application de bureau pour un centre qui accueille des animaux de compagnie et propose des consultations, du toilettage, des soins et des prestations de bien-être. L’équipe doit coordonner les services, les salariés, les produits, les espaces et les dossiers médicaux dans un même environnement.')
h('1.1 Étude de l’existant',2)
p('Le client cible est un centre de soins animaliers de petite ou moyenne taille. Il comprend un responsable, un agent d’accueil, des vétérinaires, des soigneurs, un responsable des stocks et un référent des locaux. Certains salariés peuvent cumuler plusieurs rôles selon les droits accordés.')
p('Le scénario de référence retenu pour le projet pédagogique est un fonctionnement réparti entre un agenda, des tableaux de suivi et des dossiers papier ou numériques séparés. Cet état de départ constitue une hypothèse de travail à valider avec le centre cible.')
table(['Domaine','Fonctionnement de référence','Limite rencontrée'],[
 ('Services','Agenda et fiches de prestations.','Conflits de créneaux et statuts peu visibles.'),
 ('Employés','Coordonnées et disponibilités séparées.','Affectation sans vision de la charge.'),
 ('Stock','Comptage et sorties saisis manuellement.','Ruptures et dates de péremption oubliées.'),
 ('Locaux','Disponibilité transmise oralement.','Réservations et nettoyage mal coordonnés.'),
 ('Dossiers médicaux','Historique réparti dans plusieurs supports.','Recherche lente et suivi difficile.')],[2.8,6.6,7.6])
h('1.2 Problématique',2)
p('Comment centraliser ces informations pour planifier un service avec les bonnes ressources, fiabiliser le suivi des animaux et limiter les ressaisies, tout en respectant les responsabilités de chaque utilisateur ?')
p('La solution doit permettre de retrouver rapidement une information, d’empêcher les opérations incohérentes et de conserver un historique exploitable. Le succès sera vérifié par des scénarios concrets : réserver sans conflit, enregistrer une consommation sans stock négatif et consulter un dossier selon les droits accordés.')

# Solution
new();h('2 La solution proposée et ses exigences',1)
p('Pattia est une application de bureau organisée autour de cinq modules reliés à une base de données commune. Un tableau de bord présente les prestations du jour et les alertes utiles. Chaque module conserve la même logique de navigation : liste, recherche, filtres, fiche de détail et actions autorisées.')
h('2.1 Exigences fonctionnelles',2)
p('La première version cible un poste Windows utilisé successivement par des comptes nominatifs. Le développement est prévu en C++ avec Qt Widgets et Qt SQL. Une base SQLite locale permet une démonstration autonome ; un accès simultané depuis plusieurs postes nécessiterait une évolution de l’architecture.')
h('Périmètre et acteurs',4)
table(['Utilisateur','Responsabilités et accès prévus'],[
 ('Administrateur','Créer les comptes, attribuer les rôles, gérer les paramètres et les sauvegardes. L’accès clinique exige aussi un rôle médical explicite.'),
 ('Responsable du centre','Piloter les services, les employés et les ressources ; consulter les indicateurs de gestion.'),
 ('Accueil et soigneur','Planifier ou réaliser les services autorisés ; consulter les informations d’identification nécessaires à la prise en charge.'),
 ('Vétérinaire','Consulter et compléter les dossiers médicaux, valider les observations et organiser le suivi.'),
 ('Responsable du stock','Gérer les articles, lots et mouvements ; préparer les propositions de réapprovisionnement.'),
 ('Référent des locaux','Gérer les espaces, valider le nettoyage et suivre la maintenance.')],[4.2,12.8])
p('Les fiches Animal et Propriétaire sont des données de référence du module DM. Les rendez-vous et affectations relèvent des services. Ces éléments ne constituent pas des modules supplémentaires.')
p('La vente en ligne, la paie, la comptabilité, le diagnostic automatique et l’envoi automatique de messages aux propriétaires sont hors du périmètre de cette version. Les propositions produites par l’application restent soumises à une validation humaine.')
h('Convention de gestion',4)
p('CRUD signifie créer, consulter, modifier et supprimer. Une fiche sans historique peut être supprimée après confirmation ; une fiche déjà référencée est archivée. Les informations médicales validées et les mouvements de stock se corrigent par une écriture traçable, sans effacement de leur historique.')

# Tableau du plan
new();h('Tableau de synthèse des modules',4)
p('Le tableau ci-dessous reprend les rubriques du plan de spécification. Les fiches qui suivent détaillent les attributs, les méthodes, les deux métiers innovants et les critères de validation de chaque module.')
table(['Module','Entité X','Caractéristiques Y','Utilisateur','Fonctionnalités et méthodes'],[
 ('Gestion des Services','Service','ID, libellé, catégorie, date, durée, tarif, statut, animal, employé, local.','Accueil, responsable, soigneur.','CRUD ; tri date/tarif ; recherche ; export PDF/CSV ; graphiques par catégorie/statut.\nSER-I1 : créneaux compatibles.\nSER-I2 : estimation de prestation.'),
 ('Gestion des Employés','Employé','ID, nom, prénom, courriel, poste, compétences, disponibilités, état.','Responsable et administrateur.','CRUD ; tri nom/poste ; recherche ; export PDF/CSV ; graphiques par poste/charge.\nEMP-I1 : adéquation des compétences.\nEMP-I2 : équilibrage de charge.'),
 ('Gestion du Stock','Article','ID, référence, désignation, catégorie, unité, seuil, prix ; lots et mouvements.','Responsable du stock.','CRUD ; tri quantité/échéance ; recherche ; export PDF/CSV ; graphiques de consommation.\nSTK-I1 : sortie par péremption.\nSTK-I2 : réapprovisionnement.'),
 ('Gestion des Locaux','Local','ID, nom, type, capacité, équipements, état ; réservations et entretien.','Référent des locaux, responsable.','CRUD ; tri nom/capacité ; recherche ; export PDF/CSV ; graphiques d’occupation.\nLOC-I1 : remise en service.\nLOC-I2 : maintenance préventive.'),
 ('Gestion des DM','Dossier médical','ID, animal, propriétaire, antécédents, allergies, consultations, poids, vaccins.','Vétérinaire ; accès limité pour l’accueil.','CRUD encadré ; tri visite/animal ; recherche ; export PDF ; graphiques de suivi.\nDM-I1 : rappels vaccinaux.\nDM-I2 : alerte sur le poids.')],[2.3,2.0,4.1,3.0,5.6],10)
h('Données communes',4)
p('Chaque enregistrement possède un identifiant unique, une date de création, une date de modification et un état actif ou archivé. Les liens entre modules utilisent ces identifiants. Les dates sont affichées au format jour/mois/année, les heures sur 24 heures et les montants en dinars tunisiens avec trois décimales.')
p('Les listes affichées, les exports et les statistiques appliquent les mêmes filtres et les mêmes droits. Une période sans données affiche un message explicite et des compteurs à zéro.')

modules=[
 dict(num='2.1.1',title='Gestion des Services',code='SER',purpose='Ce module organise une prestation depuis sa planification jusqu’à sa clôture. Il relie l’animal concerné, le responsable de l’exécution et le local utilisé.',
 attrs='Service : idService, libellé, catégorie, dateHeureDébut, duréeMinutes, tarifTND, statut, notes, idAnimal, idEmployé, idLocal. Les catégories comprennent consultation, soins, toilettage, pension et éducation.',
 basic=[('SER-01  CRUD','Ajouter une prestation ; consulter sa fiche ; modifier les informations autorisées ; supprimer une fiche sans historique ou l’archiver.'),('SER-02  Trier','Trier par date et heure, libellé, durée ou tarif, dans les deux sens.'),('SER-03  Rechercher','Rechercher par libellé, animal ou responsable ; filtrer par catégorie, période et statut.'),('SER-04  Exporter','Produire un planning PDF et une liste CSV respectant les filtres actifs.'),('SER-05  Statistiques','Afficher un histogramme par catégorie et un graphique de répartition par statut sur une période donnée.')],
 innovations=[('SER-I1  Proposition de créneaux compatibles','À partir de la durée demandée et des ressources choisies, proposer jusqu’à trois créneaux sans chevauchement. Le calcul tient compte des disponibilités du salarié, des réservations du local et des temps de remise en état. La confirmation recontrôle les contraintes avant l’enregistrement.'),('SER-I2  Estimation personnalisée de prestation','Calculer une estimation à partir d’un tarif de base, d’options et d’une remise autorisée. Afficher le détail et générer une fiche d’estimation PDF. Les paramètres et le total sont conservés à la validation pour retrouver le montant présenté.')],
 rules='Un libellé, un animal, une durée strictement positive et un tarif positif ou nul sont obligatoires. Le cycle est Planifié, En cours, Terminé ; l’annulation est motivée. Un service terminé conserve ses ressources et ses consommations dans l’historique.',
 test='Pour une salle déjà réservée de 10 h à 11 h, une demande à 10 h 30 est refusée. Une base de 50 TND et une option de 10 TND avec une remise de 10 % donnent 54,000 TND.'),
 dict(num='2.1.2',title='Gestion des Employés',code='EMP',purpose='Ce module gère les personnes qui travaillent au centre et les informations nécessaires à leur affectation. Il donne au responsable une vue des compétences et de la charge prévue.',
 attrs='Employé : idEmployé, nom, prénom, courriel, téléphone, poste, dateEmbauche, étatActif. Données associées : compétences, créneaux de disponibilité, absences et compte utilisateur éventuel.',
 basic=[('EMP-01  CRUD','Créer et consulter une fiche ; modifier les coordonnées, le poste et les disponibilités ; supprimer sans référence ou archiver.'),('EMP-02  Trier','Trier par nom, poste, date d’embauche ou charge planifiée.'),('EMP-03  Rechercher','Rechercher par identifiant, nom ou courriel ; filtrer par poste, compétence et état actif.'),('EMP-04  Exporter','Exporter l’annuaire autorisé et le planning en PDF ou CSV, sans données d’authentification.'),('EMP-05  Statistiques','Présenter les effectifs par poste et un histogramme des heures planifiées par employé.')],
 innovations=[('EMP-I1  Correspondance entre compétences et besoin','Comparer les compétences requises par une prestation aux habilitations des employés actifs. Proposer les profils qui remplissent toutes les conditions et expliquer les critères retenus. L’adéquation des compétences ne vaut pas réservation d’un créneau.'),('EMP-I2  Aide à l’équilibrage de la charge','Calculer, par semaine, le rapport entre les minutes planifiées et les minutes disponibles hors absences. Signaler un dépassement du seuil paramétré et proposer une réaffectation à un profil habilité moins chargé. Le responsable valide la modification.')],
 rules='Le courriel est contrôlé et unique lorsqu’il est renseigné. Une absence bloque les nouvelles affectations sur sa période. Une disponibilité nulle exclut le salarié du calcul de charge. L’archivage exige le traitement de ses services futurs.',
 test='Un salarié sans la compétence requise n’est pas proposé. Pour 24 heures planifiées sur 30 disponibles, le taux vaut 80 %. Une absence de 14 h à 17 h interdit une nouvelle affectation à 15 h.'),
 dict(num='2.1.3',title='Gestion du Stock',code='STK',purpose='Ce module trace les produits utilisés par le centre, leurs lots et leurs mouvements. Il permet d’anticiper les besoins et d’éviter l’utilisation de produits bloqués ou expirés.',
 attrs='Article : idArticle, référence, désignation, catégorie, unité, seuilAlerte, prixUnitaireTND, fournisseur. Lot : idLot, numéro, quantité, datePéremption, état. Mouvement : type, quantité, date, auteur, motif et service associé.',
 basic=[('STK-01  CRUD','Créer et consulter un article ; modifier sa fiche ; supprimer un article sans mouvement ou l’archiver. Enregistrer les entrées et sorties.'),('STK-02  Trier','Trier par désignation, quantité disponible, seuil ou date de péremption des lots.'),('STK-03  Rechercher','Rechercher par référence, désignation ou lot ; filtrer par catégorie et état d’alerte.'),('STK-04  Exporter','Exporter un inventaire PDF et les mouvements filtrés en CSV.'),('STK-05  Statistiques','Tracer les consommations par article et par mois ; comparer la valeur du stock par catégorie.')],
 innovations=[('STK-I1  Proposition de sortie selon la péremption','Appliquer la règle FEFO : proposer le lot utilisable dont la date de péremption est la plus proche. Exclure les lots expirés, bloqués et vides. Afficher le lot, la quantité disponible et la date avant la validation de la sortie.'),('STK-I2  Proposition de réapprovisionnement','Estimer la consommation quotidienne moyenne des 30 derniers jours. Proposer une quantité couvrant le délai fournisseur et la sécurité configurée, après déduction du stock utilisable. Sans historique, utiliser le seuil manuel. Aucune commande n’est envoyée automatiquement.')],
 rules='Les quantités sont strictement positives pour un mouvement ; le stock ne peut pas devenir négatif. Une sortie est atomique et reliée à son auteur. Une correction crée un mouvement inverse justifié. Le stock disponible est calculé à partir des mouvements par lot.',
 test='Avec des lots valides expirant le 31/10 et le 31/12, le premier est proposé. Une sortie de 6 unités sur un disponible de 5 est refusée. À 2 unités par jour, 7 jours de délai, 6 unités de sécurité et 10 disponibles, proposer 10 unités.'),
 dict(num='2.1.4',title='Gestion des Locaux',code='LOC',purpose='Ce module décrit les espaces du centre et leur état d’utilisation. Il permet de réserver une capacité adaptée et de maintenir les salles disponibles dans des conditions maîtrisées.',
 attrs='Local : idLocal, nom, type, capacitéAnimaux, surfaceM2, équipements, état, actif. Données associées : réservations, capacités engagées, fiches de nettoyage et interventions de maintenance.',
 basic=[('LOC-01  CRUD','Créer, consulter et modifier un local ; supprimer sans historique ou archiver après traitement des réservations futures.'),('LOC-02  Trier','Trier par nom, type, capacité ou taux d’occupation.'),('LOC-03  Rechercher','Rechercher par nom ou identifiant ; filtrer par type, équipement, état et disponibilité sur une période.'),('LOC-04  Exporter','Exporter les fiches des locaux et le planning d’occupation en PDF ou CSV.'),('LOC-05  Statistiques','Afficher les heures occupées par local et la répartition des indisponibilités par motif.')],
 innovations=[('LOC-I1  Circuit de remise en service après nettoyage','À la fin d’une prestation nécessitant un nettoyage, passer le local à l’état À nettoyer. Bloquer la prise en charge suivante jusqu’à la validation d’une liste de contrôle par un agent habilité. Conserver l’auteur et l’heure de remise en service.'),('LOC-I2  Maintenance préventive selon l’usage','Cumuler les heures d’utilisation ou le nombre de passages d’un local. Créer une alerte lorsque le seuil d’entretien configuré est atteint. Planifier une intervention avec une période d’indisponibilité et réinitialiser le compteur de cycle après validation.')],
 rules='La capacité est un entier strictement positif. Les capacités simultanément engagées ne dépassent pas la capacité du local. Un local en maintenance ou à nettoyer ne peut pas accueillir un service. Les temps de nettoyage sont inclus dans le contrôle des disponibilités.',
 test='Pour un espace de capacité 2 déjà engagé à 2, une nouvelle réservation simultanée est refusée. La remise en service est impossible sans la liste de nettoyage validée. Un seuil de 100 heures déclenche une alerte à 100 heures cumulées.'),
 dict(num='2.1.5',title='Gestion des dossiers médicaux',code='DM',purpose='Le module DM rassemble l’identité de l’animal et son historique de prise en charge. Le vétérinaire y conserve les informations médicales nécessaires au suivi, avec une traçabilité des modifications.',
 attrs='Dossier : idDossier, idAnimal, dateOuverture, antécédents, allergies, état. Animal : nom, espèce, race, sexe, dateNaissance, identifiant éventuel, idPropriétaire. Consultation : date, vétérinaire, observation, traitement, poidsKg. Vaccination : produit, date, prochaineÉchéance.',
 basic=[('DM-01  CRUD','Ouvrir et consulter un dossier ; compléter les données ; supprimer un brouillon sans acte validé ou archiver. Une observation validée se corrige par addendum daté.'),('DM-02  Trier','Trier par animal, date d’ouverture, dernière consultation ou prochaine échéance.'),('DM-03  Rechercher','Rechercher par numéro, animal ou propriétaire ; filtrer par espèce, vétérinaire et période.'),('DM-04  Exporter','Générer une synthèse médicale PDF pour un utilisateur autorisé, avec auteur, date et périmètre de l’export.'),('DM-05  Statistiques','Afficher les consultations par période et espèce ainsi que la répartition des dossiers actifs.')],
 innovations=[('DM-I1  Rappels vaccinaux préparés depuis le dossier','Repérer les échéances saisies par le vétérinaire arrivant dans les 7 jours et les retards. Préparer une liste de suivi et un texte de rappel à valider. La date médicale est définie par le vétérinaire ; aucun calendrier vaccinal n’est déduit automatiquement.'),('DM-I2  Alerte sur une variation de poids','Comparer les deux dernières pesées valides d’un animal et calculer la variation relative. Déclencher un signal de vérification si la valeur absolue dépasse le seuil configuré par le vétérinaire. Afficher les mesures et leurs dates, sans conclure à un diagnostic.')],
 rules='Un animal dispose d’un dossier actif unique. Le poids doit être strictement positif. Les observations validées gardent leur auteur et leur date. L’accueil accède aux données d’identification ; le contenu clinique exige une habilitation médicale.',
 test='Un compte d’accueil ne peut ni ouvrir ni exporter le contenu clinique. Avec un seuil de 10 %, deux pesées de 10 kg puis 8,5 kg produisent une variation de −15 % et un signal de vérification. Une seule pesée ne déclenche pas d’alerte.')
]

for m in modules:
    new();h(m['num']+' '+m['title'],3);p(m['purpose']);p(m['attrs'])
    table(['Exigence','Fonctionnalité attendue'],m['basic'],[3.6,13.4],10.2)
    h('Deux métiers innovants',4)
    for title,body in m['innovations']:p(title+'. '+body,boldlead=title+'. ')
    h('Règles de gestion et recette',4)
    p(m['rules']);p('Vérification attendue. '+m['test'],boldlead='Vérification attendue. ')

# Intégration
new();h('2.1.6 Échanges entre les modules',3)
p('Les modules partagent leurs identifiants et leurs règles de validation. Une prestation ne peut être confirmée que si l’animal, l’employé et le local référencés sont actifs et compatibles avec les besoins saisis.')
table(['Relation','Règle de cohérence'],[
 ('Service et employé','Un employé peut réaliser plusieurs services à des horaires distincts. Les affectations respectent les compétences et les absences.'),
 ('Service et local','Chaque service utilise un local principal ; plusieurs services simultanés restent possibles seulement dans la capacité configurée.'),
 ('Service et stock','Une consommation peut comprendre plusieurs lignes et plusieurs lots. Chaque sortie conserve le lien avec le service ; la clôture ne doit pas la comptabiliser une seconde fois.'),
 ('Service et dossier médical','Une prestation concerne un animal. Une prestation clinique peut être liée à une consultation du dossier ; un toilettage ne crée pas automatiquement d’observation médicale.'),
 ('Local et employé','Une fiche de nettoyage ou de maintenance désigne l’employé ayant réalisé ou validé l’intervention.')],[4.1,12.9])
h('Parcours de référence',4)
p('1. L’accueil recherche l’animal et crée une prestation planifiée. Le système contrôle les données obligatoires et les ressources demandées.')
p('2. Le responsable valide un employé habilité et un local compatible. Le contrôle de chevauchement est renouvelé à l’enregistrement.')
p('3. Le soigneur démarre la prestation et enregistre les produits effectivement consommés. Toute sortie refusée laisse le stock et la prestation dans leur état précédent.')
p('4. À la clôture, le service passe à Terminé. Le local devient À nettoyer si son protocole l’exige. Le vétérinaire valide séparément les informations cliniques éventuelles.')
p('5. Le tableau de bord actualise ses indicateurs. Les opérations significatives sont inscrites dans le journal avec l’utilisateur, la date et l’objet concerné.')
h('Responsabilité des fonctionnalités innovantes',4)
p('La recherche de créneaux appartient aux Services ; l’adéquation des compétences et l’équilibrage aux Employés ; la péremption et le réapprovisionnement au Stock ; le nettoyage et la maintenance aux Locaux ; les rappels vaccinaux et le suivi du poids aux DM. Les modules échangent des données sans dupliquer ces traitements.')

# Nonfonctionnel
new();h('2.2 Exigences non fonctionnelles',2)
p('Les exigences ci-dessous sont des objectifs de qualité à vérifier lors de la recette. Chaque objectif est associé à une solution technique et à un résultat observable. Les temps indiqués sont des cibles, et non des performances déjà mesurées.')
table(['ID et qualité','Solution retenue et justification','Critère de validation'],[
 ('NF-01\nErgonomie','Navigation stable, formulaires libellés, recherche accessible et messages près du champ incorrect. Une structure commune réduit les erreurs de saisie.','Création d’un service en 5 actions au maximum après ouverture du formulaire, hors saisie et choix des valeurs.'),
 ('NF-02\nAccessibilité','Parcours clavier, focus visible, libellés textuels des statuts et contraste lisible. La couleur complète le texte.','Fonctions essentielles utilisables au clavier ; vérification à 100 % et 125 % de mise à l’échelle.'),
 ('NF-03\nPerformance','Requêtes filtrées, index sur identifiants et dates, listes paginées ou chargées à la demande pour limiter le volume affiché.','Sur Windows 64 bits, 8 Go de RAM et SSD : 95 % des recherches en moins de 2 s sur 10 000 services ; export de 1 000 lignes en moins de 10 s.'),
 ('NF-04\nAuthentification','Comptes nominatifs, mots de passe hachés avec sel au moyen d’une bibliothèque adaptée, temporisation après échecs et verrouillage après inactivité.','Aucun mot de passe en clair ; temporisation après 5 échecs ; verrouillage après 15 min d’inactivité.'),
 ('NF-05\nAutorisations','Contrôle du rôle dans les traitements et lors des exports, en complément des boutons masqués. Les dossiers cliniques sont réservés aux profils médicaux.','Les tentatives d’accès clinique par un compte d’accueil échouent, y compris depuis une action appelée directement.'),
 ('NF-06\nIntégrité','Requêtes préparées, contrôles de types, clés étrangères et transactions. Une opération multiétape doit réussir entièrement ou être annulée.','Un identifiant absent, un stock insuffisant ou une erreur d’écriture ne laisse aucune donnée partiellement enregistrée.')],[2.8,7.9,6.3],10.3)

new();h('2.2.1 Fiabilité et validation technique',3)
table(['ID et qualité','Solution retenue et justification','Critère de validation'],[
 ('NF-07\nSauvegarde','Copie cohérente de la base et des pièces associées à la fermeture, plus sauvegarde manuelle vers un dossier protégé. Conserver les 7 dernières copies validées.','Restaurer une copie sur un environnement de test en moins de 10 min et retrouver les mêmes fiches et totaux.'),
 ('NF-08\nConfidentialité','Restreindre les fichiers aux comptes Windows autorisés. Ne pas écrire le contenu clinique dans les journaux techniques. Protéger les exports comme les données sources.','Les journaux contiennent les identifiants et actions utiles, sans mot de passe ni texte clinique. Les droits du dossier de stockage sont contrôlés.'),
 ('NF-09\nTraçabilité','Journal horodaté des créations, modifications, archivages, exports médicaux et corrections. Conserver la référence de l’auteur et de l’objet.','Toute correction d’une observation validée permet de retrouver la version précédente et son addendum.'),
 ('NF-10\nMaintenabilité','Séparer interface Qt, services métier et accès aux données. Utiliser des modèles communs et des tests sur les règles critiques.','Les cinq modules partagent les conventions ; le calcul du stock et les contrôles d’affectation sont testables sans ouvrir la GUI.'),
 ('NF-11\nDéploiement','Distribuer l’exécutable Windows avec ses dépendances Qt et un répertoire de données distinct. Fonctionner sans accès à Internet pour les opérations courantes.','Lancement sur un poste de test, création d’une fiche, fermeture puis réouverture avec conservation des données.')],[2.8,7.9,6.3],10.1)
h('Organisation de la recette',4)
p('La recette utilise une base de démonstration contenant des données fictives, séparée des données courantes. Chaque exigence CRUD et chaque métier innovant possède au moins un scénario nominal et un scénario de refus ou de limite. Les cas décrits dans les fiches de module servent de minimum de validation.')
p('Les contrôles portent notamment sur les conflits d’affectation, les absences, les quantités insuffisantes, les lots expirés, la capacité des locaux, l’archivage de fiches liées et les accès non autorisés. Les résultats sont consignés avec l’identifiant de l’exigence, le jeu de données, le résultat attendu et le résultat obtenu.')
p('La démonstration est recevable lorsque les opérations de base des cinq modules, les dix métiers innovants, les exports et les statistiques sont vérifiés, et qu’aucune anomalie bloquante ne subsiste sur la cohérence des données ou les accès.')

# ODD
new();h('2.3 Liste des ODDs touchés',2)
p('Le projet vise des contributions locales aux objectifs de développement durable. Les liens présentés ci-dessous correspondent aux usages prévus de Pattia ; ils ne constituent ni une certification ni la preuve d’un impact déjà obtenu.')
table(['ODD','Contribution envisagée','Indicateur proposé'],[
 ('ODD 12\nConsommation et production durables','Le suivi des lots, la sortie selon la péremption et les propositions de réapprovisionnement peuvent limiter les achats inutiles et les produits jetés.','Quantité et valeur des produits périmés par mois ; nombre de ruptures sur les articles suivis.'),
 ('ODD 8\nTravail décent et croissance économique','La visibilité des disponibilités, des compétences et de la charge aide à organiser le travail. La traçabilité du nettoyage améliore le suivi des conditions d’utilisation des espaces.','Nombre de conflits d’affectation ; écart de taux de charge entre employés de même fonction.'),
 ('ODD 9\nIndustrie, innovation et infrastructure','La centralisation numérique et les traitements métier soutiennent la modernisation du fonctionnement du centre. Cette contribution reste indirecte et limitée à son organisation.','Part des prestations suivies dans l’application ; nombre de ressaisies évitées dans le parcours de référence.')],[3.8,8.0,5.2],11)
h('Méthode de suivi',4)
p('Le centre pourra relever ces indicateurs sur une période de référence puis sur une période d’utilisation comparable. Les variations devront être interprétées en tenant compte du volume d’activité. Aucun objectif chiffré de réduction n’est fixé avant cette mesure initiale.')
h('Références',4)
p('Les intitulés des ODD sont issus des pages officielles des Nations Unies, consultées le 26 septembre 2026. Les rapprochements avec les fonctionnalités de Pattia constituent l’analyse du projet.')
link('Nations Unies  •  Objectif 12  •  Consommation et production durables','https://www.un.org/sustainabledevelopment/fr/sustainable-consumption-production/')
link('Nations Unies  •  Objectif 8  •  Travail décent et croissance économique','https://www.un.org/sustainabledevelopment/fr/economic-growth/')
link('Nations Unies  •  Objectif 9  •  Industrie, innovation et infrastructure','https://www.un.org/sustainabledevelopment/fr/infrastructure/')
p('Référence pédagogique : Plan du cahier de spécification, Projet C++ 2A, année universitaire 2025/2026. L’organisation du présent document suit ses sections 1, 2 et 3.','Caption')

# Identité
new();h('3 Conception graphique',1)
p('L’identité visuelle de Pattia associe une patte animale, un vert sauge et une interface claire. Elle accompagne une application de travail quotidien : les actions et les états doivent rester faciles à distinguer.')
h('3.1 Le logo de l’application',2)
pic(LOGO,4.3,'Figure 1  •  Logo de Pattia fourni pour le projet')
p('Le nom Pattia évoque la patte représentée dans le logo. Il est court et permet d’identifier l’application sans la limiter à une seule prestation. Le symbole renvoie aux animaux pris en charge ; le cercle vert donne une forme simple et reconnaissable à l’identité du centre.')
table(['Élément','Signification et utilisation'],[
 ('Patte anthracite','Symbole principal lié à l’activité du centre. Le contraste assure sa lisibilité sur le cercle vert.'),
 ('Cercle vert sauge','Couleur de marque utilisée pour les repères visuels et les surfaces secondaires.'),
 ('Nom Pattia','Affiché à proximité du symbole sur la couverture et dans la navigation de l’application.')],[4.5,12.5],11)
h('Règles d’utilisation',4)
p('Conserver les proportions du logo et son fond transparent. Prévoir un espace libre autour du symbole équivalant au quart de son diamètre. Sur l’interface, utiliser un diamètre apparent d’au moins 32 pixels ; dans un document imprimé, un diamètre d’au moins 15 mm.')
p('Privilégier un fond blanc ou ivoire. Ne pas déformer le symbole, ne pas en modifier les couleurs et éviter les fonds chargés. Le nom de l’application reste présent lorsque le contexte ne permet pas d’identifier la marque avec le seul symbole.')

new();h('3.2 La charte graphique',2)
p('La charte reprend la sobriété de la maquette de référence : navigation latérale, surfaces claires, cartes de synthèse, tableaux lisibles et boutons arrondis. La couleur verte marque les actions principales et les informations positives.')
colors=[('Vert sauge','#9FC8AA','Couleur du logo et accents secondaires.'),('Vert profond','#246756','Boutons principaux, liens et titres de l’interface.'),('Anthracite','#3B3B3B','Texte principal et symbole du logo.'),('Ivoire clair','#F5F6F2','Fond général de l’application.'),('Blanc','#FFFFFF','Tableaux, formulaires et surfaces de contenu.'),('Vert pâle','#E6F0E7','Éléments sélectionnés et informations de suivi.'),('Ocre','#9A671B','Avertissements avec libellé et icône.'),('Rouge brique','#A8423F','Erreurs et actions nécessitant une attention.')]
t=table(['Couleur','Code','Usage'],colors,[4.2,3.0,9.8],11)
for row,(name,hexc,desc) in zip(t.rows[1:],colors):
    cell=row.cells[1];sh=OxmlElement('w:shd');sh.set(qn('w:fill'),hexc[1:]);cell._tc.get_or_add_tcPr().append(sh)
    for run in cell.paragraphs[0].runs:run.font.color.rgb=RGBColor.from_string('FFFFFF' if hexc in ['#246756','#3B3B3B','#9A671B','#A8423F'] else '333333')
h('Typographie et hiérarchie',4)
p('La police de l’interface est Segoe UI. Les titres d’écran utilisent 26 à 28 pixels, les titres de zone 18 pixels et le texte courant 14 à 16 pixels. Les informations secondaires restent lisibles et ne portent pas seules une consigne essentielle. La grille utilise des espacements réguliers de 8, 16 et 24 pixels.')
h('Composants et états',4)
p('Le bouton principal est vert profond avec un texte blanc. Les actions secondaires utilisent un fond blanc et une bordure légère. Une suppression demande confirmation et précise l’objet concerné. Un état vide explique l’absence de données et propose une action utile.')
p('Les statuts possèdent toujours un libellé : Planifié, En cours, Terminé, Annulé, À nettoyer ou En maintenance. Les formulaires signalent les champs obligatoires et conservent les valeurs saisies lorsqu’une erreur est détectée. Le focus clavier doit rester visible.')

screens=[
 ('3.3.1','Tableau de bord',0,'La page d’accueil donne une vue immédiate de l’activité. Les indicateurs présentent les services du jour, les employés présents, les alertes de stock et les locaux disponibles.',[
 ('Navigation','La barre latérale donne accès aux cinq modules et indique la page active. Le logo reste visible pour conserver un repère commun.'),('Indicateurs','Les cartes résument des données calculées depuis les modules. Un clic doit ouvrir la liste correspondant au compteur.'),('Services du jour','Le tableau associe prestation, animal, horaire et statut. La sélection d’une ligne donne accès à la fiche du service.'),('Alertes','La zone inférieure regroupe les éléments demandant une action ; chaque alerte conduit au module responsable.')],
 'Le bouton Nouveau service ouvre le formulaire de création. Les utilisateurs ne voient que les données autorisées par leur rôle.'),
 ('3.3.2','Interface de gestion des Services',1,'Cet écran permet de retrouver une prestation, de préparer une affectation et de suivre son exécution.',[
 ('Liste et filtres','La recherche, les filtres et le tri réduisent la liste affichée. L’export reprend le même périmètre.'),('Actions','Ajouter ouvre un formulaire ; Consulter et Modifier agissent sur la ligne sélectionnée. Archiver préserve les prestations liées à un historique.'),('Créneau proposé','La zone de proposition indique l’horaire, la durée, le local et la ressource retenue. La confirmation réévalue les disponibilités.')],
 'Le formulaire prévoit le libellé, la catégorie, l’animal, la date, l’heure, la durée, le responsable, le local, le tarif et le statut. Une estimation peut être préparée avant validation.'),
 ('3.3.3','Interface de gestion des Employés',2,'Le responsable consulte les fiches des employés et dispose d’une vue des compétences et de la charge planifiée.',[
 ('Fiche employé','La liste affiche le nom, le poste, une compétence principale et le taux de charge. La fiche complète contient les coordonnées et les disponibilités.'),('Aide à l’affectation','La recherche d’un profil compare les compétences requises aux compétences enregistrées. Les employés inactifs sont exclus.'),('Répartition du travail','Une suggestion explique le déséquilibre détecté. Le responsable choisit d’appliquer ou de refuser la réaffectation.')],
 'Le taux de charge est calculé sur une période clairement affichée. L’interface signale une disponibilité nulle et empêche une affectation pendant une absence.'),
 ('3.3.4','Interface de gestion du Stock',3,'Le magasinier suit les articles, les lots utilisables et les alertes. Les quantités illustrées sont des exemples sans lien avec un inventaire réel.',[
 ('Suivi des articles','Les colonnes montrent la désignation, le lot, la quantité et l’état. La fiche distingue les entrées, sorties et corrections.'),('Sortie par lot','La proposition FEFO présente le lot à utiliser en priorité. Le lot expiré ou bloqué est exclu et la quantité est contrôlée à la validation.'),('Réapprovisionnement','Le détail affiche la moyenne de consommation, le délai fournisseur, la sécurité retenue et la quantité conseillée.')],
 'Le bouton Ajouter ouvre une fiche article. Les mouvements sont saisis dans la fiche de détail ; toute sortie requiert une quantité, un motif et, lorsque pertinent, un service associé.'),
 ('3.3.5','Interface de gestion des Locaux',4,'Le référent des locaux consulte les capacités, les occupations et les indisponibilités nécessaires à la planification.',[
 ('Disponibilité','La liste distingue Disponible, Occupé, À nettoyer et En maintenance. Une recherche sur une période contrôle aussi les réservations futures.'),('Nettoyage','Après une prestation concernée, la fiche de remise en service rassemble les tâches à valider, l’agent et l’heure de réalisation.'),('Maintenance','L’historique conserve les interventions et les compteurs d’utilisation. Une intervention planifiée réserve une plage d’indisponibilité.')],
 'Le formulaire comprend le nom, le type, la capacité, la surface, les équipements et l’état. La capacité engagée et les temps de nettoyage sont contrôlés avant toute réservation.'),
 ('3.3.6','Interface de gestion des dossiers médicaux',5,'Le vétérinaire recherche un animal, consulte son historique et prépare les prochaines actions de suivi depuis son dossier.',[
 ('Recherche du dossier','La recherche utilise le numéro, l’animal ou le propriétaire. Les filtres portent notamment sur l’espèce et la période.'),('Historique clinique','La fiche détaille les consultations, allergies, traitements, vaccins et pesées. Une observation validée est conservée avec son auteur.'),('Suivi','Les échéances vaccinales produisent une liste de rappels à préparer. Les variations de poids conduisent à une vérification par le vétérinaire.')],
 'Les coordonnées du propriétaire sont affichées uniquement aux profils autorisés. L’export médical est journalisé. Les alertes de suivi ne produisent aucune décision clinique automatique.')
]

for idx,(num,title,shot,intro,items,ending) in enumerate(screens):
    new()
    if idx==0:
        h('3.3 Les interfaces graphiques GUI',2)
        p('Les six captures suivantes proviennent de maquettes réalisées avec Qt Widgets. Elles présentent l’interface proposée pour Pattia avec des données fictives. Les règles et fonctions décrites dans les sections précédentes constituent la cible de développement.', 'Caption')
    h(num+' '+title,3)
    p(intro)
    pic(WORK/'gui'/f'gui_{shot}.png',17,f'Figure {shot+2}  •  {title}  •  Maquette Qt et données de démonstration')
    table(['Zone','Comportement attendu'],items,[4.0,13.0],10.4)
    p(ending)

dest=OUT/'Pattia_Cahier_de_specification.docx'
doc.save(dest)
print(dest)
print('paragraphs',len(doc.paragraphs),'tables',len(doc.tables),'images',len(doc.inline_shapes))
