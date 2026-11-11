#include "ui_mainwindow.h"
#include "../database/oracleconnection.h"
#include "servicewidget.h"
#include "servicecard.h"
#include "../database/databasemanager.h"
#include "../dialogs/servicedialog.h"
#include "../models/serviceanalytics.h"
#include "../ui/design.h"
#include "../ui/servicemotion.h"
#include "../dialogs/serviceplanningdialog.h"
#include "../ui/invoice.h"
#include <QTextBrowser>
#include <QFile>
#include <QShortcut>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QScreen>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>
#include <QButtonGroup>
#include <QSignalBlocker>
#include <QScrollBar>
#include <QTimer>
#include <QProgressBar>
#include <QLocale>
#include <algorithm>

namespace {
class ServiceTableDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        QStyledItemDelegate::initStyleOption(option,index);
        if(index.column()==5) option->text=QDateTime::fromString(index.data().toString(),Qt::ISODate).toString("dd/MM/yyyy HH:mm");
        if(index.column()==8) option->text=Design::money(index.data().toDouble());
    }
};

QString folded(const QString &value) {
    QString result;
    for (const QChar ch : value.normalized(QString::NormalizationForm_D).toCaseFolded())
        if (ch.category() != QChar::Mark_NonSpacing) result += ch;
    return result;
}
QString reference(int id) { return QString("SRV-%1").arg(id,3,10,QChar('0')); }
QString hours(int minutes) {
    return QString("%1 h %2").arg(minutes/60).arg(minutes%60,2,10,QChar('0'));
}
}

