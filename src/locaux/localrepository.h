#pragma once
#include <QList>
#include <QObject>
#include "local.h"

// Oracle in normal mode; JSON and memory remain available for offline/demo use.
class LocalRepository : public QObject
{
    Q_OBJECT
public:
    explicit LocalRepository(QObject *parent = nullptr, const QString &storagePath = ":memory:");
    QString lastError() const { return m_error; }
    void reload() { load(); emit changed(); }

    const QList<Local> &all() const { return m_items; }
    const Local *find(int id) const;
    bool nameExists(const QString &name, int ignoreId = 0) const;

    int  add(Local l);            // retourne l'id attribue
    bool update(const Local &l);
    bool remove(int id);
    void resetDemo();             // remet les donnees de demonstration

signals:
    void changed();
    void storageError(const QString &message);

private:
    QString filePath() const;
    void load();
    bool save();
    void seed();
    int nextId() const;

    QList<Local> m_items;
    QString m_path;
    QString m_error;
    bool m_loadFailed = false;
    qint64 m_revision = 0;
};
