#include <QtWidgets>
#include "../../src/ui/design.h"

using namespace Design;
static QString output;

QPixmap wordmark() {
    QImage original("C:/Users/malek/Downloads/new name.png");
    int l=original.width(),t=original.height(),r=-1,b=-1;
    for(int y=0;y<original.height();++y) for(int x=0;x<original.width();++x)
        if(qAlpha(original.pixel(x,y))>20) { l=qMin(l,x);t=qMin(t,y);r=qMax(r,x);b=qMax(b,y); }
    if(r>=l) original=original.copy(QRect(QPoint(l,t),QPoint(r,b)));
    return QPixmap::fromImage(original).scaled(154,65,Qt::KeepAspectRatio,Qt::SmoothTransformation);
}

class Mascot: public QWidget {
public: Mascot(){setFixedSize(130,96);}
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.setRenderHint(QPainter::SmoothPixmapTransform);p.setPen(Qt::NoPen);p.scale(0.82,0.82);
        p.setBrush(QColor("#D0E3D6"));p.drawEllipse(QRectF(25,0,119,119));
        p.drawPixmap(37,9,Design::brand("logo",QSize(103,103)));
        p.setBrush(QColor("#F9E5BF"));p.drawEllipse(QRectF(9,84,26,26));
        Design::icon("heart",QColor("#B29455")).paint(&p,QRect(14,89,16,16));
        Design::icon("paw",QColor("#91AF9C")).paint(&p,QRect(139,15,10,10));
        p.setBrush(QColor("#ABC9B2"));p.drawEllipse(QRectF(15,28,4,4));
    }
};

QLabel *small(QString text){return label(text,"muted");}

QWidget* status(QString text){
    QString bg="#E8F3EA",fg="#468467";
    if(text.contains("cours")||text.contains("seuil")||text.contains("nettoyer")||text.contains("proche")||text.contains("vaccinal")){bg="#FFF1D8";fg="#AC7A2D";}
    else if(text=="Planifié"||text=="Occupé"||text=="Contrôle prévu"){bg="#EDF0FB";fg="#697DC3";}
    auto *w=new QWidget;auto *h=new QHBoxLayout(w);h->setContentsMargins(9,0,4,0);
    auto *s=label("●  "+text);s->setStyleSheet("font-size:11px;font-weight:600;padding:5px 8px;border-radius:9px;color:"+fg+";background:"+bg+";");
    h->addWidget(s,0,Qt::AlignVCenter);h->addStretch();return w;
}

QTableWidget* rowsTable(QStringList headers,QList<QStringList> rows,bool isStatus=true){
    auto *t=new QTableWidget;setupTable(t,headers);t->setRowCount(rows.size());t->setWordWrap(false);t->setFocusPolicy(Qt::NoFocus);
    t->setStyleSheet("QTableWidget{font-size:12px;} QTableWidget::item{padding:7px 10px;border-bottom:1px solid #EDF0EB;} QHeaderView::section{font-size:10px;padding:11px 10px;}");
    for(int r=0;r<rows.size();++r){t->setRowHeight(r,40);for(int c=0;c<headers.size();++c){t->setItem(r,c,new QTableWidgetItem(isStatus&&c==headers.size()-1?QString():rows[r].value(c)));if(isStatus&&c==headers.size()-1)t->setCellWidget(r,c,status(rows[r].value(c)));}}
    if(headers.size()==3){t->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed);t->setColumnWidth(1,78);t->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Fixed);t->setColumnWidth(2,118);}
    if(headers.size()==4){t->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);t->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Fixed);t->setColumnWidth(1,105);t->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Fixed);t->setColumnWidth(2,105);t->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Fixed);t->setColumnWidth(3,140);}
    if(headers.value(0)!="SERVICE"){t->setColumnWidth(1,112);t->setColumnWidth(2,153);}
    t->setMinimumHeight(40*rows.size()+39);return t;
}

