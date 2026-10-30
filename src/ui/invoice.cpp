#include "invoice.h"
#include "design.h"
#include "../database/oracleconnection.h"
#include "../models/invoicemodel.h"

#include <QBuffer>
#include <QPdfWriter>
#include <QSaveFile>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QPageSize>
#include <cmath>

QString Invoice::escape(const QString &text) {
    // Permit wrapping even for long references without spaces; never interpret input as HTML.
    QString wrapped;
    int run=0;
    for (QChar ch:text) {
        wrapped+=QString(ch).toHtmlEscaped();
        if (ch.isSpace()) run=0;
        else if (++run==28) { wrapped+="&#8203;"; run=0; }
    }
    wrapped.replace('\n',"<br/>");
    return wrapped;
}

QString Invoice::html(const Service &s, bool demo) {
    QByteArray logo; QBuffer buffer(&logo); buffer.open(QIODevice::WriteOnly);
    Design::brand("logo",QSize(160,160)).save(&buffer,"PNG");
    return QString(R"(
        <html><head><style>
        body { font-family:'Segoe UI'; font-size:11pt; color:#34545e; }
        h1 { font-size:24pt; color:#225b66; margin-bottom:4px; }
        h2 { font-size:15pt; color:#225b66; }
        p { margin-top:8px; margin-bottom:8px; }
        th { color:white; background-color:#367e8b; padding:12px; }
        td { padding:12px; }
        </style></head><body>
        <table width='100%' cellspacing='0'><tr><td width='64'><img src='data:image/png;base64,%1' width='56' height='56'></td>
        <td><h2>ANIWORLD</h2>Centre de soins<br/>Une attention à chaque patte.</td></tr></table>
        <h1>Facture de paiement</h1>
        <p><b>%2</b><br/>Émise le %3%4</p>
        <hr/>
        <table width='100%' cellspacing='0'><tr bgcolor='#edf5f6'><td width='50%'>CLIENT<br/><b>%5</b></td>
        <td>CONSULTATION<br/><b>%6</b><br/>%7</td></tr></table>
        <p><b>Service :</b> %8<br/><b>Catégorie :</b> %9<br/><b>Responsable :</b> %10<br/><b>Durée :</b> %11 min</p>
        <table width='100%' cellspacing='0' cellpadding='10' border='0'>
        <thead><tr><th align='left' width='68%'>Détail des prestations</th><th align='right'>Montant</th></tr></thead>
        <tr bgcolor='#f2f7f8'><td><b>%8</b><br/>%12</td><td align='right'>%13</td></tr></table>
        <table width='100%' cellspacing='0' cellpadding='14'><tr><td width='52%'></td>
        <td bgcolor='#e4f1f2'><b>TOTAL À PAYER</b><h2>%13</h2></td></tr></table>
        <p>Montant de la prestation en dinars tunisiens (DT).<br/>Cette facture ne constitue pas une preuve de paiement.</p>
        <hr/><p>Merci de votre confiance.<br/>ANIWORLD - Centre de soins</p>
        </body></html>)")
        .arg(QString::fromLatin1(logo.toBase64()),escape(s.getInvoiceNumber()),s.getInvoiceDate().toString("dd/MM/yyyy"),
             demo?"<br/><b>DÉMONSTRATION - DONNÉES FICTIVES</b>":"",escape(s.getClientName()),
             QString("SRV-%1").arg(s.getId(),3,10,QChar('0')),s.getDateTime().toString("dd/MM/yyyy à HH:mm"),
             escape(s.getName()),escape(s.getCategory()),escape(s.getResponsible().isEmpty()?"Non attribué":s.getResponsible()),
             QString::number(s.getDuration()),escape(s.getDescription().isEmpty()?s.getName():s.getDescription()),Design::money(s.getPrice()));
}

bool Invoice::writeDocument(const QString &path, const QString &html, const QString &title, QString *error) {
    if (error) error->clear();
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) { if(error) *error=file.errorString(); return false; }
    bool rendered=true;
    {
        QPdfWriter writer(&file); writer.setResolution(96); writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setPageMargins(QMarginsF(16,15,16,17)); writer.setTitle(title); writer.setCreator("ANIWORLD");
        QTextDocument document; document.setDefaultFont(QFont("Segoe UI",11));
        document.documentLayout()->setPaintDevice(&writer);
        const int footer=30;
        document.setPageSize(QSizeF(writer.width(),writer.height()-footer)); document.setHtml(html);
        QPainter painter;
        if (!painter.begin(&writer)) { if(error) *error="Impossible de créer le document PDF."; return false; }
        const int pages=document.pageCount(); const qreal contentHeight=document.pageSize().height();
        for(int page=0;page<pages;++page) {
            if(page && !writer.newPage()) { rendered=false; break; }
            painter.save(); painter.setClipRect(QRectF(0,0,writer.width(),contentHeight));
            painter.translate(0,-page*contentHeight);
            document.drawContents(&painter,QRectF(0,page*contentHeight,writer.width(),contentHeight)); painter.restore();
            painter.setPen(QColor("#7c929a")); painter.setFont(QFont("Segoe UI",8));
            painter.drawText(QRectF(0,writer.height()-20,writer.width(),20),Qt::AlignRight,QString("ANIWORLD  |  %1 / %2").arg(page+1).arg(pages));
        }
        rendered=painter.end() && rendered;
    }
    if(!rendered) { if(error) *error="Impossible de terminer le document PDF."; return false; }
    if(!file.commit()) { if(error) *error=file.errorString(); return false; }
    return true;
}

bool Invoice::loadStored(const Service &source,Service *invoice,QString *error) {
    return InvoiceModel::loadStored(source,invoice,error);
}
bool Invoice::writePdf(const QString &path,const Service &source,bool demo,QString *error) {
    return InvoiceModel::emitDocument(source,demo,[&](const Service &s,QString *failure) {
        return writeDocument(path,html(s,demo),"Facture "+s.getInvoiceNumber(),failure);
    },error);
}
