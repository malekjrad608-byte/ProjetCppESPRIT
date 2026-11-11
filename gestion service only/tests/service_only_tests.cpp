#include <QtTest>
#include <QComboBox>
#include <QFontDatabase>
#include <QLineEdit>
#include <QStackedWidget>
#include <QTableView>
#include <QPdfDocument>
#include <QTemporaryDir>
#include "../src/database/databasemanager.h"
#include "../src/mainwindow.h"
#include "../src/dialogs/servicedialog.h"
#include "../src/models/servicereferences.h"
#include "../src/ui/invoice.h"
#include "../src/ui/design.h"
#include "../src/widgets/servicecard.h"
#include "../src/ui/motion.h"
class ServiceOnlyTests : public QObject {
 Q_OBJECT
private slots:
 void standaloneOracleReadAndForms() {
  for(const auto &f:{"segoeui.ttf","seguisb.ttf","segoeuib.ttf"})QFontDatabase::addApplicationFont("C:/Windows/Fonts/"+QString(f));qApp->setFont(QFont("Segoe UI",10));
  qApp->setApplicationName("ANIWORLD");qApp->setOrganizationName("ANIWORLD");Design::applyTheme();Motion::instance().setEnabled(false);
  auto &db=DatabaseManager::instance();QVERIFY2(db.initializeOracle(),qPrintable(db.lastError()));
  const auto before=db.getAllServices();QVERIFY2(db.lastError().isEmpty(),qPrintable(db.lastError()));
  MainWindow window(false);window.setSession("admin",Accounts::Role::Admin);window.resize(1350,850);window.show();QTest::qWait(100);
  auto *stack=window.findChild<QStackedWidget *>("pageStack");QVERIFY(stack);QCOMPARE(stack->count(),2);QCOMPARE(stack->currentIndex(),1);
  for(const auto *name:{"navStock","navEmployees","navLocaux","navMedical"})QVERIFY(!window.findChild<QPushButton *>(name));
  auto *page=window.findChild<ServiceWidget *>();QVERIFY(page);
  QCOMPARE(page->findChild<QTableView *>("serviceTable")->model()->rowCount(),before.size());
  QVERIFY(page->findChild<QPushButton *>("invoiceService")->isVisible());
  QString error;auto owners=ServiceReferences::proprietaires(&error);QVERIFY2(error.isEmpty(),qPrintable(error));
  page->onAdd();auto *form=page->findChild<ServiceDialog *>();QVERIFY(form);QVERIFY(form->isVisible());
  auto *choice=form->findChild<QComboBox *>("serviceClientChoice");QVERIFY(choice);QVERIFY(!choice->isEditable());
  for(const auto &owner:owners)QVERIFY(choice->findText(owner.second)>=0);
  form->findChild<QPushButton *>("cancelService")->click();QCOMPARE(stack->currentIndex(),1);QCOMPARE(db.getAllServices().size(),before.size());
  auto *search=page->findChild<QLineEdit *>("serviceSearch");
  QVERIFY(search);{search->setText("__no_matching_service__");QCOMPARE(page->findChild<QTableView *>("serviceTable")->model()->rowCount(),0);search->clear();}
  window.navigate(0);QCOMPARE(stack->currentIndex(),0);window.navigate(1);QTest::qWait(300);
  if(!before.isEmpty()){bool shown=false;for(auto *card:page->findChildren<ServiceCard *>())shown|=card->isVisible();QVERIFY(shown);}
  const auto output=qEnvironmentVariable("SERVICE_CAPTURE");if(!output.isEmpty())QVERIFY(window.grab().save(output));
  for(const auto &s:before)if(s.getStatus()!="Annulé"){
   QTemporaryDir tmp;auto path=tmp.filePath("facture.pdf");QVERIFY2(Invoice::writePdf(path,s,false,&error),qPrintable(error));
   QPdfDocument pdf;QCOMPARE(pdf.load(path),QPdfDocument::Error::None);QVERIFY(pdf.pageCount()>0);break;
  }
 }
};
QTEST_MAIN(ServiceOnlyTests)
#include "service_only_tests.moc"
