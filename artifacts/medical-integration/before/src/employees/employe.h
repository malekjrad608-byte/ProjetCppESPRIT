#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QString>
#include <QDate>

class Employe
{
public:
    Employe();

    int getId() const { return m_id; }
    QString getNom() const { return m_nom; }
    QString getPrenom() const { return m_prenom; }
    QString getCourriel() const { return m_courriel; }
    QString getTelephone() const { return m_telephone; }
    QString getPoste() const { return m_poste; }
    QDate getDateEmbauche() const { return m_dateEmbauche; }
    bool isDisponible() const { return m_disponible; }
    double getChargeHebdo() const { return m_chargeHebdo; }
    double getDispoHebdo() const { return m_dispoHebdo; }
    QString getCompetences() const { return m_competences; }

    void setId(int id) { m_id = id; }
    void setNom(const QString &n) { m_nom = n; }
    void setPrenom(const QString &p) { m_prenom = p; }
    void setCourriel(const QString &c) { m_courriel = c; }
    void setTelephone(const QString &t) { m_telephone = t; }
    void setPoste(const QString &p) { m_poste = p; }
    void setDateEmbauche(const QDate &d) { m_dateEmbauche = d; }
    void setDisponible(bool d) { m_disponible = d; }
    void setChargeHebdo(double c) { m_chargeHebdo = c; }
    void setDispoHebdo(double d) { m_dispoHebdo = d; }
    void setCompetences(const QString &c) { m_competences = c; }

    double tauxCharge() const;
    bool depasseSeuil(double seuil = 0.8) const;

private:
    int m_id = 0;
    QString m_nom;
    QString m_prenom;
    QString m_courriel;
    QString m_telephone;
    QString m_poste;
    QDate m_dateEmbauche;
    bool m_disponible = true;
    double m_chargeHebdo = 0.0;
    double m_dispoHebdo = 35.0;
    QString m_competences;
};

#endif