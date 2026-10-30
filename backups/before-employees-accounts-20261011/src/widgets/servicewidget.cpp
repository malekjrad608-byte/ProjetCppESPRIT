#include "servicewidget.h"
#include "servicecard.h"
#include "../database/databasemanager.h"
#include "../dialogs/servicedialog.h"
#include "../models/serviceanalytics.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include "../ui/servicereport.h"
#include "../ui/invoice.h"
#include <QTextBrowser>
#include <QPainter>
#include <QPdfDocument>
#include <QTemporaryDir>
#include <QScreen>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QSaveFile>
#include <QMenu>
#include <QButtonGroup>
#include <QSignalBlocker>
#include <QScrollBar>
#include <QTimer>
#include <QProgressBar>
#include <QLocale>
#include <algorithm>

namespace {
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

ServiceWidget::ServiceWidget(QWidget *parent) : QWidget(parent) {
    setObjectName("servicesPage");
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24,20,24,16);
    root->setSpacing(12);
    auto *head = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    titles->setSpacing(4);
    auto *titleRow=new QHBoxLayout; titleRow->setSpacing(9);
    auto *titleIcon=Design::label(""); titleIcon->setPixmap(Design::icon("services",QColor("#8268AE")).pixmap(24,24));
    titleRow->addWidget(titleIcon); titleRow->addWidget(Design::label("Services","title")); titleRow->addStretch(); titles->addLayout(titleRow);
    titles->addWidget(Design::label("Prestations, suivi et organisation des soins", "muted"));
    head->addLayout(titles);
    head->addStretch();
    m_export = Design::button("Rapport PDF", "export");
    m_export->setToolTip("Prévisualiser le rapport des services affichés, avec les filtres et le tri actuels.");
    m_export->setObjectName("exportServices");
    connect(m_export,&QPushButton::clicked,this,&ServiceWidget::exportPdf);
    head->addWidget(m_export);
    auto *add = Design::button("Ajouter un service", "plus", "primary");
    add->setObjectName("addService");
    add->setToolTip("Ajouter une prestation · Ctrl+N");
    connect(add, &QPushButton::clicked, this, &ServiceWidget::onAdd);
    head->addWidget(add);
    root->addLayout(head);

    auto *summary = new QHBoxLayout;
    summary->setSpacing(10);
    const QStringList backgrounds = {"#E8EAF8", "#FFF0D9", "#E5F2E9", "#E7F0F4"};
    const QStringList colors = {"#706BA7", "#AC7B30", "#4C896A", "#4D7989"};
    for (int i=0; i<4; ++i) {
        auto *chip = Design::label("");
        chip->setStyleSheet(QString("background:%1;color:%2;border-radius:14px;padding:6px 12px;font-size:11px;").arg(backgrounds[i],colors[i]));
        m_summary.append(chip);
        summary->addWidget(chip);
    }
    summary->addStretch();
    auto *statistics = Design::button("Voir les statistiques", "grid");
    statistics->setObjectName("serviceStatistics");
    connect(statistics, &QPushButton::clicked, this, &ServiceWidget::showStatistics);
    summary->addWidget(statistics);
    root->addLayout(summary);

    m_notice = Design::label("");
    m_notice->setObjectName("serviceNotice");
    m_notice->setWordWrap(true);
    m_notice->hide();
    root->addWidget(m_notice);
    m_noticeTimer=new QTimer(this); m_noticeTimer->setSingleShot(true);
    connect(m_noticeTimer,&QTimer::timeout,m_notice,&QWidget::hide);

    auto *filters = new QHBoxLayout;
    filters->setSpacing(10);
    m_search = new QLineEdit;
    m_search->setObjectName("serviceSearch");
    m_search->setPlaceholderText("Rechercher par service, client ou référence…");
    m_search->setClearButtonEnabled(true);
    m_search->addAction(Design::icon("search"), QLineEdit::LeadingPosition);
    filters->addWidget(m_search, 1);
    m_category = new QComboBox;
    m_category->setObjectName("categoryFilter");
    m_category->addItem("Toutes les catégories");
    m_category->addItems(Design::categories());
    filters->addWidget(m_category);
    m_status = new QComboBox;
    m_status->setObjectName("statusFilter");
    m_status->addItem("Tous les statuts");
    m_status->addItems(Design::statuses());
    filters->addWidget(m_status);
    root->addLayout(filters);

