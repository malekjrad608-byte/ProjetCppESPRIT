#include "mapwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QMouseEvent>

MapWidget::MapWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(900, 600);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
}

void MapWidget::setSalles(const QVector<Salle*>& s) {
    salles = s;
    calculerPositions();
    update();
}

// Couleur d'accent de chaque type de salle (bande à gauche de la carte)
QColor MapWidget::couleurPourType(const QString& type) const {
    if (type == "Consultation") return QColor("#4A8797");
    if (type == "Chirurgie")    return QColor("#D9695B");
    if (type == "Toilettage")   return QColor("#EA9941");
    if (type == "Pension")      return QColor("#5BA57A");
    if (type == "Adoption")     return QColor("#8E7CC3");
    if (type == "Storage")      return QColor("#98A6AB");
    return QColor("#BCBEC0");
}

void MapWidget::calculerPositions() {
    rectangles.clear();
    if (salles.isEmpty()) return;

    int x = 4, y = 4;
    int largeurSalle = 150;
    int hauteurSalle = 100;
    int espacement = 16;

    for (int i = 0; i < salles.size(); ++i) {
        Salle* s = salles[i];

        int largeurActuelle = largeurSalle;
        if (s->getType() == "Pension" || s->getType() == "Adoption") {
            largeurActuelle = largeurSalle * 2 + espacement;
        }

        if (x + largeurActuelle > width() - 4 && x > 4) {
            x = 4;
            y += hauteurSalle + espacement;
        }

        rectangles.append(QRect(x, y, largeurActuelle, hauteurSalle));
        x += largeurActuelle + espacement;
    }
}

void MapWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.fillRect(rect(), QColor("#F4F7F8"));

    calculerPositions();

    for (int i = 0; i < salles.size() && i < rectangles.size(); ++i) {
        Salle* s = salles[i];
        QRect r = rectangles[i];
        QColor accent = couleurPourType(s->getType());

        // --- Carte blanche ---
        painter.setPen(QPen(QColor("#E3E9EB"), 1));
        painter.setBrush(QColor("#FFFFFF"));
        painter.drawRoundedRect(r, 12, 12);

        // --- Bande de couleur à gauche (découpée aux coins arrondis) ---
        QPainterPath clip;
        clip.addRoundedRect(QRectF(r), 12, 12);
        painter.save();
        painter.setClipPath(clip);
        painter.fillRect(QRect(r.left(), r.top(), 6, r.height()), accent);
        painter.restore();

        // --- Ligne 1 : "S101 · Consultation" ---
        painter.setPen(QColor("#8A989E"));
        QFont f1("Segoe UI", 8);
        painter.setFont(f1);
        painter.drawText(QRect(r.left() + 18, r.top() + 10, r.width() - 30, 16),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QString("S%1 · %2").arg(s->getId()).arg(s->getType()));

        // --- Ligne 2 : nom de la salle ---
        painter.setPen(QColor("#2B4B57"));
        QFont f2("Segoe UI", 10, QFont::Bold);
        painter.setFont(f2);
        QFontMetrics fm(f2);
        QString nom = fm.elidedText(s->getNom(), Qt::ElideRight, r.width() - 30);
        painter.drawText(QRect(r.left() + 18, r.top() + 30, r.width() - 30, 22),
                         Qt::AlignLeft | Qt::AlignVCenter, nom);

        // --- Pastille d'état ---
        QString etat = s->getEtat();
        QString libelle;
        QColor fond, texte;
        if (etat == "Libre") {
            libelle = "Disponible";
            fond = QColor("#DDEEE3"); texte = QColor("#2E6B4F");
        } else if (etat == "Occupée") {
            libelle = "Occupé";
            fond = QColor("#E4E8F6"); texte = QColor("#3D4A8A");
        } else {
            libelle = "Hors service";
            fond = QColor("#FCE9CF"); texte = QColor("#9A5B12");
        }

        QFont f3("Segoe UI", 8, QFont::DemiBold);
        painter.setFont(f3);
        QFontMetrics fm3(f3);
        int largeurPill = fm3.horizontalAdvance(libelle) + 22;
        QRect rPill(r.left() + 18, r.bottom() - 30, largeurPill, 20);

        painter.setPen(Qt::NoPen);
        painter.setBrush(fond);
        painter.drawRoundedRect(rPill, 7, 7);
        painter.setPen(texte);
        painter.drawText(rPill, Qt::AlignCenter, libelle);

        // --- Occupation à droite : "1/1" ---
        if (s->getType() != "Storage") {
            painter.setPen(QColor("#6B7B82"));
            QFont f4("Segoe UI", 8);
            painter.setFont(f4);
            painter.drawText(QRect(r.left() + 18, r.bottom() - 30, r.width() - 32, 20),
                             Qt::AlignRight | Qt::AlignVCenter,
                             QString("%1/%2").arg(s->getOccupation()).arg(s->getCapacite()));
        }
    }
}

void MapWidget::mousePressEvent(QMouseEvent* event) {
    for (int i = 0; i < rectangles.size() && i < salles.size(); ++i) {
        if (rectangles[i].contains(event->pos())) {
            emit salleCliquee(salles[i]->getId());
            return;
        }
    }
}
