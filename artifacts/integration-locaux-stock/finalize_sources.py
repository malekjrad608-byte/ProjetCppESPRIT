from pathlib import Path
import re
p=Path('src/stock/stockwidget.cpp');s=p.read_text(encoding='utf-8')
a=s.index('QString categoryFromArticle');b=s.index('QString articleName',a);s=s[:a]+s[b:]
a=s.index('int numericQuantity');b=s.index('QPushButton *makeButton',a)
s=s[:a]+'''QString searchable(QString text) {
    text=text.normalized(QString::NormalizationForm_D).toCaseFolded();
    QString result;
    for(const QChar c:text) if(c.category()!=QChar::Mark_NonSpacing) result+=c;
    return result;
}

'''+s[b:]
s=s.replace('!haystack.contains(query, Qt::CaseInsensitive)','!searchable(haystack).contains(searchable(query))')
s=s.replace('Design::button("Exporter PDF", "download")','Design::button("Exporter PDF", "export")')
s=s.replace('        table->setRowCount(0);','        table->clearSpans();\n        table->setRowCount(0);')
p.write_text(s,encoding='utf-8')
p=Path('README.md');s=p.read_text(encoding='utf-8-sig')
s=s.replace('Services, Employés, Stock et comptes par module','Services, Employés, Stock, Locaux et comptes par module',1)
s=s.replace('le module Stock fourni reste une interface de démonstration.','les locaux utilisent un fichier JSON local et le Stock fourni conserve ses données en mémoire pendant la session.',1)
s=s.replace('**services**, **stock** et **employes**','**services**, **stock**, **employes** et **locaux**')
s=s.replace('| Administration |','| Locaux | `locaux` | `locaux123` | Locaux |\n| Administration |',1)
a=s.index('## Stock intégré et guide GUI');b=s.index('## ',a+4)
s=s[:a]+'''## Stock remplacé et Locaux intégrés — 12 octobre 2026

Sources originales conservées intactes :

- Stock : `C:/Users/malek/Downloads/ANIWORLD_Stock_finale/ANIWORLD_Stock_Modifie`.
- Locaux : `C:/Users/malek/Downloads/locauxF/ANIWORLD_Locaux`.

Le nouveau Stock remplace le précédent dans `src/stock/`. Son `main.cpp`, sa fenêtre de connexion et sa barre latérale séparée ne sont pas compilés : il utilise la connexion, les animations et la navigation communes. Les 19 articles fournis, l'ajout d'article depuis le mouvement, la modification, la suppression, les entrées/sorties, les statistiques et le remplacement/suppression des lots périmés sont repris. Les filtres catégorie/état/recherche se combinent ; le tri porte sur la quantité ou la date d'échéance dans les deux sens. L'export PDF conserve les résultats et l'ordre affichés. L'ancien FEFO et la proposition de réapprovisionnement sont remplacés par la gestion des échéances du nouveau module.

**Le Stock fourni est en mémoire uniquement**, comme son projet d'origine. Les changements subsistent pendant la navigation et les changements de compte mais disparaissent à la fermeture. Le bandeau et le pied de page l'indiquent. Les quantités de sortie sont contrôlées, avec un plafond de 999 999 unités par lot.

Locaux est intégré dans `src/locaux/` : cartes/liste, création/modification/suppression, occupation, nettoyage, recherche intelligente, alertes, affectation assistée, plan interactif, statistiques et rapport de nettoyage PDF. Les boutons et fonds utilisent le design commun turquoise, blanc et orange. Le plan se ferme lorsqu'on quitte la page, notamment au changement de compte.

En mode normal, les locaux sont enregistrés dans `%LOCALAPPDATA%/ANIWORLD/ANIWORLD/locaux_v2.json`. Au premier accès, une copie des locaux existants du projet fourni est importée depuis `resources/data/locaux-initial.json`. Les lancements suivants conservent les modifications et suppressions, même si tous les locaux ont été supprimés. L'écriture JSON est atomique ; un fichier illisible n'est jamais remplacé par des exemples. Le mode `--demo` garde les locaux en mémoire.

Une seule fenêtre et un seul `main.cpp` : 0 = Tableau de bord, 1 = Services, 2 = Stock, 3 = Employés, 4 = Locaux. Les comptes existants et mots de passe modifiés sont conservés lors de la migration ; seul le compte manquant `locaux` est ajouté. Les raccourcis Ctrl+N / Ctrl+F sont routés vers le module autorisé.

Sauvegarde avant intégration : `artifacts/integration-locaux-stock/backup/` (sources, ressources, tests, base SQLite et JSON d'origine).

'''+s[b:]
s=s.replace('Après connexion, l\'application ouvre **Services**, sans formulaire MySQL. Le tableau de bord reste accessible dans la navigation.','Après connexion, chaque compte ouvre directement son module. Le tableau de bord est accessible aux comptes Services et Administration.')
s=s.replace('La navigation active contient **Tableau de bord**, **Services**, **Stock**. Les futurs modules **Locaux** et **Animaux & dossiers** (animaux et dossiers médicaux réunis) restent désactivés. Stock est intégré dans le périmètre GUI décrit ci-dessus.','La navigation active contient **Tableau de bord**, **Services**, **Stock**, **Employés** et **Locaux**. Seul le futur module **Animaux & dossiers** reste désactivé. Chaque compte voit ses modules autorisés.')
s=re.sub(r'Les tests Stock vérifient les filtres combinés,.*?\n\n','Les tests Stock vérifient les filtres combinés, les tris numériques et chronologiques, le PDF filtré, les mouvements valides et invalides, les modifications après tri, la suppression, les échéances, les statistiques et la navigation intégrée. Les tests Locaux couvrent la persistance, les échecs d’écriture, les fichiers invalides, le nettoyage, la recherche, l’affectation assistée, le PDF, l’ajout et le plan. Captures actuelles : `artifacts/integration-locaux-stock/`.\n\n',s,flags=re.S)
p.write_text(s,encoding='utf-8')
print('Documentation and stock refinements saved')
