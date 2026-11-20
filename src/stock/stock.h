#pragma once
#include "stockrecord.h"

// === MODULE STOCK : classe metier, partage la connexion du projet ===
class Stock {
private:
    StockRecord m_record;
    QString m_error;
public:
    Stock() = default;
    explicit Stock(const StockRecord &record) : m_record(record) {}
    int getIdArticle() const { return m_record.idArticle; }
    QString getArticle() const { return m_record.article; }
    QString getNomArticle() const { return m_record.nomArticle; }
    QString getLot() const { return m_record.lot; }
    int getQuantiteDisp() const { return m_record.quantity; }
    QString getTypeArticle() const { return m_record.category; }
    int getSeuilMin() const { return m_record.threshold; }
    QDate getDateEcheance() const { return m_record.expiry; }
    int getIdLocal() const { return m_record.localId; }
    QString getError() const { return m_error; }
    void setArticle(const QString &v) { m_record.article=v; }
    void setNomArticle(const QString &v) { m_record.nomArticle=v; }
    void setLot(const QString &v) { m_record.lot=v; }
    void setQuantiteDisp(int v) { m_record.quantity=v; }
    void setTypeArticle(const QString &v) { m_record.category=v; }
    void setSeuilMin(int v) { m_record.threshold=v; }
    void setDateEcheance(const QDate &v) { m_record.expiry=v; }
    void setIdLocal(int v) { m_record.localId=v; }
    bool ajouter();
    static bool afficher(QVector<StockRecord> *rows,qint64 *revision,QString *error);
    bool supprimer(int id);
    bool modifier();
    static bool enregistrer(const QVector<StockRecord> &rows,qint64 *revision,QString *error);
    struct Reception { QString reference,nom,lot,type; int quantite=0,localId=0; QDate echeance; };
    // All lines are saved in one transaction; existing article/lot/local must agree.
    static bool receptionner(const QVector<Reception> &lines,QVector<int> *ids,QString *error);
};
