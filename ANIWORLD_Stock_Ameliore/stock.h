#ifndef STOCK_H
#define STOCK_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDate>

class Stock {
private:
    int id_article;
    QString article;
    QString nom_article;
    QString lot;
    int quantite_disp;
    QString units;
    int seuil_min;
    QDate date_cheance;
    int id_local;
    QString m_error;

public:
    // Constructeurs
    Stock();
    Stock(int id_article, QString article, QString nom_article, QString lot,
          int quantite_disp, QString units, int seuil_min, QDate date_cheance, int id_local);

    // Getters
    int getIdArticle() const;
    QString getArticle() const;
    QString getNomArticle() const;
    QString getLot() const;
    int getQuantiteDisp() const;
    QString getUnits() const;
    int getSeuilMin() const;
    QDate getDateCheance() const;
    int getIdLocal() const;
    QString getError() const;

    // Setters
    void setIdArticle(int id);
    void setArticle(QString article);
    void setNomArticle(QString nom_article);
    void setLot(QString lot);
    void setQuantiteDisp(int qte);
    void setUnits(QString units);
    void setSeuilMin(int seuil);
    void setDateCheance(QDate date);
    void setIdLocal(int id_local);

    // CRUD Methods
    bool ajouter();
    QSqlQueryModel* afficher();
    bool supprimer(int id_article);
    bool modifier();
};

#endif // STOCK_H