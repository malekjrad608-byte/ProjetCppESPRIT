#include <QtTest>
#include <QTemporaryDir>
#include <QTimer>
#include <QMessageBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QFontDatabase>
#include "../src/database/databasemanager.h"
#include "../src/mainwindow.h"
#include "../src/dialogs/servicedialog.h"
#include "../src/ui/design.h"

class ServiceTests : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        if(QGuiApplication::platformName()=="offscreen") {
            for(const auto &font:{"segoeui.ttf","seguisb.ttf","segoeuib.ttf"})
                QVERIFY(QFontDatabase::addApplicationFont("C:/Windows/Fonts/"+QString(font))>=0);
        }
        qApp->setFont(QFont("Segoe UI",10));
        QLocale::setDefault(QLocale(QLocale::French,QLocale::Tunisia));
        Design::applyTheme();
    }
    void storageCrudAndPersistence() {
        QTemporaryDir temp; QVERIFY(temp.isValid());
        auto &db=DatabaseManager::instance(); const QString path=temp.path()+"/services.sqlite";
        QVERIFY(db.initialize(path)); QCOMPARE(db.getAllServices().size(),0);
        Service s; s.name="   "; QVERIFY(!db.addService(s));
        s.name="Toilettage d'été; \"Milo\""; s.responsible="Élodie"; s.price=42.5; s.description="Ligne 1\nLigne 2";
        QVERIFY(db.addService(s)); auto saved=db.getAllServices().first(); QVERIFY(saved.id>0); QCOMPARE(saved.name,s.name); QCOMPARE(saved.price,42.5);
        saved.status="Terminé"; saved.price=55; QVERIFY(db.updateService(saved));
        QVERIFY(db.initialize(path)); const auto reopened=db.getAllServices(); QCOMPARE(reopened.size(),1); QCOMPARE(reopened.first().status,QString("Terminé")); QCOMPARE(reopened.first().responsible,QString("Élodie")); QCOMPARE(reopened.first().description,s.description); QCOMPARE(reopened.first().price,55.0);
        saved.duration=0; QVERIFY(!db.updateService(saved)); saved.duration=30; saved.status="invalid"; QVERIFY(!db.updateService(saved));
        QVERIFY(db.deleteService(saved.id)); QVERIFY(db.getAllServices().isEmpty()); QVERIFY(!db.deleteService(saved.id));
        QVERIFY(db.initialize(":memory:"));
    }
    void formValidation() {
        ServiceDialog dialog; dialog.show();
        auto *name=dialog.findChild<QLineEdit *>("serviceName"); auto *save=dialog.findChild<QPushButton *>("saveService");
        QVERIFY(name); QVERIFY(save); name->setText("    "); save->click();
        QCOMPARE(dialog.result(),0); QVERIFY(dialog.isVisible()); QVERIFY(dialog.findChild<QLabel *>("validationError")->isVisible());
        dialog.findChild<QLineEdit *>("serviceResponsible")->setText("Sarra"); name->setText("  Soin des griffes  "); save->click();
        QCOMPARE(dialog.result(),int(QDialog::Accepted)); QCOMPARE(dialog.getService().name,QString("Soin des griffes")); QCOMPARE(dialog.getService().responsible,QString("Sarra"));
    }
    void filterSortAndRefresh() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        ServiceWidget page; page.show();
        auto *table=page.findChild<QTableWidget *>("serviceTable"); auto *search=page.findChild<QLineEdit *>("serviceSearch");
        auto *status=page.findChild<QComboBox *>("statusFilter"); auto *category=page.findChild<QComboBox *>("categoryFilter"); auto *sort=page.findChild<QComboBox *>("sortServices");
        QCOMPARE(table->rowCount(),7); search->setText("education"); QCOMPARE(table->rowCount(),2);
        status->setCurrentText("Planifié"); QCOMPARE(table->rowCount(),1); page.refresh(); QCOMPARE(table->rowCount(),1);
        category->setCurrentText("Toilettage"); QCOMPARE(table->rowCount(),0); QVERIFY(!page.findChild<QPushButton *>("editService")->isEnabled());
        search->clear(); status->setCurrentIndex(0); category->setCurrentIndex(0); sort->setCurrentIndex(3);
        QCOMPARE(table->item(0,0)->text(),QString("Soin des griffes"));
        table->selectRow(0); QVERIFY(page.findChild<QPushButton *>("editService")->isEnabled());
        search->setText("Sarra"); QCOMPARE(table->rowCount(),2); QVERIFY(!page.findChild<QPushButton *>("editService")->isEnabled());
    }
    void interfaceCrud() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:"));
        MainWindow window; window.show(); QVERIFY(QApplication::activeModalWidget()==nullptr);
        auto *page=window.findChild<ServiceWidget *>(); QVERIFY(page);
        window.findChild<QPushButton *>("navServices")->click();
        QSignalSpy changes(page,&ServiceWidget::servicesChanged);
        QTimer::singleShot(20,[] {
            auto *dialog=qobject_cast<ServiceDialog *>(QApplication::activeModalWidget());
            if(!dialog) return;
            dialog->findChild<QLineEdit *>("serviceName")->setText("Test de création");
            dialog->findChild<QDoubleSpinBox *>("servicePrice")->setValue(18.5);
            dialog->findChild<QPushButton *>("saveService")->click();
        });
        page->onAdd(); QCOMPARE(db.getAllServices().size(),1); QCOMPARE(changes.count(),1);
        auto *table=page->findChild<QTableWidget *>("serviceTable"); table->selectRow(0);
        QTimer::singleShot(20,[] {
            auto *dialog=qobject_cast<ServiceDialog *>(QApplication::activeModalWidget());
            if(!dialog) return;
            dialog->findChild<QLineEdit *>("serviceName")->setText("Service modifié");
            dialog->findChild<QComboBox *>("serviceStatus")->setCurrentText("Terminé");
            dialog->findChild<QPushButton *>("saveService")->click();
        });
        page->findChild<QPushButton *>("editService")->click(); QCOMPARE(db.getAllServices().first().name,QString("Service modifié")); QCOMPARE(changes.count(),2);
        window.navigate(0); QCOMPARE(window.findChild<QLabel *>("stat3")->text(),QString("1")); window.navigate(1); table->selectRow(0);
        bool readOnly=false;
        QTimer::singleShot(20,[&readOnly] {
            auto *dialog=qobject_cast<ServiceDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            readOnly=dialog->findChild<QLineEdit *>("serviceName")->isReadOnly() && !dialog->findChild<QPushButton *>("saveService"); dialog->reject();
        });
        page->findChild<QPushButton *>("viewService")->click(); QVERIFY(readOnly);
        QTimer::singleShot(20,[] { auto *box=qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if(box) box->reject(); });
        page->findChild<QPushButton *>("deleteService")->click(); QCOMPARE(db.getAllServices().size(),1);
        QTimer::singleShot(20,[] {
            auto *box=qobject_cast<QMessageBox *>(QApplication::activeModalWidget()); if(!box) return;
            for(auto *button:box->buttons()) if(box->buttonRole(button)==QMessageBox::DestructiveRole) { button->click(); break; }
        });
        page->findChild<QPushButton *>("deleteService")->click(); QCOMPARE(db.getAllServices().size(),0); QCOMPARE(changes.count(),3);
        window.navigate(0); QCOMPARE(window.findChild<QLabel *>("stat0")->text(),QString("0"));
    }
    void exportEscaping() {
        Service s; s.name="=SUM(A1:A2)"; s.description="Soins; \"doux\"\nDeuxième ligne";
        const QString csv=ServiceWidget::csvContent({s}); QVERIFY(csv.startsWith(QChar(0xFEFF)));
        QVERIFY(csv.contains("\"'=SUM(A1:A2)\"")); QVERIFY(csv.contains("\"Soins; \"\"doux\"\"\nDeuxième ligne\""));
    }
    void renderScreens() {
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(output.isEmpty()) QSKIP("Set ANIWORLD_CAPTURE_DIR to capture screens.");
        QVERIFY(QDir().mkpath(output)); auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:"));
        MainWindow empty; empty.resize(1400,900); empty.show(); QTest::qWait(80); QVERIFY(empty.grab().save(output+"/dashboard-empty.png"));
        empty.navigate(1); QTest::qWait(40); QVERIFY(empty.grab().save(output+"/services-empty.png")); empty.hide();
        QVERIFY(db.addExamples()); MainWindow window(true); window.resize(1400,900); window.show(); QTest::qWait(80);
        QVERIFY(window.grab().save(output+"/dashboard.png")); window.navigate(1); QTest::qWait(50); QVERIFY(window.grab().save(output+"/services.png"));
        window.findChild<QTableWidget *>("serviceTable")->selectRow(2); QTest::qWait(30); QVERIFY(window.grab().save(output+"/services-selected.png"));
        window.resize(1100,700); QTest::qWait(40); QVERIFY(window.grab().save(output+"/services-small.png"));
        window.navigate(0); QTest::qWait(40); QVERIFY(window.grab().save(output+"/dashboard-small.png"));
        ServiceDialog form; form.show(); QTest::qWait(40); QVERIFY(form.grab().save(output+"/service-form.png")); form.hide();
        ServiceDialog details(db.getAllServices().first(),true); details.show(); QTest::qWait(40); QVERIFY(details.grab().save(output+"/service-detail.png"));
    }
};
QTEST_MAIN(ServiceTests)
#include "service_tests.moc"
