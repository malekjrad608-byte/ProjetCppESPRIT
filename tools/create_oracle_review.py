from pathlib import Path
from xml.sax.saxutils import escape
from reportlab.pdfgen import canvas
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.colors import HexColor, white
from reportlab.lib.enums import TA_LEFT
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'output/pdf/ANIWORLD_Oracle_revue_et_reunion.pdf'
OUT.parent.mkdir(parents=True, exist_ok=True)
pdfmetrics.registerFont(TTFont('UI', 'C:/Windows/Fonts/segoeui.ttf'))
pdfmetrics.registerFont(TTFont('UIB', 'C:/Windows/Fonts/segoeuib.ttf'))
pdfmetrics.registerFontFamily('UI', normal='UI', bold='UIB', italic='UI', boldItalic='UIB')
teal, ink, muted, orange = map(HexColor, ['#267080', '#203E4B', '#607985', '#DF8B24'])
styles = {
    'title': ParagraphStyle('title', fontName='UIB', fontSize=25, leading=30, textColor=ink, spaceAfter=13),
    'intro': ParagraphStyle('intro', fontName='UI', fontSize=11, leading=16, textColor=muted, spaceAfter=16),
    'h': ParagraphStyle('h', fontName='UIB', fontSize=13, leading=17, textColor=teal, spaceBefore=13, spaceAfter=7),
    'p': ParagraphStyle('p', fontName='UI', fontSize=10, leading=14.3, textColor=ink, spaceAfter=8),
    'small': ParagraphStyle('small', fontName='UI', fontSize=8.5, leading=12, textColor=muted, spaceAfter=7),
    'cell': ParagraphStyle('cell', fontName='UI', fontSize=9, leading=12.5, textColor=ink),
    'head': ParagraphStyle('head', fontName='UIB', fontSize=9, leading=12.5, textColor=white),
}
story = []
def p(text, style='p'):
    return Paragraph(text, styles[style])
def add(text, style='p'):
    story.append(p(text, style))
def title(n, text, subtitle):
    if n > 1: story.append(PageBreak())
    add(f'{n:02d} / {text}', 'title')
    add(subtitle, 'intro')
def table(headers, rows, widths):
    data = [[p(escape(x), 'head') for x in headers]] + [[p(x, 'cell') for x in row] for row in rows]
    t = Table(data, colWidths=widths, hAlign='LEFT', repeatRows=1)
    t.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), teal), ('VALIGN', (0, 0), (-1, -1), 'TOP'),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [HexColor('#F0F6F8'), white]),
        ('LEFTPADDING', (0, 0), (-1, -1), 10), ('RIGHTPADDING', (0, 0), (-1, -1), 10),
        ('TOPPADDING', (0, 0), (-1, -1), 9), ('BOTTOMPADDING', (0, 0), (-1, -1), 9),
        ('LINEBELOW', (0, -1), (-1, -1), .6, HexColor('#D5E6EB')),
    ]))
    story.append(t)
def qa(number, question, answer):
    story.append(KeepTogether([p(f'{number:02d}. {question}', 'h'), p(answer)]))

title(1, 'Oracle est connecté', 'ANIWORLD / Livraison technique et préparation de réunion / 14 octobre 2026')
add('La base fonctionne avec l’application Qt. Le lancement normal utilise désormais Oracle ; les données et les comptes de la version locale ont été conservés. Le design des cinq modules reste commun.')
table(['Connexion installée', 'Valeur'], [
    ['Serveur / service', 'localhost : 1521 / <b>XEPDB1</b>'],
    ['Schéma applicatif', '<b>ANIWORLD_APP</b> - compte dédié, sans rôle DBA'],
    ['Qt / pilote', 'Qt 6.7.3, 64 bits / QODBC / DSN <b>ANIWORLD_ORACLE</b>'],
    ['SQL Developer', 'Profil importé : <b>ANIWORLD - XEPDB1</b>'],
], [170, 329])
add('Données vérifiées', 'h')
table(['Module', 'Lignes', 'Origine'], [
    ['Services', '2', 'Copie de la base SQLite existante'],
    ['Employés', '6', 'Copie de la base SQLite existante'],
    ['Dossiers médicaux', '8', 'Copie de la base SQLite existante'],
    ['Comptes / planning', '6 / 0', 'Identifiants, sels et empreintes conservés'],
    ['Locaux / lots de stock', '14 / 19', 'Import initial des données disponibles dans les modules'],
], [170, 60, 269])
add('Ce que cette livraison couvre', 'h')
add('11 tables Oracle : accounts, services, employes, planning_affectations, medical_records, locaux, stock_lots, stock_movements, collection_versions, integration_migrations et schema_version.', 'small')
add('<b>Limite à présenter clairement :</b> ce schéma assure la compatibilité avec le code Qt actuel. Ce n’est pas la transcription complète du modèle Lucidchart. Les tables distinctes CLIENT, ANIMAL, CONSULTATION, FACTURE et LIGNE_FACTURE restent une évolution à concevoir et à intégrer. Les exemples déjà présents ne constituent pas des données métier validées.')

