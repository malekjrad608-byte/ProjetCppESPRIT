#include "servicereport.h"
#include "design.h"
#include "../models/serviceanalytics.h"
#include <QPdfWriter>
#include <QSaveFile>
#include <QPageLayout>
#include <QPainter>
#include <QFontMetricsF>

bool ServiceReport::writePdf(const QString &path, const QList<Service> &services,
                            const QString &scope, QString *error) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = file.errorString();
        return false;
    }
    bool rendered = true;
    {
        QPdfWriter writer(&file);
        writer.setTitle("ANIWORLD - Rapport des services");
        writer.setCreator("ANIWORLD");
        writer.setResolution(144);
        writer.setPageLayout(QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Landscape,
                                         QMarginsF(12,12,12,12), QPageLayout::Millimeter));
        QPainter painter;
        if (!painter.begin(&writer)) {
            if (error) *error = "Le moteur PDF n'a pas pu démarrer.";
            return false;
        }
        const qreal scale = writer.resolution()/72.0;
        const qreal width = writer.width()/scale;
        const qreal height = writer.height()/scale;
        QFont regular("Segoe UI"); regular.setPixelSize(9);
        QFont bold = regular; bold.setWeight(QFont::DemiBold);
        const QFontMetricsF regularMetrics(regular);
        const QFontMetricsF boldMetrics(bold);
        const QList<qreal> proportions = {0.28,0.16,0.17,0.19,0.11,0.09};
        QList<qreal> widths;
        for (qreal proportion : proportions) widths.append(width*proportion);
        const int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;
        const qreal scopeHeight = regularMetrics.boundingRect(QRectF(0,0,width,1000),flags,scope).height()+4;
        const qreal firstHeader = 120+scopeHeight;
        const qreal nextHeader = 69;
        QList<QStringList> rows;
        QList<qreal> heights;
        for (const auto &service : services) {
            QStringList row = {service.name+QString("\nSRV-%1").arg(service.id,3,10,QChar('0')),
                service.category,service.dateTime.toString("dd/MM/yyyy HH:mm")+QString("\n%1 min").arg(service.duration),
                service.responsible.isEmpty()?"Non attribué":service.responsible,service.status,Design::money(service.price)};
            qreal rowHeight=30;
            for (int col=0;col<row.size();++col) {
                const auto &metrics = col==0 ? boldMetrics : regularMetrics;
                rowHeight=qMax(rowHeight,metrics.boundingRect(QRectF(0,0,widths[col]-12,1000),flags,row[col]).height()+14);
            }
            rows.append(row); heights.append(rowHeight);
        }
        // Determine page boundaries before drawing, so no row or repeated heading is split.
        QList<QPair<int,int>> pages;
        int first=0;
        qreal y=firstHeader+25;
        for (int i=0;i<rows.size();++i) {
            if (i>first && y+heights[i]>height-30) {
                pages.append(qMakePair(first,i)); first=i; y=nextHeader+25;
            }
            y+=heights[i];
        }
        pages.append(qMakePair(first,int(rows.size())));
        const auto totals=ServiceMetrics::from(services);
        const QStringList headers={"RÉF. / SERVICE","CATÉGORIE","DATE / DURÉE","RESPONSABLE","STATUT","TARIF"};
        for (int page=0;page<pages.size();++page) {
            if (page>0 && !writer.newPage()) { rendered=false; break; }
            painter.save(); painter.scale(scale,scale);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.drawPixmap(QRect(0,0,43,43),Design::brand("logo",QSize(150,150)));
            QFont brand=bold; brand.setPixelSize(20); painter.setFont(brand); painter.setPen(QColor("#377E8E"));
            painter.drawText(QRectF(53,0,width-53,28),Qt::AlignVCenter,"ANIWORLD");
            painter.setFont(regular); painter.setPen(QColor("#718992"));
            painter.drawText(QRectF(54,30,width-54,14),"Centre de soins · Rapport des prestations");
            painter.drawText(QRectF(width-175,12,175,20),Qt::AlignRight,"Édité le "+QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm"));
            if (page==0) {
                painter.setPen(QColor("#435F6A"));
                painter.drawText(QRectF(0,58,width,scopeHeight),flags,scope);
                painter.setFont(bold);
                painter.drawText(QRectF(0,64+scopeHeight,width,16),
                    QString("%1 services   ·   %2 planifiés   ·   %3 en cours   ·   %4 terminés   ·   %5 annulés")
                    .arg(totals.total).arg(totals.planned).arg(totals.inProgress).arg(totals.completed).arg(totals.cancelled));
                painter.setFont(regular);
                painter.drawText(QRectF(0,83+scopeHeight,width,16),"Montant des prestations terminées : "+Design::money(totals.completedAmount)+" (sans suivi des paiements).");
            }
            y=page==0 ? firstHeader : nextHeader;
            painter.fillRect(QRectF(0,y,width,25),QColor("#E6F0F3"));
            painter.setFont(bold); painter.setPen(QColor("#315D69"));
            qreal x=0;
            for (int col=0;col<headers.size();++col) {
                painter.drawText(QRectF(x+6,y+7,widths[col]-12,18),Qt::AlignLeft,headers[col]); x+=widths[col];
            }
            y+=25;
            for (int row=pages[page].first;row<pages[page].second;++row) {
                painter.fillRect(QRectF(0,y,width,heights[row]),row%2 ? QColor("#F1F6F7") : Qt::white);
                x=0; painter.setPen(QColor("#314D59"));
                for (int col=0;col<headers.size();++col) {
                    painter.setFont(col==0 ? bold : regular);
                    painter.drawText(QRectF(x+6,y+7,widths[col]-12,heights[row]-12),flags,rows[row][col]);
                    x+=widths[col];
                }
                y+=heights[row];
            }
            if (services.isEmpty()) {
                painter.setFont(regular); painter.drawText(QRectF(6,y+12,width-12,25),"Aucune prestation dans cette sélection.");
            }
            painter.setPen(QColor("#DDE7EC")); painter.drawLine(QPointF(0,height-22),QPointF(width,height-22));
            painter.setPen(QColor("#81969F")); painter.setFont(regular);
            painter.drawText(QRectF(0,height-16,width-80,16),"Une attention à chaque patte. · ANIWORLD");
            painter.drawText(QRectF(width-80,height-16,80,16),Qt::AlignRight,QString("%1 / %2").arg(page+1).arg(pages.size()));
            painter.restore();
        }
        painter.end();
    }
    if (!rendered) { if (error) *error="Impossible de créer une page du rapport."; return false; }
    if (!file.commit()) { if (error) *error=file.errorString(); return false; }
    return true;
}