    auto *tools = new QHBoxLayout;
    tools->setSpacing(10);
    m_period = new QComboBox;
    m_period->setObjectName("periodFilter");
    m_period->addItems({"Toutes les dates", "Aujourd'hui", "7 prochains jours", "À replanifier"});
    tools->addWidget(m_period);
    m_sort = new QComboBox;
    m_sort->setObjectName("sortServices");
    m_sort->addItems({"Plus récents", "Plus anciens", "Nom : A → Z", "Tarif croissant", "Tarif décroissant", "Durée croissante", "Durée décroissante"});
    m_sort->setAccessibleName("Trier les services par date, tarif ou durée");
    tools->addWidget(Design::label("Trier par", "muted"));
    tools->addWidget(m_sort);
    auto *reset = Design::button("Réinitialiser", {}, "link");
    reset->setObjectName("resetFilters");
    tools->addWidget(reset);
    tools->addStretch();
    m_count = Design::label("", "muted");
    m_count->setObjectName("serviceCount");
    tools->addWidget(m_count);
    auto *viewGroup = new QButtonGroup(this);
    const QStringList viewNames = {"Cartes", "Liste"};
    for (int i=0; i<2; ++i) {
        auto *toggle = Design::button(viewNames[i], i==0 ? "grid" : "services", "toggle");
        toggle->setObjectName(i==0 ? "cardsView" : "tableView");
        toggle->setCheckable(true);
        toggle->setChecked(i==0);
        viewGroup->addButton(toggle,i);
        tools->addWidget(toggle);
        connect(toggle,&QPushButton::clicked,this,[this,i] {
            m_viewMode = i;
            m_views->setCurrentIndex(m_filtered.isEmpty() ? 2 : i);
            Motion::reveal(m_views,QColor("#F3F7F8"));
        });
    }
    root->addLayout(tools);

    auto *body = new QHBoxLayout;
    body->setSpacing(17);
    m_views = new QStackedWidget;
    m_gridScroll = new QScrollArea;
    m_gridScroll->setWidgetResizable(true);
    m_gridScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *cards = new QWidget;
    m_grid = new QGridLayout(cards);
    m_grid->setContentsMargins(0,0,5,0);
    m_grid->setSpacing(12);
    m_grid->setAlignment(Qt::AlignTop);
    m_grid->setSizeConstraint(QLayout::SetMinimumSize);
    m_gridScroll->setWidget(cards);
    cards->setAutoFillBackground(false);
    m_gridScroll->viewport()->setAutoFillBackground(false);
    m_views->addWidget(m_gridScroll);

    m_table = new QTableWidget;
    m_table->setObjectName("serviceTable");
    Design::setupTable(m_table, {"SERVICE", "CATÉGORIE", "DATE / HEURE", "TARIF", "STATUT", ""});
    m_table->setColumnHidden(1,true);
    m_table->setColumnHidden(5,true);
    for (int col : {2,3,4}) m_table->horizontalHeader()->setSectionResizeMode(col,QHeaderView::Fixed);
    m_table->setColumnWidth(2,139);
    m_table->setColumnWidth(3,82);
    m_table->setColumnWidth(4,108);
    m_table->horizontalHeader()->setMinimumSectionSize(80);
    m_views->addWidget(m_table);

    auto *emptyCard = Design::card();
    auto *empty = new QVBoxLayout(emptyCard);
    empty->setContentsMargins(30,30,30,30);
    empty->setSpacing(12);
    empty->addStretch();
    auto *logo = Design::label("");
    logo->setPixmap(Design::brand("logo",QSize(86,86)));
    logo->setAlignment(Qt::AlignCenter);
    empty->addWidget(logo);
    m_emptyTitle = Design::label("", "section");
    m_emptyTitle->setAlignment(Qt::AlignCenter);
    m_emptyTitle->setWordWrap(true);
    empty->addWidget(m_emptyTitle);
    m_emptyText = Design::label("", "muted");
    m_emptyText->setWordWrap(true);
    m_emptyText->setAlignment(Qt::AlignCenter);
    empty->addWidget(m_emptyText);
    auto *first = Design::button("Ajouter un service", "plus", "primary");
    empty->addWidget(first,0,Qt::AlignCenter);
    connect(first,&QPushButton::clicked,this,&ServiceWidget::onAdd);
    empty->addStretch();
    m_views->addWidget(emptyCard);
    body->addWidget(m_views,1);
    setupDetailPanel(body);
    root->addLayout(body,1);

