from pathlib import Path
p=Path('resources/theme.qss'); s=p.read_text(encoding='utf-8')
s=s.replace('QFrame#sidebar QFrame#workspace { background: #E3F0F3; border: none; }','QFrame#sidebar QFrame#workspace { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #A6D7DF,stop:1 #78B5C1); border: none; }')
s=s.replace('QPushButton[role="danger"] { color: #AE5751; border-color: #EACFCB; background: #FFF8F5; }','QPushButton[role="danger"] { color: white; border-color: #B56761; background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #D17B74,stop:1 #A65450); }').replace('QPushButton[role="danger"]:hover { background: #FBE6DF; }','QPushButton[role="danger"]:hover { background: #B86459; border-color: #E7A342; }')
s+='''
QPushButton[role="success"] { color:white; border:1px solid #53A16A; background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #69BB7D,stop:1 #418D57); }
QPushButton[role="success"]:hover { background:#4B9C62; border-color:#E7A342; }
QPushButton[role="success"]:disabled { background:#B5CDBB; border-color:#B5CDBB; color:#EDF4EF; }
QPushButton[detailAction="true"] { border-radius:10px; font-size:13px; font-weight:700; padding:9px 12px; }
QFrame#serviceDetailPanel { border-radius:10px; border-color:#D7E8EE; }
QTableView::item:hover { background:#FFF0DA; color:#795126; }
QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover { background:#F0A342; }
QMenu::item:selected { background:#FFF0DA; color:#86521D; }
'''
p.write_text(s,encoding='utf-8')
p=Path('src/widgets/servicewidget.cpp'); s=p.read_text(encoding='utf-8')
s=s.replace('m_advance = Design::button("Démarrer le service", "check", "primary");','m_advance = Design::button("Démarrer le service", "check", "success");').replace('    content->addWidget(m_advance);\n','')
s=s.replace('m_edit = Design::button("Modifier", "edit");','m_edit = Design::button("Modifier", "edit", "primary");')
s=s.replace('    auto *row = new QHBoxLayout;\n    row->setSpacing(7); row->addWidget(m_view); row->addWidget(m_edit);\n    content->addLayout(row);\n','')
s=s.replace('m_invoice = Design::button("Facture PDF", "export", "primary");','m_invoice = Design::button("Facture PDF", "export");')
s=s.replace('    content->addWidget(m_invoice);\n    content->addWidget(m_delete);\n    for (auto *button : {m_advance,m_view,m_edit,m_invoice,m_delete}) {\n        button->setMinimumHeight(31);\n        button->setStyleSheet("padding:6px 9px;");\n    }','''    auto *documents=new QHBoxLayout; documents->setSpacing(7); documents->addWidget(m_view); documents->addWidget(m_invoice);
    content->addLayout(documents);
    content->addWidget(m_edit); content->addWidget(m_advance); content->addWidget(m_delete);
    for(auto *button:{m_edit,m_advance,m_delete}) {
        button->setProperty("detailAction",true); button->setMinimumHeight(40);
    }
    for(auto *button:{m_view,m_invoice}) button->setStyleSheet("padding:6px 8px;");''')
s=s.replace('    content->addLayout(m_statusLayout);','''    content->addLayout(m_statusLayout);
    auto *separator=new QFrame; separator->setFixedHeight(1); separator->setStyleSheet("background:#E0EDF1;border:none;"); content->addWidget(separator);''')
p.write_text(s,encoding='utf-8')
p=Path('src/dialogs/servicedialog.cpp'); s=p.read_text(encoding='utf-8').replace('#include <QGridLayout>','#include <QFormLayout>')
s=s.replace('setMinimumWidth(570)','setMinimumWidth(470)')
a=s.index('    auto *head=new QHBoxLayout;'); b=s.index('    m_name=new QLineEdit',a)
s=s[:a]+'''    auto *head=new QHBoxLayout; head->setSpacing(10);
    auto *symbol=Design::label(""); symbol->setPixmap(Design::icon(readOnly?"services":editing?"edit":"plus",QColor("#8B70C5")).pixmap(22,22)); head->addWidget(symbol);
    head->addWidget(Design::label(title,"section")); head->addStretch(); root->addLayout(head);
    root->addWidget(Design::label(readOnly?QString("PRESTATION / SRV-%1").arg(s.id,3,10,QChar('0')):"* Champs obligatoires", "muted"));
    auto *form=new QFormLayout; form->setContentsMargins(0,0,0,0); form->setHorizontalSpacing(18); form->setVerticalSpacing(9);
    form->setLabelAlignment(Qt::AlignLeft|Qt::AlignVCenter); form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
'''+s[b:]
s=s.replace('    form->addWidget(Design::label("Nom du service *"),0,0,1,2); form->addWidget(m_name,1,0,1,2);','')
s=s.replace('    form->addWidget(Design::label("Catégorie"),2,0); form->addWidget(Design::label("Statut"),2,1); form->addWidget(m_category,3,0); form->addWidget(m_status,3,1);','')
s=s.replace('    form->addWidget(Design::label("Date et heure"),4,0); form->addWidget(Design::label("Durée"),4,1); form->addWidget(m_date,5,0); form->addWidget(m_duration,5,1);','')
s=s.replace('    form->addWidget(Design::label("Responsable"),6,0); form->addWidget(Design::label("Tarif"),6,1); form->addWidget(m_responsible,7,0); form->addWidget(m_price,7,1);','')
s=s.replace('m_description->setFixedHeight(88)','m_description->setFixedHeight(72)')
a=s.index('    form->addWidget(Design::label("Client *")'); b=s.index('    m_error=Design::label',a)
s=s[:a]+'''    form->addRow("Service *",m_name); form->addRow("Client *",m_client);
    form->addRow("Catégorie",m_category); form->addRow("Statut",m_status);
    form->addRow("Date et heure",m_date); form->addRow("Durée",m_duration);
    form->addRow("Responsable",m_responsible); form->addRow("Tarif",m_price);
    form->addRow("Prestations",m_description); root->addLayout(form);
'''+s[b:]
s=s.replace('Design::button(readOnly?"Fermer":"Annuler")','Design::button(readOnly?"Fermer":"Annuler",{},"primary")').replace('buttons->addWidget(cancel);','cancel->setMinimumWidth(125);')
s=s.replace('editing?"Enregistrer les modifications":"Créer le service"','editing?"Enregistrer":"Ajouter"').replace('buttons->addWidget(save);','save->setMinimumWidth(125); buttons->addWidget(save);')
s=s.replace('    root->addLayout(buttons);','    buttons->addWidget(cancel); root->addLayout(buttons);')
p.write_text(s,encoding='utf-8')
# Use filled buttons for the two stock operations, with the shared hover.
p=Path('src/stock/stockwidget.cpp'); s=p.read_text(encoding='utf-8').replace('Design::button("Préparer la sortie","arrow")','Design::button("Préparer la sortie","arrow","primary")').replace('Design::button("Vérifier la proposition","check")','Design::button("Vérifier la proposition","check","success")'); p.write_text(s,encoding='utf-8')
