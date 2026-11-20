from pathlib import Path
p=Path('src/employees/gestionemployes.cpp');s=p.read_text(encoding='utf-8-sig')
s=s.replace('#include <QSet>','#include <QSet>\n#include <QPainter>\n#include <QKeyEvent>\n#include <QMenu>\n#include <QFileDialog>\n#include "../ui/design.h"\n#include "../ui/motion.h"\n#include "../ui/invoice.h"\n#include "../widgets/animatedbutton.h"\n#include "../widgets/animatedpanel.h"')
import re
s=re.sub(r'QSqlQuery (\w+);',r'QSqlQuery \1(QSqlDatabase::database("aniworld-services"));',s)
s=s.replace('new QPushButton(', 'new AnimatedButton(')
s=s.replace('main->setContentsMargins(30, 25, 30, 25);','main->setContentsMargins(24, 24, 24, 20);').replace('main->setSpacing(18);','main->setSpacing(16);')
s=s.replace('font-size: 28px;', 'font-size: 23px;').replace('font-size: 18px;', 'font-size: 14px;')
s=s.replace('setMinimumHeight(46)','setMinimumHeight(34)').replace('setMinimumHeight(44)','setMinimumHeight(36)').replace('setMinimumHeight(40)','setMinimumHeight(34)')
s=s.replace('setMinimumHeight(170)','setFixedHeight(144)').replace('setMinimumWidth(360)','setMinimumWidth(284)').replace('setMaximumWidth(420)','setMaximumWidth(306)')
s=s.replace('setMinimumWidth(520)','setMinimumWidth(490)').replace('setMinimumHeight(650)','setMinimumHeight(580)')
s=s.replace('mainL->setSpacing(15);','mainL->setSpacing(10);').replace('form->setSpacing(12);','form->setSpacing(8);')
s=s.replace('contentL->setContentsMargins(25, 5, 25, 25);','contentL->setContentsMargins(18, 5, 18, 12);')
s=s.replace('padding: 22px 25px 8px 25px;', 'padding: 18px 18px 8px 18px;')
s=s.replace('m_gridCards->setSpacing(18);','m_gridCards->setSpacing(14);')
# Shared motion replaces the source's unbounded pulse and delayed unowned effects.
a=s.index('    // ✅ Bounce-in helper');b=s.index('    auto creerPill',a);s=s[:a]+s[b:]
s=s.replace('        bounceIn(pill, idx * 150);','        Q_UNUSED(idx);')
a=s.index('    QGraphicsOpacityEffect *btnEff');b=s.index('    connect(btnStats',a);s=s[:a]+s[b:]
a=s.index('    if (event->type() == QEvent::Enter)');b=s.index('    if (event->type() == QEvent::MouseButtonPress)',a);s=s[:a]+s[b:]
s=s.replace('if (event->type() == QEvent::MouseButtonPress) {','if ((event->type() == QEvent::MouseButtonPress && static_cast<QMouseEvent *>(event)->button()==Qt::LeftButton) ||\n        (event->type()==QEvent::KeyPress && (static_cast<QKeyEvent *>(event)->key()==Qt::Key_Return || static_cast<QKeyEvent *>(event)->key()==Qt::Key_Space))) {')
s=s.replace('    QFrame *card = new QFrame;\n    bool selectionne', '    QFrame *card = new AnimatedPanel;\n    card->setFocusPolicy(Qt::StrongFocus);\n    bool selectionne')
s=s.replace('    QFrame *card = new QFrame;\n    card->setObjectName("card");','    QFrame *card = Design::card("employeeDetailPanel");')
s=s.replace('    m_editRecherche->setObjectName("searchField");','    m_editRecherche->setObjectName("employeeSearch");')
for var,name in [('m_comboPoste','employeePostFilter'),('m_comboStatut','employeeStatusFilter'),('m_comboTri','employeeSort')]:
    s=s.replace(f'    {var} = new QComboBox;',f'    {var} = new QComboBox; {var}->setObjectName("{name}"); {var}->setMinimumWidth(110); {var}->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);')
