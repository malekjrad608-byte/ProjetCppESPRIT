#include "servicewidget.h"
#include "servicecard.h"
#include "../database/databasemanager.h"
#include "../dialogs/servicedialog.h"
#include "../models/serviceanalytics.h"
#include "../ui/design.h"
#include "../ui/servicereport.h"
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
QString csvCell(QString text) {
    const QString trimmed = text.trimmed();
    if (!trimmed.isEmpty() && QString("=+-@\t\r").contains(trimmed.at(0))) text.prepend('\'');
    text.replace('"',"\"\"");
    return '"' + text + '"';
}
QString hours(int minutes) {
    return QString("%1 h %2").arg(minutes/60).arg(minutes%60,2,10,QChar('0'));
}
}

ServiceWidget::ServiceWidget(QWidget *parent) : QWidget(parent) {
    setObjectName("servicesPage");
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28,24,28,19);
    root->setSpacing(16);
    auto *head = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    titles->setSpacing(4);
    titles->addWidget(Design::label("Services", "title"));
    titles->addWidget(Design::label("Prestations, suivi et organisation des soins", "muted"));
    head->addLayout(titles);
    head->addStretch();
    m_export = Design::button("Exporter", "export");
    m_export->setObjectName("exportServices");
    auto *exportMenu = new QMenu(m_export);
    exportMenu->addAction("Rapport PDF", this, &ServiceWidget::exportPdf);
    exportMenu->addAction("Tableau CSV", this, &ServiceWidget::exportCsv);
    m_export->setMenu(exportMenu);
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
        chip->setStyleSheet(QString("background:%1;color:%2;border-radius:8px;padding:11px 13px;font-size:12px;").arg(backgrounds[i],colors[i]));
        m_summary.append(chip);
        summary->addWidget(chip);
    }
    summary->addStretch();
    auto *statistics = Design::button("Statistiques", "grid");
    statistics->setObjectName("serviceStatistics");
    connect(statistics, &QPushButton::clicked, this, &ServiceWidget::showStatistics);
    summary->addWidget(statistics);
    root->addLayout(summary);

    m_notice = Design::label("");
    m_notice->setObjectName("serviceNotice");
    m_notice->setWordWrap(true);
    m_notice->hide();
    root->addWidget(m_notice);

    auto *filters = new QHBoxLayout;
    filters->setSpacing(10);
    m_search = new QLineEdit;
    m_search->setObjectName("serviceSearch");
    m_search->setPlaceholderText("Rechercher par nom, ID ou responsable…");
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
    m_sort->addItems({"Plus récents", "Plus anciens", "Nom : A → Z", "Tarif croissant"});
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
    m_grid->setSpacing(14);
    m_grid->setAlignment(Qt::AlignTop);
    m_grid->setSizeConstraint(QLayout::SetMinimumSize);
    m_gridScroll->setWidget(cards);
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
    panel->setFixedWidth(286);
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
    auto *infoScroll = new QScrollArea;
    infoScroll->setObjectName("serviceInfoScroll");
    infoScroll->setWidgetResizable(true);
    infoScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    infoScroll->setMinimumHeight(40);
    auto *info = new QWidget;
    auto *infoLayout = new QVBoxLayout(info);
    infoLayout->setContentsMargins(0,7,4,7);
    infoLayout->setSpacing(12);
    const QStringList fields = {"Catégorie", "Date", "Horaire", "Durée", "Responsable", "Tarif"};
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
    m_advance = Design::button("Démarrer le service", "check", "primary");
    m_advance->setObjectName("advanceService");
    content->addWidget(m_advance);
    connect(m_advance,&QPushButton::clicked,this,&ServiceWidget::advanceStatus);
    m_edit = Design::button("Modifier", "edit");
    m_edit->setObjectName("editService");
    m_view = Design::button("Consulter", "eye");
    m_view->setObjectName("viewService");
    auto *row = new QHBoxLayout;
    row->setSpacing(7); row->addWidget(m_view); row->addWidget(m_edit);
    content->addLayout(row);
    m_duplicate = Design::button("Dupliquer", "plus");
    m_duplicate->setObjectName("duplicateService");
    m_delete = Design::button("Supprimer", "trash", "danger");
    m_delete->setObjectName("deleteService");
    auto *lastRow = new QHBoxLayout;
    lastRow->setSpacing(7); lastRow->addWidget(m_duplicate); lastRow->addWidget(m_delete);
    content->addLayout(lastRow);
    for (auto *button : {m_advance,m_view,m_edit,m_duplicate,m_delete}) {
        button->setMinimumHeight(31);
        button->setStyleSheet("padding:6px 9px;");
    }
    connect(m_edit,&QPushButton::clicked,this,&ServiceWidget::editSelected);
    connect(m_view,&QPushButton::clicked,this,[this] { viewService(m_selectedId); });
    connect(m_duplicate,&QPushButton::clicked,this,&ServiceWidget::duplicateSelected);
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
        const bool matchesText = text.isEmpty() || folded(s.name+" "+s.description+" "+s.responsible+" "+s.category+" "+reference(s.id)+" "+QString::number(s.id)).contains(text);
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
    m_gridColumns = m_gridScroll->viewport()->width()>=620 ? 2 : 1;
    for (int i=0; i<m_filtered.size(); ++i) {
        const auto &service = m_filtered[i];
        auto *card = new ServiceCard(service);
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

void ServiceWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    QTimer::singleShot(0,this,[this] {
        const int columns = m_gridScroll->viewport()->width()>=620 ? 2 : 1;
        if (columns!=m_gridColumns) rebuildCards();
    });
}

