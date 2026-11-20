#ifndef LOOKUPDATA_H
#define LOOKUPDATA_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>

// Données de référence (listes de choix) utilisées par le formulaire "Dossier médical".
// Toutes les requêtes SQL sont ici (couche Modèle) : la vue (dialog) ne fait aucune requête.
//
// ATTENTION : les noms de tables / colonnes ci-dessous sont à adapter à ceux des
// modules des collègues (voir les constantes en haut de lookupdata.cpp).
// Si la requête échoue (table absente, mode hors ligne), des données d'exemple sont utilisées.

struct LookupItem
{
    QString id;      // valeur enregistrée dans la base (ex: EMP-001)
    QString label;   // texte affiché à l'utilisateur (ex: Ben Ali Sami)
    int quantity = -1;   // utilisé pour le stock (-1 = non applicable)
};

class LookupData
{
public:
    // Employés du module "Gestion employés" (id + nom complet)
    static QList<LookupItem> employes();

    // Propriétaires (clients)
    static QList<LookupItem> proprietaires();

    // Traitements / produits disponibles au centre (stock > 0)
    static QList<LookupItem> traitementsEnStock();

    // Téléphone d'un propriétaire (pour les SMS de rappel vaccinal). "" si inconnu.
    // L'argument est l'identifiant OU le nom complet, comme enregistré dans MEDICAL.PROPRIETAIRE.
    static QString telephoneProprietaire(const QString &proprietaire);

    // Espèce -> liste des races proposées
    static QMap<QString, QStringList> racesParEspece();
};

#endif // LOOKUPDATA_H
