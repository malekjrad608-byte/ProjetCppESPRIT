from pathlib import Path
p=Path('tests/service_tests.cpp')
s=p.read_text(encoding='utf-8-sig')
s='''#include "../src/locaux/localrepository.h"
#include "../src/locaux/pagelocaux.h"
#include "../src/locaux/localdialog.h"
#include "../src/locaux/plandialog.h"
#include "../src/locaux/pdfreport.h"
#include "../src/locaux/smartassign.h"
#include "../src/locaux/smartsearch.h"
#include <QDateEdit>
'''+s
a=s.index('    void stockFiltersAndMovements()');b=s.index('    void stockIntegratedNavigationAndLogout()',a)
s=s[:a]+'''    void stockFiltersAndMovements() {
        StockWidget stock; stock.resize(1050,810); stock.show();
        auto *table=stock.findChild<QTableView *>("stockTable"); QVERIFY(table);
        auto *model=table->model(); QCOMPARE(model->rowCount(),19);
        auto *search=stock.findChild<QLineEdit *>("stockSearch");
        auto *category=stock.findChild<QComboBox *>("stockCategoryFilter");
        auto *sort=stock.findChild<QComboBox *>("stockSortFilter");
        search->setText("MATÉRIEL"); QCOMPARE(model->rowCount(),2);
        search->clear(); category->setCurrentText("Chats"); QCOMPARE(model->rowCount(),4);
        search->setText("vaccin"); QCOMPARE(model->rowCount(),1);
        QTemporaryDir folder; QString error;
        QVERIFY2(stock.exportInventoryPdf(folder.filePath("stock.pdf"),&error),qPrintable(error));
        QPdfDocument pdf; QCOMPARE(pdf.load(folder.filePath("stock.pdf")),QPdfDocument::Error::None);
        QVERIFY(pdf.getAllText(0).text().contains("Typhus")); QVERIFY(!pdf.getAllText(0).text().contains("Parvovirose"));
        search->setText("inexistant"); QCOMPARE(model->rowCount(),0);
        search->clear(); category->setCurrentIndex(0);
        for(int mode:{1,2,3,4}) {
            sort->setCurrentIndex(mode);
            for(int row=1;row<model->rowCount();++row) {
                if(mode<=2) {
                    int a=model->index(row-1,2).data().toString().section(' ',0,0).toInt();
                    int b=model->index(row,2).data().toString().section(' ',0,0).toInt();
                    QVERIFY(mode==1?a<=b:a>=b);
                } else {
                    auto a=QDate::fromString(model->index(row-1,4).data().toString(),"dd/MM/yyyy");
                    auto b=QDate::fromString(model->index(row,4).data().toString(),"dd/MM/yyyy");
                    QVERIFY(mode==3?a<=b:a>=b);
                }
            }
        }
        sort->setCurrentIndex(0);
        const int before=model->index(0,2).data().toString().section(' ',0,0).toInt();
        bool rejected=false,opened=false;
        QTimer::singleShot(20,[&] {
            auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            opened=true;
            auto *type=dialog->findChild<QComboBox *>("stockMovementType");
            auto *amount=dialog->findChild<QSpinBox *>("stockMovementQuantity");
            auto *save=dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Save);
            type->setCurrentIndex(1); amount->setValue(before+1);
            QTimer::singleShot(20,[&] { if(auto *box=qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) { rejected=box->text().contains("dépasse"); box->accept(); } });
            save->click(); QVERIFY(dialog->isVisible());
            type->setCurrentIndex(0); amount->setValue(5); save->click();
        });
        stock.openMovementDialog(); QVERIFY(opened); QVERIFY(rejected);
        QCOMPARE(model->index(0,2).data().toString().section(' ',0,0).toInt(),before+5);
        stock.refresh(); QCOMPARE(model->index(0,2).data().toString().section(' ',0,0).toInt(),before+5);
        StockWidget reopened;
        QCOMPARE(reopened.findChild<QTableView *>("stockTable")->model()->index(0,2).data().toString().section(' ',0,0).toInt(),before);
    }
    void stockCrudExpiryAndStatistics() {
        StockWidget stock; stock.resize(1050,810); stock.show();
        auto *table=stock.findChild<QTableView *>("stockTable"); auto *model=table->model();
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        // Edit a row after sorting: edits must use the source article, not its visible position.
        stock.findChild<QComboBox *>("stockSortFilter")->setCurrentIndex(2); table->selectRow(0);
        const QString article=model->index(0,0).data().toString(); bool edited=false;
        QTimer::singleShot(20,[&] {
            auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            edited=true; dialog->findChild<QLineEdit *>("stockLot")->setText("LOT-TEST");
            dialog->findChild<QDateEdit *>("stockExpiry")->setDate(QDate::currentDate().addDays(-1));
            if(!output.isEmpty()) dialog->grab().save(output+"/stock-edit.png");
            dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Save)->click();
        });
        stock.findChild<QPushButton *>("stockEditButton")->click(); QVERIFY(edited);
        QCOMPARE(model->index(0,0).data().toString(),article); QCOMPARE(model->index(0,1).data().toString(),QString("LOT-TEST"));
        QCOMPARE(model->index(0,5).data().toString(),QString("Périmé"));
        stock.findChild<QComboBox *>("stockStateFilter")->setCurrentText("Périmé"); QVERIFY(model->rowCount()>=1);
        bool expiryOpened=false;
        QTimer::singleShot(30,[&] {
            auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            auto *lots=dialog->findChild<QTableWidget *>(); expiryOpened=lots && lots->rowCount()>0;
            if(!output.isEmpty()) dialog->grab().save(output+"/stock-expiry.png");
            dialog->reject();
        });
        stock.findChild<QPushButton *>("stockExpiryButton")->click(); QVERIFY(expiryOpened);
        stock.findChild<QComboBox *>("stockStateFilter")->setCurrentIndex(0); table->selectRow(0);
        QTimer::singleShot(20,[] { if(auto *box=qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) box->done(QMessageBox::Yes); });
        stock.findChild<QPushButton *>("stockDeleteButton")->click(); QCOMPARE(model->rowCount(),18);
        bool stats=false;
        QTimer::singleShot(30,[&] { if(auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
            stats=dialog->objectName()=="stockStatisticsDialog";
            if(!output.isEmpty()) dialog->grab().save(output+"/stock-statistics.png"); dialog->reject();
        } });
        stock.findChild<QPushButton *>("stockStatisticsButton")->click(); QVERIFY(stats);
    }
'''+s[b:]
s=s.replace('stack->count(),4','stack->count(),5').replace('("pageStack")->count(),4','("pageStack")->count(),5')
s=s.replace('"stockFefoButton","stockReorderButton"','"stockEditButton","stockExpiryButton"')
s=s.replace('page<4','page<5')
s=s.replace('QString("employes"),QString("admin")','QString("employes"),QString("locaux"),QString("admin")')
s=s.replace('QString("services"),QString("admin"),QString("stock")','QString("services"),QString("locaux"),QString("admin"),QString("stock")')
# Construct the actual pre-integration CHECK constraint after changing a password.
s=s.replace('        query.finish();\n        QVERIFY(manager.initialize(path));', '''        query.finish();
        QVERIFY(query.exec("ALTER TABLE accounts RENAME TO accounts_new_fixture"));
        QVERIFY(query.exec("CREATE TABLE accounts(username TEXT PRIMARY KEY COLLATE NOCASE, role TEXT NOT NULL CHECK(role IN ('admin','services','stock','employes')), salt BLOB NOT NULL, password_hash BLOB NOT NULL, iterations INTEGER NOT NULL)"));
        QVERIFY(query.exec("INSERT INTO accounts SELECT * FROM accounts_new_fixture WHERE role!='locaux'"));
        QVERIFY(query.exec("DROP TABLE accounts_new_fixture"));
        QVERIFY(manager.initialize(path));
        QCOMPARE(Accounts::authenticate("locaux","locaux123"),Accounts::Role::Locaux);
        QVERIFY(manager.initialize(path));''')
