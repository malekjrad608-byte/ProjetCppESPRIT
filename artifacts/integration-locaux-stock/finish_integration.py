from pathlib import Path
import re
root=Path(r'C:\Users\malek\Documents\gestion_service')
def read(p): return (root/p).read_text(encoding='utf-8')
def write(p,s): (root/p).write_text(s,encoding='utf-8')
s=read('src/locaux/localrepository.cpp').replace('#include <QFile>','#include <QFile>\n#include <QSaveFile>\n#include <QSet>')
a=s.index('LocalRepository::LocalRepository');b=s.index('void LocalRepository::seed()',a)
s=s[:a]+'''LocalRepository::LocalRepository(QObject *parent, const QString &storagePath)
    : QObject(parent), m_path(storagePath) { load(); }

QString LocalRepository::filePath() const { return m_path; }

void LocalRepository::load()
{
    m_items.clear();
    if(m_path==":memory:") { seed(); return; }
    const bool existing=QFile::exists(m_path);
    QFile f(existing?m_path:QString(":/data/locaux-initial.json"));
    if(!f.open(QIODevice::ReadOnly)) {
        m_loadFailed=true; m_error="Impossible de lire les locaux : "+f.errorString(); return;
    }
    QJsonParseError error;
    const auto doc=QJsonDocument::fromJson(f.readAll(),&error);
    if(error.error!=QJsonParseError::NoError || !doc.isArray()) {
        m_loadFailed=true; m_error="Le fichier des locaux est invalide. Il a été conservé sans modification."; return;
    }
    QSet<int> ids; QSet<QString> names;
    for(const auto &value:doc.array()) {
        const Local l=Local::fromJson(value.toObject());
        if(!value.isObject() || l.id<=0 || ids.contains(l.id) || l.name.trimmed().isEmpty()
                || names.contains(l.name.trimmed().toCaseFolded()) || l.capacity<1 || l.occupancy<0 || l.occupancy>l.capacity) {
            m_items.clear(); m_loadFailed=true; m_error="Le fichier des locaux contient des données invalides. L'original a été conservé."; return;
        }
        ids.insert(l.id); names.insert(l.name.trimmed().toCaseFolded()); m_items.append(l);
    }
    if(!existing) save();
}

bool LocalRepository::save()
{
    if(m_loadFailed) { emit storageError(m_error); return false; }
    if(m_path==":memory:") return true;
    QDir().mkpath(QFileInfo(m_path).absolutePath());
    QSaveFile f(m_path);
    QJsonArray arr; for(const auto &l:m_items) arr.append(l.toJson());
    const QByteArray data=QJsonDocument(arr).toJson(QJsonDocument::Indented);
    if(!f.open(QIODevice::WriteOnly) || f.write(data)!=data.size() || !f.commit()) {
        m_error="Les locaux n'ont pas pu être enregistrés : "+f.errorString();
        emit storageError(m_error); return false;
    }
    m_error.clear(); return true;
}

'''+s[b:]
s=s.replace('    seed();\n    save();\n    emit changed();','    const auto before=m_items;\n    seed();\n    if(!save()) { m_items=before; return; }\n    emit changed();')
s=s.replace('    l.id = nextId();','    if(l.name.trimmed().isEmpty() || nameExists(l.name) || l.capacity<1 || l.occupancy<0 || l.occupancy>l.capacity) return 0;\n    l.id = nextId();')
s=s.replace('    m_items.append(l);\n    save();','    m_items.append(l);\n    if(!save()) { m_items.removeLast(); return 0; }')
s=s.replace('    for (Local &x : m_items) {','    if(l.name.trimmed().isEmpty() || nameExists(l.name,l.id) || l.capacity<1 || l.occupancy<0 || l.occupancy>l.capacity) return false;\n    for (Local &x : m_items) {')
s=s.replace('            x = l;\n            save();','            const Local before=x;\n            x = l;\n            if(!save()) { x=before; return false; }')
s=s.replace('            m_items.removeAt(i);\n            save();','            const Local before=m_items.takeAt(i);\n            if(!save()) { m_items.insert(i,before); return false; }')
write('src/locaux/localrepository.cpp',s)
s=read('src/locaux/pagelocaux.cpp').replace('    root->addLayout(buildHeader());','''    if(!repo->lastError().isEmpty()) {
        auto *error=Design::label(repo->lastError()); error->setWordWrap(true);
        error->setStyleSheet("color:#B94A48;background:#FBEAEA;padding:10px;"); root->addWidget(error);
    }
    root->addLayout(buildHeader());''')
write('src/locaux/pagelocaux.cpp',s)
s=read('src/locaux/pdfreport.cpp').replace('#include <QFile>','#include <QFile>\n#include <QSaveFile>')
s=s.replace('QFile probe(path);','QSaveFile output(path);').replace('probe.open','output.open').replace('    probe.close();','')
s=s.replace('    QPdfWriter writer(path);','    {\n    QPdfWriter writer(&output);').replace('    doc.print(&writer);\n    return true;','''    doc.print(&writer);
    }
    if(!output.commit()) { if(error) *error=output.errorString(); return false; }
    return true;''')
