#include <QtTest>
#include "../src/rh/hrwidget.h"
using namespace AniRh;
#include <QTemporaryDir>

class HrTests : public QObject {
    Q_OBJECT
    Record employee(const QString &email="amira@example.com") {
        return {{"first_name","Amira"},{"last_name","Ben Ali"},{"email",email},{"phone","+216 00 000 000"},{"role","Vétérinaire"},{"department","Soins vétérinaires"},{"contract","CDI"},{"hired","2020-01-01"},{"salary",2200125},{"allowance",30}};
    }
    Record attendance(const QString &day=HrStore::today()) {return {{"employee_id",1},{"day",day},{"status","Présent"},{"arrival","08:30"},{"departure","17:00"},{"notes",""}};}
    Record leave(const QString &start="2027-02-01",const QString &end="2027-02-05") {return {{"employee_id",1},{"type","Congé annuel"},{"start",start},{"end",end},{"reason","Repos"}};}
    Record task() {return {{"employee_id",1},{"title","Préparer le planning"},{"description","Équipe soins"},{"due","2027-01-05"},{"priority","Haute"},{"status","À faire"}};}
    Record review() {return {{"employee_id",1},{"day",HrStore::today()},{"reviewer","Nour"},{"quality",5},{"teamwork",4},{"punctuality",3},{"initiative",4},{"feedback","Très bon travail"},{"goals","Formation"}};}
    QString captureDir;
    void capture(QWidget &w,const QString &name) { if(!captureDir.isEmpty()) QVERIFY(w.grab().save(captureDir+"/"+name+".png")); }
private slots:
    void initTestCase() {
        for(const auto &font:QStringList{"segoeui.ttf","segoeuib.ttf","seguisb.ttf","segoeuii.ttf"}) QFontDatabase::addApplicationFont("C:/Windows/Fonts/"+font);
        QCoreApplication::setOrganizationName("ANIWORLD_Test");QCoreApplication::setApplicationName("RH_Test");QLocale::setDefault(QLocale(QLocale::French,QLocale::Tunisia));qApp->setStyle("Fusion");Motion::instance().setEnabled(false);
        captureDir=qEnvironmentVariable("RH_CAPTURE_DIR"); if(!captureDir.isEmpty())QVERIFY(QDir().mkpath(captureDir));
    }
    void employeeValidationAndPersistence() {
        QTemporaryDir dir; QVERIFY(dir.isValid());
        {HrStore db; QVERIFY(db.open(dir.path()+"/rh.sqlite"));QVERIFY2(db.saveEmployee(employee()),qPrintable(db.error()));auto duplicate=employee("AMIRA@example.com");QVERIFY(!db.saveEmployee(duplicate));auto invalid=employee("bad");QVERIFY(!db.saveEmployee(invalid));invalid=employee("other@example.com");invalid["salary"]=-1;QVERIFY(!db.saveEmployee(invalid));auto updated=employee();updated["role"]="Responsable";QVERIFY(db.saveEmployee(updated,1));QVERIFY(db.setEmployeeActive(1,false));QCOMPARE(db.employees(true).size(),0);QCOMPARE(db.employees().size(),1);}
        {HrStore db;QVERIFY(db.open(dir.path()+"/rh.sqlite"));QCOMPARE(db.employees()[0]["role"].toString(),QString("Responsable"));QCOMPARE(db.employees()[0]["salary"].toLongLong(),2200125LL);QVERIFY(db.setEmployeeActive(1,true));QCOMPARE(db.employees(true).size(),1);}
    }
    void attendanceRules() {
        HrStore db;QVERIFY(db.open(":memory:"));QVERIFY(db.saveEmployee(employee()));QVERIFY(db.saveAttendance(attendance()));QVERIFY(!db.saveAttendance(attendance()));
        auto a=attendance();a["departure"]="08:00";QVERIFY(!db.saveAttendance(a,1));a["departure"]="";QVERIFY(db.saveAttendance(a,1));QCOMPARE(db.row("SELECT departure FROM attendance")["departure"].toString(),QString());
        a["status"]="Absent";QVERIFY(db.saveAttendance(a,1));QCOMPARE(db.row("SELECT arrival FROM attendance")["arrival"].toString(),QString());
        QVERIFY(!db.saveAttendance(attendance("2019-01-01")));QVERIFY(!db.saveAttendance(attendance("2099-01-01")));QVERIFY(db.removeRecord("attendance",1));QCOMPARE(db.scalar("SELECT COUNT(*) FROM attendance"),0LL);
    }
    void leaveOverlapBalanceAndConflict() {
        HrStore db;QVERIFY(db.open(":memory:"));auto e=employee();e["allowance"]=7;QVERIFY(db.saveEmployee(e));QVERIFY(db.saveLeave(leave()));QVERIFY(!db.saveLeave(leave("2027-02-03","2027-02-07")));
        QVERIFY(db.saveLeave(leave("2027-03-01","2027-03-05")));QVERIFY(db.decideLeave(1,true));QCOMPARE(db.usedLeave(1,2027),5);QVERIFY(!db.decideLeave(2,true));QVERIFY(db.decideLeave(2,false));QVERIFY(!db.decideLeave(2,true));QVERIFY(!db.removeRecord("leaves",1));
        QVERIFY(!db.saveLeave(leave("2027-12-30","2028-01-02")));QVERIFY(!db.saveLeave(leave("2027-05-09","2027-05-01")));
        QVERIFY(db.saveAttendance(attendance("2024-03-01")));QVERIFY(db.saveLeave(leave("2024-03-01","2024-03-01")));QVERIFY(!db.decideLeave(3,true));QVERIFY(db.removeRecord("attendance",1));QVERIFY(db.decideLeave(3,true));QVERIFY(!db.saveAttendance(attendance("2024-03-01")));
        e["allowance"]=4;QVERIFY(!db.saveEmployee(e,1));
    }
    void tasksReviewsAndArchive() {
        HrStore db;QVERIFY(db.open(":memory:"));QVERIFY(db.saveEmployee(employee()));auto t=task();QVERIFY(db.saveTask(t));t["status"]="Terminé";QVERIFY(db.saveTask(t,1));QCOMPARE(db.row("SELECT status FROM tasks")["status"].toString(),QString("Terminé"));t["title"]="";QVERIFY(!db.saveTask(t));
        auto r=review();QVERIFY(db.saveReview(r));r["quality"]=6;QVERIFY(!db.saveReview(r));r["quality"]=5;r["reviewer"]="";QVERIFY(!db.saveReview(r));
        QVERIFY(db.setEmployeeActive(1,false));QVERIFY(!db.saveTask(task()));QVERIFY(!db.saveReview(review()));QVERIFY(!db.saveLeave(leave()));QVERIFY(!db.saveAttendance(attendance()));QVERIFY(db.saveTask(task(),1));
        QVERIFY(db.removeRecord("tasks",1));QVERIFY(db.removeRecord("reviews",1));QVERIFY(!db.removeRecord("employees",1));
    }
    void payrollSnapshotsAndLocking() {
        HrStore db;QVERIFY(db.open(":memory:"));QVERIFY(db.saveEmployee(employee()));QVERIFY(!db.preparePayroll("invalid"));QVERIFY(db.preparePayroll("2026-10"));QVERIFY(db.preparePayroll("2026-10"));QCOMPARE(db.scalar("SELECT COUNT(*) FROM payroll"),1LL);
        auto e=employee();e["salary"]=3300500;QVERIFY(db.saveEmployee(e,1));QVERIFY(db.preparePayroll("2026-10"));QCOMPARE(db.scalar("SELECT base FROM payroll WHERE id=1"),2200125LL);
        QVERIFY(!db.updatePayroll(1,0,3000000,""));QVERIFY(!db.updatePayroll(1,-1,0,""));QVERIFY(db.updatePayroll(1,100050,200025,"Ajustements"));QCOMPARE(db.scalar("SELECT base+bonus-deductions FROM payroll WHERE id=1"),2100150LL);QVERIFY(db.markPaid(1));QVERIFY(!db.updatePayroll(1,0,0,""));QVERIFY(!db.markPaid(1));
        QVERIFY(db.preparePayroll("2026-11"));QCOMPARE(db.scalar("SELECT base FROM payroll WHERE month='2026-11'"),3300500LL);QVERIFY(db.setEmployeeActive(1,false));QVERIFY(db.preparePayroll("2026-12"));QCOMPARE(db.scalar("SELECT COUNT(*) FROM payroll WHERE month='2026-12'"),0LL);
    }
    void futureHireExcludedFromPayroll() {
        HrStore db;QVERIFY(db.open(":memory:"));auto e=employee();e["hired"]="2027-11-20";QVERIFY(db.saveEmployee(e));QVERIFY(db.preparePayroll("2027-10"));QCOMPARE(db.scalar("SELECT COUNT(*) FROM payroll"),0LL);QVERIFY(db.preparePayroll("2027-11"));QCOMPARE(db.scalar("SELECT COUNT(*) FROM payroll"),1LL);
    }
    void demoIsIsolated() {
        HrStore demo;QVERIFY(demo.open(":memory:"));QVERIFY2(demo.seedDemo(),qPrintable(demo.error()));QCOMPARE(demo.employees().size(),8);QVERIFY(demo.seedDemo());QCOMPARE(demo.employees().size(),8);HrStore clean;QVERIFY(clean.open(":memory:"));QCOMPARE(clean.employees().size(),0);
    }
    void scheduleConflictsAndAtomicCopy() {
        HrStore db;QVERIFY(db.open(":memory:"));QVERIFY(db.saveEmployee(employee()));
        Record shift={{"employee_id",1},{"day","2027-02-01"},{"start","08:30"},{"end","12:00"},{"assignment","Consultations"},{"notes","Salle A"}};
        QVERIFY(db.saveShift(shift));auto overlap=shift;overlap["start"]="11:00";overlap["end"]="13:00";QVERIFY(!db.saveShift(overlap));overlap["start"]="12:00";QVERIFY(db.saveShift(overlap));
        auto invalid=shift;invalid["end"]="07:00";QVERIFY(!db.saveShift(invalid));invalid["day"]="2019-01-01";QVERIFY(!db.saveShift(invalid));
        QVERIFY(db.saveLeave(leave("2027-02-01","2027-02-01")));QVERIFY(!db.decideLeave(1,true));QVERIFY(db.removeRecord("shifts",1));QVERIFY(db.removeRecord("shifts",2));QVERIFY(db.decideLeave(1,true));QVERIFY(!db.saveShift(shift));
        shift["day"]="2027-02-08";QVERIFY(db.saveShift(shift));shift["day"]="2027-02-09";QVERIFY(db.saveShift(shift));
        QVERIFY(db.copyScheduleWeek(QDate(2027,2,8)));QCOMPARE(db.scalar("SELECT COUNT(*) FROM shifts"),4LL);QVERIFY(!db.copyScheduleWeek(QDate(2027,2,8)));QCOMPARE(db.scalar("SELECT COUNT(*) FROM shifts"),4LL);
        QVERIFY(db.saveLeave(leave("2027-02-23","2027-02-23")));QVERIFY(db.decideLeave(2,true));QVERIFY(!db.copyScheduleWeek(QDate(2027,2,15)));QCOMPARE(db.scalar("SELECT COUNT(*) FROM shifts WHERE day='2027-02-22'"),0LL);
    }
    void formValidationAndSave() {
        HrStore db;QVERIFY(db.open(":memory:"));EditDialog dialog(&db,EditDialog::Employee);dialog.show();QTest::qWait(50);auto *save=dialog.findChild<QPushButton *>("saveButton");QVERIFY(save);QTest::mouseClick(save,Qt::LeftButton);QVERIFY(dialog.findChild<QLabel *>("formError")->isVisible());QCOMPARE(db.employees().size(),0);
        for(const auto &pair:QList<QPair<QString,QString>>{{"first_name","Élise"},{"last_name","Test"},{"email","elise@example.com"},{"role","Vétérinaire"}})dialog.findChild<QLineEdit *>(pair.first)->setText(pair.second);
        dialog.findChild<QDoubleSpinBox *>("salary")->setValue(1234.567);QTest::mouseClick(save,Qt::LeftButton);QCOMPARE(dialog.result(),int(QDialog::Accepted));QCOMPARE(db.scalar("SELECT salary FROM employees"),1234567LL);
    }
    void navigationFilteringAndSorting() {
        HrStore db;QVERIFY(db.open(":memory:"));QVERIFY(db.seedDemo());HrWidget window(&db,true);window.resize(1440,940);window.show();QTest::qWait(100);
        for(int i=0;i<8;++i){auto *tabs=window.findChild<QTabBar *>("rhTabs");QTest::mouseClick(tabs,Qt::LeftButton,Qt::NoModifier,tabs->tabRect(i).center());QCOMPARE(window.findChild<QStackedWidget *>("rhPageStack")->currentIndex(),i);}
        window.navigate(1);auto *table=window.findChild<QTableWidget *>("table1");QCOMPARE(table->rowCount(),8);auto *search=window.findChild<QLineEdit *>("search1");search->setText("veterinaire");QCOMPARE(table->rowCount(),3);search->clear();table->sortItems(4,Qt::DescendingOrder);QCOMPARE(table->item(0,4)->data(Qt::UserRole).toLongLong(),3410000LL);
        search->setText("no-results-at-all");QCOMPARE(table->rowCount(),0);QVERIFY(window.findChild<QLabel *>("empty1")->isVisible());QVERIFY(!window.findChild<QPushButton *>("edit1")->isEnabled());search->clear();
        window.navigate(2);auto *filter=window.findChild<QComboBox *>("filter2");filter->setCurrentText("Non pointé");QCOMPARE(window.findChild<QTableWidget *>("table2")->rowCount(),2);
        window.navigate(3);window.findChild<QComboBox *>("filter3")->setCurrentText("En attente");QCOMPARE(window.findChild<QTableWidget *>("table3")->rowCount(),2);
        window.navigate(7);auto *grid=window.findChild<QTableWidget *>("scheduleGrid");QCOMPARE(grid->rowCount(),8);auto *weekDate=window.findChild<QDateEdit *>("scheduleDate");const auto initial=weekDate->date();QTest::mouseClick(window.findChild<QPushButton *>("nextWeek"),Qt::LeftButton);QCOMPARE(weekDate->date(),initial.addDays(7));QTest::mouseClick(window.findChild<QPushButton *>("previousWeek"),Qt::LeftButton);QCOMPARE(weekDate->date(),initial);auto *scheduleSearch=window.findChild<QLineEdit *>("scheduleSearch");scheduleSearch->setText("amira");QCOMPARE(grid->rowCount(),1);scheduleSearch->clear();
        Motion::instance().setEnabled(true);for(int i=0;i<12;++i)window.navigate(i%7);Motion::instance().setEnabled(false);QTest::qWait(50);QVERIFY(!Motion::instance().enabled());
    }
    void formScreensAndAppScreens() {
        HrStore db;QVERIFY(db.open(":memory:"));QVERIFY(db.seedDemo());HrWidget window(&db,true);window.resize(1440,940);window.show();QTest::qWait(100);
        const QStringList names={"dashboard","employees","attendance","leave","tasks","reviews","payroll","schedule"};
        for(int i=0;i<8;++i){window.navigate(i);QTest::qWait(40);capture(window,names[i]);}
        window.resize(1100,700);for(int i=0;i<8;++i){window.navigate(i);QTest::qWait(40);capture(window,names[i]+"-small");QCOMPARE(window.findChild<QTabBar *>("rhTabs")->currentIndex(),i);}
        for(int i=0;i<7;++i){Record r;if(i==5)r=db.row("SELECT * FROM payroll LIMIT 1");EditDialog dialog(&db,static_cast<EditDialog::Kind>(i),r,&window);dialog.show();QTest::qWait(30);capture(dialog,"form-"+QString::number(i));QVERIFY(dialog.findChild<QPushButton *>("saveButton")->isVisible());dialog.close();}
        HrStore empty;QVERIFY(empty.open(":memory:"));HrWidget emptyWindow(&empty);emptyWindow.resize(1440,940);emptyWindow.show();emptyWindow.navigate(1);QTest::qWait(40);capture(emptyWindow,"employees-empty");
    }
    void csvAndPdfExports() {
        QTemporaryDir dir;QString error;QVERIFY(Ui::exportCsv(dir.path()+"/data.csv",{"Nom","Notes"},{{"Élise; Test","Ligne 1\n\"citation\""},{"=SUM(1+1)","@cmd"}},&error));QFile csv(dir.path()+"/data.csv");QVERIFY(csv.open(QIODevice::ReadOnly));auto bytes=csv.readAll();QVERIFY(bytes.startsWith("\xEF\xBB\xBF"));QVERIFY(bytes.contains("\"'="));QVERIFY(bytes.contains("\"\"citation\"\""));
        HrStore db;QVERIFY(db.open(":memory:"));QVERIFY(db.seedDemo());auto r=db.row("SELECT * FROM payroll LIMIT 1");r["notes"]=QString("Détail de rémunération & contrôle <b>texte</b>\n").repeated(100);
        const QString path=captureDir.isEmpty()?dir.path()+"/bulletin.pdf":captureDir+"/bulletin-demo.pdf";QVERIFY2(exportPayslip(path,r,true,&error),qPrintable(error));QFile pdf(path);QVERIFY(pdf.open(QIODevice::ReadOnly));QVERIFY(pdf.read(5)=="%PDF-");QVERIFY(pdf.size()>10000);
    }
    void animationFrames() {
        if(captureDir.isEmpty())QSKIP("Capture directory not requested");
        HrStore db;QVERIFY(db.open(":memory:"));QVERIFY(db.seedDemo());HrWidget window(&db,true);window.resize(1440,940);window.show();QTest::qWait(30);Motion::instance().setEnabled(true);window.navigate(1);window.navigate(0);
        for(int i=0;i<16;++i){QTest::qWait(50);capture(window,QString("motion-%1").arg(i,2,10,QChar('0')));}Motion::instance().setEnabled(false);
    }
};
QTEST_MAIN(HrTests)
#include "hr_tests.moc"
