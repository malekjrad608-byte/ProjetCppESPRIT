#ifndef SALLECARD_H
#define SALLECARD_H

#include <QFrame>
#include <QString>

class Salle;

// ============================================================
// SalleCard : carte visuelle d'un local (grille "Locaux")
// ------------------------------------------------------------
// Ne garde que l'ID de la salle (pas de pointeur) : aucun
// risque de pointeur invalide après un chargement CSV.
// ============================================================
class SalleCard : public QFrame
{
    Q_OBJECT

public:
    explicit SalleCard(const Salle* salle, QWidget* parent = nullptr);

    int idSalle() const { return id; }
    void setSelectionnee(bool selectionnee);

    // Petits utilitaires réutilisés par le panneau de droite
    static QString cleEtat(const QString& etat);      // "dispo" / "occupe" / "hors"
    static QString libelleEtat(const QString& etat);  // "Disponible" / "Occupé" / "Hors service"
    static QString detailSalle(const Salle* salle);   // spécialité, bloc, contenu...

signals:
    void cliquee(int idSalle);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    int id;
};

#endif // SALLECARD_H
