from pathlib import Path
p=Path('resources/theme.qss'); s=p.read_text(encoding='utf-8')
s=s.replace("font-size: 13px", "font-size: 12px").replace('font-size: 27px','font-size: 23px').replace('font-size: 16px','font-size: 15px')
s=s.replace('background: qlineargradient(x1:0,y1:0,x2:0.8,y2:1,stop:0 #F4FAF8,stop:0.45 #E8F3F1,stop:1 #D8EBEA); border-right: 1px solid #C8DDDF;', 'background: #FFFFFF; border-right: 1px solid #E1EBEF;')
s=s.replace('background: rgba(255,255,255,185); border: 1px solid #CDDFDF;', 'background: #E3F0F3; border: none;')
s=s.replace('border-radius: 14px','border-radius: 8px').replace('padding: 10px 16px','padding: 8px 14px').replace('padding: 9px 11px','padding: 7px 10px')
s=s.replace('stop:0 #408A98,stop:1 #2E7182','stop:0 #418997,stop:1 #225C68').replace('background: #F4F8FA; color: #87928A;','background: #3C8190; color: #FFFFFF;')
s=s.replace('background: #CDDAD0','background: #D0E5EA')
p.write_text(s,encoding='utf-8')
p=Path('src/widgets/servicecard.cpp'); s=p.read_text(encoding='utf-8')
s=s.replace('setFixedHeight(184)','setFixedHeight(144)').replace('12,12);\n    const QString categoryIcon','7,7);\n    const QString categoryIcon')
s=s.replace('isChecked() ? 2 : 1','isChecked() ? 1.5 : 1').replace('QRect(18,17,31,31)','QRect(16,14,25,25)').replace('QRect(25-(iconSize-17)/2,24-(iconSize-17)/2','QRect(20-(iconSize-17)/2,18-(iconSize-17)/2')
s=s.replace('badge(width()-113,21,94,25)','badge(width()-108,14,92,21)').replace('f.setPixelSize(16)','f.setPixelSize(14)').replace('QRect(19,60,width()-38,25)','QRect(17,48,width()-34,22)').replace('QRect(19,89,width()-38,20)','QRect(17,74,width()-34,18)').replace('QRect(19,115,width()-38,18)','QRect(17,95,width()-34,16)')
s=s.replace('p.setPen(QColor("#E8EEF0")); p.drawLine(19,143,width()-19,143);','')
s=s.replace('QRect(19,156,13,13)','QRect(17,121,12,12)').replace('QRect(39,152,140,23)','QRect(35,117,140,20)').replace('QRect(width()-111,152,91,23)','QRect(width()-111,117,94,20)')
p.write_text(s,encoding='utf-8')
# Stock: same palette, compact KPIs, PDF only.
p=Path('src/stock/stockwidget.h'); s=p.read_text(encoding='utf-8').replace('QString inventoryCsv() const;', 'bool exportInventoryPdf(const QString &path, QString *error=nullptr) const;'); p.write_text(s,encoding='utf-8')
p=Path('src/stock/stockwidget.cpp'); s=p.read_text(encoding='utf-8').replace('#include "../ui/design.h"','#include "../ui/design.h"\n#include "../ui/invoice.h"')
a=s.index('QString csvCell('); b=s.index('\n}\n\nclass StockFilterModel',a); s=s[:a]+s[b:]
s=s.replace('background-color: #E9F2F4','background-color: #3C8190').replace('color: #397F8D;"','color: #FFFFFF;"').replace('Exporter CSV','Exporter PDF').replace('setContentsMargins(28, 24, 28, 19)','setContentsMargins(24, 20, 24, 16)').replace('font-size: 28px','font-size: 24px')
s=s.replace('auto *card=Design::card();\n        QVBoxLayout', '''auto *card=Design::card();
        card->setStyleSheet(QString("QFrame[role=card] { background:%1; border:none; border-radius:8px; }").arg(QStringList{"#E4F1F4","#FFF2E0","#FBE9E8"}.at(i)));
        QVBoxLayout''')
a=s.index('QString StockWidget::inventoryCsv()'); s=s[:a]+'''bool StockWidget::exportInventoryPdf(const QString &path, QString *error) const {
    QString html="<html><body style='font-family:Segoe UI; color:#34545e; font-size:9pt;'><h1 style='color:#225b66;'>ANIWORLD - Inventaire du stock</h1>";
    html+="<p>DÉMONSTRATION - DONNÉES FICTIVES<br/>Édité le "+QDate::currentDate().toString("dd/MM/yyyy")+"<br/>"+Invoice::escape(categoryFilter->currentText())+" · Recherche : "+Invoice::escape(searchLineEdit->text())+"</p>";
    html+="<table width='100%' cellspacing='0' cellpadding='7'><thead><tr bgcolor='#367e8b'>";
    for(int col=0;col<tableModel->columnCount();++col) html+="<th style='color:white;'>"+Invoice::escape(tableModel->headerData(col,Qt::Horizontal).toString())+"</th>";
    html+="</tr></thead>";
    for(int row=0;row<filterModel->rowCount();++row) {
        html+=row%2?"<tr bgcolor='#edf5f6'>":"<tr>";
        for(int col=0;col<tableModel->columnCount();++col) html+="<td>"+Invoice::escape(filterModel->index(row,col).data().toString())+"</td>";
        html+="</tr>";
    }
    html+="</table><p>"+QString::number(filterModel->rowCount())+" article(s) affiché(s).</p></body></html>";
    return Invoice::writeDocument(path,html,"Inventaire du stock",error);
}
void StockWidget::exportInventory() {
    QString path=QFileDialog::getSaveFileName(this,"Exporter les articles affichés", "ANIWORLD-stock-demo.pdf","Document PDF (*.pdf)");
    if(path.isEmpty()) return;
    if(!path.endsWith(".pdf",Qt::CaseInsensitive)) path+=".pdf";
    QString error;
    if(!exportInventoryPdf(path,&error)) { QMessageBox::warning(this,"Export impossible",error); return; }
    notice->setText("Inventaire affiché exporté en PDF · démo");
}
'''
p.write_text(s,encoding='utf-8')
