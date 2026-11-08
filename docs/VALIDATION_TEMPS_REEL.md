# Validation dynamique (formulaire « Dossier médical »)

Nouveaux fichiers : `fieldvalidator.h/.cpp` (classe `ValidatedField`, ajoutée à CMakeLists.txt et dossier_medical.pro).
Modifiés : `medicalrecorddialog.h/.cpp` (champs enveloppés + champ ID dossier en lecture seule + contrôle à l'enregistrement).

| Champ | Obligatoire | Règle |
|---|---|---|
| ID dossier | non (lecture seule) | généré automatiquement ; en modification : MED-012 ou 12 |
| Réf. animal | non | ANI-012 ou 12 (colonne REF_ANIMAL = NUMBER), max 20 car. |
| Nom animal | oui | 2 à 100 car., lettres/chiffres/espace/- ' . |
| Espèce | oui | choix dans la liste |
| Race | oui | choix après l'espèce |
| Âge | non | 0 à 30 |
| Poids | non | 0 = non renseigné, sinon max 100 kg (stocké en entier par la base) |
| Propriétaire | oui | choix dans la liste, max 100 car. |
| Date consultation | oui | pas dans le futur, pas avant 1990 |
| Motif | oui | 3 à 200 car. (MOTIF = VARCHAR2(200)) |
| Diagnostic | non | 3 à 500 car. |
| Employé | oui | choix dans la liste (ID_EMPLOYES réel) |

Comportement : neutre tant que rien n'a été saisi ; vert + ✓ si valide ; rouge + ✕ + message sous le champ si invalide ;
revient au vert dès la correction. Champ facultatif vide = neutre. À « Enregistrer », tous les champs sont contrôlés et le
premier champ en erreur reçoit le focus.
