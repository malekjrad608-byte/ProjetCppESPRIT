#include "hrdialogs.h"
#include <QPdfWriter>
#include <QSaveFile>
#include <QTextDocument>
#include <cmath>

namespace AniRh {

void EditDialog::field(const QString &key,const QString &caption,QWidget *widget) {
    widget->setObjectName(key); widget->setAccessibleName(caption); m_fields[key]=widget;
    auto *l=Ui::label(caption); l->setBuddy(widget); m_form->addRow(l,widget);
}
QLineEdit *EditDialog::text(const QString &key,const QString &caption,const QString &placeholder) {
    auto *w=new QLineEdit(m_record[key].toString()); w->setMaxLength(key=="title"?150:100); w->setPlaceholderText(placeholder); field(key,caption,w); return w;
}
QComboBox *EditDialog::combo(const QString &key,const QString &caption,const QStringList &items) {
    auto *w=new QComboBox; w->addItems(items); if(m_record.contains(key)) w->setCurrentText(m_record[key].toString()); field(key,caption,w); return w;
}
QDateEdit *EditDialog::date(const QString &key,const QString &caption,const QDate &fallback) {
    auto *w=new QDateEdit; w->setCalendarPopup(true); w->setDisplayFormat("dd/MM/yyyy"); w->setDate(m_record.contains(key)?QDate::fromString(m_record[key].toString(),Qt::ISODate):fallback); field(key,caption,w); return w;
}
QDoubleSpinBox *EditDialog::amount(const QString &key,const QString &caption) {
    auto *w=new QDoubleSpinBox; w->setDecimals(3); w->setRange(0,1000000); w->setSuffix(" DT"); w->setSingleStep(50); w->setValue(m_record[key].toLongLong()/1000.0); field(key,caption,w); return w;
}
QSpinBox *EditDialog::number(const QString &key,const QString &caption,int low,int high,int fallback) {
    auto *w=new QSpinBox; w->setRange(low,high); w->setValue(m_record.value(key,fallback).toInt()); field(key,caption,w); return w;
}
void EditDialog::multiline(const QString &key,const QString &caption) {
    auto *w=new QPlainTextEdit(m_record[key].toString()); w->setFixedHeight(80); w->setTabChangesFocus(true); field(key,caption,w);
}
void EditDialog::employeePicker() {
    auto *w=new QComboBox;
    for(const auto &e:m_store->employees()) if(e["active"].toBool() || e["id"]==m_record["employee_id"]) w->addItem(e["name"].toString()+(!e["active"].toBool()?" · archivé":""),e["id"]);
    if(m_record.contains("employee_id")) w->setCurrentIndex(w->findData(m_record["employee_id"]));
    // Existing records retain their employee to preserve their history.
    if(m_record["id"].toInt()) w->setEnabled(false);
    field("employee_id","Employé *",w);
}
EditDialog::EditDialog(HrStore *store,Kind kind,const Record &record,QWidget *parent):QDialog(parent),m_store(store),m_kind(kind),m_record(record) {
    setObjectName("editDialog"); Ui::applyTheme(this); setModal(true); resize(610,qMin(kind==Employee || kind==Review?740:640,QApplication::primaryScreen()->availableGeometry().height()-90)); setMinimumWidth(560);
    const bool editing=record["id"].toInt()>0;
    const QStringList titles={editing?"Modifier le dossier":"Ajouter un employé",editing?"Modifier le pointage":"Enregistrer un pointage","Demander un congé",editing?"Modifier la mission":"Attribuer une mission",editing?"Modifier l'évaluation":"Nouvelle évaluation","Préparer le bulletin",editing?"Modifier le créneau":"Planifier un créneau"};
    const QStringList subtitles={"Les informations essentielles de votre collaborateur.","Un pointage par employé et par journée.","Les dates sont décomptées en jours calendaires, bornes incluses.","Un responsable, une échéance et des objectifs clairs.","Une évaluation de 1 (à améliorer) à 5 (excellent).","Salaire de base figé à la création. Ajustements saisis manuellement.","Une affectation et des horaires pour chaque collaborateur."};
    setWindowTitle(titles[kind]+" · ANIWORLD RH");
    auto *root=new QVBoxLayout(this); root->setContentsMargins(25,22,25,22); root->setSpacing(15);
    auto *heading=Ui::label(titles[kind],"title"); heading->setStyleSheet("font-size:24px;font-weight:700;"); root->addWidget(heading);
    auto *subtitle=Ui::label(subtitles[kind],"muted"); subtitle->setWordWrap(true); root->addWidget(subtitle);
    auto *scroll=new QScrollArea; scroll->setWidgetResizable(true); auto *body=new QWidget; body->setStyleSheet("background:transparent;");
    m_form=new QFormLayout(body); m_form->setContentsMargins(0,8,12,8); m_form->setSpacing(12); m_form->setLabelAlignment(Qt::AlignLeft|Qt::AlignVCenter); m_form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    scroll->setWidget(body); root->addWidget(scroll,1);
    const QDate today=QDate::fromString(HrStore::today(),Qt::ISODate);
    if(kind==Employee) {
        text("first_name","Prénom *","Amira"); text("last_name","Nom *","Ben Ali"); text("email","Email *","prenom.nom@exemple.com"); text("phone","Téléphone","+216 …");
        text("role","Poste *","Vétérinaire, responsable, soigneur…");
        auto *depart=combo("department","Département *",{"Soins vétérinaires","Bien-être animal","Administration","Logistique","Autre"}); depart->setEditable(true); depart->lineEdit()->setMaxLength(100); if(editing) depart->setEditText(record["department"].toString());
        combo("contract","Contrat *",{"CDI","CDD","Stage","Temps partiel"}); date("hired","Date d'embauche *",today);
        amount("salary","Salaire mensuel *"); number("allowance","Quota annuel (jours) *",0,366,30);
        auto *note=Ui::label("Quota configurable par année civile. Les congés approuvés sont déduits en jours calendaires.","muted"); note->setWordWrap(true); m_form->addRow(note);
    } else if(kind==Attendance) {
        employeePicker(); auto *day=date("day","Date *",today); day->setMaximumDate(today);
        auto *status=combo("status","Statut *",{"Présent","Retard","Absent","Télétravail"});
        auto *arrival=new QTimeEdit; arrival->setDisplayFormat("HH:mm"); arrival->setTime(QTime::fromString(record.value("arrival","08:30").toString(),"HH:mm")); field("arrival","Arrivée *",arrival);
        auto *departure=new QTimeEdit; departure->setDisplayFormat("HH:mm"); departure->setTime(QTime::fromString(record.value("departure","17:30").toString(),"HH:mm")); field("departure","Départ",departure);
        auto *done=new QCheckBox("La journée est terminée"); done->setChecked(!record.value("departure",QString()).toString().isEmpty()); field("closed","",done);
        auto update=[status,arrival,departure,done]{ bool present=status->currentText()!="Absent"; arrival->setEnabled(present); done->setEnabled(present); departure->setEnabled(present && done->isChecked()); };
        connect(status,&QComboBox::currentTextChanged,this,update); connect(done,&QCheckBox::toggled,this,update); update(); multiline("notes","Notes");
    } else if(kind==Leave) {
        employeePicker(); combo("type","Type de congé *",{"Congé annuel","Maladie","Exceptionnel","Sans solde"});
        auto *start=date("start","Du *",today.addDays(1)); auto *end=date("end","Au *",today.addDays(1));
        m_hint=Ui::label("","muted"); m_hint->setWordWrap(true); m_form->addRow(m_hint);
        auto update=[this,start,end] {
            const int employee=qobject_cast<QComboBox *>(m_fields["employee_id"])->currentData().toInt();
            const int quota=int(m_store->scalar("SELECT allowance FROM employees WHERE id=?",{employee}));
            m_hint->setText(QString("Durée : %1 jour(s) calendaire(s)\nSolde annuel %2 : %3 jour(s), hors demandes en attente.").arg(qMax<qint64>(0,start->date().daysTo(end->date())+1)).arg(start->date().year()).arg(quota-m_store->usedLeave(employee,start->date().year())));
        };
        connect(start,&QDateEdit::dateChanged,this,update); connect(end,&QDateEdit::dateChanged,this,update); connect(qobject_cast<QComboBox *>(m_fields["employee_id"]),&QComboBox::currentIndexChanged,this,update); update(); multiline("reason","Motif");
    } else if(kind==Task) {
        text("title","Mission *","Ex. Organiser le planning de soins"); employeePicker(); date("due","Échéance *",today.addDays(1));
        combo("priority","Priorité *",{"Normale","Haute","Basse"}); combo("status","Statut *",{"À faire","En cours","Terminé"}); multiline("description","Responsabilités");
    } else if(kind==Review) {
        employeePicker(); date("day","Date *",today)->setMaximumDate(today); text("reviewer","Évaluateur *","Nom du responsable");
        number("quality","Qualité du travail",1,5,3); number("teamwork","Esprit d'équipe",1,5,3); number("punctuality","Ponctualité",1,5,3); number("initiative","Initiative",1,5,3);
        multiline("feedback","Bilan"); multiline("goals","Objectifs à venir");
    } else if(kind==Shift) {
        employeePicker();
        // A shift may be reassigned; unlike historical attendance its employee is editable.
        m_fields["employee_id"]->setEnabled(true);
        date("day","Date *",today);
        auto *start=new QTimeEdit;start->setDisplayFormat("HH:mm");start->setTime(QTime::fromString(record.value("start","08:30").toString(),"HH:mm"));field("start","Début *",start);
        auto *end=new QTimeEdit;end->setDisplayFormat("HH:mm");end->setTime(QTime::fromString(record.value("end","17:00").toString(),"HH:mm"));field("end","Fin *",end);
        text("assignment","Affectation *","Ex. Consultations, accueil, soins…");multiline("notes","Consignes");
        auto *note=Ui::label("Créneaux sur une même journée. Les chevauchements et les congés approuvés sont vérifiés. Pour une pause non travaillée, créez deux créneaux distincts.","muted");note->setWordWrap(true);m_form->addRow(note);
    } else {
        m_form->addRow("Employé",Ui::label(record["employee_name"].toString(),"section")); m_form->addRow("Période",Ui::label(record["month"].toString()));
        m_form->addRow("Salaire de base",Ui::label(Ui::money(record["base"].toLongLong())));
        auto *bonus=amount("bonus","Primes"); auto *deductions=amount("deductions","Retenues");
        m_hint=Ui::label("","section"); m_form->addRow("Net à payer",m_hint);
        auto update=[this,bonus,deductions]{ m_hint->setText(Ui::money(m_record["base"].toLongLong()+qRound64(bonus->value()*1000)-qRound64(deductions->value()*1000))); };
        connect(bonus,&QDoubleSpinBox::valueChanged,this,update); connect(deductions,&QDoubleSpinBox::valueChanged,this,update); update(); multiline("notes","Détail des ajustements");
        auto *note=Ui::label("Les cotisations, impôts, absences et proratas ne sont pas calculés automatiquement. Saisissez les retenues et justifiez les ajustements avant validation.","muted"); note->setWordWrap(true); m_form->addRow(note);
    }
    m_error=Ui::label("","error"); m_error->setObjectName("formError"); m_error->setWordWrap(true); m_error->hide(); root->addWidget(m_error);
    auto *buttons=new QHBoxLayout; auto *required=Ui::label("* Champs obligatoires","muted"); buttons->addWidget(required); buttons->addStretch();
    auto *cancel=Ui::button("Annuler"); auto *submit=Ui::button(kind==Leave?"Envoyer la demande":"Enregistrer","check","primary"); submit->setObjectName("saveButton"); submit->setDefault(true); cancel->setAutoDefault(false);
    buttons->addWidget(cancel); buttons->addWidget(submit); root->addLayout(buttons); connect(cancel,&QPushButton::clicked,this,&QDialog::reject); connect(submit,&QPushButton::clicked,this,&EditDialog::save);
    QTimer::singleShot(0,this,[this]{ Motion::reveal(this); });
}
void EditDialog::save() {
    Record d;
    for(auto it=m_fields.cbegin();it!=m_fields.cend();++it) {
        QWidget *w=it.value(); const auto &key=it.key();
        if(auto *input=qobject_cast<QLineEdit *>(w)) d[key]=input->text().trimmed();
        else if(auto *input=qobject_cast<QComboBox *>(w)) d[key]=key=="employee_id"?input->currentData():QVariant(input->currentText().trimmed());
        else if(auto *input=qobject_cast<QDateEdit *>(w)) d[key]=input->date().toString(Qt::ISODate);
        else if(auto *input=qobject_cast<QTimeEdit *>(w)) d[key]=input->time().toString("HH:mm");
        else if(auto *input=qobject_cast<QDoubleSpinBox *>(w)) d[key]=qRound64(input->value()*1000);
        else if(auto *input=qobject_cast<QSpinBox *>(w)) d[key]=input->value();
        else if(auto *input=qobject_cast<QPlainTextEdit *>(w)) d[key]=input->toPlainText().trimmed();
        else if(auto *input=qobject_cast<QCheckBox *>(w)) d[key]=input->isChecked();
    }
    const int id=m_record["id"].toInt(); bool ok=false;
    if(m_kind==Employee) ok=m_store->saveEmployee(d,id);
    else if(m_kind==Attendance) { if(!d.take("closed").toBool()) d["departure"]=""; ok=m_store->saveAttendance(d,id); }
    else if(m_kind==Leave) ok=m_store->saveLeave(d);
    else if(m_kind==Task) ok=m_store->saveTask(d,id);
    else if(m_kind==Review) ok=m_store->saveReview(d,id);
    else if(m_kind==Shift) ok=m_store->saveShift(d,id);
    else ok=m_store->updatePayroll(id,d["bonus"].toLongLong(),d["deductions"].toLongLong(),d["notes"].toString());
    if(ok) accept(); else { m_error->setText(m_store->error()); m_error->show(); }
}
bool exportPayslip(const QString &path,const Record &r,bool demo,QString *error) {
    QSaveFile file(path); if(!file.open(QIODevice::WriteOnly)) { if(error) *error=file.errorString(); return false; }
    {
        QPdfWriter writer(&file); writer.setResolution(96); writer.setPageSize(QPageSize(QPageSize::A4)); writer.setPageMargins(QMarginsF(18,18,18,18)); writer.setTitle("ANIWORLD RH — Bulletin "+r["month"].toString()); writer.setCreator("ANIWORLD RH");
        QTextDocument doc; doc.setDefaultFont(QFont("Segoe UI",11)); doc.addResource(QTextDocument::ImageResource,QUrl("brand"),QImage(":/brand/logo.png"));
        auto escape=[](const QVariant &v){ return v.toString().toHtmlEscaped().replace('\n',"<br>"); };
        const QString status=r["status"].toString();
        QString html="<style>body{color:#294751;}h1{font-size:26px;}h2{color:#397F8B;}th{background:#E7F0F2;text-align:left;}td,th{padding:12px;}p{line-height:1.5;}</style>";
        html+="<table width='100%'><tr><td><img src='brand' width='80' height='80'></td><td><h1>ANIWORLD</h1>Ressources humaines</td></tr></table><h2>Bulletin de paie — "+escape(r["month"])+"</h2>";
        html+="<p><b>"+(demo?QString("DÉMONSTRATION · "):QString())+escape(status)+"</b><br>Référence : RH-"+escape(r["id"])+"<br>Employé : <b>"+escape(r["employee_name"])+"</b><br>Poste : "+escape(r["employee_role"])+"</p>";
        html+="<table width='100%' border='0' cellspacing='0'><tr><th>Élément</th><th align='right'>Montant</th></tr>";
        for(const auto &pair:QList<QPair<QString,qint64>>{{"Salaire de base",r["base"].toLongLong()},{"Primes",r["bonus"].toLongLong()},{"Retenues",r["deductions"].toLongLong()}}) html+="<tr><td>"+pair.first+"</td><td align='right'>"+Ui::money(pair.second)+"</td></tr>";
        html+="<tr><th>Net à payer</th><th align='right'>"+Ui::money(r["base"].toLongLong()+r["bonus"].toLongLong()-r["deductions"].toLongLong())+"</th></tr></table>";
        if(status=="Payé") html+="<p>Paiement enregistré le "+Ui::date(r["paid_at"].toString())+".</p>";
        if(!r["notes"].toString().isEmpty()) html+="<h3>Détail des ajustements</h3><p>"+escape(r["notes"])+"</p>";
        html+="<p style='color:#81939B;font-size:10px;'>Document de gestion interne. Montants et retenues saisis manuellement ; aucune cotisation, fiscalité ou proratisation automatique. Le statut Payé constate une saisie et ne déclenche aucun virement.</p>";
        doc.setHtml(html); doc.setPageSize(QSizeF(writer.width(),writer.height())); QPainter painter(&writer);
        if(!painter.isActive()) { if(error) *error="Impossible de créer le PDF."; return false; }
        for(int page=0;page<doc.pageCount();++page) {
            if(page && !writer.newPage()) { if(error) *error="Impossible d'ajouter une page PDF."; return false; }
            painter.save(); painter.translate(0,-page*writer.height()); doc.drawContents(&painter,QRectF(0,page*writer.height(),writer.width(),writer.height())); painter.restore();
        }
        painter.end();
    }
    if(!file.commit()) { if(error) *error=file.errorString(); return false; } return true;
}

} // namespace AniRh