void ServiceWidget::selectService(int id) {
    m_selectedId = id;
    for (auto *card : m_gridScroll->findChildren<QAbstractButton *>())
        card->setChecked(card->objectName()=="serviceCard_"+QString::number(id));
    const QSignalBlocker blocker(m_table);
    for (int row=0; row<m_table->rowCount(); ++row)
        if (m_table->item(row,0)->data(Qt::UserRole).toInt()==id) { m_table->selectRow(row); break; }
    updateDetail();
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
    for (auto *button : {m_edit,m_delete,m_view,m_duplicate,m_advance}) button->setEnabled(selected);
    m_detailName->setText(selected ? s.name : "Sélectionnez un service");
    m_detailReference->setText(selected ? reference(s.id) : "Les informations apparaîtront ici.");
    if (!selected) return;
    while (auto *item = m_statusLayout->takeAt(0)) { delete item->widget(); delete item; }
    m_statusLayout->addWidget(Design::badge(s.status));
    const QDateTime end = s.dateTime.addSecs(s.duration*60);
    const QString timing = s.dateTime.toString("HH:mm")+" → "+end.toString(end.date()==s.dateTime.date()?"HH:mm":"dd/MM HH:mm");
    const QStringList values = {s.category,QLocale().toString(s.dateTime.date(),"dd MMMM yyyy"),timing,
                               QString::number(s.duration)+" min",s.responsible.isEmpty()?"Non attribué":s.responsible,Design::money(s.price)};
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
            refresh(); emit servicesChanged(); notify("Le service a été ajouté et enregistré."); return;
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

void ServiceWidget::duplicateSelected() {
    Service copy = selectedService();
    if (!copy.id) return;
    copy.id=0; copy.name=copy.name.left(112)+" (copie)"; copy.status="Planifié";
    copy.dateTime=QDateTime::currentDateTime().addDays(1);
    ServiceDialog dialog(copy,false,this);
    while (dialog.exec()==QDialog::Accepted) {
        if (DatabaseManager::instance().addService(dialog.getService())) {
            m_selectedId = DatabaseManager::instance().lastInsertedId();
            refresh(); emit servicesChanged(); notify("La nouvelle prestation a été créée à partir du service sélectionné."); return;
        }
        QMessageBox::critical(this,"Enregistrement impossible",DatabaseManager::instance().lastError());
    }
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
    m_notice->setText(message);
    m_notice->setStyleSheet(error ? "color:#A54D4A;background:#FBECEB;padding:10px;border-radius:7px;"
                                 : "color:#42745E;background:#E8F3EC;padding:10px;border-radius:7px;");
    m_notice->show();
    if (!error) QTimer::singleShot(5000,this,[this,message] { if (m_notice->text()==message) m_notice->hide(); });
}

QString ServiceWidget::filterDescription() const {
    QStringList scope;
    if (property("demoMode").toBool()) scope << "Données de démonstration";
    scope << m_category->currentText() << m_status->currentText() << m_period->currentText();
    if (!m_search->text().trimmed().isEmpty()) scope << "Recherche : "+m_search->text().trimmed();
    return scope.join(" · ");
}

QString ServiceWidget::csvContent(const QList<Service> &services) {
    QString result=QString(QChar(0xFEFF))+"ID;Service;Catégorie;Responsable;Date;Statut;Durée (min);Tarif (DT);Description\r\n";
    for (const auto &s : services) {
        QStringList cells={QString::number(s.id),s.name,s.category,s.responsible,s.dateTime.toString("dd/MM/yyyy HH:mm"),s.status,QString::number(s.duration),QString::number(s.price,'f',2),s.description};
        for (auto &cell : cells) cell=csvCell(cell);
        result+=cells.join(';')+"\r\n";
    }
    return result;
}

void ServiceWidget::exportCsv() {
    const QString path=QFileDialog::getSaveFileName(this,"Exporter les résultats affichés","ANIWORLD-services-"+QDate::currentDate().toString("yyyy-MM-dd")+".csv","Fichier CSV (*.csv)");
    if (path.isEmpty()) return;
    QSaveFile file(path);
    const QByteArray content=csvContent(m_filtered).toUtf8();
    if (!file.open(QIODevice::WriteOnly) || file.write(content)!=content.size() || !file.commit()) {
        notify("Export impossible : "+file.errorString(),true); return;
    }
    notify(QString("%1 services exportés : %2").arg(m_filtered.size()).arg(path));
}

void ServiceWidget::exportPdf() {
    QString path=QFileDialog::getSaveFileName(this,"Exporter le rapport des services","ANIWORLD-services-"+QDate::currentDate().toString("yyyy-MM-dd")+".pdf","Document PDF (*.pdf)");
    if (path.isEmpty()) return;
    if (!path.endsWith(".pdf",Qt::CaseInsensitive)) path+=".pdf";
    QString error;
    if (!ServiceReport::writePdf(path,m_filtered,filterDescription(),&error)) { notify("Export impossible : "+error,true); return; }
    notify(QString("Rapport PDF enregistré : %1").arg(path));
}

void ServiceWidget::showStatistics() {
    const auto metrics = ServiceMetrics::from(m_filtered);
    QDialog dialog(this);
    dialog.setObjectName("serviceStatisticsDialog");
    dialog.setWindowTitle("ANIWORLD — Statistiques des services");
    dialog.setMinimumWidth(610);
    auto *layout=new QVBoxLayout(&dialog);
    layout->setContentsMargins(28,25,28,25);
    layout->setSpacing(18);
    layout->addWidget(Design::label("Statistiques des services","title"));
    auto *scope=Design::label("Sur les résultats affichés · "+filterDescription(),"muted");
    scope->setWordWrap(true); layout->addWidget(scope);
    auto *totals=new QHBoxLayout;
    const QStringList captions={"PRESTATIONS","VOLUME À RÉALISER","PRESTATIONS TERMINÉES"};
    const QStringList values={QString::number(metrics.total),hours(metrics.scheduledMinutes),Design::money(metrics.completedAmount)};
    for (int i=0;i<3;++i) {
        auto *card=Design::card(); auto *box=new QVBoxLayout(card); box->setContentsMargins(14,14,14,14);
        box->addWidget(Design::label(captions[i],"eyebrow"));
        auto *value=Design::label(values[i]); value->setStyleSheet("font-size:23px;font-weight:650;color:#377D8E;"); box->addWidget(value); totals->addWidget(card);
    }
    layout->addLayout(totals);
    layout->addWidget(Design::label("Le montant correspond aux services terminés, sans suivi des paiements.","muted"));
    auto *breakdown=Design::card(); auto *rows=new QVBoxLayout(breakdown); rows->setContentsMargins(20,18,20,18); rows->setSpacing(12);
    rows->addWidget(Design::label("Répartition des prestations","section"));
    const QList<int> statusCounts={metrics.planned,metrics.inProgress,metrics.completed,metrics.cancelled};
    const auto statuses=Design::statuses();
    for (int i=0;i<statuses.size();++i) {
        auto *line=new QHBoxLayout; line->addWidget(Design::label(statuses[i]));
        auto *bar=new QProgressBar; bar->setRange(0,qMax(1,metrics.total)); bar->setValue(statusCounts[i]); bar->setTextVisible(false); line->addWidget(bar,1);
        line->addWidget(Design::label(QString::number(statusCounts[i]))); rows->addLayout(line);
    }
    rows->addSpacing(5);
    rows->addWidget(Design::label("Par catégorie","section"));
    for (auto it=metrics.categories.cbegin();it!=metrics.categories.cend();++it) {
        auto *line=new QHBoxLayout; line->addWidget(Design::label(it.key(),"muted")); line->addStretch(); line->addWidget(Design::label(QString::number(it.value()))); rows->addLayout(line);
    }
    if (metrics.total==0) rows->addWidget(Design::label("Aucune prestation dans cette sélection.","muted"));
    layout->addWidget(breakdown);
    auto *close=Design::button("Fermer",{},"primary"); connect(close,&QPushButton::clicked,&dialog,&QDialog::accept); layout->addWidget(close,0,Qt::AlignRight);
    dialog.exec();
}
