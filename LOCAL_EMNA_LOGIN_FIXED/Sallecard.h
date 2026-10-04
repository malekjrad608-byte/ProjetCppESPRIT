#ifndef SALLECARD_H
#define SALLECARD_H

#include <QFrame>
#include <QString>
#include <QVariantAnimation>

class Salle;
class QGraphicsDropShadowEffect;
class QEnterEvent;

// ============================================================
// SalleCard : carte visuelle d'un local (grille "Locaux")
// Survol : ombre douce qui grandit, transition animée.
// Ne garde que l'ID de la salle (pas de pointeur).
// ============================================================
class SalleCard : public QFrame
{
    Q_OBJECT

public:
    explicit SalleCard(const Salle* salle, QWidget* parent = nullptr);

    int idSalle() const { return id; }
    void setSelectionnee(bool selectionnee);

    static QString cleEtat(const QString& etat);      // "dispo" / "occupe" / "hors"
    static QString libelleEtat(const QString& etat);  // "Disponible" / "Occupé" / "Hors service"
    static QString detailSalle(const Salle* salle);   // spécialité, bloc, contenu...

signals:
    void cliquee(int idSalle);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void animerSurvol(qreal cible);
    void appliquerOmbre();

    int id;
    bool m_selectionnee = false;
    qreal m_survol = 0.0;
    QGraphicsDropShadowEffect* m_ombre = nullptr;
    QVariantAnimation m_anim;
};

#endif // SALLECARD_H
