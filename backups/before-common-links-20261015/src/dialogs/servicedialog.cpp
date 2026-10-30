#include "servicedialog.h"
#include "../ui/localchoice.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QStyle>

ServiceDialog::ServiceDialog(QWidget *parent):QDialog(parent) { setup(Service{},false,false); }
ServiceDialog::ServiceDialog(const Service &service,bool readOnly,QWidget *parent):QDialog(parent) { setup(service,service.id>0,readOnly); }
void ServiceDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event); Motion::reveal(this,QColor("#F3F7F8"),180);
}
void ServiceDialog::setup(const Service &s,bool editing,bool readOnly) {
    m_id=s.id; setObjectName("serviceDialog"); setMinimumWidth(470); setModal(true);
    const QString title=readOnly?"Détails du service":editing?"Modifier le service":"Nouveau service"; setWindowTitle("ANIWORLD — "+title);
    auto *root=new QVBoxLayout(this); root->setContentsMargins(22,20,22,20); root->setSpacing(12);
    auto *head=new QHBoxLayout; head->setSpacing(10);
    auto *symbol=Design::label(""); symbol->setPixmap(Design::icon(readOnly?"services":editing?"edit":"plus",QColor("#8B70C5")).pixmap(22,22)); head->addWidget(symbol);
    head->addWidget(Design::label(title,"section")); head->addStretch(); root->addLayout(head);
    root->addWidget(Design::label(readOnly?QString("PRESTATION / SRV-%1").arg(s.id,3,10,QChar('0')):"* Champs obligatoires", "muted"));
    auto *form=new QFormLayout; form->setContentsMargins(0,0,0,0); form->setHorizontalSpacing(18); form->setVerticalSpacing(9);
    form->setLabelAlignment(Qt::AlignLeft|Qt::AlignVCenter); form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    m_name=new QLineEdit(s.name); m_name->setObjectName("serviceName"); m_name->setMaxLength(120); m_name->setPlaceholderText("Ex. Toilettage complet");

    m_category=new QComboBox; m_category->setObjectName("serviceCategory"); m_category->addItems(Design::categories()); m_category->setCurrentText(s.category);
    m_status=new QComboBox; m_status->setObjectName("serviceStatus"); m_status->addItems(Design::statuses()); m_status->setCurrentText(s.status);

    m_date=new QDateTimeEdit(s.dateTime); m_date->setObjectName("serviceDate"); m_date->setCalendarPopup(true); m_date->setDisplayFormat("dd/MM/yyyy  HH:mm"); m_date->setMinimumDate(QDate(2000,1,1));
    m_duration=new QSpinBox; m_duration->setObjectName("serviceDuration"); m_duration->setRange(5,1440); m_duration->setSingleStep(5); m_duration->setSuffix(" min"); m_duration->setValue(s.duration);

    m_responsible=new QLineEdit(s.responsible); m_responsible->setObjectName("serviceResponsible"); m_responsible->setMaxLength(100); m_responsible->setPlaceholderText("Nom du responsable (facultatif)");
    m_price=new QDoubleSpinBox; m_price->setObjectName("servicePrice"); m_price->setRange(0,99999.99); m_price->setDecimals(2); m_price->setSuffix(" DT"); m_price->setValue(s.price);

    m_description=new QTextEdit; m_description->setObjectName("serviceDescription"); m_description->setAcceptRichText(false); m_description->setPlainText(s.description); m_description->setPlaceholderText("Décrivez la prestation ou ajoutez une attention particulière…"); m_description->setFixedHeight(72);
    m_client=new QLineEdit(s.clientName); m_client->setObjectName("serviceClient"); m_client->setMaxLength(120);
    m_client->setPlaceholderText("Nom et prénom du client");
    form->addRow("Service *",m_name); form->addRow("Client *",m_client);
    form->addRow("Catégorie",m_category); form->addRow("Statut",m_status);
    form->addRow("Date et heure",m_date); form->addRow("Durée",m_duration);
    form->addRow("Responsable",m_responsible); form->addRow("Tarif",m_price);
    m_local=new QComboBox; m_local->setObjectName("serviceLocal"); LocalChoice::populate(m_local,s.localId);
    form->addRow("Local",m_local);
    form->addRow("Prestations",m_description); root->addLayout(form);
    m_error=Design::label(""); m_error->setObjectName("validationError"); m_error->setStyleSheet("color:#B25C4A;font-size:12px;"); m_error->setWordWrap(true); m_error->hide(); root->addWidget(m_error);
    auto *buttons=new QHBoxLayout; buttons->addStretch(); auto *cancel=Design::button(readOnly?"Fermer":"Annuler",{},"primary"); cancel->setObjectName("cancelService"); connect(cancel,&QPushButton::clicked,this,&QDialog::reject); cancel->setMinimumWidth(125);
    if(!readOnly) {
        auto *save=Design::button(editing?"Enregistrer":"Ajouter","check","primary"); save->setObjectName("saveService"); save->setDefault(true); connect(save,&QPushButton::clicked,this,&ServiceDialog::validateAndAccept); save->setMinimumWidth(125); buttons->addWidget(save);
    } else {
        m_client->setReadOnly(true); m_name->setReadOnly(true); m_responsible->setReadOnly(true); m_description->setReadOnly(true);
        m_local->setEnabled(false); m_category->setEnabled(false); m_status->setEnabled(false); m_date->setReadOnly(true); m_duration->setReadOnly(true); m_price->setReadOnly(true);
    }
    buttons->addWidget(cancel); root->addLayout(buttons); m_name->setFocus();
}
void ServiceDialog::validateAndAccept() {
    const bool valid=!m_name->text().trimmed().isEmpty();
    m_name->setProperty("invalid",!valid); m_name->style()->unpolish(m_name); m_name->style()->polish(m_name);
    if(!valid) { m_error->setText("Donnez un nom au service pour continuer."); m_error->show(); m_name->setFocus(); Motion::reveal(m_error,QColor("#F3F7F8")); return; }
    const bool clientValid=!m_client->text().trimmed().isEmpty();
    m_client->setProperty("invalid",!clientValid); m_client->style()->unpolish(m_client); m_client->style()->polish(m_client);
    if(!clientValid) { m_error->setText("Renseignez le nom du client pour sa facture."); m_error->show(); m_client->setFocus(); return; }
    if(m_description->toPlainText().size()>5000) { m_error->setText("La description doit contenir au maximum 5 000 caractères."); m_error->show(); m_description->setFocus(); Motion::reveal(m_error,QColor("#F3F7F8")); return; }
    accept();
}
Service ServiceDialog::getService() const {
    Service s; s.id=m_id; s.localId=m_local->currentData().toInt(); s.name=m_name->text().trimmed(); s.category=m_category->currentText(); s.status=m_status->currentText();
    s.clientName=m_client->text().trimmed(); s.responsible=m_responsible->text().trimmed(); s.description=m_description->toPlainText().trimmed(); s.dateTime=m_date->dateTime(); s.duration=m_duration->value(); s.price=m_price->value(); return s;
}
