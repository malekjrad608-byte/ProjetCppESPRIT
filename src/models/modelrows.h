#pragma once
#include <QList>
#include <QVariant>
#include <QString>

// Detached model data: the view can read rows, but cannot execute SQL.
struct ModelError {
    QString message, code;
    QString text() const { return message; }
    QString nativeErrorCode() const { return code; }
};
class ModelRows {
public:
    QList<QVariantList> rows;
    ModelError error;
    bool isOk() const { return error.message.isEmpty(); }
    bool next() { return ++m_position < rows.size(); }
    QVariant value(int column) const {
        return m_position >= 0 && m_position < rows.size() ? rows[m_position].value(column) : QVariant();
    }
    ModelError lastError() const { return error; }
private:
    int m_position = -1;
};
