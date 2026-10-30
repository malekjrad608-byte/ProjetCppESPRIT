from pathlib import Path
root=Path.cwd()
def edit(file,fn):
 p=root/file; p.write_text(fn(p.read_text(encoding='utf-8')),encoding='utf-8')
edit('sources.pri',lambda s:s.replace('include($$PWD/src/rh/rh.pri)','SOURCES += $$PWD/src/ui/invoice.cpp\nHEADERS += $$PWD/src/ui/invoice.h'))
edit('src/mainwindow.h',lambda s:s.replace('namespace AniRh { class HrWidget; class HrStore; }\n','').replace(',const QString &rhStoragePath={} رح','').replace(',const QString &rhStoragePath={}','').replace('    void loadRh();\n','').replace('    QWidget *m_rhPage;\n    AniRh::HrWidget *m_rh=nullptr;\n    AniRh::HrStore *m_rhStore=nullptr;\n    QString m_rhStoragePath;\n',''))
def main(s):
 s=s.replace('#include "rh/hrwidget.h"\n','').replace('bool demo,QWidget *parent,const QString &rhStoragePath):QMainWindow(parent),m_rhStoragePath(rhStoragePath),m_demo(demo)','bool demo,QWidget *parent):QMainWindow(parent),m_demo(demo)')
 s=s.replace('"Stock","Ressources humaines"','"Stock"').replace('"stock","users"','"stock"').replace(',"navRh"','')
 s=s.replace('        if(i==3) { button->setToolTip("RH · Ressources humaines"); button->setAccessibleName("RH · Ressources humaines"); }\n','')
 a=s.index('    m_rhPage=new QWidget;'); b=s.index('    QPalette pageColors',a); s=s[:a]+s[b:]
 s=s.replace('        else if(m_stack->currentIndex()==3) { if(m_rh) m_rh->openCreate(); }\n','').replace('        else if(m_stack->currentIndex()==3) { if(m_rh) m_rh->focusSearch(); }\n','').replace('"Stock","RH"','"Stock"')
 a=s.index('    else {\n        loadRh();'); b=s.index('    positionNavigationMarker(changed);',a); s=s[:a]+s[b:]
 a=s.index('void MainWindow::loadRh()'); b=s.index('void MainWindow::positionNavigationMarker',a); s=s[:a]+s[b:]
 # Clean white navigation and compact identity, matching the reference.
 a=s.index('        QPainter painter(this);'); b=s.index('\n    }\nprivate:',a); s=s[:a]+s[b:]
 s=s.replace('setFixedWidth(248)','setFixedWidth(224)').replace('setContentsMargins(17,20,17,17)','setContentsMargins(12,12,12,14)').replace('new AnimatedLogo; brandRow','new AnimatedLogo; brandLogo->setFixedSize(42,42); brandRow')
 s=s.replace('font-size:20px;font-weight:700','font-size:17px;font-weight:700').replace('setFixedHeight(64)','setFixedHeight(52)').replace('ESPACE PRINCIPAL','NAVIGATION').replace('"Espace de gestion   /   "','"Gestion / "')
 return s
edit('src/mainwindow.cpp',main)
edit('src/main.cpp',lambda s:s.replace('2.6.0','3.0.0'))
def dialog(s):
 s=s.replace('root->setContentsMargins(27,24,27,24); root->setSpacing(17)','root->setContentsMargins(22,20,22,20); root->setSpacing(12)')
 s=s.replace('form->addWidget(Design::label("Description et notes"),8,0,1,2); form->addWidget(m_description,9,0,1,2);', '''m_client=new QLineEdit(s.clientName); m_client->setObjectName("serviceClient"); m_client->setMaxLength(120);
    m_client->setPlaceholderText("Nom et prénom du client");
    form->addWidget(Design::label("Client *"),8,0,1,2); form->addWidget(m_client,9,0,1,2);
    form->addWidget(Design::label("Détail des prestations (repris sur la facture)"),10,0,1,2); form->addWidget(m_description,11,0,1,2);''')
 s=s.replace('m_name->setReadOnly(true);','m_client->setReadOnly(true); m_name->setReadOnly(true);')
 s=s.replace('    if(m_description->toPlainText().size()>5000)', '''    const bool clientValid=!m_client->text().trimmed().isEmpty();
    m_client->setProperty("invalid",!clientValid); m_client->style()->unpolish(m_client); m_client->style()->polish(m_client);
    if(!clientValid) { m_error->setText("Renseignez le nom du client pour sa facture."); m_error->show(); m_client->setFocus(); return; }
    if(m_description->toPlainText().size()>5000)''')
 s=s.replace('s.responsible=m_responsible','s.clientName=m_client->text().trimmed(); s.responsible=m_responsible')
 return s
