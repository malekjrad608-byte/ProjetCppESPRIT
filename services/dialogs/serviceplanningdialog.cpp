#include "serviceplanningdialog.h"
#include "servicedialog.h"
#include "../models/service.h"
#include "../database/databasemanager.h"
#include "../database/oracleconnection.h"
#include "../ui/design.h"
#include <QComboBox>
#include <QDateEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include "../models/employeequeries.h"

#include <QSignalBlocker>
#include <QMessageBox>
#include <QScreen>
#include <QLocale>

ServicePlanningDialog::ServicePlanningDialog(const QDate &date,QWidget *parent):QDialog(parent) {
    setObjectName("servicePlanningDialog");setWindowTitle("ANIWORLD — Planning des employés par service");
    resize(QSize(1160,700).boundedTo(screen()->availableGeometry().size()-QSize(50,70)));
    auto *root=new QVBoxLayout(this);root->setContentsMargins(22,20,22,20);root->setSpacing(14);
    root->addWidget(Design::label("Planning d’affectation","title"));
    auto *intro=Design::label("Emploi du temps par employé. Filtrez par personne et par semaine.","muted");intro->setWordWrap(true);root->addWidget(intro);
    auto *filters=new QHBoxLayout;
    auto *previous=Design::button("←");previous->setObjectName("planningPrevious");previous->setToolTip("Semaine précédente");filters->addWidget(previous);
    m_date=new QDateEdit(date);m_date->setMinimumWidth(130);m_date->setCalendarPopup(true);m_date->setDisplayFormat("dd/MM/yyyy");m_date->setObjectName("planningDate");m_date->setAccessibleName("Date de la semaine à afficher");filters->addWidget(m_date);
    auto *next=Design::button("→");next->setObjectName("planningNext");next->setToolTip("Semaine suivante");filters->addWidget(next);
    auto *today=Design::button("Aujourd’hui");today->setObjectName("planningToday");filters->addWidget(today);
    filters->addStretch();m_period=Design::label("","section");m_period->setObjectName("planningPeriod");m_period->setWordWrap(true);filters->addWidget(m_period);root->addLayout(filters);
    auto *info=new QHBoxLayout;info->addWidget(Design::label("Employé"));
    m_employee=new QComboBox;m_employee->setObjectName("planningEmployee");m_employee->setMinimumWidth(200);info->addWidget(m_employee,1);
    m_cancelled=new QCheckBox("Afficher les services annulés");m_cancelled->setObjectName("planningCancelled");info->addWidget(m_cancelled);
    auto *refresh=Design::button("Actualiser");refresh->setObjectName("planningRefresh");info->addWidget(refresh);root->addLayout(info);
    m_summary=Design::label("","muted");m_summary->setObjectName("planningSummary");m_summary->setWordWrap(true);root->addWidget(m_summary);
    m_table=new QTableWidget;m_table->setObjectName("servicePlanningTable");
    Design::setupTable(m_table,{"Date","Horaire","Employé","Service / tâche","Client","Statut","Attention"});
    m_table->setWordWrap(false);m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Interactive);m_table->setColumnWidth(3,240);
    m_table->horizontalHeader()->setStretchLastSection(true);
    root->addWidget(m_table,1);
    m_message=Design::label("","muted");m_message->setObjectName("planningMessage");m_message->setWordWrap(true);root->addWidget(m_message);
    auto *legend=Design::label(OracleConnection::isOracle()?"Chaque employé appartient à un service. Une réaffectation sur un créneau occupé est refusée ; le service d’origine doit garder au moins un employé.":"Une affectation est refusée si l’employé est déjà occupé. Les services annulés ne réservent pas de créneau.","muted");legend->setWordWrap(true);root->addWidget(legend);
    auto *buttons=new QHBoxLayout;
    m_edit=Design::button("Affecter / modifier le service","edit","primary");m_edit->setObjectName("planningEdit");buttons->addWidget(m_edit);buttons->addStretch();
    auto *close=Design::button("Fermer");connect(close,&QPushButton::clicked,this,&QDialog::accept);buttons->addWidget(close);root->addLayout(buttons);
    connect(previous,&QPushButton::clicked,this,[this]{m_date->setDate(m_date->date().addDays(-7));});
    connect(next,&QPushButton::clicked,this,[this]{m_date->setDate(m_date->date().addDays(7));});
    connect(today,&QPushButton::clicked,this,[this]{m_date->setDate(QDate::currentDate());});
    connect(refresh,&QPushButton::clicked,this,&ServicePlanningDialog::reload);
    connect(m_date,&QDateEdit::dateChanged,this,&ServicePlanningDialog::populate);
    connect(m_employee,qOverload<int>(&QComboBox::currentIndexChanged),this,&ServicePlanningDialog::populate);
    connect(m_cancelled,&QCheckBox::toggled,this,&ServicePlanningDialog::populate);
    connect(m_table,&QTableWidget::itemSelectionChanged,this,[this]{const auto *item=m_table->item(m_table->currentRow(),0);m_edit->setEnabled(!m_readOnly && OracleConnection::isOracle() && item && item->data(Qt::UserRole).toInt()>0);});
    connect(m_edit,&QPushButton::clicked,this,&ServicePlanningDialog::editAssignment);
    connect(m_table,&QTableWidget::cellDoubleClicked,this,[this]{if(m_edit->isEnabled())editAssignment();});
    reload();
}
void ServicePlanningDialog::reload() {
    QString error;m_entries=Service::appointments(&error);m_names.clear();m_available.clear();
    auto q = EmployeeQueries::planningEmployees();
    if(!q.isOk()) error=q.lastError().text();
    else while(q.next()){const int id=q.value(0).toInt();m_names[id]=(q.value(1).toString()+" "+q.value(2).toString()).trimmed();m_available[id]=q.value(3).toInt()!=0;}
    const int selected=m_employee->currentData().toInt();
    {QSignalBlocker block(m_employee);m_employee->clear();m_employee->addItem("Tous les employés",0);m_employee->addItem("Services non affectés",-1);
        for(auto it=m_names.cbegin();it!=m_names.cend();++it)m_employee->addItem(it.value()+QString(" (#%1)").arg(it.key()),it.key());
        m_employee->setCurrentIndex(qMax(0,m_employee->findData(selected)));}
    m_loadError=error;
    populate();
}
void ServicePlanningDialog::showEmployee(int employeeId,bool readOnly) {
    m_readOnly=readOnly;m_edit->setVisible(!readOnly);
    m_employee->setCurrentIndex(qMax(0,m_employee->findData(employeeId)));populate();
}
void ServicePlanningDialog::populate() {
    if(!m_loadError.isEmpty()){m_table->setRowCount(0);m_summary->clear();m_message->setText("Chargement impossible : "+m_loadError);m_edit->setEnabled(false);return;}
    const auto start=Service::weekStart(m_date->date());
    m_period->setText("Semaine du "+start.toString("dd/MM/yyyy")+" au "+start.addDays(6).toString("dd/MM/yyyy"));
    QList<Service::Appointment> rows;
    const int selected=m_employee->currentData().toInt();
    for(const auto &a:m_entries) {
        if(a.start>=QDateTime(start.addDays(7),QTime(0,0)) || a.finish<=QDateTime(start,QTime(0,0)))continue;
        if(!m_cancelled->isChecked() && a.status=="Annulé")continue;
        if(selected>0 && a.employeeId!=selected)continue;
        if(selected==-1 && a.employeeId>0)continue;
        rows<<a;
    }
    m_table->setRowCount(0);int minutes=0,unassigned=0,overlaps=0;QSet<int> employees;
    for(const auto &s:rows) {
        const int row=m_table->rowCount();m_table->insertRow(row);
        const auto finish=s.finish;
        const auto employee=s.employeeId>0?m_names.value(s.employeeId,QString("Employé #%1").arg(s.employeeId)):QString("Non affecté");
        QStringList alert;
        if(s.employeeId<=0){++unassigned;alert<<"À affecter";}
        else {employees.insert(s.employeeId);if(!m_available.value(s.employeeId,true))alert<<"Indisponible";}
        bool conflict=false;
        if(s.employeeId>0 && s.status!="Annulé")for(const auto &other:rows) {
            if(&other==&s || other.employeeId!=s.employeeId || other.status=="Annulé")continue;
            if(s.start<other.finish && other.start<s.finish){conflict=true;break;}
        }
        if(conflict){++overlaps;alert<<"Chevauchement";}
        if(!s.serviceId)alert<<"Module Employés";
        if(s.status!="Annulé")minutes+=qMax(s.start,QDateTime(start,QTime(0,0))).secsTo(qMin(finish,QDateTime(start.addDays(7),QTime(0,0))))/60;
        const QStringList values={QLocale(QLocale::French).toString(s.start.date(),"ddd dd/MM"),s.start.toString("HH:mm")+" → "+finish.toString(finish.date()==s.start.date()?"HH:mm":"dd/MM HH:mm"),employee,s.title,s.client,s.status,alert.join(" · ")};
        for(int col=0;col<values.size();++col){auto *cell=new QTableWidgetItem(values[col]);cell->setToolTip(values[col]);cell->setData(Qt::UserRole,s.serviceId);
            if(col==6 && !alert.isEmpty()){cell->setForeground(QColor("#A56518"));cell->setBackground(QColor("#FFF1DA"));}
            m_table->setItem(row,col,cell);}
    }
    m_table->clearSelection();m_table->setCurrentCell(-1,-1);m_edit->setEnabled(false);
    m_summary->setText(QString("%1 créneau(x) · %2 employé(s) affecté(s) · %3 h %4 sur la semaine (hors annulations) · %5 à affecter · %6 en chevauchement")
        .arg(rows.size()).arg(employees.size()).arg(minutes/60).arg(minutes%60,2,10,QChar('0')).arg(unassigned).arg(overlaps));
    m_message->setText(rows.isEmpty()?"Aucune affectation pour cette semaine et cet employé. Choisissez une autre semaine ou ajoutez un service.":
        (m_readOnly?"Fermez ce planning pour choisir l’employé et le créneau dans le formulaire.":OracleConnection::isOracle()?"Sélectionnez un service pour modifier son employé ou son créneau. Chaque employé appartient à un service ; son rattachement se modifie dans Employés.":"Aperçu local : l’affectation à un employé enregistré est disponible en mode Oracle."));
}
void ServicePlanningDialog::editAssignment() {
    const int row=m_table->currentRow();if(m_readOnly || row<0 || !OracleConnection::isOracle())return;
    const int id=m_table->item(row,0)->data(Qt::UserRole).toInt();
    auto &manager=DatabaseManager::instance();const auto current=manager.getAllServices();
    if(!manager.lastError().isEmpty()){QMessageBox::warning(this,"Chargement impossible",manager.lastError());return;}
    for(const auto &s:current)if(s.getId()==id) {
        ServiceDialog form(s,false,this);
        while(form.exec()==QDialog::Accepted) {
            if(manager.updateService(form.getService())) {reload();emit servicesChanged();return;}
            QMessageBox::warning(this,"Enregistrement impossible",manager.lastError());
        }
        return;
    }
    reload();m_message->setText("Ce service a été supprimé. Le planning a été actualisé.");
}