QWidget* metrics(QStringList data){
    auto *w=new QWidget;auto *hl=new QHBoxLayout(w);hl->setContentsMargins(0,0,0,0);hl->setSpacing(12);
    const QStringList icons={"services","calendar","clock","check"};
    const QStringList fills={"#E9F0E7","#EDF1F5","#FFF2DA","#E7F3EC"};
    for(int i=0;i<data.size();++i){auto parts=data[i].split('|');auto *c=card();auto *v=new QVBoxLayout(c);v->setContentsMargins(13,9,13,9);v->setSpacing(3);
        auto *top=new QHBoxLayout;top->addWidget(small(parts.value(0)),1);auto *ic=label("");ic->setPixmap(icon(icons[i]).pixmap(16,16));ic->setAlignment(Qt::AlignCenter);ic->setFixedSize(25,25);ic->setStyleSheet("background:"+fills[i]+";border-radius:7px;");top->addWidget(ic);v->addLayout(top);
        auto *value=label(parts.value(1));value->setStyleSheet("font-size:25px;font-weight:650;color:#315B4D;");v->addWidget(value);v->addWidget(small(parts.value(2)));hl->addWidget(c,1);}
    return w;
}

QWidget* distribution(QString title,QString subtitle,QStringList data){
    auto *c=card();auto *v=new QVBoxLayout(c);v->setContentsMargins(15,14,15,14);v->setSpacing(8);
    v->addWidget(label(title,"section"));v->addWidget(small(subtitle));
    for(auto text:data){auto parts=text.split('|');auto *h=new QHBoxLayout;auto *name=label(parts.value(0));name->setStyleSheet("font-size:11px;");h->addWidget(name,1);h->addWidget(small(parts.value(1)));v->addLayout(h);auto *bar=new QProgressBar;bar->setRange(0,100);bar->setValue(parts.value(2).toInt());bar->setTextVisible(false);v->addWidget(bar);}
    v->addStretch();return c;
}

