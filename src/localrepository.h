#pragma once
#include <QList>
#include <QObject>
#include "local.h"

// Garde en memoire la liste des locaux lue depuis Oracle (table LOCAUX)
// et previent l'interface (signal changed) apres chaque ajout / modification / suppression.
// Aucune requete SQL ici : elles sont toutes dans la classe Local.
class LocalRepository : public QObject
{
    Q_OBJECT
public:
    explicit LocalRepository(QObject *parent = nullptr);

    const QList<Local> &all() const { return m_items; }
    const Local *find(int id) const;
    bool nameExists(const QString &name, int ignoreId = 0) const;

    int  add(Local l);            // retourne l'id attribue
    bool update(const Local &l);
    bool remove(int id);

    bool reconnect();                 // retente la connexion Oracle
    QString lastDbError() const;      // derniere erreur Oracle (pour l'afficher)
    bool usesDatabase() const;        // true si la connexion Oracle est ouverte

signals:
    void changed();

private:
    bool charger();       // relit la table LOCAUX (Local::afficher)
    void seed();          // premiers locaux si la table est vide
    int nextId() const;

    QList<Local> m_items;
};
