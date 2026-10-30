#include "../src/employees/employe.h"
#include "../src/locaux/localrepository.h"
#include "../src/database/oracleconnection.h"
#include "../src/database/connection.h"
#include "../src/stock/stockrecord.h"
#include <QSqlError>
#include <QSqlQueryModel>
#include "../src/models/employeequeries.h"
#include <QStandardPaths>
#include "../src/locaux/pagelocaux.h"
#include "../src/locaux/localdialog.h"
#include "../src/locaux/plandialog.h"
#include "../src/locaux/pdfreport.h"
#include "../src/locaux/smartassign.h"
#include "../src/locaux/smartsearch.h"
#include <QDateEdit>
#include "../src/medical/medicalrecord.h"
#include "../src/medical/medicalwidget.h"
#include "../src/medical/medicalrecorddialog.h"
#include "../src/medical/medicalrecorddetailsdialog.h"
#include "../src/medical/recordcard.h"
#include <QScopeGuard>
#include <QtTest>
#include <QTemporaryDir>
#include <QTimer>
#include <QMessageBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QFontDatabase>
#include <QEnterEvent>
#include <QFocusEvent>
#include <QScreen>
#include <QCheckBox>
#include <QSettings>
#include "../src/appcontroller.h"
#include "../src/login/loginwindow.h"
#include "../src/stock/stockwidget.h"
#include "../src/ui/invoice.h"
#include <QSqlQuery>
#include <QTextBrowser>
#include <QPdfDocument>
#include <QFileDialog>
#include <QTableView>
#include <QSpinBox>
#include <QSortFilterProxyModel>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QPainter>
#include "../src/employees/gestionemployes.h"
#include "../src/auth/accounts.h"
#include "../src/database/databasemanager.h"
#include "../src/mainwindow.h"
#include "../src/dialogs/servicedialog.h"
#include "../src/dialogs/serviceplanningdialog.h"
#include "../src/models/service.h"
#include "../src/ui/design.h"
#include "../src/ui/servicereport.h"
#include "../src/models/serviceanalytics.h"
#include "../src/widgets/servicecard.h"
#include "../src/widgets/animatedlogo.h"
#include "../src/ui/motion.h"

