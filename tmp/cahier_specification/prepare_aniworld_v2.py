from pathlib import Path
W=Path(__file__).parent
s=(W/'gui_devcore.cpp').read_text(encoding='utf-8')
s=s.replace('static QString output;', 'static QString output;')
s=s.replace('size=13,QString color="#344B43"','size=13,QString color="#263B42"')
s=s.replace('auto *l=new QLabel(text);l->setWordWrap(true);','auto *l=new QLabel(text);if(size>=11 && size<=16)size+=1;l->setWordWrap(true);')
s=s.replace('QPixmap paw(int size){QImage i("C:/Users/malek/Downloads/logo-removebg-preview.png");return QPixmap::fromImage(i).scaled(size,size,Qt::KeepAspectRatio,Qt::SmoothTransformation);}',r'''
QPixmap brandAsset(QString file,int width,int height){
 QImage i("C:/Users/malek/Downloads/"+file);QRect bounds;
 for(int y=0;y<i.height();++y)for(int x=0;x<i.width();++x)if(qAlpha(i.pixel(x,y))>0)bounds=bounds.united(QRect(x,y,1,1));
 QPixmap out(width*2,height*2);out.fill(Qt::transparent);QPainter p(&out);p.setRenderHint(QPainter::SmoothPixmapTransform);QSizeF fit=bounds.size();fit.scale(out.size(),Qt::KeepAspectRatio);QRectF target((out.width()-fit.width())/2,(out.height()-fit.height())/2,fit.width(),fit.height());p.drawImage(target,i,bounds);p.end();out.setDevicePixelRatio(2);return out;
}
QPixmap appSymbol(int size){return brandAsset("new logo.png",size,size);}
''')
s=s.replace('paw(76)','appSymbol(104)')
old='auto*brand=new QHBoxLayout;auto*logo=new QLabel;logo->setPixmap(paw(44));brand->addWidget(logo);auto*bv=new QVBoxLayout;bv->setSpacing(0);bv->addWidget(txt("ANIWORLD",17,"#244D3D",true));bv->addWidget(muted("Centre de soins"));brand->addLayout(bv);sv->addLayout(brand);sv->addSpacing(25);sv->addWidget(txt("ESPACE DE GESTION",9,"#88968C",true));'
new='''auto*logo=new QLabel;logo->setPixmap(brandAsset("new name.png",170,70));logo->setAlignment(Qt::AlignCenter);sv->addWidget(logo);sv->addSpacing(18);
 auto*workspace=new QFrame;workspace->setStyleSheet("QFrame{background:#EAF2F4;border:1px solid #DAE6E9;border-radius:9px;}");auto*wv=new QVBoxLayout(workspace);wv->setContentsMargins(12,10,12,10);wv->setSpacing(3);wv->addWidget(txt("Centre de soins",13,"#225A64",true));wv->addWidget(muted("Espace de gestion"));sv->addWidget(workspace);sv->addSpacing(18);sv->addWidget(txt("NAVIGATION",10,"#73868B",true));'''
