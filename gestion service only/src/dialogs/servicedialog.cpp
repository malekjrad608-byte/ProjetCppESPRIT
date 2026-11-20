#include "servicedialog.h"
#include "serviceplanningdialog.h"
#include "../models/servicereferences.h"
#include "../ui/formvalidation.h"
#include "../models/service.h"
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
#include <QRegularExpressionValidator>

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
    root->addWidget(Design::label(readOnly?QString("PRESTATION / %1").arg(s.getReference()):"* Champs obligatoires", "muted"));

    auto *form=new QFormLayout; form->setContentsMargins(0,0,0,0); form->setHorizontalSpacing(18); form->setVerticalSpacing(9);
    form->setLabelAlignment(Qt::AlignLeft|Qt::AlignVCenter); form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    using Check = ValidatedField;
    auto addField = [&](const QString &label, QWidget *input, Check::Validator check, bool required) {
        if (readOnly) {
            form->addRow(label, input);
            return;
        }
        auto *field = FormValidation::bind(input, std::move(check), required);
        m_fields.append(field);
        form->addRow(label, field);
    };
    const bool oracle = OracleConnection::isOracle();
    m_name = new QLineEdit(s.getName(), this);
    m_name->setObjectName(oracle ? "serviceReference" : "serviceName");
    if (oracle) {
        auto *referenceRow = new QHBoxLayout;
        auto *prefix = new QLabel("SRV-", this);
        prefix->setBuddy(m_name);
        referenceRow->addWidget(prefix);
        m_name->setText(s.getName().mid(4));
        m_name->setAccessibleName("Trois chiffres de la référence service");
        m_name->setPlaceholderText("123");
        m_name->setMaxLength(3);
        m_name->setFixedWidth(84);
        m_name->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{3}"), m_name));
        m_name->setReadOnly(readOnly);
        if (readOnly) {
            referenceRow->addWidget(m_name);
        } else {
            auto *check = FormValidation::bind(m_name, [this] {
                if (!m_name->hasAcceptableInput())
                    return Check::invalid("Saisissez trois chiffres après SRV-.");
                QString error;
                return Service::referenceDisponible("SRV-" + m_name->text(), m_id, &error)
                    ? Check::valid() : Check::invalid(error);
            }, true);
            m_fields.append(check);
            referenceRow->addWidget(check);
        }
        referenceRow->addStretch();
        form->addRow("Réf. service *", referenceRow);
    } else {
        m_name->setReadOnly(true);
        m_name->hide();
    }

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
    const auto owners=ServiceReferences::proprietaires(&ownersError);
    for(const auto &owner:owners)m_clientChoice->addItem(owner.second,owner.first);
    const QString savedOwner = s.getClientName();
    int ownerIndex=m_clientChoice->findText(savedOwner);
    if(ownerIndex<0 && editing) {
        m_clientChoice->addItem(savedOwner,QVariant::fromValue<qint64>(0));
        ownerIndex=m_clientChoice->count()-1;
    }
    if(ownerIndex<0 && s.getMedicalRecordId()>0)ownerIndex=m_clientChoice->findData(s.getMedicalRecordId());
    m_clientChoice->setCurrentIndex(qMax(0,ownerIndex));
    m_clientChoice->setToolTip(ownersError.isEmpty()?"Propriétaires lus depuis les dossiers médicaux.":ownersError);
    connect(m_clientChoice,qOverload<int>(&QComboBox::currentIndexChanged),this,[this](int index){
        m_medicalRecordId=m_clientChoice->itemData(index).toLongLong();
        const QString reference = index > 0 ? m_clientChoice->itemText(index) : QString();
        if (!OracleConnection::isOracle()) m_name->setText(reference);
        m_client->setText(reference);
    });
    if(m_clientChoice->currentData().toLongLong()>0) {
        m_medicalRecordId=m_clientChoice->currentData().toLongLong();
        if (!oracle) m_name->setText(m_clientChoice->currentText());
        m_client->setText(m_clientChoice->currentText());
    }
    m_client->setReadOnly(true);m_client->hide();
    addField("Propriétaire *", m_clientChoice, [this] {
        if (m_clientChoice->currentIndex() <= 0 || (m_id == 0 && m_medicalRecordId <= 0))
            return Check::invalid("Choisissez un propriétaire enregistré dans Médical.");
        return Check::valid();
    }, true);
    auto *source=Design::label(ownersError.isEmpty()
        ? (owners.isEmpty()?"Aucun propriétaire enregistré dans Médical.":"Choisissez le propriétaire dans la liste des dossiers médicaux.")
        : "Impossible de charger les propriétaires. Fermez puis rouvrez le formulaire.","muted");
    source->setObjectName("serviceReferenceSource");source->setWordWrap(true);source->setFixedHeight(36);form->addRow("",source);
    addField("Catégorie", m_category, [this] {
        return m_category->currentText().trimmed().isEmpty()
            ? Check::invalid("Choisissez une catégorie.") : Check::valid();
    }, true);
    addField("Statut", m_status, [this] {
        return Design::statuses().contains(m_status->currentText())
            ? Check::valid() : Check::invalid("Choisissez un statut.");
    }, true);
    addField("Date et heure", m_date, [this] {
        return m_date->hasAcceptableInput() && m_date->dateTime().isValid()
            ? Check::valid() : Check::invalid("Complétez la date et l’heure.");
    }, true);
    addField("Durée", m_duration, [this] {
        return m_duration->hasAcceptableInput()
            ? Check::valid() : Check::invalid("Saisissez une durée entre 5 et 1 440 minutes.");
    }, true);
    m_employee=new QComboBox(this); m_employee->setObjectName("serviceEmployee"); RelationChoices::employees(m_employee,s.getEmployeeId(),s.getResponsible());
    if(OracleConnection::isOracle()) {m_employee->hide();m_responsible->hide();}
    else {
        addField("Responsable", m_responsible, [this] {
            return m_responsible->text().trimmed().isEmpty() ? Check::neutral() : Check::valid();
        }, false);
        m_employee->hide();
    }
    m_newEmployeeNom=new QLineEdit;m_newEmployeeNom->setObjectName("serviceNewEmployeeNom");m_newEmployeeNom->setMaxLength(120);m_newEmployeeNom->setPlaceholderText("Nom");
    m_newEmployeePrenom=new QLineEdit;m_newEmployeePrenom->setObjectName("serviceNewEmployeePrenom");m_newEmployeePrenom->setMaxLength(120);m_newEmployeePrenom->setPlaceholderText("Prénom");
    auto *newEmployee=new QWidget;auto *employeeNames=new QHBoxLayout(newEmployee);employeeNames->setContentsMargins(0,0,0,0);employeeNames->addWidget(m_newEmployeeNom);employeeNames->addWidget(m_newEmployeePrenom);
    form->addRow("",newEmployee);newEmployee->hide();
    connect(m_employee,qOverload<int>(&QComboBox::currentIndexChanged),this,[this,newEmployee]{newEmployee->setVisible(m_employee->currentData().toInt()==-1);});
    addField("Tarif", m_price, [this] {
        return m_price->hasAcceptableInput()
            ? Check::valid() : Check::invalid("Saisissez un tarif entre 0 et 99 999,99 DT.");
    }, true);
    m_local=new QComboBox(this); m_local->setObjectName("serviceLocal"); LocalChoice::populate(m_local,s.getLocalId());
    if(OracleConnection::isOracle())m_local->hide();else form->addRow("Local",m_local);
    addField("Prestations", m_description, [this] {
        const QString text = m_description->toPlainText();
        if (text.size() > 5000)
            return Check::invalid("Les prestations sont limitées à 5 000 caractères.");
        return text.trimmed().isEmpty() ? Check::neutral() : Check::valid();
    }, false);
    root->addLayout(form);root->addStretch(1);
    m_error=Design::label(""); m_error->setObjectName("validationError"); m_error->setStyleSheet("color:#B25C4A;font-size:12px;"); m_error->setWordWrap(true); m_error->hide(); root->addWidget(m_error);
    auto *buttons=new QHBoxLayout; buttons->addStretch(); auto *cancel=Design::button(readOnly?"Fermer":"Annuler",{},"primary"); cancel->setObjectName("cancelService"); connect(cancel,&QPushButton::clicked,this,&QDialog::reject); cancel->setMinimumWidth(125);
    if(!readOnly) {
        auto *save=Design::button(editing?"Enregistrer":"Ajouter","check","primary"); save->setObjectName("saveService"); save->setDefault(true); connect(save,&QPushButton::clicked,this,&ServiceDialog::validateAndAccept); save->setMinimumWidth(125); buttons->addWidget(save);
    } else {
        m_client->setReadOnly(true); m_name->setReadOnly(true); m_responsible->setReadOnly(true); m_description->setReadOnly(true);
        m_employee->setEnabled(false); m_clientChoice->setEnabled(false); m_local->setEnabled(false); m_category->setEnabled(false); m_status->setEnabled(false); m_date->setReadOnly(true); m_duration->setReadOnly(true); m_price->setReadOnly(true);
    }
    buttons->addWidget(cancel); root->addLayout(buttons);
    if (oracle && !readOnly) { m_name->setFocus(); m_name->selectAll(); }
    else m_clientChoice->setFocus();
}
void ServiceDialog::validateAndAccept() {
    m_error->hide();
    QWidget *firstInvalid = nullptr;
    for (auto *field : m_fields) {
        if (!field->validateNow() && !firstInvalid)
            firstInvalid = field->field();
    }
    if (firstInvalid) {
        firstInvalid->setFocus();
        return;
    }
    accept();
}
Service ServiceDialog::getService() const {
    const QString name = OracleConnection::isOracle() ? "SRV-" + m_name->text() : m_name->text().trimmed();
    Service s(m_id, name, m_client->text().trimmed(),
              m_category->currentText(), m_date->dateTime(), m_status->currentText(),
              m_duration->value(), m_price->value());
    s.setLocalId(m_local->currentData().toInt());
    s.setMedicalRecordId(m_medicalRecordId);
    s.setClientName(m_client->text().trimmed()); s.setClientId(0);
    s.setEmployeeId(OracleConnection::isOracle()?0:m_employee->currentData().toInt());
    s.setNewEmployeeNom(m_newEmployeeNom->text().trimmed());s.setNewEmployeePrenom(m_newEmployeePrenom->text().trimmed());
    s.setResponsible(OracleConnection::isOracle()?m_employee->currentData(Qt::UserRole+1).toString():m_responsible->text().trimmed()); s.setDescription(m_description->toPlainText().trimmed()); s.setDateTime(m_date->dateTime()); s.setDuration(m_duration->value()); s.setPrice(m_price->value()); return s;
}

