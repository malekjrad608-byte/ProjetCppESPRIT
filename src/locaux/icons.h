#pragma once
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QString>

// Icones vectorielles (SVG integres dans le code, aucune image externe)
namespace Icons {
QPixmap pixmap(const QString &name, const QColor &color, int size = 18);
QIcon icon(const QString &name, const QColor &color, int size = 18);
QPixmap paw(const QColor &color, int size = 18);
}
