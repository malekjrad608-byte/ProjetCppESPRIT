from pathlib import Path
p=Path('src/widgets/servicewidget.h');s=p.read_text(encoding='utf-8-sig').replace('#include <QTableWidget>','#include <QTableView>');s=s.replace('    QTableWidget *m_table;','    Service Stmp; // Objet du modele, comme Etmp dans l\'atelier.\n    QTableView *m_table;');p.write_text(s,encoding='utf-8')
p=Path('src/widgets/servicewidget.cpp');s=p.read_text(encoding='utf-8-sig').replace('#include <QTextBrowser>','#include <QTextBrowser>\n#include <QSqlQueryModel>\n#include <QSqlError>\n#include <QStyledItemDelegate>')
s=s.replace('namespace {','''namespace {
class ServiceTableDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        QStyledItemDelegate::initStyleOption(option,index);
        if(index.column()==5) option->text=QDateTime::fromString(index.data().toString(),Qt::ISODate).toString("dd/MM/yyyy HH:mm");
        if(index.column()==8) option->text=Design::money(index.data().toDouble());
    }
};
''',1)
a=s.index('    m_table = new QTableWidget;');b=s.index('    m_views->addWidget(m_table);',a)
s=s[:a]+'''    m_table = new QTableView;
    m_table->setObjectName("serviceTable");
    m_table->setModel(Stmp.afficher(this));
    m_table->setItemDelegate(new ServiceTableDelegate(m_table));
    m_table->verticalHeader()->hide();
    m_table->verticalHeader()->setDefaultSectionSize(65);
    m_table->setShowGrid(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setWordWrap(false);
    m_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setHighlightSections(false);
    for (int col=0;col<15;++col) m_table->setColumnHidden(col,col!=1 && col!=5 && col!=6 && col!=8);
    for (int col : {5,6,8}) m_table->horizontalHeader()->setSectionResizeMode(col,QHeaderView::Fixed);
    m_table->setColumnWidth(5,155);
    m_table->setColumnWidth(6,108);
    m_table->setColumnWidth(8,85);
'''+s[b:]
a=s.index('    connect(m_table,&QTableWidget::itemSelectionChanged');b=s.index('\n',s.index('    });',s.index('    connect(m_table,&QTableWidget::cellDoubleClicked',a)))
s=s[:a]+'''    connect(m_table,&QTableView::doubleClicked,this,[this](const QModelIndex &index) {
        viewService(index.siblingAtColumn(0).data().toInt());
    });'''+s[b:]
a=s.index('    const QSignalBlocker blocker(m_table);',s.index('void ServiceWidget::populate()'));b=s.index('    m_count->setText',a)
s=s[:a]+'''    QList<int> ids;
    for (const Service &service : m_filtered) ids.append(service.getId());
    // Actualisation apres ajout/modification/suppression : afficher() -> setModel().
    auto *previousModel = m_table->model();
    auto *previousSelection = m_table->selectionModel();
    m_table->setModel(Stmp.afficher(this, &ids));
    delete previousSelection;
    delete previousModel;
    auto *model = static_cast<QSqlQueryModel *>(m_table->model());
    while(model->canFetchMore()) model->fetchMore();
    if(model->lastError().isValid()) notify("Impossible d'afficher les services : "+model->lastError().text(),true);
    connect(m_table->selectionModel(),&QItemSelectionModel::currentRowChanged,this,[this](const QModelIndex &current) {
        if(current.isValid()) selectService(current.siblingAtColumn(0).data().toInt());
    });
    for(int row=0;row<model->rowCount();++row)
        if(model->index(row,0).data().toInt()==m_selectedId) { m_table->selectRow(row); break; }
'''+s[b:]
s=s.replace('    const QSignalBlocker blocker(m_table);\n    for (int row=0; row<m_table->rowCount(); ++row)\n        if (m_table->item(row,0)->data(Qt::UserRole).toInt()==id)', '    const QSignalBlocker blocker(m_table->selectionModel());\n    for (int row=0; row<m_table->model()->rowCount(); ++row)\n        if (m_table->model()->index(row,0).data().toInt()==id)')
s=s.replace('        auto entity=form->getService();','        Service entity=form->getService();')
s=s.replace('        QString error;\n        if(!(adding?entity.ajouter(&error):entity.modifier(&error))) {\n            form->show();form->showError(error);return;','        const bool test = adding ? entity.ajouter() : entity.modifier();\n        if(!test) {\n            form->show();form->showError(entity.lastError());return;')
s=s.replace('if (!Service::supprimer(service.id,&error)) {','if (!Stmp.supprimer(service.getId())) {\n        error = Stmp.lastError();')
p.write_text(s,encoding='utf-8')
# Construct entity from the form as in the teacher's example.
p=Path('src/dialogs/servicedialog.cpp');s=p.read_text(encoding='utf-8-sig');s=s.replace('Service s; s.id=m_id; s.localId=m_local->currentData().toInt(); s.name=m_name->text().trimmed(); s.category=m_category->currentText(); s.status=m_status->currentText();', '''Service s(m_id, m_name->text().trimmed(), m_client->text().trimmed(),
              m_category->currentText(), m_date->dateTime(), m_status->currentText(),
              m_duration->value(), m_price->value());
    s.setLocalId(m_local->currentData().toInt());''');p.write_text(s,encoding='utf-8')
# Match theme rules that previously targeted only QTableWidget.
p=Path('resources/theme.qss');s=p.read_text(encoding='utf-8-sig');import re
s=re.sub(r'([^{}]*QTableWidget[^{}]*)\{',lambda m:m.group(1)+','+m.group(1).replace('QTableWidget','QTableView')+'{',s);p.write_text(s,encoding='utf-8')
