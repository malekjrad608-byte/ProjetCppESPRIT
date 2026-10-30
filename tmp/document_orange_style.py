from pathlib import Path
p=Path('README.md'); s=p.read_text(encoding='utf-8'); s+='''
### Style de référence et survol orange

Le formulaire Service utilise une colonne de libellés et une colonne de champs. Le panneau sélectionné présente les actions pleine largeur : Modifier (turquoise), démarrer/terminer (vert), Supprimer (rouge), avec les documents au-dessus. Le Stock reprend les mêmes boutons et couleurs.

Les boutons, cartes, panneaux et champs affichent un halo orange progressif au survol (160–180 ms), avec retour doux à la sortie. Le focus clavier des boutons et champs bénéficie aussi de ce repère. Les effets ne changent ni la disposition ni les zones cliquables ; le bouton Animations les désactive. Les lignes de tableau, menus et barres de défilement utilisent également une couleur de survol orange.

Vérification : suite de régression réussie, puis tests ciblés du survol, de la saisie, des formulaires, du CRUD et du Stock réussis (`build_tests/orange-results.txt`, `build_tests/orange-final.txt`). Captures : `artifacts/reference-orange/`, dont `orange-hover.png`.
'''; p.write_text(s,encoding='utf-8')
