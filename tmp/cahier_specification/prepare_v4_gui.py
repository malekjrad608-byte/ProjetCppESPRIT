from pathlib import Path
work=Path(__file__).parent
s=(work/'gui_aniworld_v2.cpp').read_text(encoding='utf-8')
s=s.replace('"Locaux","Dossiers médicaux"','"Locaux","Dossiers médicaux","Adoptions"')
s=s.replace('if(qAlpha(i.pixel(x,y))>0)', 'if(qAlpha(i.pixel(x,y))>0 && QColor(i.pixel(x,y)).red()<245 && QColor(i.pixel(x,y)).green()<245 && QColor(i.pixel(x,y)).blue()<245)')
s=s.replace('return brandAsset("new logo.png",size,size);','return brandAsset("Plan de travail 1.png",size,size);')
s=s.replace('auto*logo=new QLabel;logo->setPixmap(brandAsset("new name.png",170,70));logo->setAlignment(Qt::AlignCenter);sv->addWidget(logo);sv->addSpacing(18);', '''auto*brand=new QHBoxLayout;brand->setSpacing(7);auto*logo=new QLabel;logo->setPixmap(appSymbol(43));brand->addWidget(logo);auto*bv=new QVBoxLayout;bv->setSpacing(2);bv->addWidget(txt("ANIWORLD",18,"#263B42",true));bv->addWidget(txt("THE OFFICIAL APPLICATION",7,"#73868B"));brand->addLayout(bv);sv->addLayout(brand);sv->addSpacing(18);''')
s=s.replace('"grid","shield"};for(int i=0;i<6;i++)','"grid","shield","users"};for(int i=0;i<7;i++)')
s=s.replace('"27 septembre 2026"','"1 octobre 2026"').replace('dimanche 27 septembre','jeudi 1 octobre')
adoption=r'''
QWidget* adoptionCard(QString name,QString type,QString ident,QString state,QString hint,bool selected=false){
 QVBoxLayout*v;auto*f=panel(v);f->setMinimumHeight(157);if(selected)f->setStyleSheet("QFrame#panel{border:1.5px solid #E8A13F;background:#FFFAF2;border-radius:12px;}");
 auto*h=new QHBoxLayout;auto*animal=new QLabel;animal->setPixmap(Design::icon("paw",QColor("#397F8D")).pixmap(25,25));animal->setStyleSheet("background:#E4F0F3;padding:5px;border-radius:7px;");h->addWidget(animal);h->addStretch();h->addWidget(chip(state,state=="En étude"?"#FFF0D8":"#E6F0E8",state=="En étude"?"#956722":"#386950"));v->addLayout(h);
 v->addWidget(txt(name,17,"#225A64",true));v->addWidget(muted(type));v->addWidget(single(ident));v->addWidget(muted(hint));return f;
}
void adoptions(){
 auto s=shell(6,"Animaux proposés, candidatures et suivi des adoptions","+ Nouvelle demande");s.w->resize(1400,880);
 auto*summary=new QHBoxLayout;summary->addWidget(chip("6 animaux disponibles"));summary->addWidget(chip("3 demandes en étude","#FFF0D8","#956722"));summary->addWidget(chip("2 adoptions validées","#E4F0F3","#225A64"));summary->addStretch();summary->addWidget(action("Statistiques"));s.v->addLayout(summary);
 s.v->addWidget(toolbar("Rechercher un animal, un candidat ou un dossier…","Tous les statuts","Exporter PDF"));
 auto*filters=new QHBoxLayout;auto*species=new QComboBox;species->addItem("Toutes les espèces");filters->addWidget(species);auto*sort=new QComboBox;sort->addItem("Plus récentes");filters->addWidget(sort);filters->addWidget(action("Réinitialiser"));filters->addStretch();filters->addWidget(single("6 animaux · données fictives"));s.v->addLayout(filters);
 auto*h=new QHBoxLayout;h->setSpacing(16);auto*cards=new QWidget;auto*g=new QGridLayout(cards);g->setContentsMargins(0,0,0,0);g->setSpacing(13);
 g->addWidget(adoptionCard("Milo","Chien · Mâle · 2 ans","ADOP-014 · candidat : Nour B.","En étude","Foyer avec jardin · rencontre le 03/10",true),0,0);
 g->addWidget(adoptionCard("Luna","Chat · Femelle · 3 ans","AN-0037 · aucune demande active","Disponible","Calme · vie en appartement"),0,1);
 g->addWidget(adoptionCard("Nala","Chien · Femelle · 6 ans","AN-0028 · aucune demande active","Disponible","Active · promenades quotidiennes"),1,0);
 g->addWidget(adoptionCard("Simba","Chat · Mâle · 1 an","ADOP-015 · candidat : Sami H.","En étude","Rencontre à organiser"),1,1);
 g->addWidget(adoptionCard("Rio","Chien · Mâle · 5 ans","AN-0053 · aucune demande active","Disponible","Foyer calme · suivi régulier"),2,0);
 g->addWidget(adoptionCard("Maya","Chat · Femelle · 2 ans","ADOP-016 · candidat : Lina K.","En étude","Pièces du dossier à compléter"),2,1);h->addWidget(cards,1);
 QVBoxLayout*dv;auto*detail=panel(dv);detail->setFixedWidth(324);dv->addWidget(txt("DEMANDE SÉLECTIONNÉE",10,"#73868B",true));dv->addWidget(txt("Milo · Nour B.",23,"#225A64",true));dv->addWidget(single("ADOP-014 · créée le 01/10/2026"));dv->addWidget(chip("En étude","#FFF0D8","#956722"),0,Qt::AlignLeft);
 keyrow(dv,"Candidat","Nour B.");keyrow(dv,"Foyer","Jardin clôturé");keyrow(dv,"Rencontre","03/10/2026 · 10:00");keyrow(dv,"Référent","Sarra Gharbi");
 note(dv,"Compatibilité proposée","3 critères sur 4 compatibles.\nPrésence d’un autre animal : à vérifier.\nDécision du responsable requise.");
 dv->addWidget(txt("Validation du dossier",14,"#225A64",true));for(auto item:QStringList{"Identité et coordonnées vérifiées","Rencontre réalisée","Accord du responsable"}){auto*c=new QCheckBox(item);c->setChecked(item.startsWith("Identité"));dv->addWidget(c);}dv->addStretch();
 auto*validate=action("Valider l’adoption",true);validate->setEnabled(false);dv->addWidget(validate);auto*buttons=new QHBoxLayout;buttons->addWidget(action("Consulter"));buttons->addWidget(action("Modifier"));dv->addLayout(buttons);dv->addWidget(action("Préparer le suivi J+7 / J+30"));h->addWidget(detail);s.v->addLayout(h,1);save(s,6);
}
'''
s=s.replace('int main(int argc,char**argv){',adoption+'\nint main(int argc,char**argv){')
s=s.replace('dashboard();services();employees();stock();locals();medical();return 0;','dashboard();medical();adoptions();return 0;')
(work/'gui_aniworld_v4.cpp').write_text(s,encoding='utf-8')
(work/'gui_aniworld_v4.pro').write_text('QT += core gui widgets\nCONFIG += c++17 release\nTEMPLATE = app\nTARGET = aniworld_spec_v4\nSOURCES += gui_aniworld_v4.cpp ../../src/ui/design.cpp\nRESOURCES += ../../resources/resources.qrc\n',encoding='utf-8')
print('Prepared ANIWORLD mockups for dashboard, medical files and adoptions.')