ServiceWidget::ServiceWidget(Ui::MainWindow *mainUi, QWidget *parent) : QWidget(parent) {
    // Widgets already created by the single MainWindow::setupUi().
    auto &ui = *mainUi;
    setObjectName("servicesPage");
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(ui.servicesCentral);
    QFile theme(":/services/theme.qss");
    if (theme.open(QIODevice::ReadOnly))
        setStyleSheet(QString::fromUtf8(theme.readAll()));
    m_advance=ui.Service_m_advance;
    m_attention=ui.Service_m_attention;
    m_category=ui.Service_m_category;
    m_count=ui.Service_m_count;
    m_delete=ui.Service_m_delete;
    m_detailContent=ui.Service_m_detailContent;
    m_detailName=ui.Service_m_detailName;
    m_detailNotes=ui.Service_m_detailNotes;
    m_detailReference=ui.Service_m_detailReference;
    m_edit=ui.Service_m_edit;
    m_emptyDetailSpacer=ui.Service_m_emptyDetailSpacer;
    m_emptyText=ui.Service_m_emptyText;
    m_emptyTitle=ui.Service_m_emptyTitle;
    m_grid=ui.Service_m_grid;
    m_gridScroll=ui.Service_m_gridScroll;
    m_invoice=ui.Service_m_invoice;
    m_listPage=ui.Service_m_listPage;
    m_notice=ui.Service_m_notice;
    m_pages=ui.Service_m_pages;
    m_period=ui.Service_m_period;
    m_search=ui.Service_m_search;
    m_sort=ui.Service_m_sort;
    m_status=ui.Service_m_status;
    m_statusLayout=ui.Service_m_statusLayout;
    m_table=ui.Service_m_table;
    m_view=ui.Service_m_view;
    m_views=ui.Service_m_views;
    m_summary.append(ui.Service_summary_0);
    m_summary.append(ui.Service_summary_1);
    m_summary.append(ui.Service_summary_2);
    m_summary.append(ui.Service_summary_3);
    m_detailValues.append(ui.Service_detailValue_0);
    m_detailValues.append(ui.Service_detailValue_1);
    m_detailValues.append(ui.Service_detailValue_2);
    m_detailValues.append(ui.Service_detailValue_3);
    m_detailValues.append(ui.Service_detailValue_4);
    m_detailValues.append(ui.Service_detailValue_5);
    m_detailValues.append(ui.Service_detailValue_6);
    m_detailValues.append(ui.Service_detailValue_7);
    m_grid->setAlignment(Qt::AlignTop);m_grid->setSizeConstraint(QLayout::SetMinimumSize);
    m_gridScroll->widget()->setAutoFillBackground(false);m_gridScroll->viewport()->setAutoFillBackground(false);
    m_search->addAction(Design::icon("search"),QLineEdit::LeadingPosition);
    // Shortcuts advertised in the Designer footer, scoped to this module.
    auto *addShortcut = new QShortcut(QKeySequence("Ctrl+N"), this);
    addShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(addShortcut, &QShortcut::activated, this, &ServiceWidget::onAdd);
    auto *searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    searchShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(searchShortcut, &QShortcut::activated, this, [this] {
        if (m_pages->currentWidget() == m_listPage) focusSearch();
    });
    m_notice->hide();m_attention->hide();
    m_noticeTimer=new QTimer(this);m_noticeTimer->setSingleShot(true);
    connect(m_noticeTimer,&QTimer::timeout,m_notice,&QWidget::hide);
    connect(ui.servicePlanning,&QPushButton::clicked,this,&ServiceWidget::showPlanning);
    connect(ui.addService,&QPushButton::clicked,this,&ServiceWidget::onAdd);
    connect(ui.serviceFirstAdd,&QPushButton::clicked,this,&ServiceWidget::onAdd);
    connect(ui.serviceStatistics,&QPushButton::clicked,this,&ServiceWidget::showStatistics);
    connect(m_edit,&QPushButton::clicked,this,&ServiceWidget::editSelected);
    connect(m_view,&QPushButton::clicked,this,[this]{viewService(m_selectedId);});
    connect(m_invoice,&QPushButton::clicked,this,&ServiceWidget::showInvoice);
    connect(m_advance,&QPushButton::clicked,this,&ServiceWidget::advanceStatus);
    connect(m_delete,&QPushButton::clicked,this,&ServiceWidget::deleteSelected);
    auto *group=new QButtonGroup(this);int mode=0;
    for(auto *toggle:{ui.cardsView,ui.tableView}){
        const int index=mode++;group->addButton(toggle,index);
        connect(toggle,&QPushButton::clicked,this,[this,index]{m_viewMode=index;m_views->setCurrentIndex(m_filtered.isEmpty()?2:index);ServiceMotion::reveal(m_views,QColor("#F3F7F8"));});
    }
    m_table->setModel(Stmp.afficher(this));
    m_table->setItemDelegate(new ServiceTableDelegate(m_table));
    m_table->verticalHeader()->hide();
    m_table->verticalHeader()->setDefaultSectionSize(65);
    m_table->setShowGrid(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setWordWrap(false);
    m_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setHighlightSections(false);

    connect(m_search,&QLineEdit::textChanged,this,&ServiceWidget::filter);
    for(auto *combo:{m_status,m_category,m_sort,m_period})connect(combo,&QComboBox::currentIndexChanged,this,&ServiceWidget::filter);
    connect(ui.resetFilters,&QPushButton::clicked,this,[this]{
        const QSignalBlocker a(m_search),b(m_status),c(m_category),d(m_period),e(m_sort);
        m_search->clear();m_status->setCurrentIndex(0);m_category->setCurrentIndex(0);m_period->setCurrentIndex(0);m_sort->setCurrentIndex(0);filter();
    });
    connect(m_table,&QTableView::doubleClicked,this,[this](const QModelIndex &index){viewService(index.siblingAtColumn(0).data().toInt());});
    auto *timer=new QTimer(this);connect(timer,&QTimer::timeout,this,[this]{updateSummary();if(m_period->currentIndex()>0)filter();else updateDetail();});timer->start(60000);
    refresh();
}



void ServiceWidget::refresh() {
    const bool recovering = !m_loadError.isEmpty();
    m_services = Service::afficherListe(&m_loadError);
    if (recovering && m_loadError.isEmpty()) m_notice->hide();
    // Block category signals until all data and the SQL model are refreshed.
    {
        const QSignalBlocker blocker(m_category);
        for (const auto &service : m_services)
            if (m_category->findText(service.getCategory()) < 0)
                m_category->addItem(service.getCategory());
    }
    updateSummary();
    filter();
}

void ServiceWidget::configureTableColumns() {
    auto *header = m_table->horizontalHeader();
    const int count = header->count();
    // A failed SQL query has zero columns. Qt requires valid section indices.
    // Reapply after every model replacement, including recovery after reconnect.
    const bool oracle = OracleConnection::isOracle();
    for (int col = 0; col < count; ++col) {
        const bool visible = oracle
            ? QList<int>{0, 1, 2, 4, 5, 6, 7, 8}.contains(col)
            : QList<int>{1, 5, 6, 8}.contains(col);
        m_table->setColumnHidden(col, !visible);
    }
    if (oracle) {
        const QList<int> order = {0, 1, 4, 6, 5, 7, 8, 2};
        int target = 0;
        for (int col : order) {
            if (col >= count) continue;
            const int from = header->visualIndex(col);
            if (from >= 0) header->moveSection(from, target++);
        }
        for (int col : {0, 7})
            if (col < count) header->setSectionResizeMode(col, QHeaderView::ResizeToContents);
    }
    for (int col : {5, 6, 8})
        if (col < count) header->setSectionResizeMode(col, QHeaderView::Fixed);
    if (5 < count) m_table->setColumnWidth(5, 155);
    if (6 < count) m_table->setColumnWidth(6, 108);
    if (8 < count) m_table->setColumnWidth(8, 85);
}

void ServiceWidget::filter() {
    m_filtered.clear();
    const QString text = folded(m_search->text().trimmed());
    const QDateTime now = QDateTime::currentDateTime();
    for (const auto &s : m_services) {
        const bool matchesText = text.isEmpty() || folded(s.getName()+" "+s.getDescription()+" "+s.getResponsible()+" "+s.getClientName()+" "+s.getInvoiceNumber()+" "+s.getCategory()+" "+reference(s.getId())+" "+QString::number(s.getId())).contains(text);
        bool matchesPeriod = true;
        if (m_period->currentIndex()==1) matchesPeriod = s.getDateTime().date()==now.date();
        if (m_period->currentIndex()==2) matchesPeriod = s.getDateTime().date()>=now.date() && s.getDateTime().date()<=now.date().addDays(6);
        if (m_period->currentIndex()==3) matchesPeriod = s.getStatus()=="Planifié" && s.getDateTime()<now;
        if (matchesText && matchesPeriod && (m_status->currentIndex()==0 || s.getStatus()==m_status->currentText())
            && (m_category->currentIndex()==0 || s.getCategory()==m_category->currentText())) m_filtered.append(s);
    }
    const int sort = m_sort->currentIndex();
    std::stable_sort(m_filtered.begin(),m_filtered.end(),[sort](const Service &a,const Service &b) {
        if (sort==1) return a.getDateTime()<b.getDateTime();
        if (sort==2) return QString::localeAwareCompare(a.getName(),b.getName())<0;
        if (sort==3) return a.getPrice()<b.getPrice();
        if (sort==4) return a.getPrice()>b.getPrice();
        if (sort==5) return a.getDuration()<b.getDuration();
        if (sort==6) return a.getDuration()>b.getDuration();
        return a.getDateTime()>b.getDateTime();
    });
    const bool selectedVisible = std::any_of(m_filtered.begin(),m_filtered.end(),[this](const Service &s) { return s.getId()==m_selectedId; });
    if (!selectedVisible) m_selectedId = m_filtered.isEmpty() ? 0 : m_filtered.first().getId();
    populate();
}

void ServiceWidget::populate() {
    QList<int> ids;
    for (const Service &service : m_filtered) ids.append(service.getId());
    // Actualisation apres ajout/modification/suppression : afficher() -> setModel().
    auto *previousModel = m_table->model();
    auto *previousSelection = m_table->selectionModel();
    m_table->setModel(Stmp.afficher(this, &ids));
    configureTableColumns();
    delete previousSelection;
    delete previousModel;
    auto *model = static_cast<QSqlQueryModel *>(m_table->model());
    while (!model->lastError().isValid() && model->canFetchMore()) model->fetchMore();
    if (model->lastError().isValid()) m_loadError = model->lastError().text();
    connect(m_table->selectionModel(),&QItemSelectionModel::currentRowChanged,this,[this](const QModelIndex &current) {
        if(current.isValid()) selectService(current.siblingAtColumn(0).data().toInt());
    });
    for(int row=0;row<model->rowCount();++row)
        if(model->index(row,0).data().toInt()==m_selectedId) { m_table->selectRow(row); break; }
    m_count->setText(QString("%1 / %2 services").arg(m_filtered.size()).arg(m_services.size()));
    const bool empty = m_services.isEmpty();
    if (!m_loadError.isEmpty()) {
        m_emptyTitle->setText("Services indisponibles");
        m_emptyText->setText("Vérifiez la connexion Oracle et les tables/vues du module Services.\n"
                            "Après correction, cliquez sur le badge Oracle ou revenez sur cette page.");
        notify("Impossible de charger les services : " +
               m_loadError.section(QChar(0), 0, 0), true);
    } else {
        m_emptyTitle->setText(empty ? "Votre première prestation" : "Aucun service trouvé");
        m_emptyText->setText(empty ? "Ajoutez un service pour organiser vos soins.\nVos données sont enregistrées dans la base Oracle commune."
                                  : "Modifiez votre recherche ou réinitialisez les filtres.");
    }
    m_views->setCurrentIndex(m_filtered.isEmpty() ? 2 : m_viewMode);
    rebuildCards();
    updateDetail();
}

void ServiceWidget::rebuildCards() {
    while (auto *item = m_grid->takeAt(0)) { delete item->widget(); delete item; }
    m_grid->setColumnStretch(0,0); m_grid->setColumnStretch(1,0);
    m_gridColumns = m_gridScroll->viewport()->width()>=520 ? 2 : 1;
    for (int i=0; i<m_filtered.size(); ++i) {
        const auto &service = m_filtered[i];
        auto *card = new ServiceCard(service);
        card->setProperty("entranceDelay",qMin(i,7)*24);
        card->setChecked(service.getId()==m_selectedId);
        connect(card,&QAbstractButton::clicked,this,[this,id=service.getId()] { selectService(id); });
        m_grid->addWidget(card,i/m_gridColumns,i%m_gridColumns);
    }
    for (int col=0; col<m_gridColumns; ++col) m_grid->setColumnStretch(col,1);
    QTimer::singleShot(0,this,[this] {
        m_grid->activate();
        if (m_views->currentIndex()==0)
            if (auto *selected=m_gridScroll->findChild<QAbstractButton *>("serviceCard_"+QString::number(m_selectedId)))
                m_gridScroll->ensureWidgetVisible(selected,0,10);
    });
}

void ServiceWidget::paintEvent(QPaintEvent *) { QPainter p(this); Design::paintWorkspace(p,rect()); }

void ServiceWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    QTimer::singleShot(0,this,[this] {
        const int columns = m_gridScroll->viewport()->width()>=520 ? 2 : 1;
        if (columns!=m_gridColumns) rebuildCards();
    });
}