pos=s.index('    void moduleAccountsAndPersistence()')
s=s[:pos]+'''    void locauxPersistenceAndBusinessRules() {
        QTemporaryDir folder; const QString path=folder.filePath("locaux.json");
        LocalRepository repo(nullptr,path); QVERIFY2(repo.lastError().isEmpty(),qPrintable(repo.lastError()));
        QVERIFY(!repo.all().isEmpty()); const int initialCount=repo.all().size();
        Local room; room.name="Test salle"; room.type="Consultation"; room.capacity=2;
        const int id=repo.add(room); QVERIFY(id>0); room=*repo.find(id); room.occupancy=1; room.markToClean();
        QVERIFY(repo.update(room));
        LocalRepository reopened(nullptr,path); QCOMPARE(reopened.all().size(),initialCount+1);
        QCOMPARE(reopened.find(id)->occupancy,1); QCOMPARE(reopened.find(id)->cleaning,CleaningState::ToClean);
        room.startCleaning(); QVERIFY(reopened.update(room)); room.markClean(); QVERIFY(reopened.update(room));
        QVERIFY(reopened.find(id)->lastCleaned.isValid());
        room.occupancy=3; QVERIFY(!reopened.update(room)); QCOMPARE(reopened.find(id)->occupancy,1);
        const auto all=reopened.all(); for(const auto &l:all) QVERIFY(reopened.remove(l.id));
        LocalRepository empty(nullptr,path); QVERIFY(empty.all().isEmpty()); // deleting the last room stays deleted
        QFile corrupt(folder.filePath("corrupt.json")); QVERIFY(corrupt.open(QIODevice::WriteOnly)); corrupt.write("invalid JSON"); corrupt.close();
        LocalRepository bad(nullptr,corrupt.fileName()); QVERIFY(!bad.lastError().isEmpty());
        room.occupancy=0; QCOMPARE(bad.add(room),0); QVERIFY(corrupt.open(QIODevice::ReadOnly)); QCOMPARE(corrupt.readAll(),QByteArray("invalid JSON")); corrupt.close();
        // Failed writes do not publish changes to the UI or retain phantom rooms.
        QFile blocker(folder.filePath("blocker")); QVERIFY(blocker.open(QIODevice::WriteOnly)); blocker.write("file"); blocker.close();
        LocalRepository unwritable(nullptr,blocker.fileName()+"/locaux.json"); const int count=unwritable.all().size();
        QSignalSpy changed(&unwritable,&LocalRepository::changed); QCOMPARE(unwritable.add(room),0); QCOMPARE(unwritable.all().size(),count); QCOMPARE(changed.count(),0);
        LocalRepository demo; const auto query=SmartSearch::parse("chirurgie dispo"); int matched=0;
        for(const auto &l:demo.all()) if(query.matches(l,0)) { QCOMPARE(l.type,QString("Chirurgie")); QCOMPARE(l.state(),RoomState::Available); ++matched; }
        QVERIFY(matched>0);
        AssignQuery assign; assign.species="Chien"; assign.need="Chirurgie";
        const auto suggestions=SmartAssign::suggest(demo.all(),assign); QVERIFY(!suggestions.isEmpty());
        for(const auto &suggestion:suggestions) { auto *l=demo.find(suggestion.localId); QVERIFY(l); QVERIFY(!l->outOfService); QVERIFY(l->occupancy<l->capacity); QCOMPARE(l->cleaning,CleaningState::Clean); }
        QString error; const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        const QString pdfPath=output.isEmpty()?folder.filePath("nettoyage.pdf"):output+"/locaux-nettoyage.pdf";
        QVERIFY2(PdfReport::generate(demo.all(),pdfPath,&error),qPrintable(error));
        QPdfDocument pdf; QCOMPARE(pdf.load(pdfPath),QPdfDocument::Error::None); QVERIFY(pdf.pageCount()>0); QVERIFY(pdf.getAllText(0).text().contains("nettoyage",Qt::CaseInsensitive));
        QVERIFY(!PdfReport::generate(demo.all(),blocker.fileName()+"/bad.pdf",&error));
    }
    void locauxNavigationCrudAndScreens() {
        QVERIFY(DatabaseManager::instance().initialize(":memory:"));
        MainWindow main; main.setSession("locaux",Accounts::Role::Locaux); main.show();
        auto *page=main.findChild<PageLocaux *>(); QVERIFY(page); QVERIFY(!page->isWindow());
        auto *repo=main.findChild<LocalRepository *>(); QVERIFY(repo); const int before=repo->all().size();
        QCOMPARE(main.findChild<QStackedWidget *>("pageStack")->currentIndex(),4);
        bool added=false; const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        QTimer::singleShot(40,[&] {
            auto *dialog=qobject_cast<LocalDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            dialog->findChild<QLineEdit *>("localName")->setText("Consultation intégration");
            if(!output.isEmpty()) dialog->grab().save(output+"/locaux-add.png");
            added=true; dialog->findChild<QPushButton *>("saveLocal")->click();
        });
        page->findChild<QPushButton *>("locauxAddButton")->click(); QVERIFY(added); QCOMPARE(repo->all().size(),before+1);
        page->findChild<QLineEdit *>("locauxSearch")->clear();
        for(const QSize size:{QSize(1400,900),QSize(1100,700)}) {
            main.resize(size); QTest::qWait(450); QCOMPARE(main.size(),size);
            for(const auto &name:{"locauxAddButton","locauxPdfButton","locauxMapButton","locauxStatisticsButton","locauxAssignButton","locauxAlertsButton"}) {
                auto *button=page->findChild<QPushButton *>(name); QVERIFY(button); QVERIFY(button->isVisible());
                QVERIFY(page->rect().contains(QRect(button->mapTo(page,QPoint()),button->size())));
            }
            if(!output.isEmpty()) QVERIFY(main.grab().save(output+(size.width()==1400?"/locaux-integrated.png":"/locaux-small.png")));
        }
        main.resize(1400,900);
        page->findChild<QPushButton *>("locauxListButton")->click();
        auto *search=page->findChild<QLineEdit *>("locauxSearch"); search->setText("Consultation intégration");
        QCOMPARE(page->findChild<QTableWidget *>("locauxTable")->rowCount(),1);
        bool stats=false;
        QTimer::singleShot(80,[&] { if(auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
            stats=true; if(!output.isEmpty()) dialog->grab().save(output+"/locaux-statistics.png"); dialog->reject();
        } });
        page->findChild<QPushButton *>("locauxStatisticsButton")->click(); QVERIFY(stats);
        page->findChild<QPushButton *>("locauxMapButton")->click(); QTest::qWait(40);
        auto *plan=page->findChild<PlanDialog *>(); QVERIFY(plan); QVERIFY(plan->isVisible());
        if(!output.isEmpty()) plan->grab().save(output+"/locaux-plan.png");
        main.setSession("stock",Accounts::Role::Stock); QVERIFY(!plan->isVisible());
        main.navigate(4); QCOMPARE(main.findChild<QStackedWidget *>("pageStack")->currentIndex(),2);
    }
'''+s[pos:]
p.write_text(s,encoding='utf-8')
p=Path('src/locaux/localdialog.cpp');s=p.read_text(encoding='utf-8').replace('m_name = new QLineEdit(m_base.name);','m_name = new QLineEdit(m_base.name); m_name->setObjectName("localName");').replace('    save->setDefault(true);','    save->setDefault(true); save->setObjectName("saveLocal");');p.write_text(s,encoding='utf-8')
print('Integration tests updated')