    auto *footer = new QHBoxLayout;
    footer->addWidget(Design::label("ANIWORLD · Gestion des services", "muted"));
    footer->addStretch();
    footer->addWidget(Design::label("Ctrl+N  Ajouter   ·   Ctrl+F  Rechercher", "muted"));
    root->addLayout(footer);
    connect(m_search,&QLineEdit::textChanged,this,&ServiceWidget::filter);
    for (auto *combo : {m_status,m_category,m_sort,m_period})
        connect(combo,&QComboBox::currentIndexChanged,this,&ServiceWidget::filter);
    connect(reset,&QPushButton::clicked,this,[this] {
        const QSignalBlocker a(m_search), b(m_status), c(m_category), d(m_period), e(m_sort);
        m_search->clear(); m_status->setCurrentIndex(0); m_category->setCurrentIndex(0);
        m_period->setCurrentIndex(0); m_sort->setCurrentIndex(0); filter();
    });
    connect(m_table,&QTableWidget::itemSelectionChanged,this,[this] {
        const int row = m_table->currentRow();
        if (row>=0 && m_table->item(row,0)) selectService(m_table->item(row,0)->data(Qt::UserRole).toInt());
    });
    connect(m_table,&QTableWidget::cellDoubleClicked,this,[this](int row,int) {
        viewService(m_table->item(row,0)->data(Qt::UserRole).toInt());
    });
    auto *timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,[this] {
        updateSummary();
        if (m_period->currentIndex()>0) filter(); else updateDetail();
    });
    timer->start(60000);
    refresh();
}