void ServiceWidget::selectService(int id) {
    const bool changed=id!=m_selectedId;
    m_selectedId = id;
    for (auto *card : m_gridScroll->findChildren<QAbstractButton *>())
        card->setChecked(card->objectName()=="serviceCard_"+QString::number(id));
    const QSignalBlocker blocker(m_table->selectionModel());
    for (int row=0; row<m_table->model()->rowCount(); ++row)
        if (m_table->model()->index(row,0).data().toInt()==id) { m_table->selectRow(row); break; }
    updateDetail();
    if(changed) ServiceMotion::reveal(findChild<QWidget *>("serviceDetailPanel"));
}

Service ServiceWidget::selectedService() const {
    for (const auto &service : m_services) if (service.getId()==m_selectedId) return service;
    return {};
}

void ServiceWidget::updateDetail() {
    const Service s = selectedService();
    const bool selected = s.getId()>0;
    m_detailContent->setVisible(selected);
    m_emptyDetailSpacer->setVisible(!selected);
    for (auto *button : {m_edit,m_delete,m_view,m_invoice,m_advance}) button->setEnabled(selected);
    m_detailName->setText(selected ? s.getName() : "Sélectionnez un service");
    m_detailReference->setText(selected ? reference(s.getId()) : "Les informations apparaîtront ici.");
    m_invoice->setEnabled(selected && s.getStatus()!="Annulé");
    m_invoice->setToolTip(s.getStatus()=="Annulé"?"Un service annulé ne peut pas être facturé.":"Facture créée automatiquement avec les informations du service.");
    if (!selected) return;
    while (auto *item = m_statusLayout->takeAt(0)) { delete item->widget(); delete item; }
    auto *statusBadge=Design::badge(s.getStatus());
    auto *badgeLayout=statusBadge->layout(); badgeLayout->setContentsMargins(0,0,0,0);
    delete badgeLayout->takeAt(1);
    if(auto *text=qobject_cast<QLabel *>(badgeLayout->itemAt(0)->widget())) {
        text->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        QString style=text->styleSheet(); style.replace("border-radius:10px","border-radius:4px"); text->setStyleSheet(style);
    }
    m_statusLayout->addWidget(statusBadge);
    const QDateTime end = s.getDateTime().addSecs(s.getDuration()*60);
    const QString timing = s.getDateTime().toString("HH:mm")+" → "+end.toString(end.date()==s.getDateTime().date()?"HH:mm":"dd/MM HH:mm");
    const QStringList values = {s.getCategory(),QLocale().toString(s.getDateTime().date(),"dd MMMM yyyy"),timing,
                               QString::number(s.getDuration())+" min",s.getResponsible().isEmpty()?"Non attribué":s.getResponsible(),s.getClientName().isEmpty()?"À compléter":s.getClientName(),Design::money(s.getPrice()),s.getInvoiceNumber()};
    for (int i=0; i<values.size(); ++i) m_detailValues[i]->setText(values[i]);
    const QString notes = s.getDescription().isEmpty() ? "Aucune note pour cette prestation." : s.getDescription();
    m_detailNotes->setText(notes.size()>150 ? notes.left(147)+"…" : notes);
    m_detailNotes->setToolTip(notes);
    m_advance->setVisible(s.getStatus()=="Planifié" || s.getStatus()=="En cours");
    m_advance->setText(s.getStatus()=="Planifié" ? "Démarrer le service" : "Marquer comme terminé");
}

