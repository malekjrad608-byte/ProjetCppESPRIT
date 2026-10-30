#include "stock.h"
#include <QDebug>

Stock::Stock() {
    id_article = 0;
    quantite_disp = 0;
    seuil_min = 0;
    id_local = 1;
}

Stock::Stock(int id_article, QString article, QString nom_article, QString lot,
             int quantite_disp, QString units, int seuil_min, QDate date_cheance, int id_local) {
    this->id_article = id_article;
    this->article = article;
    this->nom_article = nom_article;
    this->lot = lot;
    this->quantite_disp = quantite_disp;
    this->units = units;
    this->seuil_min = seuil_min;
    this->date_cheance = date_cheance;
    this->id_local = id_local;
}

// Getters
int Stock::getIdArticle() const { return id_article; }
QString Stock::getArticle() const { return article; }
QString Stock::getNomArticle() const { return nom_article; }
QString Stock::getLot() const { return lot; }
int Stock::getQuantiteDisp() const { return quantite_disp; }
QString Stock::getUnits() const { return units; }
int Stock::getSeuilMin() const { return seuil_min; }
QDate Stock::getDateCheance() const { return date_cheance; }
int Stock::getIdLocal() const { return id_local; }
QString Stock::getError() const { return m_error; }

// Setters
void Stock::setIdArticle(int id) { this->id_article = id; }
void Stock::setArticle(QString article) { this->article = article; }
void Stock::setNomArticle(QString nom_article) { this->nom_article = nom_article; }
void Stock::setLot(QString lot) { this->lot = lot; }
void Stock::setQuantiteDisp(int qte) { this->quantite_disp = qte; }
void Stock::setUnits(QString units) { this->units = units; }
void Stock::setSeuilMin(int seuil) { this->seuil_min = seuil; }
void Stock::setDateCheance(QDate date) { this->date_cheance = date; }
void Stock::setIdLocal(int id_local) { this->id_local = id_local; }

// --- AJOUTER (Avec gestion robuste pour Oracle) ---
bool Stock::ajouter() {
    QSqlQuery queryMax;
    if (!queryMax.exec("SELECT NVL(MAX(ID_ARTICLE), 0) + 1 FROM STOCK")) {
        m_error = queryMax.lastError().text();
        qDebug() << "Erreur Max ID Stock :" << m_error;
        return false;
    }
    int nextId = 1;
    if (queryMax.next()) {
        nextId = queryMax.value(0).toInt();
    }

    QSqlQuery query;
    query.prepare("INSERT INTO STOCK (ID_ARTICLE, ARTICLE, NOM_ARTICLE, LOT, QUANTITE_DISP, UNITS, SEUIL_MIN, DATE_CHEANCE, ID_LOCAL) "
                  "VALUES (:id, :art, :nom, :lot, :qte, :units, :seuil, TO_DATE(:date_ch, 'YYYY-MM-DD'), :id_loc)");

    query.bindValue(":id", nextId);
    query.bindValue(":art", article);
    query.bindValue(":nom", nom_article);
    query.bindValue(":lot", lot);
    query.bindValue(":qte", quantite_disp);
    query.bindValue(":units", units);
    query.bindValue(":seuil", seuil_min);
    query.bindValue(":date_ch", date_cheance.toString("yyyy-MM-dd"));
    query.bindValue(":id_loc", id_local > 0 ? id_local : 1);

    if (query.exec()) {
        QSqlQuery commitQuery;
        commitQuery.exec("COMMIT");
        qDebug() << "Insertion réussie et commitée dans Oracle ! ID:" << nextId;
        return true;
    } else {
        m_error = query.lastError().text();
        qDebug() << "Erreur Ajout Stock Oracle :" << m_error;
        return false;
    }
}

// --- AFFICHER ---
QSqlQueryModel* Stock::afficher() {
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT ARTICLE, LOT, QUANTITE_DISP, SEUIL_MIN, DATE_CHEANCE FROM STOCK");

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ARTICLE"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("LOT"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("DISPONIBLE"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("SEUIL"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("ÉCHÉANCE"));

    return model;
}

// --- SUPPRIMER ---
bool Stock::supprimer(int id_art) {
    QSqlQuery query;
    query.prepare("DELETE FROM STOCK WHERE ID_ARTICLE = :id");
    query.bindValue(":id", id_art);

    if (query.exec()) {
        QSqlQuery commitQuery;
        commitQuery.exec("COMMIT");
        return true;
    } else {
        m_error = query.lastError().text();
        qDebug() << "Erreur Suppression Stock Oracle :" << m_error;
        return false;
    }
}

// --- MODIFIER ---
bool Stock::modifier() {
    QSqlQuery query;
    query.prepare("UPDATE STOCK SET ARTICLE = :art, NOM_ARTICLE = :nom, LOT = :lot, "
                  "QUANTITE_DISP = :qte, UNITS = :units, SEUIL_MIN = :seuil, "
                  "DATE_CHEANCE = TO_DATE(:date_ch, 'YYYY-MM-DD'), ID_LOCAL = :id_loc WHERE ID_ARTICLE = :id");

    query.bindValue(":id", id_article);
    query.bindValue(":art", article);
    query.bindValue(":nom", nom_article);
    query.bindValue(":lot", lot);
    query.bindValue(":qte", quantite_disp);
    query.bindValue(":units", units);
    query.bindValue(":seuil", seuil_min);
    query.bindValue(":date_ch", date_cheance.toString("yyyy-MM-dd"));
    query.bindValue(":id_loc", id_local);

    if (query.exec()) {
        QSqlQuery commitQuery;
        commitQuery.exec("COMMIT");
        return true;
    } else {
        m_error = query.lastError().text();
        qDebug() << "Erreur Modification Stock Oracle :" << m_error;
        return false;
    }
}