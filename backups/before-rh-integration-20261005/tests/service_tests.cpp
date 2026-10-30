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
#include <QTableView>
#include <QSpinBox>
#include <QSortFilterProxyModel>
#include "../src/database/databasemanager.h"
#include "../src/mainwindow.h"
#include "../src/dialogs/servicedialog.h"
#include "../src/ui/design.h"
#include "../src/ui/servicereport.h"
#include "../src/models/serviceanalytics.h"
#include "../src/widgets/servicecard.h"
#include "../src/widgets/animatedlogo.h"
#include "../src/ui/motion.h"

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
        Motion::instance().setEnabled(true);
    }
    void loginSessionAndSavedServices() {
        QTemporaryDir temporary;
        auto &db=DatabaseManager::instance();
        const QString path=temporary.path()+"/services.sqlite";
        QVERIFY(db.initialize(path));
        Service existing; existing.name="Service conservé"; QVERIFY(db.addService(existing));
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
        QCOMPARE(main->findChild<QTableWidget *>("serviceTable")->rowCount(),1);
        QVERIFY(main->findChild<QLabel *>("signedInUser")->text().contains("admin"));
        bool dialogOpened=false;
        QTimer::singleShot(20,[&dialogOpened] {
            auto *dialog=qobject_cast<ServiceDialog *>(QApplication::activeModalWidget());
            if(!dialog) return;
            dialogOpened=true;
            dialog->findChild<QLineEdit *>("serviceName")->setText("Ajout après connexion");
            dialog->findChild<QPushButton *>("saveService")->click();
        });
        main->findChild<QPushButton *>("addService")->click(); QVERIFY(dialogOpened);
        QCOMPARE(db.getAllServices().size(),2);
        main->findChild<QPushButton *>("logoutButton")->click();
        QVERIFY(!controller.isSignedIn()); QVERIFY(!main->isVisible()); QVERIFY(login->isVisible());
        QVERIFY(password->text().isEmpty()); QVERIFY(user->text().isEmpty()); QVERIFY(submit->isEnabled());
        // Logout and login never initialize or replace the service database.
        QVERIFY(db.initialize(path)); QCOMPARE(db.getAllServices().size(),2);
        user->setText("admin"); password->setText("admin123"); submit->click();
        QCOMPARE(controller.mainWindow(),main); QVERIFY(main->isVisible()); QCOMPARE(accepted.count(),2);
        QCOMPARE(main->findChild<QTableWidget *>("serviceTable")->rowCount(),2);
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
        LoginWindow login(false,false); login.resize(880,620); login.show(); QTest::qWait(240);
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
    void stockFiltersAndMovements() {
        StockWidget stock; stock.resize(1050,810); stock.show(); QTest::qWait(30);
        auto *table=stock.findChild<QTableView *>("stockTable"); QVERIFY(table);
        auto *search=stock.findChild<QLineEdit *>("stockSearch");
        auto *category=stock.findChild<QComboBox *>("stockCategoryFilter");
        auto *proxy=qobject_cast<QSortFilterProxyModel *>(table->model()); QVERIFY(proxy);
        auto *source=proxy->sourceModel(); QCOMPARE(source->rowCount(),19);
        QCOMPARE(stock.findChild<QLabel *>("stockKpi0")->text(),QString("19"));
        int low=0; for(int row=0;row<source->rowCount();++row)
            if(source->index(row,2).data(Qt::UserRole+1).toInt()<source->index(row,3).data(Qt::UserRole+1).toInt()) ++low;
        QCOMPARE(stock.findChild<QLabel *>("stockKpi1")->text(),QString::number(low));
        search->setText("materiel"); QCOMPARE(proxy->rowCount(),2);
        search->clear(); category->setCurrentText("Chats"); QCOMPARE(proxy->rowCount(),4);
        search->setText("vaccin"); QCOMPARE(proxy->rowCount(),1);
        const QString csv=stock.inventoryCsv(); QVERIFY(csv.startsWith(QChar(0xFEFF)));
        QVERIFY(csv.contains("DÉMONSTRATION")); QVERIFY(csv.contains("CAT-V-03")); QVERIFY(!csv.contains("DOG-V-03"));
        search->setText("no matching article"); QCOMPARE(proxy->rowCount(),0);
        QVERIFY(stock.findChild<QLabel *>("stockResultCount")->text().contains("Aucun résultat"));
        search->clear(); category->setCurrentIndex(0);
        table->sortByColumn(2,Qt::AscendingOrder);
        for(int row=1;row<proxy->rowCount();++row)
            QVERIFY(proxy->index(row-1,2).data(Qt::UserRole+1).toInt()<=proxy->index(row,2).data(Qt::UserRole+1).toInt());
        const int before=source->index(0,2).data(Qt::UserRole+1).toInt();
        bool validated=false;
        QTimer::singleShot(20,[&validated,before] {
            auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            auto *article=dialog->findChild<QComboBox *>("stockMovementArticle"); if(!article) { dialog->reject(); return; }
            article->setCurrentIndex(0);
            auto *type=dialog->findChild<QComboBox *>("stockMovementType"); type->setCurrentIndex(1);
            auto *amount=dialog->findChild<QSpinBox *>("stockMovementQuantity"); amount->setValue(before+1);
            auto *save=dialog->findChild<QPushButton *>("saveStockMovement"); save->click();
            validated=dialog->isVisible() && dialog->findChild<QLabel *>("stockMovementError")->isVisible();
            type->setCurrentIndex(0); amount->setValue(5); save->click();
        });
        stock.findChild<QPushButton *>("stockMovementButton")->click(); QVERIFY(validated);
        QCOMPARE(source->index(0,2).data(Qt::UserRole+1).toInt(),before+5);
        stock.refresh(); QCOMPARE(source->index(0,2).data(Qt::UserRole+1).toInt(),before+5);
        QTimer::singleShot(20,[] { if(auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget())) dialog->reject(); });
        stock.openMovementDialog(); QCOMPARE(source->index(0,2).data(Qt::UserRole+1).toInt(),before+5);
        StockWidget reopened;
        auto *reopenedTable=reopened.findChild<QTableView *>("stockTable");
        QCOMPARE(reopenedTable->model()->index(0,2).data(Qt::UserRole+1).toInt(),before);
    }
    void stockFefoAndReorderDialogs() {
        StockWidget stock; stock.show();
        auto *table=stock.findChild<QTableView *>("stockTable");
        auto *source=qobject_cast<QSortFilterProxyModel *>(table->model())->sourceModel();
        int first=-1;
        for(int row=0;row<source->rowCount();++row) {
            const QDate date=source->index(row,4).data(Qt::UserRole+1).toDate();
            if(date>=QDate::currentDate() && (first<0 || date<source->index(first,4).data(Qt::UserRole+1).toDate())) first=row;
        }
        const QString output=qEnvironmentVariable("ANIWORLD_CAPTURE_DIR");
        auto *fefo=stock.findChild<QPushButton *>("stockFefoButton");
        QCOMPARE(fefo->isEnabled(),first>=0);
        if(first>=0) {
            const int before=source->index(first,2).data(Qt::UserRole+1).toInt();
            bool matched=false;
            QTimer::singleShot(250,[first,&matched,output] {
                auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
                matched=dialog->findChild<QComboBox *>("stockMovementArticle")->currentData().toInt()==first
                    && dialog->findChild<QComboBox *>("stockMovementType")->currentIndex()==1;
                if(!output.isEmpty()) dialog->grab().save(output+"/stock-movement.png");
                dialog->findChild<QPushButton *>("saveStockMovement")->click();
            });
            fefo->click(); QVERIFY(matched);
            QCOMPARE(source->index(first,2).data(Qt::UserRole+1).toInt(),before-1);
        }
        const QString before=stock.inventoryCsv(); bool calculated=false;
        QTimer::singleShot(220,[&calculated,source,output] {
            auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget()); if(!dialog) return;
            auto *article=dialog->findChild<QComboBox *>("stockReorderArticle"); if(!article) { dialog->reject(); return; }
            const int row=article->currentData().toInt();
            const int available=source->index(row,2).data(Qt::UserRole+1).toInt();
            const int threshold=source->index(row,3).data(Qt::UserRole+1).toInt();
            calculated=dialog->findChild<QLabel *>("stockReorderQuantity")->text().contains(QString("Quantité proposée : %1").arg(qMax(threshold,17)-available));
            if(!output.isEmpty()) dialog->grab().save(output+"/stock-reorder.png");
            dialog->accept();
        });
        stock.findChild<QPushButton *>("stockReorderButton")->click(); QVERIFY(calculated);
        QCOMPARE(stock.inventoryCsv(),before);
    }
    void stockIntegratedNavigationAndLogout() {
        auto &db=DatabaseManager::instance(); QVERIFY(db.initialize(":memory:")); QVERIFY(db.addExamples());
        AppController controller(false,false); controller.start(); auto *login=controller.loginWindow();
        login->findChild<QLineEdit *>("loginUsername")->setText("admin");
        login->findChild<QLineEdit *>("loginPassword")->setText("admin123");
        login->findChild<QPushButton *>("signInButton")->click();
        auto *main=controller.mainWindow(); QVERIFY(main);
        auto *stack=main->findChild<QStackedWidget *>("pageStack"); QCOMPARE(stack->count(),3);
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
            for(const auto &name:{"stockMovementButton","stockExportButton","stockFefoButton","stockReorderButton"}) {
                auto *button=stock->findChild<QPushButton *>(name); QVERIFY(button->isVisible());
                QVERIFY(stock->rect().contains(QRect(button->mapTo(stock,QPoint()),button->size())));
            }
            if(!output.isEmpty()) { QVERIFY(QDir().mkpath(output)); QVERIFY(main->grab().save(output+(size.width()==1400?"/stock-integrated.png":"/stock-small.png"))); }
        }
        main->navigate(1); QVERIFY(main->findChild<QLabel *>("storageBadge")->text().contains("Sauvegarde"));
        QCOMPARE(main->findChild<QTableWidget *>("serviceTable")->rowCount(),7); QCOMPARE(db.getAllServices().size(),7);
        main->navigate(2); main->findChild<QPushButton *>("logoutButton")->click();
        QVERIFY(login->isVisible()); QVERIFY(!main->isVisible()); QVERIFY(!controller.isSignedIn());
        login->findChild<QLineEdit *>("loginUsername")->setText("admin");
        login->findChild<QLineEdit *>("loginPassword")->setText("admin123");
        login->findChild<QPushButton *>("signInButton")->click();
        QCOMPARE(stack->currentIndex(),1); nav->click(); QCOMPARE(stack->currentWidget(),stock);
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
        MainWindow window; window.resize(1400,900); window.show(); QTest::qWait(500);
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
                    const QString name=page==0?"dashboard-transition-mid":(page==1?"services-transition-mid":"stock-transition-mid");
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
        auto *table=window.findChild<QTableWidget *>("serviceTable"); table->selectRow(1);
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
        window.findChild<QTableWidget *>("serviceTable")->selectRow(2); QTest::qWait(250); QVERIFY(window.grab().save(output+"/services-selected.png"));
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
