#include <QtTest>
#include <QTemporaryDir>
#include <QTimer>
#include <QMessageBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QFontDatabase>
#include <QEnterEvent>
#include <QFocusEvent>
#include "../src/database/databasemanager.h"
#include "../src/mainwindow.h"
#include "../src/dialogs/servicedialog.h"
#include "../src/ui/design.h"
#include "../src/ui/servicereport.h"
#include "../src/models/serviceanalytics.h"
#include "../src/widgets/servicecard.h"
#include "../src/widgets/animatedlogo.h"

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
        search->setText("Sarra"); QCOMPARE(table->rowCount(),2); QVERIFY(page.findChild<QPushButton *>("editService")->isEnabled());
        QVERIFY(page.findChild<QLabel *>("selectedServiceName")->text().contains("Bain") || page.findChild<QLabel *>("selectedServiceName")->text().contains("Toilettage"));
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
    void dateFiltersAndCards() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:"));
        const auto now=QDateTime::currentDateTime();
        const QList<int> days={-1,0,6,7};
        for(int day:days) { Service s; s.name=QString("Prestation %1").arg(day); s.dateTime=now.addDays(day); QVERIFY(db.addService(s)); }
        ServiceWidget page; page.resize(1172,820); page.show(); QTest::qWait(30);
        auto *table=page.findChild<QTableWidget *>("serviceTable");
        auto *period=page.findChild<QComboBox *>("periodFilter");
        period->setCurrentIndex(1); QCOMPARE(table->rowCount(),1);
        period->setCurrentIndex(2); QCOMPARE(table->rowCount(),2);
        period->setCurrentIndex(3); QVERIFY(table->rowCount()>=1);
        page.findChild<QPushButton *>("resetFilters")->click(); QCOMPARE(table->rowCount(),4);
        const auto cards=page.findChildren<QAbstractButton *>(QRegularExpression("serviceCard_.*"));
        QCOMPARE(cards.size(),4);
        cards.last()->click(); QVERIFY(cards.last()->isChecked());
        QCOMPARE(page.findChild<QLabel *>("selectedServiceName")->text(),QString("Prestation -1"));
        page.findChild<QPushButton *>("tableView")->click(); QVERIFY(table->isVisible());
        page.findChild<QPushButton *>("cardsView")->click(); QVERIFY(!table->isVisible());
    }
    void quickStatusAndDuplication() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:"));
        Service service; service.name="Service original"; service.price=25; service.duration=45;
        QVERIFY(db.addService(service)); const int originalId=db.lastInsertedId();
        ServiceWidget page; page.show();
        auto *advance=page.findChild<QPushButton *>("advanceService");
        advance->click(); QCOMPARE(db.getAllServices().first().status,QString("En cours"));
        advance->click(); QCOMPARE(db.getAllServices().first().status,QString("Terminé")); QVERIFY(!advance->isVisible());
        bool correctTemplate=false;
        QTimer::singleShot(20,[&correctTemplate] {
            auto *dialog=qobject_cast<ServiceDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            const Service copy=dialog->getService();
            correctTemplate=copy.id==0 && copy.status=="Planifié" && copy.price==25 && copy.duration==45 && copy.dateTime.date()==QDate::currentDate().addDays(1);
            dialog->findChild<QPushButton *>("saveService")->click();
        });
        page.findChild<QPushButton *>("duplicateService")->click(); QVERIFY(correctTemplate);
        const auto services=db.getAllServices(); QCOMPARE(services.size(),2);
        QVERIFY(services.first().id!=originalId); QCOMPARE(services.first().name,QString("Service original (copie)"));
        QCOMPARE(services.last().status,QString("Terminé"));
        QCOMPARE(page.findChild<QLabel *>("selectedServiceName")->text(),QString("Service original (copie)"));
    }
    void metricsAndPdfReport() {
        const QDateTime now(QDate(2026,10,1),QTime(12,0));
        QList<Service> services;
        for(int i=0;i<4;++i) { Service s; s.id=i+1; s.name="Soin <chat> & chien"; s.price=20; s.duration=30; s.dateTime=now.addDays(-1); s.status=Design::statuses()[i]; services.append(s); }
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
            for(int i=0;i<70;++i) { Service s=services[i%4]; s.id=i+1; s.name=QString("Prestation %1 — Soins de bien-être pour animaux").arg(i+1); s.responsible="Responsable au nom composé"; many.append(s); }
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
        MainWindow window(true); window.show(); QTest::qWait(50);
        auto *card=window.findChild<QAbstractButton *>("serviceCard_4"); QVERIFY(card);
        card->clearFocus();
        QFocusEvent focusOut(QEvent::FocusOut); QApplication::sendEvent(card,&focusOut);
        QEvent leave(QEvent::Leave); QApplication::sendEvent(card,&leave); QTest::qWait(180);
        const auto resting=card->grab().toImage(); const auto geometry=card->geometry();
        QEnterEvent enter(QPointF(20,20),QPointF(20,20),QPointF(20,20));
        QApplication::sendEvent(card,&enter); QTest::qWait(190);
        QVERIFY(resting!=card->grab().toImage()); QCOMPARE(card->geometry(),geometry);
        QApplication::sendEvent(card,&leave); QTest::qWait(190); QCOMPARE(card->grab().toImage(),resting);
        auto *transition=window.findChild<QPropertyAnimation *>("pageTransition"); QVERIFY(transition);
        window.navigate(0); QCOMPARE(transition->state(),QAbstractAnimation::Running);
        window.navigate(1); window.navigate(0); window.navigate(1);
        QTRY_COMPARE(transition->state(),QAbstractAnimation::Stopped);
        QVERIFY(window.findChild<QPushButton *>("navServices")->isChecked());
        auto *effect=qobject_cast<QGraphicsOpacityEffect *>(transition->targetObject());
        QVERIFY(effect); QVERIFY(!effect->isEnabled()); QCOMPARE(effect->opacity(),1.0);
    }
    void renderScreens() {
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        if(output.isEmpty()) QSKIP("Set ANIWORLD_CAPTURE_DIR to capture screens.");
        QVERIFY(QDir().mkpath(output)); auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:"));
        MainWindow empty; empty.resize(1400,900); empty.show(); empty.navigate(0); QTest::qWait(250); QVERIFY(empty.grab().save(output+"/dashboard-empty.png"));
        empty.navigate(1); QTest::qWait(250); QVERIFY(empty.grab().save(output+"/services-empty.png")); empty.hide();
        QVERIFY(db.addExamples()); MainWindow window(true); window.resize(1400,900); window.show(); QTest::qWait(80);
        window.navigate(0); QTest::qWait(250); QVERIFY(window.grab().save(output+"/dashboard.png")); window.navigate(1); QTest::qWait(250); QVERIFY(window.grab().save(output+"/services.png"));
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
        window.findChild<QTableWidget *>("serviceTable")->selectRow(2); QTest::qWait(30); QVERIFY(window.grab().save(output+"/services-selected.png"));
        QTimer::singleShot(50,[output] { auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(dialog) { dialog->grab().save(output+"/service-statistics.png"); dialog->accept(); } });
        window.findChild<QPushButton *>("serviceStatistics")->click();
        window.findChild<QPushButton *>("tableView")->click(); QTest::qWait(30); QVERIFY(window.grab().save(output+"/services-table.png"));
        window.findChild<QPushButton *>("cardsView")->click();
        window.resize(1100,700); QTest::qWait(40); QVERIFY(window.grab().save(output+"/services-small.png"));
        for(auto *card:window.findChildren<QAbstractButton *>()) if(card->objectName().startsWith("serviceCard_") && card->isChecked()) {
            auto *scroll=qobject_cast<QScrollArea *>(card->parentWidget()->parentWidget()->parentWidget());
            QVERIFY(scroll);
            QVERIFY(scroll->viewport()->rect().contains(QRect(card->mapTo(scroll->viewport(),QPoint(0,0)),card->size())));
        }
        auto *remove=window.findChild<QPushButton *>("deleteService");
        QVERIFY(remove->isVisible());
        QVERIFY(window.rect().contains(QRect(remove->mapTo(&window,QPoint(0,0)),remove->size())));
        window.navigate(0); QTest::qWait(250); QVERIFY(window.grab().save(output+"/dashboard-small.png"));
        ServiceDialog form; form.show(); QTest::qWait(40); QVERIFY(form.grab().save(output+"/service-form.png")); form.hide();
        ServiceDialog details(db.getAllServices().first(),true); details.show(); QTest::qWait(40); QVERIFY(details.grab().save(output+"/service-detail.png"));
    }
};
QTEST_MAIN(ServiceTests)
#include "service_tests.moc"