assert old in s;s=s.replace(old,new)
s=s.replace('sv->addStretch();sv->addWidget(txt("devCore",16,"#567A65",true));sv->addWidget(muted("Projet C++ 2A"));','sv->addStretch();sv->addWidget(txt("Une attention à chaque patte.",12,"#225A64",true));sv->addWidget(muted("ANIWORLD · Centre de soins"));')
s=s.replace('w->resize(1120,740)','w->resize(1200,790)').replace('side->setFixedWidth(183)','side->setFixedWidth(205)')
s=s.replace('setContentsMargins(15,22,15,18)','setContentsMargins(16,24,16,20)')
s=s.replace('txt(names[n],25,','txt(names[n],28,')
s=s.replace('auto*bar=new QFrame;bar->setObjectName("topbar");bar->setFixedHeight(48);','auto*bar=new QFrame;bar->setObjectName("topbar");bar->setFixedHeight(54);')
s=s.replace('bh->addWidget(muted(n==5?"Dr Amine K. · Vétérinaire":"Responsable du centre"));','bh->addWidget(single(n==5?"Dr Amine K. · Vétérinaire":"Responsable du centre",12));')
s=s.replace('muted("ANIWORLD  ·  devCore")','muted("ANIWORLD · Gestion du centre")')
s=s.replace('muted("ANIWORLD · Gestion du centre")','single("ANIWORLD · Gestion du centre")')
s=s.replace('muted("5 articles affichés sur 24")','single("5 articles affichés sur 24")')
s=s.replace('s.w->grab().save(output+', 'if(n==3||n==4){s.w->resize(s.w->width(),s.w->height()+34);QApplication::processEvents();}s.w->grab().save(output+')
s=s.replace('auto*av=new QLabel;av->setPixmap(paw(56));ph->addWidget(av);','auto*av=txt("NA",22,"#225A64",true);av->setFixedSize(56,56);av->setAlignment(Qt::AlignCenter);av->setStyleSheet("font-size:22px;font-weight:600;color:#225A64;background:#EAF2F4;border-radius:28px;");ph->addWidget(av);')
s=s.replace('setFixedWidth(265)','setFixedWidth(278)').replace('setFixedWidth(282)','setFixedWidth(292)').replace('setFixedWidth(268)','setFixedWidth(286)').replace('setFixedWidth(245)','setFixedWidth(260)')
s=s.replace('setMinimumHeight(55+42*rows.size())','setMinimumHeight(59+46*rows.size())').replace('setRowHeight(r,42)','setRowHeight(r,46)')
s=s.replace('txt(a[1],26,','txt(a[1],30,')
s=s.replace('setMinimumHeight(171)','setMinimumHeight(181)')
s=s.replace('chip("3 disponibles")','chip("4 disponibles")').replace('chip("2 occupés"','chip("1 occupé"')
s=s.replace('"2 animaux","Occupé","Nala · fin prévue à 15:45"','"2 animaux","Disponible","Nala · réservation à 14:00"')
s=s.replace('txt("Évolution du poids",14','txt("Poids (kg)",14')
# Améliorer la palette et le contraste de l’ensemble des vues, sans toucher aux visuels fournis.
palette={'#344B43':'#263B42','#718078':'#536C73','#355F49':'#225A64','#244B3D':'#173E48','#607C69':'#496B74','#647B6C':'#4D6870','#87978B':'#698088','#65776A':'#506C73','#7B8D80':'#60767D','#87928A':'#59717A','#7E8D82':'#536E76','#E3EEE5':'#E4F0F3','#D9E6DC':'#D3E4E8','#F5F6F2':'#F4F7F8','#DFE7DF':'#D8E3E7','#E2EDE5':'#DEEDF1','#246756':'#225A64','#79A185':'#57909C','#245E49':'#225A64','#EDF4EF':'#ECF4F6','#E2EEE5':'#DFEDF1','#3D7054':'#225A64','#3D674F':'#225A64','#355D48':'#225A64','#E7F0E9':'#E4F0F3','#BED6C4':'#B6D1D9','#36534E':'#2A515B'}
for a,b in palette.items():s=s.replace(a,b)
s=s.replace('QFrame#panel{background:white;border:1px solid #D8E3E7;border-radius:10px;}','QFrame#panel{background:white;border:1px solid #D8E3E7;border-radius:12px;}')
s=s.replace('QPushButton{font-size:12px;padding:9px 12px;} QPushButton[role=\'nav\']{font-size:12px;padding:11px 8px;}','''QPushButton{font-size:13px;padding:10px 13px;color:#2A515B;} QPushButton[role='nav']{font-size:13px;padding:12px 8px;color:#536C73;} QPushButton[role='nav']:checked{background:#E4F0F3;color:#225A64;border-left:3px solid #397F8D;font-weight:600;} QPushButton[role='primary']{background:#225A64;border-color:#225A64;color:white;} QPushButton[role='primary']:disabled{background:#B3C4C8;border-color:#B3C4C8;color:white;}''')
s=s.replace('QLineEdit,QComboBox{font-size:12px;padding:9px 10px;} QTableWidget{font-size:12px;}','QLineEdit,QComboBox{font-size:13px;padding:10px 11px;} QTableWidget{font-size:13px;selection-background-color:#E8F2F4;selection-color:#225A64;}')
s=s.replace('QHeaderView::section{font-size:10px;padding:11px 7px;color:#536E76;}','QHeaderView::section{font-size:11px;padding:12px 7px;color:#536E76;background:#EFF4F6;}')
s=s.replace('QCheckBox{font-size:12px;','QCheckBox{font-size:13px;')
s=s.replace('font-size:11px;color:', 'font-size:12px;color:')
s=s.replace('font-size:12px;border:none;border-radius:6px','font-size:13px;border:none;border-radius:7px')
s=s.replace('"Présent"},{"Amine','"Présente"},{"Amine')
s=s.replace('QFont("Segoe UI",9,QFont::DemiBold)','QFont("Segoe UI",10,QFont::DemiBold)')
s=s.replace('Planner(){setMinimumSize(540,365);}','Planner(){setMinimumSize(540,385);}')
s=s.replace('return t;','t->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);return t;')
(W/'gui_aniworld_v2.cpp').write_text(s,encoding='utf-8')
(W/'gui_aniworld_v2.pro').write_text('QT += core gui widgets\nCONFIG += c++17 release\nTEMPLATE = app\nTARGET = aniworld_spec_v2\nSOURCES += gui_aniworld_v2.cpp ../../src/ui/design.cpp\nRESOURCES += ../../resources/resources.qrc\n',encoding='utf-8')
print('Prepared six native Qt mockups with the supplied ANIWORLD branding.')