for var,name,role in [('btnNouveau','addEmployee','primary'),('btnStats','employeeStatistics',''),('btnModifier','editEmployee','primary'),('btnBasculer','toggleEmployeeAvailability','success'),('btnSupprimer','deleteEmployee','danger'),('btnPlanifier','planEmployeeTask','primary'),('btnConflits','checkEmployeeConflicts','')]:
    s=re.sub(rf'{var}->setObjectName\("\w+"\);',f'{var}->setObjectName("{name}"); {var}->setProperty("role","{role}");',s)
# Inputs and dialogs with stable identifiers.
for var,name in [('editNom','employeeLastName'),('editPrenom','employeeFirstName'),('editCourriel','employeeEmail'),('editTel','employeePhone'),('editComp','employeeSkills'),('spinCharge','employeeWorkload'),('spinDispo','employeeCapacity')]:
    s=s.replace(f'    {var}->setMinimumHeight',f'    {var}->setObjectName("{name}");\n    {var}->setMinimumHeight')
s=s.replace('    dlg.setWindowTitle(modification ?', '    dlg.setObjectName("employeeForm");\n    dlg.setWindowTitle(modification ?')
s=s.replace('        dlg.setWindowTitle("Statistiques des employés");','        dlg.setObjectName("employeeStatisticsDialog");\n        dlg.setWindowTitle("Statistiques des employés");')
s=s.replace('    dlg.setWindowTitle("Planification intelligente");','    dlg.setObjectName("employeePlanningDialog");\n    dlg.setWindowTitle("Planification intelligente");')
s=s.replace('        QTableWidget *table = new QTableWidget;', '        QTableWidget *table = new QTableWidget; table->setObjectName("employeeLoadTable");')
s=s.replace('    m_detailNom = new QLabel;', '    m_detailNom = new QLabel; m_detailNom->setObjectName("employeeDetailName"); m_detailNom->setTextFormat(Qt::PlainText);')
s=s.replace('        valeur = new QLabel("-");', '        valeur = new QLabel("-"); valeur->setTextFormat(Qt::PlainText);')
s=s.replace('    layout->addWidget(nom);','    nom->setTextFormat(Qt::PlainText); nom->setWordWrap(true);\n    layout->addWidget(nom);')
# White detail surface and transparent card area.
s=s.replace('    scroll->setWidget(m_cardsContainer);','    scroll->setWidget(m_cardsContainer);\n    m_cardsContainer->setAutoFillBackground(false); scroll->viewport()->setAutoFillBackground(false);')
s=s.replace('    scroll->setWidget(m_detailContent);','    scroll->setWidget(m_detailContent);\n    m_detailContent->setAutoFillBackground(false); scroll->viewport()->setAutoFillBackground(false);')
# Keep detail actions available at the bottom, outside the scrolling fields.
s=s.replace('    contentL->addSpacing(20);','    m_detailActions=new QWidget;\n    auto *actions=new QVBoxLayout(m_detailActions); actions->setContentsMargins(18,8,18,18); actions->setSpacing(8);')
for name in ['btnModifier','btnBasculer','btnSupprimer']:
    s=s.replace('    contentL->addWidget('+name+');','    actions->addWidget('+name+');')