void render(int n,QString title,QString intro,QString heroTitle,QString heroSub,QStringList stats,QStringList headers,QList<QStringList> rows,QString featureTitle,QString featureBody,QString graphTitle,QString graphSub,QStringList graph,QString cta){
    QWidget window;window.setObjectName("specRoot");window.resize(1120,740);
    auto *root=new QHBoxLayout(&window);root->setContentsMargins(0,0,0,0);root->setSpacing(0);
    auto *side=new QFrame;side->setObjectName("sidebar");side->setFixedWidth(182);auto *sv=new QVBoxLayout(side);sv->setContentsMargins(12,17,12,17);sv->setSpacing(9);
    auto *brandLabel=label("");brandLabel->setPixmap(wordmark());brandLabel->setAlignment(Qt::AlignCenter);sv->addWidget(brandLabel);
    auto *workspace=card("workspace");auto *wh=new QHBoxLayout(workspace);wh->setContentsMargins(10,10,10,10);auto *paw=label("");paw->setPixmap(icon("paw",QColor("#7D9D8A")).pixmap(20,20));wh->addWidget(paw);auto *wv=new QVBoxLayout;wv->setSpacing(3);auto *name=label("Centre de soins");name->setStyleSheet("font-size:12px;font-weight:600;");wv->addWidget(name);wv->addWidget(small("Espace de gestion"));wh->addLayout(wv);sv->addWidget(workspace);
    sv->addSpacing(7);sv->addWidget(label("ESPACE PRINCIPAL","eyebrow"));
    const QStringList nav={"Tableau de bord","Services","Employés","Stock","Locaux","Dossiers médicaux"},ico={"grid","services","users","services","grid","shield"};
    for(int i=0;i<nav.size();++i){if(i==2){sv->addSpacing(9);sv->addWidget(label("MODULES DE GESTION","eyebrow"));}auto *b=button(nav[i],ico[i],"nav");b->setCheckable(true);b->setChecked(i==n);sv->addWidget(b);}
    sv->addStretch();auto *motto=label("Leur bien-être,\nnotre plus belle mission.");motto->setStyleSheet("font-size:12px;font-weight:600;color:#7C9286;");sv->addWidget(motto);sv->addWidget(label("ANIWORLD  /  MAQUETTE","eyebrow"));root->addWidget(side);
    auto *col=new QVBoxLayout;col->setContentsMargins(0,0,0,0);col->setSpacing(0);root->addLayout(col,1);
    auto *topbar=new QFrame;topbar->setObjectName("topbar");topbar->setFixedHeight(48);auto *th=new QHBoxLayout(topbar);th->setContentsMargins(24,0,22,0);th->addWidget(small("Espace de gestion  /  "+nav[n]));th->addStretch();th->addWidget(small("dimanche 27 septembre 2026"));th->addSpacing(12);auto *demo=label("●  Maquette · données fictives");demo->setStyleSheet("font-size:10px;color:#7B9B7E;background:#EDF5EA;padding:5px 10px;border-radius:12px;");th->addWidget(demo,0,Qt::AlignVCenter);col->addWidget(topbar);
    auto *body=new QWidget;auto *v=new QVBoxLayout(body);v->setContentsMargins(22,19,22,12);v->setSpacing(12);col->addWidget(body,1);
    auto *head=new QHBoxLayout;auto *hv=new QVBoxLayout;hv->setSpacing(4);auto *tit=label(title,"title");tit->setStyleSheet("font-size:25px;font-weight:700;color:#203D3B;");hv->addWidget(tit);hv->addWidget(small(intro));head->addLayout(hv,1);head->addWidget(button(cta,"plus","primary"));v->addLayout(head);
    auto *hero=card("hero");auto *hl=new QHBoxLayout(hero);hl->setContentsMargins(20,8,13,8);auto *text=new QVBoxLayout;text->setSpacing(5);text->addWidget(label(n==0?"CHAQUE ATTENTION COMPTE":"AU SERVICE DE LEUR BIEN-ÊTRE","eyebrow"));auto *heroT=label(heroTitle);heroT->setStyleSheet("font-size:21px;font-weight:650;color:#356351;");text->addWidget(heroT);text->addWidget(small(heroSub));hl->addLayout(text,1);hl->addWidget(new Mascot);v->addWidget(hero);
    v->addWidget(metrics(stats));
    if(n>0){auto *filters=new QHBoxLayout;filters->setSpacing(10);auto *search=new QLineEdit;search->setPlaceholderText(n==5?"Rechercher un animal ou un dossier…":"Rechercher dans la liste…");filters->addWidget(search,1);auto *combo=new QComboBox;combo->addItem(n==3?"Tous les produits":"Tous les statuts");filters->addWidget(combo);filters->addWidget(button("Exporter","export"));v->addLayout(filters);}
    auto *lower=new QHBoxLayout;lower->setSpacing(14);auto *lc=card();auto *lv=new QVBoxLayout(lc);lv->setContentsMargins(0,0,0,8);lv->setSpacing(5);auto *lh=new QHBoxLayout;lh->setContentsMargins(16,12,14,4);auto *labelsv=new QVBoxLayout;labelsv->setSpacing(3);labelsv->addWidget(label(n==0?"Les services du jour":"Liste et suivi","section"));labelsv->addWidget(small(n==0?"4 prestations prévues aujourd’hui":"Sélectionnez une ligne pour consulter sa fiche"));lh->addLayout(labelsv);lh->addStretch();if(n==0)lh->addWidget(button("Tout voir  →",{},"link"));lv->addLayout(lh);
    lv->addWidget(rowsTable(headers,rows,n!=2),1);
    if(n>0){auto *actions=new QHBoxLayout;actions->setContentsMargins(12,4,12,2);for(auto act:{"Consulter","Modifier","Archiver"}){auto *b=button(act);b->setStyleSheet("font-size:11px;padding:6px 9px;");actions->addWidget(b);}actions->addStretch();lv->addLayout(actions);}
    lower->addWidget(lc,1);auto *right=new QWidget;right->setFixedWidth(n==0?274:252);auto *rv=new QVBoxLayout(right);rv->setContentsMargins(0,0,0,0);rv->setSpacing(11);rv->addWidget(distribution(graphTitle,graphSub,graph),1);
    auto *feature=card("careCard");auto *fv=new QVBoxLayout(feature);fv->setContentsMargins(14,11,14,11);fv->setSpacing(6);auto *fh=new QHBoxLayout;auto *heart=label("");heart->setPixmap(icon("heart",QColor("#AC853F")).pixmap(19,19));fh->addWidget(heart);auto *fTitle=label(featureTitle);fTitle->setWordWrap(true);fTitle->setStyleSheet("font-size:12px;font-weight:600;color:#9A783B;");fh->addWidget(fTitle,1);fv->addLayout(fh);auto *fb=label(featureBody);fb->setWordWrap(true);fb->setStyleSheet("font-size:11px;color:#967539;");fv->addWidget(fb);rv->addWidget(feature);lower->addWidget(right);v->addLayout(lower,1);
    auto *footer=new QHBoxLayout;footer->addWidget(small("Pensé pour eux. Conçu pour vous."));footer->addStretch();footer->addWidget(small("ANIWORLD  •  Smart Pet Care"));v->addLayout(footer);
    window.show();QApplication::processEvents();QThread::msleep(30);QApplication::processEvents();window.grab().save(output+QString("/gui_%1.png").arg(n));qInfo()<<"Saved"<<n<<window.size();window.close();
}

