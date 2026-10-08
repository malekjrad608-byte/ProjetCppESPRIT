#include "servicedialog.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStyle>

ServiceDialog::ServiceDialog(QWidget *parent):QDialog(parent) { Design::applyThemeTo(this); setup(Service{},false,false); }
ServiceDialog::ServiceDialog(const Service &service,bool readOnly,QWidget *parent):QDialog(parent) { setup(service,service.id>0,readOnly); }
void ServiceDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event); Motion::reveal(this,QColor("#F3F7F8"),180);
}
void ServiceDialog::setup(const Service &s,bool editing,bool readOnly) {
    m_id=s.id; setObjectName("serviceDialog"); setMinimumWidth(570); setModal(true);
    const QString title=readOnly?"Détails du service":editing?"Modifier le service":"Nouveau service"; setWindowTitle("ANIWORLD — "+title);
    auto *root=new QVBoxLayout(this); root->setContentsMargins(27,24,27,24); root->setSpacing(17);
    auto *head=new QHBoxLayout; auto *text=new QVBoxLayout; text->setSpacing(5);
    text->addWidget(Design::label(readOnly?QString("PRESTATION  /  SRV-%1").arg(s.id,3,10,QChar('0')):"PRENDRE SOIN, SIMPLEMENT","eyebrow"));
    text->addWidget(Design::label(title,"title")); text->addWidget(Design::label(readOnly?"Toutes les informations de cette prestation.":"Les petites attentions commencent ici. * Champ obligatoire.","muted")); head->addLayout(text); head->addStretch();
    auto *logo=Design::label(""); logo->setPixmap(Design::brand("logo",QSize(57,57))); head->addWidget(logo); root->addLayout(head);
    auto *card=Design::card(); auto *form=new QGridLayout(card); form->setContentsMargins(20,19,20,20); form->setHorizontalSpacing(16); form->setVerticalSpacing(8); form->setColumnStretch(0,1); form->setColumnStretch(1,1);
    m_name=new QLineEdit(s.name); m_name->setObjectName("serviceName"); m_name->setMaxLength(120); m_name->setPlaceholderText("Ex. Toilettage complet");
    form->addWidget(Design::label("Nom du service *"),0,0,1,2); form->addWidget(m_name,1,0,1,2);
    m_category=new QComboBox; m_category->setObjectName("serviceCategory"); m_category->addItems(Design::categories()); m_category->setCurrentText(s.category);
    m_status=new QComboBox; m_status->setObjectName("serviceStatus"); m_status->addItems(Design::statuses()); m_status->setCurrentText(s.status);
    form->addWidget(Design::label("Catégorie"),2,0); form->addWidget(Design::label("Statut"),2,1); form->addWidget(m_category,3,0); form->addWidget(m_status,3,1);
    m_date=new QDateTimeEdit(s.dateTime); m_date->setObjectName("serviceDate"); m_date->setCalendarPopup(true); m_date->setDisplayFormat("dd/MM/yyyy  HH:mm"); m_date->setMinimumDate(QDate(2000,1,1));
    m_duration=new QSpinBox; m_duration->setObjectName("serviceDuration"); m_duration->setRange(5,1440); m_duration->setSingleStep(5); m_duration->setSuffix(" min"); m_duration->setValue(s.duration);
    form->addWidget(Design::label("Date et heure"),4,0); form->addWidget(Design::label("Durée"),4,1); form->addWidget(m_date,5,0); form->addWidget(m_duration,5,1);
    m_responsible=new QLineEdit(s.responsible); m_responsible->setObjectName("serviceResponsible"); m_responsible->setMaxLength(100); m_responsible->setPlaceholderText("Nom du responsable (facultatif)");
    m_price=new QDoubleSpinBox; m_price->setObjectName("servicePrice"); m_price->setRange(0,99999.99); m_price->setDecimals(2); m_price->setSuffix(" DT"); m_price->setValue(s.price);
    form->addWidget(Design::label("Responsable"),6,0); form->addWidget(Design::label("Tarif"),6,1); form->addWidget(m_responsible,7,0); form->addWidget(m_price,7,1);
    m_description=new QTextEdit; m_description->setObjectName("serviceDescription"); m_description->setAcceptRichText(false); m_description->setPlainText(s.description); m_description->setPlaceholderText("Décrivez la prestation ou ajoutez une attention particulière…"); m_description->setFixedHeight(88);
    form->addWidget(Design::label("Description et notes"),8,0,1,2); form->addWidget(m_description,9,0,1,2); root->addWidget(card);
    m_error=Design::label(""); m_error->setObjectName("validationError"); m_error->setStyleSheet("color:#B25C4A;font-size:12px;"); m_error->setWordWrap(true); m_error->hide(); root->addWidget(m_error);
    auto *buttons=new QHBoxLayout; buttons->addStretch(); auto *cancel=Design::button(readOnly?"Fermer":"Annuler"); cancel->setObjectName("cancelService"); connect(cancel,&QPushButton::clicked,this,&QDialog::reject); buttons->addWidget(cancel);
    if(!readOnly) {
        auto *save=Design::button(editing?"Enregistrer les modifications":"Créer le service","check","primary"); save->setObjectName("saveService"); save->setDefault(true); connect(save,&QPushButton::clicked,this,&ServiceDialog::validateAndAccept); buttons->addWidget(save);
    } else {
        m_name->setReadOnly(true); m_responsible->setReadOnly(true); m_description->setReadOnly(true);
        m_category->setEnabled(false); m_status->setEnabled(false); m_date->setReadOnly(true); m_duration->setReadOnly(true); m_price->setReadOnly(true);
    }
    root->addLayout(buttons); m_name->setFocus();
}
void ServiceDialog::validateAndAccept() {
    const bool valid=!m_name->text().trimmed().isEmpty();
    m_name->setProperty("invalid",!valid); m_name->style()->unpolish(m_name); m_name->style()->polish(m_name);
    if(!valid) { m_error->setText("Donnez un nom au service pour continuer."); m_error->show(); m_name->setFocus(); Motion::reveal(m_error,QColor("#F3F7F8")); return; }
    if(m_description->toPlainText().size()>5000) { m_error->setText("La description doit contenir au maximum 5 000 caractères."); m_error->show(); m_description->setFocus(); Motion::reveal(m_error,QColor("#F3F7F8")); return; }
    accept();
}
Service ServiceDialog::getService() const {
    Service s; s.id=m_id; s.name=m_name->text().trimmed(); s.category=m_category->currentText(); s.status=m_status->currentText();
    s.responsible=m_responsible->text().trimmed(); s.description=m_description->toPlainText().trimmed(); s.dateTime=m_date->dateTime(); s.duration=m_duration->value(); s.price=m_price->value(); return s;
}
