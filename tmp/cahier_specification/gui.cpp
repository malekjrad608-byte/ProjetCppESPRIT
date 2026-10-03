#include <QtWidgets>

static QString out;
static QString logo = "C:/Users/malek/Downloads/logo-removebg-preview.png";
QLabel* label(const QString &s, const QString &role="") {
    auto *l=new QLabel(s); l->setProperty("role",role); l->setWordWrap(true); return l;
}
QPushButton* button(const QString &s,bool primary=false) {
    auto *b=new QPushButton(s); if(primary)b->setProperty("role","primary");return b;
}
QFrame* card(QVBoxLayout *&v) {
    auto *f=new QFrame;f->setObjectName("card");v=new QVBoxLayout(f);v->setContentsMargins(16,12,16,12);v->setSpacing(8);return f;
}
QWidget* pills(const QStringList &values) {
    auto *w=new QWidget;auto *h=new QHBoxLayout(w);h->setContentsMargins(0,0,0,0);h->setSpacing(12);
    for(const auto &x:values){QVBoxLayout *v;auto *f=card(v);auto p=x.split('|');v->addWidget(label(p[0],"muted"));v->addWidget(label(p[1],"metric"));h->addWidget(f);}return w;
}
QTableWidget* table(const QStringList &headers,const QList<QStringList> &rows) {
    auto *t=new QTableWidget(rows.size(),headers.size());t->setHorizontalHeaderLabels(headers);t->verticalHeader()->hide();t->setShowGrid(false);t->setAlternatingRowColors(true);t->setSelectionBehavior(QAbstractItemView::SelectRows);t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);t->horizontalHeader()->setFixedHeight(42);t->verticalHeader()->setDefaultSectionSize(44);t->setFocusPolicy(Qt::NoFocus);
    for(int r=0;r<rows.size();r++)for(int c=0;c<headers.size();c++){auto *it=new QTableWidgetItem(rows[r].value(c));t->setItem(r,c,it);if(c==headers.size()-1)it->setForeground(QColor("#246756"));}
    t->setMinimumHeight(42+44*rows.size()+4);return t;
}
void render(int n,const QString &title,const QString &subtitle,const QStringList &stats,const QStringList &headers,const QList<QStringList> &rows,const QString &featureTitle,const QString &featureText,const QString &cta) {
    QWidget window;window.resize(1120,730);window.setObjectName("root");
    auto *root=new QHBoxLayout(&window);root->setContentsMargins(0,0,0,0);root->setSpacing(0);
    auto *side=new QFrame;side->setObjectName("sidebar");side->setFixedWidth(220);auto *sv=new QVBoxLayout(side);sv->setContentsMargins(20,24,20,22);sv->setSpacing(10);
    auto *brand=new QHBoxLayout;auto *img=new QLabel;img->setPixmap(QPixmap(logo).scaled(48,48,Qt::KeepAspectRatio,Qt::SmoothTransformation));brand->addWidget(img);brand->addWidget(label("PATTIA","brand"));sv->addLayout(brand);sv->addWidget(label("Centre de soins animaliers","small"));sv->addSpacing(22);sv->addWidget(label("ESPACE DE GESTION","eyebrow"));
    QStringList nav={"Tableau de bord","Services","Employés","Stock","Locaux","Dossiers médicaux"};
    for(int i=0;i<nav.size();i++){auto *b=button(nav[i]);b->setProperty("role",i==n?"active":"nav");sv->addWidget(b);}sv->addStretch();sv->addWidget(label("Maquette Qt","small"));sv->addWidget(label("Données de démonstration","small"));
    root->addWidget(side);auto *main=new QWidget;auto *v=new QVBoxLayout(main);v->setContentsMargins(26,20,26,20);v->setSpacing(14);root->addWidget(main);
    auto *top=new QHBoxLayout;top->addWidget(label("Espace de gestion  /  "+nav[n],"small"));top->addStretch();top->addWidget(label("26 septembre 2026","small"));v->addLayout(top);
    auto *headline=new QHBoxLayout;auto *titles=new QVBoxLayout;titles->addWidget(label(title,"title"));titles->addWidget(label(subtitle,"muted"));headline->addLayout(titles,1);headline->addWidget(button(cta,true));v->addLayout(headline);
    v->addWidget(pills(stats));
    if(n>0){auto *filters=new QHBoxLayout;auto *search=new QLineEdit;search->setPlaceholderText(n==5?"Rechercher un animal ou un numéro de dossier":"Rechercher dans la liste");filters->addWidget(search,1);auto *combo=new QComboBox;combo->addItem(n==3?"Tous les produits":"Tous les statuts");filters->addWidget(combo);filters->addWidget(button("Exporter"));v->addLayout(filters);}
    QVBoxLayout *tv;auto *tf=card(tv);tv->addWidget(label(n==0?"Les services du jour":"Liste et suivi","section"));tv->addWidget(table(headers,rows));v->addWidget(tf,1);
    QVBoxLayout *fv;auto *ff=card(fv);ff->setObjectName("feature");fv->addWidget(label(featureTitle,"section"));fv->addWidget(label(featureText,"featureBody"));v->addWidget(ff);
    if(n>0){auto *actions=new QHBoxLayout;actions->addWidget(button("Consulter"));actions->addWidget(button("Modifier"));actions->addWidget(button("Archiver"));actions->addStretch();actions->addWidget(label("Sélectionnez une ligne pour agir","small"));v->addLayout(actions);}
    window.show();QApplication::processEvents();QThread::msleep(30);QApplication::processEvents();window.grab().save(out+QString("/gui_%1.png").arg(n));window.close();
}
int main(int argc,char **argv) {
    QApplication a(argc,argv);out=argc>1?QString::fromLocal8Bit(argv[1]):".";QDir().mkpath(out);
    for(const auto &font:{"segoeui.ttf","seguisb.ttf","segoeuib.ttf"}) QFontDatabase::addApplicationFont("C:/Windows/Fonts/"+QString(font));
    a.setFont(QFont("Segoe UI",10));
    a.setStyleSheet(R"(
      QWidget {font-family:'Segoe UI';font-size:16px;color:#34453E;}
      QWidget#root {background:#F5F6F2;} QLabel {background:transparent;border:none;}
      QFrame#sidebar {background:white;border-right:1px solid #DFE6DF;}
      QFrame#card {background:white;border:1px solid #DFE6DF;border-radius:12px;}
      QFrame#feature {background:#E6F0E7;border:1px solid #D0E2D4;border-radius:12px;}
      QLabel[role='title'] {font-size:28px;font-weight:700;color:#254A3D;}
      QLabel[role='brand'] {font-size:25px;font-weight:750;letter-spacing:1px;}
      QLabel[role='section'] {font-size:18px;font-weight:650;color:#285B48;}
      QLabel[role='metric'] {font-size:29px;font-weight:700;color:#246756;}
      QLabel[role='muted'] {font-size:14px;color:#617269;}
      QLabel[role='small'] {font-size:12px;color:#67786F;}
      QLabel[role='eyebrow'] {font-size:12px;color:#617269;letter-spacing:1px;font-weight:650;}
      QLabel[role='featureBody'] {font-size:15px;color:#335D49;}
      QPushButton {background:white;border:1px solid #CAD8CE;border-radius:7px;padding:9px 12px;font-size:14px;font-weight:600;}
      QPushButton[role='primary'] {background:#246756;color:white;border:1px solid #246756;}
      QPushButton[role='nav'] {text-align:left;background:transparent;border:none;padding:13px 9px;font-size:16px;font-weight:400;}
      QPushButton[role='active'] {text-align:left;background:#E3EFE6;border:none;padding:13px 9px;font-size:16px;color:#246756;font-weight:650;}
      QLineEdit,QComboBox {background:white;border:1px solid #D4DFD6;border-radius:7px;padding:10px;font-size:14px;}
      QTableWidget {background:white;alternate-background-color:#F7FAF7;border:none;font-size:15px;selection-background-color:#E3EFE6;}
      QTableWidget::item {padding:7px;border-bottom:1px solid #E7EDE7;}
      QHeaderView::section {background:#F0F5F0;padding:10px 6px;border:none;font-size:13px;font-weight:600;color:#547361;}
    )");
    render(0,"Tableau de bord","Les soins et les ressources du centre en un coup d’œil.",{"Services du jour|4","Employés présents|6","Alertes stock|2","Locaux disponibles|3"},{"Service","Animal","Horaire","Statut"},{{"Toilettage complet","Milo","09:00","Terminé"},{"Consultation","Luna","10:00","En cours"},{"Séance d’éducation","Nala","14:00","Planifié"},{"Bain et brossage","Simba","15:00","Planifié"}},"À traiter aujourd’hui","2 lots à contrôler et 1 rappel vaccinal à préparer. Les alertes ouvrent le module concerné.","+ Nouveau service");
    render(1,"Gestion des services","Planifier les prestations et suivre leur réalisation.",{"Prestations|7","Planifiées|3","En cours|1","Terminées|2"},{"Prestation","Responsable","Horaire","Statut"},{{"Toilettage complet","Sarra B.","09:00","Terminé"},{"Consultation","Dr Amine K.","10:00","En cours"},{"Séance d’éducation","Youssef M.","14:00","Planifié"}},"Proposition de créneau","26/09 à 16:00 · Salle de soins 2 · Sarra B. disponible · Durée de 45 min. La réservation attend votre validation.","+ Ajouter");
    render(2,"Gestion des employés","Compétences, disponibilités et répartition du travail.",{"Employés actifs|8","Présents|6","Absents|2","Charge moyenne|68 %"},{"Nom","Poste","Compétence","Charge"},{{"Sarra B.","Soigneur","Toilettage","75 %"},{"Amine K.","Vétérinaire","Consultation","60 %"},{"Youssef M.","Éducateur","Éducation","70 %"}},"Équilibrage de la charge","Une prestation de 30 min peut être réaffectée à un salarié habilité moins chargé. Le responsable confirme chaque changement.","+ Ajouter");
    render(3,"Gestion du stock","Suivre les quantités, les lots et les besoins du centre.",{"Articles actifs|24","Sous le seuil|2","Lots à contrôler|2","Mouvements du jour|8"},{"Article","Lot","Quantité","État"},{{"Gants nitrile","GN-2608","120 unités","Disponible"},{"Shampooing doux","SH-2609","4 flacons","Sous le seuil"},{"Compresses","CP-2606","30 sachets","Échéance proche"}},"Sortie conseillée par lot","Utiliser d’abord CP-2606, échéance au 31/10/2026. Un lot expiré ou bloqué ne peut pas être proposé à la sortie.","+ Ajouter");
    render(4,"Gestion des locaux","Disponibilité, hygiène et maintenance des espaces.",{"Locaux suivis|6","Disponibles|3","Occupés|2","À nettoyer|1"},{"Local","Type","Capacité","État"},{{"Salle de soins 1","Consultation","1 animal","Occupé"},{"Salle de soins 2","Polyvalente","1 animal","Disponible"},{"Espace toilettage","Toilettage","2 animaux","À nettoyer"}},"Remise en service","L’espace toilettage reste indisponible jusqu’à la validation de sa fiche de nettoyage par un employé habilité.","+ Ajouter");
    render(5,"Dossiers médicaux","Retrouver les antécédents et organiser le suivi animal.",{"Dossiers actifs|42","Consultations du jour|4","Rappels à préparer|3","Suivis à vérifier|1"},{"Dossier","Animal","Dernière visite","Suivi"},{{"DM-0042","Milo · Chien","26/09/2026","À jour"},{"DM-0037","Luna · Chat","26/09/2026","Contrôle prévu"},{"DM-0028","Nala · Chien","20/09/2026","Rappel vaccinal"}},"Suivi du dossier DM-0028","Une échéance vaccinale est enregistrée au 03/10/2026. Préparer un rappel sans envoi automatique au propriétaire.","+ Ouvrir un dossier");
    return 0;
}
