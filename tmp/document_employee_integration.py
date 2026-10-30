from pathlib import Path
p=Path('README.md');s=p.read_text(encoding='utf-8')
s=s.replace('# ANIWORLD — Services, facturation PDF et Stock','# ANIWORLD — Services, Employés, Stock et comptes par module')
s=s.replace('Les services et leurs références de facturation conservent leur base SQLite locale ; le module Stock fourni est intégré comme interface de démonstration.','Les services, factures, employés, affectations et comptes utilisent une base SQLite locale ; le module Stock fourni reste une interface de démonstration.')
s=s.replace('- La page de connexion issue de `petsssss` s\'ouvre en premier. Compte de prototype : identifiant **admin**, mot de passe **admin123**. Une connexion réussie ouvre **Services** ; **Se déconnecter** ramène à la connexion sans effacer les prestations.','- La page de connexion s\'ouvre en premier. Chaque compte ouvre directement son module ; **Se déconnecter** permet de changer de compte sans effacer les données.')
s=s.replace('- Cliquer sur **Stock**, désormais actif dans **Navigation**, pour ouvrir l\'inventaire dans la même fenêtre. Services et Tableau de bord restent accessibles.','- Le compte **admin** accède à tous les modules. Les comptes **services**, **stock** et **employes** n\'affichent que leur espace autorisé (avec le tableau de bord pour Services).')
s=s.replace('Une seule fenêtre Services est créée','Une seule fenêtre de gestion est créée')
a=s.index("L'authentification reste le **prototype local")
b=s.index('\n\n## Stock intégré',a)
s=s[:a]+'''Les comptes sont locaux à l'application. Les mots de passe sont conservés sous forme de dérivations PBKDF2-HMAC-SHA256 avec sel aléatoire individuel, jamais en clair dans la base. Les contrôles de navigation et les raccourcis respectent le module du compte connecté. **Mot de passe** permet de changer son mot de passe ; ce changement persiste après fermeture. La simulation de récupération par email/SMS est réservée au mode `--demo`, en mémoire. En mode normal, aucune récupération fictive ne peut créer un compte.

| Espace | Identifiant initial | Mot de passe initial | Ouverture |
|---|---|---|---|
| Services | `services` | `services123` | Services |
| Stock | `stock` | `stock123` | Stock |
| Employés | `employes` | `employes123` | Employés |
| Administration | `admin` | `admin123` | Services, navigation complète |

Les comptes initiaux sont créés une seule fois. Les données appartiennent au centre, avec une séparation d'accès par module dans l'interface ; cette application locale n'est pas une isolation des fichiers Windows entre utilisateurs.

## Employés intégrés

Source : `C:/Users/malek/Downloads/Employessanslogin/Employes/ANIWORLD2`, laissée intacte. Le module est adapté dans `src/employees/`. Les 6 employés et le planning présents dans sa base sont intégrés une seule fois au premier lancement grâce à une copie SQLite cohérente embarquée. Les modifications et suppressions ultérieures sont conservées ; l'import n'est pas rejoué sur un annuaire déjà importé. Sauvegarde préalable de la base Services et des sources : `backups/before-employees-accounts-20261011/`.

Fonctions reprises : ajout, modification, suppression, disponibilité, recherche, filtres, tri par embauche ou charge, statistiques hebdomadaires, notifications de surcharge, planification avec affectation automatique selon compétences/capacité/créneau et détection des conflits. L'attestation de congé utilise le même export PDF atomique que les autres documents. Les employés et leurs affectations sont enregistrés dans `services.sqlite`, dans des tables séparées des consultations.

L'annuaire, Services et Stock partagent la barre latérale, le fond illustré, les icônes vectorielles, les boutons turquoise/vert/rouge, les cartes sélectionnées en orange et les animations désactivables. Les actions du panneau Employé restent accessibles en bas pendant le défilement des détails. Les anciennes sources `src/rh/` restent hors compilation.
''' +s[b:]
s=s.replace('0 = Tableau de bord, 1 = Services, 2 = Stock. Les boutons de navigation s\'appellent `navDashboard`, `navServices`, `navStock`.','0 = Tableau de bord, 1 = Services, 2 = Stock, 3 = Employés. Les boutons de navigation s\'appellent `navDashboard`, `navServices`, `navStock`, `navEmployees`.')
s=s.replace('La duplication de service et le module RH sont retirés de la navigation et de la compilation. Les anciennes sources RH et leur base externe restent conservées hors du programme.','La duplication de service reste supprimée. L\'ancien module RH reste hors compilation ; il est remplacé par le module Employés demandé le 11 octobre 2026.')
s=s.replace('**Ctrl+F** recherche dans le module actif ; **Ctrl+N** ouvre le mouvement de Stock depuis Stock, ou la création d\'un service depuis Services/Tableau de bord.','**Ctrl+F** recherche dans le module actif ; **Ctrl+N** ouvre un mouvement depuis Stock, un employé depuis Employés ou un service depuis Services/Tableau de bord.')
p.write_text(s,encoding='utf-8')
