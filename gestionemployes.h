#ifndef GESTIONEMPLOYES_H
#define GESTIONEMPLOYES_H

#include <QWidget>
#include <QList>
#include "employe.h"

class QLineEdit;
class QComboBox;
class QLabel;
class QGridLayout;
class QPushButton;

class GestionEmployes : public QWidget
{
    Q_OBJECT
public:
    explicit GestionEmployes(QWidget *parent = nullptr);
    void actualiserStats();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void ajouterEmploye();
    void modifierEmploye();
    void supprimerEmploye();
    void basculerDisponibilite();
    void verifierAdequation();
    void verifierEquilibrage();
    void onFiltresChange();
    void ouvrirEvaluationEmployes();

private:
    // ---- Header stats pills ----
    QLabel *m_lblDisponibles;
    QLabel *m_lblIndisponibles;
    QLabel *m_lblChargeTotale;

    // ---- Barre recherche ----
    QLineEdit *m_editRecherche;
    QComboBox *m_comboPoste;
    QComboBox *m_comboStatut;
    QComboBox *m_comboTri;

    // ---- Grid cards ----
    QGridLayout *m_gridCards;
    QWidget *m_cardsContainer;

    // ---- Detail panel ----
    QLabel *m_detailPlaceholder;
    QWidget *m_detailContent;
    QLabel *m_detailNom;
    QLabel *m_detailId;
    QLabel *m_detailStatut;
    QLabel *m_detailCin;
    QLabel *m_detailNaissance;
    QLabel *m_detailAdresse;
    QLabel *m_detailTel;
    QLabel *m_detailCourriel;
    QLabel *m_detailPoste;
    QLabel *m_detailService;
    QLabel *m_detailContrat;
    QLabel *m_detailDate;
    QLabel *m_detailSalaire;
    QLabel *m_detailCharge;
    QLabel *m_detailDispo;
    QLabel *m_detailCompetences;

    int m_idSelectionne = -1;

    // ---- Etat du schema Oracle du module (voir Connection::schemaEmployesOk) ----
    QPushButton *m_btnNouveau = nullptr;
    QPushButton *m_btnEvaluation = nullptr;
    QPushButton *m_btnStats = nullptr;
    QLabel *m_bandeauSchema = nullptr;
    bool m_schemaOk = true;

    // ---- Méthodes ----
    void construireUI();
    void appliquerEtatSchema();
    QWidget *construireHeader();
    QWidget *construireStatsPills();
    QWidget *construireBarreRecherche();
    QWidget *construireGrid();
    QWidget *construireDetailPanel();
    QWidget *creerCard(const Employe &e);

    void chargerEmployes(const QString &filtre = "", const QString &tri = "date",
                         const QString &poste = "Tous les postes",
                         const QString &statut = "Tous");
    void afficherCards(const QList<Employe> &liste);
    void afficherDetail(const Employe &e);
    void effacerDetail();
    void mettreAJourStats();
    bool ouvrirDialog(Employe &e, bool modification);
};

#endif