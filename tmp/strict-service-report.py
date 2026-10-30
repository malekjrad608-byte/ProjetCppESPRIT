from pathlib import Path
p=Path('tools/create_workshop_report.py');s=p.read_text(encoding='utf-8-sig')
updates={
6:('Liste des enregistrements','Corrigé le 19/10','La liste Services est un QTableView directement branché sur Stmp.afficher(). Les cartes utilisent afficherListe(). Recherche, filtres et ordre sont conservés.'),
17:('Entité, constructeurs, accesseurs et CRUD','Corrigé pour Service','Service a des attributs privés, un constructeur par défaut, un constructeur paramétré, des getters/setters et les méthodes ajouter(), modifier(), supprimer(int), afficher(). Employe avait déjà des attributs privés.'),
18:('Implémentation dans le .cpp','Corrigé pour Service','Le constructeur et le SQL INSERT/UPDATE/DELETE/SELECT de Service sont dans service.cpp. La transaction Oracle conserve les liens avec le premier employé et les contrôles du planning.'),
31:('Exemple bindValue','Appliqué à Service','Les écritures et la suppression Service utilisent prepare(), des paramètres nommés (:name, :id, etc.), bindValue() puis exec(). Les filtres utilisent addBindValue().'),
34:('Bouton → objet → ajouter()','Corrigé','Le formulaire construit Service à partir des champs. ServiceWidget appelle bool test = adding ? entity.ajouter() : entity.modifier(), traite lastError() puis actualise après succès.'),
37:('Objet modèle dans la vue','Corrigé','servicewidget.h déclare Service Stmp, analogue à Etudiant Etmp. Il sert à afficher() et supprimer(id).'),
38:('ID typé et appel supprimer','Appliqué','La vue récupère un identifiant entier puis appelle Stmp.supprimer(id). Le modèle conserve le message Oracle si une relation empêche la suppression.'),
41:('Affichage dans le constructeur','Corrigé','Le constructeur appelle m_table->setModel(Stmp.afficher(this)). populate() utilise le même appel avec les identifiants filtrés, et libère l’ancien modèle et l’ancienne sélection.'),
44:('Actualiser après ajout réussi','Appliqué','Après ajouter()/modifier() réussi, refresh() recharge les données et populate() appelle setModel(Stmp.afficher(...)). Les erreurs restent dans le formulaire.'),
45:('Actualiser après suppression réussie','Appliqué','Après Stmp.supprimer(id) réussi, refresh() recharge la liste et son QSqlQueryModel. Une suppression refusée conserve les données visibles.')}
import re
for n,row in updates.items():
    s=re.sub(r'^\('+str(n)+r',.*$',lambda m:repr((n,*row))+',',s,flags=re.M)
s=s.replace('Vérification du 17 octobre 2026','Mise à jour du 19 octobre 2026')
a=s.index('<h2>Différences exactes');b=s.index('<h2>',a+5)
s=s[:a]+'''<h2>Ce qui a changé le 19 octobre</h2><ul><li><b>service.h</b> : attributs privés, getters/setters, constructeur paramétré et API CRUD sans paramètre d’erreur obligatoire.</li><li><b>service.cpp</b> : SQL Service regroupé dans le modèle, paramètres nommés et transactions Oracle conservées.</li><li><b>servicewidget.h / .cpp</b> : membre Service Stmp, QTableView, setModel(Stmp.afficher()), actualisation et gestion de la sélection.</li><li><b>servicedialog.cpp</b> : construction du Service depuis les champs du formulaire.</li><li>Les consommateurs du modèle (planning, factures, cartes, statistiques, tests) utilisent maintenant ses accesseurs.</li></ul>
<h2>Où montrer le code dans Qt Creator</h2><p>Ctrl+K puis saisir le nom du fichier et Entrée. Dans service.h montrer le constructeur, les méthodes CRUD et la section private ; dans service.cpp montrer Service::Service, ajouter, enregistrerOracle, supprimer et afficher ; dans servicewidget.h montrer Stmp ; dans servicewidget.cpp montrer setModel et l’appel à ajouter/supprimer. Dans connection.cpp montrer le Singleton fourni, puis dans main.cpp la connexion avant les fenêtres.</p>
<h2>Adaptations explicites</h2><ul><li>Service et ses champs remplacent Etudiant/id/nom/prenom. Les cinq tables métier restent le schéma demandé.</li><li>Les formulaires et messages intégrés restent ceux d’ANIWORLD ; les pages 47–48 permettent une autre GUI et demandent de limiter les dialogues.</li><li>L’authentification vulnérable des pages 22–26 est un contre-exemple : les requêtes préparées des pages suivantes sont appliquées. Les comptes locaux hachés sont conservés, sans sixième table account.</li><li>Le Singleton fourni reste identique. Le changement de ce jour porte sur Service et ses consommateurs. Local, StockRecord et MedicalRecord gardent leurs API spécialisées ; le rapport ne les présente pas comme des copies de la classe Etudiant.</li></ul>
'''+s[b:]
s=s.replace("('artifacts/atelier-crud-review/strict-regression.txt','39 tests réussis, aucun échec, 11 tests optionnels ignorés')", "('artifacts/atelier-crud-review/strict-service-regression.txt','Régression du 19 octobre : résultats détaillés')")
s=s.replace("('artifacts/atelier-crud-review/strict-targeted-tests.txt','6 passages ciblés réussis : modèles, formulaire, intégration Oracle, interfaces, initialisation et nettoyage')", "('artifacts/atelier-crud-review/strict-service-oracle.txt','Validation Oracle et captures du 19 octobre')")
s=s.replace('strict-deployed-check.txt','strict-service-deployed-check.txt')
p.write_text(s,encoding='utf-8')
p=Path('README.md');s=p.read_text(encoding='utf-8-sig');s=s.replace('(17 octobre 2026)','(19 octobre 2026)',1)
insert='''## Conformité Services à l’atelier — 19/10/2026

- `Service` : attributs privés, constructeur paramétré, getters/setters et appels `ajouter()`, `modifier()`, `supprimer(id)`, `afficher()`.
- `service.cpp` contient les requêtes de Service, avec `prepare`, `bindValue` et `exec`. La transaction Oracle et les contraintes d’affectation sont conservées.
- `ServiceWidget` possède `Service Stmp`. La liste est maintenant un `QTableView` qui reçoit directement `Stmp.afficher()`. Après une écriture réussie, le modèle est renouvelé et la sélection conservée. Les cartes, filtres et tris restent disponibles.
- `ServiceDialog::getService()` construit l’objet depuis les champs du formulaire. Les autres usages de Service passent par ses accesseurs.
- Le Singleton fourni par l’enseignante est inchangé. Aucune migration ni suppression des données Oracle pour cette refactorisation.

Pour la démonstration dans Qt Creator : **Ctrl+K**, ouvrir successivement `service.h`, `service.cpp`, `servicewidget.h`, `servicewidget.cpp`, `connection.cpp`, `main.cpp`. La correspondance des 50 pages et les adaptations sont dans `artifacts/atelier-crud-review/rapport-atelier.html`.

Sauvegarde de départ : `backups/before-service-atelier-20261019/source.zip`. Vérifications : `strict-service-regression.txt`, `strict-service-oracle.txt` et `strict-service-deployed-check.txt` dans `artifacts/atelier-crud-review`.

'''
s=s.replace('## Atelier CRUD',insert+'## Atelier CRUD',1);p.write_text(s,encoding='utf-8')