void ServiceWidget::setupDetailPanel(QHBoxLayout *body) {
    auto *panel = new QWidget;
    panel->setFixedWidth(288);
    auto *side = new QVBoxLayout(panel);
    side->setContentsMargins(0,0,3,0);
    side->setSpacing(14);
    auto *detail = Design::card();
    detail->setObjectName("serviceDetailPanel");
    auto *layout = new QVBoxLayout(detail);
    layout->setContentsMargins(18,17,18,17);
    layout->setSpacing(8);
    layout->addWidget(Design::label("SERVICE SÉLECTIONNÉ", "eyebrow"));
    m_detailName = Design::label("Sélectionnez un service");
    m_detailName->setObjectName("selectedServiceName");
    m_detailName->setStyleSheet("color:#306C7B;font-size:20px;font-weight:650;");
    m_detailName->setWordWrap(true);
    layout->addWidget(m_detailName);
    m_detailReference = Design::label("", "muted");
    layout->addWidget(m_detailReference);
    m_detailContent = new QWidget;
    auto *content = new QVBoxLayout(m_detailContent);
    content->setContentsMargins(0,0,0,0);
    content->setSpacing(8);
    m_statusLayout = new QVBoxLayout;
    m_statusLayout->setContentsMargins(0,0,0,0);
    content->addLayout(m_statusLayout);
    auto *separator=new QFrame; separator->setFixedHeight(1); separator->setStyleSheet("background:#E0EDF1;border:none;"); content->addWidget(separator);
    auto *infoScroll = new QScrollArea;
    infoScroll->setObjectName("serviceInfoScroll");
    infoScroll->setWidgetResizable(true);
    infoScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    infoScroll->setMinimumHeight(40);
    auto *info = new QWidget;
    auto *infoLayout = new QVBoxLayout(info);
    infoLayout->setContentsMargins(0,7,4,7);
    infoLayout->setSpacing(12);
    const QStringList fields = {"Catégorie", "Date", "Horaire", "Durée", "Responsable", "Client", "Tarif", "Facture"};
    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(10);
    for (int i=0; i<fields.size(); ++i) {
        grid->addWidget(Design::label(fields[i],"muted"),i,0,Qt::AlignTop);
        auto *value = Design::label("");
        value->setWordWrap(true);
        value->setStyleSheet("font-size:12px;color:#44636F;");
        m_detailValues.append(value);
        grid->addWidget(value,i,1);
    }
    grid->setColumnStretch(1,1);
    infoLayout->addLayout(grid);
    m_detailNotes = Design::label("", "muted");
    m_detailNotes->setWordWrap(true);
    m_detailNotes->setMaximumHeight(72);
    infoLayout->addWidget(m_detailNotes);
    infoLayout->addStretch();
    infoScroll->setWidget(info);
    info->setStyleSheet("background: white;");
    content->addWidget(infoScroll,1);
    m_advance = Design::button("Démarrer le service", "check", "success");
    m_advance->setObjectName("advanceService");
    connect(m_advance,&QPushButton::clicked,this,&ServiceWidget::advanceStatus);
    m_edit = Design::button("Modifier", "edit", "primary");
    m_edit->setObjectName("editService");
    m_edit->setIcon(Design::icon("edit",QColor("#FFB454")));
    m_view = Design::button("Consulter", "eye");
    m_view->setObjectName("viewService");
    m_invoice = Design::button("Facture PDF", "export");
    m_invoice->setObjectName("invoiceService");
    m_delete = Design::button("Supprimer", "trash", "danger");
    m_delete->setObjectName("deleteService");
    auto *documents=new QHBoxLayout; documents->setSpacing(7); documents->addWidget(m_view); documents->addWidget(m_invoice);
    content->addLayout(documents);
    content->addWidget(m_edit); content->addWidget(m_advance); content->addWidget(m_delete);
    for(auto *button:{m_edit,m_advance,m_delete}) {
        button->setProperty("detailAction",true); button->setMinimumHeight(40);
    }
    for(auto *button:{m_view,m_invoice}) button->setStyleSheet("padding:6px 8px;");
    connect(m_edit,&QPushButton::clicked,this,&ServiceWidget::editSelected);
    connect(m_view,&QPushButton::clicked,this,[this] { viewService(m_selectedId); });
    connect(m_invoice,&QPushButton::clicked,this,&ServiceWidget::showInvoice);
    connect(m_delete,&QPushButton::clicked,this,&ServiceWidget::deleteSelected);
    layout->addWidget(m_detailContent,1);
    m_emptyDetailSpacer = new QWidget;
    layout->addWidget(m_emptyDetailSpacer,1);
    side->addWidget(detail,1);
    m_attention = Design::label("", "muted");
    m_attention->setWordWrap(true);
    m_attention->setStyleSheet("color:#947039;background:#FFF0D9;border-radius:8px;padding:12px;font-size:11px;");
    side->addWidget(m_attention);
    body->addWidget(panel);
}

void ServiceWidget::refresh() {
    auto &db = DatabaseManager::instance();
    m_services = db.getAllServices();
    if (!db.lastError().isEmpty()) notify("Impossible de charger les services : " + db.lastError(),true);
    updateSummary();
    filter();
}

void ServiceWidget::filter() {
    m_filtered.clear();
    const QString text = folded(m_search->text().trimmed());
    const QDateTime now = QDateTime::currentDateTime();
    for (const auto &s : m_services) {
        const bool matchesText = text.isEmpty() || folded(s.name+" "+s.description+" "+s.responsible+" "+s.clientName+" "+s.invoiceNumber+" "+s.category+" "+reference(s.id)+" "+QString::number(s.id)).contains(text);
        bool matchesPeriod = true;
        if (m_period->currentIndex()==1) matchesPeriod = s.dateTime.date()==now.date();
        if (m_period->currentIndex()==2) matchesPeriod = s.dateTime.date()>=now.date() && s.dateTime.date()<=now.date().addDays(6);
        if (m_period->currentIndex()==3) matchesPeriod = s.status=="Planifié" && s.dateTime<now;
        if (matchesText && matchesPeriod && (m_status->currentIndex()==0 || s.status==m_status->currentText())
            && (m_category->currentIndex()==0 || s.category==m_category->currentText())) m_filtered.append(s);
    }
    const int sort = m_sort->currentIndex();
    std::stable_sort(m_filtered.begin(),m_filtered.end(),[sort](const Service &a,const Service &b) {
        if (sort==1) return a.dateTime<b.dateTime;
        if (sort==2) return QString::localeAwareCompare(a.name,b.name)<0;
        if (sort==3) return a.price<b.price;
        if (sort==4) return a.price>b.price;
        if (sort==5) return a.duration<b.duration;
        if (sort==6) return a.duration>b.duration;
        return a.dateTime>b.dateTime;
    });
    const bool selectedVisible = std::any_of(m_filtered.begin(),m_filtered.end(),[this](const Service &s) { return s.id==m_selectedId; });
    if (!selectedVisible) m_selectedId = m_filtered.isEmpty() ? 0 : m_filtered.first().id;
    populate();
}

