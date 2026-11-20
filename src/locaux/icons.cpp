#include "icons.h"
#include "theme.h"

#include <QGuiApplication>
#include <QHash>
#include <QPainter>
#include <QSvgRenderer>

namespace {

QString body(const QString &name)
{
    static const QHash<QString, QString> map = {
        { "rooms",     "<rect x='3' y='3' width='7' height='7' rx='1'/><rect x='14' y='3' width='7' height='7' rx='1'/><rect x='3' y='14' width='7' height='7' rx='1'/><rect x='14' y='14' width='7' height='7' rx='1'/>" },
        { "services",  "<rect x='5' y='4' width='14' height='17' rx='2'/><path d='M9 2h6v4H9zM9 11h6M9 15h6'/>" },
        { "people",    "<circle cx='9' cy='8' r='3.5'/><path d='M2 20c0-4 3-6 7-6s7 2 7 6M17 5a3.5 3.5 0 010 7M22 20c0-3-1.5-5-4-5.7'/>" },
        { "stock",     "<path d='M12 3l8 4v10l-8 4-8-4V7zM4 7l8 4 8-4M12 11v10'/>" },
        { "records",   "<path d='M12 3l8 3v6c0 5-3.5 8-8 9-4.5-1-8-4-8-9V6z'/><path d='M9 12l2 2 4-4'/>" },
        { "isolation", "<path d='M12 3l8 3v6c0 5-3.5 8-8 9-4.5-1-8-4-8-9V6z'/>" },
        { "consult",   "<path d='M6 3v6a4 4 0 008 0V3M10 13v2a5 5 0 0010 0v-1'/><circle cx='20' cy='12' r='2'/>" },
        { "surgery",   "<path d='M12 3v18M3 12h18'/>" },
        { "bed",       "<path d='M3 18V8M3 14h18v4M21 14v-3a3 3 0 00-3-3h-7v6'/>" },
        { "home",      "<path d='M3 11l9-7 9 7v9H3zM9 20v-6h6v6'/>" },
        { "flask",     "<path d='M9 3h6M10 3v6l-5 9a2 2 0 002 3h10a2 2 0 002-3l-5-9V3'/>" },
        { "scan",      "<rect x='3' y='5' width='18' height='14' rx='2'/><circle cx='12' cy='12' r='3'/>" },
        { "search",    "<circle cx='11' cy='11' r='7'/><path d='M21 21l-4-4'/>" },
        { "plus",      "<path d='M12 5v14M5 12h14'/>" },
        { "download",  "<path d='M12 3v12M7 10l5 5 5-5M4 21h16'/>" },
        { "map",       "<path d='M9 4L3 6v14l6-2 6 2 6-2V4l-6 2zM9 4v14M15 6v14'/>" },
        { "chart",     "<path d='M4 20V10M10 20V4M16 20v-8M22 20H2'/>" },
        { "edit",      "<path d='M4 20h4L19 9l-4-4L4 16zM13 7l4 4'/>" },
        { "trash",     "<path d='M4 7h16M9 7V4h6v3M6 7l1 13h10l1-13M10 11v6M14 11v6'/>" },
        { "logout",    "<path d='M10 4H5v16h5M16 8l4 4-4 4M20 12H9'/>" },
        { "sparkle",   "<path d='M12 3l2 6 6 2-6 2-2 6-2-6-6-2 6-2z'/>" },
        { "alert",     "<path d='M12 3l10 18H2zM12 10v5M12 18v.5'/>" },
        { "bell",      "<path d='M6 17V11a6 6 0 0112 0v6l2 2H4zM10 21h4'/>" },
        { "check",     "<path d='M5 12l5 5 9-10'/>" },
        { "brain",     "<path d='M9 4a3 3 0 00-3 3 3 3 0 00-2 5 3 3 0 002 5 3 3 0 006 0V4a3 3 0 00-3 0zM15 4a3 3 0 013 3 3 3 0 012 5 3 3 0 01-2 5 3 3 0 01-6 0'/>" },
        { "list",      "<path d='M8 6h13M8 12h13M8 18h13M3 6h.5M3 12h.5M3 18h.5'/>" },
        { "grid",      "<rect x='3' y='3' width='7' height='7' rx='1'/><rect x='14' y='3' width='7' height='7' rx='1'/><rect x='3' y='14' width='7' height='7' rx='1'/><rect x='14' y='14' width='7' height='7' rx='1'/>" },
    };
    return map.value(name, map.value("rooms"));
}

} // namespace

namespace Icons {

QPixmap pixmap(const QString &name, const QColor &color, int size)
{
    // petit cache : les cartes redessinent leur icone a chaque image d'animation
    static QHash<QString, QPixmap> cache;
    const QString key = name + color.name(QColor::HexArgb) + QString::number(size);
    const auto it = cache.constFind(key);
    if (it != cache.constEnd())
        return it.value();

    const QString svg = QStringLiteral(
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' "
        "stroke='%1' stroke-width='1.8' stroke-linecap='round' stroke-linejoin='round'>%2</svg>")
        .arg(color.name(), body(name));

    const qreal dpr = qApp ? qApp->devicePixelRatio() : 1.0;
    QPixmap pm(int(size * dpr), int(size * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QSvgRenderer renderer(svg.toUtf8());
    QPainter p(&pm);
    renderer.render(&p, QRectF(0, 0, size, size));
    p.end();
    cache.insert(key, pm);
    return pm;
}

QIcon icon(const QString &name, const QColor &color, int size)
{
    return QIcon(pixmap(name, color, size));
}

QPixmap paw(const QColor &color, int size)
{
    const qreal dpr = qApp ? qApp->devicePixelRatio() : 1.0;
    QPixmap pm(int(size * dpr), int(size * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(Theme::pawPath(QRectF(0, 0, size, size)), color);
    return pm;
}

} // namespace Icons
