from pathlib import Path
p=Path('src/employees/gestionemployes.cpp');s=p.read_text(encoding='utf-8')
# Draw application icons instead of relying on platform emoji fonts.
s=s.replace('    titreL->addWidget(titre);','    auto *titleRow=new QHBoxLayout; auto *titleIcon=Design::label(""); titleIcon->setPixmap(Design::icon("users",QColor("#8268AE")).pixmap(24,24)); titleRow->addWidget(titleIcon); titleRow->addWidget(titre); titleRow->addStretch(); titreL->addLayout(titleRow);',1)
s=s.replace('QFrame *p1 = creerPill("✅"','QFrame *p1 = creerPill("check"').replace('QFrame *p2 = creerPill("❌"','QFrame *p2 = creerPill("users"').replace('QFrame *p3 = creerPill("⏱"','QFrame *p3 = creerPill("clock"')
s=s.replace('        QLabel *ico = new QLabel(icone);','        QLabel *ico = new QLabel; ico->setPixmap(Design::icon(icone,QColor(fg)).pixmap(18,18));')
s=s.replace('creerCard("👥"','creerCard("users"').replace('creerCard("⏱"','creerCard("clock"').replace('creerCard("📈"','creerCard("grid"').replace('creerCard("⚠️"','creerCard("shield"')
s=s.replace('            QLabel *ico = new QLabel(icone);','            QLabel *ico = new QLabel; ico->setPixmap(Design::icon(icone,QColor(fg)).pixmap(24,24));')
import re
for var,title,icon,role in [('btnNouveau','Ajouter un employé','plus','primary'),('btnStats','Voir les statistiques','grid',''),('btnModifier','Modifier','edit','primary'),('btnBasculer','Basculer disponibilité','check','success'),('btnSupprimer','Supprimer','trash','danger'),('btnPlanifier','Planifier et affecter','calendar','primary'),('btnConflits','Détecter les conflits','search','')]:
    s=re.sub(rf'{var} = new AnimatedButton\("[^"\n]*"\);',f'{var} = Design::button("{title}","{icon}","{role}");',s)
s=s.replace('e.isDisponible() ? "✅ Disponible" : "❌ Indisponible"','e.isDisponible() ? "● Disponible" : "● Indisponible"')
s=s.replace('"✅  Disponible"','"● Disponible"').replace('"❌  Non disponible"','"● Non disponible"')
s=s.replace('"👈\\n\\nSélectionnez un employé\\npour voir ses détails"','"Sélectionnez un employé\\npour voir ses détails"')
s=s.replace('    QLabel *charge = new QLabel(QString("⏱ %1 h")','    auto *clock=Design::label(""); clock->setPixmap(Design::icon("clock",QColor("#84979E")).pixmap(12,12)); botRow->addWidget(clock);\n    QLabel *charge = new QLabel(QString("%1 h")')
s=s.replace('    m_editRecherche->setPlaceholderText','    m_editRecherche->addAction(Design::icon("search"),QLineEdit::LeadingPosition);\n    m_editRecherche->setPlaceholderText',1)
# All remaining emoji were decorative; keep labels and messages readable on every Qt platform.
s=re.sub('[\U00010000-\U0010ffff\u2705\u274c\u23f1\u26a0\u270f\u2795\ufe0f]','',s)
s=s.replace('"  Employés"','"Employés"').replace('"  Rechercher','"Rechercher')
s=s.replace('    btns->button(QDialogButtonBox::Cancel)->setMinimumHeight','    btns->button(QDialogButtonBox::Cancel)->setText("Annuler");\n    btns->button(QDialogButtonBox::Cancel)->setMinimumHeight')
s=s.replace('    buttons->button(QDialogButtonBox::Cancel)->setMinimumHeight','    buttons->button(QDialogButtonBox::Cancel)->setText("Annuler");\n    buttons->button(QDialogButtonBox::Cancel)->setMinimumHeight')
p.write_text(s,encoding='utf-8')
p=Path('tests/service_tests.cpp');s=p.read_text(encoding='utf-8');s=s.replace('employees->findChild<QLineEdit *>("employeeSearch")->setText("Integration"); QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);','employees->findChild<QLineEdit *>("employeeSearch")->setText("Integration"); QTest::qWait(60);')
s=s.replace('            if(!output.isEmpty()) dialog->grab().save(output+"/employee-statistics.png"); dialog->reject(); });','            if(!output.isEmpty()) dialog->grab().save(output+"/employee-statistics.png");\n            dialog->reject(); });')
p.write_text(s,encoding='utf-8')
