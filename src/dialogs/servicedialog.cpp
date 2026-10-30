#include "servicedialog.h"
#include "serviceplanningdialog.h"
#include "../medical/medicalrecord.h"
#include "../models/service.h"
#include "../employees/employe.h"
#include "../ui/localchoice.h"
#include "../ui/relationchoices.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QStyle>
#include <QTimer>
#include <QListView>

ServiceDialog::ServiceDialog(QWidget *parent):QDialog(parent) { setup(Service{},false,false); }
ServiceDialog::ServiceDialog(const Service &service,bool readOnly,QWidget *parent):QDialog(parent) { setup(service,service.getId()>0,readOnly); }
void ServiceDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event); Motion::reveal(this,QColor("#F3F7F8"),180);
}
void ServiceDialog::setup(const Service &s,bool editing,bool readOnly) {
    m_id=s.getId(); setObjectName("serviceDialog"); setMinimumWidth(470); setModal(true);
    m_medicalRecordId=s.getMedicalRecordId();
    const QString title=readOnly?"Détails du service":editing?"Modifier le service":"Nouveau service"; setWindowTitle("ANIWORLD — "+title);
    auto *root=new QVBoxLayout(this); root->setContentsMargins(22,20,22,20); root->setSpacing(12);
    auto *head=new QHBoxLayout; head->setSpacing(10);
    auto *symbol=Design::label(""); symbol->setPixmap(Design::icon(readOnly?"services":editing?"edit":"plus",QColor("#8B70C5")).pixmap(22,22)); head->addWidget(symbol);
    head->addWidget(Design::label(title,"section")); head->addStretch(); root->addLayout(head);
    root->addWidget(Design::label(readOnly?QString("PRESTATION / SRV-%1").arg(s.getId(),3,10,QChar('0')):"* Champs obligatoires", "muted"));

    auto *form=new QFormLayout; form->setContentsMargins(0,0,0,0); form->setHorizontalSpacing(18); form->setVerticalSpacing(9);
    form->setLabelAlignment(Qt::AlignLeft|Qt::AlignVCenter); form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    if(OracleConnection::isOracle()) {
        auto *id=new QLineEdit(m_id>0?QString::number(m_id):QString("Automatique"),this);
        id->setObjectName("serviceId");id->setReadOnly(true);form->addRow("ID service",id);
    }
    m_name=new QLineEdit(s.getName(),this); m_name->setObjectName("serviceName"); m_name->setMaxLength(120); m_name->setPlaceholderText("Ex. Consultation de suivi");

    m_category=new QComboBox; m_category->setObjectName("serviceCategory");
    m_category->setView(new QListView(m_category));
    m_category->setMaxVisibleItems(5);
    m_category->setStyleSheet("QComboBox { combobox-popup: 0; }");
    m_category->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_category->addItems(Design::categories());
    if(editing && m_category->findText(s.getCategory())<0)m_category->addItem(s.getCategory());
    m_category->setCurrentText(s.getCategory());
    m_status=new QComboBox; m_status->setObjectName("serviceStatus"); m_status->addItems(Design::statuses()); m_status->setCurrentText(s.getStatus());

    m_date=new QDateTimeEdit(s.getDateTime()); m_date->setObjectName("serviceDate"); m_date->setCalendarPopup(true); m_date->setDisplayFormat("dd/MM/yyyy  HH:mm"); m_date->setMinimumDate(QDate(2000,1,1));
    m_duration=new QSpinBox; m_duration->setObjectName("serviceDuration"); m_duration->setRange(5,1440); m_duration->setSingleStep(5); m_duration->setSuffix(" min"); m_duration->setValue(s.getDuration());

    m_responsible=new QLineEdit(s.getResponsible(),this); m_responsible->setObjectName("serviceResponsible"); m_responsible->setMaxLength(100); m_responsible->setPlaceholderText("Nom du responsable (facultatif)");
    m_price=new QDoubleSpinBox; m_price->setObjectName("servicePrice"); m_price->setRange(0,99999.99); m_price->setDecimals(2); m_price->setSuffix(" DT"); m_price->setValue(s.getPrice());

    m_description=new QTextEdit; m_description->setObjectName("serviceDescription"); m_description->setAcceptRichText(false); m_description->setPlainText(s.getDescription()); m_description->setPlaceholderText("Décrivez la prestation ou ajoutez une attention particulière…"); m_description->setFixedHeight(72);
    m_client=new QLineEdit(s.getClientName(),this); m_client->setObjectName("serviceClient"); m_client->setMaxLength(120);
    m_client->setPlaceholderText("Nom et prénom du client");
    m_clientChoice=new QComboBox(this); m_clientChoice->setObjectName("serviceClientChoice");
    m_clientChoice->setEditable(false);m_clientChoice->setMaxVisibleItems(6);m_clientChoice->setView(new QListView(m_clientChoice));
    m_clientChoice->addItem("Choisir un propriétaire enregistré dans Médical",QVariant::fromValue<qint64>(0));
    QString ownersError;
    const auto owners=MedicalRecord::proprietaires(&ownersError);
    for(const auto &owner:owners)m_clientChoice->addItem(owner.second,owner.first);
    int ownerIndex=m_clientChoice->findText(s.getName());
    if(ownerIndex<0 && editing) {
        m_clientChoice->addItem(s.getName(),QVariant::fromValue<qint64>(0));
        ownerIndex=m_clientChoice->count()-1;
    }
    if(ownerIndex<0 && s.getMedicalRecordId()>0)ownerIndex=m_clientChoice->findData(s.getMedicalRecordId());
    m_clientChoice->setCurrentIndex(qMax(0,ownerIndex));
    m_clientChoice->setToolTip(ownersError.isEmpty()?"Propriétaires lus depuis les dossiers médicaux. Une référence par service.":ownersError);
    connect(m_clientChoice,qOverload<int>(&QComboBox::currentIndexChanged),this,[this](int index){
        m_medicalRecordId=m_clientChoice->itemData(index).toLongLong();
        if(m_medicalRecordId>0) {m_name->setText(m_clientChoice->itemText(index));m_client->setText(m_clientChoice->itemText(index));}
        else if(m_id==0){m_name->clear();m_client->clear();}
    });
    if(m_clientChoice->currentData().toLongLong()>0) {
        m_medicalRecordId=m_clientChoice->currentData().toLongLong();
        m_name->setText(m_clientChoice->currentText());m_client->setText(m_clientChoice->currentText());
    }
    m_name->setReadOnly(true);m_name->hide();m_client->setReadOnly(true);m_client->hide();
    form->addRow("Réf. service *",m_clientChoice);
    auto *source=Design::label(ownersError.isEmpty()
        ? (owners.isEmpty()?"Aucun propriétaire enregistré dans Médical.":"Choisissez un propriétaire déjà enregistré dans Médical : son nom sera la référence du service.")
        : "Impossible de charger les propriétaires. Fermez puis rouvrez le formulaire.","muted");
    source->setObjectName("serviceReferenceSource");source->setWordWrap(true);source->setFixedHeight(36);form->addRow("",source);
    form->addRow("Catégorie",m_category); form->addRow("Statut",m_status);
    form->addRow("Date et heure",m_date); form->addRow("Durée",m_duration);
    m_employee=new QComboBox(this); m_employee->setObjectName("serviceEmployee"); RelationChoices::employees(m_employee,s.getEmployeeId(),s.getResponsible());
    if(OracleConnection::isOracle()) {m_employee->hide();m_responsible->hide();}
    else {form->addRow("Responsable",m_responsible);m_employee->hide();}
    m_newEmployeeNom=new QLineEdit;m_newEmployeeNom->setObjectName("serviceNewEmployeeNom");m_newEmployeeNom->setMaxLength(120);m_newEmployeeNom->setPlaceholderText("Nom");
    m_newEmployeePrenom=new QLineEdit;m_newEmployeePrenom->setObjectName("serviceNewEmployeePrenom");m_newEmployeePrenom->setMaxLength(120);m_newEmployeePrenom->setPlaceholderText("Prénom");
    auto *newEmployee=new QWidget;auto *employeeNames=new QHBoxLayout(newEmployee);employeeNames->setContentsMargins(0,0,0,0);employeeNames->addWidget(m_newEmployeeNom);employeeNames->addWidget(m_newEmployeePrenom);
    form->addRow("",newEmployee);newEmployee->hide();
    connect(m_employee,qOverload<int>(&QComboBox::currentIndexChanged),this,[this,newEmployee]{newEmployee->setVisible(m_employee->currentData().toInt()==-1);});
    form->addRow("Tarif",m_price);
    m_local=new QComboBox(this); m_local->setObjectName("serviceLocal"); LocalChoice::populate(m_local,s.getLocalId());
    if(OracleConnection::isOracle())m_local->hide();else form->addRow("Local",m_local);
    form->addRow("Prestations",m_description); root->addLayout(form);root->addStretch(1);
    m_error=Design::label(""); m_error->setObjectName("validationError"); m_error->setStyleSheet("color:#B25C4A;font-size:12px;"); m_error->setWordWrap(true); m_error->hide(); root->addWidget(m_error);
    auto *buttons=new QHBoxLayout; buttons->addStretch(); auto *cancel=Design::button(readOnly?"Fermer":"Annuler",{},"primary"); cancel->setObjectName("cancelService"); connect(cancel,&QPushButton::clicked,this,&QDialog::reject); cancel->setMinimumWidth(125);
    if(!readOnly) {
        auto *save=Design::button(editing?"Enregistrer":"Ajouter","check","primary"); save->setObjectName("saveService"); save->setDefault(true); connect(save,&QPushButton::clicked,this,&ServiceDialog::validateAndAccept); save->setMinimumWidth(125); buttons->addWidget(save);
    } else {
        m_client->setReadOnly(true); m_name->setReadOnly(true); m_responsible->setReadOnly(true); m_description->setReadOnly(true);
        m_employee->setEnabled(false); m_clientChoice->setEnabled(false); m_local->setEnabled(false); m_category->setEnabled(false); m_status->setEnabled(false); m_date->setReadOnly(true); m_duration->setReadOnly(true); m_price->setReadOnly(true);
    }
    buttons->addWidget(cancel); root->addLayout(buttons); m_clientChoice->setFocus();
}
void ServiceDialog::validateAndAccept() {
    const bool valid=!m_name->text().trimmed().isEmpty() && (m_id>0 || m_medicalRecordId>0);
    m_name->setProperty("invalid",!valid); m_name->style()->unpolish(m_name); m_name->style()->polish(m_name);
    if(!valid) { m_error->setText("Choisissez un propriétaire dans les dossiers médicaux."); m_error->show(); m_clientChoice->setFocus(); Motion::reveal(m_error,QColor("#F3F7F8")); return; }
    const bool clientValid=!m_client->text().trimmed().isEmpty();
    m_client->setProperty("invalid",!clientValid); m_client->style()->unpolish(m_client); m_client->style()->polish(m_client);
    if(!clientValid) { m_error->setText("Choisissez un propriétaire enregistré."); m_error->show(); m_client->setFocus(); return; }
    if(m_description->toPlainText().size()>5000) { m_error->setText("La description doit contenir au maximum 5 000 caractères."); m_error->show(); m_description->setFocus(); Motion::reveal(m_error,QColor("#F3F7F8")); return; }
    accept();
}
Service ServiceDialog::getService() const {
    Service s(m_id, m_name->text().trimmed(), m_client->text().trimmed(),
              m_category->currentText(), m_date->dateTime(), m_status->currentText(),
              m_duration->value(), m_price->value());
    s.setLocalId(m_local->currentData().toInt());
    s.setMedicalRecordId(m_medicalRecordId);
    s.setClientName(m_client->text().trimmed()); s.setClientId(0);
    s.setEmployeeId(OracleConnection::isOracle()?0:m_employee->currentData().toInt());
    s.setNewEmployeeNom(m_newEmployeeNom->text().trimmed());s.setNewEmployeePrenom(m_newEmployeePrenom->text().trimmed());
    s.setResponsible(OracleConnection::isOracle()?m_employee->currentData(Qt::UserRole+1).toString():m_responsible->text().trimmed()); s.setDescription(m_description->toPlainText().trimmed()); s.setDateTime(m_date->dateTime()); s.setDuration(m_duration->value()); s.setPrice(m_price->value()); return s;
}