s=s.replace('    layout->addWidget(scroll, 1);\n\n    m_detailContent->hide();','    layout->addWidget(scroll, 1);\n    layout->addWidget(m_detailActions); m_detailActions->hide();\n\n    m_detailContent->hide();')
s=s.replace('    m_detailContent->show();','    m_detailContent->show(); m_detailActions->show();')
s=s.replace('    m_detailContent->hide();\n    m_detailPlaceholder->show();','    m_detailContent->hide(); m_detailActions->hide();\n    m_detailPlaceholder->show();')
# Refresh selection after mutation; do not display a stale employee outside the filter.
s=s.replace('    afficherCards(liste);\n    mettreAJourStats();','    afficherCards(liste);\n    bool selected=false; for(const auto &employee:liste) if(employee.getId()==m_idSelectionne) { afficherDetail(employee); selected=true; break; }\n    if(!selected) effacerDetail();\n    mettreAJourStats();')
s=s.replace('        afficherDetail(e);\n    }\n}', '    }\n}')
s=s.replace('        if (item->widget()) item->widget()->deleteLater();','        if (item->widget()) { item->widget()->hide(); item->widget()->deleteLater(); }')
s=s.replace('    int row = 0, col = 0;\n    const int COLS = 2;', '    if(liste.isEmpty()) { auto *empty=Design::label("Aucun employé ne correspond à votre recherche.","muted"); empty->setWordWrap(true); m_gridCards->addWidget(empty,0,0,1,2); }\n    int row = 0, col = 0;\n    const int COLS = 2;')
s=s.replace('    mettreAJourStats();\n}\n\n// ==================== UI', '    onFiltresChange();\n}\n\nvoid GestionEmployes::paintEvent(QPaintEvent *) { QPainter painter(this); Design::paintWorkspace(painter,rect()); }\nvoid GestionEmployes::focusSearch() { m_editRecherche->setFocus(); m_editRecherche->selectAll(); }\n\n// ==================== UI')
# File action in employee header, without another application shell.
s=s.replace('    layout->addWidget(btnNouveau);','    auto *attestation=Design::button("Attestation PDF","export"); attestation->setObjectName("employeeLeavePdf");\n    connect(attestation,&QPushButton::clicked,this,&GestionEmployes::exportLeaveCertificate);\n    layout->addWidget(attestation);\n    layout->addWidget(btnNouveau);')
# Borrow leave certificate workflow from the supplied project; use shared atomic PDF writer.
original=Path(r'C:/Users/malek/Downloads/Employessanslogin/Employes/ANIWORLD2/mainwindow.cpp').read_text(encoding='utf-8-sig')
a=original.index('        QDialog dlg(this);',original.index('QAction *actAttestation'))
b=original.index('\n    });',a)
leave=original[a:b]
leave=re.sub(r'QSqlQuery (\w+);',r'QSqlQuery \1(QSqlDatabase::database("aniworld-services"));',leave)
a=leave.index('        QPrinter printer');b=leave.index('        document.setHtml',a)
leave=leave[:a]+'        const QString html = '+leave[b+len('        document.setHtml('):]
leave=leave.replace('QDate::currentDate().toString("dd/MM/yyyy")));','QDate::currentDate().toString("dd/MM/yyyy"));')
a=leave.index('        document.print(&printer);');b=leave.index('            QMessageBox::information',a)
leave=leave[:a]+'        QString error;\n        if (Invoice::writeDocument(fichier, html, "Attestation de congé", &error)) {\n'+leave[b:]
leave=leave.replace('"Le PDF n\'a pas pu être créé. Vérifiez les droits d\'écriture du dossier."','error')
leave=leave.replace('        dlg.setWindowTitle("Attestation de congé");','        dlg.setObjectName("employeeLeaveDialog");\n        dlg.setWindowTitle("Attestation de congé");')
s+='\nvoid GestionEmployes::exportLeaveCertificate() {\n'+leave+'\n}\n'
p.write_text(s,encoding='utf-8')
p=Path('src/employees/gestionemployes.h');s=p.read_text(encoding='utf-8-sig').replace('    void actualiserStats();','    void actualiserStats();\n    void focusSearch();\npublic slots:\n    void ajouterEmploye();\n    void exportLeaveCertificate();').replace('    bool eventFilter','    void paintEvent(QPaintEvent *event) override;\n    bool eventFilter').replace('private slots:\n    void ajouterEmploye();','private slots:').replace('    QWidget *m_detailContent;','    QWidget *m_detailContent;\n    QWidget *m_detailActions;');p.write_text(s,encoding='utf-8')
