#include "servicewidget.h"
#include "../database/databasemanager.h"
#include "../dialogs/servicedialog.h"
#include "../ui/design.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QSaveFile>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <algorithm>

namespace {
class ServiceDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *p,const QStyleOptionViewItem &option,const QModelIndex &index) const override {
        if(index.column()==4) {
            QStyleOptionViewItem opt(option); initStyleOption(&opt,index); opt.text.clear();
            QApplication::style()->drawControl(QStyle::CE_ItemViewItem,&opt,p); return;
        }
        if(index.column()!=0) { QStyledItemDelegate::paint(p,option,index); return; }
        QStyleOptionViewItem opt(option); initStyleOption(&opt,index); opt.text.clear();
        QApplication::style()->drawControl(QStyle::CE_ItemViewItem,&opt,p);
        p->save(); p->setRenderHint(QPainter::Antialiasing); const QRect r=option.rect;
        p->setPen(Qt::NoPen); p->setBrush(QColor("#EFF4EB")); p->drawRoundedRect(QRectF(r.left()+12,r.top()+15,34,34),9,9);
        Design::icon(index.data(Qt::UserRole+2).toString()).paint(p,QRect(r.left()+20,r.top()+23,18,18));
        QFont font=option.font; font.setPixelSize(12); font.setWeight(QFont::DemiBold); p->setFont(font); p->setPen(QColor("#345546"));
        const int width=r.width()-63; p->drawText(QRect(r.left()+57,r.top()+12,width,22),Qt::AlignVCenter,p->fontMetrics().elidedText(index.data().toString(),Qt::ElideRight,width));
        font.setPixelSize(10); font.setWeight(QFont::Normal); p->setFont(font); p->setPen(QColor("#8B978E")); p->drawText(QRect(r.left()+57,r.top()+34,width,18),Qt::AlignVCenter,index.data(Qt::UserRole+1).toString());
        p->setPen(QColor("#EDF0EB")); p->drawLine(r.bottomLeft(),r.bottomRight()); p->restore();
    }
};
QString folded(QString value) {
    value=value.normalized(QString::NormalizationForm_D).toCaseFolded(); QString result;
    for(const QChar ch:value) if(ch.category()!=QChar::Mark_NonSpacing) result+=ch;
    return result;
}
QString csvCell(QString text) {
    // Prevent spreadsheet applications from interpreting user text as a formula.
    const QString trimmed=text.trimmed();
    if(!trimmed.isEmpty() && QString("=+-@\t\r").contains(trimmed.at(0))) text.prepend('\'');
    text.replace('"',"\"\""); return '"'+text+'"';
}
}
ServiceWidget::ServiceWidget(QWidget *parent):QWidget(parent) {
    setObjectName("servicesPage"); auto *root=new QVBoxLayout(this); root->setContentsMargins(28,24,28,22); root->setSpacing(19);
    auto *head=new QHBoxLayout; auto *title=new QVBoxLayout; title->setSpacing(5); title->addWidget(Design::label("Gestion des services","title")); title->addWidget(Design::label("Chaque prestation, une attention de plus pour leur bien-être.","muted")); head->addLayout(title); head->addStretch();
    m_export=Design::button("Exporter","export"); m_export->setObjectName("exportServices"); m_export->setToolTip("Exporter les résultats affichés au format CSV"); head->addWidget(m_export);
    auto *add=Design::button("Nouveau service","plus","primary"); add->setObjectName("addService"); add->setToolTip("Créer un service · Ctrl+N"); head->addWidget(add); root->addLayout(head);
    m_notice=Design::label(""); m_notice->setObjectName("serviceNotice"); m_notice->setWordWrap(true); m_notice->hide(); root->addWidget(m_notice);
    auto *card=Design::card(); auto *content=new QVBoxLayout(card); content->setContentsMargins(0,0,0,0); content->setSpacing(0);
    auto *cardHead=new QHBoxLayout; cardHead->setContentsMargins(20,19,20,17); cardHead->addWidget(Design::label("Toutes les prestations","section")); cardHead->addStretch();
    m_count=Design::label("","muted"); m_count->setObjectName("serviceCount"); cardHead->addWidget(m_count); content->addLayout(cardHead);
    auto *filters=new QHBoxLayout; filters->setContentsMargins(20,0,20,19); filters->setSpacing(10);
    m_search=new QLineEdit; m_search->setObjectName("serviceSearch"); m_search->setPlaceholderText("Rechercher un service, un responsable…"); m_search->setClearButtonEnabled(true); m_search->addAction(Design::icon("search"),QLineEdit::LeadingPosition); m_search->setMinimumWidth(230); filters->addWidget(m_search,1);
    m_status=new QComboBox; m_status->setObjectName("statusFilter"); m_status->addItem("Tous les statuts"); m_status->addItems(Design::statuses()); filters->addWidget(m_status);
    m_category=new QComboBox; m_category->setObjectName("categoryFilter"); m_category->addItem("Toutes les catégories"); m_category->addItems(Design::categories()); filters->addWidget(m_category);
    m_sort=new QComboBox; m_sort->setObjectName("sortServices"); m_sort->addItems({"Plus récents","Plus anciens","Nom : A → Z","Tarif croissant"}); filters->addWidget(m_sort); content->addLayout(filters);
    m_table=new QTableWidget; m_table->setObjectName("serviceTable"); Design::setupTable(m_table,{"SERVICE","CATÉGORIE","DATE & HEURE","TARIF","STATUT",""}); m_table->setItemDelegate(new ServiceDelegate(m_table));
    m_table->horizontalHeader()->setMinimumSectionSize(42);
    const QList<int> widths={0,140,146,90,122,50};
    for(int i=1;i<widths.size();++i) { m_table->horizontalHeader()->setSectionResizeMode(i,QHeaderView::Fixed); m_table->setColumnWidth(i,widths[i]); }
    m_table->setMinimumHeight(260); content->addWidget(m_table,1);
    m_empty=new QWidget; auto *empty=new QVBoxLayout(m_empty); empty->setContentsMargins(30,65,30,65); empty->setSpacing(13); empty->addStretch();
    auto *illustration=Design::label(""); illustration->setAlignment(Qt::AlignCenter); illustration->setPixmap(Design::brand("logo",QSize(94,94))); empty->addWidget(illustration);
    m_emptyTitle=Design::label("","section"); m_emptyTitle->setAlignment(Qt::AlignCenter); empty->addWidget(m_emptyTitle);
    m_emptyText=Design::label("","muted"); m_emptyText->setWordWrap(true); m_emptyText->setAlignment(Qt::AlignCenter); empty->addWidget(m_emptyText);
    auto *reset=Design::button("Réinitialiser les filtres",{},"link"); reset->setObjectName("resetFilters"); empty->addWidget(reset,0,Qt::AlignCenter); empty->addStretch(); content->addWidget(m_empty,1);
    auto *actions=new QHBoxLayout; actions->setContentsMargins(20,15,20,15); auto *hint=Design::label("Sélectionnez une ligne pour agir.","muted"); actions->addWidget(hint); actions->addStretch();
    m_view=Design::button("Consulter","eye"); m_view->setObjectName("viewService"); m_edit=Design::button("Modifier","edit"); m_edit->setObjectName("editService"); m_delete=Design::button("Supprimer","trash","danger"); m_delete->setObjectName("deleteService");
    actions->addWidget(m_view); actions->addWidget(m_edit); actions->addWidget(m_delete); content->addLayout(actions); root->addWidget(card,1);
    auto *footer=new QHBoxLayout; footer->addWidget(Design::label("Vos soins, organisés en toute simplicité.","muted")); footer->addStretch(); footer->addWidget(Design::label("Ctrl+N  Nouveau   ·   Ctrl+F  Rechercher","muted")); root->addLayout(footer);
    connect(add,&QPushButton::clicked,this,&ServiceWidget::onAdd); connect(m_export,&QPushButton::clicked,this,&ServiceWidget::exportCsv);
    connect(m_edit,&QPushButton::clicked,this,&ServiceWidget::editSelected); connect(m_delete,&QPushButton::clicked,this,&ServiceWidget::deleteSelected);
    connect(m_view,&QPushButton::clicked,this,[this]{viewService(selectedId());});
    connect(m_search,&QLineEdit::textChanged,this,&ServiceWidget::filter);
    for(auto *combo:{m_status,m_category,m_sort}) connect(combo,&QComboBox::currentIndexChanged,this,&ServiceWidget::filter);
    connect(m_table,&QTableWidget::itemSelectionChanged,this,&ServiceWidget::selectionChanged);
    connect(m_table,&QTableWidget::cellDoubleClicked,this,[this](int row,int){viewService(m_table->item(row,0)->data(Qt::UserRole).toInt());});
    connect(reset,&QPushButton::clicked,this,[this]{m_search->clear();m_status->setCurrentIndex(0);m_category->setCurrentIndex(0);m_sort->setCurrentIndex(0);});
    refresh();
}
void ServiceWidget::refresh() {
    m_services=DatabaseManager::instance().getAllServices();
    const QString error=DatabaseManager::instance().lastError();
    if(!error.isEmpty()) { m_notice->setText("Impossible de charger les services : "+error); m_notice->setStyleSheet("color:#AE5751;background:#FFF1E9;padding:11px;border-radius:7px;"); m_notice->show(); }
    filter();
}
void ServiceWidget::filter() {
    m_filtered.clear(); const QString text=folded(m_search->text().trimmed());
    for(const auto &s:m_services) {
        const bool match=text.isEmpty() || folded(s.name+" "+s.description+" "+s.responsible+" "+s.category+" SRV-"+QString::number(s.id).rightJustified(3,'0')).contains(text);
        if(match && (m_status->currentIndex()==0 || s.status==m_status->currentText()) && (m_category->currentIndex()==0 || s.category==m_category->currentText())) m_filtered.append(s);
    }
    const int sort=m_sort->currentIndex();
    std::stable_sort(m_filtered.begin(),m_filtered.end(),[sort](const Service &a,const Service &b){
        if(sort==1) return a.dateTime<b.dateTime;
        if(sort==2) return QString::localeAwareCompare(a.name,b.name)<0;
        if(sort==3) return a.price<b.price;
        return a.dateTime>b.dateTime;
    });
    populate();
}
void ServiceWidget::populate() {
    const int preserveId=selectedId(); m_table->setRowCount(0);
    int selected=-1;
    for(const auto &s:m_filtered) {
        const int row=m_table->rowCount(); m_table->insertRow(row);
        auto *name=new QTableWidgetItem(s.name); name->setData(Qt::UserRole,s.id); name->setData(Qt::UserRole+1,QString("SRV-%1  ·  %2 min").arg(s.id,3,10,QChar('0')).arg(s.duration)); name->setData(Qt::UserRole+2,s.category=="Soins & bien-être"?"heart":s.category=="Éducation"?"paw":"services"); name->setToolTip(s.name+"\nResponsable : "+(s.responsible.isEmpty()?"Non attribué":s.responsible)); m_table->setItem(row,0,name);
        m_table->setItem(row,1,new QTableWidgetItem(s.category)); m_table->setItem(row,2,new QTableWidgetItem(s.dateTime.toString("dd/MM/yyyy · HH:mm"))); m_table->setItem(row,3,new QTableWidgetItem(Design::money(s.price)));
        m_table->setItem(row,4,new QTableWidgetItem(s.status)); m_table->setCellWidget(row,4,Design::badge(s.status));
        auto *view=Design::button("","eye","link"); view->setAccessibleName("Consulter "+s.name); view->setToolTip("Consulter "+s.name); connect(view,&QPushButton::clicked,this,[this,id=s.id]{viewService(id);}); m_table->setCellWidget(row,5,view); m_table->setRowHeight(row,65);
        if(s.id==preserveId) selected=row;
    }
    if(selected>=0) m_table->selectRow(selected);
    m_count->setText(QString("%1 sur %2 service%3").arg(m_filtered.size()).arg(m_services.size()).arg(m_services.size()==1?"":"s"));
    const bool noServices=m_services.isEmpty();
    m_emptyTitle->setText(noServices?"Le premier soin commence avec vous":"Aucun service ne correspond");
    m_emptyText->setText(noServices?"Créez votre première prestation avec « Nouveau service ».\nElle sera enregistrée automatiquement sur cet ordinateur.":"Essayez un autre mot-clé ou modifiez vos filtres.");
    m_empty->findChild<QPushButton *>("resetFilters")->setVisible(!noServices);
    m_empty->setVisible(m_filtered.isEmpty()); m_table->setVisible(!m_filtered.isEmpty()); m_export->setEnabled(!m_filtered.isEmpty()); selectionChanged();
}
int ServiceWidget::selectedId() const {
    const int row=m_table->currentRow();
    return row>=0 && m_table->item(row,0) && m_table->selectionModel()->hasSelection()?m_table->item(row,0)->data(Qt::UserRole).toInt():0;
}
void ServiceWidget::selectionChanged() { const bool selected=selectedId()>0; m_view->setEnabled(selected); m_edit->setEnabled(selected); m_delete->setEnabled(selected); }
void ServiceWidget::focusSearch() { m_search->setFocus();m_search->selectAll(); }
void ServiceWidget::onAdd() {
    ServiceDialog dialog(this);
    while(dialog.exec()==QDialog::Accepted) {
        if(DatabaseManager::instance().addService(dialog.getService())) { refresh(); emit servicesChanged(); notify("Le service a été créé. Vos modifications sont enregistrées."); return; }
        QMessageBox::critical(this,"Enregistrement impossible",DatabaseManager::instance().lastError());
    }
}
void ServiceWidget::viewService(int id) {
    for(const auto &s:m_services) if(s.id==id) { ServiceDialog dialog(s,true,this); dialog.exec(); return; }
}
void ServiceWidget::editSelected() {
    const int id=selectedId();
    for(const auto &s:m_services) if(s.id==id) {
        ServiceDialog dialog(s,false,this);
        while(dialog.exec()==QDialog::Accepted) {
            if(DatabaseManager::instance().updateService(dialog.getService())) { refresh(); emit servicesChanged(); notify("Les modifications du service ont été enregistrées."); return; }
            QMessageBox::critical(this,"Enregistrement impossible",DatabaseManager::instance().lastError());
        }
        return;
    }
}
void ServiceWidget::deleteSelected() {
    const int id=selectedId();
    for(const auto &s:m_services) if(s.id==id) {
        const QString name=s.name;
        QMessageBox confirm(QMessageBox::Question,"Supprimer le service","Supprimer « "+name+" » ?\nCette action est définitive.",QMessageBox::NoButton,this);
        confirm.setTextFormat(Qt::PlainText); auto *cancel=confirm.addButton("Conserver",QMessageBox::RejectRole); auto *remove=confirm.addButton("Supprimer",QMessageBox::DestructiveRole); confirm.setDefaultButton(cancel); confirm.exec();
        if(confirm.clickedButton()!=remove) return;
        if(!DatabaseManager::instance().deleteService(id)) { QMessageBox::critical(this,"Suppression impossible",DatabaseManager::instance().lastError()); return; }
        refresh(); emit servicesChanged(); notify("Le service « "+name+" » a été supprimé."); return;
    }
}
void ServiceWidget::notify(const QString &message) {
    m_notice->setText(message); m_notice->setStyleSheet("color:#416F50;background:#E8F2E4;border:1px solid #D8E6D3;padding:11px;border-radius:7px;"); m_notice->show();
    QTimer::singleShot(5500,this,[this,message]{if(m_notice->text()==message)m_notice->hide();});
}
QString ServiceWidget::csvContent(const QList<Service> &services) {
    QString result=QString(QChar(0xFEFF))+"ID;Service;Catégorie;Responsable;Date;Statut;Durée (min);Tarif (DT);Description\r\n";
    for(const auto &s:services) {
        QStringList cells={QString::number(s.id),s.name,s.category,s.responsible,s.dateTime.toString("dd/MM/yyyy HH:mm"),s.status,QString::number(s.duration),QString::number(s.price,'f',2),s.description};
        for(auto &cell:cells) cell=csvCell(cell);
        result+=cells.join(';')+"\r\n";
    }
    return result;
}
void ServiceWidget::exportCsv() {
    const QString path=QFileDialog::getSaveFileName(this,"Exporter les services","ANIWORLD-services-"+QDate::currentDate().toString("yyyy-MM-dd")+".csv","Fichier CSV (*.csv)"); if(path.isEmpty()) return;
    QSaveFile file(path); const QByteArray content=csvContent(m_filtered).toUtf8();
    if(!file.open(QIODevice::WriteOnly) || file.write(content)!=content.size() || !file.commit()) { QMessageBox::critical(this,"Export impossible","Le fichier n'a pas pu être enregistré.\n"+file.errorString()); return; }
    notify(QString("%1 service(s) exporté(s) dans %2").arg(m_filtered.size()).arg(path));
}
