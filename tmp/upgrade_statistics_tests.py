from pathlib import Path
p=Path('src/widgets/servicewidget.cpp'); s=p.read_text(encoding='utf-8'); a=s.index('void ServiceWidget::showStatistics()')
s=s[:a]+'''void ServiceWidget::showStatistics() {
    const auto metrics=ServiceMetrics::from(m_filtered);
    QDialog dialog(this); dialog.setObjectName("serviceStatisticsDialog");
    dialog.setWindowTitle("ANIWORLD — Statistiques des services"); dialog.resize(760,550);
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(24,22,24,22); layout->setSpacing(14);
    layout->addWidget(Design::label("Statistiques des services","title"));
    auto *scope=Design::label("Répartition et tarifs · "+filterDescription(),"muted"); scope->setWordWrap(true); layout->addWidget(scope);
    double total=0; for(const auto &s:m_filtered) total+=s.price;
    const QStringList captions={"Prestations","À réaliser","Services terminés","Tarif moyen"};
    const QStringList values={QString::number(metrics.total),hours(metrics.scheduledMinutes),Design::money(metrics.completedAmount),Design::money(metrics.total?total/metrics.total:0)};
    const QStringList notes={"dans la sélection","durée planifiée","montant des prestations","par prestation"};
    const QStringList backgrounds={"#E4F0F4","#FFF2E0","#E6F4EB","#FBE9E8"};
    auto *totals=new QHBoxLayout;
    for(int i=0;i<4;++i) {
        auto *card=Design::card(); card->setStyleSheet(QString("QFrame[role=card] { background:%1; border:none; border-radius:8px; }").arg(backgrounds[i]));
        auto *box=new QVBoxLayout(card); box->setContentsMargins(14,12,14,12); box->setSpacing(5);
        box->addWidget(Design::label(captions[i],"muted"));
        auto *value=Design::label(values[i]); value->setStyleSheet("font-size:19px;font-weight:700;color:#306d7b;"); box->addWidget(value);
        box->addWidget(Design::label(notes[i],"muted")); totals->addWidget(card);
    }
    layout->addLayout(totals); layout->addWidget(Design::label("Répartition par catégorie","section"));
    auto *table=new QTableWidget; table->setObjectName("serviceStatisticsTable");
    Design::setupTable(table,{"CATÉGORIE","PRESTATIONS","TARIF MOYEN","PART"}); table->setAlternatingRowColors(true);
    table->setStyleSheet("QTableWidget { alternate-background-color:#f2f7f9; border:1px solid #dbe9ee; border-radius:6px; }");
    table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    for(int col=1;col<4;++col) { table->horizontalHeader()->setSectionResizeMode(col,QHeaderView::Fixed); table->setColumnWidth(col,135); }
    table->setRowCount(metrics.categories.size()); int row=0;
    for(auto it=metrics.categories.cbegin();it!=metrics.categories.cend();++it,++row) {
        double amount=0; for(const auto &service:m_filtered) if(service.category==it.key()) amount+=service.price;
        table->setItem(row,0,new QTableWidgetItem(it.key())); table->setItem(row,1,new QTableWidgetItem(QString::number(it.value())));
        table->setItem(row,2,new QTableWidgetItem(Design::money(amount/it.value())));
        auto *part=new QWidget; auto *line=new QHBoxLayout(part); line->setContentsMargins(10,8,10,8);
        auto *bar=new QProgressBar; bar->setRange(0,qMax(1,metrics.total)); bar->setValue(it.value()); bar->setTextVisible(false); line->addWidget(bar,1);
        line->addWidget(Design::label(QString::number(qRound(100.0*it.value()/metrics.total))+"%","muted")); table->setCellWidget(row,3,part); table->setRowHeight(row,43);
    }
    layout->addWidget(table,1);
    auto *summary=Design::label(QString("%1 planifiés · %2 en cours · %3 terminés · %4 annulés").arg(metrics.planned).arg(metrics.inProgress).arg(metrics.completed).arg(metrics.cancelled));
    summary->setStyleSheet("background:#e4f0f4;color:#377888;padding:12px;border-radius:6px;"); layout->addWidget(summary);
    layout->addWidget(Design::label("Les montants représentent les prestations, sans suivi des encaissements.","muted"));
    auto *close=Design::button("Fermer",{},"primary"); close->setMinimumWidth(130); connect(close,&QPushButton::clicked,&dialog,&QDialog::accept); layout->addWidget(close,0,Qt::AlignRight);
    dialog.exec();
}
'''; p.write_text(s,encoding='utf-8')
# Update existing tests for removed features; keep service / stock regression coverage.
p=Path('tests/service_tests.cpp'); s=p.read_text(encoding='utf-8').replace('#include "../src/rh/hrwidget.h"','#include "../src/ui/invoice.h"\n#include <QSqlQuery>\n#include <QTextBrowser>')
a=s.index('    void rhIntegratedNavigationAndLogout()'); b=s.index('    void storageCrudAndPersistence()',a); s=s[:a]+s[b:]
a=s.index('    void exportEscaping()'); b=s.index('    void dateFiltersAndCards()',a); s=s[:a]+s[b:]
a=s.index('        bool correctTemplate=false;'); b=s.index('    void metricsAndPdfReport()',a); s=s[:a]+'''        QVERIFY(!page.findChild<QPushButton *>("duplicateService"));
        QCOMPARE(db.getAllServices().size(),1); QCOMPARE(db.getAllServices().first().id,originalId);
    }
'''+s[b:]
s=s.replace('quickStatusAndDuplication','quickStatusWithoutDuplication').replace('for(int page:{0,1,2,3,0,3,2,1,0})','for(int page:{0,1,2,0,2,1,0})')
s=s.replace('        const QString csv=stock.inventoryCsv(); QVERIFY(csv.startsWith(QChar(0xFEFF)));\n        QVERIFY(csv.contains("DÉMONSTRATION")); QVERIFY(csv.contains("CAT-V-03")); QVERIFY(!csv.contains("DOG-V-03"));', '''        QTemporaryDir pdfDir; QString pdfError;
        QVERIFY2(stock.exportInventoryPdf(pdfDir.path()+"/stock.pdf",&pdfError),qPrintable(pdfError));
        QFile pdf(pdfDir.path()+"/stock.pdf"); QVERIFY(pdf.open(QIODevice::ReadOnly)); QVERIFY(pdf.read(8).startsWith("%PDF-"));''')
# Supply required client in create/update UI flows.
s=s.replace('dialog->findChild<QPushButton *>("saveService")->click();','dialog->findChild<QLineEdit *>("serviceClient")->setText("Client test");\n            dialog->findChild<QPushButton *>("saveService")->click();')
s=s.replace('name->setText("  Soin des griffes  "); save->click();','name->setText("  Soin des griffes  "); save->click();\n        QCOMPARE(dialog.result(),0); dialog.findChild<QLineEdit *>("serviceClient")->setText("Nour Ben Salah"); save->click();')
p.write_text(s,encoding='utf-8')
