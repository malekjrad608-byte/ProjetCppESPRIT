#include "stock.h"
#include <limits>

bool Stock::ajouter() {
    QVector<StockRecord> rows;qint64 revision;
    if(!afficher(&rows,&revision,&m_error))return false;
    for(const auto &r:rows)if(r.article==getArticle()){m_error="Référence article déjà utilisée.";return false;}
    m_record.idArticle=0;rows.append(m_record);
    if(!enregistrer(rows,&revision,&m_error) || !afficher(&rows,&revision,&m_error))return false;
    for(const auto &r:rows)if(r.article==getArticle()){m_record=r;return true;}
    m_error="Article enregistré mais relecture impossible.";return false;
}
bool Stock::afficher(QVector<StockRecord> *rows,qint64 *revision,QString *error) {
    return StockRecord::afficher(rows,revision,error);
}
bool Stock::supprimer(int id) {
    QVector<StockRecord> rows;qint64 revision;
    if(!afficher(&rows,&revision,&m_error))return false;
    for(int i=0;i<rows.size();++i)if(rows[i].idArticle==id){rows.removeAt(i);return enregistrer(rows,&revision,&m_error);}
    m_error="Article introuvable. Actualisez le stock.";return false;
}
bool Stock::modifier() {
    QVector<StockRecord> rows;qint64 revision;
    if(!afficher(&rows,&revision,&m_error))return false;
    for(auto &r:rows)if(r.idArticle==getIdArticle()){r=m_record;return enregistrer(rows,&revision,&m_error);}
    m_error="Article introuvable. Actualisez le stock.";return false;
}
bool Stock::enregistrer(const QVector<StockRecord> &rows,qint64 *revision,QString *error) {
    return StockRecord::enregistrer(rows,revision,error);
}
bool Stock::receptionner(const QVector<Reception> &lines,QVector<int> *ids,QString *error) {
    ids->clear();
    if(lines.isEmpty()){*error="La facture doit contenir au moins une ligne.";return false;}
    QVector<StockRecord> rows;qint64 revision;
    if(!afficher(&rows,&revision,error))return false;
    for(const auto &line:lines) {
        if(line.reference.trimmed().isEmpty() || line.nom.trimmed().isEmpty() || line.type.trimmed().isEmpty() ||
           line.quantite<=0 || line.quantite>999999 || line.localId<=0 || !line.echeance.isValid()) {
            *error="Vérifiez la référence, le nom, la quantité entière, le type, la date et le local de chaque ligne.";return false;
        }
        int found=-1;
        for(int i=0;i<rows.size();++i)if(rows[i].article==line.reference.trimmed()){found=i;break;}
        if(found>=0) {
            auto &r=rows[found];
            if(r.lot!=line.lot.trimmed() || r.localId!=line.localId || r.expiry!=line.echeance) {
                *error="Cette référence existe avec un autre lot, local ou date. Renseignez une référence distincte pour le nouveau lot : "+line.reference;return false;
            }
            if(r.quantity>999999-line.quantite){*error="Quantité maximale dépassée.";return false;}
            r.quantity+=line.quantite;
        } else {
            StockRecord r;r.article=line.reference.trimmed();r.nomArticle=line.nom.trimmed();r.lot=line.lot.trimmed();
            r.category=line.type;r.quantity=line.quantite;r.localId=line.localId;r.expiry=line.echeance;r.threshold=1;
            rows.append(r);
        }
    }
    if(!enregistrer(rows,&revision,error))return false;
    // Resolve the Oracle-generated IDs by their unique article references.
    if(!afficher(&rows,&revision,error))return false;
    for(const auto &line:lines)for(const auto &r:rows)if(r.article==line.reference.trimmed()){ids->append(r.idArticle);break;}
    return true;
}