edit('src/dialogs/servicedialog.cpp',dialog)
def header(s):
 s=s.replace('    static QString csvContent(const QList<Service> &services);\n','').replace('    void duplicateSelected();','    void showInvoice();').replace('    void exportCsv();\n','').replace('m_duplicate','m_invoice')
 return s
edit('src/widgets/servicewidget.h',header)
def widget(s):
 s=s.replace('#include "../ui/servicereport.h"','#include "../ui/servicereport.h"\n#include "../ui/invoice.h"\n#include <QTextBrowser>')
 a=s.index('QString csvCell('); b=s.index('QString hours(',a); s=s[:a]+s[b:]
 a=s.index('    auto *exportMenu'); b=s.index('    head->addWidget(m_export);',a)
 s=s[:a]+'    connect(m_export,&QPushButton::clicked,this,&ServiceWidget::exportPdf);\n'+s[b:]
 s=s.replace('Design::button("Exporter",','Design::button("Exporter PDF",').replace('Design::button("Statistiques",','Design::button("Voir les statistiques",')
 s=s.replace('root->setContentsMargins(28,24,28,19)','root->setContentsMargins(24,20,24,16)').replace('root->setSpacing(16)','root->setSpacing(12)')
 s=s.replace('border-radius:8px;padding:11px 13px;font-size:12px','border-radius:14px;padding:6px 12px;font-size:11px')
 s=s.replace('Rechercher par nom, ID ou responsable…','Rechercher par service, client ou référence…')
 s=s.replace('"Nom : A → Z", "Tarif croissant"','"Nom : A → Z", "Tarif croissant", "Tarif décroissant", "Durée croissante", "Durée décroissante"')
 s=s.replace('    tools->addWidget(m_sort);','    m_sort->setAccessibleName("Trier les services par date, tarif ou durée");\n    tools->addWidget(Design::label("Trier par", "muted"));\n    tools->addWidget(m_sort);')
 s=s.replace('m_grid->setSpacing(14)','m_grid->setSpacing(12)').replace('>=620','>=520').replace('panel->setFixedWidth(286)','panel->setFixedWidth(288)')
 s=s.replace('"Responsable", "Tarif"','"Responsable", "Client", "Tarif", "Facture"')
 s=s.replace('m_duplicate = Design::button("Dupliquer", "plus");','m_invoice = Design::button("Facture PDF", "export", "primary");').replace('m_duplicate','m_invoice').replace('"duplicateService"','"invoiceService"').replace('&ServiceWidget::duplicateSelected','&ServiceWidget::showInvoice')
 s=s.replace('    auto *lastRow = new QHBoxLayout;\n    lastRow->setSpacing(7); lastRow->addWidget(m_invoice); lastRow->addWidget(m_delete);\n    content->addLayout(lastRow);','    content->addWidget(m_invoice);\n    content->addWidget(m_delete);')
 s=s.replace('s.responsible+" "+s.category','s.responsible+" "+s.clientName+" "+s.invoiceNumber+" "+s.category')
 s=s.replace('        if (sort==3) return a.price<b.price;','        if (sort==3) return a.price<b.price;\n        if (sort==4) return a.price>b.price;\n        if (sort==5) return a.duration<b.duration;\n        if (sort==6) return a.duration>b.duration;')
 s=s.replace('s.responsible,Design::money(s.price)}','s.responsible,s.clientName.isEmpty()?"À compléter":s.clientName,Design::money(s.price),s.invoiceNumber}')
 s=s.replace('    if (!selected) return;','    m_invoice->setEnabled(selected && s.status!="Annulé");\n    m_invoice->setToolTip(s.status=="Annulé"?"Un service annulé ne peut pas être facturé.":"Facture créée automatiquement avec les informations du service.");\n    if (!selected) return;')
 s=s.replace('Le service a été ajouté et enregistré.','Le service et sa facture ont été créés automatiquement.')
 a=s.index('void ServiceWidget::duplicateSelected()'); b=s.index('void ServiceWidget::advanceStatus()',a); s=s[:a]+s[b:]
 a=s.index('QString ServiceWidget::csvContent('); b=s.index('void ServiceWidget::exportPdf()',a); s=s[:a]+s[b:]
 return s
edit('src/widgets/servicewidget.cpp',widget)
