"""Sync the service UI while preserving the standalone project and references model."""
from pathlib import Path
import shutil
from datetime import datetime

root = Path(__file__).resolve().parents[1]
target = Path(r'C:\Users\malek\Desktop\gestion service only')
backup = root / 'artifacts' / ('service-only-before-' + datetime.now().strftime('%Y%m%d-%H%M%S'))
files = [f'src/{area}/{name}.{ext}' for area,name in [
    ('dialogs','serviceplanningdialog'),('dialogs','servicestatisticsdialog'),
    ('widgets','servicetimetable'),('widgets','servicecurve'),('models','modulesupplements')
] for ext in ['h','cpp']]
changes = {name:(root/name).read_text(encoding='utf-8-sig') for name in files}
service = (root/'src/models/service.cpp').read_text(encoding='utf-8-sig')
service = service.replace('#include "../employees/employe.h"\n', '').replace('#include "../medical/medicalrecord.h"', '#include "servicereferences.h"').replace('MedicalRecord::proprietaire(', 'ServiceReferences::proprietaire(')
changes['src/models/service.cpp'] = service
changes['src/widgets/servicewidget.cpp'] = (root/'src/widgets/servicewidget.cpp').read_text(encoding='utf-8-sig').replace('"ui_integrated_pages.h"','"ui_mainwindow.h"')
pri = (target/'sources.pri').read_text(encoding='utf-8-sig')
for name in files:
    if '$$PWD/'+name not in pri:
        pri += '\n' + ('SOURCES' if name.endswith('.cpp') else 'HEADERS') + ' += $$PWD/' + name + '\n'
changes['sources.pri'] = pri
readme=(target/'README.md').read_text(encoding='utf-8-sig')
section='## Mise à jour du 20 novembre 2026'
if section not in readme:
    readme+='\n'+section+'\n\nCourbes dynamiques (période, catégorie, mesure, actualisation automatique), planning hebdomadaire et liste détaillée. Les services non affectés restent visibles. Les tâches complémentaires des employés sont lues dans le même stockage local ANIWORLD que le projet intégré. Sur ce PC, les dix tâches Démo du 9 au 15 novembre sont donc visibles dans les deux versions. Aucun CRUD ni interface des quatre autres gestions n’est ajouté.\n'
changes['README.md']=readme
test=(target/'tests/service_only_tests.cpp').read_text(encoding='utf-8-sig')
if 'standalonePlanningAndCurves' not in test:
    test=test.replace('#include <QtTest>', '#include <QtTest>\n#include <QTableWidget>\n#include <QLabel>\n#include <QTimer>\n#include "../src/dialogs/serviceplanningdialog.h"\n#include "../src/dialogs/servicestatisticsdialog.h"\n#include "../src/widgets/servicetimetable.h"\n#include "../src/widgets/servicecurve.h"')
    test=test.replace('private slots:', '''private slots:
 void standalonePlanningAndCurves() {
  qApp->setApplicationName("ANIWORLD");qApp->setOrganizationName("ANIWORLD");Design::applyTheme();
  auto &db=DatabaseManager::instance();QVERIFY2(db.initializeOracle(),qPrintable(db.lastError()));
  QString error;const auto entries=Service::appointments(&error);QVERIFY2(error.isEmpty(),qPrintable(error));
  int expected=0;for(const auto &a:entries)if(a.start<QDateTime(QDate(2026,11,16),QTime(0,0)) && a.finish>QDateTime(QDate(2026,11,9),QTime(0,0)) && a.status!="Annulé")++expected;
  ServicePlanningDialog planning(QDate(2026,11,9));planning.show();QTest::qWait(100);
  QCOMPARE(planning.findChild<QTableWidget *>("servicePlanningTable")->rowCount(),expected);
  QVERIFY(planning.findChild<ServiceTimetable *>());QVERIFY(!planning.findChild<QLabel *>("planningMessage")->text().contains("Chargement impossible"));
  const auto folder=qEnvironmentVariable("SERVICE_CAPTURE_DIR");
  if(!folder.isEmpty())QVERIFY(planning.grab().save(folder+"/planning.png"));
  planning.hide();
  ServiceStatisticsDialog stats;stats.show();QTest::qWait(200);
  QVERIFY(stats.findChild<ServiceCurve *>());QVERIFY(stats.findChild<QTimer *>("statisticsLiveRefresh")->isActive());
  QVERIFY(stats.findChild<QLabel *>("statisticsNotice")->text().contains("Mis à jour"));
  if(!folder.isEmpty())QVERIFY(stats.grab().save(folder+"/statistics.png"));
 }
''')
changes['tests/service_only_tests.cpp']=test
for name,text in changes.items():
    dest=target/name
    if dest.exists():
        old=backup/name;old.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(dest,old)
    dest.parent.mkdir(parents=True,exist_ok=True)
    dest.write_text(text,encoding='utf-8')
print(f'Updated {len(changes)} files. Backup: {backup}')