void ServiceWidget::populate() {
    const QSignalBlocker blocker(m_table);
    m_table->setRowCount(m_filtered.size());
    for (int row=0; row<m_filtered.size(); ++row) {
        const auto &s = m_filtered[row];
        auto *name = new QTableWidgetItem(s.name);
        name->setData(Qt::UserRole,s.id);
        name->setToolTip(s.name+"\n"+reference(s.id)+" · "+s.category);
        m_table->setItem(row,0,name);
        m_table->setItem(row,1,new QTableWidgetItem(s.category));
        m_table->setItem(row,2,new QTableWidgetItem(s.dateTime.toString("dd/MM/yyyy\nHH:mm")));
        m_table->setItem(row,3,new QTableWidgetItem(Design::money(s.price)));
        auto *state = new QTableWidgetItem;
        state->setData(Qt::AccessibleTextRole,s.status);
        m_table->setItem(row,4,state);
        m_table->setCellWidget(row,4,Design::badge(s.status));
        m_table->setRowHeight(row,65);
        if (s.id==m_selectedId) m_table->selectRow(row);
    }
    m_count->setText(QString("%1 / %2 services").arg(m_filtered.size()).arg(m_services.size()));
    const bool empty = m_services.isEmpty();
    m_emptyTitle->setText(empty ? "Votre première prestation" : "Aucun service trouvé");
    m_emptyText->setText(empty ? "Ajoutez un service pour organiser vos soins.\nVos données seront conservées sur cet ordinateur."
                              : "Modifiez votre recherche ou réinitialisez les filtres.");
    m_views->setCurrentIndex(m_filtered.isEmpty() ? 2 : m_viewMode);
    m_export->setEnabled(!m_filtered.isEmpty());
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
        card->setChecked(service.id==m_selectedId);
        connect(card,&QAbstractButton::clicked,this,[this,id=service.id] { selectService(id); });
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
    const QSignalBlocker blocker(m_table);
    for (int row=0; row<m_table->rowCount(); ++row)
        if (m_table->item(row,0)->data(Qt::UserRole).toInt()==id) { m_table->selectRow(row); break; }
    updateDetail();
    if(changed) Motion::reveal(findChild<QWidget *>("serviceDetailPanel"));
}

Service ServiceWidget::selectedService() const {
    for (const auto &service : m_services) if (service.id==m_selectedId) return service;
    return {};
}

void ServiceWidget::updateDetail() {
    const Service s = selectedService();
    const bool selected = s.id>0;
    m_detailContent->setVisible(selected);
    m_emptyDetailSpacer->setVisible(!selected);
    for (auto *button : {m_edit,m_delete,m_view,m_invoice,m_advance}) button->setEnabled(selected);
    m_detailName->setText(selected ? s.name : "Sélectionnez un service");
    m_detailReference->setText(selected ? reference(s.id) : "Les informations apparaîtront ici.");
    m_invoice->setEnabled(selected && s.status!="Annulé");
    m_invoice->setToolTip(s.status=="Annulé"?"Un service annulé ne peut pas être facturé.":"Facture créée automatiquement avec les informations du service.");
    if (!selected) return;
    while (auto *item = m_statusLayout->takeAt(0)) { delete item->widget(); delete item; }
    auto *statusBadge=Design::badge(s.status);
    auto *badgeLayout=statusBadge->layout(); badgeLayout->setContentsMargins(0,0,0,0);
    delete badgeLayout->takeAt(1);
    if(auto *text=qobject_cast<QLabel *>(badgeLayout->itemAt(0)->widget())) {
        text->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        QString style=text->styleSheet(); style.replace("border-radius:10px","border-radius:4px"); text->setStyleSheet(style);
    }
    m_statusLayout->addWidget(statusBadge);
    const QDateTime end = s.dateTime.addSecs(s.duration*60);
    const QString timing = s.dateTime.toString("HH:mm")+" → "+end.toString(end.date()==s.dateTime.date()?"HH:mm":"dd/MM HH:mm");
    const QStringList values = {s.category,QLocale().toString(s.dateTime.date(),"dd MMMM yyyy"),timing,
                               QString::number(s.duration)+" min",s.responsible.isEmpty()?"Non attribué":s.responsible,s.clientName.isEmpty()?"À compléter":s.clientName,Design::money(s.price),s.invoiceNumber};
    for (int i=0; i<values.size(); ++i) m_detailValues[i]->setText(values[i]);
    const QString notes = s.description.isEmpty() ? "Aucune note pour cette prestation." : s.description;
    m_detailNotes->setText(notes.size()>150 ? notes.left(147)+"…" : notes);
    m_detailNotes->setToolTip(notes);
    m_advance->setVisible(s.status=="Planifié" || s.status=="En cours");
    m_advance->setText(s.status=="Planifié" ? "Démarrer le service" : "Marquer comme terminé");
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
    ServiceDialog dialog(this);
    while (dialog.exec()==QDialog::Accepted) {
        if (DatabaseManager::instance().addService(dialog.getService())) {
            m_selectedId = DatabaseManager::instance().lastInsertedId();
            refresh(); emit servicesChanged(); notify("Le service et sa facture ont été créés automatiquement."); return;
        }
        QMessageBox::critical(this,"Enregistrement impossible",DatabaseManager::instance().lastError());
    }
}