namespace {
ServiceDialog *visibleServiceForm() {
    if(auto *modal=qobject_cast<ServiceDialog *>(QApplication::activeModalWidget()))return modal;
    for(auto *widget:QApplication::allWidgets())
        if(auto *form=qobject_cast<ServiceDialog *>(widget);form && form->isVisible())return form;
    return nullptr;
}
bool openMedicalService(MainWindow &window) {
    QString error;
    auto records=MedicalRecord::afficher(&error);
    if(!error.isEmpty())return false;
    if(records.isEmpty() && !OracleConnection::isOracle()){
        auto record=MedicalRecord::examples().first();
        record.storageId=0;
        if(!record.ajouter(&error))return false;
        records=MedicalRecord::afficher(&error);
    }
    if(records.isEmpty())return false;
    window.navigate(1);
    if(!visibleServiceForm())window.findChild<ServiceWidget *>()->onAdd();
    if(auto *form=visibleServiceForm()) {
        auto *owners=form->findChild<QComboBox *>("serviceClientChoice");
        if(owners && owners->count()>1) owners->setCurrentIndex(1);
    }
    return visibleServiceForm()!=nullptr;
}
}
class ServiceTests : public QObject {
    Q_OBJECT
private slots:
    void serviceCreatesFromMedicalOwner() {
        auto &db=DatabaseManager::instance();QVERIFY(db.initialize(":memory:"));
        QString error;
        auto record=MedicalRecord::examples().first();record.storageId=0;
        record.animal.owner="Propriétaire sélectionné";QVERIFY(record.ajouter(&error));
        MainWindow window;window.resize(1400,900);window.setSession("admin",Accounts::Role::Admin);window.show();
        QVERIFY(openMedicalService(window));
        auto *cancelled=visibleServiceForm();QVERIFY(cancelled);
        cancelled->findChild<QPushButton *>("cancelService")->click();
        QCOMPARE(window.findChild<QStackedWidget *>("pageStack")->currentIndex(),1);
        QCOMPARE(db.getAllServices().size(),0);QCOMPARE(MedicalRecord::afficher(&error).size(),1);
        record.animal.owner="Propriétaire sélectionné après actualisation";QVERIFY(record.modifier(&error));
        QVERIFY(openMedicalService(window));
        auto *form=visibleServiceForm();QVERIFY(form);QVERIFY(!form->isWindow());
        auto *owner=form->findChild<QLineEdit *>("serviceClient");
        QVERIFY(owner->isReadOnly());QCOMPARE(owner->text(),record.animal.owner);
        auto *categories=form->findChild<QComboBox *>("serviceCategory");
        QCOMPARE(categories->maxVisibleItems(),5);QVERIFY(categories->count()>5);
        QCOMPARE(categories->findText("Pension & garde"),-1);
        categories->setCurrentText("Vaccination");
        const auto output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(!output.isEmpty()){QDir().mkpath(output);window.grab().save(output+"/medical-service-form.png");}
        categories->showPopup();QTest::qWait(50);
        QVERIFY(categories->view()->verticalScrollBar()->maximum()>0);
        if(!output.isEmpty())categories->view()->window()->grab().save(output+"/service-category-scroll.png");
        categories->hidePopup();
        // The model rereads the owner from the selected record, even if the UI value is stale.
        record.animal.owner="Propriétaire actualisé";QVERIFY(record.modifier(&error));
        form->findChild<QPushButton *>("saveService")->click();
        QTRY_COMPARE(db.getAllServices().size(),1);
        QCOMPARE(db.getAllServices().first().getClientName(),record.animal.owner);
        QCOMPARE(db.getAllServices().first().getCategory(),QString("Vaccination"));
        QVERIFY(!visibleServiceForm());QCOMPARE(window.findChild<QStackedWidget *>("pageStack")->currentIndex(),1);
        QVERIFY(openMedicalService(window));form=visibleServiceForm();
        QVERIFY(MedicalRecord::supprimer(record.storageId,&error));
        form->findChild<QPushButton *>("saveService")->click();
        QVERIFY(form->isVisible());QVERIFY(form->findChild<QLabel *>("validationError")->text().contains("n'existe plus"));
        QCOMPARE(db.getAllServices().size(),1);form->reject();
    }
    void atelierModelCrudAndValidation() {
        auto &manager=DatabaseManager::instance();QVERIFY(manager.initialize(":memory:"));
        QCOMPARE(&manager,&DatabaseManager::instance());
        QString error;
        Service service(0,"Soin d'Oscar'); DROP TABLE services; --","Client test",
                        "Soins & bien-être",QDateTime::currentDateTime(),"Planifié",30,0);
        QVERIFY2(service.ajouter(),qPrintable(service.lastError()));QVERIFY(service.getId()>0);
        QScopedPointer<QSqlQueryModel> model(Service().afficher());
        QVERIFY(!model->lastError().isValid());QCOMPARE(model->rowCount(),1);QCOMPARE(model->data(model->index(0,1)).toString(),service.getName());
        service.setName("Soin modifié");QVERIFY(service.modifier(&error));QCOMPARE(Service::afficherListe(&error).first().getName(),service.getName());
        service.setDuration(0);QVERIFY(!service.modifier(&error));QCOMPARE(Service::afficherListe(&error).first().getDuration(),30);
        Employe employee;employee.setNom("O'Brien");employee.setPrenom("Test");employee.setDateEmbauche(QDate::currentDate());employee.setCourriel("test@example.test");
        QVERIFY2(employee.ajouter(&error),qPrintable(error));QVERIFY(employee.getId()>0);
        QCOMPARE(Employe::afficher("O'Brien","date","Tous les postes","Tous",&error).size(),1);
        QCOMPARE(Employe::afficher("' OR '1'='1","date","Tous les postes","Tous",&error).size(),0);
        employee.setCourriel("incorrect");QVERIFY(!employee.modifier(&error));employee.setCourriel("valid@example.test");employee.setPrenom("Modifié");QVERIFY(employee.modifier(&error));
        QVERIFY(Employe::basculerDisponibilite(employee.getId(),&error));QVERIFY(!Employe::afficher().first().isDisponible());
        QVERIFY(Employe::supprimer(employee.getId(),&error));QVERIFY(!Employe::supprimer(employee.getId(),&error));
        QVERIFY(service.supprimer(service.getId()));QVERIFY(Service::afficherListe(&error).isEmpty());
        qint64 revision=0;StockRecord stock;stock.article="Test";stock.lot="Test";stock.quantity=-1;
        QVERIFY(!StockRecord::enregistrer({stock},&revision,&error));
        Local local;local.name="Test";local.capacity=0;QVERIFY(!Local::enregistrer({local},&revision,&error));
        QCOMPARE(QSqlDatabase::connectionNames().count("aniworld-services"),1);
    }
    void atelierEmbeddedFormKeepsInputOnError() {
        auto &manager=DatabaseManager::instance();QVERIFY(manager.initialize(":memory:"));
        MainWindow page;page.resize(1280,800);page.show();QVERIFY(openMedicalService(page));QTest::qWait(50);
        auto *form=visibleServiceForm();QVERIFY(form);QVERIFY(!form->isWindow());QVERIFY(!QApplication::activeModalWidget());
        form->findChild<QLineEdit *>("serviceName")->clear();form->findChild<QPushButton *>("saveService")->click();QVERIFY(form->findChild<QLabel *>("validationError")->isVisible());
        form->findChild<QLineEdit *>("serviceName")->setText("Service conservé dans le formulaire");
        form->findChild<QLineEdit *>("serviceClient")->setText("Client");
        QSqlQuery reject(OracleConnection::database());
        QVERIFY(reject.exec("CREATE TRIGGER test_reject_service BEFORE INSERT ON services BEGIN SELECT RAISE(ABORT,'Test: stockage indisponible'); END"));
        form->findChild<QPushButton *>("saveService")->click();
        QTest::qWait(250);
        QVERIFY(form->isVisible());
        QCOMPARE(form->findChild<QLineEdit *>("serviceName")->text(),QString("Service conservé dans le formulaire"));
        QVERIFY(form->findChild<QLabel *>("validationError")->text().contains("stockage indisponible"));
        QCOMPARE(manager.getAllServices().size(),0);
        QCOMPARE(page.findChild<QTableView *>("serviceTable")->model()->rowCount(),0);
        QVERIFY(reject.exec("DROP TRIGGER test_reject_service"));
        const auto capture=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");if(!capture.isEmpty())page.grab().save(capture+"/atelier-service-form.png");
        form->findChild<QPushButton *>("saveService")->click();QCOMPARE(manager.getAllServices().size(),1);
        QCOMPARE(page.findChild<QTableView *>("serviceTable")->model()->rowCount(),1);
        QCOMPARE(page.findChild<QStackedWidget *>("servicePages")->currentIndex(),0);
    }
    void oracleFiveTablesScreens() {
        if(qEnvironmentVariable("ANIWORLD_FIVE_TESTS")!="1")QSKIP("Five-table Oracle UI opt-in.");
        const auto appName=qApp->applicationName(),org=qApp->organizationName();qApp->setApplicationName("ANIWORLD");qApp->setOrganizationName("ANIWORLD");
        auto &manager=DatabaseManager::instance();const auto restore=qScopeGuard([&]{manager.initialize(":memory:");qApp->setApplicationName(appName);qApp->setOrganizationName(org);});
        QVERIFY2(manager.initializeOracle(),qPrintable(manager.lastError()));Motion::instance().setEnabled(false);
        QStringList errors;QTimer watcher;connect(&watcher,&QTimer::timeout,this,[&]{if(auto *box=qobject_cast<QMessageBox *>(QApplication::activeModalWidget())){errors<<box->text();box->accept();}});watcher.start(30);
        MainWindow window(false);window.setSession("admin",Accounts::Role::Admin);window.resize(1280,800);window.show();
        const auto output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        for(int page=0;page<=5;++page){window.navigate(page);QTest::qWait(120);QVERIFY2(errors.isEmpty(),qPrintable(errors.join('\n')));if(!output.isEmpty())window.grab().save(output+QString("/five-module-%1.png").arg(page));}
        window.navigate(1);window.findChild<ServiceWidget *>()->onAdd();QCOMPARE(window.findChild<QStackedWidget *>("pageStack")->currentIndex(),1);QVERIFY(openMedicalService(window));QTest::qWait(250);
        auto *embedded=visibleServiceForm();QVERIFY(embedded);QVERIFY(!embedded->isWindow());QVERIFY(!QApplication::activeModalWidget());
        auto *reference=embedded->findChild<QComboBox *>("serviceClientChoice");QVERIFY(reference);QVERIFY(!reference->isEditable());
        QCOMPARE(embedded->getService().getName(),reference->currentText());
        QVERIFY(embedded->findChild<QLineEdit *>("serviceId")->isReadOnly());
        QVERIFY(embedded->findChild<QComboBox *>("serviceEmployee")->isHidden());
        QVERIFY(embedded->findChild<QComboBox *>("serviceLocal")->isHidden());
        QVERIFY(embedded->findChild<QLineEdit *>("serviceClient")->isHidden());
        auto *serviceTable=window.findChild<QTableView *>("serviceTable");
        for(int col:{0,1,2,4,5,6,7,8})QVERIFY(!serviceTable->isColumnHidden(col));
        for(int col:{3,9,10,11,12,13,14})QVERIFY(serviceTable->isColumnHidden(col));
        if(!output.isEmpty())window.grab().save(output+"/atelier-oracle-form.png");
        embedded->findChild<QPushButton *>("cancelService")->click();QTest::qWait(30);
        QCOMPARE(window.findChild<QStackedWidget *>("pageStack")->currentIndex(),1);
        QCOMPARE(window.findChild<QStackedWidget *>("servicePages")->currentIndex(),0);
        LocalRepository repo(nullptr,"oracle:");LocalDialog local(nullptr,&repo);local.show();QTest::qWait(30);auto *service=local.findChild<QComboBox *>("localService");QVERIFY(service);QVERIFY(service->isVisible());QVERIFY(service->count()>1);
        QVERIFY(local.findChild<QLineEdit *>("localId")->isReadOnly());QVERIFY(local.findChild<QComboBox *>("localState")->isVisible());
        if(!output.isEmpty())local.grab().save(output+"/oracle-local-form.png");local.hide();
        MedicalRecordDialog medical;medical.show();QTest::qWait(30);auto *employee=medical.findChild<QComboBox *>("medicalEmployee");QVERIFY(employee);QVERIFY(employee->isVisible());QVERIFY(employee->count()>1);QVERIFY(medical.findChild<QTextEdit *>("medicalTreatment")->isHidden());
        if(!output.isEmpty())medical.grab().save(output+"/oracle-medical-form.png");medical.hide();
        window.navigate(3);bool employeeFields=false;
        QTimer::singleShot(100,this,[&]{auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget());if(!dialog)return;
            auto *id=dialog->findChild<QLineEdit *>("employeeId");auto *cin=dialog->findChild<QLineEdit *>("employeeCin");
            employeeFields=id && id->isReadOnly() && cin && dialog->findChild<QComboBox *>("employeeService");
            if(!output.isEmpty())dialog->grab().save(output+"/oracle-employee-form.png");dialog->reject();});
        window.findChild<QPushButton *>("addEmployee")->click();QVERIFY(employeeFields);
        window.navigate(2);QCOMPARE(window.findChild<QTableView *>("stockTable")->model()->columnCount(),9);
        QCOMPARE(window.findChild<QTableWidget *>("locauxTable")->columnCount(),8);
        QVERIFY2(errors.isEmpty(),qPrintable(errors.join('\n')));
        Motion::instance().setEnabled(true);
    }

    void oracleFiveTablesIntegration() {
        if(qEnvironmentVariable("ANIWORLD_FIVE_TESTS")!="1")QSKIP("Five-table Oracle tests opt-in.");
        const auto appName=qApp->applicationName(),org=qApp->organizationName();qApp->setApplicationName("ANIWORLD");qApp->setOrganizationName("ANIWORLD");
        auto &manager=DatabaseManager::instance();
        const auto restore=qScopeGuard([&]{manager.initialize(":memory:");qApp->setApplicationName(appName);qApp->setOrganizationName(org);});
        QVERIFY2(manager.initializeOracle(),qPrintable(manager.lastError()));auto db=OracleConnection::database();QSqlQuery q(db);
        QCOMPARE(&Connection::createInstance(), &Connection::createInstance());
        QCOMPARE(db.connectionName(), QString::fromLatin1(QSqlDatabase::defaultConnection));
        QCOMPARE(db.driverName(), QString("QODBC"));
        QVERIFY(!QSqlDatabase::contains("aniworld-services"));
        int openConnections=0;
        for(const auto &name:QSqlDatabase::connectionNames())
            if(QSqlDatabase::database(name,false).isOpen())++openConnections;
        QCOMPARE(openConnections,1);
        QVERIFY(q.exec("SELECT COUNT(*) FROM user_tables"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),5);q.finish();
        QVERIFY(q.exec("SELECT COUNT(*) FROM user_constraints WHERE constraint_type='R' AND status='ENABLED' AND validated='VALIDATED'"));
        QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),4);q.finish();
        QVERIFY(q.exec("SELECT COUNT(*) FROM user_tab_columns WHERE table_name='SERVICES' AND column_name='ID_EMPLOYE'"));
        QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),0);q.finish();
        QVERIFY(q.exec("SELECT COUNT(*) FROM user_objects WHERE status<>'VALID'"));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),0);q.finish();
        QString error;QList<int> services;int localId=0;MedicalRecord record;Employe employee;
        const auto cleanup=qScopeGuard([&]{
            db.rollback();db.transaction();QSqlQuery c(db);
            bool ok=c.exec("DELETE FROM stock WHERE article IN ('MLD-TEST-ARTICLE','MLD-TEST-ARTICLE-2')");
            if(record.storageId)ok=c.exec(QString("DELETE FROM medical WHERE id_doss=%1").arg(record.storageId)) && ok;
            if(localId)ok=c.exec(QString("DELETE FROM local WHERE id_local=%1").arg(localId)) && ok;
            if(employee.getId())ok=c.exec(QString("DELETE FROM employer WHERE id_emp=%1").arg(employee.getId())) && ok;
            for(int id:services)ok=c.exec(QString("DELETE FROM service WHERE id_serv=%1").arg(id)) && ok;
            if(!ok || !db.commit())qFatal("MLD test cleanup failed");
        });
        // Bootstrap a fixture service; user creation itself is exercised through Medical's owner below.
        QVERIFY2(q.exec("INSERT INTO service(ref_service,categorie,statut,date_heur,duree,tarif) VALUES('MLD-TEST-BASE','Consultation','Planifié','2035-01-08T09:00:00',30,10)"),qPrintable(q.lastError().text()));
        QVERIFY(q.exec("SELECT services_seq.CURRVAL FROM dual"));QVERIFY(q.next());int base=q.value(0).toInt();q.finish();services<<base;QVERIFY(base>0);
        employee.setNom("MLD_TEST");employee.setPrenom("Employe");employee.setServiceId(base);
        employee.setCourriel("mld@example.test");employee.setDateEmbauche(QDate(2025,1,1));
        employee.setCin("MLD-TEST-CIN");employee.setDateNaissance(QDate(1995,3,2));employee.setAdresse("Adresse test");employee.setTypeContrat("CDI");employee.setSalaire(1250.5);
        QVERIFY2(employee.ajouter(&error),qPrintable(error));QVERIFY(employee.getId()>0);
        auto duplicateEmployee=employee;duplicateEmployee.setId(0);QVERIFY(!duplicateEmployee.ajouter(&error));QVERIFY(error.contains("CIN"));
        employee.setSalaire(1350);QVERIFY2(employee.modifier(&error),qPrintable(error));
        auto employees=Employe::afficher("MLD_TEST","date","Tous les postes","Tous",&error);QVERIFY2(error.isEmpty(),qPrintable(error));QCOMPARE(employees.size(),1);
        QCOMPARE(employees.first().getCin(),employee.getCin());QCOMPARE(employees.first().getSalaire(),1350.0);QCOMPARE(employees.first().getDateNaissance(),employee.getDateNaissance());
        record=MedicalRecord::examples().first();record.storageId=0;record.medical.employeeId=employee.getId();record.animal.animalId="MLD_TEST_ANIMAL";
        record.animal.owner="MLD Owner d'Oscar";
        QVERIFY2(record.ajouter(&error),qPrintable(error));QVERIFY(record.storageId>0);
        auto duplicateRecord=record;duplicateRecord.storageId=0;QVERIFY(!duplicateRecord.ajouter(&error));
        record.medical.reason="Consultation modifiée";QVERIFY2(record.modifier(&error),qPrintable(error));
        auto medical=MedicalRecord::afficher(&error);QVERIFY2(error.isEmpty(),qPrintable(error));bool found=false;
        for(const auto &r:medical)if(r.storageId==record.storageId){found=true;QCOMPARE(r.animal.species,record.animal.species);}
        QVERIFY(found);
        MedicalRecordDialog editor;editor.setMedicalRecord(record);
        QCOMPARE(editor.findChild<QComboBox *>("medicalAnimalSpecies")->currentText(),record.animal.species);
        QVERIFY(!editor.findChild<QComboBox *>("medicalPayment"));
        MedicalRecordDetailsDialog details;details.setMedicalRecord(record);
        QVERIFY(!details.findChild<QLabel *>("medicalPaymentValue"));
        QVERIFY(q.exec(QString("SELECT espece,race FROM medical WHERE id_doss=%1").arg(record.storageId)));QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(),record.animal.species);QCOMPARE(q.value(1).toString(),record.animal.breed);q.finish();
        Service fromMedical;fromMedical.setMedicalRecordId(record.storageId);fromMedical.setClientName("Valeur ignoree");
        fromMedical.setDateTime(QDateTime(QDate(2035,2,8),QTime(9,0)));
        QVERIFY2(fromMedical.ajouter(),qPrintable(fromMedical.lastError()));services<<fromMedical.getId();
        QCOMPARE(fromMedical.getName(),record.animal.owner);
        auto duplicate=fromMedical;duplicate.setId(0);QVERIFY(!duplicate.ajouter());QVERIFY(duplicate.lastError().contains("unique"));
        fromMedical.setPrice(48.5);QVERIFY2(fromMedical.modifier(&error),qPrintable(error));
        QVERIFY(q.exec(QString("SELECT ref_service,tarif FROM service WHERE id_serv=%1").arg(fromMedical.getId())));QVERIFY(q.next());QCOMPARE(q.value(0).toString(),record.animal.owner);QCOMPARE(q.value(1).toDouble(),48.5);q.finish();
        LocalRepository locals(nullptr,"oracle:");QVERIFY2(locals.lastError().isEmpty(),qPrintable(locals.lastError()));
        Local room;room.name="MLD_TEST local";room.type="Consultation";room.capacity=2;room.serviceId=base;
        localId=locals.add(room);QVERIFY2(localId>0,qPrintable(locals.lastError()));
        room=*locals.find(localId);room.capacity=3;QVERIFY2(locals.update(room),qPrintable(locals.lastError()));
        room.outOfService=true;QVERIFY2(locals.update(room),qPrintable(locals.lastError()));
        locals.reload();QVERIFY(locals.find(localId)->outOfService);
        room=*locals.find(localId);room.outOfService=false;room.occupancy=1;
        QVERIFY2(locals.update(room),qPrintable(locals.lastError()));locals.reload();
        QVERIFY(!locals.find(localId)->outOfService);QCOMPARE(locals.find(localId)->occupancy,1);
        QVERIFY(q.exec(QString("SELECT etat FROM local WHERE id_local=%1").arg(localId)));QVERIFY(q.next());QCOMPARE(q.value(0).toString(),QString("Occupé"));q.finish();
        QVector<StockRecord> stock;qint64 revision;QVERIFY2(StockRecord::afficher(&stock,&revision,&error),qPrintable(error));const auto oldStock=stock;
        StockRecord item;item.article="MLD-TEST-ARTICLE";item.nomArticle="Compresses stériles";item.lot="MLD-TEST-LOT";item.category="Soins";item.quantity=2;item.unit="boîte";item.threshold=1;item.expiry=QDate(2036,1,1);item.localId=localId;stock<<item;
        const auto stale=revision;QVERIFY2(StockRecord::enregistrer(stock,&revision,&error),qPrintable(error));
        auto staleCopy=stale;QVERIFY(!StockRecord::enregistrer(oldStock,&staleCopy,&error));QVERIFY(error.contains("autre session"));
        stock.last().quantity=4;QVERIFY2(StockRecord::enregistrer(stock,&revision,&error),qPrintable(error));
        QVERIFY(q.exec("SELECT id_article,nom_article,quantite_disp FROM stock WHERE article='MLD-TEST-ARTICLE'"));QVERIFY(q.next());QVERIFY(q.value(0).toInt()>0);QCOMPARE(q.value(1).toString(),item.nomArticle);QCOMPARE(q.value(2).toInt(),4);q.finish();
        QVERIFY2(StockRecord::afficher(&stock,&revision,&error),qPrintable(error));
        int stableId=0;for(const auto &r:stock)if(r.article==item.article)stableId=r.idArticle;QVERIFY(stableId>0);
        auto second=item;second.article="MLD-TEST-ARTICLE-2";stock<<second;
        QVERIFY2(StockRecord::enregistrer(stock,&revision,&error),qPrintable(error));
        QVERIFY2(StockRecord::afficher(&stock,&revision,&error),qPrintable(error));
        int sharedLotCount=0;for(auto &r:stock){if(r.lot==item.lot)++sharedLotCount;if(r.article==item.article){QCOMPARE(r.idArticle,stableId);r.nomArticle="Nom modifié";}}
        QCOMPARE(sharedLotCount,2);for(auto &r:stock)if(r.article==second.article)r.lot.clear();
        QVERIFY2(StockRecord::enregistrer(stock,&revision,&error),qPrintable(error));
        QVERIFY2(StockRecord::afficher(&stock,&revision,&error),qPrintable(error));
        for(const auto &r:stock)if(r.article==item.article){QCOMPARE(r.idArticle,stableId);QCOMPARE(r.nomArticle,QString("Nom modifié"));}
        for(const auto &r:stock)if(r.article==second.article)QVERIFY(r.lot.isEmpty());
        QVERIFY(!q.exec(QString("UPDATE local SET id_serv=NULL WHERE id_local=%1").arg(localId)));
        QVERIFY(!q.exec("UPDATE stock SET id_local=99999999 WHERE article='MLD-TEST-ARTICLE'"));
        QVERIFY(!q.exec(QString("UPDATE medical SET id_emp=99999999 WHERE id_doss=%1").arg(record.storageId)));
        QVERIFY(!q.exec(QString("UPDATE employer SET id_serv=99999999 WHERE id_emp=%1").arg(employee.getId())));
        QVERIFY(!locals.remove(localId));QVERIFY(!Service::supprimer(base,&error));
        QVERIFY2(fromMedical.supprimer(fromMedical.getId()),qPrintable(fromMedical.lastError()));services.removeAll(fromMedical.getId());
        QVERIFY2(MedicalRecord::supprimer(record.storageId,&error),qPrintable(error));record.storageId=0;
        QVERIFY2(StockRecord::enregistrer(oldStock,&revision,&error),qPrintable(error));
        locals.reload();QVERIFY2(locals.remove(localId),qPrintable(locals.lastError()));localId=0;
    }

    void scheduleConflictValidation() {
        auto &db=DatabaseManager::instance();QVERIFY(db.initialize(":memory:"));
        QSqlQuery q(OracleConnection::database());
        QVERIFY(q.exec("INSERT INTO employes(id,nom,prenom) VALUES(701,'Planning','Nour')"));
        Service s;s.setEmployeeId(701);s.setName("Consultation");s.setClientName("Test");s.setDateTime(QDateTime(QDate(2030,1,7),QTime(23,30)));s.setDuration(60);
        QVERIFY(db.addService(s));s.setId(db.lastInsertedId());QVERIFY(db.updateService(s)); // self is excluded
        Service overlap=s;overlap.setId(0);overlap.setDateTime(s.getDateTime().addSecs(1800));
        QVERIFY(!db.addService(overlap));QVERIFY(db.lastError().contains("déjà occupé"));
        Service adjacent=overlap;adjacent.setDateTime(s.getDateTime().addSecs(3600));QVERIFY(db.addService(adjacent));adjacent.setId(db.lastInsertedId());
        adjacent.setDateTime(overlap.getDateTime());QVERIFY(!db.updateService(adjacent));
        overlap.setEmployeeId(702);QVERIFY(db.addService(overlap));
        overlap.setEmployeeId(701);overlap.setStatus("Annulé");QVERIFY(db.addService(overlap));
        QVERIFY(q.exec("INSERT INTO planning_affectations(titre,competence_requise,debut,fin,responsable_id) VALUES('Nettoyage','Soins','2030-01-08 08:00:00','2030-01-08 09:00:00',701)"));
        overlap.setStatus("Planifié");overlap.setDateTime(QDateTime(QDate(2030,1,8),QTime(8,30)));QVERIFY(!db.addService(overlap));QVERIFY(db.lastError().contains("Nettoyage"));
        ServicePlanningDialog dialog(QDate(2030,1,7));dialog.showEmployee(701,true);
        auto *table=dialog.findChild<QTableWidget *>("servicePlanningTable");QCOMPARE(table->rowCount(),3);
        QVERIFY(table->item(2,3)->text().contains("Nettoyage"));QVERIFY(dialog.findChild<QPushButton *>("planningEdit")->isHidden());
    }
    void oracleScheduleGuards() {
        if(qEnvironmentVariable("ANIWORLD_ORACLE_TESTS")!="1")QSKIP("Oracle development schema opt-in.");
        const auto appName=qApp->applicationName(),org=qApp->organizationName();qApp->setApplicationName("ANIWORLD");qApp->setOrganizationName("ANIWORLD");
        auto &manager=DatabaseManager::instance();
        const auto restore=qScopeGuard([&]{manager.initialize(":memory:");qApp->setApplicationName(appName);qApp->setOrganizationName(org);});
        QVERIFY2(manager.initializeOracle(),qPrintable(manager.lastError()));auto db=OracleConnection::database();
        QVERIFY(db.transaction());const auto rollback=qScopeGuard([&]{db.rollback();});QSqlQuery q(db);
        QVERIFY(q.exec("INSERT INTO employes(nom,prenom) VALUES('ScheduleTest','Fictif')"));
        QVERIFY(q.exec("SELECT employes_seq.CURRVAL FROM dual"));QVERIFY(q.next());const int employee=q.value(0).toInt();q.finish();
        Service s;s.setName("Consultation test planning");s.setClientName("Client fictif");s.setEmployeeId(employee);s.setDateTime(QDateTime(QDate(2030,1,7),QTime(9,0)));s.setDuration(60);
        QVERIFY2(manager.addService(s),qPrintable(manager.lastError()));s.setId(manager.lastInsertedId());QVERIFY(manager.updateService(s));
        auto task=[&](QString from,QString to){q.prepare("INSERT INTO planning_affectations(titre,competence_requise,debut,fin,responsable_id) VALUES('Tache test planning','Soins',?,?,?)");q.addBindValue(from);q.addBindValue(to);q.addBindValue(employee);return q.exec();};
        QVERIFY(!task("2030-01-07 09:30:00","2030-01-07 10:30:00"));QVERIFY(q.lastError().text().contains("20051"));
        QVERIFY(task("2030-01-07 10:00:00","2030-01-07 11:00:00")); // touching endpoints allowed
        QVERIFY(!task("2030-01-07 10:30:00","2030-01-07 11:30:00"));QVERIFY(q.lastError().text().contains("20051"));
        QVERIFY(!q.exec(QString("UPDATE services SET datetime='2030-01-07T10:30:00' WHERE id=%1").arg(s.getId())));QVERIFY(q.lastError().text().contains("20051"));
        // Bypass Qt's precheck: Oracle itself rejects service/service overlaps too.
        q.prepare("INSERT INTO services(name,category,datetime,status,duration,price,id_employe) VALUES('Collision','Soins','2030-01-07T09:30:00','Planifié',30,0,?)");q.addBindValue(employee);
        QVERIFY(!q.exec());QVERIFY(q.lastError().text().contains("20051"));
        Service collision=s;collision.setId(0);collision.setDateTime(s.getDateTime().addSecs(1800));
        ServiceDialog form(collision,false);form.show();QTest::qWait(40);
        form.findChild<QPushButton *>("saveService")->click();QVERIFY(form.result()!=QDialog::Accepted);
        QVERIFY(form.findChild<QLabel *>("validationError")->text().contains("déjà occupé"));
        bool preview=false;QTimer::singleShot(100,[&]{auto *d=qobject_cast<ServicePlanningDialog *>(QApplication::activeModalWidget());if(!d)return;
            preview=d->findChild<QComboBox *>("planningEmployee")->currentData().toInt()==employee && d->findChild<QTableWidget *>("servicePlanningTable")->rowCount()==2;
            const auto path=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");if(!path.isEmpty()){d->resize(1160,700);d->grab().save(path+"/employee-schedule.png");}d->accept();});
        form.findChild<QPushButton *>("employeeSchedule")->click();QVERIFY(preview);
        const auto path=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");if(!path.isEmpty())form.grab().save(path+"/blocked-assignment.png");form.hide();
        QVERIFY(q.exec(QString("UPDATE services SET status='Annulé' WHERE id=%1").arg(s.getId())));
        QVERIFY(task("2030-01-07 09:00:00","2030-01-07 10:00:00"));
        QVERIFY(!q.exec(QString("UPDATE services SET status='Planifié' WHERE id=%1").arg(s.getId())));QVERIFY(q.lastError().text().contains("20051"));
        qInfo()<<"QODBC: service/task, task/task, service/service, cancelled reactivation rejected; adjacent slots and self-update accepted. All test data rolled back.";
    }
    void servicePlanningBoundaries() {
        const QDate monday(2030,1,7);
        Service a;a.setId(1);a.setEmployeeId(10);a.setDateTime(QDateTime(monday,QTime(9,0)));a.setDuration(60);
        Service b=a;b.setId(2);b.setDateTime(a.getDateTime().addSecs(1800));
        Service c=a;c.setId(3);c.setDateTime(a.getDateTime().addSecs(5400));
        Service cancelled=a;cancelled.setId(4);cancelled.setStatus("Annulé");
        Service other=a;other.setId(5);other.setEmployeeId(20);
        Service night=a;night.setId(6);night.setDateTime(QDateTime(monday.addDays(-1),QTime(23,30)));
        Service boundary=night;boundary.setId(7);boundary.setDuration(30);boundary.setEmployeeId(20);
        const QList<Service> all={a,b,c,cancelled,other,night,boundary};
        const auto conflicts=Service::conflictingServices(all);
        QCOMPARE(conflicts,QSet<int>({1,2})); // touching endpoints, cancellations and different employees do not collide
        QCOMPARE(Service::inWeek(all,monday,10).size(),4);
        QCOMPARE(Service::inWeek(all,monday,20).size(),1);
        QCOMPARE(Service::inWeek(all,monday,10,true).size(),5);
        QCOMPARE(Service::inWeek(all,monday.addDays(7)).size(),0);
        QCOMPARE(Service::weekStart(monday.addDays(6)),monday);
    }
    void servicePlanningFiltersAndEntry() {
        auto &db=DatabaseManager::instance();QVERIFY(db.initialize(":memory:"));
        QSqlQuery q(OracleConnection::database());
        QVERIFY(q.exec("INSERT INTO employes(id,nom,prenom,disponible) VALUES(701,'Jelassi','Nour',1)"));
        QVERIFY(q.exec("INSERT INTO employes(id,nom,prenom,disponible) VALUES(702,'Trabelsi','Amine',0)"));
        const QDate monday(2030,1,7);
        for(int i=0;i<5;++i) {
            Service s;s.setName(QStringList{"Consultation générale","Vaccination","Toilettage","Contrôle annuel","Consultation annulée"}[i]);
            s.setClientName("Client de démonstration");s.setDateTime(QDateTime(monday,QTime(9,0)).addSecs(i*3600));s.setDuration(60);
            s.setEmployeeId(i==2?702:i==3?0:701);s.setResponsible(i==2?"Trabelsi Amine":i==3?"":"Jelassi Nour");
            if(i==4)s.setStatus("Annulé");
            QVERIFY(db.addService(s));
        }
        ServicePlanningDialog dialog(monday);dialog.show();QTest::qWait(80);
        auto *table=dialog.findChild<QTableWidget *>("servicePlanningTable");auto *employee=dialog.findChild<QComboBox *>("planningEmployee");
        QCOMPARE(table->rowCount(),4);QVERIFY(!table->item(0,6)->text().contains("Chevauchement"));
        const auto output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");if(!output.isEmpty()){QDir().mkpath(output);QVERIFY(dialog.grab().save(output+"/planning-services-small.png"));dialog.resize(1160,700);QTest::qWait(60);QVERIFY(dialog.grab().save(output+"/planning-services.png"));}
        employee->setCurrentIndex(employee->findData(701));QCOMPARE(table->rowCount(),2);
        dialog.findChild<QCheckBox *>("planningCancelled")->setChecked(true);QCOMPARE(table->rowCount(),3);
        employee->setCurrentIndex(employee->findData(-1));QCOMPARE(table->rowCount(),1);QVERIFY(table->item(0,6)->text().contains("À affecter"));
        dialog.findChild<QPushButton *>("planningNext")->click();QCOMPARE(table->rowCount(),0);QVERIFY(!dialog.findChild<QPushButton *>("planningEdit")->isEnabled());
        dialog.findChild<QPushButton *>("planningPrevious")->click();QCOMPARE(table->rowCount(),1);dialog.hide();
        ServiceWidget page;page.resize(1350,850);page.show();
        QVERIFY(!page.findChild<QPushButton *>("exportServices"));QVERIFY(page.findChild<QPushButton *>("invoiceService"));
        bool opened=false;QTimer::singleShot(120,[&]{auto *d=qobject_cast<ServicePlanningDialog *>(QApplication::activeModalWidget());opened=d;if(d)d->accept();});
        page.findChild<QPushButton *>("servicePlanning")->click();QVERIFY(opened);
        if(!output.isEmpty())QVERIFY(page.grab().save(output+"/services-with-planning.png"));
    }
    void oracleServicePlanningAssignment() {
        if(qEnvironmentVariable("ANIWORLD_ORACLE_TESTS")!="1")QSKIP("Oracle development schema opt-in.");
        const auto appName=qApp->applicationName(),org=qApp->organizationName();qApp->setApplicationName("ANIWORLD");qApp->setOrganizationName("ANIWORLD");
        auto &manager=DatabaseManager::instance();
        const auto restore=qScopeGuard([&]{manager.initialize(":memory:");qApp->setApplicationName(appName);qApp->setOrganizationName(org);});
        QVERIFY2(manager.initializeOracle(),qPrintable(manager.lastError()));auto db=OracleConnection::database();
        QVERIFY(db.transaction());const auto rollback=qScopeGuard([&]{db.rollback();});QSqlQuery q(db);
        auto addEmployee=[&](const QString &name){q.prepare("INSERT INTO employes(nom,prenom) VALUES(?,'PlanningTest')");q.addBindValue(name);if(!q.exec())return 0;if(!q.exec("SELECT employes_seq.CURRVAL FROM dual")||!q.next())return 0;int id=q.value(0).toInt();q.finish();return id;};
        const int first=addEmployee("Premier"),second=addEmployee("Second");QVERIFY(first>0 && second>0);
        Service s;s.setName("Test planning transactionnel");s.setClientName("Client fictif");s.setEmployeeId(first);s.setDateTime(QDateTime(QDate(2030,1,7),QTime(9,0)));
        QVERIFY(manager.addService(s));s.setId(manager.lastInsertedId());
        ServicePlanningDialog dialog(s.getDateTime().date());dialog.show();QSignalSpy changed(&dialog,&ServicePlanningDialog::servicesChanged);
        auto *employee=dialog.findChild<QComboBox *>("planningEmployee");employee->setCurrentIndex(employee->findData(first));
        auto *table=dialog.findChild<QTableWidget *>("servicePlanningTable");QCOMPARE(table->rowCount(),1);table->selectRow(0);
        bool formOpened=false;QTimer::singleShot(120,[&]{auto *form=visibleServiceForm();if(!form)return;formOpened=true;
            auto *choice=form->findChild<QComboBox *>("serviceEmployee");choice->setCurrentIndex(choice->findData(second));
            form->findChild<QDateTimeEdit *>("serviceDate")->setDateTime(s.getDateTime().addSecs(3600));form->findChild<QPushButton *>("saveService")->click();});
        dialog.findChild<QPushButton *>("planningEdit")->click();QVERIFY(formOpened);QCOMPARE(changed.size(),1);QCOMPARE(table->rowCount(),0);
        employee->setCurrentIndex(employee->findData(second));QCOMPARE(table->rowCount(),1);QVERIFY(table->item(0,1)->text().startsWith("10:00"));
        bool found=false;for(const auto &read:manager.getAllServices())if(read.getId()==s.getId()){found=true;QCOMPARE(read.getEmployeeId(),second);QCOMPARE(read.getDateTime(),s.getDateTime().addSecs(3600));}QVERIFY(found);
        qInfo()<<"Planning reassignment and time change saved/read through Qt QODBC; test data rolled back.";
    }
    void oracleFreshRawand() {
        if(qEnvironmentVariable("ANIWORLD_RAWAND_TEST")!="1") QSKIP("Fresh RAWAND schema opt-in.");
        const auto appName=qApp->applicationName(),org=qApp->organizationName();
        qApp->setApplicationName("ANIWORLD");qApp->setOrganizationName("ANIWORLD");
        auto &manager=DatabaseManager::instance();
        const auto restore=qScopeGuard([&]{manager.initialize(":memory:");qApp->setApplicationName(appName);qApp->setOrganizationName(org);});
        QVERIFY2(manager.initializeOracle(),qPrintable(manager.lastError()));
        auto db=OracleConnection::database();QSqlQuery q(db);QString error;
        QVERIFY(q.exec("SELECT USER,SYS_CONTEXT('USERENV','CON_NAME') FROM dual"));QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(),QString("RAWAND"));QCOMPARE(q.value(1).toString(),QString("CDB$ROOT"));q.finish();
        for(const QString &name:QStringList{"admin","services","stock","employes","locaux","medical"})
            QVERIFY(Accounts::authenticate(name,name+"123")!=Accounts::Role::None);
        QVERIFY(!OracleConnection::migrateLocal(manager.storagePath(),&error));QVERIFY(error.contains("désactivé"));
        MainWindow main;main.setSession("admin",Accounts::Role::Admin);main.show();
        for(int page=1;page<=5;++page){main.navigate(page);QTest::qWait(30);}
        for(const QString &table:QStringList{"services","employes","medical_records","planning_affectations","stock_lots","stock_movements","locaux","clients","factures","lignes_facture"}) {
            QVERIFY(q.exec("SELECT COUNT(*) FROM "+table));QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),0);q.finish();
        }
        qInfo()<<"RAWAND/XE via QODBC: six application logins, all module screens, empty business tables and blocked legacy import verified.";
    }
    void oracleCommonRelations() {
        if(qEnvironmentVariable("ANIWORLD_ORACLE_TESTS")!="1") QSKIP("Oracle development schema opt-in.");
        const auto appName=qApp->applicationName(),org=qApp->organizationName();
        qApp->setApplicationName("ANIWORLD");qApp->setOrganizationName("ANIWORLD");
        auto &manager=DatabaseManager::instance();
        const auto restore=qScopeGuard([&]{manager.initialize(":memory:");qApp->setApplicationName(appName);qApp->setOrganizationName(org);});
        QVERIFY2(manager.initializeOracle(),qPrintable(manager.lastError()));
        auto db=OracleConnection::database();QString error;
        auto scalar=[&](const QString &sql){QSqlQuery q(db);if(!q.exec(sql)||!q.next())return -1;return q.value(0).toInt();};
        const int clientsBefore=scalar("SELECT COUNT(*) FROM clients"),invoicesBefore=scalar("SELECT COUNT(*) FROM factures");
        QVERIFY(db.transaction());
        {
            const auto rollback=qScopeGuard([&]{db.rollback();});
            QSqlQuery q(db);
            QVERIFY(q.exec("INSERT INTO employes(nom,prenom) VALUES('RelationTest','Fictif')"));
            const int employee=scalar("SELECT employes_seq.CURRVAL FROM dual");QVERIFY(employee>0);
            Service s;s.setName("Prestation de test relations");s.setClientName("Client homonyme fictif");s.setPrice(55);s.setEmployeeId(employee);s.setDescription("Détail de test");
            QVERIFY2(manager.addService(s),qPrintable(manager.lastError()));s.setId(manager.lastInsertedId());
            auto read=[&](int id){for(const auto &v:manager.getAllServices())if(v.getId()==id)return v;return Service{};};
            s=read(s.getId());QVERIFY(s.getClientId()>0);QCOMPARE(s.getEmployeeId(),employee);QCOMPARE(s.getResponsible(),QString("RelationTest Fictif"));
            QCOMPARE(scalar("SELECT COUNT(*) FROM clients"),clientsBefore+1);
            Service other=s;other.setId(0);other.setDateTime(s.getDateTime().addSecs(3600));QVERIFY(manager.addService(other));const int otherId=manager.lastInsertedId();
            QCOMPARE(read(otherId).getClientId(),s.getClientId());QCOMPARE(scalar("SELECT COUNT(*) FROM clients"),clientsBefore+1);
            other.setClientId(0);other.setDateTime(s.getDateTime().addSecs(7200));QVERIFY(manager.addService(other));QVERIFY(read(manager.lastInsertedId()).getClientId()!=s.getClientId());
            QCOMPARE(scalar("SELECT COUNT(*) FROM clients"),clientsBefore+2);
            auto record=MedicalRecord::examples().first();record.storageId=0;record.medical.serviceId=s.getId();
            QVERIFY2(MedicalRecord::enregistrer(record,&error),qPrintable(error));
            bool found=false;for(const auto &r:MedicalRecord::afficher(&error))if(r.storageId==record.storageId){found=true;QCOMPARE(r.medical.serviceId,s.getId());}QVERIFY(found);
            QVERIFY(!manager.deleteService(s.getId()));
            QVERIFY(!q.exec("UPDATE medical_records SET id_service=-2147483647 WHERE id="+QString::number(record.storageId)));
            other=read(otherId);other.setEmployeeId(2147483647);QVERIFY(!manager.updateService(other));
            other=read(otherId);other.setClientId(2147483647);QVERIFY(!manager.updateService(other));
            q.prepare("INSERT INTO planning_affectations(titre,competence_requise,debut,fin,responsable_id) VALUES('Test','Soins','2026-10-15 10:00:00','2026-10-15 11:00:00',?)");q.addBindValue(employee);QVERIFY(q.exec());
            const int planning=scalar("SELECT planning_affectations_seq.CURRVAL FROM dual");
            QVERIFY(!q.exec("DELETE FROM employes WHERE id="+QString::number(employee)));
            QCOMPARE(scalar("SELECT COUNT(*) FROM planning_affectations WHERE id="+QString::number(planning)),1);
            Service stored;QVERIFY2(Invoice::loadStored(s,&stored,&error),qPrintable(error));QCOMPARE(stored.getPrice(),55.0);QCOMPARE(stored.getDescription(),s.getDescription());
            s.setPrice(60);QVERIFY2(manager.updateService(s),qPrintable(manager.lastError()));QVERIFY(Invoice::loadStored(s,&stored,&error));QCOMPARE(stored.getPrice(),60.0);
            const int fid=scalar("SELECT id FROM factures WHERE id_service="+QString::number(s.getId()));QVERIFY(fid>0);
            QCOMPARE(scalar("SELECT prix_unitaire FROM lignes_facture WHERE id_facture="+QString::number(fid)),60);
            QVERIFY(q.exec("UPDATE factures SET statut='EMISE',date_emission=SYSTIMESTAMP WHERE id="+QString::number(fid)));
            s.setPrice(99);QVERIFY(!manager.updateService(s));
            QVERIFY(!q.exec("UPDATE lignes_facture SET prix_unitaire=99 WHERE id_facture="+QString::number(fid)));
            QVERIFY(!q.exec("UPDATE factures SET total=99 WHERE id="+QString::number(fid)));
            QVERIFY(Invoice::loadStored(s,&stored,&error));QCOMPARE(stored.getPrice(),60.0);
            QTemporaryDir pdf;QVERIFY2(Invoice::writeDocument(pdf.filePath("snapshot.pdf"),Invoice::html(stored),"Test",&error),qPrintable(error));
            QVERIFY(MedicalRecord::supprimer(record.storageId,&error));QVERIFY(manager.deleteService(s.getId()));
            QCOMPARE(scalar("SELECT COUNT(*) FROM factures WHERE id="+QString::number(fid)+" AND id_service IS NULL"),1);
            ServiceDialog form(read(otherId),false);QCOMPARE(form.findChild<QComboBox *>("serviceEmployee")->currentData().toInt(),employee);
            QCOMPARE(form.getService().getClientId(),read(otherId).getClientId());
            MedicalRecordDialog medical;medical.setMedicalRecord(record);QCOMPARE(medical.getMedicalRecord().medical.serviceId,s.getId());
            const auto output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
            if(!output.isEmpty()){QDir().mkpath(output);form.show();QTest::qWait(220);QVERIFY(form.grab().save(output+"/common-service.png"));}
        }
        QCOMPARE(scalar("SELECT COUNT(*) FROM clients"),clientsBefore);
        QCOMPARE(scalar("SELECT COUNT(*) FROM factures"),invoicesBefore);
        qInfo()<<"Employee/client/service FKs, same-name clients separated, invoice snapshots, protected planning and historical invoices: OK; all test rows rolled back.";
    }
    void oracleLocalLinks() {
        if(qEnvironmentVariable("ANIWORLD_ORACLE_TESTS")!="1") QSKIP("Oracle development schema opt-in.");
        const auto appName=qApp->applicationName(),org=qApp->organizationName();
        qApp->setApplicationName("ANIWORLD");qApp->setOrganizationName("ANIWORLD");
        auto &manager=DatabaseManager::instance();
        const auto restore=qScopeGuard([&]{manager.initialize(":memory:");qApp->setApplicationName(appName);qApp->setOrganizationName(org);});
        QVERIFY2(manager.initializeOracle(),qPrintable(manager.lastError()));
        auto db=OracleConnection::database();QString error;
        int localId=0;
        { QSqlQuery q(db); QVERIFY(q.exec("SELECT id FROM locaux WHERE out_of_service=0 ORDER BY id")); QVERIFY(q.next()); localId=q.value(0).toInt(); }
        const auto output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");if(!output.isEmpty())QDir().mkpath(output);
        QVERIFY(db.transaction());
        {
            const auto rollback=qScopeGuard([&]{db.rollback();});
            Service s;s.setName("Test affectation local");s.setClientName("Client fictif");s.setLocalId(localId);
            QVERIFY2(manager.addService(s),qPrintable(manager.lastError()));s.setId(manager.lastInsertedId());
            QVERIFY2(manager.addService(s),qPrintable(manager.lastError())); // multiple consultations/services per room
            Service saved;for(const auto &v:manager.getAllServices())if(v.getId()==s.getId())saved=v;
            QCOMPARE(saved.getLocalId(),localId);
            ServiceDialog dialog(saved,false);auto *choice=dialog.findChild<QComboBox *>("serviceLocal");QVERIFY(choice);QCOMPARE(choice->currentData().toInt(),localId);
            QCOMPARE(dialog.getService().getLocalId(),localId);
            dialog.show();QTest::qWait(100);if(!output.isEmpty())QVERIFY(dialog.grab().save(output+"/service-local.png"));dialog.hide();
            saved.setLocalId(0);QVERIFY2(manager.updateService(saved),qPrintable(manager.lastError()));
            for(const auto &v:manager.getAllServices())if(v.getId()==s.getId())QCOMPARE(v.getLocalId(),0);
            auto r=MedicalRecord::examples().first();r.storageId=0;r.medical.localId=localId;
            QVERIFY2(MedicalRecord::enregistrer(r,&error),qPrintable(error));
            bool found=false;for(const auto &v:MedicalRecord::afficher(&error))if(v.storageId==r.storageId){found=true;QCOMPARE(v.medical.localId,localId);}QVERIFY(found);
            MedicalRecordDialog medical;medical.setMedicalRecord(r);QCOMPARE(medical.getMedicalRecord().medical.localId,localId);
            medical.show();QTest::qWait(100);if(!output.isEmpty())QVERIFY(medical.grab().save(output+"/medical-local.png"));medical.hide();
            QSqlQuery q(db);q.prepare("DELETE FROM locaux WHERE id=?");q.addBindValue(localId);QVERIFY(!q.exec());QVERIFY(q.lastError().nativeErrorCode().contains("2292"));q.finish();
            for(const QString &table:{QString("services"),QString("medical_records"),QString("stock_lots")}) {
                QVERIFY(!q.exec("UPDATE "+table+" SET id_local=-2147483647"));QVERIFY2(q.lastError().nativeErrorCode().contains("2291"),qPrintable(q.lastError().text()));q.finish();
            }
        }
        StockRecord::Rows base; qint64 revision=0;
        QVERIFY2(StockRecord::readRows(&base,&revision,&error),qPrintable(error));
        const QString lot="LOCAL-LINK-TEST-"+QString::number(QDateTime::currentMSecsSinceEpoch());
        const auto cleanup=qScopeGuard([&]{StockRecord::Rows rows;qint64 rev=0;QString e;if(StockRecord::readRows(&rows,&rev,&e)){for(int i=rows.size()-1;i>=0;--i)if(rows[i][0].toString()==lot)rows.removeAt(i);StockRecord::saveRows(rows,&rev,&e);}QSqlQuery q(db);q.prepare("DELETE FROM stock_movements WHERE lot=?");q.addBindValue(lot);q.exec();});
        auto rows=base;rows<<QVariantList{lot,"Lot de test","Chiens",1,"unités",1,"2027-01-01",localId};
        QVERIFY2(StockRecord::saveRows(rows,&revision,&error),qPrintable(error));
        rows.clear();QVERIFY2(StockRecord::readRows(&rows,&revision,&error),qPrintable(error));
        bool found=false;for(const auto &r:rows)if(r[0].toString()==lot){found=true;QCOMPARE(r[7].toInt(),localId);}QVERIFY(found);
        StockWidget stock; stock.show();auto *table=stock.findChild<QTableView *>();QVERIFY(table);table->selectRow(0);
        bool stockFormChecked=false;
        QTimer::singleShot(150,[&]{
            auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if(dialog) {
                auto *choice=dialog->findChild<QComboBox *>("stockLocal");
                stockFormChecked=choice && choice->findData(localId)>=0;
                if(choice)choice->setCurrentIndex(choice->findData(localId));
                if(!output.isEmpty())dialog->grab().save(output+"/stock-local.png");
                dialog->reject();
            }
        });
        QVERIFY(QMetaObject::invokeMethod(&stock,"editSelectedItem",Qt::DirectConnection));QVERIFY(stockFormChecked);
        qInfo()<<"Local links: service/medical/stock round-trip, optional assignment, many-to-one, 3 FK rejections and protected deletion verified.";
    }
    void oracleEndToEnd() {
        if(qEnvironmentVariable("ANIWORLD_ORACLE_TESTS")!="1") QSKIP("Opt-in: uses the local Oracle development schema.");
        const auto appName=qApp->applicationName(),org=qApp->organizationName();
        qApp->setApplicationName("ANIWORLD");qApp->setOrganizationName("ANIWORLD");
        auto &manager=DatabaseManager::instance();
        const auto restore=qScopeGuard([&]{manager.initialize(":memory:");qApp->setApplicationName(appName);qApp->setOrganizationName(org);});
        QVERIFY2(manager.initializeOracle(),qPrintable(manager.lastError()));
        auto db=OracleConnection::database();QString error;
        QVERIFY2(OracleConnection::migrateLocal(manager.storagePath(),&error),qPrintable(error));
        QCOMPARE(Accounts::authenticate("ADMIN","admin123"),Accounts::Role::Admin);
        QCOMPARE(Accounts::authenticate("medical","wrong-password"),Accounts::Role::None);
        qInfo()<<"Oracle connection, idempotent migration and account authentication: OK";
        QVERIFY(db.transaction());
        {
            const auto rollback=qScopeGuard([&]{db.rollback();});
            Service s;s.setName("Test Oracle Éléphant");s.setClientName("Client d'essai");s.setPrice(47.25);s.setDescription(QString(5000,QChar(0x00e9)));
            QVERIFY2(manager.addService(s),qPrintable(manager.lastError()));s.setId(manager.lastInsertedId());QVERIFY(s.getId()>0);
            auto findService=[&]{for(const auto &v:manager.getAllServices())if(v.getId()==s.getId())return v;return Service{};};
            auto saved=findService();QCOMPARE(saved.getDescription(),s.getDescription());QCOMPARE(saved.getPrice(),s.getPrice());QVERIFY(!saved.getInvoiceNumber().isEmpty());
            const auto invoice=saved.getInvoiceNumber();const auto issued=saved.getInvoiceDate();
            saved.setPrice(51.75);QVERIFY2(manager.updateService(saved),qPrintable(manager.lastError()));saved=findService();
            QCOMPARE(saved.getInvoiceNumber(),invoice);QCOMPARE(saved.getInvoiceDate(),issued);QCOMPARE(saved.getPrice(),51.75);
            Service invoiceSnapshot;QVERIFY(Invoice::loadStored(saved,&invoiceSnapshot,&error));
            QTemporaryDir pdf;QVERIFY2(Invoice::writeDocument(pdf.filePath("oracle-invoice.pdf"),Invoice::html(invoiceSnapshot),"Test Oracle",&error),qPrintable(error));
            QVERIFY(manager.deleteService(s.getId()));QVERIFY(!manager.deleteService(s.getId()));
            auto medical=MedicalRecord::examples().first();medical.storageId=0;medical.animal.name="Test Oracle";medical.medical.observations=QString(5000,QChar(0x00e9));
            QVERIFY2(MedicalRecord::enregistrer(medical,&error),qPrintable(error));QVERIFY(medical.storageId>0);
            bool found=false;for(const auto &v:MedicalRecord::afficher(&error))if(v.storageId==medical.storageId){found=true;QCOMPARE(v.medical.observations,medical.medical.observations);}QVERIFY(found);
            medical.animal.name="Test Oracle modifié";QVERIFY2(MedicalRecord::enregistrer(medical,&error),qPrintable(error));
            QVERIFY2(MedicalRecord::supprimer(medical.storageId,&error),qPrintable(error));
            QSqlQuery q(db);QVERIFY2(q.exec("INSERT INTO employes(nom,prenom,poste,date_embauche) VALUES('TestOracle','Essai','Vétérinaire','2026-10-14')"),qPrintable(q.lastError().text()));
            QVERIFY(q.exec("SELECT employes_seq.CURRVAL FROM dual"));QVERIFY(q.next());const int employeeId=q.value(0).toInt();q.finish();
            q.prepare("INSERT INTO planning_affectations(titre,competence_requise,debut,fin,charge_heures,responsable_id) VALUES(?,?,?,?,?,?)");
            q.addBindValue("Test planning");q.addBindValue("Soins");q.addBindValue("2026-10-14 10:00:00");q.addBindValue("2026-10-14 11:00:00");q.addBindValue(1);q.addBindValue(employeeId);
            QVERIFY2(q.exec(),qPrintable(q.lastError().text()));
            GestionEmployes employees;employees.show();
            employees.findChild<QLineEdit *>("employeeSearch")->setText(QString::number(employeeId));QTest::qWait(60);
            const auto cards=employees.findChildren<QWidget *>("cardEmploye");QVERIFY(!cards.isEmpty());
            q.prepare("DELETE FROM employes WHERE id=?");q.addBindValue(employeeId);QVERIFY(!q.exec());
            q.prepare("SELECT COUNT(*) FROM planning_affectations WHERE responsable_id=?");q.addBindValue(employeeId);QVERIFY(q.exec());QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),1);q.finish();
            QVERIFY2(Accounts::changePassword("admin","admin123","OracleTestOnly123",&error),qPrintable(error));
            QCOMPARE(Accounts::authenticate("admin","OracleTestOnly123"),Accounts::Role::Admin);
        }
        QCOMPARE(Accounts::authenticate("admin","admin123"),Accounts::Role::Admin);
        qInfo()<<"Service/invoice, medical CLOB, employee/planning CRUD and password rollback: OK";
        MainWindow main;main.resize(1400,900);main.setSession("admin",Accounts::Role::Admin);main.show();QTest::qWait(80);
        QCOMPARE(main.findChild<QLabel *>("storageBadge")->text(),QString("●  Oracle · connecté"));
        const auto output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");if(!output.isEmpty())QDir().mkpath(output);
        for(int page=1;page<=5;++page) { main.navigate(page);QTest::qWait(80);if(!output.isEmpty())QVERIFY(main.grab().save(output+QString("/oracle-page-%1.png").arg(page))); }
        StockRecord::Rows base; qint64 revision=0;
        QVERIFY2(StockRecord::readRows(&base,&revision,&error),qPrintable(error));QVERIFY(!base.isEmpty());
        const QString testLot="ORACLE-TEST-"+QString::number(QDateTime::currentMSecsSinceEpoch());
        const auto cleanup=qScopeGuard([&]{StockRecord::Rows current;qint64 rev=0;QString e;if(StockRecord::readRows(&current,&rev,&e)){for(int i=current.size()-1;i>=0;--i)if(current[i][0].toString()==testLot)current.removeAt(i);StockRecord::saveRows(current,&rev,&e);}QSqlQuery remove(db);remove.prepare("DELETE FROM stock_movements WHERE lot=?");remove.addBindValue(testLot);remove.exec();});
        auto rows=base;rows<<QVariantList{testLot,"Test Oracle","Chiens",4,"unités",1,"2027-01-01",QVariant(QMetaType(QMetaType::Int))};
        const auto staleRevision=revision;
        QVERIFY2(StockRecord::saveRows(rows,&revision,&error),qPrintable(error));
        StockRecord::Rows loaded;qint64 loadedRevision=0;QVERIFY(StockRecord::readRows(&loaded,&loadedRevision,&error));QCOMPARE(loaded.size(),base.size()+1);
        qint64 stale=staleRevision;QVERIFY(!StockRecord::saveRows(base,&stale,&error));QVERIFY(error.contains("autre session"));
        rows.last()[3]=-1;QVERIFY(!StockRecord::saveRows(rows,&revision,&error));
        QSqlQuery q(db);q.prepare("SELECT quantity FROM stock_lots WHERE lot=?");q.addBindValue(testLot);QVERIFY(q.exec());QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),4);q.finish();
        q.prepare("SELECT COUNT(*) FROM stock_movements WHERE lot=?");q.addBindValue(testLot);QVERIFY(q.exec());QVERIFY(q.next());QCOMPARE(q.value(0).toInt(),1);q.finish();
        LocalRepository rooms(nullptr,"oracle:");QVERIFY2(rooms.lastError().isEmpty(),qPrintable(rooms.lastError()));QVERIFY(!rooms.all().isEmpty());
        qInfo()<<"Oracle UI for five modules, stock persistence, audit and stale-write rejection, local import: OK";
    }
    void medicalReferenceImportOnce() {
        QTemporaryDir folder;const auto path=folder.filePath("reference.sqlite");
        auto &db=DatabaseManager::instance();QVERIFY(db.initialize(path));QString error;
        QVERIFY2(MedicalRecord::importInitial(&error),qPrintable(error));QCOMPARE(MedicalRecord::afficher().size(),8);
        {
            MainWindow main;main.resize(1400,900);main.setSession("medical",Accounts::Role::Medical);main.show();
            QTest::qWait(100);
            const auto output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
            if(!output.isEmpty()){QDir().mkpath(output);QVERIFY(main.grab().save(output+"/medical-normal.png"));}
        }
        QSqlQuery query(OracleConnection::database());QVERIFY(query.exec("DELETE FROM medical_records"));
        QVERIFY(db.initialize(path));
        QVERIFY(MedicalRecord::importInitial(&error));QVERIFY(MedicalRecord::afficher().isEmpty());
        QVERIFY(db.initialize(":memory:"));auto record=MedicalRecord::examples().first();record.animal.name="Dossier personnel";
        QVERIFY(MedicalRecord::enregistrer(record,&error));QVERIFY(MedicalRecord::importInitial(&error));
        QCOMPARE(MedicalRecord::afficher().size(),1);QCOMPARE(MedicalRecord::afficher().first().animal.name,QString("Dossier personnel"));
    }
    void medicalPersistenceAndHistory() {
        QTemporaryDir folder; const auto path=folder.filePath("medical.sqlite");
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(path));
        QString error; QVERIFY(MedicalRecord::afficher(&error).isEmpty()); QVERIFY(error.isEmpty());
        auto first=MedicalRecord::examples().first(); first.animal.name="Test <animal> & ami";
        QVERIFY2(MedicalRecord::enregistrer(first,&error),qPrintable(error)); QVERIFY(first.storageId>0);
        const auto id=first.storageId; const auto number=first.medical.recordId;
        first.medical.diagnosis="Diagnostic conservé"; QVERIFY(MedicalRecord::enregistrer(first,&error)); QCOMPARE(first.medical.recordId,number);
        auto invalid=first; invalid.animal.name="  "; QVERIFY(!MedicalRecord::enregistrer(invalid,&error));
        invalid=first; invalid.medical.weight=std::numeric_limits<double>::quiet_NaN(); QVERIFY(!MedicalRecord::enregistrer(invalid,&error));
        invalid=first; invalid.medical.observations=QString(5001,'x'); QVERIFY(!MedicalRecord::enregistrer(invalid,&error));
        auto second=first; second.storageId=0; second.medical.consultationDate=first.medical.consultationDate.addDays(1);
        QVERIFY(MedicalRecord::enregistrer(second,&error)); const auto removedId=second.storageId; QVERIFY(removedId>id);
        auto other=MedicalRecord::examples()[1]; QVERIFY(MedicalRecord::enregistrer(other,&error));
        MedicalRecordDetailsDialog details; details.setMedicalRecord(first);
        QCOMPARE(details.findChild<QTableWidget *>("historyTable")->rowCount(),2);
        QVERIFY(MedicalRecord::supprimer(second.storageId,&error)); QVERIFY(MedicalRecord::supprimer(other.storageId,&error));
        auto third=first; third.storageId=0; QVERIFY(MedicalRecord::enregistrer(third,&error)); QVERIFY(third.storageId>other.storageId);
        QVERIFY(db.initialize(path)); const auto saved=MedicalRecord::afficher(&error); QCOMPARE(saved.size(),2);
        QCOMPARE(saved.last().medical.diagnosis,QString("Diagnostic conservé"));
        const auto html=MedicalRecord::reportHtml({first},true); QVERIFY(html.contains("&lt;animal&gt; &amp; ami")); QVERIFY(html.contains("DÉMONSTRATION")); QVERIFY(!html.contains("post-opératoire"));
        const auto pdfPath=folder.filePath("medical.pdf"); QVERIFY2(Invoice::writeDocument(pdfPath,html,"Test medical",&error),qPrintable(error));
        QPdfDocument pdf; QCOMPARE(pdf.load(pdfPath),QPdfDocument::Error::None); QCOMPARE(pdf.pageCount(),1);
        const auto text=pdf.getAllText(0).text(); QVERIFY(text.contains("Diagnostic conservé")); QVERIFY(text.contains(number));
        pdf.close(); QVERIFY(db.initialize(":memory:"));
    }
    void medicalIntegratedCrudAndScreens() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QString error;
        QVERIFY(MedicalRecord::seedExamples(&error)); QCOMPARE(MedicalRecord::afficher().size(),8);
        QVERIFY(MedicalRecord::seedExamples(&error)); QCOMPARE(MedicalRecord::afficher().size(),8);
        MainWindow main(true); main.resize(1400,900); main.setSession("medical",Accounts::Role::Medical); main.show();
        Motion::instance().setEnabled(false); QTest::qWait(100);
        auto *page=main.findChild<MedicalWidget *>(); QVERIFY(page);
        auto *stack=main.findChild<QStackedWidget *>("pageStack"); QCOMPARE(stack->currentIndex(),5);
        main.navigate(2); QCOMPARE(stack->currentIndex(),5); QVERIFY(!stack->widget(2)->isEnabled());
        auto *search=page->findChild<QLineEdit *>("medicalSearch");
        auto *species=page->findChild<QComboBox *>("medicalSpecies"); auto *vacc=page->findChild<QComboBox *>("medicalVaccination");
        auto cards=[&]{QVector<RecordCard *> visible;for(auto *card:page->findChildren<RecordCard *>())if(!card->isHidden())visible.append(card);return visible;};
        QCOMPARE(cards().size(),8); species->setCurrentText("Chien"); vacc->setCurrentText("À renouveler"); QCOMPARE(cards().size(),1);
        search->setText("Pierre"); QCOMPARE(cards().size(),1); search->setText("introuvable"); QCOMPARE(cards().size(),0);
        QVERIFY(!main.findChild<QPushButton *>("medicalExport")->isEnabled());
        page->findChild<QPushButton *>("medicalReset")->click(); QCOMPARE(cards().size(),8);
        const auto output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR"); if(!output.isEmpty()) QDir().mkpath(output);
        auto *file=main.findChild<QPushButton *>("medicalExport");
        auto *motion=main.findChild<QPushButton *>("motionToggle");
        QVERIFY(file->isVisible());QVERIFY(!file->geometry().intersects(motion->geometry()));
        auto *selectedPanel=page->findChild<QWidget *>("placeholderText")->parentWidget();
        QVERIFY(selectedPanel->height()<=160);
        QVERIFY(selectedPanel->mapTo(page,QPoint()).y()<search->mapTo(page,QPoint()).y());
        if(!output.isEmpty()) QVERIFY(main.grab().save(output+"/medical-reference-unselected.png"));
        QTest::mouseClick(cards().first(),Qt::LeftButton); QTest::qWait(50);
        if(!output.isEmpty()) QVERIFY(main.grab().save(output+"/medical-overview.png"));
        main.resize(1100,700); QTest::qWait(80);
        QVERIFY(page->geometry().right()<=stack->width());
        auto *cardScroll=page->findChild<QScrollArea *>("medicalCardsScroll");
        for(auto *card:cards()) QVERIFY(card->geometry().right()<=cardScroll->viewport()->width());
        if(!output.isEmpty()) QVERIFY(main.grab().save(output+"/medical-small.png"));
        main.resize(1400,900);
        int forms=0; bool historySeen=false,statsSeen=false,pdfSeen=false;
        QTimer automation; automation.setInterval(30);
        connect(&automation,&QTimer::timeout,&main,[&]{
            if(auto *message=qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {if(auto *yes=message->button(QMessageBox::Yes)) yes->click();else message->reject();return;}
            auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget());if(!dialog)return;
            if(auto *form=qobject_cast<MedicalRecordDialog *>(dialog)) {
                form->findChild<QLineEdit *>("medicalAnimalId")->setText("ANI-TEST");
                form->findChild<QLineEdit *>("medicalAnimalName")->setText(forms?"Test modifié":"Test intégré");
                form->findChild<QLineEdit *>("medicalOwner")->setText("Propriétaire test");
                form->findChild<QTextEdit *>("medicalReason")->setPlainText("Visite de contrôle");
                QVERIFY(form->findChild<QLineEdit *>("medicalRecordId")->isReadOnly());
                if(!output.isEmpty()) form->grab().save(output+"/medical-form.png");
                ++forms; form->findChild<QPushButton *>("primaryButton")->click();
            } else if(auto *details=qobject_cast<MedicalRecordDetailsDialog *>(dialog)) {
                historySeen=true; QCOMPARE(details->findChild<QTableWidget *>("historyTable")->rowCount(),1);
                if(!output.isEmpty()) details->grab().save(output+"/medical-details.png");
                details->accept();
            } else if(dialog->objectName()=="medicalStatisticsDialog") {
                statsSeen=true; QCOMPARE(dialog->findChild<QTableWidget *>("medicalStatsTable")->rowCount(),4);
                if(!output.isEmpty()) dialog->grab().save(output+"/medical-statistics.png");
                dialog->accept();
            } else if(dialog->objectName()=="medicalPdfPreview") {
                pdfSeen=true; QVERIFY(dialog->findChild<QTextBrowser *>()->toPlainText().contains("Test modifié"));
                if(!output.isEmpty()) dialog->grab().save(output+"/medical-pdf-preview.png");
                dialog->reject();
            }
        });
        automation.start(); page->addRecord(); QCOMPARE(forms,1); QCOMPARE(MedicalRecord::afficher().size(),9);
        page->findChild<QPushButton *>("medicalEdit")->click(); QCOMPARE(forms,2);
        search->setText("Test modifié"); QCOMPARE(cards().size(),1);
        auto record=MedicalRecord::afficher().first(); QCOMPARE(record.animal.name,QString("Test modifié"));const auto originalStatus=record.medical.vaccinationStatus;
        page->findChild<QPushButton *>("medicalToggle")->click(); QVERIFY(MedicalRecord::afficher().first().medical.vaccinationStatus!=originalStatus);
        page->findChild<QPushButton *>("medicalView")->click(); QVERIFY(historySeen);
        page->findChild<QPushButton *>("medicalStatistics")->click(); QVERIFY(statsSeen);
        page->findChild<QPushButton *>("medicalRecordPdf")->click(); QVERIFY(pdfSeen);
        if(!output.isEmpty()) {const auto html=MedicalRecord::reportHtml({MedicalRecord::afficher().first()},true);QVERIFY(Invoice::writeDocument(output+"/medical-dossier.pdf",html,"Dossier médical",&error));}
        page->findChild<QPushButton *>("medicalDelete")->click(); QCOMPARE(MedicalRecord::afficher().size(),8);QCOMPARE(cards().size(),0);
        automation.stop();Motion::instance().setEnabled(true);
    }
    void initTestCase() {
        if(QGuiApplication::platformName()=="offscreen") {
            for(const auto &font:{"segoeui.ttf","seguisb.ttf","segoeuib.ttf"})
                QVERIFY(QFontDatabase::addApplicationFont("C:/Windows/Fonts/"+QString(font))>=0);
        }
        qApp->setFont(QFont("Segoe UI",10));
        QLocale::setDefault(QLocale(QLocale::French,QLocale::Tunisia));
        Design::applyTheme();
        Motion::instance().setEnabled(true);
    }
    void loginSessionAndSavedServices() {
        QTemporaryDir temporary;
        auto &db=DatabaseManager::instance();
        const QString path=temporary.path()+"/services.sqlite";
        QVERIFY(db.initialize(path));
        Service existing; existing.setName("Service conservé"); QVERIFY(db.addService(existing));
        AppController controller(false,false); controller.start();
        auto *login=controller.loginWindow(); QVERIFY(login->isVisible());
        QVERIFY(!controller.mainWindow()); QVERIFY(!controller.isSignedIn());
        auto *user=login->findChild<QLineEdit *>("loginUsername");
        auto *password=login->findChild<QLineEdit *>("loginPassword");
        auto *submit=login->findChild<QPushButton *>("signInButton");
        QSignalSpy accepted(login,&LoginWindow::loginSucceeded);
        submit->click(); QVERIFY(login->findChild<QLabel *>("message")->isVisible());
        user->setText("admin"); password->setText("wrongpassword"); submit->click();
        QCOMPARE(accepted.count(),0); QVERIFY(!controller.mainWindow());
        password->setText("admin123"); QTest::keyClick(password,Qt::Key_Return);
        auto *main=controller.mainWindow(); QVERIFY(main); QVERIFY(main->isVisible());
        QVERIFY(controller.isSignedIn()); QVERIFY(!login->isVisible()); QVERIFY(password->text().isEmpty());
        QCOMPARE(accepted.count(),1); submit->click(); QCOMPARE(accepted.count(),1);
        QVERIFY(main->findChild<QPushButton *>("navServices")->isChecked());
        QCOMPARE(main->findChild<QTableView *>("serviceTable")->model()->rowCount(),1);
        QVERIFY(main->findChild<QLabel *>("signedInUser")->text().contains("admin"));
        bool dialogOpened=false;
        QTimer::singleShot(20,[&dialogOpened] {
            auto *dialog=visibleServiceForm();
            if(!dialog) return;
            dialogOpened=true;
            dialog->findChild<QLineEdit *>("serviceName")->setText("Ajout après connexion");
            dialog->findChild<QLineEdit *>("serviceClient")->setText("Client test");
            dialog->findChild<QPushButton *>("saveService")->click();
        });
        main->findChild<QPushButton *>("addService")->click();
        QCOMPARE(main->findChild<QStackedWidget *>("pageStack")->currentIndex(),5);
        QVERIFY(!visibleServiceForm());QVERIFY(openMedicalService(*main));QTRY_VERIFY(dialogOpened);
        QCOMPARE(db.getAllServices().size(),2);
        main->findChild<QPushButton *>("logoutButton")->click();
        QVERIFY(!controller.isSignedIn()); QVERIFY(!main->isVisible()); QVERIFY(login->isVisible());
        QVERIFY(password->text().isEmpty()); QVERIFY(user->text().isEmpty()); QVERIFY(submit->isEnabled());
        // Logout and login never initialize or replace the service database.
        QVERIFY(db.initialize(path)); QCOMPARE(db.getAllServices().size(),2);
        user->setText("admin"); password->setText("admin123"); submit->click();
        QCOMPARE(controller.mainWindow(),main); QVERIFY(main->isVisible()); QCOMPARE(accepted.count(),2);
        QCOMPARE(main->findChild<QTableView *>("serviceTable")->model()->rowCount(),2);
        QVERIFY(db.initialize(":memory:"));
    }
    void logoutMotionAndImmediateReturn() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        Motion::instance().setEnabled(true);
        AppController controller(true,false); controller.start();
        auto *login=controller.loginWindow();
        auto signIn=[login] {
            login->findChild<QLineEdit *>("loginUsername")->setText("admin");
            login->findChild<QLineEdit *>("loginPassword")->setText("admin123");
            login->findChild<QPushButton *>("signInButton")->click();
        };
        signIn(); auto *main=controller.mainWindow(); QVERIFY(main);
        main->resize(1100,700); QTest::qWait(240);
        auto *logout=main->findChild<QPushButton *>("logoutButton"); QVERIFY(logout);
        auto *top=main->findChild<QFrame *>("topbar");
        QVERIFY(top->isAncestorOf(logout));
        QVERIFY(top->rect().contains(QRect(logout->mapTo(top,QPoint()),logout->size())));
        const QRect restGeometry=logout->geometry();
        QEnterEvent enter(QPointF(20,20),QPointF(20,20),QPointF(20,20));
        QApplication::sendEvent(logout,&enter); QTest::qWait(180);
        QCOMPARE(logout->geometry(),restGeometry);
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(!output.isEmpty()) {
            QVERIFY(QDir().mkpath(output+"/logout-motion"));
            QVERIFY(main->grab().save(output+"/services-logout-button.png"));
        }
        logout->click();
        QVERIFY(!controller.isSignedIn()); QVERIFY(!main->isVisible()); QVERIFY(login->isVisible());
        QVERIFY(login->findChild<QLineEdit *>("loginPassword")->text().isEmpty());
        QCOMPARE(db.getAllServices().size(),7);
        auto *overlay=login->findChild<QWidget *>("logoutTransition"); QVERIFY(overlay);
        QVERIFY(overlay->isVisible()); QVERIFY(overlay->testAttribute(Qt::WA_TransparentForMouseEvents));
        QVERIFY(!login->findChild<QStackedWidget *>("pages")->graphicsEffect());
        auto *animation=overlay->findChild<QVariantAnimation *>("logoutMotion"); QVERIFY(animation);
        QCOMPARE(animation->state(),QAbstractAnimation::Running);
        // Deterministic visual frames at the start and end of the painted transition.
        animation->pause(); animation->setCurrentTime(100);
        const QImage start=overlay->grab().toImage();
        if(!output.isEmpty()) {
            QVERIFY(login->grab().save(output+"/logout-transition.png"));
            for(int frame=0;frame<13;++frame) {
                animation->setCurrentTime(frame*50);
                QVERIFY(login->grab().save(output+QString("/logout-motion/frame_%1.png").arg(frame,2,10,QChar('0'))));
            }
        }
        animation->setCurrentTime(490); QVERIFY(overlay->grab().toImage()!=start);
        animation->resume(); QTest::qWait(190);
        QVERIFY(!login->findChild<QWidget *>("logoutTransition"));
        QVERIFY(login->findChild<QLabel *>("message")->text().contains("signed out"));
        if(!output.isEmpty()) QVERIFY(login->grab().save(output+"/logout-complete.png"));
        // Re-login during the decorative transition must work immediately.
        signIn(); logout->click(); QVERIFY(login->findChild<QWidget *>("logoutTransition"));
        signIn(); QVERIFY(controller.isSignedIn()); QVERIFY(main->isVisible());
        QTest::qWait(30); QVERIFY(!login->findChild<QWidget *>("logoutTransition"));
        // Disabling motion cancels a running transition and skips future ones.
        logout->click(); Motion::instance().setEnabled(false); QTest::qWait(20);
        QVERIFY(!login->findChild<QWidget *>("logoutTransition"));
        signIn(); logout->click(); QVERIFY(!login->findChild<QWidget *>("logoutTransition"));
        QVERIFY(login->isVisible()); QVERIFY(!controller.isSignedIn());
        Motion::instance().setEnabled(true);
    }
    void loginRecoveryAndTransitions() {
        LoginWindow login(true,false); login.resize(880,620); login.show(); QTest::qWait(240);
        auto *pages=login.findChild<QStackedWidget *>("pages"); QVERIFY(!pages->graphicsEffect());
        auto *forgot=login.findChild<QPushButton *>("forgot");
        auto *contact=login.findChild<QLineEdit *>("recoveryContact");
        auto *code=login.findChild<QLineEdit *>("recoveryCode");
        auto *newPassword=login.findChild<QLineEdit *>("recoveryPassword");
        auto *confirm=login.findChild<QLineEdit *>("recoveryConfirm");
        auto *generate=login.findChild<QPushButton *>("secondaryBtn");
        auto *reset=login.findChild<QPushButton *>("resetPasswordButton");
        forgot->click(); QCOMPARE(pages->currentIndex(),1); QVERIFY(!reset->isEnabled());
        contact->setText("invalid"); generate->click(); QVERIFY(!code->isEnabled());
        contact->setText("demo@example.com"); generate->click(); QVERIFY(code->isEnabled());
        const QString message=login.findChild<QLabel *>("recoveryMessage")->text();
        QVERIFY(message.contains("No message was sent"));
        const auto match=QRegularExpression("Code: ([0-9]{6})").match(message); QVERIFY(match.hasMatch());
        code->setText("000000"); newPassword->setText("updated123"); confirm->setText("updated123");
        reset->click(); QCOMPARE(pages->currentIndex(),1);
        code->setText(match.captured(1)); confirm->setText("mismatch"); reset->click(); QCOMPARE(pages->currentIndex(),1);
        confirm->setText("updated123"); reset->click(); QCOMPARE(pages->currentIndex(),0);
        QVERIFY(code->text().isEmpty()); QVERIFY(newPassword->text().isEmpty()); QVERIFY(confirm->text().isEmpty());
        QCOMPARE(login.findChild<QLineEdit *>("loginUsername")->text(),QString("demo@example.com"));
        QSignalSpy accepted(&login,&LoginWindow::loginSucceeded);
        login.findChild<QLineEdit *>("loginPassword")->setText("updated123");
        login.findChild<QPushButton *>("signInButton")->click(); QCOMPARE(accepted.count(),1);
        login.prepareForLogin();
        for(int i=0;i<4;++i) { forgot->click(); login.findChild<QPushButton *>("linkBtn")->click(); }
        QTest::qWait(250); QVERIFY(!pages->graphicsEffect()); QCOMPARE(pages->currentIndex(),0);
        QVERIFY(pages->findChildren<QWidget *>("motionReveal",Qt::FindDirectChildrenOnly).isEmpty());
        Motion::instance().setEnabled(false);
        QVERIFY(login.findChild<QPushButton *>("motionToggle")->isChecked());
        forgot->click(); QVERIFY(pages->findChildren<QWidget *>("motionReveal",Qt::FindDirectChildrenOnly).isEmpty());
        Motion::instance().setEnabled(true);
        QVERIFY(!login.findChild<QPushButton *>("motionToggle")->isChecked());
    }
    void loginRememberUsernameOnly() {
        QTemporaryDir temporary; QVERIFY(temporary.isValid());
        const auto oldFormat=QSettings::defaultFormat();
        struct RestoreFormat { QSettings::Format value; ~RestoreFormat() { QSettings::setDefaultFormat(value); } } restore{oldFormat};
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,temporary.path());
        QSettings isolated(QSettings::IniFormat,QSettings::UserScope,"ANIWORLD","ANIWORLD");
        QVERIFY(isolated.fileName().startsWith(temporary.path()));
        {
            LoginWindow login(false,true);
            login.findChild<QLineEdit *>("loginUsername")->setText("admin");
            login.findChild<QLineEdit *>("loginPassword")->setText("admin123");
            login.findChild<QCheckBox *>("rememberUsername")->setChecked(true);
            login.findChild<QPushButton *>("signInButton")->click();
            QSettings stored(QSettings::IniFormat,QSettings::UserScope,"ANIWORLD","ANIWORLD");
            QCOMPARE(stored.allKeys(),QStringList{"login/username"});
            QCOMPARE(stored.value("login/username").toString(),QString("admin"));
        }
        LoginWindow reopened(false,true);
        QCOMPARE(reopened.findChild<QLineEdit *>("loginUsername")->text(),QString("admin"));
        QVERIFY(reopened.findChild<QLineEdit *>("loginPassword")->text().isEmpty());
        reopened.findChild<QCheckBox *>("rememberUsername")->setChecked(false);
        reopened.findChild<QLineEdit *>("loginPassword")->setText("admin123");
        reopened.findChild<QPushButton *>("signInButton")->click();
        QSettings stored(QSettings::IniFormat,QSettings::UserScope,"ANIWORLD","ANIWORLD"); QVERIFY(stored.allKeys().isEmpty());
    }
    void captureLoginIntegration() {
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(output.isEmpty()) QSKIP("Set ANIWORLD_CAPTURE_DIR for login captures.");
        QVERIFY(QDir().mkpath(output));
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        AppController controller(true,false); controller.start();
        auto *login=controller.loginWindow(); login->resize(1080,760); QTest::qWait(250);
        QVERIFY(login->grab().save(output+"/login.png"));
        login->resize(880,620); QTest::qWait(50);
        QVERIFY(login->grab().save(output+"/login-small.png"));
        for(const auto &name:{"loginUsername","loginPassword","signInButton","closeControl"}) {
            auto *field=login->findChild<QWidget *>(name); QVERIFY(field->isVisible());
            QVERIFY(login->rect().contains(QRect(field->mapTo(login,QPoint()),field->size())));
        }
        login->findChild<QPushButton *>("signInButton")->click(); QTest::qWait(50);
        QVERIFY(login->grab().save(output+"/login-error-small.png"));
        login->resize(1080,760); login->findChild<QPushButton *>("forgot")->click(); QTest::qWait(250);
        QVERIFY(login->grab().save(output+"/login-recovery.png"));
        login->findChild<QPushButton *>("linkBtn")->click();
        login->findChild<QLineEdit *>("loginUsername")->setText("admin");
        login->findChild<QLineEdit *>("loginPassword")->setText("admin123");
        login->findChild<QPushButton *>("signInButton")->click();
        auto *main=controller.mainWindow(); QVERIFY(main); main->resize(1400,900); QTest::qWait(550);
        QVERIFY(main->grab().save(output+"/login-services.png"));
        main->resize(1100,700); QTest::qWait(250);
        auto *logout=main->findChild<QPushButton *>("logoutButton"); QVERIFY(logout->isVisible());
        QVERIFY(main->rect().contains(QRect(logout->mapTo(main,QPoint()),logout->size())));
        QVERIFY(main->grab().save(output+"/login-services-small.png"));
        main->navigate(0); QTest::qWait(550);
        QVERIFY(main->grab().save(output+"/login-dashboard-small.png"));
        logout->click(); QTest::qWait(100); QVERIFY(login->isVisible());
        QVERIFY(login->grab().save(output+"/login-after-logout.png"));
    }
    void stockColorsIgnoreSystemAccent() {
        // Reproduce the dark purple alternate rows seen with the Windows palette.
        const QPalette original=qApp->palette();
        const auto restore=qScopeGuard([original] { qApp->setPalette(original); });
        QPalette dark=original;
        dark.setColor(QPalette::Base,QColor("#170B20"));
        dark.setColor(QPalette::AlternateBase,QColor("#39004D"));
        dark.setColor(QPalette::Highlight,QColor("#600080"));
        dark.setColor(QPalette::Text,Qt::white);
        qApp->setPalette(dark);
        StockWidget stock; stock.resize(1100,750);
        auto *table=stock.findChild<QTableView *>("stockTable"); QVERIFY(table);
        stock.show(); QTest::qWait(80);
        QTest::mouseMove(&stock,QPoint(10,10));
        auto rowColor=[table](int row) {
            QImage frame(table->viewport()->size(),QImage::Format_ARGB32_Premultiplied);
            frame.fill(Qt::transparent); table->viewport()->render(&frame);
            const QRect cell=table->visualRect(table->model()->index(row,0));
            return frame.pixelColor(cell.right()-12,cell.top()+5);
        };
        QCOMPARE(rowColor(0),QColor("#FFFFFF"));
        QCOMPARE(rowColor(1),QColor("#F4F9FA"));
        auto lightSelection=[&] {
            const QColor actual=rowColor(1),expected("#DDEEF1");
            // Fusion adds a subtle gradient to the focused selection.
            return qAbs(actual.red()-expected.red())<=3
                && qAbs(actual.green()-expected.green())<=3
                && qAbs(actual.blue()-expected.blue())<=3;
        };
        table->selectRow(1); table->setFocus(); QTest::qWait(20);
        QVERIFY2(lightSelection(),qPrintable(rowColor(1).name()));
        stock.findChild<QLineEdit *>("stockSearch")->setFocus(); QTest::qWait(20);
        QVERIFY2(lightSelection(),qPrintable(rowColor(1).name()));
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(!output.isEmpty()) { QVERIFY(QDir().mkpath(output)); QVERIFY(stock.grab().save(output+"/stock-colors.png")); }
    }
    void stockFiltersAndMovements() {
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
        QTimer::singleShot(20,[] { if(auto *box=qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) box->button(QMessageBox::Yes)->click(); });
        stock.findChild<QPushButton *>("stockDeleteButton")->click(); QCOMPARE(model->rowCount(),18);
        bool stats=false;
        QTimer::singleShot(30,[&] { if(auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
            stats=dialog->objectName()=="stockStatisticsDialog";
            if(!output.isEmpty()) dialog->grab().save(output+"/stock-statistics.png");
            dialog->reject();
        } });
        stock.findChild<QPushButton *>("stockStatisticsButton")->click(); QVERIFY(stats);
    }
    void stockIntegratedNavigationAndLogout() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        AppController controller(false,false); controller.start(); auto *login=controller.loginWindow();
        login->findChild<QLineEdit *>("loginUsername")->setText("admin");
        login->findChild<QLineEdit *>("loginPassword")->setText("admin123");
        login->findChild<QPushButton *>("signInButton")->click();
        auto *main=controller.mainWindow(); QVERIFY(main);
        auto *stack=main->findChild<QStackedWidget *>("pageStack"); QCOMPARE(stack->count(),6);
        auto *stock=main->findChild<StockWidget *>(); QVERIFY(stock); QVERIFY(!stock->isWindow());
        auto *nav=main->findChild<QPushButton *>("navStock"); QVERIFY(nav); QVERIFY(nav->isEnabled());
        nav->click(); QTest::qWait(250); QCOMPARE(stack->currentWidget(),stock); QVERIFY(nav->isChecked());
        QVERIFY(main->findChild<QLabel *>("storageBadge")->text().contains("Stock"));
        stock->findChild<QLineEdit *>("stockSearch")->setText("vaccin");
        main->navigate(0); main->navigate(1); main->navigate(2); QTest::qWait(250);
        QCOMPARE(stock->findChild<QTableView *>("stockTable")->model()->rowCount(),2);
        QVERIFY(!main->findChild<ServiceWidget *>()->isVisible()); QVERIFY(!main->findChild<DashboardWidget *>()->isVisible());
        stock->findChild<QLineEdit *>("stockSearch")->clear();
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        for(const QSize size:{QSize(1400,900),QSize(1100,700)}) {
            const QSize target=QGuiApplication::platformName()=="windows"
                ?size.boundedTo(main->screen()->availableGeometry().size()-QSize(40,60)).expandedTo(main->minimumSize()):size;
            main->resize(target); QTest::qWait(200); QCOMPARE(main->size(),target);
            for(const auto &name:{"stockMovementButton","stockExportButton","stockEditButton","stockExpiryButton"}) {
                auto *button=stock->findChild<QPushButton *>(name); QVERIFY(button->isVisible());
                QVERIFY(stock->rect().contains(QRect(button->mapTo(stock,QPoint()),button->size())));
            }
            if(!output.isEmpty()) { QVERIFY(QDir().mkpath(output)); QVERIFY(main->grab().save(output+(size.width()==1400?"/stock-integrated.png":"/stock-small.png"))); }
        }
        main->navigate(1); QVERIFY(main->findChild<QLabel *>("storageBadge")->text().contains("Sauvegarde"));
        QCOMPARE(main->findChild<QTableView *>("serviceTable")->model()->rowCount(),7); QCOMPARE(db.getAllServices().size(),7);
        main->navigate(2); main->findChild<QPushButton *>("logoutButton")->click();
        QVERIFY(login->isVisible()); QVERIFY(!main->isVisible()); QVERIFY(!controller.isSignedIn());
        login->findChild<QLineEdit *>("loginUsername")->setText("admin");
        login->findChild<QLineEdit *>("loginPassword")->setText("admin123");
        login->findChild<QPushButton *>("signInButton")->click();
        QCOMPARE(stack->currentIndex(),1); nav->click(); QCOMPARE(stack->currentWidget(),stock);
    }
    void invoiceMigrationPersistenceAndPdf() {
        QTemporaryDir temporary; const QString path=temporary.path()+"/legacy.sqlite";
        {
            auto legacy=QSqlDatabase::addDatabase("QSQLITE","legacy-fixture"); legacy.setDatabaseName(path); QVERIFY(legacy.open());
            QSqlQuery query(legacy);
            QVERIFY(query.exec("CREATE TABLE services(id INTEGER PRIMARY KEY AUTOINCREMENT,name TEXT,description TEXT,responsible TEXT,category TEXT,dateTime TEXT,status TEXT,duration INTEGER,price REAL)"));
            QVERIFY(query.exec("INSERT INTO services VALUES(42,'Consultation','Bilan général','Dr. Amara','Soins & bien-être','2026-10-10T10:30:00','Terminé',30,50.5)")); legacy.close();
        }
        QSqlDatabase::removeDatabase("legacy-fixture");
        auto &db=DatabaseManager::instance(); QVERIFY2(db.initialize(path),qPrintable(db.lastError()));
        auto service=db.getAllServices().first(); QCOMPARE(service.getId(),42); QCOMPARE(service.getPrice(),50.5);
        QCOMPARE(service.getInvoiceNumber(),QString("FAC-2026-000042")); QVERIFY(service.getInvoiceDate().isValid()); QVERIFY(service.getClientName().isEmpty());
        QString error; const QString pdfPath=temporary.path()+"/invoice.pdf";
        QVERIFY(!Invoice::writePdf(pdfPath,service,false,&error)); QVERIFY(!QFile::exists(pdfPath));
        const auto invoiceDate=service.getInvoiceDate(); service.setClientName("Inès Ben Salah & famille"); service.setDescription("Bilan général\nContrôle des griffes\nConseils de prévention");
        service.setDateTime(service.getDateTime().addYears(1)); QVERIFY(db.updateService(service));
        QVERIFY(db.initialize(path)); service=db.getAllServices().first();
        QCOMPARE(service.getInvoiceNumber(),QString("FAC-2026-000042")); QCOMPARE(service.getInvoiceDate(),invoiceDate); QCOMPARE(service.getClientName(),QString("Inès Ben Salah & famille"));
        QVERIFY2(Invoice::writePdf(pdfPath,service,false,&error),qPrintable(error));
        QFile pdf(pdfPath); QVERIFY(pdf.open(QIODevice::ReadOnly)); QVERIFY(pdf.read(8).startsWith("%PDF-")); pdf.close();
        QVERIFY(!Invoice::writePdf(temporary.path()+"/missing/invoice.pdf",service,false,&error)); QVERIFY(!error.isEmpty());
        auto cancelled=service; cancelled.setStatus("Annulé"); QVERIFY(!Invoice::writePdf(pdfPath,cancelled,false,&error));
        auto escaped=service; escaped.setClientName("<img src=x> & client"); QVERIFY(Invoice::html(escaped).contains("&lt;img src=x&gt; &amp; client"));
        Service next=service; next.setId(0); QVERIFY(db.addService(next));
        const auto services=db.getAllServices(); QCOMPARE(services.size(),2); QVERIFY(services[0].getInvoiceNumber()!=services[1].getInvoiceNumber());
        QVERIFY(db.initialize(path)); QCOMPARE(db.getAllServices().size(),2);
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(!output.isEmpty()) {
            QVERIFY(QDir().mkpath(output));
            QVERIFY(Invoice::writePdf(output+"/facture-exemple.pdf",service,true,&error));
            auto longService=service; longService.setClientName(QString(120,'W')); longService.setName(QString(120,'M'));
            longService.setDescription(QString("Prestation détaillée avec soins et suivi. ").repeated(115));
            QVERIFY(Invoice::writePdf(output+"/facture-longue.pdf",longService,true,&error));
            StockWidget stock; QVERIFY(stock.exportInventoryPdf(output+"/stock-inventaire.pdf",&error));
            stock.findChild<QComboBox *>("stockCategoryFilter")->setCurrentText("Chats"); stock.findChild<QLineEdit *>("stockSearch")->setText("vaccin");
            QVERIFY(stock.exportInventoryPdf(output+"/stock-filtre.pdf",&error));
        }
        QVERIFY(db.initialize(":memory:"));
    }
    void invoicePreviewAndRemovedFeatures() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        MainWindow window(true); window.resize(1400,900); window.show(); QTest::qWait(300);
        QVERIFY(!window.findChild<QPushButton *>("navRh")); QCOMPARE(window.findChild<QStackedWidget *>("pageStack")->count(),6);
        QVERIFY(!window.findChild<QPushButton *>("duplicateService"));
        QVERIFY(!window.findChild<QPushButton *>("exportServices")); QVERIFY(window.findChild<QPushButton *>("servicePlanning"));
        auto *table=window.findChild<QTableView *>("serviceTable"); table->selectRow(1);
        bool previewOpened=false; const auto output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        QTimer::singleShot(100,[&previewOpened,output] {
            auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            auto *preview=dialog->findChild<QTextBrowser *>("invoicePreview");
            previewOpened=preview && preview->toPlainText().contains("TOTAL À PAYER") && preview->toPlainText().contains("FAC-");
            if(!output.isEmpty()) dialog->grab().save(output+"/facture-apercu.png");
            dialog->reject();
        });
        window.findChild<QPushButton *>("invoiceService")->click(); QVERIFY(previewOpened);
        auto service=db.getAllServices().at(1); service.setClientName(""); QVERIFY(db.updateService(service)); window.findChild<ServiceWidget *>()->refresh();
        window.findChild<QPushButton *>("invoiceService")->click(); QVERIFY(!QApplication::activeModalWidget());
        QVERIFY(window.findChild<QLabel *>("serviceNotice")->text().contains("client"));
    }
    void tariffAndDurationOrdering() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:"));
        for(int i=0;i<4;++i) { Service s; s.setName(QString("Soin %1").arg(i)); s.setPrice(QList<double>{0,100,9.5,9.5}[i]); s.setDuration(QList<int>{60,5,30,45}[i]); QVERIFY(db.addService(s)); }
        ServiceWidget page; auto *sort=page.findChild<QComboBox *>("sortServices"); auto *table=page.findChild<QTableView *>("serviceTable");
        const auto all=db.getAllServices();
        for(int mode:{3,4,5,6}) {
            sort->setCurrentIndex(mode); double previous=(mode==3 || mode==5)?-1:1e9;
            for(int row=0;row<table->model()->rowCount();++row) {
                const int id=table->model()->index(row,0).data().toInt();
                for(const auto &s:all) if(s.getId()==id) {
                    const double value=mode<5?s.getPrice():s.getDuration();
                    QVERIFY((mode==3 || mode==5)?value>=previous:value<=previous); previous=value;
                }
            }
        }
    }
    void storageCrudAndPersistence() {
        QTemporaryDir temp; QVERIFY(temp.isValid());
        auto &db=DatabaseManager::instance(); const QString path=temp.path()+"/services.sqlite";
        QVERIFY(db.initialize(path)); QCOMPARE(db.getAllServices().size(),0);
        Service s; s.setName("   "); QVERIFY(!db.addService(s));
        s.setName("Toilettage d'été; \"Milo\""); s.setResponsible("Élodie"); s.setPrice(42.5); s.setDescription("Ligne 1\nLigne 2");
        QVERIFY(db.addService(s)); auto saved=db.getAllServices().first(); QVERIFY(saved.getId()>0); QCOMPARE(saved.getName(),s.getName()); QCOMPARE(saved.getPrice(),42.5);
        saved.setStatus("Terminé"); saved.setPrice(55); QVERIFY(db.updateService(saved));
        QVERIFY(db.initialize(path)); const auto reopened=db.getAllServices(); QCOMPARE(reopened.size(),1); QCOMPARE(reopened.first().getStatus(),QString("Terminé")); QCOMPARE(reopened.first().getResponsible(),QString("Élodie")); QCOMPARE(reopened.first().getDescription(),s.getDescription()); QCOMPARE(reopened.first().getPrice(),55.0);
        saved.setDuration(0); QVERIFY(!db.updateService(saved)); saved.setDuration(30); saved.setStatus("invalid"); QVERIFY(!db.updateService(saved));
        QVERIFY(db.deleteService(saved.getId())); QVERIFY(db.getAllServices().isEmpty()); QVERIFY(!db.deleteService(saved.getId()));
        QVERIFY(db.initialize(":memory:"));
    }
    void formValidation() {
        ServiceDialog dialog; dialog.show();
        auto *name=dialog.findChild<QLineEdit *>("serviceName"); auto *save=dialog.findChild<QPushButton *>("saveService");
        QVERIFY(name); QVERIFY(save); name->setText("    "); save->click();
        QCOMPARE(dialog.result(),0); QVERIFY(dialog.isVisible()); QVERIFY(dialog.findChild<QLabel *>("validationError")->isVisible());
        dialog.findChild<QLineEdit *>("serviceResponsible")->setText("Sarra"); name->setText("  Soin des griffes  "); save->click();
        QCOMPARE(dialog.result(),0); dialog.findChild<QLineEdit *>("serviceClient")->setText("Nour Ben Salah"); save->click();
        QCOMPARE(dialog.result(),int(QDialog::Accepted)); QCOMPARE(dialog.getService().getName(),QString("Soin des griffes")); QCOMPARE(dialog.getService().getResponsible(),QString("Sarra"));
    }
    void filterSortAndRefresh() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        ServiceWidget page; page.show();
        auto *table=page.findChild<QTableView *>("serviceTable"); auto *search=page.findChild<QLineEdit *>("serviceSearch");
        auto *status=page.findChild<QComboBox *>("statusFilter"); auto *category=page.findChild<QComboBox *>("categoryFilter"); auto *sort=page.findChild<QComboBox *>("sortServices");
        QVERIFY(qobject_cast<QSqlQueryModel *>(table->model()));
        QCOMPARE(table->model()->rowCount(),7); search->setText("education"); QCOMPARE(table->model()->rowCount(),2);
        status->setCurrentText("Planifié"); QCOMPARE(table->model()->rowCount(),1); page.refresh(); QCOMPARE(table->model()->rowCount(),1);
        category->setCurrentText("Toilettage"); QCOMPARE(table->model()->rowCount(),0); QVERIFY(!page.findChild<QPushButton *>("editService")->isEnabled());
        search->clear(); status->setCurrentIndex(0); category->setCurrentIndex(0); sort->setCurrentIndex(3);
        QCOMPARE(table->model()->index(0,1).data().toString(),QString("Soin des griffes"));
        table->selectRow(0); QVERIFY(page.findChild<QPushButton *>("editService")->isEnabled());
        search->setText("Sarra"); QCOMPARE(table->model()->rowCount(),2); QVERIFY(page.findChild<QPushButton *>("editService")->isEnabled());
        QVERIFY(page.findChild<QLabel *>("selectedServiceName")->text().contains("Bain") || page.findChild<QLabel *>("selectedServiceName")->text().contains("Toilettage"));
    }
    void interfaceCrud() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:"));
        MainWindow window; window.show(); QVERIFY(QApplication::activeModalWidget()==nullptr);
        auto *page=window.findChild<ServiceWidget *>(); QVERIFY(page);
        window.findChild<QPushButton *>("navServices")->click();
        QSignalSpy changes(page,&ServiceWidget::servicesChanged);
        QTimer::singleShot(20,[] {
            auto *dialog=visibleServiceForm();
            if(!dialog) return;
            dialog->findChild<QLineEdit *>("serviceName")->setText("Test de création");
            dialog->findChild<QDoubleSpinBox *>("servicePrice")->setValue(18.5);
            dialog->findChild<QLineEdit *>("serviceClient")->setText("Client test");
            dialog->findChild<QPushButton *>("saveService")->click();
        });
        QVERIFY(openMedicalService(window)); QTRY_COMPARE(db.getAllServices().size(),1);window.navigate(1); QCOMPARE(changes.count(),1);
        auto *table=page->findChild<QTableView *>("serviceTable");
        QVERIFY(qobject_cast<QSqlQueryModel *>(table->model()));
        QCOMPARE(table->model()->rowCount(),1);
        QCOMPARE(table->model()->index(0,1).data().toString(),QString("Test de création"));
        table->selectRow(0);
        QTimer::singleShot(20,[] {
            auto *dialog=visibleServiceForm();
            if(!dialog) return;
            dialog->findChild<QLineEdit *>("serviceName")->setText("Service modifié");
            dialog->findChild<QComboBox *>("serviceStatus")->setCurrentText("Terminé");
            dialog->findChild<QLineEdit *>("serviceClient")->setText("Client test");
            dialog->findChild<QPushButton *>("saveService")->click();
        });
        page->findChild<QPushButton *>("editService")->click(); QTRY_COMPARE(db.getAllServices().first().getName(),QString("Service modifié")); QCOMPARE(changes.count(),2);
        QCOMPARE(table->model()->rowCount(),1);
        QCOMPARE(table->model()->index(0,1).data().toString(),QString("Service modifié"));
        QCOMPARE(table->model()->index(0,6).data().toString(),QString("Terminé"));
        window.navigate(0); QCOMPARE(window.findChild<QLabel *>("stat3")->text(),QString("1")); window.navigate(1); table->selectRow(0);
        bool readOnly=false;
        QTimer::singleShot(20,[&readOnly] {
            auto *dialog=visibleServiceForm(); if(!dialog) return;
            readOnly=dialog->findChild<QLineEdit *>("serviceName")->isReadOnly() && !dialog->findChild<QPushButton *>("saveService"); dialog->reject();
        });
        page->findChild<QPushButton *>("viewService")->click(); QTRY_VERIFY(readOnly);
        QTimer::singleShot(20,[] { auto *box=qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if(box) box->reject(); });
        page->findChild<QPushButton *>("deleteService")->click(); QCOMPARE(db.getAllServices().size(),1);
        QCOMPARE(table->model()->rowCount(),1);
        QTimer::singleShot(20,[] {
            auto *box=qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if(!box) return;
            for(auto *button:box->buttons()) if(box->buttonRole(button)==QMessageBox::DestructiveRole) { button->click(); break; }
        });
        page->findChild<QPushButton *>("deleteService")->click(); QCOMPARE(db.getAllServices().size(),0); QCOMPARE(changes.count(),3);
        QCOMPARE(table->model()->rowCount(),0);
        window.navigate(0); QCOMPARE(window.findChild<QLabel *>("stat0")->text(),QString("0"));
    }
    void dateFiltersAndCards() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:"));
        const auto now=QDateTime::currentDateTime();
        const QList<int> days={-1,0,6,7};
        for(int day:days) { Service s; s.setName(QString("Prestation %1").arg(day)); s.setDateTime(now.addDays(day)); QVERIFY(db.addService(s)); }
        ServiceWidget page; page.resize(1172,820); page.show(); QTest::qWait(30);
        auto *table=page.findChild<QTableView *>("serviceTable");
        auto *period=page.findChild<QComboBox *>("periodFilter");
        period->setCurrentIndex(1); QCOMPARE(table->model()->rowCount(),1);
        period->setCurrentIndex(2); QCOMPARE(table->model()->rowCount(),2);
        period->setCurrentIndex(3); QVERIFY(table->model()->rowCount()>=1);
        page.findChild<QPushButton *>("resetFilters")->click(); QCOMPARE(table->model()->rowCount(),4);
        const auto cards=page.findChildren<QAbstractButton *>(QRegularExpression("serviceCard_.*"));
        QCOMPARE(cards.size(),4);
        cards.last()->click(); QVERIFY(cards.last()->isChecked());
        QCOMPARE(page.findChild<QLabel *>("selectedServiceName")->text(),QString("Prestation -1"));
        page.findChild<QPushButton *>("tableView")->click(); QVERIFY(table->isVisible());
        page.findChild<QPushButton *>("cardsView")->click(); QVERIFY(!table->isVisible());
    }
    void quickStatusWithoutDuplication() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:"));
        Service service; service.setName("Service original"); service.setPrice(25); service.setDuration(45);
        QVERIFY(db.addService(service)); const int originalId=db.lastInsertedId();
        ServiceWidget page; page.show();
        auto *advance=page.findChild<QPushButton *>("advanceService");
        advance->click(); QCOMPARE(db.getAllServices().first().getStatus(),QString("En cours"));
        advance->click(); QCOMPARE(db.getAllServices().first().getStatus(),QString("Terminé")); QVERIFY(!advance->isVisible());
        QVERIFY(!page.findChild<QPushButton *>("duplicateService"));
        QCOMPARE(db.getAllServices().size(),1); QCOMPARE(db.getAllServices().first().getId(),originalId);
    }
    void metricsAndPdfReport() {
        const QDateTime now(QDate(2026,10,1),QTime(12,0));
        QList<Service> services;
        for(int i=0;i<4;++i) { Service s; s.setId(i+1); s.setName("Soin <chat> & chien"); s.setPrice(20); s.setDuration(30); s.setDateTime(now.addDays(-1)); s.setStatus(Design::statuses()[i]); services.append(s); }
        const auto metrics=ServiceMetrics::from(services,now);
        QCOMPARE(metrics.total,4); QCOMPARE(metrics.overdue,1); QCOMPARE(metrics.scheduledMinutes,60); QCOMPARE(metrics.completedAmount,20.0);
        QTemporaryDir temporary; QString error;
        QVERIFY2(ServiceReport::writePdf(temporary.path()+"/services.pdf",services,"Test",&error),qPrintable(error));
        QFile pdf(temporary.path()+"/services.pdf"); QVERIFY(pdf.open(QIODevice::ReadOnly)); QVERIFY(pdf.read(8).startsWith("%PDF-"));
        QVERIFY(!ServiceReport::writePdf(temporary.path()+"/missing/report.pdf",services,"Test",&error)); QVERIFY(!error.isEmpty());
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(!output.isEmpty()) {
            QVERIFY(QDir().mkpath(output)); auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
            QVERIFY(ServiceReport::writePdf(output+"/services-report.pdf",db.getAllServices(),"Données de démonstration · Toutes les prestations",&error));
            QList<Service> many;
            for(int i=0;i<70;++i) { Service s=services[i%4]; s.setId(i+1); s.setName(QString("Prestation %1 — Soins de bien-être pour animaux").arg(i+1)); s.setResponsible("Responsable au nom composé"); many.append(s); }
            QVERIFY(ServiceReport::writePdf(output+"/services-report-multipage.pdf",many,"Vérification de pagination",&error));
        }
    }
    void logoHoverAnimation() {
        QVERIFY(DatabaseManager::instance().initialize(":memory:"));
        MainWindow window(true); window.show(); QTest::qWait(40);
        auto *logo=window.findChild<AnimatedLogo *>("animatedBrandLogo"); QVERIFY(logo);
        auto *movie=logo->findChild<QMovie *>("brandHoverMovie"); QVERIFY(movie);
        QVERIFY(movie->isValid()); QVERIFY(movie->frameCount()>1);
        logo->clearFocus();
        QFocusEvent focusOut(QEvent::FocusOut); QApplication::sendEvent(logo,&focusOut);
        QEvent leave(QEvent::Leave); QApplication::sendEvent(logo,&leave); QTest::qWait(250);
        const auto layoutBefore=logo->geometry();
        const auto restingImage=logo->grab().toImage();
        QSignalSpy frames(movie,&QMovie::frameChanged);
        QEnterEvent enter(QPointF(35,35),QPointF(35,35),QPointF(35,35));
        QApplication::sendEvent(logo,&enter); QTest::qWait(320);
        QCOMPARE(movie->state(),QMovie::Running); QVERIFY(frames.size()>3);
        QCOMPARE(logo->geometry(),layoutBefore); QVERIFY(logo->grab().toImage()!=restingImage);
        QApplication::sendEvent(logo,&leave); QTest::qWait(260);
        QVERIFY(movie->state()!=QMovie::Running);
        QCOMPARE(logo->grab().toImage(),restingImage);
        const int pausedFrames=frames.size(); QTest::qWait(100); QCOMPARE(frames.size(),pausedFrames);
        QFocusEvent keyboardFocus(QEvent::FocusIn,Qt::TabFocusReason);
        QApplication::sendEvent(logo,&keyboardFocus); QTest::qWait(60);
        QCOMPARE(movie->state(),QMovie::Running);
        QTest::keyClick(logo,Qt::Key_Space);
        QVERIFY(window.findChild<QPushButton *>("navDashboard")->isChecked());
        window.hide(); QCOMPARE(movie->state(),QMovie::NotRunning);
    }
    void pageAndCardMotion() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        MainWindow window(true); window.show(); QTest::qWait(500);
        auto *card=window.findChild<QAbstractButton *>("serviceCard_4"); QVERIFY(card);
        card->clearFocus();
        QFocusEvent focusOut(QEvent::FocusOut); QApplication::sendEvent(card,&focusOut);
        QEvent leave(QEvent::Leave); QApplication::sendEvent(card,&leave); QTest::qWait(180);
        const auto resting=card->grab().toImage(); const auto geometry=card->geometry();
        QEnterEvent enter(QPointF(20,20),QPointF(20,20),QPointF(20,20));
        QApplication::sendEvent(card,&enter); QTest::qWait(190);
        QVERIFY(resting!=card->grab().toImage()); QCOMPARE(card->geometry(),geometry);
        QApplication::sendEvent(card,&leave); QTest::qWait(190); QCOMPARE(card->grab().toImage(),resting);
        auto *stack=window.findChild<QStackedWidget *>("pageStack"); QVERIFY(stack);
        window.navigate(0); QTest::qWait(20);
        QVERIFY(stack->findChild<QWidget *>("motionReveal",Qt::FindDirectChildrenOnly));
        window.navigate(1); window.navigate(0); window.navigate(1);
        QTRY_VERIFY(!stack->findChild<QWidget *>("motionReveal",Qt::FindDirectChildrenOnly));
        QVERIFY(window.findChild<QPushButton *>("navServices")->isChecked());
        QVERIFY(!stack->graphicsEffect());
    }
    void dashboardPaintsOpaqueDuringNavigation() {
        QVERIFY(DatabaseManager::instance().initialize(":memory:"));
        Motion::instance().setEnabled(true);
        MainWindow window(true); window.resize(1400,900); window.show(); QTest::qWait(500);
        auto *stack=window.findChild<ServiceWidget *>()->parentWidget(); QVERIFY(stack);
        auto verifyBackground=[stack]() {
            // Old backing-store pixels must never show through a new page.
            // Magenta makes this independent of the previous page's contents.
            const qreal dpr=stack->devicePixelRatioF();
            auto paintOver=[stack,dpr](const QColor &previousFrame) {
                QImage surface(QSize(qRound(stack->width()*dpr),qRound(stack->height()*dpr)),QImage::Format_ARGB32_Premultiplied);
                surface.setDevicePixelRatio(dpr); surface.fill(previousFrame);
                stack->render(&surface); return surface;
            };
            // Compare the entire frame, including rounded corners and text,
            // without relying on exact alpha-blending rounding at fractional DPI.
            return paintOver(QColor("#FF00FF"))==paintOver(QColor("#00FF00"));
        };
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(!output.isEmpty()) QVERIFY(QDir().mkpath(output));
        for(bool populated:{false,true}) {
            if(populated) QVERIFY(DatabaseManager::instance().addExamples());
            for(int page:{0,1,2,0,2,1,0}) {
                window.navigate(page); QTest::qWait(35);
                QVERIFY2(verifyBackground(),"The new page depends on previous backing-store pixels.");
                QCOMPARE(window.findChild<ServiceWidget *>()->isVisible(),page==1);
                QCOMPARE(window.findChild<DashboardWidget *>()->isVisible(),page==0);
                QCOMPARE(window.findChild<StockWidget *>()->isVisible(),page==2);
                if(!output.isEmpty() && !populated) {
                    const QString name=QStringList{"dashboard-transition-mid","services-transition-mid","stock-transition-mid","rh-transition-mid"}[page];
                    QVERIFY(window.grab().save(output+"/"+name+".png"));
                    if(QGuiApplication::platformName()=="windows")
                        QVERIFY(window.screen()->grabWindow(window.winId()).save(output+"/"+name+"-native.png"));
                }
            }
        }
        window.resize(1100,700); QTest::qWait(25); QVERIFY(verifyBackground());
        Motion::instance().setEnabled(false); QVERIFY(verifyBackground());
        Motion::instance().setEnabled(true);
        QTest::qWait(550);
        QVERIFY(verifyBackground());
    }
    void motionControlsAndInteractions() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        MainWindow window(true); window.show(); QTest::qWait(500);
        auto *toggle=window.findChild<QPushButton *>("motionToggle"); QVERIFY(toggle); QVERIFY(toggle->isChecked());
        auto *card=window.findChild<QAbstractButton *>("serviceCard_4"); QVERIFY(card);
        QTest::mouseClick(card,Qt::LeftButton);
        QCOMPARE(window.findChild<QLabel *>("selectedServiceName")->text(),QString("Bain & brossage"));
        QTest::qWait(20);
        QVERIFY(window.findChild<QWidget *>("motionReveal"));
        auto *logo=window.findChild<AnimatedLogo *>(); QVERIFY(logo);
        QEnterEvent enter(QPointF(35,35),QPointF(35,35),QPointF(35,35));
        QApplication::sendEvent(logo,&enter); QTest::qWait(50);
        QCOMPARE(logo->findChild<QMovie *>()->state(),QMovie::Running);
        Motion::instance().setEnabled(false);
        QVERIFY(!toggle->isChecked());
        QCOMPARE(logo->findChild<QMovie *>()->state(),QMovie::NotRunning);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QVERIFY(!window.findChild<QWidget *>("motionReveal"));
        window.navigate(0);
        QVERIFY(!window.findChild<QStackedWidget *>("pageStack")->findChild<QWidget *>("motionReveal",Qt::FindDirectChildrenOnly));
        QCOMPARE(window.findChild<QLabel *>("stat0")->text(),QString("7"));
        window.navigate(1);
        auto *table=window.findChild<QTableView *>("serviceTable"); table->selectRow(1);
        QVERIFY(window.findChild<QPushButton *>("editService")->isEnabled());
        QCOMPARE(db.getAllServices().size(),7);
        Motion::instance().setEnabled(true); QVERIFY(toggle->isChecked());
        // Decorative feedback never postpones the button's real action.
        auto *button=Design::button("Action","check","primary"); button->resize(160,42); button->show(); QTest::qWait(20);
        QSignalSpy clicks(button,&QPushButton::clicked);
        QTest::mouseClick(button,Qt::LeftButton);
        QCOMPARE(clicks.count(),1);
        QCOMPARE(button->findChild<QVariantAnimation *>("buttonRippleMotion")->state(),QAbstractAnimation::Running);
        button->hide(); QCOMPARE(button->findChild<QVariantAnimation *>("buttonRippleMotion")->state(),QAbstractAnimation::Stopped);
        delete button;
        window.navigate(0); QTest::qWait(80);
        Motion::instance().setEnabled(false);
        QCOMPARE(window.findChild<QLabel *>("stat0")->text(),QString("7"));
        for(auto *bar:window.findChildren<QProgressBar *>()) QCOMPARE(bar->value(),bar->property("motionTarget").toInt());
        Motion::instance().setEnabled(true);
    }
    void orangeHoverPreservesInteraction() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        Motion::instance().setEnabled(true);
        MainWindow window(true); window.resize(1400,900); window.show(); QTest::qWait(400);
        auto *edit=window.findChild<QPushButton *>("editService");
        auto *search=window.findChild<QLineEdit *>("serviceSearch");
        auto *card=window.findChild<QAbstractButton *>("serviceCard_4");
        QVERIFY(edit && search && card);
        auto *glow=search->findChild<QWidget *>("inputHoverGlow",Qt::FindDirectChildrenOnly); QVERIFY(glow);
        QVERIFY(glow->testAttribute(Qt::WA_TransparentForMouseEvents));
        const auto buttonRect=edit->geometry(),cardRect=card->geometry(),searchRect=search->geometry();
        QEnterEvent enter(QPointF(10,10),QPointF(10,10),QPointF(10,10)); QEvent leave(QEvent::Leave);
        const auto before=edit->grab().toImage();
        QApplication::sendEvent(edit,&enter); QApplication::sendEvent(card,&enter); QApplication::sendEvent(search,&enter);
        QTest::qWait(200);
        QVERIFY(edit->grab().toImage()!=before);
        QCOMPARE(edit->geometry(),buttonRect); QCOMPARE(card->geometry(),cardRect); QCOMPARE(search->geometry(),searchRect);
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(!output.isEmpty()) { QVERIFY(QDir().mkpath(output)); QVERIFY(window.grab().save(output+"/orange-hover.png")); }
        QApplication::sendEvent(edit,&leave); QApplication::sendEvent(card,&leave); QApplication::sendEvent(search,&leave);
        QTest::mouseClick(search,Qt::LeftButton); QTest::keyClicks(search,"education");
        QCOMPARE(window.findChild<QTableView *>("serviceTable")->model()->rowCount(),2);
        search->clear();
        Motion::instance().setEnabled(false);
        QCOMPARE(glow->findChild<QVariantAnimation *>("inputHoverMotion")->state(),QAbstractAnimation::Stopped);
        QCOMPARE(edit->findChild<QVariantAnimation *>("buttonHoverMotion")->state(),QAbstractAnimation::Stopped);
        QCOMPARE(edit->geometry(),buttonRect);
        Motion::instance().setEnabled(true);
    }
    void locauxPersistenceAndBusinessRules() {
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
        for(const auto &suggestion:suggestions) { auto *l=demo.find(suggestion.localId); QVERIFY(l); QVERIFY(!l->outOfService); QVERIFY(l->occupancy<l->capacity); if(l->cleaning!=CleaningState::Clean) QVERIFY(!suggestion.warnings.isEmpty()); }
        QString error; const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        const QString pdfPath=output.isEmpty()?folder.filePath("nettoyage.pdf"):output+"/locaux-nettoyage.pdf";
        QVERIFY2(PdfReport::generate(demo.all(),pdfPath,&error),qPrintable(error));
        QPdfDocument pdf; QCOMPARE(pdf.load(pdfPath),QPdfDocument::Error::None); QVERIFY(pdf.pageCount()>0); QVERIFY(pdf.getAllText(0).text().contains("nettoyage",Qt::CaseInsensitive));
        if(!output.isEmpty()) for(int i=0;i<pdf.pageCount();++i)
            QVERIFY(pdf.render(i,QSize(1263,893)).save(output+QString("/locaux-nettoyage-page-%1.png").arg(i+1)));
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
            auto *dialog=dynamic_cast<LocalDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
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
        QTimer::singleShot(750,[&] { if(auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
            stats=true; if(!output.isEmpty()) dialog->grab().save(output+"/locaux-statistics.png"); dialog->reject();
        } });
        page->findChild<QPushButton *>("locauxStatisticsButton")->click(); QVERIFY(stats);
        page->findChild<QPushButton *>("locauxMapButton")->click(); QTest::qWait(40);
        auto *plan=page->findChild<PlanDialog *>(); QVERIFY(plan); QVERIFY(plan->isVisible());
        if(!output.isEmpty()) plan->grab().save(output+"/locaux-plan.png");
        main.setSession("stock",Accounts::Role::Stock); QVERIFY(!plan->isVisible());
        main.navigate(4); QCOMPARE(main.findChild<QStackedWidget *>("pageStack")->currentIndex(),2);
    }
    void moduleAccountsAndPersistence() {
        QTemporaryDir folder; const QString path=folder.filePath("accounts.sqlite");
        auto &manager=DatabaseManager::instance(); QVERIFY(manager.initialize(path));
        QString error;
        auto database=OracleConnection::database();
        QVERIFY2(Employe::importInitial(database,&error),qPrintable(error));
        QVERIFY(Employe::importInitial(database,&error));
        QSqlQuery query(database); QVERIFY(query.exec("SELECT COUNT(*) FROM employes")); QVERIFY(query.next()); QCOMPARE(query.value(0).toInt(),6);
        QVERIFY(query.exec("UPDATE employes SET prenom='Conservé' WHERE id=1"));
        for(const QString &name:{QString("services"),QString("stock"),QString("employes"),QString("locaux"),QString("medical"),QString("admin")}) {
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
        QVERIFY(query.exec("ALTER TABLE accounts RENAME TO accounts_new_fixture"));
        QVERIFY(query.exec("CREATE TABLE accounts(username TEXT PRIMARY KEY COLLATE NOCASE, role TEXT NOT NULL CHECK(role IN ('admin','services','stock','employes')), salt BLOB NOT NULL, password_hash BLOB NOT NULL, iterations INTEGER NOT NULL)"));
        QVERIFY(query.exec("INSERT INTO accounts SELECT * FROM accounts_new_fixture WHERE role NOT IN ('locaux','medical')"));
        QVERIFY(query.exec("DROP TABLE accounts_new_fixture"));
        QVERIFY(manager.initialize(path));
        QCOMPARE(Accounts::authenticate("locaux","locaux123"),Accounts::Role::Locaux);
        QVERIFY(manager.initialize(path)); QVERIFY(Employe::importInitial(database,&error));
        QCOMPARE(Accounts::authenticate("stock","stock123"),Accounts::Role::None);
        QCOMPARE(Accounts::authenticate("stock","newStock2026"),Accounts::Role::Stock);
        QVERIFY(query.exec("SELECT prenom FROM employes WHERE id=1")); QVERIFY(query.next()); QCOMPARE(query.value(0).toString(),QString("Conservé"));
        QVERIFY(query.exec("DELETE FROM planning_affectations"));
        QVERIFY(query.exec("DELETE FROM employes")); QVERIFY(Employe::importInitial(database,&error));
        QVERIFY(query.exec("SELECT COUNT(*) FROM employes")); QVERIFY(query.next()); QCOMPARE(query.value(0).toInt(),0);
        QVERIFY(manager.initialize(":memory:"));
    }
    void moduleLoginRoutingAndIsolation() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        QString error; QVERIFY(Employe::importInitial(OracleConnection::database(),&error));
        AppController controller(false,false); controller.start(); auto *login=controller.loginWindow();
        Motion::instance().setEnabled(false);
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR"); if(!output.isEmpty()) QDir().mkpath(output);
        for(const QString &account:{QString("employes"),QString("stock"),QString("services"),QString("locaux"),QString("medical"),QString("admin"),QString("stock")}) {
            login->findChild<QLineEdit *>("loginUsername")->setText(account);
            login->findChild<QLineEdit *>("loginPassword")->setText(account+"123");
            login->findChild<QPushButton *>("signInButton")->click();
            QVERIFY(controller.isSignedIn()); auto *main=controller.mainWindow(); main->resize(1400,900);
            const auto role=Accounts::authenticate(account,account+"123"); QCOMPARE(main->sessionRole(),role);
            auto *stack=main->findChild<QStackedWidget *>("pageStack"); QCOMPARE(stack->currentIndex(),Accounts::homePage(role));
            for(int page=0;page<6;++page) {
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
        auto database=OracleConnection::database(); QString error;
        QVERIFY(Employe::importInitial(database,&error));
        MainWindow main; main.resize(1400,900); main.setSession("employes",Accounts::Role::Employees); main.show();
        auto *employees=main.findChild<GestionEmployes *>(); QVERIFY(employees);
        QCOMPARE(employees->findChild<QLabel *>("employeeTotalHours")->text(),QString("149"));
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        bool formSeen=false;
        QTimer automation; automation.setInterval(30);
        connect(&automation,&QTimer::timeout,&main,[&] {
            if(auto *message=qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) { if(auto *yes=message->button(QMessageBox::Yes)) yes->click(); else message->accept(); return; }
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
            employees->findChild<QLineEdit *>("employeeSearch")->setText("Integration"); QTest::qWait(60);
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
        QVERIFY(q.exec("SELECT COUNT(*) FROM employes WHERE nom='Integration'")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),1);
        QVERIFY(q.exec("SELECT COUNT(*) FROM planning_affectations")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),1);
        // Removing the test assignment explicitly makes the employee deletable again.
        QVERIFY(q.exec("DELETE FROM planning_affectations"));
        automation.start(); employees->findChild<QPushButton *>("deleteEmployee")->click(); automation.stop();
        QVERIFY(q.exec("SELECT COUNT(*) FROM employes WHERE nom='Integration'")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),0);
        QVERIFY(q.exec("SELECT COUNT(*) FROM planning_affectations")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),0);
        employees->findChild<QLineEdit *>("employeeSearch")->clear(); QTest::qWait(200);
        auto cards=employees->findChildren<QFrame *>("cardEmploye"); QVERIFY(!cards.isEmpty()); QTest::mouseClick(cards.first(),Qt::LeftButton); QTest::qWait(200);
        if(!output.isEmpty()) { QVERIFY(main.grab().save(output+"/employees-integrated.png")); main.resize(1100,700); QTest::qWait(100); QVERIFY(main.grab().save(output+"/employees-small.png")); }
        bool stats=false;
        QTimer::singleShot(100,[&] { auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            auto *table=dialog->findChild<QTableWidget *>("employeeLoadTable"); stats=table && table->rowCount()==6;
            if(!output.isEmpty()) dialog->grab().save(output+"/employee-statistics.png");
            dialog->reject(); });
        employees->findChild<QPushButton *>("employeeStatistics")->click(); QVERIFY(stats);
    }

    void employeeLeavePdfAndPasswordDialog() {
        auto &manager=DatabaseManager::instance(); QVERIFY(manager.initialize(":memory:"));
        QString error; QVERIFY(Employe::importInitial(OracleConnection::database(),&error));
        MainWindow main; main.setSession("employes",Accounts::Role::Employees); main.show();
        bool changed=false;
        QTimer::singleShot(70,[&] {
            auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            dialog->findChild<QLineEdit *>("currentPassword")->setText("employes123");
            dialog->findChild<QLineEdit *>("newPassword")->setText("employes2026");
            dialog->findChild<QLineEdit *>("confirmPassword")->setText("employes2026");
            dialog->findChild<QPushButton *>("savePassword")->click(); changed=Accounts::authenticate("employes","employes2026")==Accounts::Role::Employees;
            dialog->accept();
        });
        main.findChild<QPushButton *>("changePassword")->click(); QVERIFY(changed);
        QTemporaryDir folder; const QString target=folder.filePath("conge.pdf");
        const bool nativeDisabled=qApp->testAttribute(Qt::AA_DontUseNativeDialogs); qApp->setAttribute(Qt::AA_DontUseNativeDialogs,true);
        bool formSeen=false,saved=false; QTimer automation; automation.setInterval(40);
        connect(&automation,&QTimer::timeout,&main,[&] {
            auto *modal=QApplication::activeModalWidget();
            if(auto *save=qobject_cast<QFileDialog *>(modal)) {
                if(auto *name=save->findChild<QLineEdit *>("fileNameEdit")) name->setText(QDir::toNativeSeparators(target));
                QMetaObject::invokeMethod(save,"accept",Qt::DirectConnection); return;
            }
            if(auto *message=qobject_cast<QMessageBox *>(modal)) { saved=message->windowTitle()=="Attestation créée"; message->accept(); return; }
            if(auto *dialog=qobject_cast<QDialog *>(modal); dialog && dialog->objectName()=="employeeLeaveDialog") {
                formSeen=true; dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Save)->click();
            }
        });
        automation.start(); main.findChild<QPushButton *>("employeeLeavePdf")->click(); automation.stop();
        qApp->setAttribute(Qt::AA_DontUseNativeDialogs,nativeDisabled);
        QVERIFY(formSeen); QVERIFY(saved); QPdfDocument pdf; QCOMPARE(pdf.load(target),QPdfDocument::Error::None);
        const QString text=pdf.getAllText(0).text(); QVERIFY(text.contains("ATTESTATION DE CONGÉ")); QVERIFY(text.contains("Amine"));
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(!output.isEmpty()) {
            QFile::remove(output+"/attestation-conge.pdf"); QVERIFY(QFile::copy(target,output+"/attestation-conge.pdf"));
            auto rendered=pdf.render(0,QSize(794,1123)); QImage paper(rendered.size(),QImage::Format_ARGB32_Premultiplied); paper.fill(Qt::white);
            { QPainter painter(&paper); painter.drawImage(0,0,rendered); }
            QVERIFY(paper.save(output+"/attestation-conge.png"));
        }
    }
    void renderScreens() {
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(output.isEmpty()) QSKIP("Set ANIWORLD_CAPTURE_DIR to capture screens.");
        QVERIFY(QDir().mkpath(output)); auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:"));
        MainWindow empty; empty.resize(1400,900); empty.show(); empty.navigate(0); QTest::qWait(520); QVERIFY(empty.grab().save(output+"/dashboard-empty.png"));
        empty.navigate(1); QTest::qWait(520); QVERIFY(empty.grab().save(output+"/services-empty.png")); empty.hide();
        QVERIFY(db.addExamples()); MainWindow window(true); window.resize(1400,900); window.show(); QTest::qWait(80);
        window.navigate(0); QTest::qWait(520); QVERIFY(window.grab().save(output+"/dashboard.png")); window.navigate(1); QTest::qWait(520); QVERIFY(window.grab().save(output+"/services.png"));
        auto *logo=window.findChild<AnimatedLogo *>("animatedBrandLogo"); QVERIFY(logo);
        QEnterEvent enter(QPointF(35,35),QPointF(35,35),QPointF(35,35));
        QEvent leave(QEvent::Leave);
        QVERIFY(QDir().mkpath(output+"/logo-motion"));
        for(int frame=0;frame<33;++frame) {
            if(frame==3) QApplication::sendEvent(logo,&enter);
            if(frame==29) QApplication::sendEvent(logo,&leave);
            QTest::qWait(100);
            QVERIFY(window.grab(QRect(0,0,248,112)).save(output+QString("/logo-motion/frame_%1.png").arg(frame,3,10,QChar('0'))));
            if(frame==15) QVERIFY(window.grab().save(output+"/services-logo-hover.png"));
        }
        window.findChild<QTableView *>("serviceTable")->selectRow(2); QTest::qWait(250); QVERIFY(window.grab().save(output+"/services-selected.png"));
        QTimer::singleShot(550,[output] { auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(dialog) { dialog->grab().save(output+"/service-statistics.png"); dialog->accept(); } });
        window.findChild<QPushButton *>("serviceStatistics")->click();
        window.findChild<QPushButton *>("tableView")->click(); QTest::qWait(250); QVERIFY(window.grab().save(output+"/services-table.png"));
        window.findChild<QPushButton *>("cardsView")->click();
        window.resize(1100,700); QTest::qWait(520); QVERIFY(window.grab().save(output+"/services-small.png"));
        for(auto *card:window.findChildren<QAbstractButton *>()) if(card->objectName().startsWith("serviceCard_") && card->isChecked()) {
            auto *scroll=qobject_cast<QScrollArea *>(card->parentWidget()->parentWidget()->parentWidget());
            QVERIFY(scroll);
            QVERIFY(scroll->viewport()->rect().contains(QRect(card->mapTo(scroll->viewport(),QPoint(0,0)),card->size())));
        }
        auto *remove=window.findChild<QPushButton *>("deleteService");
        QVERIFY(remove->isVisible());
        QVERIFY(window.rect().contains(QRect(remove->mapTo(&window,QPoint(0,0)),remove->size())));
        window.navigate(0); QTest::qWait(520); QVERIFY(window.grab().save(output+"/dashboard-small.png"));
        ServiceDialog form; form.show(); QTest::qWait(250); QVERIFY(form.grab().save(output+"/service-form.png")); form.hide();
        ServiceDialog details(db.getAllServices().first(),true); details.show(); QTest::qWait(250); QVERIFY(details.grab().save(output+"/service-detail.png"));
    }
    void captureMotionPreview() {
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(output.isEmpty()) QSKIP("Set ANIWORLD_CAPTURE_DIR to capture motion.");
        QVERIFY(QDir().mkpath(output+"/interface-motion"));
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        MainWindow window(true); window.resize(1400,900); window.show(); QTest::qWait(500);
        for(int frame=0;frame<72;++frame) {
            if(frame==4) {
                QEnterEvent enter(QPointF(20,20),QPointF(20,20),QPointF(20,20));
                QApplication::sendEvent(window.findChild<QAbstractButton *>("serviceCard_4"),&enter);
            }
            if(frame==10) window.findChild<QAbstractButton *>("serviceCard_4")->click();
            if(frame==20) window.navigate(0);
            if(frame==40) {
                QEnterEvent enter(QPointF(35,35),QPointF(35,35),QPointF(35,35));
                QApplication::sendEvent(window.findChild<AnimatedLogo *>(),&enter);
            }
            if(frame==51) { QEvent leave(QEvent::Leave); QApplication::sendEvent(window.findChild<AnimatedLogo *>(),&leave); }
            if(frame==55) window.navigate(1);
            if(frame==62) { window.findChild<QAbstractButton *>("serviceCard_4")->click(); window.findChild<QPushButton *>("advanceService")->click(); }
            QTest::qWait(100);
            QVERIFY(window.grab().scaled(980,630,Qt::KeepAspectRatio,Qt::SmoothTransformation).save(output+QString("/interface-motion/frame_%1.png").arg(frame,3,10,QChar('0'))));
        }
    }
};
QTEST_MAIN(ServiceTests)
#include "service_tests.moc"