void ServiceWidget::updateSummary() {
    const auto metrics = ServiceMetrics::from(m_services);
    const QStringList values = {QString("%1 planifiés").arg(metrics.planned),QString("%1 en cours").arg(metrics.inProgress),
                               QString("%1 terminés").arg(metrics.completed),hours(metrics.scheduledMinutes)+" à réaliser"};
    for (int i=0; i<values.size(); ++i) m_summary[i]->setText(values[i]);
    m_attention->setText(metrics.overdue>0
        ? QString("%1 service(s) planifié(s) ont une date passée. Utilisez le filtre « À replanifier » pour les retrouver.").arg(metrics.overdue)
        : "Aucun service planifié en retard. Les statuts et les indicateurs sont mis à jour après chaque modification.");
    m_attention->setVisible(metrics.overdue>0);
}

void ServiceWidget::focusSearch() { m_search->setFocus(); m_search->selectAll(); }

void ServiceWidget::onAdd() {
    openEditor(Service{}, false);
}

void ServiceWidget::openEditor(const Service &service, bool readOnly) {
    if(m_pages->currentWidget()!=m_listPage)return;
    auto *scroll=new QScrollArea(m_pages);
    scroll->setWidgetResizable(true);
    scroll->setObjectName("serviceEditorPage");
    auto *form=new ServiceDialog(service,readOnly,scroll);
    form->setWindowFlags(Qt::Widget);
    form->setModal(false);
    scroll->setWidget(form);
    m_pages->addWidget(scroll);
    m_pages->setCurrentWidget(scroll);
    auto close=[this,scroll]{m_pages->setCurrentWidget(m_listPage);m_pages->removeWidget(scroll);scroll->deleteLater();};
    connect(form,&QDialog::rejected,this,close);
    connect(form,&QDialog::accepted,this,[this,form,close] {
        Service entity=form->getService();
        const bool adding=entity.getId()==0;
        bool test;
        if (adding)
            test = entity.ajouter();
        else
            test = entity.modifier();

        if (test) {
            m_selectedId = entity.getId();
            close();
            refresh(); // Reexecute afficher() et setModel(), comme dans l'atelier.
            emit servicesChanged();
            notify(adding ? "Le service a été ajouté." : "Les modifications ont été enregistrées.");
        } else {
            form->show();
            form->showError(entity.lastError());
        }
    });
    form->show();
}
void ServiceWidget::viewService(int id) {
    for(const auto &service:m_services) if(service.getId()==id){openEditor(service,true);return;}
}
void ServiceWidget::editSelected() {
    const auto service=selectedService();if(service.getId()>0)openEditor(service);
}