title(2, 'Corriger la conception', 'Lecture du diagramme Lucidchart exporté en PDF. Le document partagé n’a pas été modifié.')
add('La séparation EMPLOYE / SERVICE / CONSULTATION / ANIMAL est une bonne base. Les identifiants et les clés étrangères sont présents. En revanche, le dessin de certaines relations ne correspond pas aux clés étrangères, et plusieurs besoins de l’application manquent.')
table(['Priorité / constat', 'Correction proposée'], [
    ['<b>1. Cardinalités inversées</b><br/>STOCK - ARTICLE<br/>ANIMAL - ADOPTION', 'Un stock contient <b>0..N articles</b> ; chaque article dépend d’un stock. Un animal reçoit <b>0..N demandes</b> ; chaque demande concerne un animal. Placer la patte d’oie du côté ARTICLE ou ADOPTION.'],
    ['<b>2. Dossier par animal</b><br/>ANIMAL - DOSSIER_MEDICAL', 'Si un animal possède au maximum un dossier : Animal <b>0..1 dossier</b>, dossier <b>1 animal</b>, avec <b>UNIQUE(id_animal)</b>. Si le dossier est obligatoire dès la création, préciser 1..1 et le garantir dans le processus.'],
    ['<b>3. Facturation absente</b>', 'Ajouter FACTURE et LIGNE_FACTURE, un numéro unique, une date et le client. Conserver quantité, désignation et prix facturé dans les lignes. Plusieurs prestations nécessitent une association avec la consultation ; son unique id_service actuel ne suffit pas.'],
    ['<b>4. Propriétaire absent</b>', 'Créer CLIENT / PROPRIETAIRE et relier l’animal à son propriétaire selon la règle retenue. Un adoptant n’est pas nécessairement le client qui règle une consultation.'],
    ['<b>5. Lots et locaux absents</b>', 'Séparer ARTICLE et LOT_STOCK : un article peut avoir plusieurs lots et échéances. Rattacher les mouvements au lot. Ajouter LOCAL pour représenter le module déjà intégré à Qt.'],
    ['<b>6. Périmètre et contraintes</b>', 'Retirer RH si ce module est hors périmètre. Ajouter les comptes et rôles. Définir NOT NULL, statuts autorisés, unicité pertinente et règles de suppression. CHECK(prix &gt;= 0) seul n’interdit pas NULL.'],
], [155, 344])
add('Deux points à valider avec la prof', 'h')
add('Une consultation peut-elle contenir plusieurs prestations ? Une facture correspond-elle à une seule consultation ? Ces réponses déterminent les tables d’association et les contraintes UNIQUE. Pour le suivi vaccinal, prévoir des événements datés si un historique est attendu, plutôt qu’un simple champ texte.')

