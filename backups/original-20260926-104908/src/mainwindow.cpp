#include "mainwindow.h"
#include "database/databasemanager.h"
#include "dialogs/employeedialog.h"
#include "models/employee.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QFont>
#include <QStatusBar>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QFrame>
#include <QIcon>
#include <QSpacerItem>

// ─── Inline Employee Management Widget ───────────────────────────────────────
class EmployeeManagementWidget : public QWidget {
    Q_OBJECT
public:
    explicit EmployeeManagementWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setStyleSheet(R"(
            QWidget { background: #1e1e2e; color: #cdd6f4; font-size: 13px; }
            QTableWidget { background: #181825; border: 1px solid #313244; border-radius: 10px; gridline-color: transparent; alternate-background-color: #202032; }
            QTableWidget::item { padding: 10px; border-bottom: 1px solid #25253a; }
            QTableWidget::item:selected { background: #313244; color: #a6e3a1; }
            QHeaderView::section { background: #313244; color: #a6e3a1; font-weight: bold; padding: 12px 10px; border: none; border-bottom: 2px solid #a6e3a1; }
            QPushButton { border: none; border-radius: 7px; padding: 9px 18px; font-weight: bold; }
            QPushButton#addBtn { background: #a6e3a1; color: #1e1e2e; }
            QPushButton#addBtn:hover { background: #94e2d5; }
            QPushButton#editBtn { background: #f9e2af; color: #1e1e2e; }
            QPushButton#editBtn:hover { background: #fab387; }
            QPushButton#deleteBtn { background: #f38ba8; color: #1e1e2e; }
            QPushButton#deleteBtn:hover { background: #eba0ac; }
        )");

        auto *root = new QVBoxLayout(this);
        root->setSpacing(16);
        root->setContentsMargins(28, 26, 28, 24);

        auto *header = new QHBoxLayout;
        auto *titleBlock = new QVBoxLayout;
        auto *titleLbl = new QLabel("Équipe", this);
        QFont tf = titleLbl->font(); tf.setPointSize(22); tf.setBold(true);
        titleLbl->setFont(tf); titleLbl->setStyleSheet("color: #a6e3a1;");
        titleBlock->addWidget(titleLbl);
        auto *subtitle = new QLabel("Gérez les collaborateurs et leurs coordonnées.", this);
        subtitle->setStyleSheet("color: #8187a1; font-size: 13px;");
        titleBlock->addWidget(subtitle);
        header->addLayout(titleBlock);
        header->addStretch();
        root->addLayout(header);

        m_table = new QTableWidget(this);
        m_table->setColumnCount(6);
        m_table->setHorizontalHeaderLabels({"ID", "Prénom", "Nom", "Poste", "Téléphone", "Email"});
        m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        m_table->verticalHeader()->setVisible(false);
        m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_table->setSelectionMode(QAbstractItemView::SingleSelection);
        m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_table->setAlternatingRowColors(true);
        m_table->setShowGrid(false);
        m_table->setMinimumHeight(260);
        root->addWidget(m_table, 1);

        auto *actBar = new QHBoxLayout;
        auto *countLabel = new QLabel(this);
        countLabel->setObjectName("employeeCount");
        countLabel->setStyleSheet("color: #8187a1; font-size: 12px;");
        actBar->addWidget(countLabel);
        actBar->addStretch();
        auto *addBtn    = new QPushButton("＋  Ajouter", this);    addBtn->setObjectName("addBtn");
        auto *editBtn   = new QPushButton("Modifier", this);     editBtn->setObjectName("editBtn");
        auto *deleteBtn = new QPushButton("Supprimer", this);    deleteBtn->setObjectName("deleteBtn");
        actBar->addWidget(addBtn);
        actBar->addWidget(editBtn);
        actBar->addWidget(deleteBtn);
        root->addLayout(actBar);

        connect(addBtn,    &QPushButton::clicked, this, &EmployeeManagementWidget::onAdd);
        connect(editBtn,   &QPushButton::clicked, this, &EmployeeManagementWidget::onEdit);
        connect(deleteBtn, &QPushButton::clicked, this, &EmployeeManagementWidget::onDelete);

        refresh();
    }

    void refresh() {
        m_employees = DatabaseManager::instance().getAllEmployees();
        m_table->setRowCount(0);
        for (const auto &e : m_employees) {
            int row = m_table->rowCount();
            m_table->insertRow(row);
            m_table->setItem(row,0, new QTableWidgetItem(QString::number(e.id)));
            m_table->setItem(row,1, new QTableWidgetItem(e.firstName));
            m_table->setItem(row,2, new QTableWidgetItem(e.lastName));
            m_table->setItem(row,3, new QTableWidgetItem(e.poste));
            m_table->setItem(row,4, new QTableWidgetItem(e.phone));
            m_table->setItem(row,5, new QTableWidgetItem(e.email));
            m_table->setRowHeight(row, 48);
        }
        if (auto *countLabel = findChild<QLabel *>("employeeCount"))
            countLabel->setText(QString("%1 collaborateur(s)").arg(m_employees.size()));
    }

private slots:
    void onAdd() {
        EmployeeDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            Employee e = dlg.getEmployee();
            if (e.firstName.isEmpty() || e.lastName.isEmpty()) {
                QMessageBox::warning(this, "Erreur", "Prenom et nom sont obligatoires.");
                return;
            }
            if (!DatabaseManager::instance().addEmployee(e))
                QMessageBox::critical(this, "Erreur", "Impossible d'ajouter l'employe.");
            else refresh();
        }
    }

    void onEdit() {
        int row = m_table->currentRow();
        if (row < 0) { QMessageBox::information(this, "Info", "Selectionnez un employe."); return; }
        int id = m_table->item(row,0)->text().toInt();
        Employee cur;
        for (const auto &e : m_employees) if (e.id == id) { cur = e; break; }
        EmployeeDialog dlg(cur, this);
        if (dlg.exec() == QDialog::Accepted) {
            Employee e = dlg.getEmployee();
            if (!DatabaseManager::instance().updateEmployee(e))
                QMessageBox::critical(this, "Erreur", "Impossible de modifier l'employe.");
            else refresh();
        }
    }

    void onDelete() {
        int row = m_table->currentRow();
        if (row < 0) { QMessageBox::information(this, "Info", "Selectionnez un employe."); return; }
        int id = m_table->item(row,0)->text().toInt();
        QString name = m_table->item(row,1)->text() + " " + m_table->item(row,2)->text();
        auto ans = QMessageBox::question(this, "Confirmation",
            QString("Supprimer l'employe \"%1\" ?").arg(name),
            QMessageBox::Yes | QMessageBox::No);
        if (ans == QMessageBox::Yes) {
            if (!DatabaseManager::instance().deleteEmployee(id))
                QMessageBox::critical(this, "Erreur", "Impossible de supprimer l'employe.");
            else refresh();
        }
    }

private:
    QTableWidget    *m_table;
    QList<Employee>  m_employees;
};

#include "mainwindow.moc"

// ─── MainWindow ──────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    applyGlobalStyle();
    setupUi();
    showConnectionDialog();
}

void MainWindow::applyGlobalStyle() {
    setStyleSheet(R"(
        QMainWindow { background: #1e1e2e; }
        QStatusBar  { background: #181825; color: #6c7086; font-size: 12px; }
    )");
}

void MainWindow::setupUi() {
    setWindowTitle("Gestion de Service");
    setMinimumSize(1200, 750);
    resize(1400, 850);

    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    setupSidebar();
    mainLayout->addWidget(m_sidebar);

    // Separator line
    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("color: #313244;");
    mainLayout->addWidget(sep);

    setupContent();
    mainLayout->addWidget(m_stack, 1);

    // Status bar
    m_statusLabel = new QLabel("Deconnecte de la base de donnees", this);
    statusBar()->addPermanentWidget(m_statusLabel);
}

void MainWindow::setupSidebar() {
    m_sidebar = new QWidget(this);
    m_sidebar->setFixedWidth(220);
    m_sidebar->setStyleSheet(R"(
        QWidget { background: #181825; }
        QListWidget {
            background: #181825;
            border: none;
            padding: 8px 0;
        }
        QListWidget::item {
            height: 48px;
            padding: 0 20px;
            color: #6c7086;
            font-size: 14px;
            border-radius: 0;
        }
        QListWidget::item:hover   { background: #1e1e2e; color: #cdd6f4; }
        QListWidget::item:selected { background: #313244; color: #89b4fa; border-left: 3px solid #89b4fa; }
    )");

    auto *sideLayout = new QVBoxLayout(m_sidebar);
    sideLayout->setContentsMargins(0,0,0,0);
    sideLayout->setSpacing(0);

    // Logo / App name
    auto *logoLabel = new QLabel("  GestionService", m_sidebar);
    QFont lf = logoLabel->font(); lf.setPointSize(14); lf.setBold(true);
    logoLabel->setFont(lf);
    logoLabel->setStyleSheet("color: #89b4fa; padding: 24px 20px; background: #181825;");
    sideLayout->addWidget(logoLabel);

    auto *divider = new QFrame(m_sidebar);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("color: #313244;");
    sideLayout->addWidget(divider);

    m_navList = new QListWidget(m_sidebar);
    m_navList->addItem("  Dashboard");
    m_navList->addItem("  Services");
    m_navList->addItem("  Employes");
    m_navList->addItem("  Planning");
    m_navList->addItem("  Calendrier");
    m_navList->setCurrentRow(0);
    sideLayout->addWidget(m_navList, 1);

    // Version label at bottom
    auto *versionLbl = new QLabel("  v1.0.0", m_sidebar);
    versionLbl->setStyleSheet("color: #45475a; font-size: 11px; padding: 12px 20px;");
    sideLayout->addWidget(versionLbl);

    connect(m_navList, &QListWidget::currentRowChanged, this, &MainWindow::onNavChanged);
}

void MainWindow::setupContent() {
    m_stack = new QStackedWidget(this);
    m_stack->setStyleSheet("QStackedWidget { background: #1e1e2e; }");

    m_dashboard      = new DashboardWidget(this);
    m_serviceWidget  = new ServiceWidget(this);
    m_employeeWidget = new EmployeeManagementWidget(this);
    m_scheduleWidget = new ScheduleWidget(this);
    m_calendarWidget = new CalendarWidget(this);

    m_stack->addWidget(m_dashboard);      // index 0
    m_stack->addWidget(m_serviceWidget);  // index 1
    m_stack->addWidget(m_employeeWidget); // index 2
    m_stack->addWidget(m_scheduleWidget); // index 3
    m_stack->addWidget(m_calendarWidget); // index 4
}

void MainWindow::onNavChanged(int row) {
    m_stack->setCurrentIndex(row);
    // Refresh the current widget when switching
    switch (row) {
    case 0: m_dashboard->refresh();      break;
    case 1: m_serviceWidget->refresh();  break;
    case 2: static_cast<EmployeeManagementWidget*>(m_employeeWidget)->refresh(); break;
    case 3: m_scheduleWidget->refresh(); break;
    case 4: m_calendarWidget->refresh(); break;
    }
}

void MainWindow::showConnectionDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Connexion MySQL");
    dlg.setMinimumWidth(380);
    dlg.setStyleSheet(R"(
        QDialog { background: #1e1e2e; color: #cdd6f4; }
        QLabel  { color: #cdd6f4; }
        QLineEdit, QSpinBox { background: #313244; border: 1px solid #45475a; border-radius: 6px; padding: 6px 10px; color: #cdd6f4; }
        QLineEdit:focus, QSpinBox:focus { border: 1px solid #89b4fa; }
        QPushButton#connectBtn { background: #89b4fa; color: #1e1e2e; border-radius: 6px; padding: 8px 24px; font-weight: bold; }
        QPushButton#connectBtn:hover { background: #b4befe; }
        QPushButton#skipBtn { background: #313244; color: #cdd6f4; border-radius: 6px; padding: 8px 24px; }
        QPushButton#skipBtn:hover { background: #45475a; }
    )");

    auto *layout = new QVBoxLayout(&dlg);
    layout->setSpacing(16);
    layout->setContentsMargins(24,24,24,24);

    auto *title = new QLabel("Connexion a la base de donnees", &dlg);
    QFont tf = title->font(); tf.setPointSize(14); tf.setBold(true);
    title->setFont(tf); title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #89b4fa;");
    layout->addWidget(title);

    auto *form = new QFormLayout;
    auto *hostEdit = new QLineEdit("localhost", &dlg);
    auto *portSpin = new QSpinBox(&dlg);
    portSpin->setRange(1, 65535); portSpin->setValue(3306);
    auto *dbEdit   = new QLineEdit("gestion_service", &dlg);
    auto *userEdit = new QLineEdit("root", &dlg);
    auto *passEdit = new QLineEdit(&dlg);
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setPlaceholderText("Mot de passe...");

    form->addRow("Hote",        hostEdit);
    form->addRow("Port",        portSpin);
    form->addRow("Base",        dbEdit);
    form->addRow("Utilisateur", userEdit);
    form->addRow("Mot de passe", passEdit);
    layout->addLayout(form);

    auto *btnLayout = new QHBoxLayout;
    auto *skipBtn    = new QPushButton("Ignorer", &dlg);     skipBtn->setObjectName("skipBtn");
    auto *connectBtn = new QPushButton("Connecter", &dlg);   connectBtn->setObjectName("connectBtn");
    btnLayout->addStretch();
    btnLayout->addWidget(skipBtn);
    btnLayout->addWidget(connectBtn);
    layout->addLayout(btnLayout);

    connect(connectBtn, &QPushButton::clicked, [&]() {
        bool ok = DatabaseManager::instance().connect(
            hostEdit->text(), portSpin->value(),
            dbEdit->text(), userEdit->text(), passEdit->text()
        );
        if (ok) {
            m_statusLabel->setText(QString("Connecte a %1@%2/%3").arg(userEdit->text()).arg(hostEdit->text()).arg(dbEdit->text()));
            m_statusLabel->setStyleSheet("color: #a6e3a1;");
            m_dashboard->refresh();
            dlg.accept();
        } else {
            QMessageBox::critical(&dlg, "Erreur de connexion",
                "Impossible de se connecter a MySQL.\nVerifiez vos parametres et assurez-vous que MySQL est en cours d'execution.");
        }
    });
    connect(skipBtn, &QPushButton::clicked, [&]() {
        m_statusLabel->setText("Mode hors-ligne (pas de base de donnees)");
        m_statusLabel->setStyleSheet("color: #f38ba8;");
        dlg.reject();
    });

    dlg.exec();
}
