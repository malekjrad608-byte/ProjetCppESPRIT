#include "hrschedule.h"
#include "hrdialogs.h"

namespace AniRh {

ScheduleWidget::ScheduleWidget(HrStore *store,bool demo,QWidget *parent):QWidget(parent),m_store(store),m_demo(demo) {
    setObjectName("schedulePage");auto *root=new QVBoxLayout(this);root->setContentsMargins(30,25,30,25);root->setSpacing(18);
    auto *header=new QHBoxLayout;auto *title=new QVBoxLayout;title->setSpacing(5);title->addWidget(Ui::label("Planning de l'équipe","title"));title->addWidget(Ui::label("La bonne personne, au bon endroit, au bon moment.","muted"));header->addLayout(title,1);
    auto *add=Ui::button("Planifier un créneau","plus","primary");add->setObjectName("createShift");header->addWidget(add);connect(add,&QPushButton::clicked,this,&ScheduleWidget::createShift);root->addLayout(header);
    auto *weekbar=Ui::card();auto *wl=new QHBoxLayout(weekbar);wl->setContentsMargins(16,12,16,12);wl->setSpacing(9);
    auto *prev=Ui::button("‹");prev->setObjectName("previousWeek");prev->setAccessibleName("Semaine précédente");prev->setFixedWidth(37);auto *next=Ui::button("›");next->setObjectName("nextWeek");next->setAccessibleName("Semaine suivante");next->setFixedWidth(37);
    wl->addWidget(prev);wl->addWidget(next);m_range=Ui::label("","section");wl->addSpacing(6);wl->addWidget(m_range,1);
    auto *today=Ui::button("Cette semaine");today->setObjectName("currentWeek");wl->addWidget(today);m_week=new QDateEdit; m_week->setObjectName("scheduleDate");m_week->setMinimumWidth(140);m_week->setCalendarPopup(true);m_week->setDisplayFormat("dd/MM/yyyy");m_week->setDate(QDate::fromString(HrStore::today(),Qt::ISODate));wl->addWidget(m_week);root->addWidget(weekbar);
    connect(prev,&QPushButton::clicked,this,[this]{m_week->setDate(m_week->date().addDays(-7));});connect(next,&QPushButton::clicked,this,[this]{m_week->setDate(m_week->date().addDays(7));});connect(today,&QPushButton::clicked,this,[this]{m_week->setDate(QDate::fromString(HrStore::today(),Qt::ISODate));});connect(m_week,&QDateEdit::dateChanged,this,&ScheduleWidget::refresh);
    auto *card=Ui::card();auto *cl=new QVBoxLayout(card);cl->setContentsMargins(0,0,0,0);cl->setSpacing(0);auto *toolbar=new QHBoxLayout;toolbar->setContentsMargins(16,15,16,15);toolbar->setSpacing(10);
    m_search=new QLineEdit;m_search->setObjectName("scheduleSearch");m_search->setPlaceholderText("Rechercher un collaborateur…");m_search->addAction(Ui::icon("search"),QLineEdit::LeadingPosition);m_search->setClearButtonEnabled(true);toolbar->addWidget(m_search,1);
    m_department=new QComboBox;m_department->setObjectName("scheduleDepartment");m_department->addItem("Tous les départements");m_department->setMinimumWidth(175);toolbar->addWidget(m_department);
    auto *csv=Ui::button("Exporter","export");toolbar->addWidget(csv);connect(csv,&QPushButton::clicked,this,&ScheduleWidget::exportWeek);cl->addLayout(toolbar);
    m_grid=new QTableWidget;m_grid->setObjectName("scheduleGrid");Ui::setupTable(m_grid,{"ÉQUIPE","LUN","MAR","MER","JEU","VEN","SAM","DIM"});m_grid->setSelectionMode(QAbstractItemView::NoSelection);m_grid->setShowGrid(true);m_grid->setStyleSheet("QTableWidget{gridline-color:#EDF2F4;} QHeaderView::section{padding:14px 7px;}");m_grid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);m_grid->horizontalHeader()->setMinimumSectionSize(122);m_grid->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);m_grid->setColumnWidth(0,190);cl->addWidget(m_grid,1);
    m_empty=Ui::label("Ajoutez votre équipe pour commencer à planifier.","muted");m_empty->setAlignment(Qt::AlignCenter);m_empty->setMinimumHeight(200);cl->addWidget(m_empty,1);
    auto *footer=new QHBoxLayout;footer->setContentsMargins(17,13,17,13);m_summary=Ui::label("","muted");m_summary->setWordWrap(true);footer->addWidget(m_summary,1);footer->addWidget(Ui::label("●  Travail planifié     ●  Congé approuvé","muted"));cl->addLayout(footer);root->addWidget(card,1);
    auto *actions=new QHBoxLayout;auto *copy=Ui::button("Copier vers la semaine suivante","calendar");copy->setObjectName("copyWeek");actions->addWidget(copy);actions->addStretch();auto *hint=Ui::label("Double-cliquez sur une case vide pour ajouter un créneau.","muted");hint->setWordWrap(true);actions->addWidget(hint);root->addLayout(actions);
    connect(copy,&QPushButton::clicked,this,&ScheduleWidget::copyWeek);connect(m_search,&QLineEdit::textChanged,this,&ScheduleWidget::refresh);connect(m_department,&QComboBox::currentTextChanged,this,&ScheduleWidget::refresh);
    connect(m_grid,&QTableWidget::cellDoubleClicked,this,[this](int row,int col){if(col<1 || !m_grid->item(row,0))return;Record r={{"employee_id",m_grid->item(row,0)->data(Qt::UserRole)},{"day",monday().addDays(col-1).toString(Qt::ISODate)}};editShift(r);});
    connect(store,&HrStore::changed,this,&ScheduleWidget::refresh,Qt::QueuedConnection);refresh();
}
QDate ScheduleWidget::monday() const {return m_week->date().addDays(1-m_week->date().dayOfWeek());}
void ScheduleWidget::refresh() {
    const QDate start=monday(),end=start.addDays(6);const QString first=start.toString(Qt::ISODate),last=end.toString(Qt::ISODate);
    m_range->setText(QString("%1 — %2").arg(QLocale().toString(start,"d MMM"),QLocale().toString(end,"d MMM yyyy")));
    const QString department=m_department->currentText();
    {QSignalBlocker block(m_department);m_department->clear();m_department->addItem("Tous les départements");for(const auto &r:m_store->rows("SELECT DISTINCT department FROM employees ORDER BY department"))m_department->addItem(r["department"].toString());const int found=m_department->findText(department);m_department->setCurrentIndex(qMax(0,found));}
    QStringList headers={"COLLABORATEUR"};for(int i=0;i<7;++i)headers.append(QLocale().toString(start.addDays(i),"ddd dd/MM").toUpper());m_grid->setHorizontalHeaderLabels(headers);
    const auto shifts=m_store->rows("SELECT * FROM shifts WHERE day BETWEEN ? AND ? ORDER BY day,start",{first,last});
    const auto leaves=m_store->rows("SELECT * FROM leaves WHERE status='Approuvé' AND start<=? AND end>=?",{last,first});
    Records people;const QString search=Ui::normalized(m_search->text());
    for(const auto &e:m_store->employees()) {
        const int id=e["id"].toInt();bool scheduled=false;for(const auto &s:shifts)if(s["employee_id"].toInt()==id)scheduled=true;for(const auto &l:leaves)if(l["employee_id"].toInt()==id)scheduled=true;
        if((!e["active"].toBool() || e["hired"].toString()>last) && !scheduled)continue;
        if(m_department->currentIndex()>0 && e["department"].toString()!=m_department->currentText())continue;
        if(!search.isEmpty() && !Ui::normalized(e["name"].toString()+" "+e["role"].toString()+" "+e["department"].toString()).contains(search))continue;
        people.append(e);
    }
    m_grid->setRowCount(0);m_grid->setRowCount(people.size());int count=0,minutes=0;
    for(int row=0;row<people.size();++row) {
        const auto &e=people[row];const int id=e["id"].toInt();auto *identity=new QTableWidgetItem;identity->setData(Qt::UserRole,id);m_grid->setItem(row,0,identity);m_grid->setCellWidget(row,0,Ui::person(e["name"].toString(),e["role"].toString()+(!e["active"].toBool()?" · archivé":""),id));int maxEntries=1;
        for(int col=1;col<8;++col) {
            const QString day=start.addDays(col-1).toString(Qt::ISODate);auto *item=new QTableWidgetItem;item->setToolTip("Double-clic : planifier un créneau");m_grid->setItem(row,col,item);
            if(day==HrStore::today())item->setBackground(QColor("#F1F8F9"));else if(col>=6)item->setBackground(QColor("#FBFCFD"));
            QString leaveType;for(const auto &l:leaves)if(l["employee_id"].toInt()==id && l["start"].toString()<=day && l["end"].toString()>=day)leaveType=l["type"].toString();
            if(!leaveType.isEmpty()){auto *label=Ui::label("En congé\n"+leaveType);label->setWordWrap(true);label->setAlignment(Qt::AlignCenter);label->setStyleSheet("margin:5px;background:#FFF3DD;border-radius:8px;color:#A07E42;font-size:10px;padding:7px;");label->setAttribute(Qt::WA_TransparentForMouseEvents);m_grid->setCellWidget(row,col,label);continue;}
            Records dayShifts;for(const auto &s:shifts)if(s["employee_id"].toInt()==id && s["day"]==day)dayShifts.append(s);
            if(dayShifts.isEmpty()){item->setText(e["hired"].toString()>day?"—":"+");item->setTextAlignment(Qt::AlignCenter);item->setForeground(QColor("#CADADD"));continue;}
            maxEntries=qMax(maxEntries,int(dayShifts.size()));auto *cell=new QWidget;auto *layout=new QVBoxLayout(cell);layout->setContentsMargins(5,5,5,5);layout->setSpacing(5);
            for(const auto &shift:dayShifts){
                ++count;minutes+=QTime::fromString(shift["start"].toString(),"HH:mm").secsTo(QTime::fromString(shift["end"].toString(),"HH:mm"))/60;
                auto *button=Ui::button(shift["start"].toString()+" – "+shift["end"].toString()+"\n"+shift["assignment"].toString());button->setMinimumHeight(55);button->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);button->setStyleSheet("QPushButton{background:#E7F2F4;border:1px solid #D5E9ED;border-left:3px solid #5B9EAA;border-radius:6px;color:#447782;text-align:left;font-size:10px;padding:7px 5px;}QPushButton:hover{background:#D5EBEF;}");button->setToolTip(shift["start"].toString()+" – "+shift["end"].toString()+"\n"+shift["assignment"].toString()+"\n"+shift["notes"].toString()+"\nCliquer pour modifier ou supprimer");connect(button,&QPushButton::clicked,this,[this,shift]{editShift(shift);});layout->addWidget(button);
            }
            layout->addStretch();m_grid->setCellWidget(row,col,cell);
        }
        m_grid->setRowHeight(row,qMax(91,maxEntries*65+8));
    }
    m_grid->setVisible(!people.isEmpty());m_empty->setVisible(people.isEmpty());m_empty->setText(m_store->employees().isEmpty()?"Votre planning commence avec votre équipe.\nAjoutez des employés dans le module Employés.":"Aucun collaborateur pour cette semaine et ces filtres.");
    m_summary->setText(QString("%1 collaborateurs    ·    %2 créneaux    ·    %3 h %4 planifiées").arg(people.size()).arg(count).arg(minutes/60).arg(minutes%60,2,10,QChar('0')));
}
void ScheduleWidget::createShift() {if(m_store->employees(true).isEmpty()){QMessageBox::information(this,"Équipe requise","Ajoutez ou réactivez un employé avant de planifier un créneau.");return;}editShift({{"day",m_week->date().toString(Qt::ISODate)}});}
void ScheduleWidget::editShift(const Record &record) {
    EditDialog dialog(m_store,EditDialog::Shift,record,this);
    if(record["id"].toInt()){
        auto *remove=Ui::button("Supprimer ce créneau",{},"danger");remove->setObjectName("deleteShift");dialog.layout()->addWidget(remove);
        connect(remove,&QPushButton::clicked,&dialog,[this,&dialog,record]{
            QMessageBox box(QMessageBox::Question,"Supprimer le créneau","Supprimer ce créneau de travail ?",QMessageBox::NoButton,&dialog);auto *confirm=box.addButton("Supprimer",QMessageBox::AcceptRole);box.addButton("Annuler",QMessageBox::RejectRole);box.exec();
            if(box.clickedButton()!=confirm)return;
            if(m_store->removeRecord("shifts",record["id"].toInt()))dialog.accept();else QMessageBox::warning(&dialog,"Suppression impossible",m_store->error());
        });
    }
    if(dialog.exec()==QDialog::Accepted)refresh();
}
void ScheduleWidget::copyWeek() {
    QMessageBox box(QMessageBox::Question,"Copier le planning","Copier tous les créneaux de la semaine affichée vers la semaine suivante ?\n\nCette action inclut les collaborateurs masqués par les filtres. Si un chevauchement, un congé ou un dossier archivé est détecté, aucun créneau ne sera copié.",QMessageBox::NoButton,this);auto *confirm=box.addButton("Copier la semaine",QMessageBox::AcceptRole);box.addButton("Annuler",QMessageBox::RejectRole);box.exec();if(box.clickedButton()!=confirm)return;
    const auto start=monday();if(!m_store->copyScheduleWeek(start)){QMessageBox::warning(this,"Copie non effectuée",m_store->error());return;}m_week->setDate(start.addDays(7));
}
void ScheduleWidget::exportWeek() {
    const auto shifts=m_store->rows("SELECT s.*,e.first_name||' '||e.last_name AS name,e.department FROM shifts s JOIN employees e ON e.id=s.employee_id WHERE day BETWEEN ? AND ? ORDER BY day,start,name",{monday().toString(Qt::ISODate),monday().addDays(6).toString(Qt::ISODate)});QList<QStringList> rows;
    QSet<int> shown;for(int i=0;i<m_grid->rowCount();++i)shown.insert(m_grid->item(i,0)->data(Qt::UserRole).toInt());
    for(const auto &s:shifts)if(shown.contains(s["employee_id"].toInt())){QStringList r={s["name"].toString(),s["department"].toString(),s["day"].toString(),s["start"].toString(),s["end"].toString(),s["assignment"].toString(),s["notes"].toString()};if(m_demo)r.prepend("DÉMONSTRATION");rows.append(r);}
    if(rows.isEmpty()){QMessageBox::information(this,"Planning vide","Aucun créneau à exporter pour les collaborateurs affichés.");return;}
    QString path=QFileDialog::getSaveFileName(this,"Exporter la semaine",QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/Planning_"+monday().toString(Qt::ISODate)+(m_demo?"_DEMO":"")+".csv","CSV UTF-8 (*.csv)");if(path.isEmpty())return;if(!path.endsWith(".csv",Qt::CaseInsensitive))path+=".csv";
    QStringList headers={"Employé","Département","Date","Début","Fin","Affectation","Consignes"};if(m_demo)headers.prepend("Mode");QString error;if(!Ui::exportCsv(path,headers,rows,&error))QMessageBox::warning(this,"Export impossible",error);else QMessageBox::information(this,"Export terminé","Le planning de la semaine a été enregistré.");
}

} // namespace AniRh