void ServiceWidget::viewService(int id) {
    for (const auto &service : m_services) if (service.id==id) {
        ServiceDialog dialog(service,true,this); dialog.exec(); return;
    }
}

void ServiceWidget::editSelected() {
    const Service service = selectedService();
    if (!service.id) return;
    ServiceDialog dialog(service,false,this);
    while (dialog.exec()==QDialog::Accepted) {
        if (DatabaseManager::instance().updateService(dialog.getService())) {
            refresh(); emit servicesChanged(); notify("Les modifications ont été enregistrées."); return;
        }
        QMessageBox::critical(this,"Enregistrement impossible",DatabaseManager::instance().lastError());
    }
}

void ServiceWidget::showInvoice() {
    const Service service=selectedService();
    if(!service.id || service.status=="Annulé") return;
    if(service.clientName.trimmed().isEmpty()) {
        notify("Complétez le nom du client dans « Modifier » pour exporter sa facture.",true);
        return;
    }
    const bool demo=property("demoMode").toBool();
    QDialog dialog(this); dialog.setObjectName("invoiceDialog");
    dialog.setWindowTitle("ANIWORLD — "+service.invoiceNumber); dialog.resize(680,720);
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(20,18,20,18); layout->setSpacing(12);
    layout->addWidget(Design::label("Facture de paiement","title"));
    layout->addWidget(Design::label("Créée automatiquement · "+service.invoiceNumber,"muted"));
    auto *preview=new QTextBrowser; preview->setObjectName("invoicePreview"); preview->setOpenLinks(false);
    preview->setHtml(Invoice::html(service,demo)); preview->setStyleSheet("QTextBrowser { background:white; border:1px solid #dce9ed; border-radius:6px; padding:12px; }");
    layout->addWidget(preview,1);
    auto *message=Design::label("","muted"); message->setObjectName("invoiceExportNotice"); message->setWordWrap(true); layout->addWidget(message);
    auto *buttons=new QHBoxLayout; buttons->addStretch();
    auto *close=Design::button("Fermer"); connect(close,&QPushButton::clicked,&dialog,&QDialog::reject); buttons->addWidget(close);
    auto *save=Design::button("Exporter PDF","export","primary"); save->setObjectName("exportInvoicePdf"); buttons->addWidget(save); layout->addLayout(buttons);
    connect(save,&QPushButton::clicked,&dialog,[&dialog,message,service,demo] {
        QString path=QFileDialog::getSaveFileName(&dialog,"Enregistrer la facture",service.invoiceNumber+".pdf","Document PDF (*.pdf)");
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
    if (!service.id || (service.status!="Planifié" && service.status!="En cours")) return;
    service.status = service.status=="Planifié" ? "En cours" : "Terminé";
    if (!DatabaseManager::instance().updateService(service)) {
        notify("Statut non enregistré : "+DatabaseManager::instance().lastError(),true); return;
    }
    refresh(); emit servicesChanged(); notify(service.name+" : "+service.status+".");
}

void ServiceWidget::deleteSelected() {
    const Service service = selectedService();
    if (!service.id) return;
    QMessageBox confirm(QMessageBox::Question,"Supprimer le service",
                        "Supprimer « "+service.name+" » ?\nCette action est définitive.",QMessageBox::NoButton,this);
    confirm.setTextFormat(Qt::PlainText);
    auto *cancel = confirm.addButton("Conserver",QMessageBox::RejectRole);
    auto *remove = confirm.addButton("Supprimer",QMessageBox::DestructiveRole);
    confirm.setDefaultButton(cancel);
    confirm.exec();
    if (confirm.clickedButton()!=remove) return;
    if (!DatabaseManager::instance().deleteService(service.id)) {
        notify("Suppression impossible : "+DatabaseManager::instance().lastError(),true); return;
    }
    refresh(); emit servicesChanged(); notify("Le service « "+service.name+" » a été supprimé.");
}

void ServiceWidget::notify(const QString &message, bool error) {
    m_noticeTimer->stop();
    m_notice->setText(message);
    m_notice->setStyleSheet(error ? "color:#A54D4A;background:#FBECEB;padding:10px;border-radius:7px;"
                                 : "color:#42745E;background:#E8F3EC;padding:10px;border-radius:7px;");
    m_notice->show();
    Motion::reveal(m_notice,error?QColor("#FBECEB"):QColor("#E8F3EC"));
    if (!error) m_noticeTimer->start(5000);
}

QString ServiceWidget::filterDescription() const {
    QStringList scope;
    if (property("demoMode").toBool()) scope << "Données de démonstration";
    scope << m_category->currentText() << m_status->currentText() << m_period->currentText();
    if (!m_search->text().trimmed().isEmpty()) scope << "Recherche : "+m_search->text().trimmed();
    return scope.join(" · ");
}

void ServiceWidget::exportPdf() {
    if(m_filtered.isEmpty()) return;
    // Preview and save the same immutable PDF, including the current filter and order.
    QTemporaryDir temporary;
    if(!temporary.isValid()) { notify("Impossible de préparer l'aperçu PDF.",true); return; }
    const QString reportPath=temporary.filePath("rapport.pdf");
    const QString scope=filterDescription();
    QString error;
    if(!ServiceReport::writePdf(reportPath,m_filtered,scope,&error)) { notify("Rapport impossible : "+error,true); return; }
    QDialog dialog(this); dialog.setObjectName("reportPreviewDialog"); dialog.setWindowTitle("ANIWORLD — Rapport des services");
    const QSize available=screen()->availableGeometry().size()-QSize(60,70);
    dialog.resize(QSize(1050,760).boundedTo(available));
    QPdfDocument document(&dialog); document.setObjectName("reportDocument");
    if(document.load(reportPath)!=QPdfDocument::Error::None || document.pageCount()==0) { notify("Impossible d'afficher le rapport PDF.",true); return; }
    auto *layout=new QVBoxLayout(&dialog); layout->setContentsMargins(22,18,22,18); layout->setSpacing(10);
    layout->addWidget(Design::label("Rapport des services","title"));
    auto *scopeLabel=Design::label(QString("%1 services · %2").arg(m_filtered.size()).arg(scope),"muted");
    scopeLabel->setObjectName("reportScope"); scopeLabel->setWordWrap(true); layout->addWidget(scopeLabel);
    layout->addWidget(Design::label("Pour la facture d'un client, utilisez « Facture PDF » dans le service sélectionné.","muted"));
    auto *scroll=new QScrollArea; scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { background:#DEE9ED;border:1px solid #CDDFE5;border-radius:8px; }");
    auto *preview=new QLabel; preview->setObjectName("reportPageImage"); preview->setAlignment(Qt::AlignCenter); preview->setContentsMargins(10,10,10,10);
    scroll->setWidget(preview); layout->addWidget(scroll,1);
    auto *pager=new QHBoxLayout; pager->addStretch();
    auto *previous=Design::button("Précédente"); previous->setObjectName("reportPrevious"); pager->addWidget(previous);
    auto *pageLabel=Design::label("","muted"); pageLabel->setObjectName("reportPageNumber"); pager->addWidget(pageLabel);
    auto *next=Design::button("Suivante"); next->setObjectName("reportNext"); pager->addWidget(next); pager->addStretch(); layout->addLayout(pager);
    auto *message=Design::label("","muted"); message->setWordWrap(true); message->setObjectName("reportSaveNotice"); layout->addWidget(message);
    auto *buttons=new QHBoxLayout; buttons->addStretch();
    auto *close=Design::button("Fermer"); connect(close,&QPushButton::clicked,&dialog,&QDialog::reject); buttons->addWidget(close);
    auto *save=Design::button("Enregistrer le PDF","export","primary"); save->setObjectName("saveReportPdf"); buttons->addWidget(save); layout->addLayout(buttons);
    int currentPage=0;
    auto renderPage=[&] {
        const QSizeF points=document.pagePointSize(currentPage);
        const qreal dpr=dialog.devicePixelRatioF(); const int width=dialog.width()-90;
        const QImage image=document.render(currentPage,QSize(qRound(width*dpr),qRound(width*dpr*points.height()/points.width())));
        if(image.isNull()) { message->setText("L'aperçu de cette page n'a pas pu être affiché."); return; }
        QImage paper(image.size(),QImage::Format_ARGB32_Premultiplied);
        paper.fill(Qt::white);
        { QPainter painter(&paper); painter.drawImage(0,0,image); }
        QPixmap pixmap=QPixmap::fromImage(paper); pixmap.setDevicePixelRatio(dpr); preview->setPixmap(pixmap);
        pageLabel->setText(QString("Page %1 / %2").arg(currentPage+1).arg(document.pageCount()));
        previous->setEnabled(currentPage>0); next->setEnabled(currentPage+1<document.pageCount()); scroll->verticalScrollBar()->setValue(0);
    };
    connect(previous,&QPushButton::clicked,&dialog,[&] { if(currentPage>0) { --currentPage; renderPage(); } });
    connect(next,&QPushButton::clicked,&dialog,[&] { if(currentPage+1<document.pageCount()) { ++currentPage; renderPage(); } });
    connect(save,&QPushButton::clicked,&dialog,[&dialog,message,reportPath] {
        QString path=QFileDialog::getSaveFileName(&dialog,"Enregistrer le rapport des services","ANIWORLD-services-"+QDate::currentDate().toString("yyyy-MM-dd")+".pdf","Document PDF (*.pdf)");
        if(path.isEmpty()) return;
        if(!path.endsWith(".pdf",Qt::CaseInsensitive)) path+=".pdf";
        QFile source(reportPath); QSaveFile destination(path);
        if(!source.open(QIODevice::ReadOnly)) { message->setText("Lecture impossible : "+source.errorString()); return; }
        const QByteArray data=source.readAll();
        if(!destination.open(QIODevice::WriteOnly) || destination.write(data)!=data.size() || !destination.commit()) {
            message->setText("Enregistrement impossible : "+destination.errorString()); return;
        }
        message->setText("Rapport PDF enregistré : "+path);
    });
    renderPage(); dialog.exec();
}

void ServiceWidget::showStatistics() {
    const auto metrics=ServiceMetrics::from(m_filtered);
    QDialog dialog(this); dialog.setObjectName("serviceStatisticsDialog");
    dialog.setWindowTitle("ANIWORLD — Statistiques des services"); dialog.resize(780,620);
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
    auto *table=new QTableWidget; table->setObjectName("serviceStatisticsTable"); table->setMinimumHeight(240);
    Design::setupTable(table,{"CATÉGORIE","PRESTATIONS","TARIF MOYEN","PART"}); table->setAlternatingRowColors(true);
    table->setStyleSheet("QTableWidget { alternate-background-color:#f2f7f9; border:1px solid #dbe9ee; border-radius:6px; }");
    table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    for(int col=1;col<4;++col) { table->horizontalHeader()->setSectionResizeMode(col,QHeaderView::Fixed); table->setColumnWidth(col,135); }
    table->setRowCount(metrics.categories.size()); int row=0;
    for(auto it=metrics.categories.cbegin();it!=metrics.categories.cend();++it,++row) {
        double amount=0; for(const auto &service:m_filtered) if(service.category==it.key()) amount+=service.price;
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
