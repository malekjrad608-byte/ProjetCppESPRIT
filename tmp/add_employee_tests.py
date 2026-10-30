from pathlib import Path
p=Path('tests/service_tests.cpp');s=p.read_text(encoding='utf-8-sig')
s=s.replace('#include <QSortFilterProxyModel>','#include <QSortFilterProxyModel>\n#include <QDialogButtonBox>\n#include <QDoubleSpinBox>\n#include "../src/employees/gestionemployes.h"\n#include "../src/employees/employeestore.h"\n#include "../src/auth/accounts.h"')
a=s.index('    void renderScreens()')
s=s[:a]+r'''
    void moduleAccountsAndPersistence() {
        QTemporaryDir folder; const QString path=folder.filePath("accounts.sqlite");
        auto &manager=DatabaseManager::instance(); QVERIFY(manager.initialize(path));
        QString error;
        auto database=QSqlDatabase::database("aniworld-services");
        QVERIFY2(EmployeeStore::importInitial(database,&error),qPrintable(error));
        QVERIFY(EmployeeStore::importInitial(database,&error));
        QSqlQuery query(database); QVERIFY(query.exec("SELECT COUNT(*) FROM employes")); QVERIFY(query.next()); QCOMPARE(query.value(0).toInt(),6);
        QVERIFY(query.exec("UPDATE employes SET prenom='Conservé' WHERE id=1"));
        for(const QString &name:{QString("services"),QString("stock"),QString("employes"),QString("admin")}) {
            QVERIFY(Accounts::authenticate(name,name+"123")!=Accounts::Role::None);
            QCOMPARE(Accounts::authenticate(name,"wrongpass"),Accounts::Role::None);
        }
        QCOMPARE(Accounts::authenticate("unknown","services123"),Accounts::Role::None);
        QCOMPARE(Accounts::authenticate("STOCK","stock123"),Accounts::Role::Stock);
        QVERIFY(!Accounts::changePassword("stock","wrong","newStock2026",&error));
        QVERIFY(Accounts::changePassword("stock","stock123","newStock2026",&error));
        QVERIFY(query.exec("SELECT password_hash,salt FROM accounts WHERE username='stock'")); QVERIFY(query.next());
        QCOMPARE(query.value(0).toByteArray().size(),32); QCOMPARE(query.value(1).toByteArray().size(),24);
        query.finish();
        QVERIFY(manager.initialize(path)); QVERIFY(EmployeeStore::importInitial(database,&error));
        QCOMPARE(Accounts::authenticate("stock","stock123"),Accounts::Role::None);
        QCOMPARE(Accounts::authenticate("stock","newStock2026"),Accounts::Role::Stock);
        QVERIFY(query.exec("SELECT prenom FROM employes WHERE id=1")); QVERIFY(query.next()); QCOMPARE(query.value(0).toString(),QString("Conservé"));
        QVERIFY(query.exec("DELETE FROM employes")); QVERIFY(EmployeeStore::importInitial(database,&error));
        QVERIFY(query.exec("SELECT COUNT(*) FROM employes")); QVERIFY(query.next()); QCOMPARE(query.value(0).toInt(),0);
        QVERIFY(manager.initialize(":memory:"));
    }
    void moduleLoginRoutingAndIsolation() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        QString error; QVERIFY(EmployeeStore::importInitial(QSqlDatabase::database("aniworld-services"),&error));
        AppController controller(false,false); controller.start(); auto *login=controller.loginWindow();
        Motion::instance().setEnabled(false);
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR"); if(!output.isEmpty()) QDir().mkpath(output);
        for(const QString &account:{QString("employes"),QString("stock"),QString("services"),QString("admin"),QString("stock")}) {
            login->findChild<QLineEdit *>("loginUsername")->setText(account);
            login->findChild<QLineEdit *>("loginPassword")->setText(account+"123");
            login->findChild<QPushButton *>("signInButton")->click();
            QVERIFY(controller.isSignedIn()); auto *main=controller.mainWindow(); main->resize(1400,900);
            const auto role=Accounts::authenticate(account,account+"123"); QCOMPARE(main->sessionRole(),role);
            auto *stack=main->findChild<QStackedWidget *>("pageStack"); QCOMPARE(stack->currentIndex(),Accounts::homePage(role));
            for(int page=0;page<4;++page) {
                const int before=stack->currentIndex(); main->navigate(page);
                if(Accounts::allows(role,page)) QCOMPARE(stack->currentIndex(),page);
                else { QCOMPARE(stack->currentIndex(),before); QVERIFY(!stack->widget(page)->isEnabled()); }
            }
            main->navigate(Accounts::homePage(role));
            QTest::qWait(70);
            if(!output.isEmpty()) QVERIFY(main->grab().save(output+"/account-"+account+".png"));
            main->findChild<QPushButton *>("logoutButton")->click();
            QVERIFY(!controller.isSignedIn()); QCOMPARE(main->sessionRole(),Accounts::Role::None);
            QVERIFY(login->findChild<QLineEdit *>("loginPassword")->text().isEmpty());
        }
        login->findChild<QLineEdit *>("loginUsername")->setText("employes");
        login->findChild<QLineEdit *>("loginPassword")->setText("stock123"); login->findChild<QPushButton *>("signInButton")->click();
        QVERIFY(!controller.isSignedIn());
        login->findChild<QPushButton *>("forgot")->click(); QCOMPARE(login->findChild<QStackedWidget *>("pages")->currentIndex(),0);
        Motion::instance().setEnabled(true);
    }
    void integratedEmployeeCrudPlanningAndScreens() {
        auto &manager=DatabaseManager::instance(); QVERIFY(manager.initialize(":memory:"));
        auto database=QSqlDatabase::database("aniworld-services"); QString error;
        QVERIFY(EmployeeStore::importInitial(database,&error));
        MainWindow main; main.resize(1400,900); main.setSession("employes",Accounts::Role::Employees); main.show();
        auto *employees=main.findChild<GestionEmployes *>(); QVERIFY(employees);
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        bool formSeen=false;
        QTimer automation; automation.setInterval(30);
        connect(&automation,&QTimer::timeout,&main,[&] {
            if(auto *message=qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) { message->done(message->standardButtons().testFlag(QMessageBox::Yes)?QMessageBox::Yes:QMessageBox::Ok); return; }
            auto *form=qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if(form && form->objectName()=="employeeForm") {
                auto *last=form->findChild<QLineEdit *>("employeeLastName"); auto *first=form->findChild<QLineEdit *>("employeeFirstName");
                last->setText("Integration"); first->setText(formSeen?"Modifié":"Test");
                form->findChild<QLineEdit *>("employeeSkills")->setText("UniqueSkill");
                form->findChild<QDoubleSpinBox *>("employeeWorkload")->setValue(0);
                if(!output.isEmpty()) form->grab().save(output+"/employee-form.png");
                formSeen=true; form->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
            }
        });
        automation.start(); employees->findChild<QPushButton *>("addEmployee")->click(); QVERIFY(formSeen);
        QSqlQuery q(database); QVERIFY(q.exec("SELECT id,prenom FROM employes WHERE nom='Integration'")); QVERIFY(q.next()); const int id=q.value(0).toInt(); QCOMPARE(q.value(1).toString(),QString("Test"));
        auto select=[&] {
            employees->findChild<QLineEdit *>("employeeSearch")->setText("Integration"); QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
            for(auto *card:employees->findChildren<QFrame *>()) if(card->property("employe_id").toInt()==id && card->isVisible()) { QTest::mouseClick(card,Qt::LeftButton); return true; }
            return false;
        };
        QVERIFY(select()); employees->findChild<QPushButton *>("editEmployee")->click();
        QVERIFY(employees->findChild<QLabel *>("employeeDetailName")->text().contains("Modifié"));
        employees->findChild<QPushButton *>("toggleEmployeeAvailability")->click();
        QVERIFY(q.exec("SELECT disponible FROM employes WHERE nom='Integration'")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),0);
        employees->findChild<QPushButton *>("toggleEmployeeAvailability")->click();
        automation.stop();
        // A unique specialist gets the first task; a conflicting second task cannot be assigned.
        bool taskForm=false; QString outcome;
        QTimer planning; planning.setInterval(30);
        connect(&planning,&QTimer::timeout,&main,[&] {
            if(auto *message=qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) { outcome=message->text(); message->accept(); return; }
            auto *form=qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if(form && form->objectName()=="employeePlanningDialog") {
                taskForm=true; form->findChild<QLineEdit *>("employeeTaskTitle")->setText("Test de planning");
                form->findChild<QLineEdit *>("employeeTaskSkill")->setText("UniqueSkill");
                form->findChild<QDateTimeEdit *>("employeeTaskStart")->setDateTime(QDateTime(QDate(2030,1,7),QTime(10,0)));
                form->findChild<QDateTimeEdit *>("employeeTaskEnd")->setDateTime(QDateTime(QDate(2030,1,7),QTime(11,0)));
                if(!output.isEmpty()) form->grab().save(output+"/employee-planning.png");
                form->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
            }
        });
        planning.start(); QVERIFY(QMetaObject::invokeMethod(employees,"ouvrirPlanification",Qt::DirectConnection));
        QVERIFY(taskForm); QVERIFY(outcome.contains("enregistrée"));
        QVERIFY(QMetaObject::invokeMethod(employees,"ouvrirPlanification",Qt::DirectConnection));
        QVERIFY(outcome.contains("Aucune personne disponible")); planning.stop();
        QVERIFY(q.exec("SELECT COUNT(*) FROM planning_affectations")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),1);
        automation.start(); employees->findChild<QPushButton *>("deleteEmployee")->click(); automation.stop();
        QVERIFY(q.exec("SELECT COUNT(*) FROM planning_affectations")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),0);
        employees->findChild<QLineEdit *>("employeeSearch")->clear(); QTest::qWait(200);
        auto cards=employees->findChildren<QFrame *>("cardEmploye"); QVERIFY(!cards.isEmpty()); QTest::mouseClick(cards.first(),Qt::LeftButton); QTest::qWait(200);
        if(!output.isEmpty()) { QVERIFY(main.grab().save(output+"/employees-integrated.png")); main.resize(1100,700); QTest::qWait(100); QVERIFY(main.grab().save(output+"/employees-small.png")); }
        bool stats=false;
        QTimer::singleShot(100,[&] { auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            auto *table=dialog->findChild<QTableWidget *>("employeeLoadTable"); stats=table && table->rowCount()==6;
            if(!output.isEmpty()) dialog->grab().save(output+"/employee-statistics.png"); dialog->reject(); });
        employees->findChild<QPushButton *>("employeeStatistics")->click(); QVERIFY(stats);
    }
''' + s[a:]
p.write_text(s,encoding='utf-8')
