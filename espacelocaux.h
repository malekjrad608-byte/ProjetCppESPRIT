#pragma once
#include <QWidget>

class LocalRepository;
class PageLocaux;
class QPushButton;

// Espace complet du module "Gestion des locaux" dans l'application de l'equipe :
//   bande du haut du module (fil d'Ariane, date, statut Oracle, animations, deconnexion)
//   + page Locaux + pied de page.
// La feuille de style du module est appliquee ici : elle ne touche pas les autres modules.
class EspaceLocaux : public QWidget
{
    Q_OBJECT
public:
    explicit EspaceLocaux(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *e) override;   // relit la table LOCAUX a chaque ouverture

private:
    QWidget *creerBande();
    QWidget *creerPied();
    void rafraichirBadge();

    LocalRepository *m_repo;
    QPushButton *m_badge;
};
