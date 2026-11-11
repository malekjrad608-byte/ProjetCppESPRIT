#include <QtTest>
#include <QFile>
#include <QFontDatabase>
#include <QPdfDocument>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableView>
#include <QHeaderView>
#include <QComboBox>
#include <QTimer>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connection.h"
#include "services/database/oracleconnection.h"
#include "services/models/service.h"
#include "services/models/servicereferences.h"
#include "services/models/employeequeries.h"
#include "services/widgets/servicewidget.h"
#include "services/widgets/servicecard.h"
#include "services/dialogs/servicedialog.h"
#include "services/ui/invoice.h"

class ServiceIntegrationTests : public QObject {
    Q_OBJECT
private slots:
    void initTestCase() {
        // The Windows offscreen plugin needs explicit fonts for readable captures.
        for(const auto *font:{"segoeui.ttf","seguisb.ttf","segoeuib.ttf"})
            QVERIFY(QFontDatabase::addApplicationFont("C:/Windows/Fonts/"+QString(font))>=0);
        qApp->setFont(QFont("Segoe UI",10));
    }
    void isolatedCrudAndValidation() {
        {
            auto db=QSqlDatabase::addDatabase("QSQLITE"); db.setDatabaseName(":memory:");
            QVERIFY2(db.open(),qPrintable(db.lastError().text()));
            QSqlQuery query(db);
            QVERIFY(query.exec("CREATE TABLE services(id INTEGER PRIMARY KEY AUTOINCREMENT,name TEXT,description TEXT,responsible TEXT,category TEXT,dateTime TEXT,status TEXT,duration INTEGER,price REAL,clientName TEXT,invoiceNumber TEXT,invoiceDate TEXT,id_local INTEGER,id_employe INTEGER,id_client INTEGER)"));
            Service s; s.setName("Integration temporaire");s.setClientName("Client test");s.setPrice(40);
            QVERIFY2(s.ajouter(),qPrintable(s.lastError()));QVERIFY(s.getId()>0);
            QString error;auto rows=Service::afficherListe(&error);QVERIFY2(error.isEmpty(),qPrintable(error));QCOMPARE(rows.size(),1);
            s.setPrice(65);s.setStatus("Terminé");QVERIFY2(s.modifier(),qPrintable(s.lastError()));
            rows=Service::afficherListe(&error);QCOMPARE(rows.first().getPrice(),65.0);QCOMPARE(rows.first().getStatus(),QString("Terminé"));
            Service invalid=s;invalid.setDuration(0);QVERIFY(!invalid.modifier());QVERIFY(!invalid.lastError().isEmpty());
            invalid=s;invalid.setPrice(-1);QVERIFY(!invalid.modifier());
            QVERIFY2(s.supprimer(s.getId()),qPrintable(s.lastError()));QVERIFY(Service::afficherListe(&error).isEmpty());
            db.close();
        }
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
    void servicesWithoutSchema_data() {
        QTest::addColumn<QString>("driver");
        QTest::addColumn<bool>("open");
        QTest::newRow("missing-services-table") << QString("QSQLITE") << true;
        QTest::newRow("closed-database") << QString("QSQLITE") << false;
        QTest::newRow("closed-oracle") << QString("QODBC") << false;
        QTest::newRow("no-connection") << QString() << false;
    }
    void servicesWithoutSchema() {
        QFETCH(QString, driver);
        QFETCH(bool, open);
        {
            QSqlDatabase db;
            if (!driver.isEmpty()) {
                db = QSqlDatabase::addDatabase(driver);
                db.setDatabaseName(":memory:");
                if (open) QVERIFY(db.open());
            }
            QMainWindow host;
            Ui::MainWindow ui;
            ui.setupUi(&host);
            auto *page = new ServiceWidget(&ui, ui.pageServices);
            ui.layoutPageServices->addWidget(page);
            ui.stackedWidget->setCurrentWidget(ui.pageServices);
            host.show();
            QTest::qWait(50);
            auto *table = page->findChild<QTableView *>("serviceTable");
            QVERIFY(table);
            QCOMPARE(table->model()->rowCount(), 0);
            QCOMPARE(table->model()->columnCount(), 0);
            QVERIFY(ui.Service_m_notice->isVisible());
            QCOMPARE(ui.Service_m_emptyTitle->text(), QString("Services indisponibles"));
            ui.Service_m_search->setText("test");
            page->refresh();
            QCOMPARE(table->model()->rowCount(), 0);
            ui.Service_m_search->clear();
            if (open) {
                // Repair only this in-memory test schema, then refresh the SAME widget.
                QSqlQuery query(db);
                QVERIFY(query.exec("CREATE TABLE services(id INTEGER PRIMARY KEY,name TEXT,description TEXT,responsible TEXT,category TEXT,dateTime TEXT,status TEXT,duration INTEGER,price REAL,clientName TEXT,invoiceNumber TEXT,invoiceDate TEXT,id_local INTEGER,id_employe INTEGER,id_client INTEGER)"));
                page->refresh();
                QCOMPARE(table->model()->columnCount(), 15);
                QVERIFY(ui.Service_m_notice->isHidden());
                QCOMPARE(ui.Service_m_emptyTitle->text(), QString("Votre première prestation"));
                Service service;service.setName("Reprise après erreur");
                QVERIFY2(service.ajouter(),qPrintable(service.lastError()));
                page->refresh();
                QCOMPARE(table->model()->rowCount(), 1);
                QVERIFY(!table->isColumnHidden(1));
                QVERIFY(table->isColumnHidden(0));
                QCOMPARE(table->horizontalHeader()->sectionResizeMode(5),QHeaderView::Fixed);
                // A later schema failure must also stay recoverable.
                QVERIFY(query.exec("DROP TABLE services"));
                page->refresh();
                QCOMPARE(table->model()->columnCount(), 0);
                QVERIFY(ui.Service_m_notice->isVisible());
            }
        }
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
    void sharedOracleNavigationFormsAndPdf() {
        qApp->setApplicationName("ANIWORLD-IntegrationTests");qApp->setOrganizationName("ANIWORLD-Tests");
        auto &connection=Connection::createInstance();QVERIFY2(connection.createConnection(),qPrintable(connection.lastError()));
        const QStringList connections=QSqlDatabase::connectionNames();QCOMPARE(connections.size(),1);
        QCOMPARE(OracleConnection::database().connectionName(),QString(QSqlDatabase::defaultConnection));
        QString error;const auto before=Service::afficherListe(&error);QVERIFY2(error.isEmpty(),qPrintable(error));
        const auto owners=ServiceReferences::proprietaires(&error);QVERIFY2(error.isEmpty(),qPrintable(error));
        Service::appointments(&error);QVERIFY2(error.isEmpty(),qPrintable(error));
        const auto employees=EmployeeQueries::planningEmployees();QVERIFY2(employees.error.text().isEmpty(),qPrintable(employees.error.text()));
        QFile css(":/style.qss");QVERIFY(css.open(QIODevice::ReadOnly));qApp->setStyleSheet(QString::fromUtf8(css.readAll()));
        const QString globalStyle=qApp->styleSheet();
        // Dismiss informational startup reminders from the pre-existing medical module.
        QTimer reminders;connect(&reminders,&QTimer::timeout,[] {
            for(auto *w:QApplication::topLevelWidgets())
                if(auto *box=qobject_cast<QMessageBox *>(w))box->reject();
        });reminders.start(100);
        MainWindow window;window.resize(1400,900);window.show();QTest::qWait(800);
        QVERIFY(QMetaObject::invokeMethod(&window,"afficherServices",Qt::DirectConnection));QTest::qWait(300);
        auto *stack=window.findChild<QStackedWidget *>("stackedWidget");QVERIFY(stack);QCOMPARE(stack->count(),5);QCOMPARE(stack->currentIndex(),1);
        auto *page=window.findChild<ServiceWidget *>();QVERIFY(page);QCOMPARE(window.findChildren<ServiceWidget *>().size(),1);
        QCOMPARE(qApp->styleSheet(),globalStyle);QCOMPARE(QSqlDatabase::connectionNames(),connections);
        auto *table=page->findChild<QTableView *>("serviceTable");QVERIFY(table);QCOMPARE(table->model()->rowCount(),before.size());
        auto *search=page->findChild<QLineEdit *>("serviceSearch");QVERIFY(search);
        search->setText("__aucun_service_integration__");QCOMPARE(table->model()->rowCount(),0);search->clear();QCOMPARE(table->model()->rowCount(),before.size());
        page->onAdd();auto *form=page->findChild<ServiceDialog *>();QVERIFY(form);QVERIFY(form->isVisible());
        auto *choice=form->findChild<QComboBox *>("serviceClientChoice");QVERIFY(choice);QVERIFY(!choice->isEditable());
        for(const auto &owner:owners)QVERIFY(choice->findText(owner.second)>=0);
        auto *cancel=form->findChild<QPushButton *>("cancelService");QVERIFY(cancel);cancel->click();QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        // Open the real Planning and Statistics dialogs, close without changing data.
        for(const auto *name:{"servicePlanning","serviceStatistics"}) {
            auto *button=page->findChild<QPushButton *>(name);QVERIFY(button);bool opened=false;
            QTimer::singleShot(100,&window,[&opened]{auto *dlg=qobject_cast<QDialog *>(QApplication::activeModalWidget());if(dlg){opened=true;dlg->reject();}});
            button->click();QVERIFY(opened);
        }
        for(const auto *slot:{"afficherEmployes","afficherLocaux","afficherDossiers","afficherStock","afficherServices"}) {
            QVERIFY(QMetaObject::invokeMethod(&window,slot,Qt::DirectConnection));QTest::qWait(200);
        }
        QCOMPARE(stack->currentIndex(),1);QCOMPARE(window.findChildren<ServiceWidget *>().size(),1);QCOMPARE(QSqlDatabase::connectionNames(),connections);
        QVERIFY(connection.isConnected());
        if(!before.isEmpty()){bool visible=false;for(auto *card:page->findChildren<QAbstractButton *>(QRegularExpression("^serviceCard_")))visible|=card->isVisible();QVERIFY(visible);}
        QTest::qWait(650); const auto capture=qEnvironmentVariable("SERVICE_CAPTURE");if(!capture.isEmpty())QVERIFY(window.grab().save(capture));
        bool pdfChecked=false;
        for(const auto &s:before)if(s.getStatus()!="Annulé") {
            QTemporaryDir temp;auto path=temp.filePath("facture.pdf");QVERIFY2(Invoice::writePdf(path,s,false,&error),qPrintable(error));
            QPdfDocument pdf;QCOMPARE(pdf.load(path),QPdfDocument::Error::None);QVERIFY(pdf.pageCount()>0);pdfChecked=true;break;
        }
        QVERIFY2(pdfChecked,"Aucun service facturable pour verifier le PDF.");
        const auto after=Service::afficherListe(&error);QVERIFY2(error.isEmpty(),qPrintable(error));QCOMPARE(after.size(),before.size());
        for(int i=0;i<before.size();++i){QCOMPARE(after[i].getId(),before[i].getId());QCOMPARE(after[i].getPrice(),before[i].getPrice());QCOMPARE(after[i].getStatus(),before[i].getStatus());}
    }
};
QTEST_MAIN(ServiceIntegrationTests)
#include "service_integration_tests.moc"



