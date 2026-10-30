#include "medicalwidget.h"
#include "medicalstore.h"
#include "medicalrecorddialog.h"
#include "medicalrecorddetailsdialog.h"
#include "recordcard.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include "../ui/invoice.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPainter>
#include <QTimer>
#include <QEvent>
#include <QStyle>
#include <QSignalBlocker>
#include <QHeaderView>
#include <QProgressBar>
#include <QTextBrowser>
#include <algorithm>

MedicalWidget::MedicalWidget(bool demo,QWidget *parent):QWidget(parent),m_demo(demo) {
    setObjectName("medicalPage");
    auto *root=new QVBoxLayout(this); root->setContentsMargins(24,22,24,16); root->setSpacing(14);
    auto *head=new QHBoxLayout; auto *titles=new QVBoxLayout; titles->setSpacing(3);
    titles->addWidget(Design::label("Dossiers médicaux","pageTitle"));
    titles->addWidget(Design::label("Animaux, consultations et suivi vaccinal","muted")); head->addLayout(titles); head->addStretch();
    m_export=Design::button("Exporter PDF","export"); m_export->setObjectName("medicalExport");
    m_export->setToolTip("Aperçu puis export des dossiers correspondant aux filtres.");
    head->addWidget(m_export); connect(m_export,&QPushButton::clicked,this,[this]{exportPdf(false);});
    auto *add=Design::button("Ajouter un dossier","plus","primary"); add->setObjectName("medicalAdd"); head->addWidget(add);
    connect(add,&QPushButton::clicked,this,&MedicalWidget::addRecord); root->addLayout(head);
    auto *chips=new QHBoxLayout; chips->setSpacing(10);
    m_total=Design::label(""); m_upToDate=Design::label(""); m_renew=Design::label("");
    int i=0; for(auto *label:{m_total,m_upToDate,m_renew}) {
        label->setObjectName("medicalChip"); label->setProperty("tone",QStringList{"blue","ok","warn"}[i++]); chips->addWidget(label);
    }
    chips->addStretch(); auto *stats=Design::button("Statistiques","grid"); stats->setObjectName("medicalStatistics"); chips->addWidget(stats);
    connect(stats,&QPushButton::clicked,this,&MedicalWidget::statistics); root->addLayout(chips);
    auto *filters=new QHBoxLayout; filters->setSpacing(10);
    m_search=new QLineEdit; m_search->setObjectName("medicalSearch"); m_search->setPlaceholderText("Rechercher un animal, un dossier, un propriétaire…");
    m_search->setClearButtonEnabled(true); m_search->addAction(Design::icon("search"),QLineEdit::LeadingPosition); filters->addWidget(m_search,1);
    m_species=new QComboBox; m_species->setObjectName("medicalSpecies"); m_species->addItems({"Toutes les espèces","Chien","Chat","Lapin","Oiseau"});
    m_vaccination=new QComboBox; m_vaccination->setObjectName("medicalVaccination"); m_vaccination->addItems({"Tous les statuts","À jour","À renouveler"});
    m_sort=new QComboBox; m_sort->setObjectName("medicalSort"); m_sort->addItems({"Date : plus récents","Date : plus anciens","Nom de l'animal","Propriétaire"});
    for(auto *combo:{m_species,m_vaccination,m_sort}) { filters->addWidget(combo); connect(combo,&QComboBox::currentIndexChanged,this,&MedicalWidget::filter); }
    connect(m_search,&QLineEdit::textChanged,this,&MedicalWidget::filter); root->addLayout(filters);
    auto *filterInfo=new QHBoxLayout; m_counter=Design::label("","muted"); filterInfo->addWidget(m_counter); filterInfo->addStretch();
    auto *reset=Design::button("Réinitialiser","refresh"); reset->setObjectName("medicalReset"); filterInfo->addWidget(reset); root->addLayout(filterInfo);
    connect(reset,&QPushButton::clicked,this,[this]{QSignalBlocker a(m_search),b(m_species),c(m_vaccination),d(m_sort); m_search->clear();m_species->setCurrentIndex(0);m_vaccination->setCurrentIndex(0);m_sort->setCurrentIndex(0);filter();});
    m_error=Design::label(""); m_error->setObjectName("medicalError"); m_error->setWordWrap(true); m_error->hide(); root->addWidget(m_error);
    auto *body=new QHBoxLayout; body->setSpacing(16); root->addLayout(body,1);
    m_scroll=new QScrollArea; m_scroll->setWidgetResizable(true); m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); m_scroll->setObjectName("medicalCardsScroll");
    m_cardSurface=new QWidget; m_scroll->setWidget(m_cardSurface); m_scroll->viewport()->installEventFilter(this); body->addWidget(m_scroll,1);
    m_empty=Design::label("Aucun dossier trouvé.\nAjoutez une consultation ou modifiez les filtres.","muted",m_cardSurface);
    m_empty->setAlignment(Qt::AlignCenter); m_empty->setWordWrap(true);
    auto *panel=Design::card("medicalDetailPanel"); panel->setFixedWidth(286); body->addWidget(panel);
    auto *panelLayout=new QVBoxLayout(panel); panelLayout->setContentsMargins(18,18,18,18); panelLayout->setSpacing(12);
    panelLayout->addWidget(Design::label("DOSSIER SÉLECTIONNÉ","eyebrow"));
    m_placeholder=Design::label("Sélectionnez un animal\npour voir son dossier.","muted"); m_placeholder->setAlignment(Qt::AlignCenter); m_placeholder->setWordWrap(true); panelLayout->addWidget(m_placeholder,1);
    m_detailContent=new QWidget; auto *detailLayout=new QVBoxLayout(m_detailContent); detailLayout->setContentsMargins(0,0,0,0); detailLayout->setSpacing(10);
    auto *detailScroll=new QScrollArea; detailScroll->setWidgetResizable(true); detailScroll->setFrameShape(QFrame::NoFrame); detailScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *detailText=new QWidget; auto *textLayout=new QVBoxLayout(detailText); textLayout->setContentsMargins(0,0,7,0); textLayout->setSpacing(12);
    auto colors=detailText->palette(); colors.setColor(QPalette::Window,Qt::white);
    detailText->setPalette(colors); detailText->setAutoFillBackground(true);
    m_title=Design::label("","section"); m_title->setWordWrap(true); m_title->setTextFormat(Qt::PlainText); textLayout->addWidget(m_title);
    m_badge=Design::label(""); m_badge->setObjectName("medicalBadge"); textLayout->addWidget(m_badge);
    m_details=Design::label(""); m_details->setWordWrap(true); m_details->setTextFormat(Qt::PlainText); m_details->setTextInteractionFlags(Qt::TextSelectableByMouse); textLayout->addWidget(m_details); textLayout->addStretch();
    detailScroll->setWidget(detailText); detailLayout->addWidget(detailScroll,1);
    auto *documents=new QHBoxLayout;documents->setSpacing(8);detailLayout->addLayout(documents);
    auto *view=Design::button("Consulter","eye"); view->setObjectName("medicalView"); documents->addWidget(view,1); connect(view,&QPushButton::clicked,this,&MedicalWidget::viewRecord);
    auto *edit=Design::button("Modifier","edit","primary"); edit->setObjectName("medicalEdit"); detailLayout->addWidget(edit); connect(edit,&QPushButton::clicked,this,[this]{editRecord(false);});
    m_toggle=Design::button("","check","success"); m_toggle->setObjectName("medicalToggle"); detailLayout->addWidget(m_toggle); connect(m_toggle,&QPushButton::clicked,this,&MedicalWidget::toggleVaccination);
    auto *pdf=Design::button("PDF","export"); pdf->setToolTip("Aperçu et export PDF du dossier sélectionné");pdf->setObjectName("medicalRecordPdf"); documents->addWidget(pdf,1); connect(pdf,&QPushButton::clicked,this,[this]{exportPdf(true);});
    auto *remove=Design::button("Supprimer","trash","danger"); remove->setObjectName("medicalDelete"); detailLayout->addWidget(remove); connect(remove,&QPushButton::clicked,this,&MedicalWidget::removeRecord);
    panelLayout->addWidget(m_detailContent,1);
    root->addWidget(Design::label(demo?"Mode démo · données fictives, non conservées à la fermeture":"ANIWORLD · Dossiers enregistrés localement","muted"));
    refresh();
}
void MedicalWidget::paintEvent(QPaintEvent *event) { QWidget::paintEvent(event); QPainter painter(this); Design::paintWorkspace(painter,rect()); }
void MedicalWidget::focusSearch() { m_search->setFocus(); m_search->selectAll(); }
void MedicalWidget::refresh() {
    QString error; auto records=MedicalStore::all(&error);
    m_error->setText(error.isEmpty()?"":"Impossible de charger les dossiers : "+error); m_error->setVisible(!error.isEmpty());
    if(!error.isEmpty()) return;
    m_records=records; int up=0;for(const auto &r:m_records) if(r.medical.vaccinationStatus=="À jour") ++up;
    m_total->setText(QString("%1 dossiers").arg(m_records.size())); m_upToDate->setText(QString("%1 vaccins à jour").arg(up)); m_renew->setText(QString("%1 à renouveler").arg(m_records.size()-up)); filter();
}
void MedicalWidget::filter() {
    m_filtered.clear(); const auto query=m_search->text().trimmed();
    for(const auto &r:m_records) {
        if(m_species->currentIndex() && r.animal.species!=m_species->currentText()) continue;
        if(m_vaccination->currentIndex() && r.medical.vaccinationStatus!=m_vaccination->currentText()) continue;
        const auto text=QStringList{r.medical.recordId,r.animal.animalId,r.animal.name,r.animal.species,r.animal.breed,r.animal.owner,r.medical.reason,r.medical.diagnosis}.join(" ");
        if(!text.contains(query,Qt::CaseInsensitive)) continue;
        m_filtered.append(r);
    }
    const int sort=m_sort->currentIndex();
    std::stable_sort(m_filtered.begin(),m_filtered.end(),[sort](const MedicalRecord &a,const MedicalRecord &b){
        if(sort==2) return a.animal.name.localeAwareCompare(b.animal.name)<0;
        if(sort==3) return a.animal.owner.localeAwareCompare(b.animal.owner)<0;
        return sort==1?a.medical.consultationDate<b.medical.consultationDate:a.medical.consultationDate>b.medical.consultationDate;
    });
    bool visible=false; for(const auto &r:m_filtered) if(r.medical.recordId==m_selectedId) visible=true;
    if(!visible) m_selectedId.clear();
    for(auto *card:m_cards) {card->hide();card->deleteLater();} m_cards.clear();
    for(const auto &r:m_filtered) {
        auto *card=new RecordCard(r,m_cardSurface); card->setSelected(r.medical.recordId==m_selectedId);
        connect(card,&RecordCard::clicked,this,&MedicalWidget::select);
        connect(card,&RecordCard::doubleClicked,this,[this](const QString &id){select(id);viewRecord();});
        m_cards.append(card);card->show();
    }
    m_empty->setVisible(m_cards.isEmpty()); m_export->setEnabled(!m_filtered.isEmpty());
    m_counter->setText(QString("%1 / %2 dossiers").arg(m_filtered.size()).arg(m_records.size()));
    arrangeCards(); updateDetails();
}
bool MedicalWidget::eventFilter(QObject *watched,QEvent *event) {
    if(watched==m_scroll->viewport() && event->type()==QEvent::Resize) arrangeCards();
    return QWidget::eventFilter(watched,event);
}
void MedicalWidget::arrangeCards() {
    const int width=qMax(1,m_scroll->viewport()->width()-8),gap=12,height=180;
    const int columns=width>=540?2:1, cardWidth=(width-gap*(columns-1))/columns;
    for(int i=0;i<m_cards.size();++i) m_cards[i]->setGeometry((i%columns)*(cardWidth+gap),(i/columns)*(height+gap),cardWidth,height);
    m_cardSurface->setMinimumHeight(m_cards.isEmpty()?190:((m_cards.size()+columns-1)/columns)*(height+gap));
    m_empty->setGeometry(12,30,qMax(1,width-24),150);
}
const MedicalRecord *MedicalWidget::selected() const { for(const auto &r:m_records) if(r.medical.recordId==m_selectedId) return &r; return nullptr; }
void MedicalWidget::select(const QString &id) { m_selectedId=id; for(auto *card:m_cards) card->setSelected(card->recordId()==id); updateDetails(); }
void MedicalWidget::updateDetails() {
    const auto *r=selected(); m_placeholder->setVisible(!r);m_detailContent->setVisible(r); if(!r) return;
    m_title->setText(r->animal.name); const bool up=r->medical.vaccinationStatus=="À jour";
    m_badge->setText("Vaccination · "+r->medical.vaccinationStatus);m_badge->setProperty("tone",up?"ok":"warn");m_badge->style()->unpolish(m_badge);m_badge->style()->polish(m_badge);
    m_details->setText(r->medical.recordId+"  ·  "+r->animal.animalId+"\n\n"+r->animal.species+" · "+r->animal.breed+"\n"+QString::number(r->animal.age)+" ans · "+r->animal.sex+"\n\nPropriétaire\n"+r->animal.owner+"\n\nConsultation\n"+r->medical.consultationDate.toString("dd/MM/yyyy")+"\n\nMotif\n"+r->medical.reason+"\n\nPoids\n"+QString::number(r->medical.weight,'f',2)+" kg");
    m_toggle->setText(up?"Vaccin à renouveler":"Marquer à jour");
}
void MedicalWidget::addRecord() { editRecord(true); }
void MedicalWidget::editRecord(bool create) {
    MedicalRecord original; if(!create) {const auto *r=selected();if(!r)return; original=*r;}
    MedicalRecordDialog dialog(this); if(!create) dialog.setMedicalRecord(original);
    dialog.setWindowTitle(create?"Nouveau dossier médical":"Modifier le dossier médical");
    // Keep the form open and its fields intact if persistence fails.
    disconnect(dialog.findChild<QPushButton *>("primaryButton"),nullptr,&dialog,nullptr);
    connect(dialog.findChild<QPushButton *>("primaryButton"),&QPushButton::clicked,&dialog,[&]{
        auto record=dialog.getMedicalRecord(); record.storageId=original.storageId; QString error;
        if(!MedicalStore::save(record,&error)) {QMessageBox::warning(&dialog,"Enregistrement impossible",error);return;}
        m_selectedId=record.medical.recordId; dialog.accept();
    });
    if(dialog.exec()==QDialog::Accepted) refresh();
}
void MedicalWidget::viewRecord() { const auto *r=selected();if(!r)return; MedicalRecordDetailsDialog dialog(this);dialog.setMedicalRecord(*r);dialog.exec(); }
void MedicalWidget::removeRecord() {
    const auto *r=selected(); if(!r)return; const auto id=r->storageId;
    if(QMessageBox::question(this,"Supprimer le dossier","Supprimer définitivement "+r->medical.recordId+" ?",QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes)return;
    QString error;if(!MedicalStore::remove(id,&error)) {QMessageBox::warning(this,"Suppression impossible",error);return;}refresh();
}
void MedicalWidget::toggleVaccination() {
    const auto *r=selected();if(!r)return;auto changed=*r;changed.medical.vaccinationStatus=r->medical.vaccinationStatus=="À jour"?"À renouveler":"À jour";
    QString error;if(!MedicalStore::save(changed,&error)){QMessageBox::warning(this,"Enregistrement impossible",error);return;}refresh();
}
void MedicalWidget::statistics() {
    QDialog dialog(this);dialog.setObjectName("medicalStatisticsDialog");dialog.setWindowTitle("Statistiques des dossiers médicaux");dialog.resize(800,560);dialog.setMinimumSize(700,500);
    auto *root=new QVBoxLayout(&dialog);root->setContentsMargins(24,24,24,24);root->setSpacing(18);
    root->addWidget(Design::label("Statistiques des dossiers médicaux","section"));root->addWidget(Design::label("Répartition de tous les dossiers et statuts vaccinaux enregistrés","muted"));
    auto *metrics=new QHBoxLayout;for(auto *source:{m_total,m_upToDate,m_renew}){auto *label=Design::label(source->text());label->setObjectName("medicalChip");label->setProperty("tone",source->property("tone"));metrics->addWidget(label);}root->addLayout(metrics);
    auto *table=new QTableWidget;table->setObjectName("medicalStatsTable");Design::setupTable(table,{"Espèce","Dossiers","À jour","À renouveler","Taux à jour"});table->setAlternatingRowColors(false);root->addWidget(table,1);
    QMap<QString,QPair<int,int>> counts;for(const auto &r:m_records){auto &count=counts[r.animal.species];++count.first;if(r.medical.vaccinationStatus=="À jour")++count.second;}
    for(auto it=counts.cbegin();it!=counts.cend();++it){const int row=table->rowCount();table->insertRow(row);const auto count=it.value();QStringList values={it.key(),QString::number(count.first),QString::number(count.second),QString::number(count.first-count.second)};
        for(int col=0;col<values.size();++col)table->setItem(row,col,new QTableWidgetItem(values[col]));
        auto *bar=new QProgressBar;bar->setObjectName("medicalRate");bar->setRange(0,100);bar->setTextVisible(true);Motion::progressTo(bar,count.first?count.second*100/count.first:0);table->setCellWidget(row,4,bar);table->setRowHeight(row,44);
    }
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    root->addWidget(Design::label(m_records.isEmpty()?"Aucun dossier enregistré.":"Statuts saisis dans les dossiers de consultation.","muted"));
    auto *buttons=new QHBoxLayout;buttons->addStretch();auto *close=Design::button("Fermer",{},"primary");buttons->addWidget(close);root->addLayout(buttons);connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);dialog.exec();
}
void MedicalWidget::exportPdf(bool selectedOnly) {
    QVector<MedicalRecord> records=m_filtered;if(selectedOnly){const auto *r=selected();if(!r)return;records={*r};}if(records.isEmpty())return;
    const QString html=MedicalStore::reportHtml(records,m_demo);
    QDialog dialog(this);dialog.setObjectName("medicalPdfPreview");dialog.setWindowTitle("Aperçu du PDF médical");dialog.resize(780,640);
    auto *layout=new QVBoxLayout(&dialog);auto *preview=new QTextBrowser;preview->setOpenExternalLinks(false);preview->setHtml(html);layout->addWidget(preview);
    auto *buttons=new QHBoxLayout;buttons->addStretch();auto *close=Design::button("Fermer");auto *save=Design::button("Enregistrer le PDF","export","primary");buttons->addWidget(close);buttons->addWidget(save);layout->addLayout(buttons);
    connect(close,&QPushButton::clicked,&dialog,&QDialog::reject);
    connect(save,&QPushButton::clicked,&dialog,[&]{QString path=QFileDialog::getSaveFileName(&dialog,"Enregistrer le PDF",selectedOnly?records.first().medical.recordId+".pdf":"ANIWORLD-dossiers-medicaux.pdf","Document PDF (*.pdf)");if(path.isEmpty())return;if(!path.endsWith(".pdf",Qt::CaseInsensitive))path+=".pdf";QString error;
        if(!Invoice::writeDocument(path,html,"Dossiers médicaux ANIWORLD",&error)){QMessageBox::warning(&dialog,"Export impossible",error);return;}dialog.accept();});dialog.exec();
}