title(3, 'Questions de la prof', 'Réponses proposées / Architecture, connexion et données')
qa(1, 'Pourquoi utiliser un Singleton ?', 'Le gestionnaire de base expose une instance unique via une variable statique locale, comme dans l’atelier Meyers. Son constructeur est privé et la copie est interdite. Cela centralise la connexion utilisée par l’interface. Ce mécanisme ne rend pas automatiquement les requêtes utilisables depuis plusieurs threads.')
qa(2, 'Quel est le chemin entre Qt et Oracle ?', 'QtSql utilise le pilote QODBC, le DSN 64 bits ANIWORLD_ORACLE, puis le pilote Oracle vers localhost:1521/XEPDB1. SQL Developer est un autre client de la même base ; il n’a pas besoin de rester ouvert pour que Qt fonctionne.')
qa(3, 'Pourquoi XEPDB1 plutôt que XE ?', 'Sur cette installation Oracle XE 21c, XEPDB1 est la base enfichable choisie pour le schéma applicatif. Les valeurs du support de cours servent d’exemple ; elles ne remplacent pas les paramètres de l’installation réelle. La connexion vérifiée affiche ANIWORLD_APP / XEPDB1.')
qa(4, 'Comment prouves-tu que les données sont enregistrées ?', 'Les opérations Qt sont testées sur le schéma Oracle. Les valeurs migrées ont été comparées à SQLite : 256 cellules identiques sur les cinq tables métier et de comptes. Les tables sont également consultables dans SQL Developer avec verification.sql. L’ancienne base reste disponible séparément.')
qa(5, 'Comment sont générés les identifiants ?', 'Les tables migrées utilisent des séquences et des triggers. Le code récupère CURRVAL dans sa session pour les insertions concernées, sans calculer MAX(id)+1. Les séquences ont été avancées au-delà des identifiants importés. Les numéros existants de factures sont préservés.')
qa(6, 'Comment éviter les injections et les erreurs partielles ?', 'Les valeurs sont liées à des requêtes préparées. Les écritures groupées utilisent une transaction : validation complète puis COMMIT, sinon ROLLBACK. Les contraintes Oracle complètent les validations Qt. Le test d’une quantité négative confirme l’annulation de l’opération.')

title(4, 'Questions de la prof', 'Réponses proposées / Métier, sécurité et limites')
qa(7, 'Les comptes des modules sont-ils des utilisateurs Oracle ?', 'Non. Les comptes services, stock, employes, locaux, medical et admin sont des comptes applicatifs avec des rôles. Qt accède à Oracle par ANIWORLD_APP. Les mots de passe applicatifs restent protégés par PBKDF2 avec sel ; le secret Oracle est chiffré pour le compte Windows par DPAPI.')
qa(8, 'Comment fonctionne la facture PDF ?', 'La version actuelle conserve le numéro, la date et les informations de facturation dans le service et génère le PDF. Elle ne dispose pas encore du modèle normalisé FACTURE / LIGNE_FACTURE ni d’une liaison complète avec les consultations médicales. L’export PDF ne prouve pas, à lui seul, le paiement.')
qa(9, 'Pourquoi des dates texte dans la base installée ?', 'Le modèle Lucidchart propose DATE, ce qui convient à la cible relationnelle. Pour conserver les interfaces et requêtes actuelles, cette migration garde des dates au format ISO dans plusieurs colonnes VARCHAR2. L’évolution recommandée est DATE ou TIMESTAMP avec adaptation des liaisons Qt et validation des données existantes.')
qa(10, 'Que se passe-t-il si deux personnes modifient le stock ?', 'Les sauvegardes Stock et Locaux verrouillent une révision dans une transaction et refusent une copie périmée. Le stock conserve un journal des variations. Cette protection a été testée. Les autres modules ne disposent pas encore du même contrôle de concurrence : c’est une amélioration à prévoir pour un usage multi-utilisateur.')
qa(11, 'Pourquoi ne pas supprimer toutes les données liées ?', 'Une suppression automatique peut effacer l’historique médical ou comptable. Il faut définir, relation par relation, les règles de conservation : désactivation, refus ou suppression justifiée. Le planning suit actuellement la suppression de l’employé ; cette règle doit être validée avec le métier.')
qa(12, 'Que se passe-t-il si Oracle est arrêté ?', 'Le mode Oracle affiche une erreur de connexion et ne bascule pas silencieusement vers une base locale vide. Le mode --local reste un choix explicite et indépendant. Pour une installation sur un autre poste, il faut configurer Oracle, le pilote, le DSN et un nouveau secret Windows ; copier seulement l’exécutable ne suffit pas.')