void ServiceWidget::showInvoice() {
    Service service=selectedService();
    if(!service.getId() || service.getStatus()=="Annulé") return;
    if(service.getClientName().trimmed().isEmpty()) {
        notify("Complétez le nom du client dans « Modifier » pour exporter sa facture.",true);
        return;
    }
    const bool demo=property("demoMode").toBool();
    if(!demo && OracleConnection::isOracle()) {
        Service stored;QString error;
        if(!Invoice::loadStored(service,&stored,&error)){notify(error,true);return;}
        service=stored;
    }
    QDialog dialog(this); dialog.setObjectName("invoiceDialog");
    dialog.setWindowTitle("ANIWORLD — "+service.getInvoiceNumber()); dialog.resize(680,720);
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(20,18,20,18); layout->setSpacing(12);
    layout->addWidget(Design::label("Facture de paiement","title"));
    layout->addWidget(Design::label("Créée automatiquement · "+service.getInvoiceNumber(),"muted"));
    auto *preview=new QTextBrowser; preview->setObjectName("invoicePreview"); preview->setOpenLinks(false);
    Service invoice=service; QString invoiceError;
    const bool loaded=demo || Invoice::loadStored(service,&invoice,&invoiceError);
    preview->setHtml(loaded?Invoice::html(invoice,demo):Invoice::escape(invoiceError)); preview->setStyleSheet("QTextBrowser { background:white; border:1px solid #dce9ed; border-radius:6px; padding:12px; }");
    layout->addWidget(preview,1);
    auto *message=Design::label("Le PDF reprend les informations actuelles du service et les conserve dans le fichier exporté. Il ne constitue pas une preuve de paiement.","muted"); message->setObjectName("invoiceExportNotice"); message->setWordWrap(true); layout->addWidget(message);
    auto *buttons=new QHBoxLayout; buttons->addStretch();
    auto *close=Design::button("Fermer"); connect(close,&QPushButton::clicked,&dialog,&QDialog::reject); buttons->addWidget(close);
    auto *save=Design::button("Exporter PDF","export","primary"); save->setObjectName("exportInvoicePdf"); buttons->addWidget(save); layout->addLayout(buttons);
    connect(save,&QPushButton::clicked,&dialog,[&dialog,message,service,demo] {
        QString path=QFileDialog::getSaveFileName(&dialog,"Enregistrer la facture",service.getInvoiceNumber()+".pdf","Document PDF (*.pdf)");
        if(path.isEmpty()) return;
        if(!path.endsWith(".pdf",Qt::CaseInsensitive)) path+=".pdf";
        QString error;
        if(!Invoice::writePdf(path,service,demo,&error)) { message->setText("Export impossible : "+error); return; }
        message->setText("Facture PDF enregistrée : "+path);
    });
    dialog.exec();
}