write('src/locaux/pdfreport.cpp',s)
s=read('src/locaux/roomcard.cpp')
s=s.replace('QColor c = Theme::TealDark;', 'QColor c = Theme::Orange;').replace('QColor c0 = Theme::Teal;', 'QColor c0 = Theme::Orange;').replace('QColor c1 = Theme::Teal;', 'QColor c1 = Theme::Orange;').replace('QColor pc = Theme::Teal;', 'QColor pc = Theme::Orange;').replace('Theme::lerp(Theme::Line, QColor(0x9c, 0xc6, 0xcf), m_hover)','Theme::lerp(Theme::Line, Theme::Orange, m_hover)')
write('src/locaux/roomcard.cpp',s)
s=read('src/stock/stockwidget.cpp')
s='#include "../widgets/animatedbutton.h"\n#include <limits>\n'+s
# Use common styles in all modal dialogs.
s=re.sub(r'        setStyleSheet\(QStringLiteral\(.*?\)\.arg\(kInk\)\);','',s,count=1,flags=re.S)
s=re.sub(r'    dialog.setStyleSheet\(QStringLiteral\(.*?\)\.arg\(kInk\)\);','',s,flags=re.S)
s=re.sub(r'    dialog.setStyleSheet\("QDialog.*?\);','',s)
s=re.sub(r'        buttons->button\(QDialogButtonBox::Save\)->setStyleSheet[^\n]+','',s)
s=re.sub(r'    buttons->button\(QDialogButtonBox::Save\)->setStyleSheet[^\n]+','',s)
# Animated equivalents of standard dialog actions.
pos=s.index('class ProductEditDialog')
s=s[:pos]+'''QDialogButtonBox *actionBox(QDialogButtonBox::StandardButtons actions) {
    auto *box=new QDialogButtonBox;
    if(actions.testFlag(QDialogButtonBox::Save)) box->addButton(Design::button("Enregistrer", "check", "primary"),QDialogButtonBox::AcceptRole);
    if(actions.testFlag(QDialogButtonBox::Cancel)) box->addButton(Design::button("Annuler"),QDialogButtonBox::RejectRole);
    if(actions.testFlag(QDialogButtonBox::Close)) box->addButton(Design::button("Fermer",{},"primary"),QDialogButtonBox::RejectRole);
    return box;
}

'''+s[pos:]
# Keep standard button identities so existing callers can label or test them.
a=s.index('QDialogButtonBox *actionBox');b=s.index('class ProductEditDialog',a)
s=s[:a]+'''QDialogButtonBox *actionBox(QDialogButtonBox::StandardButtons actions) {
    auto *box=new QDialogButtonBox(actions);
    for(auto *button:box->buttons()) {
        const auto standard=box->standardButton(button);
        button->setProperty("role",standard==QDialogButtonBox::Save || standard==QDialogButtonBox::Close?"primary":"normal");
    }
    return box;
}

'''+s[b:]
s=s.replace('new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel)','actionBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel)').replace('new QDialogButtonBox(QDialogButtonBox::Close)','actionBox(QDialogButtonBox::Close)')
s=s.replace('new QPushButton(', 'new AnimatedButton(')
a=s.index('    addArticleButton->setStyleSheet');b=s.index('    articleSelectorLayout->addWidget',a)
s=s[:a]+s[b:]
s=s.replace('        setWindowTitle(title);','        setWindowTitle(title); setObjectName("stockProductDialog");')
for var,name in [('articleEdit','stockArticleName'),('lotEdit','stockLot'),('quantitySpin','stockAvailable'),('thresholdSpin','stockThreshold'),('expiryEdit','stockExpiry')]:
    s=s.replace('        form->addRow("Catégorie", categoryBox);',f'        {var}->setObjectName("{name}");\n        form->addRow("Catégorie", categoryBox);',1)
for title,name in [('Enregistrer un mouvement de stock','stockMovementDialog'),('Statistiques du stock','stockStatisticsDialog'),('Gestion des articles périmés','stockExpiryDialog')]:
    s=s.replace(f'dialog.setWindowTitle("{title}");',f'dialog.setWindowTitle("{title}"); dialog.setObjectName("{name}");')
for var,name in [('itemBox','stockMovementArticle'),('movementType','stockMovementType'),('quantity','stockMovementQuantity'),('addArticleButton','stockAddArticleButton')]:
    s=s.replace('    form->addRow("Article / lot", articleSelector);',f'    {var}->setObjectName("{name}");\n    form->addRow("Article / lot", articleSelector);',1)
s=s.replace('        if (isOutput) items[sourceRow].quantity -= amount;','''        if (!isOutput && items.at(sourceRow).quantity > 999999-amount) {
            QMessageBox::warning(&dialog,"Quantité maximale","Le stock ne peut pas dépasser 999 999 unités par lot."); return;
        }
        if (isOutput) items[sourceRow].quantity -= amount;''')
s=s.replace('cell->setEditable(false);','cell->setEditable(false); cell->setToolTip(values.at(column));')
write('src/stock/stockwidget.cpp',s)
print('Persistence, shared dialogs and atomic PDF export updated')