title(5, 'Démonstration et preuves', 'Mode d’emploi pour la réunion et repères pour reprendre le projet')
add('Présenter en quatre étapes', 'h')
add('<b>1.</b> Lancer ANIWORLD/ANIWORLD.exe et se connecter avec un compte applicatif existant. Le badge doit afficher « Oracle · connecté ».<br/><b>2.</b> Créer un service de démonstration identifiable, vérifier sa facture PDF, puis ouvrir le même enregistrement dans SQL Developer.<br/><b>3.</b> Fermer et relancer l’application pour montrer la persistance. Nettoyer uniquement l’enregistrement de démonstration créé.<br/><b>4.</b> Montrer la conception et expliquer les écarts restants, en particulier facture / consultation et types de dates.')
add('Ouvrir les tables dans SQL Developer', 'h')
add('Le profil <b>ANIWORLD - XEPDB1</b> est importé. Le mot de passe n’est pas enregistré dans ce profil. Exécuter <b>tools/copy-oracle-password.ps1</b>, ouvrir la connexion, coller le mot de passe puis valider soi-même. Ouvrir <b>database/oracle/verification.sql</b> et utiliser F5. Ne pas réexécuter schema.sql sur la base déjà installée.')
add('Vérifications exécutées le 14 octobre 2026', 'h')
table(['Preuve', 'Résultat'], [
    ['Application livrée', '--check-oracle : connexion réelle réussie, six comptages réussis'],
    ['Suite habituelle', '35 réussites, 0 échec ; 4 tests conditionnels exécutés séparément'],
    ['Test Oracle dédié', '3 réussites avec initialisation/nettoyage : CRUD, CLOB, PDF, comptes, stock, conflits et annulation'],
    ['Captures visuelles', '5 réussites avec initialisation/nettoyage : connexion, écrans, animations'],
    ['Migration et schéma', '256 valeurs identiques ; aucun objet Oracle invalide'],
], [165, 334])
add('Fichiers à conserver', 'h')
add('Schéma : database/oracle/schema.sql. Configuration locale : %LOCALAPPDATA%/ANIWORLD/ANIWORLD/oracle.ini et oracle-password.dpapi. Sauvegarde avant migration : backups/before-oracle-20261014-005853/. Logs et captures : artifacts/oracle-integration/. Les secrets chiffrés sont liés au compte Windows et ne constituent pas une configuration portable.', 'small')
add('Sources de la revue', 'h')
add('Documents fournis : « Atelier Connexion Oracle QT -Singleton (1).pptx » et archive « Atelier_conexion_Meyers Singleton.zip ». Conception consultée : export « smart pet.pdf » du diagramme Lucidchart 5828c0fa-ae33-48df-9c20-4779f3605e3e, copie dans artifacts/oracle-integration/schema-lucid-original.pdf.', 'small')
add('Références officielles : <link href="https://doc.qt.io/qt-6/qsqldatabase.html" color="#267080">Qt / QSqlDatabase</link> ; <link href="https://doc.qt.io/qt-6/qsqlquery.html" color="#267080">Qt / QSqlQuery</link> ; <link href="https://docs.oracle.com/en/database/oracle/oracle-database/21/sqlrf/CREATE-TABLE.html" color="#267080">Oracle 21c / CREATE TABLE</link>. Recommandations de conception : analyse du diagramme et des besoins ANIWORLD.', 'small')

def decoration(c, doc):
    w, h = doc.pagesize
    c.setFillColor(teal); c.rect(0, h-10, w, 10, fill=1, stroke=0)
    c.setFont('UIB', 9); c.drawString(48, h-36, 'ANIWORLD')
    c.setFont('UI', 8); c.setFillColor(muted); c.drawRightString(w-48, h-36, 'ORACLE + QT / REVUE DE PROJET')
    c.setStrokeColor(HexColor('#D5E6EB')); c.line(48, 43, w-48, 43)
    c.setFont('UI', 8); c.drawString(48, 28, '14 octobre 2026 / Préparation de réunion')
    c.drawRightString(w-48, 28, str(doc.page))

SimpleDocTemplate(str(OUT), pagesize=(595,842), leftMargin=48, rightMargin=48, topMargin=65, bottomMargin=60,
                  title='ANIWORLD - Oracle, revue de conception et préparation de réunion', author='ANIWORLD').build(story, onFirstPage=decoration, onLaterPages=decoration)
print(OUT)