void ServiceWidget::advanceStatus() {
    Service service = selectedService();
    if (!service.getId() || (service.getStatus()!="Planifié" && service.getStatus()!="En cours")) return;
    service.setStatus( service.getStatus()=="Planifié" ? "En cours" : "Terminé");
    QString error;
    if (!service.modifier(&error)) {
        notify("Statut non enregistré : "+error,true); return;
    }
    refresh(); emit servicesChanged(); notify(service.getName()+" : "+service.getStatus()+".");
}

void ServiceWidget::deleteSelected() {
    const Service service = selectedService();
    if (!service.getId()) return;
    QMessageBox confirm(QMessageBox::Question,"Supprimer le service",
                        "Supprimer « "+service.getName()+" » ?\nCette action est définitive.",QMessageBox::NoButton,this);
    confirm.setTextFormat(Qt::PlainText);
    auto *cancel = confirm.addButton("Conserver",QMessageBox::RejectRole);
    auto *remove = confirm.addButton("Supprimer",QMessageBox::DestructiveRole);
    confirm.setDefaultButton(cancel);
    confirm.exec();
    if (confirm.clickedButton()!=remove) return;
    const int id = service.getId();
    const bool test = Stmp.supprimer(id);
    if (test) {
        refresh(); // Reexecute afficher() et setModel() apres suppression reussie.
        emit servicesChanged();
        notify("Le service « "+service.getName()+" » a été supprimé.");
    } else {
        notify("Suppression impossible : "+Stmp.lastError(),true);
    }
}