int main(int argc,char **argv){
    QApplication app(argc,argv);output=argc>1?QString::fromLocal8Bit(argv[1]):"gui_aniworld";QDir().mkpath(output);
    for(const auto &font:{"segoeui.ttf","seguisb.ttf","segoeuib.ttf"})QFontDatabase::addApplicationFont("C:/Windows/Fonts/"+QString(font));
    app.setFont(QFont("Segoe UI",10));applyTheme();
    app.setStyleSheet(app.styleSheet()+"QWidget#specRoot{background:#F5F6F2;} QLabel[role='muted']{font-size:10px;color:#7C8987;} QLabel[role='section']{font-size:14px;font-weight:600;} QLabel[role='eyebrow']{font-size:8px;letter-spacing:1.4px;} QPushButton[role='nav']{font-size:12px;padding:9px 9px;} QLineEdit,QComboBox{font-size:11px;padding:8px 10px;} QPushButton{font-size:11px;padding:9px 12px;}");
    render(0,"Tableau de bord","Une belle journée pour prendre soin d’eux.","De petits soins.\nDe grands moments de bonheur.","Tous vos services, réunis pour leur bien-être.",
      {"Total des services|7|Toutes les prestations","Services planifiés|3|À organiser","En cours|1|En prise en charge","Services terminés|2|Soins réalisés"},
      {"SERVICE","HORAIRE","STATUT"},{{"Toilettage complet","09:00","Terminé"},{"Consultation de bien-être","10:00","En cours"},{"Séance d’éducation","14:00","Planifié"},{"Bain & brossage","15:00","Planifié"}},
      "2 soins terminés","Autant d’attentions qui comptent.","Nos prestations","Répartition par catégorie",{"Soins & bien-être|2|29","Toilettage|2|29","Pension & garde|1|14","Éducation|2|29","Autre|0|0"},"Nouveau service");
    render(1,"Gestion des services","Planifier les prestations et suivre leur réalisation.","Chaque soin commence\npar une bonne organisation.","Prestations, disponibilités et suivi dans un même espace.",
      {"Total des services|7|Toutes les prestations","Services planifiés|3|À organiser","En cours|1|En prise en charge","Services terminés|2|Soins réalisés"},
      {"PRESTATION","HORAIRE","STATUT"},{{"Toilettage complet","09:00","Terminé"},{"Consultation","10:00","En cours"},{"Séance d’éducation","14:00","Planifié"}},
      "Créneau proposé","16:00 · Salle de soins 2\nSarra B. · Durée de 45 min\nÀ confirmer par le responsable.","Nos prestations","Répartition par catégorie",{"Soins & bien-être|2|29","Toilettage|2|29","Éducation|2|29"},"Nouveau service");
    render(2,"Gestion des employés","Compétences, disponibilités et répartition du travail.","Une équipe attentive.\nDes soins bien organisés.","Les bonnes compétences pour chaque prise en charge.",
      {"Employés actifs|8|Dans l’équipe","Présents|6|Disponibles aujourd’hui","Absents|2|Absences déclarées","Charge moyenne|68 %|Cette semaine"},
      {"EMPLOYÉ","POSTE","CHARGE"},{{"Sarra B.","Soigneur","75 %"},{"Amine K.","Vétérinaire","60 %"},{"Youssef M.","Éducateur","70 %"}},
      "Équilibrage de la charge","Une prestation de 30 min peut être réaffectée à un profil habilité moins chargé. Validation requise.","Charge prévue","Cette semaine",{"Sarra B.|75 %|75","Amine K.|60 %|60","Youssef M.|70 %|70"},"Nouvel employé");
    render(3,"Gestion du stock","Suivre les produits, les lots et les besoins du centre.","Le bon produit.\nAu bon moment.","Des ressources disponibles pour assurer chaque soin.",
      {"Articles actifs|24|Au catalogue","Sous le seuil|2|À réapprovisionner","Lots à contrôler|2|Échéances proches","Mouvements du jour|8|Entrées et sorties"},
      {"ARTICLE","QUANTITÉ","ÉTAT"},{{"Gants nitrile","120 unités","Disponible"},{"Shampooing doux","4 flacons","Sous le seuil"},{"Compresses","30 sachets","Échéance proche"}},
      "Sortie conseillée","Lot CP-2606 à utiliser en priorité. Échéance au 31/10/2026. Les lots expirés sont exclus.","Niveaux disponibles","Part de la quantité cible",{"Gants nitrile|80 %|80","Shampooing doux|20 %|20","Compresses|60 %|60"},"Nouvel article");
    render(4,"Gestion des locaux","Disponibilité, hygiène et maintenance des espaces.","Des espaces prêts.\nPour chaque prise en charge.","Organiser l’occupation et préparer les prochains soins.",
      {"Locaux suivis|6|Dans le centre","Disponibles|3|Prêts à accueillir","Occupés|2|En utilisation","À nettoyer|1|Remise en service"},
      {"LOCAL","CAPACITÉ","ÉTAT"},{{"Salle de soins 1","1 animal","Occupé"},{"Salle de soins 2","1 animal","Disponible"},{"Espace toilettage","2 animaux","À nettoyer"}},
      "Remise en service","L’espace toilettage reste indisponible jusqu’à la validation de sa fiche de nettoyage.","État des espaces","Répartition actuelle",{"Disponibles|3|50","Occupés|2|33","À nettoyer|1|17"},"Nouveau local");
    render(5,"Dossiers médicaux","Retrouver les antécédents et organiser le suivi animal.","Leur histoire de soins.\nToujours à portée de main.","Un suivi attentif, partagé avec les personnes autorisées.",
      {"Dossiers actifs|42|Animaux suivis","Consultations du jour|4|Prises en charge","Rappels à préparer|3|Échéances vaccinales","Suivis à vérifier|1|Attention requise"},
      {"DOSSIER ET ANIMAL","VISITE","SUIVI"},{{"DM-0042 · Milo","27/09/2026","À jour"},{"DM-0037 · Luna","27/09/2026","Contrôle prévu"},{"DM-0028 · Nala","20/09/2026","Rappel vaccinal"}},
      "Rappel à préparer","Dossier DM-0028 · Nala\nÉchéance au 03/10/2026.\nPréparer le rappel pour validation.","Suivi médical","Sur 42 dossiers actifs",{"À jour|38|90","Rappels à préparer|3|7","Suivis à vérifier|1|3"},"Nouveau dossier");
    return 0;
}
