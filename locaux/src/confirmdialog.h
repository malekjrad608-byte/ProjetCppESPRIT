#pragma once
#include <QString>

class QWidget;

namespace Confirm {

// Fenetre de confirmation au style ANIWORLD. Retourne true si l'utilisateur confirme.
bool ask(QWidget *parent, const QString &title, const QString &text,
         const QString &okText, bool danger = true);

}