void ServiceWidget::notify(const QString &message, bool error) {
    m_noticeTimer->stop();
    m_notice->setText(message);
    m_notice->setStyleSheet(error ? "color:#A54D4A;background:#FBECEB;padding:10px;border-radius:7px;"
                                 : "color:#42745E;background:#E8F3EC;padding:10px;border-radius:7px;");
    m_notice->show();
    ServiceMotion::reveal(m_notice,error?QColor("#FBECEB"):QColor("#E8F3EC"));
    if (!error) m_noticeTimer->start(5000);
}

QString ServiceWidget::filterDescription() const {
    QStringList scope;
    if (property("demoMode").toBool()) scope << "Données de démonstration";
    scope << m_category->currentText() << m_status->currentText() << m_period->currentText();
    if (!m_search->text().trimmed().isEmpty()) scope << "Recherche : "+m_search->text().trimmed();
    return scope.join(" · ");
}

void ServiceWidget::showPlanning() {
    const auto service=selectedService();
    ServicePlanningDialog dialog(service.getId()>0?service.getDateTime().date():QDate::currentDate(),this);
    connect(&dialog,&ServicePlanningDialog::servicesChanged,this,[this]{refresh();emit servicesChanged();});
    dialog.exec();
}

void ServiceWidget::showStatistics() {
    const auto metrics=ServiceMetrics::from(m_filtered);
    QDialog dialog(this); dialog.setObjectName("serviceStatisticsDialog");
    dialog.setWindowTitle("ANIWORLD — Statistiques des services"); dialog.resize(780,620);
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(24,22,24,22); layout->setSpacing(14);
    layout->addWidget(Design::label("Statistiques des services","title"));
    auto *scope=Design::label("Répartition et tarifs · "+filterDescription(),"muted"); scope->setWordWrap(true); layout->addWidget(scope);
    double total=0; for(const auto &s:m_filtered) total+=s.getPrice();
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
    auto *table=new QTableWidget; table->setObjectName("serviceStatisticsTable"); table->setMinimumHeight(240);
    Design::setupTable(table,{"CATÉGORIE","PRESTATIONS","TARIF MOYEN","PART"}); table->setAlternatingRowColors(true);
    table->setStyleSheet("QTableWidget { alternate-background-color:#f2f7f9; border:1px solid #dbe9ee; border-radius:6px; }");
    table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    for(int col=1;col<4;++col) { table->horizontalHeader()->setSectionResizeMode(col,QHeaderView::Fixed); table->setColumnWidth(col,135); }
    table->setRowCount(metrics.categories.size()); int row=0;
    for(auto it=metrics.categories.cbegin();it!=metrics.categories.cend();++it,++row) {
        double amount=0; for(const auto &service:m_filtered) if(service.getCategory()==it.key()) amount+=service.getPrice();
        table->setItem(row,0,new QTableWidgetItem(it.key())); table->setItem(row,1,new QTableWidgetItem(QString::number(it.value())));
        table->setItem(row,2,new QTableWidgetItem(Design::money(amount/it.value())));
        auto *part=new QWidget; auto *line=new QHBoxLayout(part); line->setContentsMargins(10,0,10,0);
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
