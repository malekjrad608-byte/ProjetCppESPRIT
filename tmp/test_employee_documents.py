from pathlib import Path
p=Path('tests/service_tests.cpp');s=p.read_text(encoding='utf-8')
s=s.replace('QCOMPARE(stack->count(),3);','QCOMPARE(stack->count(),4);').replace('QCOMPARE(window.findChild<QStackedWidget *>("pageStack")->count(),3);','QCOMPARE(window.findChild<QStackedWidget *>("pageStack")->count(),4);')
s=s.replace('message->done(message->standardButtons().testFlag(QMessageBox::Yes)?QMessageBox::Yes:QMessageBox::Ok);','if(auto *yes=message->button(QMessageBox::Yes)) yes->click(); else message->accept();')
s=s.replace('        QVERIFY(q.exec("SELECT COUNT(*) FROM planning_affectations")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),0);','        QVERIFY(q.exec("SELECT COUNT(*) FROM employes WHERE nom=\'Integration\'")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),0);\n        QVERIFY(q.exec("SELECT COUNT(*) FROM planning_affectations")); QVERIFY(q.next()); QCOMPARE(q.value(0).toInt(),0);')
a=s.index('    void renderScreens()')
s=s[:a]+r'''
    void employeeLeavePdfAndPasswordDialog() {
        auto &manager=DatabaseManager::instance(); QVERIFY(manager.initialize(":memory:"));
        QString error; QVERIFY(EmployeeStore::importInitial(QSqlDatabase::database("aniworld-services"),&error));
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
''' + s[a:]
s=s.replace('#include <QDoubleSpinBox>','#include <QDoubleSpinBox>\n#include <QPainter>')
p.write_text(s,encoding='utf-8')
