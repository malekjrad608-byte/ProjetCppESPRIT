#include "hrwidget.h"
#include <QDesktopServices>
#include <QStandardPaths>
#include <algorithm>

namespace AniRh {

namespace {
const QStringList titles={"Tableau de bord","Employés","Présences & absences","Congés & demandes","Missions & responsabilités","Évaluations","Paie & bulletins","Planning de l'équipe"};
const QStringList navTitles={"Tableau de bord","Employés","Présences","Congés","Missions","Évaluations","Paie","Planning"};
const QStringList icons={"grid","users","clock","calendar","tasks","star","money","calendar"};
const QStringList entities={"","employees","attendance","leaves","tasks","reviews","payroll"};
const QStringList descriptions={"","Les personnes qui font grandir ANIWORLD.","Un regard clair sur les journées de votre équipe.","Organisez les absences, préservez l'équilibre de l'équipe.","Des objectifs partagés et des responsabilités bien définies.","Accompagnez les talents et valorisez les progrès.","Préparez, vérifiez et suivez la rémunération de vos collaborateurs."};
const QStringList createLabels={"","Ajouter un employé","Enregistrer un pointage","Demander un congé","Attribuer une mission","Nouvelle évaluation","Préparer le mois"};
class SortItem : public QTableWidgetItem {
public:
    SortItem(const QString &text,const QVariant &sort):QTableWidgetItem(text) {setData(Qt::UserRole,sort);setData(Qt::UserRole+2,text);setToolTip(text);}
    bool operator<(const QTableWidgetItem &other) const override {
        const auto a=data(Qt::UserRole),b=other.data(Qt::UserRole);
        if(a.metaType().id()==QMetaType::Double || a.metaType().id()==QMetaType::LongLong || a.metaType().id()==QMetaType::Int) return a.toDouble()<b.toDouble();
        return QString::localeAwareCompare(a.toString(),b.toString())<0;
    }
};
bool confirm(QWidget *parent,const QString &title,const QString &message) {
    QMessageBox box(QMessageBox::Question,title,message,QMessageBox::NoButton,parent); box.setTextFormat(Qt::PlainText);
    auto *yes=box.addButton("Confirmer",QMessageBox::AcceptRole); box.addButton("Annuler",QMessageBox::RejectRole); box.exec(); return box.clickedButton()==yes;
}
}
HrWidget::HrWidget(HrStore *store,bool demo,QWidget *parent):QWidget(parent),m_store(store),m_demo(demo) {
    setObjectName("rhPage"); Ui::applyTheme(this);
    auto *root=new QVBoxLayout(this); root->setContentsMargins(0,0,0,0); root->setSpacing(0);
    auto *heading=new QFrame; heading->setObjectName("rhHeader");
    auto *header=new QHBoxLayout(heading); header->setContentsMargins(28,16,28,10);
    auto *title=Ui::label("RH · Ressources humaines");
    title->setStyleSheet("font-size:19px;font-weight:700;color:#2E6975;"); header->addWidget(title); header->addStretch();
    auto *guide=Ui::button("Guide RH", "tasks", "toggle"); guide->setObjectName("rhGuide"); header->addWidget(guide);
    connect(guide,&QPushButton::clicked,this,[this]{
        QMessageBox box(this); box.setWindowTitle("Ressources humaines — Guide rapide"); box.setTextFormat(Qt::PlainText);
        box.setText("Employés : ajouter, modifier, archiver et réactiver.\nPrésences : enregistrer les pointages.\nCongés : demander, approuver ou refuser.\nMissions, évaluations, paie et planning : utilisez les onglets.\n\nCtrl+N : créer · Ctrl+F : rechercher · Double-clic : modifier\n\nCongés : quota configurable, jours calendaires.\nPaie : ajustements manuels, sans calcul fiscal ou social automatique. Aucun virement n'est envoyé.\n\n" +
            (m_demo?QString("Démo : données fictives en mémoire, perdues à la fermeture."):QString("Base RH locale : ")+m_store->path()+"\nFermez ANIWORLD et l'application RH avant de copier la base pour une sauvegarde.")); box.exec();
    }); root->addWidget(heading);
    auto *tabRow=new QHBoxLayout; tabRow->setContentsMargins(28,0,28,0);
    m_tabs=new QTabBar; m_tabs->setObjectName("rhTabs"); m_tabs->setExpanding(false); m_tabs->setDrawBase(false); m_tabs->setUsesScrollButtons(true);
    for(int i=0;i<navTitles.size();++i) { m_tabs->addTab(navTitles[i]); m_tabs->setTabToolTip(i,titles[i]); }
    tabRow->addWidget(m_tabs); tabRow->addStretch(); root->addLayout(tabRow);
    m_stack=new QStackedWidget; m_stack->setObjectName("rhPageStack"); m_stack->setAutoFillBackground(true); root->addWidget(m_stack,1);
    m_dashboard=new Dashboard(store); auto *scroll=new QScrollArea; scroll->setWidgetResizable(true); scroll->setWidget(m_dashboard); m_stack->addWidget(scroll);
    connect(m_dashboard,&Dashboard::navigateRequested,this,&HrWidget::navigate);
    connect(m_dashboard,&Dashboard::addEmployeeRequested,this,[this]{navigate(1);openCreate();});
    m_sections.resize(7); for(int i=1;i<7;++i) m_stack->addWidget(buildSection(i));
    m_schedule=new ScheduleWidget(store,demo);m_stack->addWidget(m_schedule);
    m_toast=Ui::label(""); m_toast->setParent(this); m_toast->setStyleSheet("background:#2F6470;color:white;border-radius:9px;padding:13px 22px;font-size:12px;"); m_toast->hide();
    m_toastTimer.setSingleShot(true); connect(&m_toastTimer,&QTimer::timeout,m_toast,&QWidget::hide);
    connect(store,&HrStore::changed,this,&HrWidget::refresh,Qt::QueuedConnection);
    connect(m_tabs,&QTabBar::currentChanged,this,&HrWidget::navigate);
    m_lastDay=HrStore::today(); auto *timer=new QTimer(this); timer->setInterval(60000);
    connect(timer,&QTimer::timeout,this,[this]{const auto day=HrStore::today();if(day!=m_lastDay){m_lastDay=day;refresh();}}); timer->start();
    navigate(0);
}
void HrWidget::focusSearch() {
    if(m_stack->currentIndex()==0) navigate(1);
    if(m_stack->currentIndex()==7) m_schedule->focusSearch();
    else m_sections[m_stack->currentIndex()].search->setFocus();
}
QWidget *HrWidget::buildSection(int index) {
    auto &s=m_sections[index]; s.page=new QWidget; s.page->setObjectName("section"+QString::number(index)); auto *root=new QVBoxLayout(s.page); root->setContentsMargins(30,25,30,25); root->setSpacing(20);
    auto *header=new QHBoxLayout; auto *text=new QVBoxLayout; text->setSpacing(5); text->addWidget(Ui::label(titles[index],"title")); auto *subtitle=Ui::label(descriptions[index],"muted"); subtitle->setWordWrap(true); text->addWidget(subtitle); header->addLayout(text,1);
    auto *add=Ui::button(createLabels[index],"plus","primary"); add->setObjectName("create"+QString::number(index)); header->addWidget(add); connect(add,&QPushButton::clicked,this,&HrWidget::openCreate); root->addLayout(header);
    auto *summary=Ui::card(); auto *sumLayout=new QHBoxLayout(summary); sumLayout->setContentsMargins(19,16,19,16); auto *sumIcon=Ui::label(""); sumIcon->setPixmap(Ui::icon(icons[index]).pixmap(24,24)); sumLayout->addWidget(sumIcon); sumLayout->addSpacing(9); s.summary=Ui::label(""); s.summary->setObjectName("summary"+QString::number(index)); s.summary->setWordWrap(true); sumLayout->addWidget(s.summary,1); root->addWidget(summary);
    auto *card=Ui::card(); auto *cl=new QVBoxLayout(card); cl->setContentsMargins(0,0,0,0); cl->setSpacing(0); auto *toolbar=new QHBoxLayout; toolbar->setContentsMargins(17,17,17,17); toolbar->setSpacing(10);
    s.search=new QLineEdit; s.search->setObjectName("search"+QString::number(index)); s.search->setPlaceholderText(index==1?"Rechercher un nom, un poste, un email…":"Rechercher dans la liste…"); s.search->addAction(Ui::icon("search"),QLineEdit::LeadingPosition); s.search->setClearButtonEnabled(true); s.search->setMinimumWidth(200); toolbar->addWidget(s.search,1);
    s.filter=new QComboBox; s.filter->setObjectName("filter"+QString::number(index)); s.filter->setMinimumWidth(152);
    const QList<QStringList> filters={{},{"Tous les employés","Actifs","Archivés"},{"Tous les statuts","Présent","Retard","Absent","Télétravail","En congé","Non pointé"},{"Tous les statuts","En attente","Approuvé","Refusé"},{"Tous les statuts","À faire","En cours","Terminé"},{"Toutes les notes","4 et plus","Moins de 4"},{"Tous les statuts","Brouillon","Payé"}};
    s.filter->addItems(filters[index]); toolbar->addWidget(s.filter);
    if(index==2 || index==6) { s.period=new QDateEdit; s.period->setObjectName("period"+QString::number(index)); s.period->setCalendarPopup(true); s.period->setDate(QDate::fromString(HrStore::today(),Qt::ISODate)); s.period->setDisplayFormat(index==6?"MMMM yyyy":"dd/MM/yyyy"); s.period->setMinimumWidth(index==6?160:132); toolbar->addWidget(s.period); connect(s.period,&QDateEdit::dateChanged,this,[this,index]{refreshSection(index);}); }
    auto *exportButton=Ui::button("Exporter","export"); exportButton->setObjectName("export"+QString::number(index)); toolbar->addWidget(exportButton); connect(exportButton,&QPushButton::clicked,this,&HrWidget::exportCurrent); cl->addLayout(toolbar);
    const QList<QStringList> headers={{},{"COLLABORATEUR","POSTE","DÉPARTEMENT","CONTRAT","SALAIRE DE BASE","STATUT"},{"COLLABORATEUR","DÉPARTEMENT","STATUT","ARRIVÉE","DÉPART","DURÉE"},{"COLLABORATEUR","TYPE","DU","AU","JOURS","STATUT"},{"MISSION","RESPONSABLE","ÉCHÉANCE","PRIORITÉ","STATUT"},{"COLLABORATEUR","DATE","ÉVALUATEUR","NOTE / 5","BILAN"},{"COLLABORATEUR","BASE","PRIMES","RETENUES","NET À PAYER","STATUT"}};
    s.headers=headers[index]; s.table=new QTableWidget; s.table->setObjectName("table"+QString::number(index)); Ui::setupTable(s.table,s.headers); cl->addWidget(s.table,1);
    s.empty=Ui::label("","muted"); s.empty->setObjectName("empty"+QString::number(index)); s.empty->setAlignment(Qt::AlignCenter); s.empty->setWordWrap(true); s.empty->setMinimumHeight(180); cl->addWidget(s.empty,1); s.empty->hide();
    auto *footer=new QHBoxLayout; footer->setContentsMargins(18,12,18,12); s.count=Ui::label("","muted"); footer->addWidget(s.count); footer->addStretch(); footer->addWidget(Ui::label("Cliquez sur un en-tête pour trier","muted")); cl->addLayout(footer); root->addWidget(card,1);
    auto *actions=new QHBoxLayout; actions->setSpacing(9);
    s.edit=Ui::button(index==3?"Consulter":(index==2?"Pointer / modifier":"Modifier"),"edit"); s.edit->setObjectName("edit"+QString::number(index)); actions->addWidget(s.edit); connect(s.edit,&QPushButton::clicked,this,&HrWidget::openEdit);
    if(index!=6) { s.remove=Ui::button(index==1?"Archiver":"Supprimer","archive",index==1?nullptr:"danger"); s.remove->setObjectName("remove"+QString::number(index)); actions->addWidget(s.remove); connect(s.remove,&QPushButton::clicked,this,&HrWidget::removeSelected); }
    if(index==3) {
        s.approve=Ui::button("Approuver","check","primary"); s.reject=Ui::button("Refuser",{},"danger"); s.approve->setObjectName("approveLeave"); s.reject->setObjectName("rejectLeave"); actions->addWidget(s.approve); actions->addWidget(s.reject); connect(s.approve,&QPushButton::clicked,this,[this]{decide(true);}); connect(s.reject,&QPushButton::clicked,this,[this]{decide(false);});
    }
    if(index==6) {
        s.pdf=Ui::button("Bulletin PDF","export"); s.pdf->setObjectName("payslipPdf"); s.paid=Ui::button("Marquer comme payé","check","primary"); s.paid->setObjectName("markPaid"); actions->addWidget(s.pdf); actions->addWidget(s.paid); connect(s.pdf,&QPushButton::clicked,this,&HrWidget::exportSelectedPdf); connect(s.paid,&QPushButton::clicked,this,&HrWidget::paySelected);
    }
    actions->addStretch(); root->addLayout(actions);
    if(index==6) { auto *note=Ui::label("Net = base + primes − retenues. Ajustements manuels : fiscalité, cotisations, absences et proratas à renseigner.","muted"); note->setWordWrap(true); root->addWidget(note); }
    connect(s.search,&QLineEdit::textChanged,this,[this,index]{refreshSection(index);}); connect(s.filter,&QComboBox::currentTextChanged,this,[this,index]{refreshSection(index);}); connect(s.table,&QTableWidget::itemSelectionChanged,this,[this,index]{selectionChanged(index);}); connect(s.table,&QTableWidget::cellDoubleClicked,this,[this]{openEdit();}); return s.page;
}
void HrWidget::refreshSection(int index) {
    auto &s=m_sections[index]; const QString day=s.period?s.period->date().toString(Qt::ISODate):HrStore::today();
    Records data;
    if(index==1) data=m_store->employees();
    else if(index==2) data=m_store->rows("SELECT a.id,e.id AS employee_id,e.first_name||' '||e.last_name AS name,e.email,e.department,a.arrival,a.departure,a.notes,? AS day,COALESCE(a.status,CASE WHEN l.id IS NOT NULL THEN 'En congé' ELSE 'Non pointé' END) AS status,l.id AS leave_id FROM employees e LEFT JOIN attendance a ON a.employee_id=e.id AND a.day=? LEFT JOIN leaves l ON l.employee_id=e.id AND l.status='Approuvé' AND l.start<=? AND l.end>=? WHERE (e.active=1 AND e.hired<=?) OR a.id IS NOT NULL OR l.id IS NOT NULL ORDER BY e.last_name,e.first_name",{day,day,day,day,day});
    else if(index==6) data=m_store->rows("SELECT *,employee_name AS name FROM payroll WHERE month=? ORDER BY employee_name",{day.left(7)});
    else data=m_store->rows("SELECT x.*,e.first_name||' '||e.last_name AS name,e.email FROM "+entities[index]+" x JOIN employees e ON e.id=x.employee_id ORDER BY x.id DESC");
    int count1=0,count2=0; qint64 total=0; double score=0;
    for(auto &r:data) {
        if(index==1) { if(r["active"].toBool()) {++count1;total+=r["salary"].toLongLong();} else ++count2; }
        if(index==2) { if(QStringList{"Présent","Retard","Télétravail"}.contains(r["status"].toString())) ++count1; if(r["status"]=="Non pointé") ++count2; }
        if(index==3) { if(r["status"]=="En attente") ++count1; if(r["status"]=="Approuvé") ++count2; }
        if(index==4) { if(r["status"]!="Terminé") ++count1; if(r["status"]!="Terminé" && r["due"].toString()<HrStore::today()) ++count2; }
        if(index==5) { r["score"]=(r["quality"].toDouble()+r["teamwork"].toDouble()+r["punctuality"].toDouble()+r["initiative"].toDouble())/4.; score+=r["score"].toDouble(); }
        if(index==6) { r["net"]=r["base"].toLongLong()+r["bonus"].toLongLong()-r["deductions"].toLongLong(); total+=r["net"].toLongLong(); if(r["status"]=="Payé") ++count1; else ++count2; }
    }
    if(index==1) s.summary->setText(QString("%1 employés actifs    ·    %2 dossiers archivés    ·    Masse salariale de base : %3 / mois").arg(count1).arg(count2).arg(Ui::money(total)));
    if(index==2) s.summary->setText(QString("%1 présents    ·    %2 non pointés    ·    %3 collaborateurs à cette date\nLes congés approuvés apparaissent automatiquement. Non pointé ne signifie pas absent.").arg(count1).arg(count2).arg(data.size()));
    if(index==3) s.summary->setText(QString("%1 demandes en attente    ·    %2 approuvées    ·    %3 demandes au total\nSolde annuel configurable par employé ; décompte en jours calendaires.").arg(count1).arg(count2).arg(data.size()));
    if(index==4) s.summary->setText(QString("%1 missions ouvertes    ·    %2 échéances dépassées    ·    %3 missions au total").arg(count1).arg(count2).arg(data.size()));
    if(index==5) s.summary->setText(QString("%1 évaluations    ·    Note moyenne : %2 / 5\nQualité du travail, esprit d'équipe, ponctualité et initiative.").arg(data.size()).arg(data.isEmpty()?QString("—"):QLocale().toString(score/data.size(),'f',2)));
    if(index==6) s.summary->setText(QString("Net du mois : %1    ·    %2 bulletins en brouillon    ·    %3 payés\nPréparer le mois ajoute uniquement les bulletins manquants.").arg(Ui::money(total)).arg(count2).arg(count1));
    const QString search=Ui::normalized(s.search->text().trimmed()),filter=s.filter->currentText();
    Records visible;
    for(const auto &r:data) {
        QString haystack; for(auto it=r.cbegin();it!=r.cend();++it) haystack+=it.value().toString()+" ";
        if(!search.isEmpty() && !Ui::normalized(haystack).contains(search)) continue;
        if(s.filter->currentIndex()>0) {
            if(index==1 && ((filter=="Actifs")!=r["active"].toBool())) continue;
            if(index==5 && ((filter=="4 et plus")!=(r["score"].toDouble()>=4))) continue;
            if(index!=1 && index!=5 && r["status"].toString()!=filter) continue;
        }
        visible.append(r);
    }
    Record old;
    if(s.table->currentRow()>=0 && s.table->item(s.table->currentRow(),0)) old=s.table->item(s.table->currentRow(),0)->data(Qt::UserRole+1).toMap();
    const int sortColumn=s.table->horizontalHeader()->sortIndicatorSection(); const auto sortOrder=s.table->horizontalHeader()->sortIndicatorOrder();
    {
        QSignalBlocker blocker(s.table); s.table->setSortingEnabled(false); s.table->setRowCount(0); s.table->setRowCount(visible.size());
        for(int row=0;row<visible.size();++row) {
            const auto &r=visible[row]; QStringList values; QList<QVariant> sorting; QList<int> badgeColumns;
            if(index==1) { values={r["name"].toString(),r["role"].toString(),r["department"].toString(),r["contract"].toString(),Ui::money(r["salary"].toLongLong()),r["active"].toBool()?"Actif":"Archivé"}; badgeColumns={5}; }
            if(index==2) {
                const QTime arrival=QTime::fromString(r["arrival"].toString(),"HH:mm"),departure=QTime::fromString(r["departure"].toString(),"HH:mm");
                QString duration="—"; if(arrival.isValid() && departure.isValid()) {int minutes=arrival.secsTo(departure)/60;duration=QString("%1 h %2").arg(minutes/60).arg(minutes%60,2,10,QChar('0'));}
                values={r["name"].toString(),r["department"].toString(),r["status"].toString(),arrival.isValid()?r["arrival"].toString():"—",departure.isValid()?r["departure"].toString():"—",duration}; badgeColumns={2};
            }
            if(index==3) { values={r["name"].toString(),r["type"].toString(),Ui::date(r["start"].toString()),Ui::date(r["end"].toString()),r["days"].toString()+" j",r["status"].toString()}; badgeColumns={5}; }
            if(index==4) { values={r["title"].toString(),r["name"].toString(),Ui::date(r["due"].toString()),r["priority"].toString(),r["status"].toString()}; badgeColumns={3,4}; }
            if(index==5) values={r["name"].toString(),Ui::date(r["day"].toString()),r["reviewer"].toString(),QLocale().toString(r["score"].toDouble(),'f',2),r["feedback"].toString()};
            if(index==6) { values={r["name"].toString(),Ui::money(r["base"].toLongLong()),Ui::money(r["bonus"].toLongLong()),Ui::money(r["deductions"].toLongLong()),Ui::money(r["net"].toLongLong()),r["status"].toString()}; badgeColumns={5}; }
            for(const auto &v:values) sorting.append(Ui::normalized(v));
            if(index==1) sorting[4]=r["salary"];
            if(index==2) { const auto a=QTime::fromString(r["arrival"].toString(),"HH:mm"),d=QTime::fromString(r["departure"].toString(),"HH:mm"); sorting[5]=a.isValid()&&d.isValid()?a.secsTo(d):-1; }
            if(index==3) {sorting[2]=r["start"];sorting[3]=r["end"];sorting[4]=r["days"];}
            if(index==4) sorting[2]=r["due"];
            if(index==5) {sorting[1]=r["day"];sorting[3]=r["score"];}
            if(index==6) {sorting[1]=r["base"];sorting[2]=r["bonus"];sorting[3]=r["deductions"];sorting[4]=r["net"];}
            for(int col=0;col<values.size();++col) {
                auto *item=new SortItem(values[col],sorting[col]); if(col==0) item->setData(Qt::UserRole+1,r); s.table->setItem(row,col,item);
                if(badgeColumns.contains(col)) { item->setText(""); s.table->setCellWidget(row,col,Ui::badge(values[col])); }
                if(index==4 && col==2 && r["status"]!="Terminé" && r["due"].toString()<HrStore::today()) item->setForeground(QColor("#B36359"));
                if((index==6 && col==4) || (index==5 && col==3)) { item->setForeground(QColor("#3C8591")); QFont f=item->font();f.setBold(true);item->setFont(f); }
            }
            if(index!=4) { s.table->item(row,0)->setText(""); s.table->setCellWidget(row,0,Ui::person(r["name"].toString(),index==6?r["employee_role"].toString():r["email"].toString(),r.value("employee_id",r["id"]).toInt())); }
        }
        s.table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Interactive); s.table->setColumnWidth(0,index==4?285:220);
        s.table->setSortingEnabled(true); s.table->sortItems(qMax(0,sortColumn),sortOrder);
        if(!old.isEmpty()) for(int i=0;i<s.table->rowCount();++i) {
            const auto r=s.table->item(i,0)->data(Qt::UserRole+1).toMap(); const QString key=index==2?"employee_id":"id";
            if(r[key]==old[key]) {s.table->selectRow(i);break;}
        }
    }
    s.table->setVisible(!visible.isEmpty()); s.empty->setVisible(visible.isEmpty());
    if(visible.isEmpty()) s.empty->setText(data.isEmpty()?(index==1?"Votre équipe commence ici.\nAjoutez votre premier employé pour démarrer.":(index==6?"Aucun bulletin pour ce mois.\nCliquez sur « Préparer le mois » pour créer les brouillons.":"Aucun enregistrement pour le moment.\nUtilisez le bouton en haut à droite pour commencer.")):"Aucun résultat pour ces critères.\nModifiez la recherche ou les filtres.");
    s.count->setText(QString("%1 résultat(s) sur %2").arg(visible.size()).arg(data.size())); selectionChanged(index);
}
void HrWidget::selectionChanged(int index) {
    auto &s=m_sections[index]; Record r;
    if(s.table->currentRow()>=0 && s.table->item(s.table->currentRow(),0)) r=s.table->item(s.table->currentRow(),0)->data(Qt::UserRole+1).toMap();
    const bool has=!r.isEmpty(); s.edit->setEnabled(has && (index!=6 || r["status"]=="Brouillon") && (index!=2 || r["status"]!="En congé"));
    if(s.remove) {s.remove->setEnabled(has && (index!=2 || r["id"].toInt()) && (index!=3 || r["status"]!="Approuvé"));if(index==1)s.remove->setText(has && !r["active"].toBool()?"Réactiver":"Archiver");}
    if(s.approve) {s.approve->setEnabled(has && r["status"]=="En attente");s.reject->setEnabled(has && r["status"]=="En attente");}
    if(s.pdf) {s.pdf->setEnabled(has);s.paid->setEnabled(has && r["status"]=="Brouillon");}
}
Record HrWidget::selected() const {
    const int index=m_stack->currentIndex(); if(index==0 || index==7) return {};
    auto *table=m_sections[index].table; const int row=table->currentRow(); return row>=0 && table->item(row,0)?table->item(row,0)->data(Qt::UserRole+1).toMap():Record();
}
void HrWidget::navigate(int index) {
    if(index<0 || index>=8) return;
    const bool changed=index!=m_stack->currentIndex(); m_stack->setCurrentIndex(index);
    { QSignalBlocker blocker(m_tabs); m_tabs->setCurrentIndex(index); }
    if(index==0) m_dashboard->refresh(); else if(index==7)m_schedule->refresh();else refreshSection(index); if(changed && isVisible()) Motion::reveal(m_stack,QColor("#F3F7F8"));
}
void HrWidget::refresh() { const int index=m_stack->currentIndex(); if(index==0)m_dashboard->refresh();else if(index==7)m_schedule->refresh();else refreshSection(index); }
void HrWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if(m_toast && m_toast->isVisible()) m_toast->move((width()-m_toast->width())/2,height()-m_toast->height()-25);
}
void HrWidget::notify(const QString &message) { m_toast->setText(message); m_toast->adjustSize();m_toast->move((width()-m_toast->width())/2,height()-m_toast->height()-25);m_toast->show();m_toast->raise();m_toastTimer.start(3200); }
void HrWidget::reportError() { QMessageBox box(QMessageBox::Warning,"Action non effectuée",m_store->error(),QMessageBox::Ok,this);box.setTextFormat(Qt::PlainText);box.exec(); }
void HrWidget::openCreate() {
    int index=m_stack->currentIndex(); if(index==0){navigate(1);index=1;}
    if(index==7){m_schedule->createShift();return;}
    if(index>1 && m_store->employees(true).isEmpty()) {notify("Ajoutez ou réactivez un employé pour continuer.");return;}
    if(index==6) {
        const QString month=m_sections[6].period->date().toString("yyyy-MM"); const qint64 before=m_store->scalar("SELECT COUNT(*) FROM payroll WHERE month=?",{month});
        if(!m_store->preparePayroll(month)){reportError();return;}refresh(); const qint64 added=m_store->scalar("SELECT COUNT(*) FROM payroll WHERE month=?",{month})-before;
        notify(QString("%1 bulletin(s) créé(s) pour %2. Vérifiez les ajustements.").arg(added).arg(month));return;
    }
    Record initial; if(index==2)initial["day"]=m_sections[2].period->date().toString(Qt::ISODate);
    EditDialog dialog(m_store,static_cast<EditDialog::Kind>(index-1),initial,this); if(dialog.exec()==QDialog::Accepted){refresh();notify("Enregistrement sauvegardé.");}
}
void HrWidget::openEdit() {
    const int index=m_stack->currentIndex(); Record r=selected(); if(r.isEmpty())return;
    if(index==3) {
        auto employee=m_store->row("SELECT allowance FROM employees WHERE id=?",{r["employee_id"]});const int year=QDate::fromString(r["start"].toString(),Qt::ISODate).year();
        QMessageBox box(this);box.setWindowTitle("Demande de congé");box.setTextFormat(Qt::PlainText);box.setText(r["name"].toString()+"\n"+r["type"].toString()+" · "+r["status"].toString()+"\nDu "+Ui::date(r["start"].toString())+" au "+Ui::date(r["end"].toString())+"\n"+r["days"].toString()+" jour(s) calendaire(s)\n\nMotif : "+r["reason"].toString()+QString("\n\nSolde annuel %1 : %2 jour(s), hors demandes en attente.").arg(year).arg(employee["allowance"].toInt()-m_store->usedLeave(r["employee_id"].toInt(),year)));box.exec();return;
    }
    if(index==2 && r["status"]=="En congé"){notify("Cette date est couverte par un congé approuvé.");return;}
    if(index==6 && r["status"]=="Payé"){notify("Ce bulletin est payé et conservé sans modification.");return;}
    if(index==2 && !r["id"].toInt()) {r.remove("id");r["status"]="Présent";r["arrival"]="08:30";r["departure"]="";}
    EditDialog dialog(m_store,static_cast<EditDialog::Kind>(index-1),r,this); if(dialog.exec()==QDialog::Accepted){refresh();notify("Modifications sauvegardées.");}
}
void HrWidget::removeSelected() {
    const int index=m_stack->currentIndex();const auto r=selected();if(r.isEmpty())return;
    if(index==1) {
        const bool active=r["active"].toBool(); if(!confirm(this,active?"Archiver le dossier":"Réactiver le dossier",r["name"].toString()+"\n\n"+(active?"Le dossier et l'historique seront conservés. L'employé ne sera plus proposé pour les nouvelles missions, les congés et les nouveaux bulletins.":"L'employé sera à nouveau proposé dans les formulaires.")))return;
        if(!m_store->setEmployeeActive(r["id"].toInt(),!active)){reportError();return;}
    } else {
        if(!r["id"].toInt())return;
        if(!confirm(this,"Supprimer l'enregistrement","Supprimer cet enregistrement ? Cette action est définitive."))return;
        if(!m_store->removeRecord(entities[index],r["id"].toInt())){reportError();return;}
    }
    refresh();notify(index==1?"Dossier mis à jour.":"Enregistrement supprimé.");
}
void HrWidget::decide(bool approve) {
    const auto r=selected();if(r.isEmpty())return;
    if(!confirm(this,approve?"Approuver le congé":"Refuser le congé",QString("%1\nDu %2 au %3 · %4 jour(s) calendaire(s)\n\n%5").arg(r["name"].toString(),Ui::date(r["start"].toString()),Ui::date(r["end"].toString()),r["days"].toString(),approve?"Le congé approuvé sera conservé dans l'historique.":"La demande restera visible avec le statut Refusé.")))return;
    if(!m_store->decideLeave(r["id"].toInt(),approve)){reportError();return;}refresh();notify(approve?"Congé approuvé.":"Demande refusée.");
}
void HrWidget::paySelected() {
    const auto r=selected();if(r.isEmpty())return;
    if(!confirm(this,"Constater le paiement",r["name"].toString()+" · "+r["month"].toString()+"\nNet : "+Ui::money(r["net"].toLongLong())+"\n\nConfirmez que ce montant a déjà été payé. Le bulletin sera figé. Aucun virement n'est déclenché."))return;
    if(!m_store->markPaid(r["id"].toInt())){reportError();return;}refresh();notify("Paiement enregistré. Le bulletin est maintenant figé.");
}
void HrWidget::exportCurrent() {
    const int index=m_stack->currentIndex();if(index<1 || index>=m_sections.size())return;const auto &s=m_sections[index];
    if(!s.table->rowCount()){notify("Aucune donnée à exporter pour ces filtres.");return;}
    QString path=QFileDialog::getSaveFileName(this,"Exporter les résultats affichés",QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/ANIWORLD_RH_"+entities[index]+(m_demo?"_DEMO":"")+".csv","CSV UTF-8 (*.csv)");if(path.isEmpty())return;if(!path.endsWith(".csv",Qt::CaseInsensitive))path+=".csv";
    QStringList headers=s.headers; QList<QStringList> values;
    if(index==2)headers.prepend("DATE");
    if(index==6)headers.prepend("MOIS");
    if(m_demo)headers.prepend("MODE");
    for(int row=0;row<s.table->rowCount();++row) {QStringList cells;for(int col=0;col<s.table->columnCount();++col)cells.append(s.table->item(row,col)->data(Qt::UserRole+2).toString());if(index==2)cells.prepend(s.period->date().toString(Qt::ISODate));if(index==6)cells.prepend(s.period->date().toString("yyyy-MM"));if(m_demo)cells.prepend("DÉMONSTRATION");values.append(cells);}
    QString error;if(Ui::exportCsv(path,headers,values,&error))notify("Export CSV enregistré.");else QMessageBox::warning(this,"Export impossible",error);
}
void HrWidget::exportSelectedPdf() {
    auto r=selected();if(r.isEmpty() || m_stack->currentIndex()!=6)return;
    QString path=QFileDialog::getSaveFileName(this,"Enregistrer le bulletin PDF",QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/Bulletin_"+r["month"].toString()+"_"+r["id"].toString()+(m_demo?"_DEMO":"")+".pdf","PDF (*.pdf)");if(path.isEmpty())return;if(!path.endsWith(".pdf",Qt::CaseInsensitive))path+=".pdf";
    QString error;if(exportPayslip(path,r,m_demo,&error))notify("Bulletin PDF enregistré.");else QMessageBox::warning(this,"Export impossible",error);
}

} // namespace AniRh
