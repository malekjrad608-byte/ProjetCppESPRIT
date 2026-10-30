from pathlib import Path
p=Path('tests/service_tests.cpp');s=p.read_text(encoding='utf-8-sig')
s=s.replace('#include <QSqlError>','#include <QSqlError>\n#include <QSqlQueryModel>\n#include "../src/models/employeequeries.h"')
s=s.replace('class ServiceTests :', '''namespace {
ServiceDialog *visibleServiceForm() {
    if(auto *modal=qobject_cast<ServiceDialog *>(QApplication::activeModalWidget()))return modal;
    for(auto *widget:QApplication::allWidgets())
        if(auto *form=qobject_cast<ServiceDialog *>(widget);form && form->isVisible())return form;
    return nullptr;
}
}
class ServiceTests :''')
# The helper itself must retain the original modal check.
start=s.index('class ServiceTests :')
s=s[:start]+s[start:].replace('qobject_cast<ServiceDialog *>(QApplication::activeModalWidget())','visibleServiceForm()')
s=s.replace('->click(); QVERIFY(dialogOpened);','->click(); QTRY_VERIFY(dialogOpened);')
s=s.replace('page->onAdd(); QCOMPARE(db.getAllServices().size(),1);','page->onAdd(); QTRY_COMPARE(db.getAllServices().size(),1);')
s=s.replace('->click(); QCOMPARE(db.getAllServices().first().name,QString("Service modifié"));','->click(); QTRY_COMPARE(db.getAllServices().first().name,QString("Service modifié"));')
s=s.replace('->click(); QVERIFY(readOnly);','->click(); QTRY_VERIFY(readOnly);')
test='''    void atelierModelCrudAndValidation() {
        auto &manager=DatabaseManager::instance();QVERIFY(manager.initialize(":memory:"));
        QCOMPARE(&manager,&DatabaseManager::instance());
        QString error;
        Service service;service.name="Soin d'Oscar'); DROP TABLE services; --";service.clientName="Client test";
        QVERIFY2(service.ajouter(&error),qPrintable(error));QVERIFY(service.id>0);
        QScopedPointer<QSqlQueryModel> model(Service::afficher());
        QVERIFY(!model->lastError().isValid());QCOMPARE(model->rowCount(),1);QCOMPARE(model->data(model->index(0,1)).toString(),service.name);
        service.name="Soin modifié";QVERIFY(service.modifier(&error));QCOMPARE(Service::afficherListe(&error).first().name,service.name);
        service.duration=0;QVERIFY(!service.modifier(&error));QCOMPARE(Service::afficherListe(&error).first().duration,30);
        Employe employee;employee.setNom("O'Brien");employee.setPrenom("Test");employee.setDateEmbauche(QDate::currentDate());employee.setCourriel("test@example.test");
        QVERIFY2(employee.ajouter(&error),qPrintable(error));QVERIFY(employee.getId()>0);
        QCOMPARE(Employe::afficher("O'Brien","date","Tous les postes","Tous",&error).size(),1);
        QCOMPARE(Employe::afficher("' OR '1'='1","date","Tous les postes","Tous",&error).size(),0);
        employee.setCourriel("incorrect");QVERIFY(!employee.modifier(&error));employee.setCourriel("valid@example.test");employee.setPrenom("Modifié");QVERIFY(employee.modifier(&error));
        QVERIFY(Employe::basculerDisponibilite(employee.getId(),&error));QVERIFY(!Employe::afficher().first().isDisponible());
        QVERIFY(Employe::supprimer(employee.getId(),&error));QVERIFY(!Employe::supprimer(employee.getId(),&error));
        QVERIFY(Service::supprimer(service.id,&error));QVERIFY(Service::afficherListe(&error).isEmpty());
        qint64 revision=0;StockRecord stock;stock.article="Test";stock.lot="Test";stock.quantity=-1;
        QVERIFY(!StockRecord::enregistrer({stock},&revision,&error));
        Local local;local.name="Test";local.capacity=0;QVERIFY(!Local::enregistrer({local},&revision,&error));
        QCOMPARE(QSqlDatabase::connectionNames().count("aniworld-services"),1);
    }
    void atelierEmbeddedFormKeepsInputOnError() {
        auto &manager=DatabaseManager::instance();QVERIFY(manager.initialize(":memory:"));
        ServiceWidget page;page.resize(1100,780);page.show();page.onAdd();QTest::qWait(50);
        auto *form=visibleServiceForm();QVERIFY(form);QVERIFY(!form->isWindow());QVERIFY(!QApplication::activeModalWidget());
        form->findChild<QPushButton *>("saveService")->click();QVERIFY(form->findChild<QLabel *>("validationError")->isVisible());
        form->findChild<QLineEdit *>("serviceName")->setText("Service conservé dans le formulaire");
        form->findChild<QLineEdit *>("serviceClient")->setText("Client");
        const auto capture=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");if(!capture.isEmpty())page.grab().save(capture+"/atelier-service-form.png");
        form->findChild<QPushButton *>("saveService")->click();QCOMPARE(manager.getAllServices().size(),1);
        QCOMPARE(page.findChild<QStackedWidget *>("servicePages")->currentIndex(),0);
    }
'''
s=s.replace('private slots:\n','private slots:\n'+test,1)
# Exercise the public entity model API with real Oracle fixtures as well.
s=s.replace('        QVERIFY(!locals.remove(localId)); // parent protected while stock exists','''        QVector<StockRecord> modelStock;QVERIFY2(StockRecord::afficher(&modelStock,&revision,&error),qPrintable(error));
        for(auto &item:modelStock)if(item.lot=="FIVE-INTEGRATION-TEST")item.article="Produit d'Oscar";
        QVERIFY2(StockRecord::enregistrer(modelStock,&revision,&error),qPrintable(error));
        auto employees=Employe::afficher("Test","date","Tous les postes","Tous",&error);QVERIFY2(error.isEmpty(),qPrintable(error));
        bool updated=false;for(auto &e:employees)if(e.getId()==team){e.setServiceId(b.id);e.setPrenom("Equipe d'Oscar");QVERIFY2(e.modifier(&error),qPrintable(error));updated=true;}QVERIFY(updated);
        QVERIFY(!locals.remove(localId)); // parent protected while stock exists''')
s=s.replace('QVERIFY2(MedicalStore::save(record,&error),qPrintable(error));record.medical.reason="Modification test";QVERIFY2(MedicalStore::save(record,&error),qPrintable(error));','QVERIFY2(record.ajouter(&error),qPrintable(error));record.medical.reason="Modification test";QVERIFY2(record.modifier(&error),qPrintable(error));')
p.write_text(s,encoding='utf-8')
