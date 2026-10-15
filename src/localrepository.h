#pragma once
#include <QList>
#include <QObject>
#include "local.h"

class QSqlQuery;

// Stockage des locaux (fichier JSON local).
// Plus tard, c'est ici seulement qu'on remplacera par SQLite/Oracle
// pour l'integration avec le reste du projet.
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
    void resetDemo();             // remet les donnees de demonstration

    bool usesDatabase() const { return m_useDb; }   // true = Oracle, false = fichier JSON (secours)

signals:
    void changed();

private:
    // --- Oracle ---
    bool loadFromDb();
    bool dbInsert(const Local &l);
    bool dbUpdate(const Local &l);
    bool dbDelete(int id);
    bool dbClear();
    int  dbNextId() const;
    static Local fromRow(QSqlQuery &q);
    static QString extraToJson(const Local &l);
    static QString statutOf(const Local &l);
    bool m_useDb = false;

    // --- secours JSON ---
    QString filePath() const;
    void load();
    void save() const;
    void seed();
    int nextId() const;

    QList<Local> m_items;
};
