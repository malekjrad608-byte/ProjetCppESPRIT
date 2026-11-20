from pathlib import Path
p=Path('README.md'); s=p.read_text(encoding='utf-8')
s=s.replace('Services, Stock et Ressources humaines','Services, facturation PDF et Stock').replace('**2.6.0**','**3.0.0**').replace('Les services et les ressources humaines conservent leurs bases SQLite locales','Les services et leurs références de facturation conservent leur base SQLite locale')
s=s.replace('- Cliquer sur **Ressources humaines** pour retrouver les huit sections RH et les données de l’application RH existante.\n','').replace('sept prestations et huit employés fictifs','sept prestations fictives').replace('**Espace principal**','**Navigation**')
a=s.index('## Ressources humaines') if '## Ressources humaines' in s else s.index('## Intégration RH')
b=s.index('## Fonctionnalités Services',a)
s=s[:a]+'''## Facturation automatique et nouvelle interface

À chaque création d'un service, SQLite attribue automatiquement un numéro unique `FAC-AAAA-000001` et une date d'émission. Le numéro reste identique après modification et réouverture. Le formulaire demande le **nom du client** ; le détail des prestations reprend le nom, la description, la catégorie, le responsable, la date, la durée et le tarif du service.

Sélectionner un service puis **Facture PDF** pour consulter la facture et l'exporter. Le total à payer correspond au tarif saisi en DT. Le contenu reflète les dernières informations enregistrées du service ; un PDF déjà exporté reste un fichier indépendant. Il ne s'agit pas d'un suivi d'encaissement. Les services annulés ne sont pas facturables.

Les bases précédentes sont migrées sans suppression de services : le client reste vide pour les anciennes consultations et doit être complété avec **Modifier** avant l'export de leur facture. Aucun nom de client n'est inventé.

L'application propose uniquement des exports **PDF** : factures individuelles, rapport filtré des services et inventaire filtré du stock. La duplication de service et le module RH sont retirés de la navigation et de la compilation. Les anciennes sources RH et leur base externe restent conservées hors du programme.

La présentation reprend les références fournies : navigation blanche compacte, fond bleu très clair, cartes avec sélection dorée, boutons turquoise et statistiques en fenêtre avec indicateurs pastel et tableau. Le Stock utilise les mêmes composants et couleurs.

'''+s[b:]
s=s.replace('- Logo agrandi :','- Logo :')
s='\n'.join(line for line in s.split('\n') if not line.startswith('- Duplication :'))
s=s.replace('durée, responsable, tarif en DT','durée, responsable, client obligatoire, tarif en DT').replace('ID, catégorie, notes ou responsable','ID, client, numéro de facture, catégorie, notes ou responsable').replace('tri par date, nom ou tarif','tri par date, nom, tarif et durée (croissant/décroissant)')
s=s.replace('- Menu **Exporter** : rapport PDF paginé avec le logo et les filtres appliqués, ou CSV UTF-8 des résultats affichés.','- **Exporter PDF** : rapport paginé avec le logo et les filtres appliqués. **Facture PDF** : facture individuelle avec client et total à payer.')
s=s.replace('**Stock** et **Ressources humaines**','**Stock**').replace('- `src/rh/` : module RH intégré ; modèles SQLite, formulaires, onglets, planning et rapports.','- `src/ui/invoice.*` : facture, échappement du contenu et rendu PDF paginé commun avec le stock.')
s=s.replace('export CSV','export PDF').replace("l'échappement CSV", "la migration des factures, leur numérotation et leur persistance").replace('le CSV filtré','le PDF filtré')
a=s.find("Les tests d'intégration RH")
if a>=0:s=s[:a]
p.write_text(s,encoding='utf-8')

