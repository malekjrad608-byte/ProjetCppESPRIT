# ANIWORLD - Gestion des dossiers médicaux

Application de gestion vétérinaire développée en Qt/C++ avec Qt Widgets.

## Structure du projet

```
ANIWORLD/
├── CMakeLists.txt
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── mainwindow.ui          <- interface (Qt Designer, sans layouts ni spacers)
├── medicalrecord.h
├── medicalrecord.cpp
├── medicalrecorddialog.h
├── medicalrecorddialog.cpp
├── medicalrecorddetailsdialog.h
├── medicalrecorddetailsdialog.cpp
├── resources.qrc
├── styles/
│   └── style.qss
└── assets/
    └── icons/
```

## Prérequis

- Qt 6.5 ou supérieur
- CMake 3.19 ou supérieur
- C++ compilateur (MSVC, GCC, ou Clang)

## Instructions pour Qt Creator

### 1. Ouvrir le projet

1. Lancez Qt Creator
2. Cliquez sur **File** → **Open File or Project**
3. Sélectionnez le fichier `CMakeLists.txt` à la racine du projet
4. Cliquez sur **Open**

### 2. Configurer le Kit Qt

1. Dans la fenêtre de configuration de projet, sélectionnez un Kit Qt approprié
2. Assurez-vous que le Kit inclut:
   - Qt 6.5 ou supérieur
   - CMake
   - Un compilateur C++

### 3. Configurer CMake

1. Cliquez sur le bouton **Configure Project**
2. Qt Creator va automatiquement configurer CMake
3. Attendez que la configuration soit terminée

### 4. Compiler le projet

1. Cliquez sur le bouton **Build** (marteau) dans la barre d'outils
   - Ou appuyez sur `Ctrl+B`
2. Attendez que la compilation soit terminée
3. Vérifiez qu'il n'y a pas d'erreurs dans la fenêtre de sortie

### 5. Exécuter l'application

1. Cliquez sur le bouton **Run** (triangle vert) dans la barre d'outils
   - Ou appuyez sur `Ctrl+R`
2. L'application ANIWORLD devrait s'ouvrir

## Fonctionnalités

### Navigation
- **Tableau de bord** (non implémenté)
- **Services** (non implémenté)
- **Employés** (non implémenté)
- **Stock** (non implémenté)
- **Locaux** (non implémenté)
- **Dossiers médicaux** (module actif)

### Gestion des dossiers médicaux

#### Statistiques
- Nombre total de dossiers médicaux
- Nombre de consultations
- Vaccinations à jour
- Vaccinations à renouveler

#### Actions
- **Ajouter dossier**: Créer un nouveau dossier médical
- **Consulter**: Voir les détails d'un dossier
- **Modifier**: Modifier un dossier existant
- **Supprimer**: Supprimer un dossier (avec confirmation)
- **Actualiser**: Rafraîchir la liste

#### Recherche et filtres
- Recherche par: ID dossier, ID animal, nom, espèce, race, propriétaire, diagnostic
- Filtre par espèce: Toutes, Chien, Chat, Lapin, Oiseau
- Filtre par sexe: Tous, Mâle, Femelle
- Filtre par statut vaccination: Tous, À jour, À renouveler
- Bouton Réinitialiser pour effacer tous les filtres

#### Tableau des dossiers
- 14 colonnes avec toutes les informations
- Tri automatique
- Alternance des couleurs des lignes
- Boutons d'action dans chaque ligne

#### Détails du dossier
- Informations de l'animal
- Informations de consultation
- Informations de santé
- Historique médical (fictif)

## Données

Les données viennent de la table **MEDICAL** (Oracle, via ODBC). Architecture Modèle-Vue de l'atelier CRUD :

| Couche | Fichiers | Rôle |
|---|---|---|
| Modèle | medicalrecord.h, medicalrecord.cpp | Gère les données et les requêtes (ajouter, afficher, modifier, supprimer) |
| Vue | mainwindow.ui, mainwindow.h, mainwindow.cpp | Affiche les données, gère l'interface et les interactions |

- Attributs `private`, méthodes `public` dans la classe `MedicalRecord`.
- Toutes les requêtes sont des **requêtes préparées** (`prepare()` + `bindValue()` + `exec()`) dans `medicalrecord.cpp` : protection contre l'injection SQL.
- Aucune requête derrière un bouton : `mainwindow.cpp` appelle `M.ajouter()`, `Mtmp.supprimer(id)`, `M.modifier()` et `Mtmp.afficher()`, puis actualise l'affichage.
- Une seule connexion à la BD (Singleton `Connection`).

Si la connexion échoue, l'application démarre en mode hors ligne avec des données d'exemple en mémoire.

## Données initiales

L'application démarre avec 8 dossiers médicaux fictifs:
1. Max - Chien (Golden Retriever)
2. Luna - Chat (Siamois)
3. Rocky - Chien (Bulldog)
4. Mimi - Chat (Européen)
5. Bella - Lapin (Lop)
6. Simba - Chat (Maine Coon)
7. Toby - Chien (Labrador)
8. Coco - Oiseau (Perroquet)

## Styles

L'application utilise un fichier QSS (`styles/style.qss`) pour le style professionnel.

Le fichier est chargé via le système de ressources Qt (`resources.qrc`).

## Compilation manuelle

Si vous préférez compiler manuellement sans Qt Creator:

```bash
# Créer le dossier de build
mkdir build
cd build

# Configurer CMake
cmake ..

# Compiler
cmake --build .

# Exécuter (Windows)
./ANIWORLD.exe

# Exécuter (Linux/Mac)
./ANIWORLD
```

## Résolution des problèmes

### Erreur: Qt6 non trouvé
- Vérifiez que Qt est installé
- Dans Qt Creator, configurez le Kit pour pointer vers votre installation Qt

### Erreur: CMake non trouvé
- Installez CMake depuis https://cmake.org/download/
- Ajoutez CMake à votre PATH

### Styles non appliqués
- Vérifiez que le fichier `resources.qrc` est dans le CMakeLists.txt
- Vérifiez que le fichier `styles/style.qss` existe

## Notes techniques

- Architecture: Qt Widgets (pas de QML)
- Langage: C++17
- Framework: Qt 6.5+
- Build system: CMake
- Style: QSS (Qt Style Sheets)
- Données: table MEDICAL (Oracle / QODBC), requêtes préparées

## Auteur

Projet académique pour le module "Gestion des dossiers médicaux" de l'application ANIWORLD.
