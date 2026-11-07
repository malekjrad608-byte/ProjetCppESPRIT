/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_CHECK_H
#define UI_MAINWINDOW_CHECK_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "src/locaux/cardgrid.h"
#include "src/locaux/clinicmap.h"
#include "src/locaux/detailpanel.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *integratedLayout;
    QStackedWidget *pageStack;
    QWidget *ServicePage;
    QVBoxLayout *Service_control_1;
    QStackedWidget *Service_m_pages;
    QWidget *Service_m_listPage;
    QVBoxLayout *Service_control_3;
    QHBoxLayout *Service_control_4;
    QVBoxLayout *Service_control_5;
    QHBoxLayout *Service_control_6;
    QLabel *Service_control_12;
    QLabel *Service_control_13;
    QSpacerItem *spacerItem;
    QLabel *Service_control_14;
    QSpacerItem *spacerItem1;
    QPushButton *servicePlanning;
    QPushButton *addService;
    QHBoxLayout *Service_control_7;
    QLabel *Service_summary_0;
    QLabel *Service_summary_1;
    QLabel *Service_summary_2;
    QLabel *Service_summary_3;
    QSpacerItem *spacerItem2;
    QPushButton *serviceStatistics;
    QLabel *Service_m_notice;
    QHBoxLayout *Service_control_8;
    QLineEdit *Service_m_search;
    QComboBox *Service_m_category;
    QComboBox *Service_m_status;
    QHBoxLayout *Service_control_9;
    QComboBox *Service_m_period;
    QLabel *Service_control_50;
    QComboBox *Service_m_sort;
    QPushButton *resetFilters;
    QSpacerItem *spacerItem3;
    QLabel *Service_m_count;
    QPushButton *cardsView;
    QPushButton *tableView;
    QHBoxLayout *Service_control_10;
    QStackedWidget *Service_m_views;
    QScrollArea *Service_m_gridScroll;
    QWidget *Service_control_89;
    QGridLayout *Service_m_grid;
    QTableView *Service_m_table;
    QFrame *Service_control_96;
    QVBoxLayout *Service_control_97;
    QSpacerItem *spacerItem4;
    QLabel *Service_control_98;
    QLabel *Service_m_emptyTitle;
    QLabel *Service_m_emptyText;
    QPushButton *serviceFirstAdd;
    QSpacerItem *spacerItem5;
    QWidget *Service_control_99;
    QVBoxLayout *Service_control_100;
    QFrame *serviceDetailPanel;
    QVBoxLayout *Service_control_101;
    QLabel *Service_control_102;
    QLabel *Service_m_detailName;
    QLabel *Service_m_detailReference;
    QWidget *Service_m_detailContent;
    QVBoxLayout *Service_control_103;
    QVBoxLayout *Service_m_statusLayout;
    QFrame *Service_control_105;
    QScrollArea *serviceInfoScroll;
    QWidget *Service_control_107;
    QVBoxLayout *Service_control_108;
    QGridLayout *Service_control_109;
    QLabel *Service_control_110;
    QLabel *Service_detailValue_0;
    QLabel *Service_control_111;
    QLabel *Service_detailValue_1;
    QLabel *Service_control_112;
    QLabel *Service_detailValue_2;
    QLabel *Service_control_113;
    QLabel *Service_detailValue_3;
    QLabel *Service_control_114;
    QLabel *Service_detailValue_4;
    QLabel *Service_control_115;
    QLabel *Service_detailValue_5;
    QLabel *Service_control_116;
    QLabel *Service_detailValue_6;
    QLabel *Service_control_117;
    QLabel *Service_detailValue_7;
    QLabel *Service_m_detailNotes;
    QSpacerItem *spacerItem6;
    QHBoxLayout *Service_control_104;
    QPushButton *Service_m_view;
    QPushButton *Service_m_invoice;
    QPushButton *Service_m_edit;
    QPushButton *Service_m_advance;
    QPushButton *Service_m_delete;
    QWidget *Service_m_emptyDetailSpacer;
    QLabel *Service_m_attention;
    QHBoxLayout *Service_control_11;
    QLabel *Service_control_124;
    QSpacerItem *spacerItem7;
    QLabel *Service_control_125;
    QWidget *StockPage;
    QVBoxLayout *Stock_control_1;
    QHBoxLayout *Stock_control_2;
    QVBoxLayout *Stock_control_3;
    QLabel *Stock_control_7;
    QLabel *Stock_control_8;
    QSpacerItem *spacerItem8;
    QPushButton *stockMovementButton;
    QPushButton *stockExportButton;
    QHBoxLayout *Stock_control_4;
    QPushButton *stockStatisticsButton;
    QSpacerItem *spacerItem9;
    QPushButton *stockEditButton;
    QPushButton *stockDeleteButton;
    QHBoxLayout *Stock_control_5;
    QLineEdit *Stock_searchLineEdit;
    QComboBox *Stock_categoryFilter;
    QComboBox *Stock_sortFilter;
    QComboBox *Stock_stateFilter;
    QTableView *Stock_stockTable;
    QFrame *AdvancedModulesPanel;
    QVBoxLayout *Stock_control_67;
    QHBoxLayout *Stock_control_68;
    QLabel *Stock_control_70;
    QSpacerItem *spacerItem10;
    QLabel *Stock_control_71;
    QHBoxLayout *Stock_control_69;
    QPushButton *stockPurchasesButton;
    QPushButton *Stock_alertsNavButton;
    QPushButton *stockReplenishmentButton;
    QFrame *Stock_control_72;
    QHBoxLayout *Stock_control_73;
    QVBoxLayout *Stock_control_74;
    QLabel *Stock_control_75;
    QLabel *Stock_control_76;
    QPushButton *stockExpiryButton;
    QHBoxLayout *Stock_control_6;
    QLabel *Stock_footerLabel;
    QSpacerItem *spacerItem11;
    QLabel *Stock_control_77;
    QWidget *EmployeesPage;
    QVBoxLayout *Employees_control_1;
    QWidget *Employees_control_3;
    QHBoxLayout *Employees_control_4;
    QVBoxLayout *Employees_control_5;
    QHBoxLayout *Employees_control_6;
    QLabel *Employees_control_7;
    QLabel *Employees_control_8;
    QSpacerItem *spacerItem12;
    QLabel *Employees_control_9;
    QSpacerItem *spacerItem13;
    QPushButton *employeeLeavePdf;
    QPushButton *addEmployee;
    QWidget *Employees_control_10;
    QHBoxLayout *Employees_control_11;
    QFrame *Employees_control_12;
    QHBoxLayout *Employees_control_13;
    QLabel *Employees_control_14;
    QLabel *Employees_m_lblDisponibles;
    QLabel *Employees_control_15;
    QFrame *Employees_control_16;
    QHBoxLayout *Employees_control_17;
    QLabel *Employees_control_18;
    QLabel *Employees_m_lblIndisponibles;
    QLabel *Employees_control_19;
    QFrame *Employees_control_20;
    QHBoxLayout *Employees_control_21;
    QLabel *Employees_control_22;
    QLabel *Employees_m_lblChargeTotale;
    QLabel *Employees_control_23;
    QSpacerItem *spacerItem14;
    QPushButton *employeeStatistics;
    QWidget *Employees_control_24;
    QHBoxLayout *Employees_control_25;
    QLineEdit *Employees_m_editRecherche;
    QComboBox *Employees_m_comboPoste;
    QComboBox *Employees_m_comboStatut;
    QComboBox *Employees_m_comboTri;
    QHBoxLayout *Employees_control_2;
    QScrollArea *Employees_control_60;
    QWidget *Employees_m_cardsContainer;
    QGridLayout *Employees_m_gridCards;
    QFrame *employeeDetailPanel;
    QVBoxLayout *Employees_control_69;
    QLabel *Employees_control_70;
    QLabel *Employees_m_detailPlaceholder;
    QScrollArea *Employees_control_71;
    QWidget *Employees_m_detailContent;
    QVBoxLayout *Employees_control_73;
    QLabel *Employees_m_detailNom;
    QLabel *Employees_m_detailId;
    QLabel *Employees_m_detailStatut;
    QSpacerItem *spacerItem15;
    QFrame *Employees_control_81;
    QSpacerItem *spacerItem16;
    QHBoxLayout *Employees_control_74;
    QLabel *Employees_control_82;
    QLabel *Employees_m_detailPoste;
    QHBoxLayout *Employees_control_75;
    QLabel *Employees_control_83;
    QLabel *Employees_m_detailCourriel;
    QHBoxLayout *Employees_control_76;
    QLabel *Employees_control_84;
    QLabel *Employees_m_detailTelephone;
    QHBoxLayout *Employees_control_77;
    QLabel *Employees_control_85;
    QLabel *Employees_m_detailDate;
    QHBoxLayout *Employees_control_78;
    QLabel *Employees_control_86;
    QLabel *Employees_m_detailCharge;
    QHBoxLayout *Employees_control_79;
    QLabel *Employees_control_87;
    QLabel *Employees_m_detailDispo;
    QHBoxLayout *Employees_control_80;
    QLabel *Employees_control_88;
    QLabel *Employees_m_detailCompetences;
    QSpacerItem *spacerItem17;
    QWidget *Employees_m_detailActions;
    QVBoxLayout *Employees_control_95;
    QPushButton *editEmployee;
    QPushButton *toggleEmployeeAvailability;
    QPushButton *deleteEmployee;
    QWidget *LocauxPage;
    QVBoxLayout *Locaux_control_1;
    QVBoxLayout *Locaux_control_2;
    QVBoxLayout *Locaux_control_3;
    QHBoxLayout *Locaux_control_4;
    QLabel *Locaux_control_13;
    QLabel *h1;
    QSpacerItem *spacerItem18;
    QPushButton *Locaux_m_btnAdd;
    QLabel *sub;
    QSpacerItem *spacerItem19;
    QHBoxLayout *Locaux_control_5;
    QPushButton *Locaux_m_chipAvail;
    QPushButton *Locaux_m_chipOcc;
    QPushButton *Locaux_m_chipOos;
    QPushButton *Locaux_m_chipDirty;
    QSpacerItem *spacerItem20;
    QVBoxLayout *Locaux_control_6;
    QHBoxLayout *Locaux_control_7;
    QSpacerItem *spacerItem21;
    QPushButton *Locaux_m_btnAlerts;
    QPushButton *Locaux_m_btnSmart;
    QHBoxLayout *Locaux_control_8;
    QPushButton *Locaux_m_btnPdf;
    QPushButton *Locaux_m_btnMap;
    QPushButton *Locaux_m_btnStats;
    QSpacerItem *spacerItem22;
    QVBoxLayout *Locaux_control_9;
    QHBoxLayout *Locaux_control_10;
    QLineEdit *Locaux_m_search;
    QComboBox *Locaux_m_sortCombo;
    QPushButton *locauxResetFilters;
    QLabel *Locaux_m_count;
    QPushButton *Locaux_m_btnCards;
    QPushButton *Locaux_m_btnList;
    QLabel *Locaux_m_queryHint;
    QHBoxLayout *Locaux_control_11;
    QVBoxLayout *Locaux_control_12;
    QStackedWidget *Locaux_m_views;
    QScrollArea *Locaux_m_cardScroll;
    CardGrid *Locaux_m_grid;
    QTableWidget *Locaux_m_table;
    QLabel *Locaux_m_empty;
    QScrollArea *Locaux_control_71;
    QWidget *Locaux_control_73;
    QVBoxLayout *Locaux_control_74;
    DetailPanel *Locaux_m_detail;
    QFrame *panel;
    QVBoxLayout *Locaux_control_102;
    QHBoxLayout *Locaux_control_103;
    QLabel *Locaux_control_104;
    QSpacerItem *spacerItem23;
    QPushButton *locauxEnlargeMap;
    ClinicMap *Locaux_m_miniMap;
    QLabel *Locaux_control_105;
    QFrame *Locaux_control_106;
    QVBoxLayout *Locaux_control_107;
    QLabel *panelTitle;
    QLabel *Locaux_m_pct;
    QProgressBar *Locaux_m_bar;
    QLabel *Locaux_m_globalText;
    QSpacerItem *spacerItem24;
    QMainWindow *MedicalPage;
    QWidget *Medical_centralwidget;
    QWidget *Medical_sidebar;
    QLabel *Medical_logoIcon;
    QLabel *Medical_brandName;
    QLabel *Medical_brandTag;
    QFrame *Medical_sectionCard;
    QLabel *Medical_sectionTitle;
    QLabel *Medical_sectionSub;
    QLabel *Medical_navLabel;
    QPushButton *Medical_btnNavDashboard;
    QPushButton *Medical_btnNavServices;
    QPushButton *Medical_btnNavEmployees;
    QPushButton *Medical_btnNavStock;
    QPushButton *Medical_btnNavPremises;
    QPushButton *Medical_btnNavRecords;
    QLabel *Medical_sloganLabel;
    QLabel *Medical_sloganSub;
    QWidget *Medical_mainContent;
    QWidget *Medical_topBar;
    QLabel *Medical_crumbGestion;
    QLabel *Medical_crumbSep;
    QLabel *Medical_crumbCurrent;
    QLabel *Medical_demoBadge;
    QPushButton *Medical_btnFile;
    QLabel *Medical_roleLabel;
    QWidget *Medical_pageArea;
    QWidget *Medical_recordsPage;
    QLabel *Medical_pageTitle;
    QLabel *Medical_pageSubtitle;
    QPushButton *Medical_btnAdd;
    QLabel *Medical_chipTotal;
    QLabel *Medical_chipConsultations;
    QLabel *Medical_chipUpToDate;
    QLabel *Medical_chipToRenew;
    QPushButton *Medical_btnStatsTop;
    QLineEdit *Medical_searchEdit;
    QComboBox *Medical_filterSpecies;
    QComboBox *Medical_filterVaccination;
    QComboBox *Medical_sortCombo;
    QPushButton *Medical_btnReset;
    QScrollArea *Medical_cardsScroll;
    QWidget *Medical_cardsContainer;
    QLabel *Medical_emptyLabel;
    QWidget *Medical_detailPanel;
    QFrame *Medical_selectedCard;
    QLabel *Medical_eyebrow;
    QLabel *Medical_placeholderText;
    QWidget *Medical_detailContent;
    QLabel *Medical_panelTitle;
    QLabel *Medical_statusPill;
    QLabel *Medical_detailLine1;
    QLabel *Medical_detailLine2;
    QLabel *Medical_detailLine3;
    QPushButton *Medical_btnView;
    QPushButton *Medical_btnToggle;
    QPushButton *Medical_btnEdit;
    QPushButton *Medical_btnDelete;
    QFrame *Medical_globalCard;
    QLabel *Medical_panelSubTitle;
    QLabel *Medical_bigValue;
    QProgressBar *Medical_globalBar;
    QLabel *Medical_globalText;
    QPushButton *Medical_btnStatsPanel;
    QWidget *Medical_pageFooter;
    QLabel *Medical_footerLeft;
    QLabel *Medical_footerDate;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        integratedLayout = new QVBoxLayout(centralWidget);
        integratedLayout->setObjectName("integratedLayout");
        pageStack = new QStackedWidget(centralWidget);
        pageStack->setObjectName("pageStack");
        ServicePage = new QWidget();
        ServicePage->setObjectName("ServicePage");
        ServicePage->setAutoFillBackground(true);
        ServicePage->setMinimumSize(QSize(0, 0));
        ServicePage->setMaximumSize(QSize(16777215, 16777215));
        ServicePage->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        ServicePage->setGeometry(QRect(0, 0, 640, 480));
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ServicePage->sizePolicy().hasHeightForWidth());
        ServicePage->setSizePolicy(sizePolicy);
        QFont font;
        font.setFamilies({QString::fromUtf8("Segoe UI")});
        ServicePage->setFont(font);
        Service_control_1 = new QVBoxLayout(ServicePage);
        Service_control_1->setSpacing(6);
        Service_control_1->setObjectName("Service_control_1");
        Service_control_1->setContentsMargins(0, 0, 0, 0);
        Service_m_pages = new QStackedWidget(ServicePage);
        Service_m_pages->setObjectName("Service_m_pages");
        Service_m_pages->setMinimumSize(QSize(0, 0));
        Service_m_pages->setAutoFillBackground(false);
        Service_m_pages->setMaximumSize(QSize(16777215, 16777215));
        Service_m_pages->setLineWidth(1);
        Service_m_pages->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_pages->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_m_pages->sizePolicy().hasHeightForWidth());
        Service_m_pages->setSizePolicy(sizePolicy);
        Service_m_pages->setMidLineWidth(0);
        Service_m_pages->setFont(font);
        Service_m_listPage = new QWidget();
        Service_m_listPage->setObjectName("Service_m_listPage");
        Service_m_listPage->setAutoFillBackground(false);
        Service_m_listPage->setMinimumSize(QSize(0, 0));
        Service_m_listPage->setMaximumSize(QSize(16777215, 16777215));
        Service_m_listPage->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_listPage->setGeometry(QRect(0, 0, 100, 30));
        sizePolicy.setHeightForWidth(Service_m_listPage->sizePolicy().hasHeightForWidth());
        Service_m_listPage->setSizePolicy(sizePolicy);
        Service_m_listPage->setFont(font);
        Service_control_3 = new QVBoxLayout(Service_m_listPage);
        Service_control_3->setSpacing(12);
        Service_control_3->setObjectName("Service_control_3");
        Service_control_3->setContentsMargins(24, 20, 24, 16);
        Service_control_4 = new QHBoxLayout();
        Service_control_4->setSpacing(12);
        Service_control_4->setObjectName("Service_control_4");
        Service_control_4->setContentsMargins(0, 0, 0, 0);
        Service_control_5 = new QVBoxLayout();
        Service_control_5->setSpacing(4);
        Service_control_5->setObjectName("Service_control_5");
        Service_control_5->setContentsMargins(0, 0, 0, 0);
        Service_control_6 = new QHBoxLayout();
        Service_control_6->setSpacing(9);
        Service_control_6->setObjectName("Service_control_6");
        Service_control_6->setContentsMargins(0, 0, 0, 0);
        Service_control_12 = new QLabel(Service_m_listPage);
        Service_control_12->setObjectName("Service_control_12");
        Service_control_12->setMinimumSize(QSize(0, 0));
        Service_control_12->setAutoFillBackground(false);
        Service_control_12->setMaximumSize(QSize(16777215, 16777215));
        Service_control_12->setLineWidth(1);
        Service_control_12->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_12->setWordWrap(false);
        Service_control_12->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_control_12->sizePolicy().hasHeightForWidth());
        Service_control_12->setSizePolicy(sizePolicy);
        Service_control_12->setMidLineWidth(0);
        Service_control_12->setFont(font);
        Service_control_12->setPixmap(QPixmap(QString::fromUtf8(":/designer/Service_control_12.png")));

        Service_control_6->addWidget(Service_control_12);

        Service_control_13 = new QLabel(Service_m_listPage);
        Service_control_13->setObjectName("Service_control_13");
        Service_control_13->setMinimumSize(QSize(0, 0));
        Service_control_13->setAutoFillBackground(false);
        Service_control_13->setMaximumSize(QSize(16777215, 16777215));
        Service_control_13->setLineWidth(1);
        Service_control_13->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_13->setWordWrap(false);
        Service_control_13->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_control_13->sizePolicy().hasHeightForWidth());
        Service_control_13->setSizePolicy(sizePolicy);
        Service_control_13->setMidLineWidth(0);
        QFont font1;
        font1.setFamilies({QString::fromUtf8("Segoe UI")});
        font1.setBold(true);
        Service_control_13->setFont(font1);

        Service_control_6->addWidget(Service_control_13);

        spacerItem = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Service_control_6->addItem(spacerItem);


        Service_control_5->addLayout(Service_control_6);

        Service_control_14 = new QLabel(Service_m_listPage);
        Service_control_14->setObjectName("Service_control_14");
        Service_control_14->setMinimumSize(QSize(0, 0));
        Service_control_14->setAutoFillBackground(false);
        Service_control_14->setMaximumSize(QSize(16777215, 16777215));
        Service_control_14->setLineWidth(1);
        Service_control_14->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_14->setWordWrap(false);
        Service_control_14->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_control_14->sizePolicy().hasHeightForWidth());
        Service_control_14->setSizePolicy(sizePolicy);
        Service_control_14->setMidLineWidth(0);
        Service_control_14->setFont(font);

        Service_control_5->addWidget(Service_control_14);


        Service_control_4->addLayout(Service_control_5);

        spacerItem1 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Service_control_4->addItem(spacerItem1);

        servicePlanning = new QPushButton(Service_m_listPage);
        servicePlanning->setObjectName("servicePlanning");
        servicePlanning->setMinimumSize(QSize(0, 0));
        servicePlanning->setChecked(false);
        servicePlanning->setIconSize(QSize(17, 17));
        servicePlanning->setAutoFillBackground(false);
        servicePlanning->setAutoDefault(false);
        servicePlanning->setMaximumSize(QSize(16777215, 16777215));
        servicePlanning->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        servicePlanning->setFlat(false);
        servicePlanning->setCheckable(false);
        servicePlanning->setGeometry(QRect(0, 0, 640, 480));
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(servicePlanning->sizePolicy().hasHeightForWidth());
        servicePlanning->setSizePolicy(sizePolicy1);
        QFont font2;
        font2.setFamilies({QString::fromUtf8("Segoe UI")});
        font2.setWeight(QFont::DemiBold);
        servicePlanning->setFont(font2);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/designer/servicePlanning.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        servicePlanning->setIcon(icon);

        Service_control_4->addWidget(servicePlanning);

        addService = new QPushButton(Service_m_listPage);
        addService->setObjectName("addService");
        addService->setMinimumSize(QSize(0, 0));
        addService->setChecked(false);
        addService->setIconSize(QSize(17, 17));
        addService->setAutoFillBackground(false);
        addService->setAutoDefault(false);
        addService->setMaximumSize(QSize(16777215, 16777215));
        addService->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        addService->setFlat(false);
        addService->setCheckable(false);
        addService->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(addService->sizePolicy().hasHeightForWidth());
        addService->setSizePolicy(sizePolicy1);
        addService->setFont(font2);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/designer/addService.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        addService->setIcon(icon1);

        Service_control_4->addWidget(addService);


        Service_control_3->addLayout(Service_control_4);

        Service_control_7 = new QHBoxLayout();
        Service_control_7->setSpacing(10);
        Service_control_7->setObjectName("Service_control_7");
        Service_control_7->setContentsMargins(0, 0, 0, 0);
        Service_summary_0 = new QLabel(Service_m_listPage);
        Service_summary_0->setObjectName("Service_summary_0");
        Service_summary_0->setMinimumSize(QSize(0, 0));
        Service_summary_0->setAutoFillBackground(false);
        Service_summary_0->setMaximumSize(QSize(16777215, 16777215));
        Service_summary_0->setLineWidth(1);
        Service_summary_0->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_summary_0->setWordWrap(false);
        Service_summary_0->setGeometry(QRect(0, 0, 640, 480));
        Service_summary_0->setStyleSheet(QString::fromUtf8("background:#E8EAF8;color:#706BA7;border-radius:14px;padding:6px 12px;font-size:11px;"));
        sizePolicy.setHeightForWidth(Service_summary_0->sizePolicy().hasHeightForWidth());
        Service_summary_0->setSizePolicy(sizePolicy);
        Service_summary_0->setMidLineWidth(0);
        Service_summary_0->setFont(font);

        Service_control_7->addWidget(Service_summary_0);

        Service_summary_1 = new QLabel(Service_m_listPage);
        Service_summary_1->setObjectName("Service_summary_1");
        Service_summary_1->setMinimumSize(QSize(0, 0));
        Service_summary_1->setAutoFillBackground(false);
        Service_summary_1->setMaximumSize(QSize(16777215, 16777215));
        Service_summary_1->setLineWidth(1);
        Service_summary_1->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_summary_1->setWordWrap(false);
        Service_summary_1->setGeometry(QRect(0, 0, 640, 480));
        Service_summary_1->setStyleSheet(QString::fromUtf8("background:#FFF0D9;color:#AC7B30;border-radius:14px;padding:6px 12px;font-size:11px;"));
        sizePolicy.setHeightForWidth(Service_summary_1->sizePolicy().hasHeightForWidth());
        Service_summary_1->setSizePolicy(sizePolicy);
        Service_summary_1->setMidLineWidth(0);
        Service_summary_1->setFont(font);

        Service_control_7->addWidget(Service_summary_1);

        Service_summary_2 = new QLabel(Service_m_listPage);
        Service_summary_2->setObjectName("Service_summary_2");
        Service_summary_2->setMinimumSize(QSize(0, 0));
        Service_summary_2->setAutoFillBackground(false);
        Service_summary_2->setMaximumSize(QSize(16777215, 16777215));
        Service_summary_2->setLineWidth(1);
        Service_summary_2->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_summary_2->setWordWrap(false);
        Service_summary_2->setGeometry(QRect(0, 0, 640, 480));
        Service_summary_2->setStyleSheet(QString::fromUtf8("background:#E5F2E9;color:#4C896A;border-radius:14px;padding:6px 12px;font-size:11px;"));
        sizePolicy.setHeightForWidth(Service_summary_2->sizePolicy().hasHeightForWidth());
        Service_summary_2->setSizePolicy(sizePolicy);
        Service_summary_2->setMidLineWidth(0);
        Service_summary_2->setFont(font);

        Service_control_7->addWidget(Service_summary_2);

        Service_summary_3 = new QLabel(Service_m_listPage);
        Service_summary_3->setObjectName("Service_summary_3");
        Service_summary_3->setMinimumSize(QSize(0, 0));
        Service_summary_3->setAutoFillBackground(false);
        Service_summary_3->setMaximumSize(QSize(16777215, 16777215));
        Service_summary_3->setLineWidth(1);
        Service_summary_3->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_summary_3->setWordWrap(false);
        Service_summary_3->setGeometry(QRect(0, 0, 640, 480));
        Service_summary_3->setStyleSheet(QString::fromUtf8("background:#E7F0F4;color:#4D7989;border-radius:14px;padding:6px 12px;font-size:11px;"));
        sizePolicy.setHeightForWidth(Service_summary_3->sizePolicy().hasHeightForWidth());
        Service_summary_3->setSizePolicy(sizePolicy);
        Service_summary_3->setMidLineWidth(0);
        Service_summary_3->setFont(font);

        Service_control_7->addWidget(Service_summary_3);

        spacerItem2 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Service_control_7->addItem(spacerItem2);

        serviceStatistics = new QPushButton(Service_m_listPage);
        serviceStatistics->setObjectName("serviceStatistics");
        serviceStatistics->setMinimumSize(QSize(0, 0));
        serviceStatistics->setChecked(false);
        serviceStatistics->setIconSize(QSize(17, 17));
        serviceStatistics->setAutoFillBackground(false);
        serviceStatistics->setAutoDefault(false);
        serviceStatistics->setMaximumSize(QSize(16777215, 16777215));
        serviceStatistics->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        serviceStatistics->setFlat(false);
        serviceStatistics->setCheckable(false);
        serviceStatistics->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(serviceStatistics->sizePolicy().hasHeightForWidth());
        serviceStatistics->setSizePolicy(sizePolicy1);
        serviceStatistics->setFont(font2);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/designer/serviceStatistics.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        serviceStatistics->setIcon(icon2);

        Service_control_7->addWidget(serviceStatistics);


        Service_control_3->addLayout(Service_control_7);

        Service_m_notice = new QLabel(Service_m_listPage);
        Service_m_notice->setObjectName("Service_m_notice");
        Service_m_notice->setMinimumSize(QSize(0, 0));
        Service_m_notice->setAutoFillBackground(false);
        Service_m_notice->setMaximumSize(QSize(16777215, 16777215));
        Service_m_notice->setLineWidth(1);
        Service_m_notice->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_notice->setWordWrap(true);
        Service_m_notice->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_m_notice->sizePolicy().hasHeightForWidth());
        Service_m_notice->setSizePolicy(sizePolicy);
        Service_m_notice->setMidLineWidth(0);
        QFont font3;
        Service_m_notice->setFont(font3);
        Service_m_notice->setVisible(false);

        Service_control_3->addWidget(Service_m_notice);

        Service_control_8 = new QHBoxLayout();
        Service_control_8->setSpacing(10);
        Service_control_8->setObjectName("Service_control_8");
        Service_control_8->setContentsMargins(0, 0, 0, 0);
        Service_m_search = new QLineEdit(Service_m_listPage);
        Service_m_search->setObjectName("Service_m_search");
        Service_m_search->setMinimumSize(QSize(0, 0));
        Service_m_search->setAutoFillBackground(false);
        Service_m_search->setMaxLength(32767);
        Service_m_search->setMaximumSize(QSize(16777215, 16777215));
        Service_m_search->setCursor(QCursor(Qt::CursorShape::IBeamCursor));
        Service_m_search->setReadOnly(false);
        Service_m_search->setClearButtonEnabled(true);
        Service_m_search->setGeometry(QRect(0, 0, 640, 480));
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(Service_m_search->sizePolicy().hasHeightForWidth());
        Service_m_search->setSizePolicy(sizePolicy2);
        Service_m_search->setFont(font);

        Service_control_8->addWidget(Service_m_search);

        Service_m_category = new QComboBox(Service_m_listPage);
        Service_m_category->addItem(QString());
        Service_m_category->addItem(QString());
        Service_m_category->addItem(QString());
        Service_m_category->addItem(QString());
        Service_m_category->addItem(QString());
        Service_m_category->addItem(QString());
        Service_m_category->addItem(QString());
        Service_m_category->addItem(QString());
        Service_m_category->addItem(QString());
        Service_m_category->setObjectName("Service_m_category");
        Service_m_category->setMinimumSize(QSize(0, 0));
        Service_m_category->setMinimumContentsLength(0);
        Service_m_category->setIconSize(QSize(16, 16));
        Service_m_category->setAutoFillBackground(false);
        Service_m_category->setMaximumSize(QSize(16777215, 16777215));
        Service_m_category->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_category->setGeometry(QRect(0, 0, 640, 480));
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(Service_m_category->sizePolicy().hasHeightForWidth());
        Service_m_category->setSizePolicy(sizePolicy3);
        Service_m_category->setEditable(false);
        Service_m_category->setFont(font3);

        Service_control_8->addWidget(Service_m_category);

        Service_m_status = new QComboBox(Service_m_listPage);
        Service_m_status->addItem(QString());
        Service_m_status->addItem(QString());
        Service_m_status->addItem(QString());
        Service_m_status->addItem(QString());
        Service_m_status->addItem(QString());
        Service_m_status->setObjectName("Service_m_status");
        Service_m_status->setMinimumSize(QSize(0, 0));
        Service_m_status->setMinimumContentsLength(0);
        Service_m_status->setIconSize(QSize(16, 16));
        Service_m_status->setAutoFillBackground(false);
        Service_m_status->setMaximumSize(QSize(16777215, 16777215));
        Service_m_status->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_status->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy3.setHeightForWidth(Service_m_status->sizePolicy().hasHeightForWidth());
        Service_m_status->setSizePolicy(sizePolicy3);
        Service_m_status->setEditable(false);
        Service_m_status->setFont(font3);

        Service_control_8->addWidget(Service_m_status);

        Service_control_8->setStretch(0, 1);

        Service_control_3->addLayout(Service_control_8);

        Service_control_9 = new QHBoxLayout();
        Service_control_9->setSpacing(10);
        Service_control_9->setObjectName("Service_control_9");
        Service_control_9->setContentsMargins(0, 0, 0, 0);
        Service_m_period = new QComboBox(Service_m_listPage);
        Service_m_period->addItem(QString());
        Service_m_period->addItem(QString());
        Service_m_period->addItem(QString());
        Service_m_period->addItem(QString());
        Service_m_period->setObjectName("Service_m_period");
        Service_m_period->setMinimumSize(QSize(0, 0));
        Service_m_period->setMinimumContentsLength(0);
        Service_m_period->setIconSize(QSize(16, 16));
        Service_m_period->setAutoFillBackground(false);
        Service_m_period->setMaximumSize(QSize(16777215, 16777215));
        Service_m_period->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_period->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy3.setHeightForWidth(Service_m_period->sizePolicy().hasHeightForWidth());
        Service_m_period->setSizePolicy(sizePolicy3);
        Service_m_period->setEditable(false);
        Service_m_period->setFont(font3);

        Service_control_9->addWidget(Service_m_period);

        Service_control_50 = new QLabel(Service_m_listPage);
        Service_control_50->setObjectName("Service_control_50");
        Service_control_50->setMinimumSize(QSize(0, 0));
        Service_control_50->setAutoFillBackground(false);
        Service_control_50->setMaximumSize(QSize(16777215, 16777215));
        Service_control_50->setLineWidth(1);
        Service_control_50->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_50->setWordWrap(false);
        Service_control_50->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_control_50->sizePolicy().hasHeightForWidth());
        Service_control_50->setSizePolicy(sizePolicy);
        Service_control_50->setMidLineWidth(0);
        Service_control_50->setFont(font);

        Service_control_9->addWidget(Service_control_50);

        Service_m_sort = new QComboBox(Service_m_listPage);
        Service_m_sort->addItem(QString());
        Service_m_sort->addItem(QString());
        Service_m_sort->addItem(QString());
        Service_m_sort->addItem(QString());
        Service_m_sort->addItem(QString());
        Service_m_sort->addItem(QString());
        Service_m_sort->addItem(QString());
        Service_m_sort->setObjectName("Service_m_sort");
        Service_m_sort->setMinimumSize(QSize(0, 0));
        Service_m_sort->setMinimumContentsLength(0);
        Service_m_sort->setIconSize(QSize(16, 16));
        Service_m_sort->setAutoFillBackground(false);
        Service_m_sort->setMaximumSize(QSize(16777215, 16777215));
        Service_m_sort->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_sort->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy3.setHeightForWidth(Service_m_sort->sizePolicy().hasHeightForWidth());
        Service_m_sort->setSizePolicy(sizePolicy3);
        Service_m_sort->setEditable(false);
        Service_m_sort->setFont(font3);

        Service_control_9->addWidget(Service_m_sort);

        resetFilters = new QPushButton(Service_m_listPage);
        resetFilters->setObjectName("resetFilters");
        resetFilters->setMinimumSize(QSize(0, 0));
        resetFilters->setChecked(false);
        resetFilters->setIconSize(QSize(17, 17));
        resetFilters->setAutoFillBackground(false);
        resetFilters->setAutoDefault(false);
        resetFilters->setMaximumSize(QSize(16777215, 16777215));
        resetFilters->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        resetFilters->setFlat(false);
        resetFilters->setCheckable(false);
        resetFilters->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(resetFilters->sizePolicy().hasHeightForWidth());
        resetFilters->setSizePolicy(sizePolicy1);
        resetFilters->setFont(font2);

        Service_control_9->addWidget(resetFilters);

        spacerItem3 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Service_control_9->addItem(spacerItem3);

        Service_m_count = new QLabel(Service_m_listPage);
        Service_m_count->setObjectName("Service_m_count");
        Service_m_count->setMinimumSize(QSize(0, 0));
        Service_m_count->setAutoFillBackground(false);
        Service_m_count->setMaximumSize(QSize(16777215, 16777215));
        Service_m_count->setLineWidth(1);
        Service_m_count->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_count->setWordWrap(false);
        Service_m_count->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_m_count->sizePolicy().hasHeightForWidth());
        Service_m_count->setSizePolicy(sizePolicy);
        Service_m_count->setMidLineWidth(0);
        Service_m_count->setFont(font);

        Service_control_9->addWidget(Service_m_count);

        cardsView = new QPushButton(Service_m_listPage);
        QButtonGroup *buttonGroup = new QButtonGroup(MainWindow);
        buttonGroup->setObjectName("buttonGroup");
        buttonGroup->addButton(cardsView);
        cardsView->setObjectName("cardsView");
        cardsView->setMinimumSize(QSize(0, 0));
        cardsView->setChecked(true);
        cardsView->setIconSize(QSize(17, 17));
        cardsView->setAutoFillBackground(false);
        cardsView->setAutoDefault(false);
        cardsView->setMaximumSize(QSize(16777215, 16777215));
        cardsView->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        cardsView->setFlat(false);
        cardsView->setCheckable(true);
        cardsView->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(cardsView->sizePolicy().hasHeightForWidth());
        cardsView->setSizePolicy(sizePolicy1);
        cardsView->setFont(font2);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/designer/cardsView.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        cardsView->setIcon(icon3);

        Service_control_9->addWidget(cardsView);

        tableView = new QPushButton(Service_m_listPage);
        buttonGroup->addButton(tableView);
        tableView->setObjectName("tableView");
        tableView->setMinimumSize(QSize(0, 0));
        tableView->setChecked(false);
        tableView->setIconSize(QSize(17, 17));
        tableView->setAutoFillBackground(false);
        tableView->setAutoDefault(false);
        tableView->setMaximumSize(QSize(16777215, 16777215));
        tableView->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        tableView->setFlat(false);
        tableView->setCheckable(true);
        tableView->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(tableView->sizePolicy().hasHeightForWidth());
        tableView->setSizePolicy(sizePolicy1);
        tableView->setFont(font2);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/designer/tableView.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        tableView->setIcon(icon4);

        Service_control_9->addWidget(tableView);


        Service_control_3->addLayout(Service_control_9);

        Service_control_10 = new QHBoxLayout();
        Service_control_10->setSpacing(17);
        Service_control_10->setObjectName("Service_control_10");
        Service_control_10->setContentsMargins(0, 0, 0, 0);
        Service_m_views = new QStackedWidget(Service_m_listPage);
        Service_m_views->setObjectName("Service_m_views");
        Service_m_views->setMinimumSize(QSize(0, 0));
        Service_m_views->setAutoFillBackground(false);
        Service_m_views->setMaximumSize(QSize(16777215, 16777215));
        Service_m_views->setLineWidth(1);
        Service_m_views->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_views->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_m_views->sizePolicy().hasHeightForWidth());
        Service_m_views->setSizePolicy(sizePolicy);
        Service_m_views->setMidLineWidth(0);
        Service_m_views->setFont(font);
        Service_m_gridScroll = new QScrollArea();
        Service_m_gridScroll->setObjectName("Service_m_gridScroll");
        Service_m_gridScroll->setMinimumSize(QSize(0, 0));
        Service_m_gridScroll->setWidgetResizable(true);
        Service_m_gridScroll->setAutoFillBackground(false);
        Service_m_gridScroll->setMaximumSize(QSize(16777215, 16777215));
        Service_m_gridScroll->setLineWidth(1);
        Service_m_gridScroll->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CursorShape::ArrowCursor)));
        Service_m_gridScroll->setGeometry(QRect(0, 0, 640, 480));
        QSizePolicy sizePolicy4(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(Service_m_gridScroll->sizePolicy().hasHeightForWidth());
        Service_m_gridScroll->setSizePolicy(sizePolicy4);
        Service_m_gridScroll->setMidLineWidth(0);
        Service_m_gridScroll->setFont(font);
        Service_control_89 = new QWidget();
        Service_control_89->setObjectName("Service_control_89");
        Service_control_89->setAutoFillBackground(false);
        Service_control_89->setMinimumSize(QSize(5, 0));
        Service_control_89->setMaximumSize(QSize(16777215, 16777215));
        Service_control_89->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_89->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_control_89->sizePolicy().hasHeightForWidth());
        Service_control_89->setSizePolicy(sizePolicy);
        Service_control_89->setFont(font);
        Service_m_grid = new QGridLayout(Service_control_89);
        Service_m_grid->setSpacing(12);
        Service_m_grid->setObjectName("Service_m_grid");
        Service_m_grid->setContentsMargins(0, 0, 5, 0);
        Service_m_grid->setColumnStretch(0, 1);
        Service_m_grid->setColumnStretch(1, 1);
        Service_m_gridScroll->setWidget(Service_control_89);
        Service_m_views->addWidget(Service_m_gridScroll);
        Service_m_table = new QTableView();
        Service_m_table->setObjectName("Service_m_table");
        Service_m_table->setMinimumSize(QSize(0, 0));
        Service_m_table->setIconSize(QSize(-1, -1));
        Service_m_table->setAutoFillBackground(false);
        Service_m_table->setMaximumSize(QSize(16777215, 16777215));
        Service_m_table->setAlternatingRowColors(false);
        Service_m_table->setSortingEnabled(false);
        Service_m_table->setLineWidth(1);
        Service_m_table->setShowGrid(false);
        Service_m_table->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CursorShape::ArrowCursor)));
        Service_m_table->setWordWrap(false);
        Service_m_table->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy4.setHeightForWidth(Service_m_table->sizePolicy().hasHeightForWidth());
        Service_m_table->setSizePolicy(sizePolicy4);
        Service_m_table->setMidLineWidth(0);
        Service_m_table->setFont(font);
        Service_m_table->setVisible(false);
        Service_m_views->addWidget(Service_m_table);
        Service_m_table->horizontalHeader()->setVisible(false);
        Service_m_table->horizontalHeader()->setCascadingSectionResizes(false);
        Service_m_table->horizontalHeader()->setMinimumSectionSize(28);
        Service_m_table->horizontalHeader()->setDefaultSectionSize(100);
        Service_m_table->horizontalHeader()->setHighlightSections(false);
        Service_m_table->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
        Service_m_table->horizontalHeader()->setStretchLastSection(false);
        Service_m_table->verticalHeader()->setVisible(false);
        Service_m_table->verticalHeader()->setCascadingSectionResizes(false);
        Service_m_table->verticalHeader()->setMinimumSectionSize(20);
        Service_m_table->verticalHeader()->setDefaultSectionSize(65);
        Service_m_table->verticalHeader()->setHighlightSections(true);
        Service_m_table->verticalHeader()->setProperty("showSortIndicator", QVariant(false));
        Service_m_table->verticalHeader()->setStretchLastSection(false);
        Service_control_96 = new QFrame();
        Service_control_96->setObjectName("Service_control_96");
        Service_control_96->setMinimumSize(QSize(0, 0));
        Service_control_96->setAutoFillBackground(false);
        Service_control_96->setMaximumSize(QSize(16777215, 16777215));
        Service_control_96->setLineWidth(1);
        Service_control_96->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_96->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_control_96->sizePolicy().hasHeightForWidth());
        Service_control_96->setSizePolicy(sizePolicy);
        Service_control_96->setMidLineWidth(0);
        Service_control_96->setFont(font);
        Service_control_97 = new QVBoxLayout(Service_control_96);
        Service_control_97->setSpacing(12);
        Service_control_97->setObjectName("Service_control_97");
        Service_control_97->setContentsMargins(30, 30, 30, 30);
        spacerItem4 = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        Service_control_97->addItem(spacerItem4);

        Service_control_98 = new QLabel(Service_control_96);
        Service_control_98->setObjectName("Service_control_98");
        Service_control_98->setMinimumSize(QSize(0, 0));
        Service_control_98->setAutoFillBackground(false);
        Service_control_98->setMaximumSize(QSize(16777215, 16777215));
        Service_control_98->setLineWidth(1);
        Service_control_98->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_98->setWordWrap(false);
        Service_control_98->setGeometry(QRect(31, 136, 578, 86));
        sizePolicy.setHeightForWidth(Service_control_98->sizePolicy().hasHeightForWidth());
        Service_control_98->setSizePolicy(sizePolicy);
        Service_control_98->setMidLineWidth(0);
        Service_control_98->setFont(font);
        Service_control_98->setPixmap(QPixmap(QString::fromUtf8(":/designer/Service_control_98.png")));

        Service_control_97->addWidget(Service_control_98);

        Service_m_emptyTitle = new QLabel(Service_control_96);
        Service_m_emptyTitle->setObjectName("Service_m_emptyTitle");
        Service_m_emptyTitle->setMinimumSize(QSize(0, 0));
        Service_m_emptyTitle->setAutoFillBackground(false);
        Service_m_emptyTitle->setMaximumSize(QSize(16777215, 16777215));
        Service_m_emptyTitle->setLineWidth(1);
        Service_m_emptyTitle->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_emptyTitle->setWordWrap(true);
        Service_m_emptyTitle->setGeometry(QRect(31, 234, 578, 20));
        sizePolicy.setHeightForWidth(Service_m_emptyTitle->sizePolicy().hasHeightForWidth());
        Service_m_emptyTitle->setSizePolicy(sizePolicy);
        Service_m_emptyTitle->setMidLineWidth(0);
        Service_m_emptyTitle->setFont(font2);

        Service_control_97->addWidget(Service_m_emptyTitle);

        Service_m_emptyText = new QLabel(Service_control_96);
        Service_m_emptyText->setObjectName("Service_m_emptyText");
        Service_m_emptyText->setMinimumSize(QSize(0, 0));
        Service_m_emptyText->setAutoFillBackground(false);
        Service_m_emptyText->setMaximumSize(QSize(16777215, 16777215));
        Service_m_emptyText->setLineWidth(1);
        Service_m_emptyText->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_emptyText->setWordWrap(true);
        Service_m_emptyText->setGeometry(QRect(31, 266, 578, 32));
        sizePolicy.setHeightForWidth(Service_m_emptyText->sizePolicy().hasHeightForWidth());
        Service_m_emptyText->setSizePolicy(sizePolicy);
        Service_m_emptyText->setMidLineWidth(0);
        Service_m_emptyText->setFont(font);

        Service_control_97->addWidget(Service_m_emptyText);

        serviceFirstAdd = new QPushButton(Service_control_96);
        serviceFirstAdd->setObjectName("serviceFirstAdd");
        serviceFirstAdd->setMinimumSize(QSize(0, 0));
        serviceFirstAdd->setChecked(false);
        serviceFirstAdd->setIconSize(QSize(17, 17));
        serviceFirstAdd->setAutoFillBackground(false);
        serviceFirstAdd->setAutoDefault(false);
        serviceFirstAdd->setMaximumSize(QSize(16777215, 16777215));
        serviceFirstAdd->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        serviceFirstAdd->setFlat(false);
        serviceFirstAdd->setCheckable(false);
        serviceFirstAdd->setGeometry(QRect(247, 310, 146, 35));
        sizePolicy1.setHeightForWidth(serviceFirstAdd->sizePolicy().hasHeightForWidth());
        serviceFirstAdd->setSizePolicy(sizePolicy1);
        serviceFirstAdd->setFont(font2);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/designer/serviceFirstAdd.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        serviceFirstAdd->setIcon(icon5);

        Service_control_97->addWidget(serviceFirstAdd, 0, |Qt::AlignmentFlag::AlignVCenter);

        spacerItem5 = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        Service_control_97->addItem(spacerItem5);

        Service_m_views->addWidget(Service_control_96);

        Service_control_10->addWidget(Service_m_views);

        Service_control_99 = new QWidget(Service_m_listPage);
        Service_control_99->setObjectName("Service_control_99");
        Service_control_99->setAutoFillBackground(false);
        Service_control_99->setMinimumSize(QSize(288, 0));
        Service_control_99->setMaximumSize(QSize(288, 16777215));
        Service_control_99->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_99->setGeometry(QRect(0, 0, 288, 480));
        sizePolicy.setHeightForWidth(Service_control_99->sizePolicy().hasHeightForWidth());
        Service_control_99->setSizePolicy(sizePolicy);
        Service_control_99->setFont(font);
        Service_control_100 = new QVBoxLayout(Service_control_99);
        Service_control_100->setSpacing(14);
        Service_control_100->setObjectName("Service_control_100");
        Service_control_100->setContentsMargins(0, 0, 3, 0);
        serviceDetailPanel = new QFrame(Service_control_99);
        serviceDetailPanel->setObjectName("serviceDetailPanel");
        serviceDetailPanel->setMinimumSize(QSize(0, 0));
        serviceDetailPanel->setAutoFillBackground(false);
        serviceDetailPanel->setMaximumSize(QSize(16777215, 16777215));
        serviceDetailPanel->setLineWidth(1);
        serviceDetailPanel->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        serviceDetailPanel->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(serviceDetailPanel->sizePolicy().hasHeightForWidth());
        serviceDetailPanel->setSizePolicy(sizePolicy);
        serviceDetailPanel->setMidLineWidth(0);
        serviceDetailPanel->setFont(font);
        Service_control_101 = new QVBoxLayout(serviceDetailPanel);
        Service_control_101->setSpacing(8);
        Service_control_101->setObjectName("Service_control_101");
        Service_control_101->setContentsMargins(18, 17, 18, 17);
        Service_control_102 = new QLabel(serviceDetailPanel);
        Service_control_102->setObjectName("Service_control_102");
        Service_control_102->setMinimumSize(QSize(0, 0));
        Service_control_102->setAutoFillBackground(false);
        Service_control_102->setMaximumSize(QSize(16777215, 16777215));
        Service_control_102->setLineWidth(1);
        Service_control_102->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_102->setWordWrap(false);
        Service_control_102->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_control_102->sizePolicy().hasHeightForWidth());
        Service_control_102->setSizePolicy(sizePolicy);
        Service_control_102->setMidLineWidth(0);
        Service_control_102->setFont(font1);

        Service_control_101->addWidget(Service_control_102);

        Service_m_detailName = new QLabel(serviceDetailPanel);
        Service_m_detailName->setObjectName("Service_m_detailName");
        Service_m_detailName->setMinimumSize(QSize(0, 0));
        Service_m_detailName->setAutoFillBackground(false);
        Service_m_detailName->setMaximumSize(QSize(16777215, 16777215));
        Service_m_detailName->setLineWidth(1);
        Service_m_detailName->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_detailName->setWordWrap(true);
        Service_m_detailName->setGeometry(QRect(0, 0, 640, 480));
        Service_m_detailName->setStyleSheet(QString::fromUtf8("color:#306C7B;font-size:20px;font-weight:650;"));
        sizePolicy.setHeightForWidth(Service_m_detailName->sizePolicy().hasHeightForWidth());
        Service_m_detailName->setSizePolicy(sizePolicy);
        Service_m_detailName->setMidLineWidth(0);
        Service_m_detailName->setFont(font);

        Service_control_101->addWidget(Service_m_detailName);

        Service_m_detailReference = new QLabel(serviceDetailPanel);
        Service_m_detailReference->setObjectName("Service_m_detailReference");
        Service_m_detailReference->setMinimumSize(QSize(0, 0));
        Service_m_detailReference->setAutoFillBackground(false);
        Service_m_detailReference->setMaximumSize(QSize(16777215, 16777215));
        Service_m_detailReference->setLineWidth(1);
        Service_m_detailReference->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_detailReference->setWordWrap(false);
        Service_m_detailReference->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_m_detailReference->sizePolicy().hasHeightForWidth());
        Service_m_detailReference->setSizePolicy(sizePolicy);
        Service_m_detailReference->setMidLineWidth(0);
        Service_m_detailReference->setFont(font);

        Service_control_101->addWidget(Service_m_detailReference);

        Service_m_detailContent = new QWidget(serviceDetailPanel);
        Service_m_detailContent->setObjectName("Service_m_detailContent");
        Service_m_detailContent->setAutoFillBackground(false);
        Service_m_detailContent->setMinimumSize(QSize(0, 0));
        Service_m_detailContent->setMaximumSize(QSize(16777215, 16777215));
        Service_m_detailContent->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_detailContent->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_m_detailContent->sizePolicy().hasHeightForWidth());
        Service_m_detailContent->setSizePolicy(sizePolicy);
        Service_m_detailContent->setFont(font);
        Service_control_103 = new QVBoxLayout(Service_m_detailContent);
        Service_control_103->setSpacing(8);
        Service_control_103->setObjectName("Service_control_103");
        Service_control_103->setContentsMargins(0, 0, 0, 0);
        Service_m_statusLayout = new QVBoxLayout();
        Service_m_statusLayout->setSpacing(8);
        Service_m_statusLayout->setObjectName("Service_m_statusLayout");
        Service_m_statusLayout->setContentsMargins(0, 0, 0, 0);

        Service_control_103->addLayout(Service_m_statusLayout);

        Service_control_105 = new QFrame(Service_m_detailContent);
        Service_control_105->setObjectName("Service_control_105");
        Service_control_105->setMinimumSize(QSize(0, 1));
        Service_control_105->setAutoFillBackground(false);
        Service_control_105->setMaximumSize(QSize(16777215, 1));
        Service_control_105->setLineWidth(1);
        Service_control_105->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_105->setGeometry(QRect(0, 0, 640, 1));
        Service_control_105->setStyleSheet(QString::fromUtf8("background:#E0EDF1;border:none;"));
        sizePolicy.setHeightForWidth(Service_control_105->sizePolicy().hasHeightForWidth());
        Service_control_105->setSizePolicy(sizePolicy);
        Service_control_105->setMidLineWidth(0);
        Service_control_105->setFont(font);

        Service_control_103->addWidget(Service_control_105);

        serviceInfoScroll = new QScrollArea(Service_m_detailContent);
        serviceInfoScroll->setObjectName("serviceInfoScroll");
        serviceInfoScroll->setMinimumSize(QSize(0, 40));
        serviceInfoScroll->setWidgetResizable(true);
        serviceInfoScroll->setAutoFillBackground(false);
        serviceInfoScroll->setMaximumSize(QSize(16777215, 16777215));
        serviceInfoScroll->setLineWidth(1);
        serviceInfoScroll->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CursorShape::ArrowCursor)));
        serviceInfoScroll->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy4.setHeightForWidth(serviceInfoScroll->sizePolicy().hasHeightForWidth());
        serviceInfoScroll->setSizePolicy(sizePolicy4);
        serviceInfoScroll->setMidLineWidth(0);
        serviceInfoScroll->setFont(font);
        Service_control_107 = new QWidget();
        Service_control_107->setObjectName("Service_control_107");
        Service_control_107->setAutoFillBackground(false);
        Service_control_107->setMinimumSize(QSize(0, 0));
        Service_control_107->setMaximumSize(QSize(16777215, 16777215));
        Service_control_107->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_107->setGeometry(QRect(0, 0, 638, 478));
        Service_control_107->setStyleSheet(QString::fromUtf8("background: white;"));
        sizePolicy.setHeightForWidth(Service_control_107->sizePolicy().hasHeightForWidth());
        Service_control_107->setSizePolicy(sizePolicy);
        Service_control_107->setFont(font);
        Service_control_108 = new QVBoxLayout(Service_control_107);
        Service_control_108->setSpacing(12);
        Service_control_108->setObjectName("Service_control_108");
        Service_control_108->setContentsMargins(0, 7, 4, 7);
        Service_control_109 = new QGridLayout();
        Service_control_109->setSpacing(10);
        Service_control_109->setObjectName("Service_control_109");
        Service_control_109->setContentsMargins(0, 0, 0, 0);
        Service_control_110 = new QLabel(Service_control_107);
        Service_control_110->setObjectName("Service_control_110");
        Service_control_110->setMinimumSize(QSize(0, 0));
        Service_control_110->setAutoFillBackground(false);
        Service_control_110->setMaximumSize(QSize(16777215, 16777215));
        Service_control_110->setLineWidth(1);
        Service_control_110->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_110->setWordWrap(false);
        Service_control_110->setGeometry(QRect(0, 7, 66, 16));
        sizePolicy.setHeightForWidth(Service_control_110->sizePolicy().hasHeightForWidth());
        Service_control_110->setSizePolicy(sizePolicy);
        Service_control_110->setMidLineWidth(0);
        Service_control_110->setFont(font);

        Service_control_109->addWidget(Service_control_110, 0, 0, 1, 1, Qt::AlignmentFlag::Qt::AlignmentFlag::AlignTop);

        Service_detailValue_0 = new QLabel(Service_control_107);
        Service_detailValue_0->setObjectName("Service_detailValue_0");
        Service_detailValue_0->setMinimumSize(QSize(0, 0));
        Service_detailValue_0->setAutoFillBackground(false);
        Service_detailValue_0->setMaximumSize(QSize(16777215, 16777215));
        Service_detailValue_0->setLineWidth(1);
        Service_detailValue_0->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_detailValue_0->setWordWrap(true);
        Service_detailValue_0->setGeometry(QRect(76, 7, 558, 16));
        Service_detailValue_0->setStyleSheet(QString::fromUtf8("font-size:12px;color:#44636F;"));
        sizePolicy.setHeightForWidth(Service_detailValue_0->sizePolicy().hasHeightForWidth());
        Service_detailValue_0->setSizePolicy(sizePolicy);
        Service_detailValue_0->setMidLineWidth(0);
        Service_detailValue_0->setFont(font);

        Service_control_109->addWidget(Service_detailValue_0, 0, 1, 1, 1);

        Service_control_111 = new QLabel(Service_control_107);
        Service_control_111->setObjectName("Service_control_111");
        Service_control_111->setMinimumSize(QSize(0, 0));
        Service_control_111->setAutoFillBackground(false);
        Service_control_111->setMaximumSize(QSize(16777215, 16777215));
        Service_control_111->setLineWidth(1);
        Service_control_111->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_111->setWordWrap(false);
        Service_control_111->setGeometry(QRect(0, 33, 66, 16));
        sizePolicy.setHeightForWidth(Service_control_111->sizePolicy().hasHeightForWidth());
        Service_control_111->setSizePolicy(sizePolicy);
        Service_control_111->setMidLineWidth(0);
        Service_control_111->setFont(font);

        Service_control_109->addWidget(Service_control_111, 1, 0, 1, 1, Qt::AlignmentFlag::Qt::AlignmentFlag::AlignTop);

        Service_detailValue_1 = new QLabel(Service_control_107);
        Service_detailValue_1->setObjectName("Service_detailValue_1");
        Service_detailValue_1->setMinimumSize(QSize(0, 0));
        Service_detailValue_1->setAutoFillBackground(false);
        Service_detailValue_1->setMaximumSize(QSize(16777215, 16777215));
        Service_detailValue_1->setLineWidth(1);
        Service_detailValue_1->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_detailValue_1->setWordWrap(true);
        Service_detailValue_1->setGeometry(QRect(76, 33, 558, 16));
        Service_detailValue_1->setStyleSheet(QString::fromUtf8("font-size:12px;color:#44636F;"));
        sizePolicy.setHeightForWidth(Service_detailValue_1->sizePolicy().hasHeightForWidth());
        Service_detailValue_1->setSizePolicy(sizePolicy);
        Service_detailValue_1->setMidLineWidth(0);
        Service_detailValue_1->setFont(font);

        Service_control_109->addWidget(Service_detailValue_1, 1, 1, 1, 1);

        Service_control_112 = new QLabel(Service_control_107);
        Service_control_112->setObjectName("Service_control_112");
        Service_control_112->setMinimumSize(QSize(0, 0));
        Service_control_112->setAutoFillBackground(false);
        Service_control_112->setMaximumSize(QSize(16777215, 16777215));
        Service_control_112->setLineWidth(1);
        Service_control_112->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_112->setWordWrap(false);
        Service_control_112->setGeometry(QRect(0, 59, 66, 16));
        sizePolicy.setHeightForWidth(Service_control_112->sizePolicy().hasHeightForWidth());
        Service_control_112->setSizePolicy(sizePolicy);
        Service_control_112->setMidLineWidth(0);
        Service_control_112->setFont(font);

        Service_control_109->addWidget(Service_control_112, 2, 0, 1, 1, Qt::AlignmentFlag::Qt::AlignmentFlag::AlignTop);

        Service_detailValue_2 = new QLabel(Service_control_107);
        Service_detailValue_2->setObjectName("Service_detailValue_2");
        Service_detailValue_2->setMinimumSize(QSize(0, 0));
        Service_detailValue_2->setAutoFillBackground(false);
        Service_detailValue_2->setMaximumSize(QSize(16777215, 16777215));
        Service_detailValue_2->setLineWidth(1);
        Service_detailValue_2->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_detailValue_2->setWordWrap(true);
        Service_detailValue_2->setGeometry(QRect(76, 59, 558, 16));
        Service_detailValue_2->setStyleSheet(QString::fromUtf8("font-size:12px;color:#44636F;"));
        sizePolicy.setHeightForWidth(Service_detailValue_2->sizePolicy().hasHeightForWidth());
        Service_detailValue_2->setSizePolicy(sizePolicy);
        Service_detailValue_2->setMidLineWidth(0);
        Service_detailValue_2->setFont(font);

        Service_control_109->addWidget(Service_detailValue_2, 2, 1, 1, 1);

        Service_control_113 = new QLabel(Service_control_107);
        Service_control_113->setObjectName("Service_control_113");
        Service_control_113->setMinimumSize(QSize(0, 0));
        Service_control_113->setAutoFillBackground(false);
        Service_control_113->setMaximumSize(QSize(16777215, 16777215));
        Service_control_113->setLineWidth(1);
        Service_control_113->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_113->setWordWrap(false);
        Service_control_113->setGeometry(QRect(0, 85, 66, 16));
        sizePolicy.setHeightForWidth(Service_control_113->sizePolicy().hasHeightForWidth());
        Service_control_113->setSizePolicy(sizePolicy);
        Service_control_113->setMidLineWidth(0);
        Service_control_113->setFont(font);

        Service_control_109->addWidget(Service_control_113, 3, 0, 1, 1, Qt::AlignmentFlag::Qt::AlignmentFlag::AlignTop);

        Service_detailValue_3 = new QLabel(Service_control_107);
        Service_detailValue_3->setObjectName("Service_detailValue_3");
        Service_detailValue_3->setMinimumSize(QSize(0, 0));
        Service_detailValue_3->setAutoFillBackground(false);
        Service_detailValue_3->setMaximumSize(QSize(16777215, 16777215));
        Service_detailValue_3->setLineWidth(1);
        Service_detailValue_3->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_detailValue_3->setWordWrap(true);
        Service_detailValue_3->setGeometry(QRect(76, 85, 558, 16));
        Service_detailValue_3->setStyleSheet(QString::fromUtf8("font-size:12px;color:#44636F;"));
        sizePolicy.setHeightForWidth(Service_detailValue_3->sizePolicy().hasHeightForWidth());
        Service_detailValue_3->setSizePolicy(sizePolicy);
        Service_detailValue_3->setMidLineWidth(0);
        Service_detailValue_3->setFont(font);

        Service_control_109->addWidget(Service_detailValue_3, 3, 1, 1, 1);

        Service_control_114 = new QLabel(Service_control_107);
        Service_control_114->setObjectName("Service_control_114");
        Service_control_114->setMinimumSize(QSize(0, 0));
        Service_control_114->setAutoFillBackground(false);
        Service_control_114->setMaximumSize(QSize(16777215, 16777215));
        Service_control_114->setLineWidth(1);
        Service_control_114->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_114->setWordWrap(false);
        Service_control_114->setGeometry(QRect(0, 111, 66, 16));
        sizePolicy.setHeightForWidth(Service_control_114->sizePolicy().hasHeightForWidth());
        Service_control_114->setSizePolicy(sizePolicy);
        Service_control_114->setMidLineWidth(0);
        Service_control_114->setFont(font);

        Service_control_109->addWidget(Service_control_114, 4, 0, 1, 1, Qt::AlignmentFlag::Qt::AlignmentFlag::AlignTop);

        Service_detailValue_4 = new QLabel(Service_control_107);
        Service_detailValue_4->setObjectName("Service_detailValue_4");
        Service_detailValue_4->setMinimumSize(QSize(0, 0));
        Service_detailValue_4->setAutoFillBackground(false);
        Service_detailValue_4->setMaximumSize(QSize(16777215, 16777215));
        Service_detailValue_4->setLineWidth(1);
        Service_detailValue_4->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_detailValue_4->setWordWrap(true);
        Service_detailValue_4->setGeometry(QRect(76, 111, 558, 16));
        Service_detailValue_4->setStyleSheet(QString::fromUtf8("font-size:12px;color:#44636F;"));
        sizePolicy.setHeightForWidth(Service_detailValue_4->sizePolicy().hasHeightForWidth());
        Service_detailValue_4->setSizePolicy(sizePolicy);
        Service_detailValue_4->setMidLineWidth(0);
        Service_detailValue_4->setFont(font);

        Service_control_109->addWidget(Service_detailValue_4, 4, 1, 1, 1);

        Service_control_115 = new QLabel(Service_control_107);
        Service_control_115->setObjectName("Service_control_115");
        Service_control_115->setMinimumSize(QSize(0, 0));
        Service_control_115->setAutoFillBackground(false);
        Service_control_115->setMaximumSize(QSize(16777215, 16777215));
        Service_control_115->setLineWidth(1);
        Service_control_115->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_115->setWordWrap(false);
        Service_control_115->setGeometry(QRect(0, 137, 66, 16));
        sizePolicy.setHeightForWidth(Service_control_115->sizePolicy().hasHeightForWidth());
        Service_control_115->setSizePolicy(sizePolicy);
        Service_control_115->setMidLineWidth(0);
        Service_control_115->setFont(font);

        Service_control_109->addWidget(Service_control_115, 5, 0, 1, 1, Qt::AlignmentFlag::Qt::AlignmentFlag::AlignTop);

        Service_detailValue_5 = new QLabel(Service_control_107);
        Service_detailValue_5->setObjectName("Service_detailValue_5");
        Service_detailValue_5->setMinimumSize(QSize(0, 0));
        Service_detailValue_5->setAutoFillBackground(false);
        Service_detailValue_5->setMaximumSize(QSize(16777215, 16777215));
        Service_detailValue_5->setLineWidth(1);
        Service_detailValue_5->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_detailValue_5->setWordWrap(true);
        Service_detailValue_5->setGeometry(QRect(76, 137, 558, 16));
        Service_detailValue_5->setStyleSheet(QString::fromUtf8("font-size:12px;color:#44636F;"));
        sizePolicy.setHeightForWidth(Service_detailValue_5->sizePolicy().hasHeightForWidth());
        Service_detailValue_5->setSizePolicy(sizePolicy);
        Service_detailValue_5->setMidLineWidth(0);
        Service_detailValue_5->setFont(font);

        Service_control_109->addWidget(Service_detailValue_5, 5, 1, 1, 1);

        Service_control_116 = new QLabel(Service_control_107);
        Service_control_116->setObjectName("Service_control_116");
        Service_control_116->setMinimumSize(QSize(0, 0));
        Service_control_116->setAutoFillBackground(false);
        Service_control_116->setMaximumSize(QSize(16777215, 16777215));
        Service_control_116->setLineWidth(1);
        Service_control_116->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_116->setWordWrap(false);
        Service_control_116->setGeometry(QRect(0, 163, 66, 16));
        sizePolicy.setHeightForWidth(Service_control_116->sizePolicy().hasHeightForWidth());
        Service_control_116->setSizePolicy(sizePolicy);
        Service_control_116->setMidLineWidth(0);
        Service_control_116->setFont(font);

        Service_control_109->addWidget(Service_control_116, 6, 0, 1, 1, Qt::AlignmentFlag::Qt::AlignmentFlag::AlignTop);

        Service_detailValue_6 = new QLabel(Service_control_107);
        Service_detailValue_6->setObjectName("Service_detailValue_6");
        Service_detailValue_6->setMinimumSize(QSize(0, 0));
        Service_detailValue_6->setAutoFillBackground(false);
        Service_detailValue_6->setMaximumSize(QSize(16777215, 16777215));
        Service_detailValue_6->setLineWidth(1);
        Service_detailValue_6->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_detailValue_6->setWordWrap(true);
        Service_detailValue_6->setGeometry(QRect(76, 163, 558, 16));
        Service_detailValue_6->setStyleSheet(QString::fromUtf8("font-size:12px;color:#44636F;"));
        sizePolicy.setHeightForWidth(Service_detailValue_6->sizePolicy().hasHeightForWidth());
        Service_detailValue_6->setSizePolicy(sizePolicy);
        Service_detailValue_6->setMidLineWidth(0);
        Service_detailValue_6->setFont(font);

        Service_control_109->addWidget(Service_detailValue_6, 6, 1, 1, 1);

        Service_control_117 = new QLabel(Service_control_107);
        Service_control_117->setObjectName("Service_control_117");
        Service_control_117->setMinimumSize(QSize(0, 0));
        Service_control_117->setAutoFillBackground(false);
        Service_control_117->setMaximumSize(QSize(16777215, 16777215));
        Service_control_117->setLineWidth(1);
        Service_control_117->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_117->setWordWrap(false);
        Service_control_117->setGeometry(QRect(0, 189, 66, 16));
        sizePolicy.setHeightForWidth(Service_control_117->sizePolicy().hasHeightForWidth());
        Service_control_117->setSizePolicy(sizePolicy);
        Service_control_117->setMidLineWidth(0);
        Service_control_117->setFont(font);

        Service_control_109->addWidget(Service_control_117, 7, 0, 1, 1, Qt::AlignmentFlag::Qt::AlignmentFlag::AlignTop);

        Service_detailValue_7 = new QLabel(Service_control_107);
        Service_detailValue_7->setObjectName("Service_detailValue_7");
        Service_detailValue_7->setMinimumSize(QSize(0, 0));
        Service_detailValue_7->setAutoFillBackground(false);
        Service_detailValue_7->setMaximumSize(QSize(16777215, 16777215));
        Service_detailValue_7->setLineWidth(1);
        Service_detailValue_7->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_detailValue_7->setWordWrap(true);
        Service_detailValue_7->setGeometry(QRect(76, 189, 558, 16));
        Service_detailValue_7->setStyleSheet(QString::fromUtf8("font-size:12px;color:#44636F;"));
        sizePolicy.setHeightForWidth(Service_detailValue_7->sizePolicy().hasHeightForWidth());
        Service_detailValue_7->setSizePolicy(sizePolicy);
        Service_detailValue_7->setMidLineWidth(0);
        Service_detailValue_7->setFont(font);

        Service_control_109->addWidget(Service_detailValue_7, 7, 1, 1, 1);

        Service_control_109->setColumnStretch(1, 1);

        Service_control_108->addLayout(Service_control_109);

        Service_m_detailNotes = new QLabel(Service_control_107);
        Service_m_detailNotes->setObjectName("Service_m_detailNotes");
        Service_m_detailNotes->setMinimumSize(QSize(0, 0));
        Service_m_detailNotes->setAutoFillBackground(false);
        Service_m_detailNotes->setMaximumSize(QSize(16777215, 72));
        Service_m_detailNotes->setLineWidth(1);
        Service_m_detailNotes->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_detailNotes->setWordWrap(true);
        Service_m_detailNotes->setGeometry(QRect(0, 217, 634, 16));
        sizePolicy.setHeightForWidth(Service_m_detailNotes->sizePolicy().hasHeightForWidth());
        Service_m_detailNotes->setSizePolicy(sizePolicy);
        Service_m_detailNotes->setMidLineWidth(0);
        Service_m_detailNotes->setFont(font);

        Service_control_108->addWidget(Service_m_detailNotes);

        spacerItem6 = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        Service_control_108->addItem(spacerItem6);

        serviceInfoScroll->setWidget(Service_control_107);

        Service_control_103->addWidget(serviceInfoScroll);

        Service_control_104 = new QHBoxLayout();
        Service_control_104->setSpacing(7);
        Service_control_104->setObjectName("Service_control_104");
        Service_control_104->setContentsMargins(0, 0, 0, 0);
        Service_m_view = new QPushButton(Service_m_detailContent);
        Service_m_view->setObjectName("Service_m_view");
        Service_m_view->setMinimumSize(QSize(0, 0));
        Service_m_view->setChecked(false);
        Service_m_view->setIconSize(QSize(17, 17));
        Service_m_view->setAutoFillBackground(false);
        Service_m_view->setAutoDefault(false);
        Service_m_view->setMaximumSize(QSize(16777215, 16777215));
        Service_m_view->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Service_m_view->setFlat(false);
        Service_m_view->setCheckable(false);
        Service_m_view->setGeometry(QRect(0, 0, 640, 480));
        Service_m_view->setStyleSheet(QString::fromUtf8("padding:6px 8px;"));
        sizePolicy1.setHeightForWidth(Service_m_view->sizePolicy().hasHeightForWidth());
        Service_m_view->setSizePolicy(sizePolicy1);
        Service_m_view->setFont(font2);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/designer/Service_m_view.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Service_m_view->setIcon(icon6);

        Service_control_104->addWidget(Service_m_view);

        Service_m_invoice = new QPushButton(Service_m_detailContent);
        Service_m_invoice->setObjectName("Service_m_invoice");
        Service_m_invoice->setMinimumSize(QSize(0, 0));
        Service_m_invoice->setChecked(false);
        Service_m_invoice->setIconSize(QSize(17, 17));
        Service_m_invoice->setAutoFillBackground(false);
        Service_m_invoice->setAutoDefault(false);
        Service_m_invoice->setMaximumSize(QSize(16777215, 16777215));
        Service_m_invoice->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Service_m_invoice->setFlat(false);
        Service_m_invoice->setCheckable(false);
        Service_m_invoice->setGeometry(QRect(2, 2, 115, 35));
        Service_m_invoice->setStyleSheet(QString::fromUtf8("padding:6px 8px;"));
        sizePolicy1.setHeightForWidth(Service_m_invoice->sizePolicy().hasHeightForWidth());
        Service_m_invoice->setSizePolicy(sizePolicy1);
        Service_m_invoice->setFont(font2);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/designer/Service_m_invoice.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Service_m_invoice->setIcon(icon7);

        Service_control_104->addWidget(Service_m_invoice);


        Service_control_103->addLayout(Service_control_104);

        Service_m_edit = new QPushButton(Service_m_detailContent);
        Service_m_edit->setObjectName("Service_m_edit");
        Service_m_edit->setMinimumSize(QSize(0, 40));
        Service_m_edit->setChecked(false);
        Service_m_edit->setIconSize(QSize(17, 17));
        Service_m_edit->setAutoFillBackground(false);
        Service_m_edit->setAutoDefault(false);
        Service_m_edit->setMaximumSize(QSize(16777215, 16777215));
        Service_m_edit->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Service_m_edit->setFlat(false);
        Service_m_edit->setCheckable(false);
        Service_m_edit->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Service_m_edit->sizePolicy().hasHeightForWidth());
        Service_m_edit->setSizePolicy(sizePolicy1);
        Service_m_edit->setFont(font1);
        Service_m_edit->setProperty("detailAction", QVariant(true));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/designer/Service_m_edit.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Service_m_edit->setIcon(icon8);

        Service_control_103->addWidget(Service_m_edit);

        Service_m_advance = new QPushButton(Service_m_detailContent);
        Service_m_advance->setObjectName("Service_m_advance");
        Service_m_advance->setMinimumSize(QSize(0, 40));
        Service_m_advance->setChecked(false);
        Service_m_advance->setIconSize(QSize(17, 17));
        Service_m_advance->setAutoFillBackground(false);
        Service_m_advance->setAutoDefault(false);
        Service_m_advance->setMaximumSize(QSize(16777215, 16777215));
        Service_m_advance->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Service_m_advance->setFlat(false);
        Service_m_advance->setCheckable(false);
        Service_m_advance->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Service_m_advance->sizePolicy().hasHeightForWidth());
        Service_m_advance->setSizePolicy(sizePolicy1);
        Service_m_advance->setFont(font1);
        Service_m_advance->setProperty("detailAction", QVariant(true));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/designer/Service_m_advance.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Service_m_advance->setIcon(icon9);

        Service_control_103->addWidget(Service_m_advance);

        Service_m_delete = new QPushButton(Service_m_detailContent);
        Service_m_delete->setObjectName("Service_m_delete");
        Service_m_delete->setMinimumSize(QSize(0, 40));
        Service_m_delete->setChecked(false);
        Service_m_delete->setIconSize(QSize(17, 17));
        Service_m_delete->setAutoFillBackground(false);
        Service_m_delete->setAutoDefault(false);
        Service_m_delete->setMaximumSize(QSize(16777215, 16777215));
        Service_m_delete->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Service_m_delete->setFlat(false);
        Service_m_delete->setCheckable(false);
        Service_m_delete->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Service_m_delete->sizePolicy().hasHeightForWidth());
        Service_m_delete->setSizePolicy(sizePolicy1);
        Service_m_delete->setFont(font1);
        Service_m_delete->setProperty("detailAction", QVariant(true));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/designer/Service_m_delete.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Service_m_delete->setIcon(icon10);

        Service_control_103->addWidget(Service_m_delete);

        Service_control_103->setStretch(2, 1);

        Service_control_101->addWidget(Service_m_detailContent);

        Service_m_emptyDetailSpacer = new QWidget(serviceDetailPanel);
        Service_m_emptyDetailSpacer->setObjectName("Service_m_emptyDetailSpacer");
        Service_m_emptyDetailSpacer->setAutoFillBackground(false);
        Service_m_emptyDetailSpacer->setMinimumSize(QSize(0, 0));
        Service_m_emptyDetailSpacer->setMaximumSize(QSize(16777215, 16777215));
        Service_m_emptyDetailSpacer->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_emptyDetailSpacer->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_m_emptyDetailSpacer->sizePolicy().hasHeightForWidth());
        Service_m_emptyDetailSpacer->setSizePolicy(sizePolicy);
        Service_m_emptyDetailSpacer->setFont(font);

        Service_control_101->addWidget(Service_m_emptyDetailSpacer);

        Service_control_101->setStretch(3, 1);
        Service_control_101->setStretch(4, 1);

        Service_control_100->addWidget(serviceDetailPanel);

        Service_m_attention = new QLabel(Service_control_99);
        Service_m_attention->setObjectName("Service_m_attention");
        Service_m_attention->setMinimumSize(QSize(0, 0));
        Service_m_attention->setAutoFillBackground(false);
        Service_m_attention->setMaximumSize(QSize(16777215, 16777215));
        Service_m_attention->setLineWidth(1);
        Service_m_attention->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_m_attention->setWordWrap(true);
        Service_m_attention->setGeometry(QRect(0, 0, 640, 480));
        Service_m_attention->setStyleSheet(QString::fromUtf8("color:#947039;background:#FFF0D9;border-radius:8px;padding:12px;font-size:11px;"));
        sizePolicy.setHeightForWidth(Service_m_attention->sizePolicy().hasHeightForWidth());
        Service_m_attention->setSizePolicy(sizePolicy);
        Service_m_attention->setMidLineWidth(0);
        Service_m_attention->setFont(font);
        Service_m_attention->setVisible(false);

        Service_control_100->addWidget(Service_m_attention);

        Service_control_100->setStretch(0, 1);

        Service_control_10->addWidget(Service_control_99);

        Service_control_10->setStretch(0, 1);

        Service_control_3->addLayout(Service_control_10);

        Service_control_11 = new QHBoxLayout();
        Service_control_11->setSpacing(12);
        Service_control_11->setObjectName("Service_control_11");
        Service_control_11->setContentsMargins(0, 0, 0, 0);
        Service_control_124 = new QLabel(Service_m_listPage);
        Service_control_124->setObjectName("Service_control_124");
        Service_control_124->setMinimumSize(QSize(0, 0));
        Service_control_124->setAutoFillBackground(false);
        Service_control_124->setMaximumSize(QSize(16777215, 16777215));
        Service_control_124->setLineWidth(1);
        Service_control_124->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_124->setWordWrap(false);
        Service_control_124->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_control_124->sizePolicy().hasHeightForWidth());
        Service_control_124->setSizePolicy(sizePolicy);
        Service_control_124->setMidLineWidth(0);
        Service_control_124->setFont(font);

        Service_control_11->addWidget(Service_control_124);

        spacerItem7 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Service_control_11->addItem(spacerItem7);

        Service_control_125 = new QLabel(Service_m_listPage);
        Service_control_125->setObjectName("Service_control_125");
        Service_control_125->setMinimumSize(QSize(0, 0));
        Service_control_125->setAutoFillBackground(false);
        Service_control_125->setMaximumSize(QSize(16777215, 16777215));
        Service_control_125->setLineWidth(1);
        Service_control_125->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Service_control_125->setWordWrap(false);
        Service_control_125->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Service_control_125->sizePolicy().hasHeightForWidth());
        Service_control_125->setSizePolicy(sizePolicy);
        Service_control_125->setMidLineWidth(0);
        Service_control_125->setFont(font);

        Service_control_11->addWidget(Service_control_125);


        Service_control_3->addLayout(Service_control_11);

        Service_control_3->setStretch(5, 1);
        Service_m_pages->addWidget(Service_m_listPage);

        Service_control_1->addWidget(Service_m_pages);

        pageStack->addWidget(ServicePage);
        StockPage = new QWidget();
        StockPage->setObjectName("StockPage");
        StockPage->setAutoFillBackground(true);
        StockPage->setMinimumSize(QSize(0, 0));
        StockPage->setMaximumSize(QSize(16777215, 16777215));
        StockPage->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        StockPage->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(StockPage->sizePolicy().hasHeightForWidth());
        StockPage->setSizePolicy(sizePolicy);
        StockPage->setFont(font3);
        Stock_control_1 = new QVBoxLayout(StockPage);
        Stock_control_1->setSpacing(14);
        Stock_control_1->setObjectName("Stock_control_1");
        Stock_control_1->setContentsMargins(24, 18, 24, 12);
        Stock_control_2 = new QHBoxLayout();
        Stock_control_2->setSpacing(14);
        Stock_control_2->setObjectName("Stock_control_2");
        Stock_control_2->setContentsMargins(0, 0, 0, 0);
        Stock_control_3 = new QVBoxLayout();
        Stock_control_3->setSpacing(2);
        Stock_control_3->setObjectName("Stock_control_3");
        Stock_control_3->setContentsMargins(0, 0, 0, 0);
        Stock_control_7 = new QLabel(StockPage);
        Stock_control_7->setObjectName("Stock_control_7");
        Stock_control_7->setMinimumSize(QSize(0, 0));
        Stock_control_7->setAutoFillBackground(false);
        Stock_control_7->setMaximumSize(QSize(16777215, 16777215));
        Stock_control_7->setLineWidth(1);
        Stock_control_7->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_control_7->setWordWrap(false);
        Stock_control_7->setGeometry(QRect(0, 0, 640, 480));
        Stock_control_7->setStyleSheet(QString::fromUtf8("font-size: 23px; font-weight: 800; color: #225A64; background: transparent;"));
        sizePolicy.setHeightForWidth(Stock_control_7->sizePolicy().hasHeightForWidth());
        Stock_control_7->setSizePolicy(sizePolicy);
        Stock_control_7->setMidLineWidth(0);
        QFont font4;
        font4.setFamilies({QString::fromUtf8("Segoe UI")});
        font4.setWeight(QFont::ExtraBold);
        Stock_control_7->setFont(font4);

        Stock_control_3->addWidget(Stock_control_7);

        Stock_control_8 = new QLabel(StockPage);
        Stock_control_8->setObjectName("Stock_control_8");
        Stock_control_8->setMinimumSize(QSize(0, 0));
        Stock_control_8->setAutoFillBackground(false);
        Stock_control_8->setMaximumSize(QSize(16777215, 16777215));
        Stock_control_8->setLineWidth(1);
        Stock_control_8->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_control_8->setWordWrap(false);
        Stock_control_8->setGeometry(QRect(0, 0, 640, 480));
        Stock_control_8->setStyleSheet(QString::fromUtf8("font-size: 11px; color: #71878D; background: transparent;"));
        sizePolicy.setHeightForWidth(Stock_control_8->sizePolicy().hasHeightForWidth());
        Stock_control_8->setSizePolicy(sizePolicy);
        Stock_control_8->setMidLineWidth(0);
        Stock_control_8->setFont(font);

        Stock_control_3->addWidget(Stock_control_8);


        Stock_control_2->addLayout(Stock_control_3);

        spacerItem8 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Stock_control_2->addItem(spacerItem8);

        stockMovementButton = new QPushButton(StockPage);
        stockMovementButton->setObjectName("stockMovementButton");
        stockMovementButton->setMinimumSize(QSize(0, 0));
        stockMovementButton->setChecked(false);
        stockMovementButton->setIconSize(QSize(17, 17));
        stockMovementButton->setAutoFillBackground(false);
        stockMovementButton->setAutoDefault(false);
        stockMovementButton->setMaximumSize(QSize(16777215, 16777215));
        stockMovementButton->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        stockMovementButton->setFlat(false);
        stockMovementButton->setCheckable(false);
        stockMovementButton->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(stockMovementButton->sizePolicy().hasHeightForWidth());
        stockMovementButton->setSizePolicy(sizePolicy1);
        stockMovementButton->setFont(font3);

        Stock_control_2->addWidget(stockMovementButton);

        stockExportButton = new QPushButton(StockPage);
        stockExportButton->setObjectName("stockExportButton");
        stockExportButton->setMinimumSize(QSize(0, 0));
        stockExportButton->setChecked(false);
        stockExportButton->setIconSize(QSize(17, 17));
        stockExportButton->setAutoFillBackground(false);
        stockExportButton->setAutoDefault(false);
        stockExportButton->setMaximumSize(QSize(16777215, 16777215));
        stockExportButton->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        stockExportButton->setFlat(false);
        stockExportButton->setCheckable(false);
        stockExportButton->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(stockExportButton->sizePolicy().hasHeightForWidth());
        stockExportButton->setSizePolicy(sizePolicy1);
        stockExportButton->setFont(font3);
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/designer/stockExportButton.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        stockExportButton->setIcon(icon11);

        Stock_control_2->addWidget(stockExportButton);

        Stock_control_2->setStretch(1, 1);

        Stock_control_1->addLayout(Stock_control_2);

        Stock_control_4 = new QHBoxLayout();
        Stock_control_4->setSpacing(14);
        Stock_control_4->setObjectName("Stock_control_4");
        Stock_control_4->setContentsMargins(0, 0, 0, 0);
        stockStatisticsButton = new QPushButton(StockPage);
        stockStatisticsButton->setObjectName("stockStatisticsButton");
        stockStatisticsButton->setMinimumSize(QSize(0, 0));
        stockStatisticsButton->setChecked(false);
        stockStatisticsButton->setIconSize(QSize(17, 17));
        stockStatisticsButton->setAutoFillBackground(false);
        stockStatisticsButton->setAutoDefault(false);
        stockStatisticsButton->setMaximumSize(QSize(16777215, 16777215));
        stockStatisticsButton->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        stockStatisticsButton->setFlat(false);
        stockStatisticsButton->setCheckable(false);
        stockStatisticsButton->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(stockStatisticsButton->sizePolicy().hasHeightForWidth());
        stockStatisticsButton->setSizePolicy(sizePolicy1);
        stockStatisticsButton->setFont(font3);

        Stock_control_4->addWidget(stockStatisticsButton);

        spacerItem9 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Stock_control_4->addItem(spacerItem9);

        stockEditButton = new QPushButton(StockPage);
        stockEditButton->setObjectName("stockEditButton");
        stockEditButton->setMinimumSize(QSize(0, 0));
        stockEditButton->setChecked(false);
        stockEditButton->setIconSize(QSize(17, 17));
        stockEditButton->setAutoFillBackground(false);
        stockEditButton->setAutoDefault(false);
        stockEditButton->setMaximumSize(QSize(16777215, 16777215));
        stockEditButton->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        stockEditButton->setFlat(false);
        stockEditButton->setCheckable(false);
        stockEditButton->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(stockEditButton->sizePolicy().hasHeightForWidth());
        stockEditButton->setSizePolicy(sizePolicy1);
        stockEditButton->setFont(font3);

        Stock_control_4->addWidget(stockEditButton);

        stockDeleteButton = new QPushButton(StockPage);
        stockDeleteButton->setObjectName("stockDeleteButton");
        stockDeleteButton->setMinimumSize(QSize(0, 0));
        stockDeleteButton->setChecked(false);
        stockDeleteButton->setIconSize(QSize(17, 17));
        stockDeleteButton->setAutoFillBackground(false);
        stockDeleteButton->setAutoDefault(false);
        stockDeleteButton->setMaximumSize(QSize(16777215, 16777215));
        stockDeleteButton->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        stockDeleteButton->setFlat(false);
        stockDeleteButton->setCheckable(false);
        stockDeleteButton->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(stockDeleteButton->sizePolicy().hasHeightForWidth());
        stockDeleteButton->setSizePolicy(sizePolicy1);
        stockDeleteButton->setFont(font3);

        Stock_control_4->addWidget(stockDeleteButton);


        Stock_control_1->addLayout(Stock_control_4);

        Stock_control_5 = new QHBoxLayout();
        Stock_control_5->setSpacing(10);
        Stock_control_5->setObjectName("Stock_control_5");
        Stock_control_5->setContentsMargins(0, 0, 0, 0);
        Stock_searchLineEdit = new QLineEdit(StockPage);
        Stock_searchLineEdit->setObjectName("Stock_searchLineEdit");
        Stock_searchLineEdit->setMinimumSize(QSize(0, 37));
        Stock_searchLineEdit->setAutoFillBackground(false);
        Stock_searchLineEdit->setMaxLength(32767);
        Stock_searchLineEdit->setMaximumSize(QSize(16777215, 16777215));
        Stock_searchLineEdit->setCursor(QCursor(Qt::CursorShape::IBeamCursor));
        Stock_searchLineEdit->setReadOnly(false);
        Stock_searchLineEdit->setClearButtonEnabled(true);
        Stock_searchLineEdit->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy2.setHeightForWidth(Stock_searchLineEdit->sizePolicy().hasHeightForWidth());
        Stock_searchLineEdit->setSizePolicy(sizePolicy2);
        Stock_searchLineEdit->setFont(font3);

        Stock_control_5->addWidget(Stock_searchLineEdit);

        Stock_categoryFilter = new QComboBox(StockPage);
        Stock_categoryFilter->addItem(QString());
        Stock_categoryFilter->addItem(QString());
        Stock_categoryFilter->addItem(QString());
        Stock_categoryFilter->addItem(QString());
        Stock_categoryFilter->addItem(QString());
        Stock_categoryFilter->addItem(QString());
        Stock_categoryFilter->addItem(QString());
        Stock_categoryFilter->setObjectName("Stock_categoryFilter");
        Stock_categoryFilter->setMinimumSize(QSize(150, 37));
        Stock_categoryFilter->setMinimumContentsLength(0);
        Stock_categoryFilter->setIconSize(QSize(16, 16));
        Stock_categoryFilter->setAutoFillBackground(false);
        Stock_categoryFilter->setMaximumSize(QSize(16777215, 16777215));
        Stock_categoryFilter->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_categoryFilter->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy3.setHeightForWidth(Stock_categoryFilter->sizePolicy().hasHeightForWidth());
        Stock_categoryFilter->setSizePolicy(sizePolicy3);
        Stock_categoryFilter->setEditable(false);
        Stock_categoryFilter->setFont(font3);

        Stock_control_5->addWidget(Stock_categoryFilter);

        Stock_sortFilter = new QComboBox(StockPage);
        Stock_sortFilter->addItem(QString());
        Stock_sortFilter->addItem(QString());
        Stock_sortFilter->addItem(QString());
        Stock_sortFilter->addItem(QString());
        Stock_sortFilter->addItem(QString());
        Stock_sortFilter->setObjectName("Stock_sortFilter");
        Stock_sortFilter->setMinimumSize(QSize(190, 37));
        Stock_sortFilter->setMinimumContentsLength(0);
        Stock_sortFilter->setIconSize(QSize(16, 16));
        Stock_sortFilter->setAutoFillBackground(false);
        Stock_sortFilter->setMaximumSize(QSize(16777215, 16777215));
        Stock_sortFilter->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_sortFilter->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy3.setHeightForWidth(Stock_sortFilter->sizePolicy().hasHeightForWidth());
        Stock_sortFilter->setSizePolicy(sizePolicy3);
        Stock_sortFilter->setEditable(false);
        Stock_sortFilter->setFont(font3);

        Stock_control_5->addWidget(Stock_sortFilter);

        Stock_stateFilter = new QComboBox(StockPage);
        Stock_stateFilter->addItem(QString());
        Stock_stateFilter->addItem(QString());
        Stock_stateFilter->addItem(QString());
        Stock_stateFilter->addItem(QString());
        Stock_stateFilter->addItem(QString());
        Stock_stateFilter->setObjectName("Stock_stateFilter");
        Stock_stateFilter->setMinimumSize(QSize(155, 37));
        Stock_stateFilter->setMinimumContentsLength(0);
        Stock_stateFilter->setIconSize(QSize(16, 16));
        Stock_stateFilter->setAutoFillBackground(false);
        Stock_stateFilter->setMaximumSize(QSize(16777215, 16777215));
        Stock_stateFilter->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_stateFilter->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy3.setHeightForWidth(Stock_stateFilter->sizePolicy().hasHeightForWidth());
        Stock_stateFilter->setSizePolicy(sizePolicy3);
        Stock_stateFilter->setEditable(false);
        Stock_stateFilter->setFont(font3);

        Stock_control_5->addWidget(Stock_stateFilter);

        Stock_control_5->setStretch(0, 1);

        Stock_control_1->addLayout(Stock_control_5);

        Stock_stockTable = new QTableView(StockPage);
        Stock_stockTable->setObjectName("Stock_stockTable");
        Stock_stockTable->setMinimumSize(QSize(0, 150));
        Stock_stockTable->setIconSize(QSize(-1, -1));
        Stock_stockTable->setAutoFillBackground(false);
        Stock_stockTable->setMaximumSize(QSize(16777215, 16777215));
        Stock_stockTable->setAlternatingRowColors(true);
        Stock_stockTable->setSortingEnabled(false);
        Stock_stockTable->setLineWidth(1);
        Stock_stockTable->setShowGrid(true);
        Stock_stockTable->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CursorShape::ArrowCursor)));
        Stock_stockTable->setWordWrap(true);
        Stock_stockTable->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy4.setHeightForWidth(Stock_stockTable->sizePolicy().hasHeightForWidth());
        Stock_stockTable->setSizePolicy(sizePolicy4);
        Stock_stockTable->setMidLineWidth(0);
        Stock_stockTable->setFont(font3);
        Stock_stockTable->horizontalHeader()->setVisible(false);
        Stock_stockTable->horizontalHeader()->setCascadingSectionResizes(false);
        Stock_stockTable->horizontalHeader()->setMinimumSectionSize(28);
        Stock_stockTable->horizontalHeader()->setDefaultSectionSize(100);
        Stock_stockTable->horizontalHeader()->setHighlightSections(true);
        Stock_stockTable->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
        Stock_stockTable->horizontalHeader()->setStretchLastSection(true);
        Stock_stockTable->verticalHeader()->setVisible(false);
        Stock_stockTable->verticalHeader()->setCascadingSectionResizes(false);
        Stock_stockTable->verticalHeader()->setMinimumSectionSize(21);
        Stock_stockTable->verticalHeader()->setDefaultSectionSize(38);
        Stock_stockTable->verticalHeader()->setHighlightSections(true);
        Stock_stockTable->verticalHeader()->setProperty("showSortIndicator", QVariant(false));
        Stock_stockTable->verticalHeader()->setStretchLastSection(false);

        Stock_control_1->addWidget(Stock_stockTable);

        AdvancedModulesPanel = new QFrame(StockPage);
        AdvancedModulesPanel->setObjectName("AdvancedModulesPanel");
        AdvancedModulesPanel->setMinimumSize(QSize(0, 0));
        AdvancedModulesPanel->setAutoFillBackground(false);
        AdvancedModulesPanel->setMaximumSize(QSize(16777215, 16777215));
        AdvancedModulesPanel->setLineWidth(1);
        AdvancedModulesPanel->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        AdvancedModulesPanel->setGeometry(QRect(0, 0, 640, 480));
        AdvancedModulesPanel->setStyleSheet(QString::fromUtf8("QFrame#AdvancedModulesPanel { background: #FFFFFF; border: 1px solid #D8E9EC; border-radius: 9px; }"));
        sizePolicy.setHeightForWidth(AdvancedModulesPanel->sizePolicy().hasHeightForWidth());
        AdvancedModulesPanel->setSizePolicy(sizePolicy);
        AdvancedModulesPanel->setMidLineWidth(0);
        AdvancedModulesPanel->setFont(font);
        Stock_control_67 = new QVBoxLayout(AdvancedModulesPanel);
        Stock_control_67->setSpacing(8);
        Stock_control_67->setObjectName("Stock_control_67");
        Stock_control_67->setContentsMargins(14, 10, 14, 12);
        Stock_control_68 = new QHBoxLayout();
        Stock_control_68->setSpacing(8);
        Stock_control_68->setObjectName("Stock_control_68");
        Stock_control_68->setContentsMargins(0, 0, 0, 0);
        Stock_control_70 = new QLabel(AdvancedModulesPanel);
        Stock_control_70->setObjectName("Stock_control_70");
        Stock_control_70->setMinimumSize(QSize(0, 0));
        Stock_control_70->setAutoFillBackground(false);
        Stock_control_70->setMaximumSize(QSize(16777215, 16777215));
        Stock_control_70->setLineWidth(1);
        Stock_control_70->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_control_70->setWordWrap(false);
        Stock_control_70->setGeometry(QRect(0, 0, 640, 480));
        Stock_control_70->setStyleSheet(QString::fromUtf8("font-size: 13px; font-weight: 800; color: #225A64; background: transparent;"));
        sizePolicy.setHeightForWidth(Stock_control_70->sizePolicy().hasHeightForWidth());
        Stock_control_70->setSizePolicy(sizePolicy);
        Stock_control_70->setMidLineWidth(0);
        Stock_control_70->setFont(font4);

        Stock_control_68->addWidget(Stock_control_70);

        spacerItem10 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Stock_control_68->addItem(spacerItem10);

        Stock_control_71 = new QLabel(AdvancedModulesPanel);
        Stock_control_71->setObjectName("Stock_control_71");
        Stock_control_71->setMinimumSize(QSize(0, 0));
        Stock_control_71->setAutoFillBackground(false);
        Stock_control_71->setMaximumSize(QSize(16777215, 16777215));
        Stock_control_71->setLineWidth(1);
        Stock_control_71->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_control_71->setWordWrap(false);
        Stock_control_71->setGeometry(QRect(0, 0, 640, 480));
        Stock_control_71->setStyleSheet(QString::fromUtf8("font-size: 10px; color: #71878D; background: transparent;"));
        sizePolicy.setHeightForWidth(Stock_control_71->sizePolicy().hasHeightForWidth());
        Stock_control_71->setSizePolicy(sizePolicy);
        Stock_control_71->setMidLineWidth(0);
        Stock_control_71->setFont(font);

        Stock_control_68->addWidget(Stock_control_71);

        Stock_control_68->setStretch(1, 1);

        Stock_control_67->addLayout(Stock_control_68);

        Stock_control_69 = new QHBoxLayout();
        Stock_control_69->setSpacing(10);
        Stock_control_69->setObjectName("Stock_control_69");
        Stock_control_69->setContentsMargins(0, 0, 0, 0);
        stockPurchasesButton = new QPushButton(AdvancedModulesPanel);
        stockPurchasesButton->setObjectName("stockPurchasesButton");
        stockPurchasesButton->setMinimumSize(QSize(0, 42));
        stockPurchasesButton->setChecked(false);
        stockPurchasesButton->setIconSize(QSize(17, 17));
        stockPurchasesButton->setAutoFillBackground(false);
        stockPurchasesButton->setAutoDefault(false);
        stockPurchasesButton->setMaximumSize(QSize(16777215, 16777215));
        stockPurchasesButton->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        stockPurchasesButton->setFlat(false);
        stockPurchasesButton->setCheckable(false);
        stockPurchasesButton->setGeometry(QRect(0, 0, 640, 480));
        stockPurchasesButton->setStyleSheet(QString::fromUtf8("QPushButton { text-align: left; background: #F8FCFC; color: #225A64; border: 1px solid #D8E9EC; border-radius: 7px; padding: 10px 14px; font-weight: 600; } QPushButton:hover { background: #EAF5F6; border-color: #9BC6CE; }"));
        sizePolicy1.setHeightForWidth(stockPurchasesButton->sizePolicy().hasHeightForWidth());
        stockPurchasesButton->setSizePolicy(sizePolicy1);
        stockPurchasesButton->setFont(font2);

        Stock_control_69->addWidget(stockPurchasesButton);

        Stock_alertsNavButton = new QPushButton(AdvancedModulesPanel);
        Stock_alertsNavButton->setObjectName("Stock_alertsNavButton");
        Stock_alertsNavButton->setMinimumSize(QSize(0, 42));
        Stock_alertsNavButton->setChecked(false);
        Stock_alertsNavButton->setIconSize(QSize(17, 17));
        Stock_alertsNavButton->setAutoFillBackground(false);
        Stock_alertsNavButton->setAutoDefault(false);
        Stock_alertsNavButton->setMaximumSize(QSize(16777215, 16777215));
        Stock_alertsNavButton->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Stock_alertsNavButton->setFlat(false);
        Stock_alertsNavButton->setCheckable(false);
        Stock_alertsNavButton->setGeometry(QRect(0, 0, 640, 480));
        Stock_alertsNavButton->setStyleSheet(QString::fromUtf8("QPushButton { text-align:left; background:#FBEAEA; color:#B94A48; border:1px solid #F0CCCA; border-radius:7px; padding:10px 14px; font-weight:700; } QPushButton:hover { background:#F9DEDC; }"));
        sizePolicy1.setHeightForWidth(Stock_alertsNavButton->sizePolicy().hasHeightForWidth());
        Stock_alertsNavButton->setSizePolicy(sizePolicy1);
        Stock_alertsNavButton->setFont(font1);

        Stock_control_69->addWidget(Stock_alertsNavButton);

        stockReplenishmentButton = new QPushButton(AdvancedModulesPanel);
        stockReplenishmentButton->setObjectName("stockReplenishmentButton");
        stockReplenishmentButton->setMinimumSize(QSize(0, 42));
        stockReplenishmentButton->setChecked(false);
        stockReplenishmentButton->setIconSize(QSize(17, 17));
        stockReplenishmentButton->setAutoFillBackground(false);
        stockReplenishmentButton->setAutoDefault(false);
        stockReplenishmentButton->setMaximumSize(QSize(16777215, 16777215));
        stockReplenishmentButton->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        stockReplenishmentButton->setFlat(false);
        stockReplenishmentButton->setCheckable(false);
        stockReplenishmentButton->setGeometry(QRect(0, 0, 640, 480));
        stockReplenishmentButton->setStyleSheet(QString::fromUtf8("QPushButton { text-align: left; background: #F8FCFC; color: #225A64; border: 1px solid #D8E9EC; border-radius: 7px; padding: 10px 14px; font-weight: 600; } QPushButton:hover { background: #EAF5F6; border-color: #9BC6CE; }"));
        sizePolicy1.setHeightForWidth(stockReplenishmentButton->sizePolicy().hasHeightForWidth());
        stockReplenishmentButton->setSizePolicy(sizePolicy1);
        stockReplenishmentButton->setFont(font2);

        Stock_control_69->addWidget(stockReplenishmentButton);

        Stock_control_69->setStretch(0, 1);
        Stock_control_69->setStretch(1, 1);
        Stock_control_69->setStretch(2, 1);

        Stock_control_67->addLayout(Stock_control_69);


        Stock_control_1->addWidget(AdvancedModulesPanel);

        Stock_control_72 = new QFrame(StockPage);
        Stock_control_72->setObjectName("Stock_control_72");
        Stock_control_72->setMinimumSize(QSize(0, 0));
        Stock_control_72->setAutoFillBackground(false);
        Stock_control_72->setMaximumSize(QSize(16777215, 16777215));
        Stock_control_72->setLineWidth(1);
        Stock_control_72->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_control_72->setGeometry(QRect(0, 0, 640, 480));
        Stock_control_72->setStyleSheet(QString::fromUtf8("QFrame { background: #FFFFFF; border: 1px solid #D8E9EC; border-radius: 9px; }"));
        sizePolicy.setHeightForWidth(Stock_control_72->sizePolicy().hasHeightForWidth());
        Stock_control_72->setSizePolicy(sizePolicy);
        Stock_control_72->setMidLineWidth(0);
        Stock_control_72->setFont(font);
        Stock_control_73 = new QHBoxLayout(Stock_control_72);
        Stock_control_73->setSpacing(12);
        Stock_control_73->setObjectName("Stock_control_73");
        Stock_control_73->setContentsMargins(16, 12, 16, 12);
        Stock_control_74 = new QVBoxLayout();
        Stock_control_74->setSpacing(4);
        Stock_control_74->setObjectName("Stock_control_74");
        Stock_control_74->setContentsMargins(0, 0, 0, 0);
        Stock_control_75 = new QLabel(Stock_control_72);
        Stock_control_75->setObjectName("Stock_control_75");
        Stock_control_75->setMinimumSize(QSize(0, 0));
        Stock_control_75->setAutoFillBackground(false);
        Stock_control_75->setMaximumSize(QSize(16777215, 16777215));
        Stock_control_75->setLineWidth(1);
        Stock_control_75->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_control_75->setWordWrap(false);
        Stock_control_75->setGeometry(QRect(0, 0, 640, 480));
        Stock_control_75->setStyleSheet(QString::fromUtf8("color: #B94A48; font-weight: 700; background: transparent;"));
        sizePolicy.setHeightForWidth(Stock_control_75->sizePolicy().hasHeightForWidth());
        Stock_control_75->setSizePolicy(sizePolicy);
        Stock_control_75->setMidLineWidth(0);
        Stock_control_75->setFont(font1);

        Stock_control_74->addWidget(Stock_control_75);

        Stock_control_76 = new QLabel(Stock_control_72);
        Stock_control_76->setObjectName("Stock_control_76");
        Stock_control_76->setMinimumSize(QSize(0, 0));
        Stock_control_76->setAutoFillBackground(false);
        Stock_control_76->setMaximumSize(QSize(16777215, 16777215));
        Stock_control_76->setLineWidth(1);
        Stock_control_76->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_control_76->setWordWrap(true);
        Stock_control_76->setGeometry(QRect(0, 0, 640, 480));
        Stock_control_76->setStyleSheet(QString::fromUtf8("color: #71878D; background: transparent;"));
        sizePolicy.setHeightForWidth(Stock_control_76->sizePolicy().hasHeightForWidth());
        Stock_control_76->setSizePolicy(sizePolicy);
        Stock_control_76->setMidLineWidth(0);
        Stock_control_76->setFont(font);

        Stock_control_74->addWidget(Stock_control_76);


        Stock_control_73->addLayout(Stock_control_74);

        stockExpiryButton = new QPushButton(Stock_control_72);
        stockExpiryButton->setObjectName("stockExpiryButton");
        stockExpiryButton->setMinimumSize(QSize(245, 0));
        stockExpiryButton->setChecked(false);
        stockExpiryButton->setIconSize(QSize(17, 17));
        stockExpiryButton->setAutoFillBackground(false);
        stockExpiryButton->setAutoDefault(false);
        stockExpiryButton->setMaximumSize(QSize(16777215, 16777215));
        stockExpiryButton->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        stockExpiryButton->setFlat(false);
        stockExpiryButton->setCheckable(false);
        stockExpiryButton->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(stockExpiryButton->sizePolicy().hasHeightForWidth());
        stockExpiryButton->setSizePolicy(sizePolicy1);
        stockExpiryButton->setFont(font2);

        Stock_control_73->addWidget(stockExpiryButton);

        Stock_control_73->setStretch(0, 1);

        Stock_control_1->addWidget(Stock_control_72);

        Stock_control_6 = new QHBoxLayout();
        Stock_control_6->setSpacing(14);
        Stock_control_6->setObjectName("Stock_control_6");
        Stock_control_6->setContentsMargins(0, 0, 0, 0);
        Stock_footerLabel = new QLabel(StockPage);
        Stock_footerLabel->setObjectName("Stock_footerLabel");
        Stock_footerLabel->setMinimumSize(QSize(0, 0));
        Stock_footerLabel->setAutoFillBackground(false);
        Stock_footerLabel->setMaximumSize(QSize(16777215, 16777215));
        Stock_footerLabel->setLineWidth(1);
        Stock_footerLabel->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_footerLabel->setWordWrap(false);
        Stock_footerLabel->setGeometry(QRect(0, 0, 640, 480));
        Stock_footerLabel->setStyleSheet(QString::fromUtf8("color: #71878D; font-size: 10px; background: transparent;"));
        sizePolicy.setHeightForWidth(Stock_footerLabel->sizePolicy().hasHeightForWidth());
        Stock_footerLabel->setSizePolicy(sizePolicy);
        Stock_footerLabel->setMidLineWidth(0);
        Stock_footerLabel->setFont(font);

        Stock_control_6->addWidget(Stock_footerLabel);

        spacerItem11 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Stock_control_6->addItem(spacerItem11);

        Stock_control_77 = new QLabel(StockPage);
        Stock_control_77->setObjectName("Stock_control_77");
        Stock_control_77->setMinimumSize(QSize(0, 0));
        Stock_control_77->setAutoFillBackground(false);
        Stock_control_77->setMaximumSize(QSize(16777215, 16777215));
        Stock_control_77->setLineWidth(1);
        Stock_control_77->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Stock_control_77->setWordWrap(false);
        Stock_control_77->setGeometry(QRect(0, 0, 640, 480));
        Stock_control_77->setStyleSheet(QString::fromUtf8("color: #83979C; font-size: 10px; background: transparent;"));
        sizePolicy.setHeightForWidth(Stock_control_77->sizePolicy().hasHeightForWidth());
        Stock_control_77->setSizePolicy(sizePolicy);
        Stock_control_77->setMidLineWidth(0);
        Stock_control_77->setFont(font);

        Stock_control_6->addWidget(Stock_control_77);

        Stock_control_6->setStretch(1, 1);

        Stock_control_1->addLayout(Stock_control_6);

        Stock_control_1->setStretch(3, 1);
        pageStack->addWidget(StockPage);
        EmployeesPage = new QWidget();
        EmployeesPage->setObjectName("EmployeesPage");
        EmployeesPage->setAutoFillBackground(false);
        EmployeesPage->setMinimumSize(QSize(0, 0));
        EmployeesPage->setMaximumSize(QSize(16777215, 16777215));
        EmployeesPage->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        EmployeesPage->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(EmployeesPage->sizePolicy().hasHeightForWidth());
        EmployeesPage->setSizePolicy(sizePolicy);
        EmployeesPage->setFont(font3);
        Employees_control_1 = new QVBoxLayout(EmployeesPage);
        Employees_control_1->setSpacing(16);
        Employees_control_1->setObjectName("Employees_control_1");
        Employees_control_1->setContentsMargins(24, 24, 24, 20);
        Employees_control_3 = new QWidget(EmployeesPage);
        Employees_control_3->setObjectName("Employees_control_3");
        Employees_control_3->setAutoFillBackground(false);
        Employees_control_3->setMinimumSize(QSize(0, 0));
        Employees_control_3->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_3->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_3->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Employees_control_3->sizePolicy().hasHeightForWidth());
        Employees_control_3->setSizePolicy(sizePolicy);
        Employees_control_3->setFont(font3);
        Employees_control_4 = new QHBoxLayout(Employees_control_3);
        Employees_control_4->setSpacing(6);
        Employees_control_4->setObjectName("Employees_control_4");
        Employees_control_4->setContentsMargins(0, 0, 0, 0);
        Employees_control_5 = new QVBoxLayout();
        Employees_control_5->setSpacing(4);
        Employees_control_5->setObjectName("Employees_control_5");
        Employees_control_5->setContentsMargins(0, 0, 0, 0);
        Employees_control_6 = new QHBoxLayout();
        Employees_control_6->setSpacing(4);
        Employees_control_6->setObjectName("Employees_control_6");
        Employees_control_6->setContentsMargins(0, 0, 0, 0);
        Employees_control_7 = new QLabel(Employees_control_3);
        Employees_control_7->setObjectName("Employees_control_7");
        Employees_control_7->setMinimumSize(QSize(0, 0));
        Employees_control_7->setAutoFillBackground(false);
        Employees_control_7->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_7->setLineWidth(1);
        Employees_control_7->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_7->setWordWrap(false);
        Employees_control_7->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Employees_control_7->sizePolicy().hasHeightForWidth());
        Employees_control_7->setSizePolicy(sizePolicy);
        Employees_control_7->setMidLineWidth(0);
        Employees_control_7->setFont(font3);
        Employees_control_7->setPixmap(QPixmap(QString::fromUtf8(":/designer/Employees_control_7.png")));

        Employees_control_6->addWidget(Employees_control_7);

        Employees_control_8 = new QLabel(Employees_control_3);
        Employees_control_8->setObjectName("Employees_control_8");
        Employees_control_8->setMinimumSize(QSize(0, 0));
        Employees_control_8->setAutoFillBackground(false);
        Employees_control_8->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_8->setLineWidth(1);
        Employees_control_8->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_8->setWordWrap(false);
        Employees_control_8->setGeometry(QRect(0, 0, 640, 480));
        Employees_control_8->setStyleSheet(QString::fromUtf8("font-size: 23px; font-weight: bold; color: #225A64;background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_control_8->sizePolicy().hasHeightForWidth());
        Employees_control_8->setSizePolicy(sizePolicy);
        Employees_control_8->setMidLineWidth(0);
        Employees_control_8->setFont(font1);

        Employees_control_6->addWidget(Employees_control_8);

        spacerItem12 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Employees_control_6->addItem(spacerItem12);


        Employees_control_5->addLayout(Employees_control_6);

        Employees_control_9 = new QLabel(Employees_control_3);
        Employees_control_9->setObjectName("Employees_control_9");
        Employees_control_9->setMinimumSize(QSize(0, 0));
        Employees_control_9->setAutoFillBackground(false);
        Employees_control_9->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_9->setLineWidth(1);
        Employees_control_9->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_9->setWordWrap(false);
        Employees_control_9->setGeometry(QRect(0, 0, 640, 480));
        Employees_control_9->setStyleSheet(QString::fromUtf8("font-size: 13px; color: #7A8B8F; background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_control_9->sizePolicy().hasHeightForWidth());
        Employees_control_9->setSizePolicy(sizePolicy);
        Employees_control_9->setMidLineWidth(0);
        Employees_control_9->setFont(font);

        Employees_control_5->addWidget(Employees_control_9);


        Employees_control_4->addLayout(Employees_control_5);

        spacerItem13 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Employees_control_4->addItem(spacerItem13);

        employeeLeavePdf = new QPushButton(Employees_control_3);
        employeeLeavePdf->setObjectName("employeeLeavePdf");
        employeeLeavePdf->setMinimumSize(QSize(0, 0));
        employeeLeavePdf->setChecked(false);
        employeeLeavePdf->setIconSize(QSize(17, 17));
        employeeLeavePdf->setAutoFillBackground(false);
        employeeLeavePdf->setAutoDefault(false);
        employeeLeavePdf->setMaximumSize(QSize(16777215, 16777215));
        employeeLeavePdf->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        employeeLeavePdf->setFlat(false);
        employeeLeavePdf->setCheckable(false);
        employeeLeavePdf->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(employeeLeavePdf->sizePolicy().hasHeightForWidth());
        employeeLeavePdf->setSizePolicy(sizePolicy1);
        employeeLeavePdf->setFont(font3);
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/designer/employeeLeavePdf.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        employeeLeavePdf->setIcon(icon12);

        Employees_control_4->addWidget(employeeLeavePdf);

        addEmployee = new QPushButton(Employees_control_3);
        addEmployee->setObjectName("addEmployee");
        addEmployee->setMinimumSize(QSize(210, 36));
        addEmployee->setChecked(false);
        addEmployee->setIconSize(QSize(17, 17));
        addEmployee->setAutoFillBackground(false);
        addEmployee->setAutoDefault(false);
        addEmployee->setMaximumSize(QSize(16777215, 16777215));
        addEmployee->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        addEmployee->setFlat(false);
        addEmployee->setCheckable(false);
        addEmployee->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(addEmployee->sizePolicy().hasHeightForWidth());
        addEmployee->setSizePolicy(sizePolicy1);
        addEmployee->setFont(font3);
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/designer/addEmployee.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        addEmployee->setIcon(icon13);

        Employees_control_4->addWidget(addEmployee);


        Employees_control_1->addWidget(Employees_control_3);

        Employees_control_10 = new QWidget(EmployeesPage);
        Employees_control_10->setObjectName("Employees_control_10");
        Employees_control_10->setAutoFillBackground(false);
        Employees_control_10->setMinimumSize(QSize(0, 0));
        Employees_control_10->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_10->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_10->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Employees_control_10->sizePolicy().hasHeightForWidth());
        Employees_control_10->setSizePolicy(sizePolicy);
        Employees_control_10->setFont(font3);
        Employees_control_11 = new QHBoxLayout(Employees_control_10);
        Employees_control_11->setSpacing(12);
        Employees_control_11->setObjectName("Employees_control_11");
        Employees_control_11->setContentsMargins(0, 0, 0, 0);
        Employees_control_12 = new QFrame(Employees_control_10);
        Employees_control_12->setObjectName("Employees_control_12");
        Employees_control_12->setMinimumSize(QSize(0, 44));
        Employees_control_12->setAutoFillBackground(false);
        Employees_control_12->setMaximumSize(QSize(16777215, 44));
        Employees_control_12->setLineWidth(1);
        Employees_control_12->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_12->setGeometry(QRect(0, 0, 640, 44));
        Employees_control_12->setStyleSheet(QString::fromUtf8("QFrame { background-color: #E6F4EA; border-radius: 22px; border: none; }"));
        sizePolicy.setHeightForWidth(Employees_control_12->sizePolicy().hasHeightForWidth());
        Employees_control_12->setSizePolicy(sizePolicy);
        Employees_control_12->setMidLineWidth(0);
        Employees_control_12->setFont(font);
        Employees_control_13 = new QHBoxLayout(Employees_control_12);
        Employees_control_13->setSpacing(8);
        Employees_control_13->setObjectName("Employees_control_13");
        Employees_control_13->setContentsMargins(18, 5, 22, 5);
        Employees_control_14 = new QLabel(Employees_control_12);
        Employees_control_14->setObjectName("Employees_control_14");
        Employees_control_14->setMinimumSize(QSize(0, 0));
        Employees_control_14->setAutoFillBackground(false);
        Employees_control_14->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_14->setLineWidth(1);
        Employees_control_14->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_14->setWordWrap(false);
        Employees_control_14->setGeometry(QRect(0, 0, 640, 480));
        Employees_control_14->setStyleSheet(QString::fromUtf8("font-size: 15px; background: transparent; border: none;"));
        sizePolicy.setHeightForWidth(Employees_control_14->sizePolicy().hasHeightForWidth());
        Employees_control_14->setSizePolicy(sizePolicy);
        Employees_control_14->setMidLineWidth(0);
        Employees_control_14->setFont(font);
        Employees_control_14->setPixmap(QPixmap(QString::fromUtf8(":/designer/Employees_control_14.png")));

        Employees_control_13->addWidget(Employees_control_14);

        Employees_m_lblDisponibles = new QLabel(Employees_control_12);
        Employees_m_lblDisponibles->setObjectName("Employees_m_lblDisponibles");
        Employees_m_lblDisponibles->setMinimumSize(QSize(0, 0));
        Employees_m_lblDisponibles->setAutoFillBackground(false);
        Employees_m_lblDisponibles->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_lblDisponibles->setLineWidth(1);
        Employees_m_lblDisponibles->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_lblDisponibles->setWordWrap(false);
        Employees_m_lblDisponibles->setGeometry(QRect(0, 0, 640, 480));
        Employees_m_lblDisponibles->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: #2E7D32; background: transparent; border: none;"));
        sizePolicy.setHeightForWidth(Employees_m_lblDisponibles->sizePolicy().hasHeightForWidth());
        Employees_m_lblDisponibles->setSizePolicy(sizePolicy);
        Employees_m_lblDisponibles->setMidLineWidth(0);
        Employees_m_lblDisponibles->setFont(font1);

        Employees_control_13->addWidget(Employees_m_lblDisponibles);

        Employees_control_15 = new QLabel(Employees_control_12);
        Employees_control_15->setObjectName("Employees_control_15");
        Employees_control_15->setMinimumSize(QSize(0, 0));
        Employees_control_15->setAutoFillBackground(false);
        Employees_control_15->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_15->setLineWidth(1);
        Employees_control_15->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_15->setWordWrap(false);
        Employees_control_15->setGeometry(QRect(0, 0, 640, 480));
        Employees_control_15->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #2E7D32; background: transparent; border: none;"));
        sizePolicy.setHeightForWidth(Employees_control_15->sizePolicy().hasHeightForWidth());
        Employees_control_15->setSizePolicy(sizePolicy);
        Employees_control_15->setMidLineWidth(0);
        Employees_control_15->setFont(font);

        Employees_control_13->addWidget(Employees_control_15);


        Employees_control_11->addWidget(Employees_control_12);

        Employees_control_16 = new QFrame(Employees_control_10);
        Employees_control_16->setObjectName("Employees_control_16");
        Employees_control_16->setMinimumSize(QSize(0, 44));
        Employees_control_16->setAutoFillBackground(false);
        Employees_control_16->setMaximumSize(QSize(16777215, 44));
        Employees_control_16->setLineWidth(1);
        Employees_control_16->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_16->setGeometry(QRect(0, 0, 640, 44));
        Employees_control_16->setStyleSheet(QString::fromUtf8("QFrame { background-color: #FBE9E7; border-radius: 22px; border: none; }"));
        sizePolicy.setHeightForWidth(Employees_control_16->sizePolicy().hasHeightForWidth());
        Employees_control_16->setSizePolicy(sizePolicy);
        Employees_control_16->setMidLineWidth(0);
        Employees_control_16->setFont(font);
        Employees_control_17 = new QHBoxLayout(Employees_control_16);
        Employees_control_17->setSpacing(8);
        Employees_control_17->setObjectName("Employees_control_17");
        Employees_control_17->setContentsMargins(18, 5, 22, 5);
        Employees_control_18 = new QLabel(Employees_control_16);
        Employees_control_18->setObjectName("Employees_control_18");
        Employees_control_18->setMinimumSize(QSize(0, 0));
        Employees_control_18->setAutoFillBackground(false);
        Employees_control_18->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_18->setLineWidth(1);
        Employees_control_18->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_18->setWordWrap(false);
        Employees_control_18->setGeometry(QRect(0, 0, 640, 480));
        Employees_control_18->setStyleSheet(QString::fromUtf8("font-size: 15px; background: transparent; border: none;"));
        sizePolicy.setHeightForWidth(Employees_control_18->sizePolicy().hasHeightForWidth());
        Employees_control_18->setSizePolicy(sizePolicy);
        Employees_control_18->setMidLineWidth(0);
        Employees_control_18->setFont(font);
        Employees_control_18->setPixmap(QPixmap(QString::fromUtf8(":/designer/Employees_control_18.png")));

        Employees_control_17->addWidget(Employees_control_18);

        Employees_m_lblIndisponibles = new QLabel(Employees_control_16);
        Employees_m_lblIndisponibles->setObjectName("Employees_m_lblIndisponibles");
        Employees_m_lblIndisponibles->setMinimumSize(QSize(0, 0));
        Employees_m_lblIndisponibles->setAutoFillBackground(false);
        Employees_m_lblIndisponibles->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_lblIndisponibles->setLineWidth(1);
        Employees_m_lblIndisponibles->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_lblIndisponibles->setWordWrap(false);
        Employees_m_lblIndisponibles->setGeometry(QRect(0, 0, 640, 480));
        Employees_m_lblIndisponibles->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: #AE5751; background: transparent; border: none;"));
        sizePolicy.setHeightForWidth(Employees_m_lblIndisponibles->sizePolicy().hasHeightForWidth());
        Employees_m_lblIndisponibles->setSizePolicy(sizePolicy);
        Employees_m_lblIndisponibles->setMidLineWidth(0);
        Employees_m_lblIndisponibles->setFont(font1);

        Employees_control_17->addWidget(Employees_m_lblIndisponibles);

        Employees_control_19 = new QLabel(Employees_control_16);
        Employees_control_19->setObjectName("Employees_control_19");
        Employees_control_19->setMinimumSize(QSize(0, 0));
        Employees_control_19->setAutoFillBackground(false);
        Employees_control_19->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_19->setLineWidth(1);
        Employees_control_19->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_19->setWordWrap(false);
        Employees_control_19->setGeometry(QRect(0, 0, 640, 480));
        Employees_control_19->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #AE5751; background: transparent; border: none;"));
        sizePolicy.setHeightForWidth(Employees_control_19->sizePolicy().hasHeightForWidth());
        Employees_control_19->setSizePolicy(sizePolicy);
        Employees_control_19->setMidLineWidth(0);
        Employees_control_19->setFont(font);

        Employees_control_17->addWidget(Employees_control_19);


        Employees_control_11->addWidget(Employees_control_16);

        Employees_control_20 = new QFrame(Employees_control_10);
        Employees_control_20->setObjectName("Employees_control_20");
        Employees_control_20->setMinimumSize(QSize(0, 44));
        Employees_control_20->setAutoFillBackground(false);
        Employees_control_20->setMaximumSize(QSize(16777215, 44));
        Employees_control_20->setLineWidth(1);
        Employees_control_20->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_20->setGeometry(QRect(0, 0, 640, 44));
        Employees_control_20->setStyleSheet(QString::fromUtf8("QFrame { background-color: #FDF3E3; border-radius: 22px; border: none; }"));
        sizePolicy.setHeightForWidth(Employees_control_20->sizePolicy().hasHeightForWidth());
        Employees_control_20->setSizePolicy(sizePolicy);
        Employees_control_20->setMidLineWidth(0);
        Employees_control_20->setFont(font);
        Employees_control_21 = new QHBoxLayout(Employees_control_20);
        Employees_control_21->setSpacing(8);
        Employees_control_21->setObjectName("Employees_control_21");
        Employees_control_21->setContentsMargins(18, 5, 22, 5);
        Employees_control_22 = new QLabel(Employees_control_20);
        Employees_control_22->setObjectName("Employees_control_22");
        Employees_control_22->setMinimumSize(QSize(0, 0));
        Employees_control_22->setAutoFillBackground(false);
        Employees_control_22->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_22->setLineWidth(1);
        Employees_control_22->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_22->setWordWrap(false);
        Employees_control_22->setGeometry(QRect(0, 0, 640, 480));
        Employees_control_22->setStyleSheet(QString::fromUtf8("font-size: 15px; background: transparent; border: none;"));
        sizePolicy.setHeightForWidth(Employees_control_22->sizePolicy().hasHeightForWidth());
        Employees_control_22->setSizePolicy(sizePolicy);
        Employees_control_22->setMidLineWidth(0);
        Employees_control_22->setFont(font);
        Employees_control_22->setPixmap(QPixmap(QString::fromUtf8(":/designer/Employees_control_22.png")));

        Employees_control_21->addWidget(Employees_control_22);

        Employees_m_lblChargeTotale = new QLabel(Employees_control_20);
        Employees_m_lblChargeTotale->setObjectName("Employees_m_lblChargeTotale");
        Employees_m_lblChargeTotale->setMinimumSize(QSize(0, 0));
        Employees_m_lblChargeTotale->setAutoFillBackground(false);
        Employees_m_lblChargeTotale->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_lblChargeTotale->setLineWidth(1);
        Employees_m_lblChargeTotale->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_lblChargeTotale->setWordWrap(false);
        Employees_m_lblChargeTotale->setGeometry(QRect(0, 0, 640, 480));
        Employees_m_lblChargeTotale->setStyleSheet(QString::fromUtf8("font-size: 16px; font-weight: bold; color: #B8741A; background: transparent; border: none;"));
        sizePolicy.setHeightForWidth(Employees_m_lblChargeTotale->sizePolicy().hasHeightForWidth());
        Employees_m_lblChargeTotale->setSizePolicy(sizePolicy);
        Employees_m_lblChargeTotale->setMidLineWidth(0);
        Employees_m_lblChargeTotale->setFont(font1);

        Employees_control_21->addWidget(Employees_m_lblChargeTotale);

        Employees_control_23 = new QLabel(Employees_control_20);
        Employees_control_23->setObjectName("Employees_control_23");
        Employees_control_23->setMinimumSize(QSize(0, 0));
        Employees_control_23->setAutoFillBackground(false);
        Employees_control_23->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_23->setLineWidth(1);
        Employees_control_23->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_23->setWordWrap(false);
        Employees_control_23->setGeometry(QRect(0, 0, 640, 480));
        Employees_control_23->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #B8741A; background: transparent; border: none;"));
        sizePolicy.setHeightForWidth(Employees_control_23->sizePolicy().hasHeightForWidth());
        Employees_control_23->setSizePolicy(sizePolicy);
        Employees_control_23->setMidLineWidth(0);
        Employees_control_23->setFont(font);

        Employees_control_21->addWidget(Employees_control_23);


        Employees_control_11->addWidget(Employees_control_20);

        spacerItem14 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Employees_control_11->addItem(spacerItem14);

        employeeStatistics = new QPushButton(Employees_control_10);
        employeeStatistics->setObjectName("employeeStatistics");
        employeeStatistics->setMinimumSize(QSize(0, 36));
        employeeStatistics->setChecked(false);
        employeeStatistics->setIconSize(QSize(17, 17));
        employeeStatistics->setAutoFillBackground(false);
        employeeStatistics->setAutoDefault(false);
        employeeStatistics->setMaximumSize(QSize(16777215, 16777215));
        employeeStatistics->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        employeeStatistics->setFlat(false);
        employeeStatistics->setCheckable(false);
        employeeStatistics->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(employeeStatistics->sizePolicy().hasHeightForWidth());
        employeeStatistics->setSizePolicy(sizePolicy1);
        employeeStatistics->setFont(font3);
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/designer/employeeStatistics.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        employeeStatistics->setIcon(icon14);

        Employees_control_11->addWidget(employeeStatistics);


        Employees_control_1->addWidget(Employees_control_10);

        Employees_control_24 = new QWidget(EmployeesPage);
        Employees_control_24->setObjectName("Employees_control_24");
        Employees_control_24->setAutoFillBackground(false);
        Employees_control_24->setMinimumSize(QSize(0, 0));
        Employees_control_24->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_24->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_24->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Employees_control_24->sizePolicy().hasHeightForWidth());
        Employees_control_24->setSizePolicy(sizePolicy);
        Employees_control_24->setFont(font3);
        Employees_control_25 = new QHBoxLayout(Employees_control_24);
        Employees_control_25->setSpacing(10);
        Employees_control_25->setObjectName("Employees_control_25");
        Employees_control_25->setContentsMargins(0, 0, 0, 0);
        Employees_m_editRecherche = new QLineEdit(Employees_control_24);
        Employees_m_editRecherche->setObjectName("Employees_m_editRecherche");
        Employees_m_editRecherche->setMinimumSize(QSize(0, 34));
        Employees_m_editRecherche->setAutoFillBackground(false);
        Employees_m_editRecherche->setMaxLength(32767);
        Employees_m_editRecherche->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_editRecherche->setCursor(QCursor(Qt::CursorShape::IBeamCursor));
        Employees_m_editRecherche->setReadOnly(false);
        Employees_m_editRecherche->setClearButtonEnabled(false);
        Employees_m_editRecherche->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy2.setHeightForWidth(Employees_m_editRecherche->sizePolicy().hasHeightForWidth());
        Employees_m_editRecherche->setSizePolicy(sizePolicy2);
        Employees_m_editRecherche->setFont(font3);

        Employees_control_25->addWidget(Employees_m_editRecherche);

        Employees_m_comboPoste = new QComboBox(Employees_control_24);
        Employees_m_comboPoste->addItem(QString());
        Employees_m_comboPoste->addItem(QString());
        Employees_m_comboPoste->addItem(QString());
        Employees_m_comboPoste->addItem(QString());
        Employees_m_comboPoste->addItem(QString());
        Employees_m_comboPoste->addItem(QString());
        Employees_m_comboPoste->addItem(QString());
        Employees_m_comboPoste->setObjectName("Employees_m_comboPoste");
        Employees_m_comboPoste->setMinimumSize(QSize(110, 34));
        Employees_m_comboPoste->setMinimumContentsLength(0);
        Employees_m_comboPoste->setIconSize(QSize(16, 16));
        Employees_m_comboPoste->setAutoFillBackground(false);
        Employees_m_comboPoste->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_comboPoste->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_comboPoste->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy3.setHeightForWidth(Employees_m_comboPoste->sizePolicy().hasHeightForWidth());
        Employees_m_comboPoste->setSizePolicy(sizePolicy3);
        Employees_m_comboPoste->setEditable(false);
        Employees_m_comboPoste->setFont(font3);

        Employees_control_25->addWidget(Employees_m_comboPoste);

        Employees_m_comboStatut = new QComboBox(Employees_control_24);
        Employees_m_comboStatut->addItem(QString());
        Employees_m_comboStatut->addItem(QString());
        Employees_m_comboStatut->addItem(QString());
        Employees_m_comboStatut->setObjectName("Employees_m_comboStatut");
        Employees_m_comboStatut->setMinimumSize(QSize(110, 34));
        Employees_m_comboStatut->setMinimumContentsLength(0);
        Employees_m_comboStatut->setIconSize(QSize(16, 16));
        Employees_m_comboStatut->setAutoFillBackground(false);
        Employees_m_comboStatut->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_comboStatut->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_comboStatut->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy3.setHeightForWidth(Employees_m_comboStatut->sizePolicy().hasHeightForWidth());
        Employees_m_comboStatut->setSizePolicy(sizePolicy3);
        Employees_m_comboStatut->setEditable(false);
        Employees_m_comboStatut->setFont(font3);

        Employees_control_25->addWidget(Employees_m_comboStatut);

        Employees_m_comboTri = new QComboBox(Employees_control_24);
        Employees_m_comboTri->addItem(QString());
        Employees_m_comboTri->addItem(QString());
        Employees_m_comboTri->setObjectName("Employees_m_comboTri");
        Employees_m_comboTri->setMinimumSize(QSize(110, 34));
        Employees_m_comboTri->setMinimumContentsLength(0);
        Employees_m_comboTri->setIconSize(QSize(16, 16));
        Employees_m_comboTri->setAutoFillBackground(false);
        Employees_m_comboTri->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_comboTri->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_comboTri->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy3.setHeightForWidth(Employees_m_comboTri->sizePolicy().hasHeightForWidth());
        Employees_m_comboTri->setSizePolicy(sizePolicy3);
        Employees_m_comboTri->setEditable(false);
        Employees_m_comboTri->setFont(font3);

        Employees_control_25->addWidget(Employees_m_comboTri);

        Employees_control_25->setStretch(0, 3);
        Employees_control_25->setStretch(1, 1);
        Employees_control_25->setStretch(2, 1);
        Employees_control_25->setStretch(3, 1);

        Employees_control_1->addWidget(Employees_control_24);

        Employees_control_2 = new QHBoxLayout();
        Employees_control_2->setSpacing(20);
        Employees_control_2->setObjectName("Employees_control_2");
        Employees_control_2->setContentsMargins(0, 0, 0, 0);
        Employees_control_60 = new QScrollArea(EmployeesPage);
        Employees_control_60->setObjectName("Employees_control_60");
        Employees_control_60->setMinimumSize(QSize(0, 0));
        Employees_control_60->setWidgetResizable(true);
        Employees_control_60->setAutoFillBackground(false);
        Employees_control_60->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_60->setLineWidth(1);
        Employees_control_60->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CursorShape::ArrowCursor)));
        Employees_control_60->setGeometry(QRect(0, 0, 640, 480));
        Employees_control_60->setStyleSheet(QString::fromUtf8("QScrollArea { background: transparent; border: none; }QScrollArea > QWidget > QWidget { background: transparent; }"));
        sizePolicy4.setHeightForWidth(Employees_control_60->sizePolicy().hasHeightForWidth());
        Employees_control_60->setSizePolicy(sizePolicy4);
        Employees_control_60->setMidLineWidth(0);
        Employees_control_60->setFont(font);
        Employees_m_cardsContainer = new QWidget();
        Employees_m_cardsContainer->setObjectName("Employees_m_cardsContainer");
        Employees_m_cardsContainer->setAutoFillBackground(false);
        Employees_m_cardsContainer->setMinimumSize(QSize(0, 0));
        Employees_m_cardsContainer->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_cardsContainer->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_cardsContainer->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Employees_m_cardsContainer->sizePolicy().hasHeightForWidth());
        Employees_m_cardsContainer->setSizePolicy(sizePolicy);
        Employees_m_cardsContainer->setFont(font);
        Employees_m_gridCards = new QGridLayout(Employees_m_cardsContainer);
        Employees_m_gridCards->setSpacing(14);
        Employees_m_gridCards->setObjectName("Employees_m_gridCards");
        Employees_m_gridCards->setContentsMargins(0, 0, 0, 0);
        Employees_control_60->setWidget(Employees_m_cardsContainer);

        Employees_control_2->addWidget(Employees_control_60);

        employeeDetailPanel = new QFrame(EmployeesPage);
        employeeDetailPanel->setObjectName("employeeDetailPanel");
        employeeDetailPanel->setMinimumSize(QSize(284, 0));
        employeeDetailPanel->setAutoFillBackground(false);
        employeeDetailPanel->setMaximumSize(QSize(306, 16777215));
        employeeDetailPanel->setLineWidth(1);
        employeeDetailPanel->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        employeeDetailPanel->setGeometry(QRect(0, 0, 306, 480));
        sizePolicy.setHeightForWidth(employeeDetailPanel->sizePolicy().hasHeightForWidth());
        employeeDetailPanel->setSizePolicy(sizePolicy);
        employeeDetailPanel->setMidLineWidth(0);
        employeeDetailPanel->setFont(font3);
        Employees_control_69 = new QVBoxLayout(employeeDetailPanel);
        Employees_control_69->setSpacing(0);
        Employees_control_69->setObjectName("Employees_control_69");
        Employees_control_69->setContentsMargins(0, 0, 0, 0);
        Employees_control_70 = new QLabel(employeeDetailPanel);
        Employees_control_70->setObjectName("Employees_control_70");
        Employees_control_70->setMinimumSize(QSize(0, 0));
        Employees_control_70->setAutoFillBackground(false);
        Employees_control_70->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_70->setLineWidth(1);
        Employees_control_70->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_70->setWordWrap(false);
        Employees_control_70->setGeometry(QRect(0, 0, 640, 480));
        Employees_control_70->setStyleSheet(QString::fromUtf8("font-size: 11px; font-weight: bold; color: #7A8B8F;letter-spacing: 1px; padding: 18px 18px 8px 18px;background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_control_70->sizePolicy().hasHeightForWidth());
        Employees_control_70->setSizePolicy(sizePolicy);
        Employees_control_70->setMidLineWidth(0);
        Employees_control_70->setFont(font1);

        Employees_control_69->addWidget(Employees_control_70);

        Employees_m_detailPlaceholder = new QLabel(employeeDetailPanel);
        Employees_m_detailPlaceholder->setObjectName("Employees_m_detailPlaceholder");
        Employees_m_detailPlaceholder->setMinimumSize(QSize(0, 0));
        Employees_m_detailPlaceholder->setAutoFillBackground(false);
        Employees_m_detailPlaceholder->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailPlaceholder->setLineWidth(1);
        Employees_m_detailPlaceholder->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailPlaceholder->setWordWrap(false);
        Employees_m_detailPlaceholder->setGeometry(QRect(0, 0, 640, 480));
        Employees_m_detailPlaceholder->setStyleSheet(QString::fromUtf8("font-size: 13px; color: #7A8B8F; padding: 80px 20px;background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_m_detailPlaceholder->sizePolicy().hasHeightForWidth());
        Employees_m_detailPlaceholder->setSizePolicy(sizePolicy);
        Employees_m_detailPlaceholder->setMidLineWidth(0);
        Employees_m_detailPlaceholder->setFont(font);

        Employees_control_69->addWidget(Employees_m_detailPlaceholder);

        Employees_control_71 = new QScrollArea(employeeDetailPanel);
        Employees_control_71->setObjectName("Employees_control_71");
        Employees_control_71->setMinimumSize(QSize(0, 0));
        Employees_control_71->setWidgetResizable(true);
        Employees_control_71->setAutoFillBackground(false);
        Employees_control_71->setMaximumSize(QSize(16777215, 16777215));
        Employees_control_71->setLineWidth(1);
        Employees_control_71->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CursorShape::ArrowCursor)));
        Employees_control_71->setGeometry(QRect(0, 0, 640, 480));
        Employees_control_71->setStyleSheet(QString::fromUtf8("QScrollArea { background: transparent; border: none; }"));
        sizePolicy4.setHeightForWidth(Employees_control_71->sizePolicy().hasHeightForWidth());
        Employees_control_71->setSizePolicy(sizePolicy4);
        Employees_control_71->setMidLineWidth(0);
        Employees_control_71->setFont(font);
        Employees_m_detailContent = new QWidget();
        Employees_m_detailContent->setObjectName("Employees_m_detailContent");
        Employees_m_detailContent->setAutoFillBackground(false);
        Employees_m_detailContent->setMinimumSize(QSize(0, 0));
        Employees_m_detailContent->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailContent->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailContent->setGeometry(QRect(0, 0, 640, 480));
        Employees_m_detailContent->setStyleSheet(QString::fromUtf8("QWidget#detailContent { background: transparent; }"));
        sizePolicy.setHeightForWidth(Employees_m_detailContent->sizePolicy().hasHeightForWidth());
        Employees_m_detailContent->setSizePolicy(sizePolicy);
        Employees_m_detailContent->setFont(font);
        Employees_control_73 = new QVBoxLayout(Employees_m_detailContent);
        Employees_control_73->setSpacing(10);
        Employees_control_73->setObjectName("Employees_control_73");
        Employees_control_73->setContentsMargins(18, 5, 18, 12);
        Employees_m_detailNom = new QLabel(Employees_m_detailContent);
        Employees_m_detailNom->setObjectName("Employees_m_detailNom");
        Employees_m_detailNom->setMinimumSize(QSize(0, 0));
        Employees_m_detailNom->setAutoFillBackground(false);
        Employees_m_detailNom->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailNom->setLineWidth(1);
        Employees_m_detailNom->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailNom->setWordWrap(true);
        Employees_m_detailNom->setGeometry(QRect(18, 5, 604, 30));
        Employees_m_detailNom->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; color: #225A64;background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_m_detailNom->sizePolicy().hasHeightForWidth());
        Employees_m_detailNom->setSizePolicy(sizePolicy);
        Employees_m_detailNom->setMidLineWidth(0);
        Employees_m_detailNom->setFont(font1);

        Employees_control_73->addWidget(Employees_m_detailNom);

        Employees_m_detailId = new QLabel(Employees_m_detailContent);
        Employees_m_detailId->setObjectName("Employees_m_detailId");
        Employees_m_detailId->setMinimumSize(QSize(0, 0));
        Employees_m_detailId->setAutoFillBackground(false);
        Employees_m_detailId->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailId->setLineWidth(1);
        Employees_m_detailId->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailId->setWordWrap(false);
        Employees_m_detailId->setGeometry(QRect(18, 45, 604, 16));
        Employees_m_detailId->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #7A8B8F; background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_m_detailId->sizePolicy().hasHeightForWidth());
        Employees_m_detailId->setSizePolicy(sizePolicy);
        Employees_m_detailId->setMidLineWidth(0);
        Employees_m_detailId->setFont(font);

        Employees_control_73->addWidget(Employees_m_detailId);

        Employees_m_detailStatut = new QLabel(Employees_m_detailContent);
        Employees_m_detailStatut->setObjectName("Employees_m_detailStatut");
        Employees_m_detailStatut->setMinimumSize(QSize(0, 0));
        Employees_m_detailStatut->setAutoFillBackground(false);
        Employees_m_detailStatut->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailStatut->setLineWidth(1);
        Employees_m_detailStatut->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailStatut->setWordWrap(false);
        Employees_m_detailStatut->setGeometry(QRect(18, 71, 604, 16));
        sizePolicy.setHeightForWidth(Employees_m_detailStatut->sizePolicy().hasHeightForWidth());
        Employees_m_detailStatut->setSizePolicy(sizePolicy);
        Employees_m_detailStatut->setMidLineWidth(0);
        Employees_m_detailStatut->setFont(font);

        Employees_control_73->addWidget(Employees_m_detailStatut);

        spacerItem15 = new QSpacerItem(0, 8, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        Employees_control_73->addItem(spacerItem15);

        Employees_control_81 = new QFrame(Employees_m_detailContent);
        Employees_control_81->setObjectName("Employees_control_81");
        Employees_control_81->setMinimumSize(QSize(0, 0));
        Employees_control_81->setAutoFillBackground(false);
        Employees_control_81->setMaximumSize(QSize(16777215, 1));
        Employees_control_81->setLineWidth(1);
        Employees_control_81->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_81->setGeometry(QRect(18, 105, 604, 1));
        Employees_control_81->setStyleSheet(QString::fromUtf8("background-color: #E4F0F3; max-height: 1px; border: none;"));
        sizePolicy1.setHeightForWidth(Employees_control_81->sizePolicy().hasHeightForWidth());
        Employees_control_81->setSizePolicy(sizePolicy1);
        Employees_control_81->setMidLineWidth(0);
        Employees_control_81->setFont(font);

        Employees_control_73->addWidget(Employees_control_81);

        spacerItem16 = new QSpacerItem(0, 6, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        Employees_control_73->addItem(spacerItem16);

        Employees_control_74 = new QHBoxLayout();
        Employees_control_74->setSpacing(8);
        Employees_control_74->setObjectName("Employees_control_74");
        Employees_control_74->setContentsMargins(0, 0, 0, 0);
        Employees_control_82 = new QLabel(Employees_m_detailContent);
        Employees_control_82->setObjectName("Employees_control_82");
        Employees_control_82->setMinimumSize(QSize(95, 0));
        Employees_control_82->setAutoFillBackground(false);
        Employees_control_82->setMaximumSize(QSize(95, 16777215));
        Employees_control_82->setLineWidth(1);
        Employees_control_82->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_82->setWordWrap(false);
        Employees_control_82->setGeometry(QRect(18, 122, 95, 16));
        Employees_control_82->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #7A8B8F; background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_control_82->sizePolicy().hasHeightForWidth());
        Employees_control_82->setSizePolicy(sizePolicy);
        Employees_control_82->setMidLineWidth(0);
        Employees_control_82->setFont(font);

        Employees_control_74->addWidget(Employees_control_82);

        Employees_m_detailPoste = new QLabel(Employees_m_detailContent);
        Employees_m_detailPoste->setObjectName("Employees_m_detailPoste");
        Employees_m_detailPoste->setMinimumSize(QSize(0, 0));
        Employees_m_detailPoste->setAutoFillBackground(false);
        Employees_m_detailPoste->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailPoste->setLineWidth(1);
        Employees_m_detailPoste->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailPoste->setWordWrap(true);
        Employees_m_detailPoste->setGeometry(QRect(121, 122, 501, 16));
        Employees_m_detailPoste->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #263B42; font-weight: 500;background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_m_detailPoste->sizePolicy().hasHeightForWidth());
        Employees_m_detailPoste->setSizePolicy(sizePolicy);
        Employees_m_detailPoste->setMidLineWidth(0);
        QFont font5;
        font5.setFamilies({QString::fromUtf8("Segoe UI")});
        font5.setWeight(QFont::Medium);
        Employees_m_detailPoste->setFont(font5);

        Employees_control_74->addWidget(Employees_m_detailPoste);

        Employees_control_74->setStretch(1, 1);

        Employees_control_73->addLayout(Employees_control_74);

        Employees_control_75 = new QHBoxLayout();
        Employees_control_75->setSpacing(8);
        Employees_control_75->setObjectName("Employees_control_75");
        Employees_control_75->setContentsMargins(0, 0, 0, 0);
        Employees_control_83 = new QLabel(Employees_m_detailContent);
        Employees_control_83->setObjectName("Employees_control_83");
        Employees_control_83->setMinimumSize(QSize(95, 0));
        Employees_control_83->setAutoFillBackground(false);
        Employees_control_83->setMaximumSize(QSize(95, 16777215));
        Employees_control_83->setLineWidth(1);
        Employees_control_83->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_83->setWordWrap(false);
        Employees_control_83->setGeometry(QRect(18, 148, 95, 16));
        Employees_control_83->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #7A8B8F; background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_control_83->sizePolicy().hasHeightForWidth());
        Employees_control_83->setSizePolicy(sizePolicy);
        Employees_control_83->setMidLineWidth(0);
        Employees_control_83->setFont(font);

        Employees_control_75->addWidget(Employees_control_83);

        Employees_m_detailCourriel = new QLabel(Employees_m_detailContent);
        Employees_m_detailCourriel->setObjectName("Employees_m_detailCourriel");
        Employees_m_detailCourriel->setMinimumSize(QSize(0, 0));
        Employees_m_detailCourriel->setAutoFillBackground(false);
        Employees_m_detailCourriel->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailCourriel->setLineWidth(1);
        Employees_m_detailCourriel->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailCourriel->setWordWrap(true);
        Employees_m_detailCourriel->setGeometry(QRect(121, 148, 501, 16));
        Employees_m_detailCourriel->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #263B42; font-weight: 500;background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_m_detailCourriel->sizePolicy().hasHeightForWidth());
        Employees_m_detailCourriel->setSizePolicy(sizePolicy);
        Employees_m_detailCourriel->setMidLineWidth(0);
        Employees_m_detailCourriel->setFont(font5);

        Employees_control_75->addWidget(Employees_m_detailCourriel);

        Employees_control_75->setStretch(1, 1);

        Employees_control_73->addLayout(Employees_control_75);

        Employees_control_76 = new QHBoxLayout();
        Employees_control_76->setSpacing(8);
        Employees_control_76->setObjectName("Employees_control_76");
        Employees_control_76->setContentsMargins(0, 0, 0, 0);
        Employees_control_84 = new QLabel(Employees_m_detailContent);
        Employees_control_84->setObjectName("Employees_control_84");
        Employees_control_84->setMinimumSize(QSize(95, 0));
        Employees_control_84->setAutoFillBackground(false);
        Employees_control_84->setMaximumSize(QSize(95, 16777215));
        Employees_control_84->setLineWidth(1);
        Employees_control_84->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_84->setWordWrap(false);
        Employees_control_84->setGeometry(QRect(18, 174, 95, 16));
        Employees_control_84->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #7A8B8F; background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_control_84->sizePolicy().hasHeightForWidth());
        Employees_control_84->setSizePolicy(sizePolicy);
        Employees_control_84->setMidLineWidth(0);
        Employees_control_84->setFont(font);

        Employees_control_76->addWidget(Employees_control_84);

        Employees_m_detailTelephone = new QLabel(Employees_m_detailContent);
        Employees_m_detailTelephone->setObjectName("Employees_m_detailTelephone");
        Employees_m_detailTelephone->setMinimumSize(QSize(0, 0));
        Employees_m_detailTelephone->setAutoFillBackground(false);
        Employees_m_detailTelephone->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailTelephone->setLineWidth(1);
        Employees_m_detailTelephone->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailTelephone->setWordWrap(true);
        Employees_m_detailTelephone->setGeometry(QRect(121, 174, 501, 16));
        Employees_m_detailTelephone->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #263B42; font-weight: 500;background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_m_detailTelephone->sizePolicy().hasHeightForWidth());
        Employees_m_detailTelephone->setSizePolicy(sizePolicy);
        Employees_m_detailTelephone->setMidLineWidth(0);
        Employees_m_detailTelephone->setFont(font5);

        Employees_control_76->addWidget(Employees_m_detailTelephone);

        Employees_control_76->setStretch(1, 1);

        Employees_control_73->addLayout(Employees_control_76);

        Employees_control_77 = new QHBoxLayout();
        Employees_control_77->setSpacing(8);
        Employees_control_77->setObjectName("Employees_control_77");
        Employees_control_77->setContentsMargins(0, 0, 0, 0);
        Employees_control_85 = new QLabel(Employees_m_detailContent);
        Employees_control_85->setObjectName("Employees_control_85");
        Employees_control_85->setMinimumSize(QSize(95, 0));
        Employees_control_85->setAutoFillBackground(false);
        Employees_control_85->setMaximumSize(QSize(95, 16777215));
        Employees_control_85->setLineWidth(1);
        Employees_control_85->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_85->setWordWrap(false);
        Employees_control_85->setGeometry(QRect(18, 200, 95, 16));
        Employees_control_85->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #7A8B8F; background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_control_85->sizePolicy().hasHeightForWidth());
        Employees_control_85->setSizePolicy(sizePolicy);
        Employees_control_85->setMidLineWidth(0);
        Employees_control_85->setFont(font);

        Employees_control_77->addWidget(Employees_control_85);

        Employees_m_detailDate = new QLabel(Employees_m_detailContent);
        Employees_m_detailDate->setObjectName("Employees_m_detailDate");
        Employees_m_detailDate->setMinimumSize(QSize(0, 0));
        Employees_m_detailDate->setAutoFillBackground(false);
        Employees_m_detailDate->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailDate->setLineWidth(1);
        Employees_m_detailDate->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailDate->setWordWrap(true);
        Employees_m_detailDate->setGeometry(QRect(121, 200, 501, 16));
        Employees_m_detailDate->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #263B42; font-weight: 500;background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_m_detailDate->sizePolicy().hasHeightForWidth());
        Employees_m_detailDate->setSizePolicy(sizePolicy);
        Employees_m_detailDate->setMidLineWidth(0);
        Employees_m_detailDate->setFont(font5);

        Employees_control_77->addWidget(Employees_m_detailDate);

        Employees_control_77->setStretch(1, 1);

        Employees_control_73->addLayout(Employees_control_77);

        Employees_control_78 = new QHBoxLayout();
        Employees_control_78->setSpacing(8);
        Employees_control_78->setObjectName("Employees_control_78");
        Employees_control_78->setContentsMargins(0, 0, 0, 0);
        Employees_control_86 = new QLabel(Employees_m_detailContent);
        Employees_control_86->setObjectName("Employees_control_86");
        Employees_control_86->setMinimumSize(QSize(95, 0));
        Employees_control_86->setAutoFillBackground(false);
        Employees_control_86->setMaximumSize(QSize(95, 16777215));
        Employees_control_86->setLineWidth(1);
        Employees_control_86->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_86->setWordWrap(false);
        Employees_control_86->setGeometry(QRect(18, 226, 95, 16));
        Employees_control_86->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #7A8B8F; background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_control_86->sizePolicy().hasHeightForWidth());
        Employees_control_86->setSizePolicy(sizePolicy);
        Employees_control_86->setMidLineWidth(0);
        Employees_control_86->setFont(font);

        Employees_control_78->addWidget(Employees_control_86);

        Employees_m_detailCharge = new QLabel(Employees_m_detailContent);
        Employees_m_detailCharge->setObjectName("Employees_m_detailCharge");
        Employees_m_detailCharge->setMinimumSize(QSize(0, 0));
        Employees_m_detailCharge->setAutoFillBackground(false);
        Employees_m_detailCharge->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailCharge->setLineWidth(1);
        Employees_m_detailCharge->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailCharge->setWordWrap(true);
        Employees_m_detailCharge->setGeometry(QRect(121, 226, 501, 16));
        Employees_m_detailCharge->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #263B42; font-weight: 500;background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_m_detailCharge->sizePolicy().hasHeightForWidth());
        Employees_m_detailCharge->setSizePolicy(sizePolicy);
        Employees_m_detailCharge->setMidLineWidth(0);
        Employees_m_detailCharge->setFont(font5);

        Employees_control_78->addWidget(Employees_m_detailCharge);

        Employees_control_78->setStretch(1, 1);

        Employees_control_73->addLayout(Employees_control_78);

        Employees_control_79 = new QHBoxLayout();
        Employees_control_79->setSpacing(8);
        Employees_control_79->setObjectName("Employees_control_79");
        Employees_control_79->setContentsMargins(0, 0, 0, 0);
        Employees_control_87 = new QLabel(Employees_m_detailContent);
        Employees_control_87->setObjectName("Employees_control_87");
        Employees_control_87->setMinimumSize(QSize(95, 0));
        Employees_control_87->setAutoFillBackground(false);
        Employees_control_87->setMaximumSize(QSize(95, 16777215));
        Employees_control_87->setLineWidth(1);
        Employees_control_87->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_87->setWordWrap(false);
        Employees_control_87->setGeometry(QRect(18, 252, 95, 16));
        Employees_control_87->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #7A8B8F; background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_control_87->sizePolicy().hasHeightForWidth());
        Employees_control_87->setSizePolicy(sizePolicy);
        Employees_control_87->setMidLineWidth(0);
        Employees_control_87->setFont(font);

        Employees_control_79->addWidget(Employees_control_87);

        Employees_m_detailDispo = new QLabel(Employees_m_detailContent);
        Employees_m_detailDispo->setObjectName("Employees_m_detailDispo");
        Employees_m_detailDispo->setMinimumSize(QSize(0, 0));
        Employees_m_detailDispo->setAutoFillBackground(false);
        Employees_m_detailDispo->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailDispo->setLineWidth(1);
        Employees_m_detailDispo->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailDispo->setWordWrap(true);
        Employees_m_detailDispo->setGeometry(QRect(121, 252, 501, 16));
        Employees_m_detailDispo->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #263B42; font-weight: 500;background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_m_detailDispo->sizePolicy().hasHeightForWidth());
        Employees_m_detailDispo->setSizePolicy(sizePolicy);
        Employees_m_detailDispo->setMidLineWidth(0);
        Employees_m_detailDispo->setFont(font5);

        Employees_control_79->addWidget(Employees_m_detailDispo);

        Employees_control_79->setStretch(1, 1);

        Employees_control_73->addLayout(Employees_control_79);

        Employees_control_80 = new QHBoxLayout();
        Employees_control_80->setSpacing(8);
        Employees_control_80->setObjectName("Employees_control_80");
        Employees_control_80->setContentsMargins(0, 0, 0, 0);
        Employees_control_88 = new QLabel(Employees_m_detailContent);
        Employees_control_88->setObjectName("Employees_control_88");
        Employees_control_88->setMinimumSize(QSize(95, 0));
        Employees_control_88->setAutoFillBackground(false);
        Employees_control_88->setMaximumSize(QSize(95, 16777215));
        Employees_control_88->setLineWidth(1);
        Employees_control_88->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_control_88->setWordWrap(false);
        Employees_control_88->setGeometry(QRect(18, 278, 95, 16));
        Employees_control_88->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #7A8B8F; background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_control_88->sizePolicy().hasHeightForWidth());
        Employees_control_88->setSizePolicy(sizePolicy);
        Employees_control_88->setMidLineWidth(0);
        Employees_control_88->setFont(font);

        Employees_control_80->addWidget(Employees_control_88);

        Employees_m_detailCompetences = new QLabel(Employees_m_detailContent);
        Employees_m_detailCompetences->setObjectName("Employees_m_detailCompetences");
        Employees_m_detailCompetences->setMinimumSize(QSize(0, 0));
        Employees_m_detailCompetences->setAutoFillBackground(false);
        Employees_m_detailCompetences->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailCompetences->setLineWidth(1);
        Employees_m_detailCompetences->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailCompetences->setWordWrap(true);
        Employees_m_detailCompetences->setGeometry(QRect(121, 278, 501, 16));
        Employees_m_detailCompetences->setStyleSheet(QString::fromUtf8("font-size: 12px; color: #263B42; font-weight: 500;background: transparent;"));
        sizePolicy.setHeightForWidth(Employees_m_detailCompetences->sizePolicy().hasHeightForWidth());
        Employees_m_detailCompetences->setSizePolicy(sizePolicy);
        Employees_m_detailCompetences->setMidLineWidth(0);
        Employees_m_detailCompetences->setFont(font5);

        Employees_control_80->addWidget(Employees_m_detailCompetences);

        Employees_control_80->setStretch(1, 1);

        Employees_control_73->addLayout(Employees_control_80);

        spacerItem17 = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        Employees_control_73->addItem(spacerItem17);

        Employees_control_71->setWidget(Employees_m_detailContent);

        Employees_control_69->addWidget(Employees_control_71);

        Employees_m_detailActions = new QWidget(employeeDetailPanel);
        Employees_m_detailActions->setObjectName("Employees_m_detailActions");
        Employees_m_detailActions->setAutoFillBackground(false);
        Employees_m_detailActions->setMinimumSize(QSize(0, 0));
        Employees_m_detailActions->setMaximumSize(QSize(16777215, 16777215));
        Employees_m_detailActions->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Employees_m_detailActions->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Employees_m_detailActions->sizePolicy().hasHeightForWidth());
        Employees_m_detailActions->setSizePolicy(sizePolicy);
        Employees_m_detailActions->setFont(font3);
        Employees_control_95 = new QVBoxLayout(Employees_m_detailActions);
        Employees_control_95->setSpacing(8);
        Employees_control_95->setObjectName("Employees_control_95");
        Employees_control_95->setContentsMargins(18, 8, 18, 18);
        editEmployee = new QPushButton(Employees_m_detailActions);
        editEmployee->setObjectName("editEmployee");
        editEmployee->setMinimumSize(QSize(0, 34));
        editEmployee->setChecked(false);
        editEmployee->setIconSize(QSize(17, 17));
        editEmployee->setAutoFillBackground(false);
        editEmployee->setAutoDefault(false);
        editEmployee->setMaximumSize(QSize(16777215, 16777215));
        editEmployee->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        editEmployee->setFlat(false);
        editEmployee->setCheckable(false);
        editEmployee->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(editEmployee->sizePolicy().hasHeightForWidth());
        editEmployee->setSizePolicy(sizePolicy1);
        editEmployee->setFont(font3);
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/designer/editEmployee.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        editEmployee->setIcon(icon15);

        Employees_control_95->addWidget(editEmployee);

        toggleEmployeeAvailability = new QPushButton(Employees_m_detailActions);
        toggleEmployeeAvailability->setObjectName("toggleEmployeeAvailability");
        toggleEmployeeAvailability->setMinimumSize(QSize(0, 34));
        toggleEmployeeAvailability->setChecked(false);
        toggleEmployeeAvailability->setIconSize(QSize(17, 17));
        toggleEmployeeAvailability->setAutoFillBackground(false);
        toggleEmployeeAvailability->setAutoDefault(false);
        toggleEmployeeAvailability->setMaximumSize(QSize(16777215, 16777215));
        toggleEmployeeAvailability->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        toggleEmployeeAvailability->setFlat(false);
        toggleEmployeeAvailability->setCheckable(false);
        toggleEmployeeAvailability->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(toggleEmployeeAvailability->sizePolicy().hasHeightForWidth());
        toggleEmployeeAvailability->setSizePolicy(sizePolicy1);
        toggleEmployeeAvailability->setFont(font3);
        QIcon icon16;
        icon16.addFile(QString::fromUtf8(":/designer/toggleEmployeeAvailability.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        toggleEmployeeAvailability->setIcon(icon16);

        Employees_control_95->addWidget(toggleEmployeeAvailability);

        deleteEmployee = new QPushButton(Employees_m_detailActions);
        deleteEmployee->setObjectName("deleteEmployee");
        deleteEmployee->setMinimumSize(QSize(0, 34));
        deleteEmployee->setChecked(false);
        deleteEmployee->setIconSize(QSize(17, 17));
        deleteEmployee->setAutoFillBackground(false);
        deleteEmployee->setAutoDefault(false);
        deleteEmployee->setMaximumSize(QSize(16777215, 16777215));
        deleteEmployee->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        deleteEmployee->setFlat(false);
        deleteEmployee->setCheckable(false);
        deleteEmployee->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(deleteEmployee->sizePolicy().hasHeightForWidth());
        deleteEmployee->setSizePolicy(sizePolicy1);
        deleteEmployee->setFont(font3);
        QIcon icon17;
        icon17.addFile(QString::fromUtf8(":/designer/deleteEmployee.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        deleteEmployee->setIcon(icon17);

        Employees_control_95->addWidget(deleteEmployee);


        Employees_control_69->addWidget(Employees_m_detailActions);

        Employees_control_69->setStretch(2, 1);

        Employees_control_2->addWidget(employeeDetailPanel);

        Employees_control_2->setStretch(0, 3);
        Employees_control_2->setStretch(1, 1);

        Employees_control_1->addLayout(Employees_control_2);

        Employees_control_1->setStretch(3, 1);
        pageStack->addWidget(EmployeesPage);
        LocauxPage = new QWidget();
        LocauxPage->setObjectName("LocauxPage");
        LocauxPage->setAutoFillBackground(false);
        LocauxPage->setMinimumSize(QSize(0, 0));
        LocauxPage->setMaximumSize(QSize(16777215, 16777215));
        LocauxPage->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        LocauxPage->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(LocauxPage->sizePolicy().hasHeightForWidth());
        LocauxPage->setSizePolicy(sizePolicy);
        LocauxPage->setFont(font3);
        Locaux_control_1 = new QVBoxLayout(LocauxPage);
        Locaux_control_1->setSpacing(14);
        Locaux_control_1->setObjectName("Locaux_control_1");
        Locaux_control_1->setContentsMargins(26, 22, 26, 10);
        Locaux_control_2 = new QVBoxLayout();
        Locaux_control_2->setSpacing(10);
        Locaux_control_2->setObjectName("Locaux_control_2");
        Locaux_control_2->setContentsMargins(0, 0, 0, 0);
        Locaux_control_3 = new QVBoxLayout();
        Locaux_control_3->setSpacing(4);
        Locaux_control_3->setObjectName("Locaux_control_3");
        Locaux_control_3->setContentsMargins(0, 0, 0, 0);
        Locaux_control_4 = new QHBoxLayout();
        Locaux_control_4->setSpacing(10);
        Locaux_control_4->setObjectName("Locaux_control_4");
        Locaux_control_4->setContentsMargins(0, 0, 0, 0);
        Locaux_control_13 = new QLabel(LocauxPage);
        Locaux_control_13->setObjectName("Locaux_control_13");
        Locaux_control_13->setMinimumSize(QSize(0, 0));
        Locaux_control_13->setAutoFillBackground(false);
        Locaux_control_13->setMaximumSize(QSize(16777215, 16777215));
        Locaux_control_13->setLineWidth(1);
        Locaux_control_13->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_control_13->setWordWrap(false);
        Locaux_control_13->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Locaux_control_13->sizePolicy().hasHeightForWidth());
        Locaux_control_13->setSizePolicy(sizePolicy);
        Locaux_control_13->setMidLineWidth(0);
        Locaux_control_13->setFont(font3);
        Locaux_control_13->setPixmap(QPixmap(QString::fromUtf8(":/designer/Locaux_control_13.png")));

        Locaux_control_4->addWidget(Locaux_control_13);

        h1 = new QLabel(LocauxPage);
        h1->setObjectName("h1");
        h1->setMinimumSize(QSize(0, 0));
        h1->setAutoFillBackground(false);
        h1->setMaximumSize(QSize(16777215, 16777215));
        h1->setLineWidth(1);
        h1->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        h1->setWordWrap(false);
        h1->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(h1->sizePolicy().hasHeightForWidth());
        h1->setSizePolicy(sizePolicy);
        h1->setMidLineWidth(0);
        h1->setFont(font3);

        Locaux_control_4->addWidget(h1);

        spacerItem18 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Locaux_control_4->addItem(spacerItem18);

        Locaux_m_btnAdd = new QPushButton(LocauxPage);
        Locaux_m_btnAdd->setObjectName("Locaux_m_btnAdd");
        Locaux_m_btnAdd->setMinimumSize(QSize(0, 0));
        Locaux_m_btnAdd->setChecked(false);
        Locaux_m_btnAdd->setIconSize(QSize(16, 16));
        Locaux_m_btnAdd->setAutoFillBackground(false);
        Locaux_m_btnAdd->setAutoDefault(false);
        Locaux_m_btnAdd->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_btnAdd->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_btnAdd->setFlat(false);
        Locaux_m_btnAdd->setCheckable(false);
        Locaux_m_btnAdd->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_btnAdd->sizePolicy().hasHeightForWidth());
        Locaux_m_btnAdd->setSizePolicy(sizePolicy1);
        Locaux_m_btnAdd->setFont(font3);
        QIcon icon18;
        icon18.addFile(QString::fromUtf8(":/designer/Locaux_m_btnAdd.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Locaux_m_btnAdd->setIcon(icon18);

        Locaux_control_4->addWidget(Locaux_m_btnAdd);


        Locaux_control_3->addLayout(Locaux_control_4);

        sub = new QLabel(LocauxPage);
        sub->setObjectName("sub");
        sub->setMinimumSize(QSize(0, 0));
        sub->setAutoFillBackground(false);
        sub->setMaximumSize(QSize(16777215, 16777215));
        sub->setLineWidth(1);
        sub->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        sub->setWordWrap(false);
        sub->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(sub->sizePolicy().hasHeightForWidth());
        sub->setSizePolicy(sizePolicy);
        sub->setMidLineWidth(0);
        sub->setFont(font3);

        Locaux_control_3->addWidget(sub);

        spacerItem19 = new QSpacerItem(0, 6, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        Locaux_control_3->addItem(spacerItem19);

        Locaux_control_5 = new QHBoxLayout();
        Locaux_control_5->setSpacing(10);
        Locaux_control_5->setObjectName("Locaux_control_5");
        Locaux_control_5->setContentsMargins(0, 0, 0, 0);
        Locaux_m_chipAvail = new QPushButton(LocauxPage);
        Locaux_m_chipAvail->setObjectName("Locaux_m_chipAvail");
        Locaux_m_chipAvail->setMinimumSize(QSize(0, 0));
        Locaux_m_chipAvail->setChecked(false);
        Locaux_m_chipAvail->setIconSize(QSize(16, 16));
        Locaux_m_chipAvail->setAutoFillBackground(false);
        Locaux_m_chipAvail->setAutoDefault(false);
        Locaux_m_chipAvail->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_chipAvail->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_chipAvail->setFlat(false);
        Locaux_m_chipAvail->setCheckable(false);
        Locaux_m_chipAvail->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_chipAvail->sizePolicy().hasHeightForWidth());
        Locaux_m_chipAvail->setSizePolicy(sizePolicy1);
        Locaux_m_chipAvail->setFont(font3);
        Locaux_m_chipAvail->setProperty("chip", QVariant(true));

        Locaux_control_5->addWidget(Locaux_m_chipAvail);

        Locaux_m_chipOcc = new QPushButton(LocauxPage);
        Locaux_m_chipOcc->setObjectName("Locaux_m_chipOcc");
        Locaux_m_chipOcc->setMinimumSize(QSize(0, 0));
        Locaux_m_chipOcc->setChecked(false);
        Locaux_m_chipOcc->setIconSize(QSize(16, 16));
        Locaux_m_chipOcc->setAutoFillBackground(false);
        Locaux_m_chipOcc->setAutoDefault(false);
        Locaux_m_chipOcc->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_chipOcc->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_chipOcc->setFlat(false);
        Locaux_m_chipOcc->setCheckable(false);
        Locaux_m_chipOcc->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_chipOcc->sizePolicy().hasHeightForWidth());
        Locaux_m_chipOcc->setSizePolicy(sizePolicy1);
        Locaux_m_chipOcc->setFont(font3);
        Locaux_m_chipOcc->setProperty("chip", QVariant(true));

        Locaux_control_5->addWidget(Locaux_m_chipOcc);

        Locaux_m_chipOos = new QPushButton(LocauxPage);
        Locaux_m_chipOos->setObjectName("Locaux_m_chipOos");
        Locaux_m_chipOos->setMinimumSize(QSize(0, 0));
        Locaux_m_chipOos->setChecked(false);
        Locaux_m_chipOos->setIconSize(QSize(16, 16));
        Locaux_m_chipOos->setAutoFillBackground(false);
        Locaux_m_chipOos->setAutoDefault(false);
        Locaux_m_chipOos->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_chipOos->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_chipOos->setFlat(false);
        Locaux_m_chipOos->setCheckable(false);
        Locaux_m_chipOos->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_chipOos->sizePolicy().hasHeightForWidth());
        Locaux_m_chipOos->setSizePolicy(sizePolicy1);
        Locaux_m_chipOos->setFont(font3);
        Locaux_m_chipOos->setProperty("chip", QVariant(true));

        Locaux_control_5->addWidget(Locaux_m_chipOos);

        Locaux_m_chipDirty = new QPushButton(LocauxPage);
        Locaux_m_chipDirty->setObjectName("Locaux_m_chipDirty");
        Locaux_m_chipDirty->setMinimumSize(QSize(0, 0));
        Locaux_m_chipDirty->setChecked(false);
        Locaux_m_chipDirty->setIconSize(QSize(16, 16));
        Locaux_m_chipDirty->setAutoFillBackground(false);
        Locaux_m_chipDirty->setAutoDefault(false);
        Locaux_m_chipDirty->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_chipDirty->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_chipDirty->setFlat(false);
        Locaux_m_chipDirty->setCheckable(false);
        Locaux_m_chipDirty->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_chipDirty->sizePolicy().hasHeightForWidth());
        Locaux_m_chipDirty->setSizePolicy(sizePolicy1);
        Locaux_m_chipDirty->setFont(font3);
        Locaux_m_chipDirty->setProperty("chip", QVariant(true));

        Locaux_control_5->addWidget(Locaux_m_chipDirty);

        spacerItem20 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Locaux_control_5->addItem(spacerItem20);


        Locaux_control_3->addLayout(Locaux_control_5);


        Locaux_control_2->addLayout(Locaux_control_3);

        Locaux_control_6 = new QVBoxLayout();
        Locaux_control_6->setSpacing(10);
        Locaux_control_6->setObjectName("Locaux_control_6");
        Locaux_control_6->setContentsMargins(0, 0, 0, 0);
        Locaux_control_7 = new QHBoxLayout();
        Locaux_control_7->setSpacing(10);
        Locaux_control_7->setObjectName("Locaux_control_7");
        Locaux_control_7->setContentsMargins(0, 0, 0, 0);
        spacerItem21 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Locaux_control_7->addItem(spacerItem21);

        Locaux_m_btnAlerts = new QPushButton(LocauxPage);
        Locaux_m_btnAlerts->setObjectName("Locaux_m_btnAlerts");
        Locaux_m_btnAlerts->setMinimumSize(QSize(0, 0));
        Locaux_m_btnAlerts->setChecked(false);
        Locaux_m_btnAlerts->setIconSize(QSize(16, 16));
        Locaux_m_btnAlerts->setAutoFillBackground(false);
        Locaux_m_btnAlerts->setAutoDefault(false);
        Locaux_m_btnAlerts->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_btnAlerts->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_btnAlerts->setFlat(false);
        Locaux_m_btnAlerts->setCheckable(false);
        Locaux_m_btnAlerts->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_btnAlerts->sizePolicy().hasHeightForWidth());
        Locaux_m_btnAlerts->setSizePolicy(sizePolicy1);
        Locaux_m_btnAlerts->setFont(font1);
        QIcon icon19;
        icon19.addFile(QString::fromUtf8(":/designer/Locaux_m_btnAlerts.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Locaux_m_btnAlerts->setIcon(icon19);

        Locaux_control_7->addWidget(Locaux_m_btnAlerts);

        Locaux_m_btnSmart = new QPushButton(LocauxPage);
        Locaux_m_btnSmart->setObjectName("Locaux_m_btnSmart");
        Locaux_m_btnSmart->setMinimumSize(QSize(0, 0));
        Locaux_m_btnSmart->setChecked(false);
        Locaux_m_btnSmart->setIconSize(QSize(16, 16));
        Locaux_m_btnSmart->setAutoFillBackground(false);
        Locaux_m_btnSmart->setAutoDefault(false);
        Locaux_m_btnSmart->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_btnSmart->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_btnSmart->setFlat(false);
        Locaux_m_btnSmart->setCheckable(false);
        Locaux_m_btnSmart->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_btnSmart->sizePolicy().hasHeightForWidth());
        Locaux_m_btnSmart->setSizePolicy(sizePolicy1);
        Locaux_m_btnSmart->setFont(font3);
        QIcon icon20;
        icon20.addFile(QString::fromUtf8(":/designer/Locaux_m_btnSmart.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Locaux_m_btnSmart->setIcon(icon20);

        Locaux_control_7->addWidget(Locaux_m_btnSmart);

        Locaux_control_8 = new QHBoxLayout();
        Locaux_control_8->setSpacing(10);
        Locaux_control_8->setObjectName("Locaux_control_8");
        Locaux_control_8->setContentsMargins(0, 0, 0, 0);
        Locaux_m_btnPdf = new QPushButton(LocauxPage);
        Locaux_m_btnPdf->setObjectName("Locaux_m_btnPdf");
        Locaux_m_btnPdf->setMinimumSize(QSize(0, 0));
        Locaux_m_btnPdf->setChecked(false);
        Locaux_m_btnPdf->setIconSize(QSize(16, 16));
        Locaux_m_btnPdf->setAutoFillBackground(false);
        Locaux_m_btnPdf->setAutoDefault(false);
        Locaux_m_btnPdf->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_btnPdf->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_btnPdf->setFlat(false);
        Locaux_m_btnPdf->setCheckable(false);
        Locaux_m_btnPdf->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_btnPdf->sizePolicy().hasHeightForWidth());
        Locaux_m_btnPdf->setSizePolicy(sizePolicy1);
        Locaux_m_btnPdf->setFont(font3);
        QIcon icon21;
        icon21.addFile(QString::fromUtf8(":/designer/Locaux_m_btnPdf.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Locaux_m_btnPdf->setIcon(icon21);

        Locaux_control_8->addWidget(Locaux_m_btnPdf);

        Locaux_m_btnMap = new QPushButton(LocauxPage);
        Locaux_m_btnMap->setObjectName("Locaux_m_btnMap");
        Locaux_m_btnMap->setMinimumSize(QSize(0, 0));
        Locaux_m_btnMap->setChecked(false);
        Locaux_m_btnMap->setIconSize(QSize(16, 16));
        Locaux_m_btnMap->setAutoFillBackground(false);
        Locaux_m_btnMap->setAutoDefault(false);
        Locaux_m_btnMap->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_btnMap->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_btnMap->setFlat(false);
        Locaux_m_btnMap->setCheckable(false);
        Locaux_m_btnMap->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_btnMap->sizePolicy().hasHeightForWidth());
        Locaux_m_btnMap->setSizePolicy(sizePolicy1);
        Locaux_m_btnMap->setFont(font3);
        QIcon icon22;
        icon22.addFile(QString::fromUtf8(":/designer/Locaux_m_btnMap.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Locaux_m_btnMap->setIcon(icon22);

        Locaux_control_8->addWidget(Locaux_m_btnMap);

        Locaux_m_btnStats = new QPushButton(LocauxPage);
        Locaux_m_btnStats->setObjectName("Locaux_m_btnStats");
        Locaux_m_btnStats->setMinimumSize(QSize(0, 0));
        Locaux_m_btnStats->setChecked(false);
        Locaux_m_btnStats->setIconSize(QSize(16, 16));
        Locaux_m_btnStats->setAutoFillBackground(false);
        Locaux_m_btnStats->setAutoDefault(false);
        Locaux_m_btnStats->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_btnStats->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_btnStats->setFlat(false);
        Locaux_m_btnStats->setCheckable(false);
        Locaux_m_btnStats->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_btnStats->sizePolicy().hasHeightForWidth());
        Locaux_m_btnStats->setSizePolicy(sizePolicy1);
        Locaux_m_btnStats->setFont(font3);
        QIcon icon23;
        icon23.addFile(QString::fromUtf8(":/designer/Locaux_m_btnStats.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Locaux_m_btnStats->setIcon(icon23);

        Locaux_control_8->addWidget(Locaux_m_btnStats);


        Locaux_control_7->addLayout(Locaux_control_8);


        Locaux_control_6->addLayout(Locaux_control_7);

        spacerItem22 = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        Locaux_control_6->addItem(spacerItem22);


        Locaux_control_2->addLayout(Locaux_control_6);

        Locaux_control_2->setStretch(0, 1);

        Locaux_control_1->addLayout(Locaux_control_2);

        Locaux_control_9 = new QVBoxLayout();
        Locaux_control_9->setSpacing(6);
        Locaux_control_9->setObjectName("Locaux_control_9");
        Locaux_control_9->setContentsMargins(0, 0, 0, 0);
        Locaux_control_10 = new QHBoxLayout();
        Locaux_control_10->setSpacing(10);
        Locaux_control_10->setObjectName("Locaux_control_10");
        Locaux_control_10->setContentsMargins(0, 0, 0, 0);
        Locaux_m_search = new QLineEdit(LocauxPage);
        Locaux_m_search->setObjectName("Locaux_m_search");
        Locaux_m_search->setMinimumSize(QSize(0, 0));
        Locaux_m_search->setAutoFillBackground(false);
        Locaux_m_search->setMaxLength(32767);
        Locaux_m_search->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_search->setCursor(QCursor(Qt::CursorShape::IBeamCursor));
        Locaux_m_search->setReadOnly(false);
        Locaux_m_search->setClearButtonEnabled(true);
        Locaux_m_search->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy2.setHeightForWidth(Locaux_m_search->sizePolicy().hasHeightForWidth());
        Locaux_m_search->setSizePolicy(sizePolicy2);
        Locaux_m_search->setFont(font3);

        Locaux_control_10->addWidget(Locaux_m_search);

        Locaux_m_sortCombo = new QComboBox(LocauxPage);
        Locaux_m_sortCombo->addItem(QString());
        Locaux_m_sortCombo->addItem(QString());
        Locaux_m_sortCombo->addItem(QString());
        Locaux_m_sortCombo->setObjectName("Locaux_m_sortCombo");
        Locaux_m_sortCombo->setMinimumSize(QSize(0, 0));
        Locaux_m_sortCombo->setMinimumContentsLength(0);
        Locaux_m_sortCombo->setIconSize(QSize(16, 16));
        Locaux_m_sortCombo->setAutoFillBackground(false);
        Locaux_m_sortCombo->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_sortCombo->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_m_sortCombo->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy3.setHeightForWidth(Locaux_m_sortCombo->sizePolicy().hasHeightForWidth());
        Locaux_m_sortCombo->setSizePolicy(sizePolicy3);
        Locaux_m_sortCombo->setEditable(false);
        Locaux_m_sortCombo->setFont(font3);

        Locaux_control_10->addWidget(Locaux_m_sortCombo);

        locauxResetFilters = new QPushButton(LocauxPage);
        locauxResetFilters->setObjectName("locauxResetFilters");
        locauxResetFilters->setMinimumSize(QSize(0, 0));
        locauxResetFilters->setChecked(false);
        locauxResetFilters->setIconSize(QSize(16, 16));
        locauxResetFilters->setAutoFillBackground(false);
        locauxResetFilters->setAutoDefault(false);
        locauxResetFilters->setMaximumSize(QSize(16777215, 16777215));
        locauxResetFilters->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        locauxResetFilters->setFlat(false);
        locauxResetFilters->setCheckable(false);
        locauxResetFilters->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(locauxResetFilters->sizePolicy().hasHeightForWidth());
        locauxResetFilters->setSizePolicy(sizePolicy1);
        locauxResetFilters->setFont(font3);

        Locaux_control_10->addWidget(locauxResetFilters);

        Locaux_m_count = new QLabel(LocauxPage);
        Locaux_m_count->setObjectName("Locaux_m_count");
        Locaux_m_count->setMinimumSize(QSize(0, 0));
        Locaux_m_count->setAutoFillBackground(false);
        Locaux_m_count->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_count->setLineWidth(1);
        Locaux_m_count->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_m_count->setWordWrap(false);
        Locaux_m_count->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Locaux_m_count->sizePolicy().hasHeightForWidth());
        Locaux_m_count->setSizePolicy(sizePolicy);
        Locaux_m_count->setMidLineWidth(0);
        Locaux_m_count->setFont(font3);

        Locaux_control_10->addWidget(Locaux_m_count);

        Locaux_m_btnCards = new QPushButton(LocauxPage);
        Locaux_m_btnCards->setObjectName("Locaux_m_btnCards");
        Locaux_m_btnCards->setMinimumSize(QSize(0, 0));
        Locaux_m_btnCards->setChecked(true);
        Locaux_m_btnCards->setIconSize(QSize(16, 16));
        Locaux_m_btnCards->setAutoFillBackground(false);
        Locaux_m_btnCards->setAutoDefault(false);
        Locaux_m_btnCards->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_btnCards->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_btnCards->setFlat(false);
        Locaux_m_btnCards->setCheckable(true);
        Locaux_m_btnCards->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_btnCards->sizePolicy().hasHeightForWidth());
        Locaux_m_btnCards->setSizePolicy(sizePolicy1);
        Locaux_m_btnCards->setFont(font3);
        QIcon icon24;
        icon24.addFile(QString::fromUtf8(":/designer/Locaux_m_btnCards.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Locaux_m_btnCards->setIcon(icon24);

        Locaux_control_10->addWidget(Locaux_m_btnCards);

        Locaux_m_btnList = new QPushButton(LocauxPage);
        Locaux_m_btnList->setObjectName("Locaux_m_btnList");
        Locaux_m_btnList->setMinimumSize(QSize(0, 0));
        Locaux_m_btnList->setChecked(false);
        Locaux_m_btnList->setIconSize(QSize(16, 16));
        Locaux_m_btnList->setAutoFillBackground(false);
        Locaux_m_btnList->setAutoDefault(false);
        Locaux_m_btnList->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_btnList->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Locaux_m_btnList->setFlat(false);
        Locaux_m_btnList->setCheckable(true);
        Locaux_m_btnList->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(Locaux_m_btnList->sizePolicy().hasHeightForWidth());
        Locaux_m_btnList->setSizePolicy(sizePolicy1);
        Locaux_m_btnList->setFont(font3);
        QIcon icon25;
        icon25.addFile(QString::fromUtf8(":/designer/Locaux_m_btnList.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Locaux_m_btnList->setIcon(icon25);

        Locaux_control_10->addWidget(Locaux_m_btnList);

        Locaux_control_10->setStretch(0, 1);

        Locaux_control_9->addLayout(Locaux_control_10);

        Locaux_m_queryHint = new QLabel(LocauxPage);
        Locaux_m_queryHint->setObjectName("Locaux_m_queryHint");
        Locaux_m_queryHint->setMinimumSize(QSize(0, 0));
        Locaux_m_queryHint->setAutoFillBackground(false);
        Locaux_m_queryHint->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_queryHint->setLineWidth(1);
        Locaux_m_queryHint->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_m_queryHint->setWordWrap(false);
        Locaux_m_queryHint->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Locaux_m_queryHint->sizePolicy().hasHeightForWidth());
        Locaux_m_queryHint->setSizePolicy(sizePolicy);
        Locaux_m_queryHint->setMidLineWidth(0);
        Locaux_m_queryHint->setFont(font3);
        Locaux_m_queryHint->setVisible(false);

        Locaux_control_9->addWidget(Locaux_m_queryHint);


        Locaux_control_1->addLayout(Locaux_control_9);

        Locaux_control_11 = new QHBoxLayout();
        Locaux_control_11->setSpacing(18);
        Locaux_control_11->setObjectName("Locaux_control_11");
        Locaux_control_11->setContentsMargins(0, 0, 0, 0);
        Locaux_control_12 = new QVBoxLayout();
        Locaux_control_12->setSpacing(18);
        Locaux_control_12->setObjectName("Locaux_control_12");
        Locaux_control_12->setContentsMargins(0, 0, 0, 0);
        Locaux_m_views = new QStackedWidget(LocauxPage);
        Locaux_m_views->setObjectName("Locaux_m_views");
        Locaux_m_views->setMinimumSize(QSize(0, 0));
        Locaux_m_views->setAutoFillBackground(false);
        Locaux_m_views->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_views->setLineWidth(1);
        Locaux_m_views->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_m_views->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Locaux_m_views->sizePolicy().hasHeightForWidth());
        Locaux_m_views->setSizePolicy(sizePolicy);
        Locaux_m_views->setMidLineWidth(0);
        Locaux_m_views->setFont(font3);
        Locaux_m_cardScroll = new QScrollArea();
        Locaux_m_cardScroll->setObjectName("Locaux_m_cardScroll");
        Locaux_m_cardScroll->setMinimumSize(QSize(0, 0));
        Locaux_m_cardScroll->setWidgetResizable(true);
        Locaux_m_cardScroll->setAutoFillBackground(false);
        Locaux_m_cardScroll->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_cardScroll->setLineWidth(1);
        Locaux_m_cardScroll->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CursorShape::ArrowCursor)));
        Locaux_m_cardScroll->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy4.setHeightForWidth(Locaux_m_cardScroll->sizePolicy().hasHeightForWidth());
        Locaux_m_cardScroll->setSizePolicy(sizePolicy4);
        Locaux_m_cardScroll->setMidLineWidth(0);
        Locaux_m_cardScroll->setFont(font);
        Locaux_m_grid = new CardGrid();
        Locaux_m_grid->setObjectName("Locaux_m_grid");
        Locaux_m_grid->setAutoFillBackground(true);
        Locaux_m_grid->setMinimumSize(QSize(0, 1274));
        Locaux_m_grid->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_grid->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_m_grid->setGeometry(QRect(0, 0, 640, 1274));
        sizePolicy.setHeightForWidth(Locaux_m_grid->sizePolicy().hasHeightForWidth());
        Locaux_m_grid->setSizePolicy(sizePolicy);
        Locaux_m_grid->setFont(font);
        Locaux_m_cardScroll->setWidget(Locaux_m_grid);
        Locaux_m_views->addWidget(Locaux_m_cardScroll);
        Locaux_m_table = new QTableWidget();
        if (Locaux_m_table->columnCount() < 7)
            Locaux_m_table->setColumnCount(7);
        if (Locaux_m_table->rowCount() < 14)
            Locaux_m_table->setRowCount(14);
        QFont font6;
        font6.setBold(true);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        __qtablewidgetitem->setFont(font6);
        Locaux_m_table->setItem(0, 1, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        __qtablewidgetitem1->setFont(font6);
        Locaux_m_table->setItem(0, 5, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        __qtablewidgetitem2->setFont(font6);
        Locaux_m_table->setItem(0, 6, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        __qtablewidgetitem3->setFont(font6);
        Locaux_m_table->setItem(1, 1, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        __qtablewidgetitem4->setFont(font6);
        Locaux_m_table->setItem(1, 5, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        __qtablewidgetitem5->setFont(font6);
        Locaux_m_table->setItem(1, 6, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        __qtablewidgetitem6->setFont(font6);
        Locaux_m_table->setItem(2, 1, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        __qtablewidgetitem7->setFont(font6);
        Locaux_m_table->setItem(2, 5, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        __qtablewidgetitem8->setFont(font6);
        Locaux_m_table->setItem(2, 6, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        __qtablewidgetitem9->setFont(font6);
        Locaux_m_table->setItem(3, 1, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        __qtablewidgetitem10->setFont(font6);
        Locaux_m_table->setItem(3, 5, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        __qtablewidgetitem11->setFont(font6);
        Locaux_m_table->setItem(3, 6, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        __qtablewidgetitem12->setFont(font6);
        Locaux_m_table->setItem(4, 1, __qtablewidgetitem12);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        __qtablewidgetitem13->setFont(font6);
        Locaux_m_table->setItem(4, 5, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        __qtablewidgetitem14->setFont(font6);
        Locaux_m_table->setItem(4, 6, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        __qtablewidgetitem15->setFont(font6);
        Locaux_m_table->setItem(5, 1, __qtablewidgetitem15);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        __qtablewidgetitem16->setFont(font6);
        Locaux_m_table->setItem(5, 5, __qtablewidgetitem16);
        QTableWidgetItem *__qtablewidgetitem17 = new QTableWidgetItem();
        __qtablewidgetitem17->setFont(font6);
        Locaux_m_table->setItem(5, 6, __qtablewidgetitem17);
        QTableWidgetItem *__qtablewidgetitem18 = new QTableWidgetItem();
        __qtablewidgetitem18->setFont(font6);
        Locaux_m_table->setItem(6, 1, __qtablewidgetitem18);
        QTableWidgetItem *__qtablewidgetitem19 = new QTableWidgetItem();
        __qtablewidgetitem19->setFont(font6);
        Locaux_m_table->setItem(6, 5, __qtablewidgetitem19);
        QTableWidgetItem *__qtablewidgetitem20 = new QTableWidgetItem();
        __qtablewidgetitem20->setFont(font6);
        Locaux_m_table->setItem(6, 6, __qtablewidgetitem20);
        QTableWidgetItem *__qtablewidgetitem21 = new QTableWidgetItem();
        __qtablewidgetitem21->setFont(font6);
        Locaux_m_table->setItem(7, 1, __qtablewidgetitem21);
        QTableWidgetItem *__qtablewidgetitem22 = new QTableWidgetItem();
        __qtablewidgetitem22->setFont(font6);
        Locaux_m_table->setItem(7, 5, __qtablewidgetitem22);
        QTableWidgetItem *__qtablewidgetitem23 = new QTableWidgetItem();
        __qtablewidgetitem23->setFont(font6);
        Locaux_m_table->setItem(7, 6, __qtablewidgetitem23);
        QTableWidgetItem *__qtablewidgetitem24 = new QTableWidgetItem();
        __qtablewidgetitem24->setFont(font6);
        Locaux_m_table->setItem(8, 1, __qtablewidgetitem24);
        QTableWidgetItem *__qtablewidgetitem25 = new QTableWidgetItem();
        __qtablewidgetitem25->setFont(font6);
        Locaux_m_table->setItem(8, 5, __qtablewidgetitem25);
        QTableWidgetItem *__qtablewidgetitem26 = new QTableWidgetItem();
        __qtablewidgetitem26->setFont(font6);
        Locaux_m_table->setItem(8, 6, __qtablewidgetitem26);
        QTableWidgetItem *__qtablewidgetitem27 = new QTableWidgetItem();
        __qtablewidgetitem27->setFont(font6);
        Locaux_m_table->setItem(9, 1, __qtablewidgetitem27);
        QTableWidgetItem *__qtablewidgetitem28 = new QTableWidgetItem();
        __qtablewidgetitem28->setFont(font6);
        Locaux_m_table->setItem(9, 5, __qtablewidgetitem28);
        QTableWidgetItem *__qtablewidgetitem29 = new QTableWidgetItem();
        __qtablewidgetitem29->setFont(font6);
        Locaux_m_table->setItem(9, 6, __qtablewidgetitem29);
        QTableWidgetItem *__qtablewidgetitem30 = new QTableWidgetItem();
        __qtablewidgetitem30->setFont(font6);
        Locaux_m_table->setItem(10, 1, __qtablewidgetitem30);
        QTableWidgetItem *__qtablewidgetitem31 = new QTableWidgetItem();
        __qtablewidgetitem31->setFont(font6);
        Locaux_m_table->setItem(10, 5, __qtablewidgetitem31);
        QTableWidgetItem *__qtablewidgetitem32 = new QTableWidgetItem();
        __qtablewidgetitem32->setFont(font6);
        Locaux_m_table->setItem(10, 6, __qtablewidgetitem32);
        QTableWidgetItem *__qtablewidgetitem33 = new QTableWidgetItem();
        __qtablewidgetitem33->setFont(font6);
        Locaux_m_table->setItem(11, 1, __qtablewidgetitem33);
        QTableWidgetItem *__qtablewidgetitem34 = new QTableWidgetItem();
        __qtablewidgetitem34->setFont(font6);
        Locaux_m_table->setItem(11, 5, __qtablewidgetitem34);
        QTableWidgetItem *__qtablewidgetitem35 = new QTableWidgetItem();
        __qtablewidgetitem35->setFont(font6);
        Locaux_m_table->setItem(11, 6, __qtablewidgetitem35);
        QTableWidgetItem *__qtablewidgetitem36 = new QTableWidgetItem();
        __qtablewidgetitem36->setFont(font6);
        Locaux_m_table->setItem(12, 1, __qtablewidgetitem36);
        QTableWidgetItem *__qtablewidgetitem37 = new QTableWidgetItem();
        __qtablewidgetitem37->setFont(font6);
        Locaux_m_table->setItem(12, 5, __qtablewidgetitem37);
        QTableWidgetItem *__qtablewidgetitem38 = new QTableWidgetItem();
        __qtablewidgetitem38->setFont(font6);
        Locaux_m_table->setItem(12, 6, __qtablewidgetitem38);
        QTableWidgetItem *__qtablewidgetitem39 = new QTableWidgetItem();
        __qtablewidgetitem39->setFont(font6);
        Locaux_m_table->setItem(13, 1, __qtablewidgetitem39);
        QTableWidgetItem *__qtablewidgetitem40 = new QTableWidgetItem();
        __qtablewidgetitem40->setFont(font6);
        Locaux_m_table->setItem(13, 5, __qtablewidgetitem40);
        QTableWidgetItem *__qtablewidgetitem41 = new QTableWidgetItem();
        __qtablewidgetitem41->setFont(font6);
        Locaux_m_table->setItem(13, 6, __qtablewidgetitem41);
        Locaux_m_table->setObjectName("Locaux_m_table");
        Locaux_m_table->setMinimumSize(QSize(0, 0));
        Locaux_m_table->setIconSize(QSize(-1, -1));
        Locaux_m_table->setAutoFillBackground(false);
        Locaux_m_table->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_table->setAlternatingRowColors(false);
        Locaux_m_table->setSortingEnabled(false);
        Locaux_m_table->setLineWidth(1);
        Locaux_m_table->setShowGrid(false);
        Locaux_m_table->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CursorShape::ArrowCursor)));
        Locaux_m_table->setWordWrap(true);
        Locaux_m_table->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy4.setHeightForWidth(Locaux_m_table->sizePolicy().hasHeightForWidth());
        Locaux_m_table->setSizePolicy(sizePolicy4);
        Locaux_m_table->setMidLineWidth(0);
        Locaux_m_table->setFont(font3);
        Locaux_m_table->setVisible(false);
        Locaux_m_views->addWidget(Locaux_m_table);
        Locaux_m_table->horizontalHeader()->setVisible(false);
        Locaux_m_table->horizontalHeader()->setCascadingSectionResizes(false);
        Locaux_m_table->horizontalHeader()->setMinimumSectionSize(28);
        Locaux_m_table->horizontalHeader()->setDefaultSectionSize(100);
        Locaux_m_table->horizontalHeader()->setHighlightSections(false);
        Locaux_m_table->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
        Locaux_m_table->horizontalHeader()->setStretchLastSection(false);
        Locaux_m_table->verticalHeader()->setVisible(false);
        Locaux_m_table->verticalHeader()->setCascadingSectionResizes(false);
        Locaux_m_table->verticalHeader()->setMinimumSectionSize(21);
        Locaux_m_table->verticalHeader()->setDefaultSectionSize(42);
        Locaux_m_table->verticalHeader()->setHighlightSections(true);
        Locaux_m_table->verticalHeader()->setProperty("showSortIndicator", QVariant(false));
        Locaux_m_table->verticalHeader()->setStretchLastSection(false);

        Locaux_control_12->addWidget(Locaux_m_views);

        Locaux_m_empty = new QLabel(LocauxPage);
        Locaux_m_empty->setObjectName("Locaux_m_empty");
        Locaux_m_empty->setMinimumSize(QSize(0, 0));
        Locaux_m_empty->setAutoFillBackground(false);
        Locaux_m_empty->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_empty->setLineWidth(1);
        Locaux_m_empty->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_m_empty->setWordWrap(false);
        Locaux_m_empty->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Locaux_m_empty->sizePolicy().hasHeightForWidth());
        Locaux_m_empty->setSizePolicy(sizePolicy);
        Locaux_m_empty->setMidLineWidth(0);
        Locaux_m_empty->setFont(font3);
        Locaux_m_empty->setVisible(false);

        Locaux_control_12->addWidget(Locaux_m_empty);

        Locaux_control_12->setStretch(0, 1);

        Locaux_control_11->addLayout(Locaux_control_12);

        Locaux_control_71 = new QScrollArea(LocauxPage);
        Locaux_control_71->setObjectName("Locaux_control_71");
        Locaux_control_71->setMinimumSize(QSize(306, 0));
        Locaux_control_71->setWidgetResizable(true);
        Locaux_control_71->setAutoFillBackground(false);
        Locaux_control_71->setMaximumSize(QSize(306, 16777215));
        Locaux_control_71->setLineWidth(1);
        Locaux_control_71->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CursorShape::ArrowCursor)));
        Locaux_control_71->setGeometry(QRect(0, 0, 306, 480));
        sizePolicy4.setHeightForWidth(Locaux_control_71->sizePolicy().hasHeightForWidth());
        Locaux_control_71->setSizePolicy(sizePolicy4);
        Locaux_control_71->setMidLineWidth(0);
        Locaux_control_71->setFont(font3);
        Locaux_control_73 = new QWidget();
        Locaux_control_73->setObjectName("Locaux_control_73");
        Locaux_control_73->setAutoFillBackground(true);
        Locaux_control_73->setMinimumSize(QSize(0, 0));
        Locaux_control_73->setMaximumSize(QSize(16777215, 16777215));
        Locaux_control_73->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_control_73->setGeometry(QRect(0, 0, 306, 587));
        sizePolicy.setHeightForWidth(Locaux_control_73->sizePolicy().hasHeightForWidth());
        Locaux_control_73->setSizePolicy(sizePolicy);
        Locaux_control_73->setFont(font);
        Locaux_control_74 = new QVBoxLayout(Locaux_control_73);
        Locaux_control_74->setSpacing(16);
        Locaux_control_74->setObjectName("Locaux_control_74");
        Locaux_control_74->setContentsMargins(0, 0, 8, 0);
        Locaux_m_detail = new DetailPanel(Locaux_control_73);
        Locaux_m_detail->setObjectName("Locaux_m_detail");
        Locaux_m_detail->setMinimumSize(QSize(0, 0));
        Locaux_m_detail->setAutoFillBackground(false);
        Locaux_m_detail->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_detail->setLineWidth(1);
        Locaux_m_detail->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_m_detail->setGeometry(QRect(0, 0, 298, 116));
        sizePolicy.setHeightForWidth(Locaux_m_detail->sizePolicy().hasHeightForWidth());
        Locaux_m_detail->setSizePolicy(sizePolicy);
        Locaux_m_detail->setMidLineWidth(0);
        Locaux_m_detail->setFont(font);

        Locaux_control_74->addWidget(Locaux_m_detail);

        panel = new QFrame(Locaux_control_73);
        panel->setObjectName("panel");
        panel->setMinimumSize(QSize(0, 0));
        panel->setAutoFillBackground(false);
        panel->setMaximumSize(QSize(16777215, 16777215));
        panel->setLineWidth(1);
        panel->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        panel->setGeometry(QRect(0, 132, 298, 85));
        sizePolicy.setHeightForWidth(panel->sizePolicy().hasHeightForWidth());
        panel->setSizePolicy(sizePolicy);
        panel->setMidLineWidth(0);
        panel->setFont(font);
        Locaux_control_102 = new QVBoxLayout(panel);
        Locaux_control_102->setSpacing(8);
        Locaux_control_102->setObjectName("Locaux_control_102");
        Locaux_control_102->setContentsMargins(16, 14, 16, 14);
        Locaux_control_103 = new QHBoxLayout();
        Locaux_control_103->setSpacing(8);
        Locaux_control_103->setObjectName("Locaux_control_103");
        Locaux_control_103->setContentsMargins(0, 0, 0, 0);
        Locaux_control_104 = new QLabel(panel);
        Locaux_control_104->setObjectName("Locaux_control_104");
        Locaux_control_104->setMinimumSize(QSize(0, 0));
        Locaux_control_104->setAutoFillBackground(false);
        Locaux_control_104->setMaximumSize(QSize(16777215, 16777215));
        Locaux_control_104->setLineWidth(1);
        Locaux_control_104->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_control_104->setWordWrap(false);
        Locaux_control_104->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Locaux_control_104->sizePolicy().hasHeightForWidth());
        Locaux_control_104->setSizePolicy(sizePolicy);
        Locaux_control_104->setMidLineWidth(0);
        Locaux_control_104->setFont(font1);

        Locaux_control_103->addWidget(Locaux_control_104);

        spacerItem23 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        Locaux_control_103->addItem(spacerItem23);

        locauxEnlargeMap = new QPushButton(panel);
        locauxEnlargeMap->setObjectName("locauxEnlargeMap");
        locauxEnlargeMap->setMinimumSize(QSize(0, 0));
        locauxEnlargeMap->setChecked(false);
        locauxEnlargeMap->setIconSize(QSize(16, 16));
        locauxEnlargeMap->setAutoFillBackground(false);
        locauxEnlargeMap->setAutoDefault(false);
        locauxEnlargeMap->setMaximumSize(QSize(16777215, 16777215));
        locauxEnlargeMap->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        locauxEnlargeMap->setFlat(false);
        locauxEnlargeMap->setCheckable(false);
        locauxEnlargeMap->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy1.setHeightForWidth(locauxEnlargeMap->sizePolicy().hasHeightForWidth());
        locauxEnlargeMap->setSizePolicy(sizePolicy1);
        locauxEnlargeMap->setFont(font2);

        Locaux_control_103->addWidget(locauxEnlargeMap);


        Locaux_control_102->addLayout(Locaux_control_103);

        Locaux_m_miniMap = new ClinicMap(panel);
        Locaux_m_miniMap->setObjectName("Locaux_m_miniMap");
        Locaux_m_miniMap->setAutoFillBackground(false);
        Locaux_m_miniMap->setMinimumSize(QSize(0, 202));
        Locaux_m_miniMap->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_miniMap->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_m_miniMap->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Locaux_m_miniMap->sizePolicy().hasHeightForWidth());
        Locaux_m_miniMap->setSizePolicy(sizePolicy);
        Locaux_m_miniMap->setFont(font);

        Locaux_control_102->addWidget(Locaux_m_miniMap);

        Locaux_control_105 = new QLabel(panel);
        Locaux_control_105->setObjectName("Locaux_control_105");
        Locaux_control_105->setMinimumSize(QSize(0, 0));
        Locaux_control_105->setAutoFillBackground(false);
        Locaux_control_105->setMaximumSize(QSize(16777215, 16777215));
        Locaux_control_105->setLineWidth(1);
        Locaux_control_105->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_control_105->setWordWrap(false);
        Locaux_control_105->setGeometry(QRect(0, 0, 640, 480));
        Locaux_control_105->setStyleSheet(QString::fromUtf8("font-size:11px;color:#6f8a92;"));
        sizePolicy.setHeightForWidth(Locaux_control_105->sizePolicy().hasHeightForWidth());
        Locaux_control_105->setSizePolicy(sizePolicy);
        Locaux_control_105->setMidLineWidth(0);
        Locaux_control_105->setFont(font);

        Locaux_control_102->addWidget(Locaux_control_105);


        Locaux_control_74->addWidget(panel);

        Locaux_control_106 = new QFrame(Locaux_control_73);
        Locaux_control_106->setObjectName("Locaux_control_106");
        Locaux_control_106->setMinimumSize(QSize(0, 0));
        Locaux_control_106->setAutoFillBackground(false);
        Locaux_control_106->setMaximumSize(QSize(16777215, 16777215));
        Locaux_control_106->setLineWidth(1);
        Locaux_control_106->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_control_106->setGeometry(QRect(0, 233, 298, 104));
        sizePolicy.setHeightForWidth(Locaux_control_106->sizePolicy().hasHeightForWidth());
        Locaux_control_106->setSizePolicy(sizePolicy);
        Locaux_control_106->setMidLineWidth(0);
        Locaux_control_106->setFont(font);
        Locaux_control_107 = new QVBoxLayout(Locaux_control_106);
        Locaux_control_107->setSpacing(6);
        Locaux_control_107->setObjectName("Locaux_control_107");
        Locaux_control_107->setContentsMargins(20, 16, 20, 16);
        panelTitle = new QLabel(Locaux_control_106);
        panelTitle->setObjectName("panelTitle");
        panelTitle->setMinimumSize(QSize(0, 0));
        panelTitle->setAutoFillBackground(false);
        panelTitle->setMaximumSize(QSize(16777215, 16777215));
        panelTitle->setLineWidth(1);
        panelTitle->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        panelTitle->setWordWrap(false);
        panelTitle->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(panelTitle->sizePolicy().hasHeightForWidth());
        panelTitle->setSizePolicy(sizePolicy);
        panelTitle->setMidLineWidth(0);
        panelTitle->setFont(font1);

        Locaux_control_107->addWidget(panelTitle);

        Locaux_m_pct = new QLabel(Locaux_control_106);
        Locaux_m_pct->setObjectName("Locaux_m_pct");
        Locaux_m_pct->setMinimumSize(QSize(0, 0));
        Locaux_m_pct->setAutoFillBackground(false);
        Locaux_m_pct->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_pct->setLineWidth(1);
        Locaux_m_pct->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_m_pct->setWordWrap(false);
        Locaux_m_pct->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Locaux_m_pct->sizePolicy().hasHeightForWidth());
        Locaux_m_pct->setSizePolicy(sizePolicy);
        Locaux_m_pct->setMidLineWidth(0);
        Locaux_m_pct->setFont(font1);

        Locaux_control_107->addWidget(Locaux_m_pct);

        Locaux_m_bar = new QProgressBar(Locaux_control_106);
        Locaux_m_bar->setObjectName("Locaux_m_bar");
        Locaux_m_bar->setMinimum(0);
        Locaux_m_bar->setMinimumSize(QSize(0, 10));
        Locaux_m_bar->setTextVisible(false);
        Locaux_m_bar->setMaximum(100);
        Locaux_m_bar->setAutoFillBackground(false);
        Locaux_m_bar->setMaximumSize(QSize(16777215, 10));
        Locaux_m_bar->setValue(55);
        Locaux_m_bar->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_m_bar->setGeometry(QRect(0, 0, 640, 10));
        sizePolicy2.setHeightForWidth(Locaux_m_bar->sizePolicy().hasHeightForWidth());
        Locaux_m_bar->setSizePolicy(sizePolicy2);
        Locaux_m_bar->setFont(font);
        Locaux_m_bar->setProperty("motionTarget", QVariant(55));

        Locaux_control_107->addWidget(Locaux_m_bar);

        Locaux_m_globalText = new QLabel(Locaux_control_106);
        Locaux_m_globalText->setObjectName("Locaux_m_globalText");
        Locaux_m_globalText->setMinimumSize(QSize(0, 0));
        Locaux_m_globalText->setAutoFillBackground(false);
        Locaux_m_globalText->setMaximumSize(QSize(16777215, 16777215));
        Locaux_m_globalText->setLineWidth(1);
        Locaux_m_globalText->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        Locaux_m_globalText->setWordWrap(false);
        Locaux_m_globalText->setGeometry(QRect(0, 0, 640, 480));
        sizePolicy.setHeightForWidth(Locaux_m_globalText->sizePolicy().hasHeightForWidth());
        Locaux_m_globalText->setSizePolicy(sizePolicy);
        Locaux_m_globalText->setMidLineWidth(0);
        Locaux_m_globalText->setFont(font);

        Locaux_control_107->addWidget(Locaux_m_globalText);


        Locaux_control_74->addWidget(Locaux_control_106);

        spacerItem24 = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        Locaux_control_74->addItem(spacerItem24);

        Locaux_control_71->setWidget(Locaux_control_73);

        Locaux_control_11->addWidget(Locaux_control_71);

        Locaux_control_11->setStretch(0, 1);

        Locaux_control_1->addLayout(Locaux_control_11);

        Locaux_control_1->setStretch(2, 1);
        pageStack->addWidget(LocauxPage);
        MedicalPage = new QMainWindow();
        MedicalPage->setObjectName("MedicalPage");
        MedicalPage->setGeometry(QRect(0, 0, 1400, 900));
        MedicalPage->setMinimumSize(QSize(1100, 800));
        Medical_centralwidget = new QWidget(MedicalPage);
        Medical_centralwidget->setObjectName("Medical_centralwidget");
        Medical_centralwidget->setGeometry(QRect(0, 0, 1400, 900));
        Medical_sidebar = new QWidget(Medical_centralwidget);
        Medical_sidebar->setObjectName("Medical_sidebar");
        Medical_sidebar->setGeometry(QRect(0, 0, 290, 900));
        Medical_logoIcon = new QLabel(Medical_sidebar);
        Medical_logoIcon->setObjectName("Medical_logoIcon");
        Medical_logoIcon->setGeometry(QRect(24, 26, 56, 56));
        Medical_logoIcon->setAlignment(Qt::AlignCenter);
        Medical_brandName = new QLabel(Medical_sidebar);
        Medical_brandName->setObjectName("Medical_brandName");
        Medical_brandName->setGeometry(QRect(94, 26, 180, 30));
        Medical_brandTag = new QLabel(Medical_sidebar);
        Medical_brandTag->setObjectName("Medical_brandTag");
        Medical_brandTag->setGeometry(QRect(94, 56, 180, 16));
        Medical_sectionCard = new QFrame(Medical_sidebar);
        Medical_sectionCard->setObjectName("Medical_sectionCard");
        Medical_sectionCard->setGeometry(QRect(18, 106, 254, 70));
        Medical_sectionTitle = new QLabel(Medical_sectionCard);
        Medical_sectionTitle->setObjectName("Medical_sectionTitle");
        Medical_sectionTitle->setGeometry(QRect(18, 10, 218, 27));
        Medical_sectionSub = new QLabel(Medical_sectionCard);
        Medical_sectionSub->setObjectName("Medical_sectionSub");
        Medical_sectionSub->setGeometry(QRect(18, 40, 218, 22));
        Medical_navLabel = new QLabel(Medical_sidebar);
        Medical_navLabel->setObjectName("Medical_navLabel");
        Medical_navLabel->setGeometry(QRect(18, 202, 254, 16));
        Medical_btnNavDashboard = new QPushButton(Medical_sidebar);
        Medical_btnNavDashboard->setObjectName("Medical_btnNavDashboard");
        Medical_btnNavDashboard->setGeometry(QRect(18, 226, 254, 44));
        Medical_btnNavDashboard->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon26;
        icon26.addFile(QString::fromUtf8(":/medical-icons/dashboard.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Medical_btnNavDashboard->setIcon(icon26);
        Medical_btnNavDashboard->setIconSize(QSize(20, 20));
        Medical_btnNavDashboard->setProperty("active", QVariant(false));
        Medical_btnNavServices = new QPushButton(Medical_sidebar);
        Medical_btnNavServices->setObjectName("Medical_btnNavServices");
        Medical_btnNavServices->setGeometry(QRect(18, 276, 254, 44));
        Medical_btnNavServices->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon27;
        icon27.addFile(QString::fromUtf8(":/medical-icons/services.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Medical_btnNavServices->setIcon(icon27);
        Medical_btnNavServices->setIconSize(QSize(20, 20));
        Medical_btnNavServices->setProperty("active", QVariant(false));
        Medical_btnNavEmployees = new QPushButton(Medical_sidebar);
        Medical_btnNavEmployees->setObjectName("Medical_btnNavEmployees");
        Medical_btnNavEmployees->setGeometry(QRect(18, 326, 254, 44));
        Medical_btnNavEmployees->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon28;
        icon28.addFile(QString::fromUtf8(":/medical-icons/employees.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Medical_btnNavEmployees->setIcon(icon28);
        Medical_btnNavEmployees->setIconSize(QSize(20, 20));
        Medical_btnNavEmployees->setProperty("active", QVariant(false));
        Medical_btnNavStock = new QPushButton(Medical_sidebar);
        Medical_btnNavStock->setObjectName("Medical_btnNavStock");
        Medical_btnNavStock->setGeometry(QRect(18, 376, 254, 44));
        Medical_btnNavStock->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon29;
        icon29.addFile(QString::fromUtf8(":/medical-icons/stock.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Medical_btnNavStock->setIcon(icon29);
        Medical_btnNavStock->setIconSize(QSize(20, 20));
        Medical_btnNavStock->setProperty("active", QVariant(false));
        Medical_btnNavPremises = new QPushButton(Medical_sidebar);
        Medical_btnNavPremises->setObjectName("Medical_btnNavPremises");
        Medical_btnNavPremises->setGeometry(QRect(18, 426, 254, 44));
        Medical_btnNavPremises->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon30;
        icon30.addFile(QString::fromUtf8(":/medical-icons/premises.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Medical_btnNavPremises->setIcon(icon30);
        Medical_btnNavPremises->setIconSize(QSize(20, 20));
        Medical_btnNavPremises->setProperty("active", QVariant(false));
        Medical_btnNavRecords = new QPushButton(Medical_sidebar);
        Medical_btnNavRecords->setObjectName("Medical_btnNavRecords");
        Medical_btnNavRecords->setGeometry(QRect(18, 476, 254, 44));
        Medical_btnNavRecords->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon31;
        icon31.addFile(QString::fromUtf8(":/medical-icons/records.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        Medical_btnNavRecords->setIcon(icon31);
        Medical_btnNavRecords->setIconSize(QSize(20, 20));
        Medical_btnNavRecords->setProperty("active", QVariant(true));
        Medical_sloganLabel = new QLabel(Medical_sidebar);
        Medical_sloganLabel->setObjectName("Medical_sloganLabel");
        Medical_sloganLabel->setGeometry(QRect(18, 838, 254, 18));
        Medical_sloganSub = new QLabel(Medical_sidebar);
        Medical_sloganSub->setObjectName("Medical_sloganSub");
        Medical_sloganSub->setGeometry(QRect(18, 860, 254, 18));
        Medical_mainContent = new QWidget(Medical_centralwidget);
        Medical_mainContent->setObjectName("Medical_mainContent");
        Medical_mainContent->setGeometry(QRect(290, 0, 1110, 900));
        Medical_topBar = new QWidget(Medical_mainContent);
        Medical_topBar->setObjectName("Medical_topBar");
        Medical_topBar->setGeometry(QRect(0, 0, 1110, 68));
        Medical_crumbGestion = new QLabel(Medical_topBar);
        Medical_crumbGestion->setObjectName("Medical_crumbGestion");
        Medical_crumbGestion->setGeometry(QRect(34, 22, 70, 24));
        Medical_crumbSep = new QLabel(Medical_topBar);
        Medical_crumbSep->setObjectName("Medical_crumbSep");
        Medical_crumbSep->setGeometry(QRect(108, 22, 14, 24));
        Medical_crumbCurrent = new QLabel(Medical_topBar);
        Medical_crumbCurrent->setObjectName("Medical_crumbCurrent");
        Medical_crumbCurrent->setGeometry(QRect(126, 22, 180, 24));
        Medical_demoBadge = new QLabel(Medical_topBar);
        Medical_demoBadge->setObjectName("Medical_demoBadge");
        Medical_demoBadge->setGeometry(QRect(556, 10, 210, 48));
        Medical_demoBadge->setAlignment(Qt::AlignCenter);
        Medical_btnFile = new QPushButton(Medical_topBar);
        Medical_btnFile->setObjectName("Medical_btnFile");
        Medical_btnFile->setGeometry(QRect(780, 11, 120, 45));
        Medical_btnFile->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_roleLabel = new QLabel(Medical_topBar);
        Medical_roleLabel->setObjectName("Medical_roleLabel");
        Medical_roleLabel->setGeometry(QRect(914, 22, 170, 24));
        Medical_pageArea = new QWidget(Medical_mainContent);
        Medical_pageArea->setObjectName("Medical_pageArea");
        Medical_pageArea->setGeometry(QRect(34, 96, 1042, 752));
        Medical_recordsPage = new QWidget(Medical_pageArea);
        Medical_recordsPage->setObjectName("Medical_recordsPage");
        Medical_recordsPage->setGeometry(QRect(0, 0, 656, 752));
        Medical_pageTitle = new QLabel(Medical_recordsPage);
        Medical_pageTitle->setObjectName("Medical_pageTitle");
        Medical_pageTitle->setGeometry(QRect(0, 0, 436, 44));
        Medical_pageSubtitle = new QLabel(Medical_recordsPage);
        Medical_pageSubtitle->setObjectName("Medical_pageSubtitle");
        Medical_pageSubtitle->setGeometry(QRect(0, 50, 656, 20));
        Medical_btnAdd = new QPushButton(Medical_recordsPage);
        Medical_btnAdd->setObjectName("Medical_btnAdd");
        Medical_btnAdd->setGeometry(QRect(446, 0, 210, 49));
        Medical_btnAdd->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_btnAdd->setProperty("big", QVariant(true));
        Medical_chipTotal = new QLabel(Medical_recordsPage);
        Medical_chipTotal->setObjectName("Medical_chipTotal");
        Medical_chipTotal->setGeometry(QRect(0, 90, 110, 39));
        Medical_chipTotal->setAlignment(Qt::AlignCenter);
        Medical_chipConsultations = new QLabel(Medical_recordsPage);
        Medical_chipConsultations->setObjectName("Medical_chipConsultations");
        Medical_chipConsultations->setGeometry(QRect(120, 90, 130, 39));
        Medical_chipConsultations->setAlignment(Qt::AlignCenter);
        Medical_chipUpToDate = new QLabel(Medical_recordsPage);
        Medical_chipUpToDate->setObjectName("Medical_chipUpToDate");
        Medical_chipUpToDate->setGeometry(QRect(260, 90, 90, 39));
        Medical_chipUpToDate->setAlignment(Qt::AlignCenter);
        Medical_chipToRenew = new QLabel(Medical_recordsPage);
        Medical_chipToRenew->setObjectName("Medical_chipToRenew");
        Medical_chipToRenew->setGeometry(QRect(360, 90, 145, 39));
        Medical_chipToRenew->setAlignment(Qt::AlignCenter);
        Medical_btnStatsTop = new QPushButton(Medical_recordsPage);
        Medical_btnStatsTop->setObjectName("Medical_btnStatsTop");
        Medical_btnStatsTop->setGeometry(QRect(526, 54, 130, 30));
        Medical_btnStatsTop->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_searchEdit = new QLineEdit(Medical_recordsPage);
        Medical_searchEdit->setObjectName("Medical_searchEdit");
        Medical_searchEdit->setGeometry(QRect(0, 147, 514, 45));
        Medical_searchEdit->setClearButtonEnabled(true);
        Medical_filterSpecies = new QComboBox(Medical_recordsPage);
        Medical_filterSpecies->addItem(QString());
        Medical_filterSpecies->addItem(QString());
        Medical_filterSpecies->addItem(QString());
        Medical_filterSpecies->addItem(QString());
        Medical_filterSpecies->addItem(QString());
        Medical_filterSpecies->setObjectName("Medical_filterSpecies");
        Medical_filterSpecies->setGeometry(QRect(0, 202, 160, 42));
        Medical_filterSpecies->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_filterVaccination = new QComboBox(Medical_recordsPage);
        Medical_filterVaccination->addItem(QString());
        Medical_filterVaccination->addItem(QString());
        Medical_filterVaccination->addItem(QString());
        Medical_filterVaccination->setObjectName("Medical_filterVaccination");
        Medical_filterVaccination->setGeometry(QRect(170, 202, 155, 42));
        Medical_filterVaccination->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_sortCombo = new QComboBox(Medical_recordsPage);
        Medical_sortCombo->addItem(QString());
        Medical_sortCombo->addItem(QString());
        Medical_sortCombo->addItem(QString());
        Medical_sortCombo->addItem(QString());
        Medical_sortCombo->setObjectName("Medical_sortCombo");
        Medical_sortCombo->setGeometry(QRect(335, 202, 155, 42));
        Medical_sortCombo->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_btnReset = new QPushButton(Medical_recordsPage);
        Medical_btnReset->setObjectName("Medical_btnReset");
        Medical_btnReset->setGeometry(QRect(500, 202, 156, 42));
        Medical_btnReset->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_cardsScroll = new QScrollArea(Medical_recordsPage);
        Medical_cardsScroll->setObjectName("Medical_cardsScroll");
        Medical_cardsScroll->setGeometry(QRect(0, 263, 656, 489));
        Medical_cardsScroll->setFrameShape(QFrame::NoFrame);
        Medical_cardsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        Medical_cardsScroll->setWidgetResizable(false);
        Medical_cardsContainer = new QWidget();
        Medical_cardsContainer->setObjectName("Medical_cardsContainer");
        Medical_cardsContainer->setGeometry(QRect(0, 0, 646, 489));
        Medical_emptyLabel = new QLabel(Medical_cardsContainer);
        Medical_emptyLabel->setObjectName("Medical_emptyLabel");
        Medical_emptyLabel->setGeometry(QRect(0, 0, 646, 160));
        Medical_emptyLabel->setAlignment(Qt::AlignCenter);
        Medical_cardsScroll->setWidget(Medical_cardsContainer);
        Medical_detailPanel = new QWidget(Medical_pageArea);
        Medical_detailPanel->setObjectName("Medical_detailPanel");
        Medical_detailPanel->setGeometry(QRect(682, 0, 360, 658));
        Medical_selectedCard = new QFrame(Medical_detailPanel);
        Medical_selectedCard->setObjectName("Medical_selectedCard");
        Medical_selectedCard->setGeometry(QRect(0, 0, 360, 434));
        Medical_eyebrow = new QLabel(Medical_selectedCard);
        Medical_eyebrow->setObjectName("Medical_eyebrow");
        Medical_eyebrow->setGeometry(QRect(22, 20, 316, 16));
        Medical_placeholderText = new QLabel(Medical_selectedCard);
        Medical_placeholderText->setObjectName("Medical_placeholderText");
        Medical_placeholderText->setGeometry(QRect(22, 44, 316, 60));
        Medical_placeholderText->setWordWrap(true);
        Medical_placeholderText->setVisible(false);
        Medical_detailContent = new QWidget(Medical_selectedCard);
        Medical_detailContent->setObjectName("Medical_detailContent");
        Medical_detailContent->setGeometry(QRect(22, 44, 316, 390));
        Medical_panelTitle = new QLabel(Medical_detailContent);
        Medical_panelTitle->setObjectName("Medical_panelTitle");
        Medical_panelTitle->setGeometry(QRect(0, 0, 316, 36));
        Medical_statusPill = new QLabel(Medical_detailContent);
        Medical_statusPill->setObjectName("Medical_statusPill");
        Medical_statusPill->setGeometry(QRect(0, 44, 120, 28));
        Medical_detailLine1 = new QLabel(Medical_detailContent);
        Medical_detailLine1->setObjectName("Medical_detailLine1");
        Medical_detailLine1->setGeometry(QRect(0, 78, 316, 20));
        Medical_detailLine1->setWordWrap(true);
        Medical_detailLine2 = new QLabel(Medical_detailContent);
        Medical_detailLine2->setObjectName("Medical_detailLine2");
        Medical_detailLine2->setGeometry(QRect(0, 102, 316, 36));
        Medical_detailLine2->setWordWrap(true);
        Medical_detailLine3 = new QLabel(Medical_detailContent);
        Medical_detailLine3->setObjectName("Medical_detailLine3");
        Medical_detailLine3->setGeometry(QRect(0, 142, 316, 56));
        Medical_detailLine3->setWordWrap(true);
        Medical_btnView = new QPushButton(Medical_detailContent);
        Medical_btnView->setObjectName("Medical_btnView");
        Medical_btnView->setGeometry(QRect(0, 208, 316, 36));
        Medical_btnView->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_btnToggle = new QPushButton(Medical_detailContent);
        Medical_btnToggle->setObjectName("Medical_btnToggle");
        Medical_btnToggle->setGeometry(QRect(0, 253, 153, 36));
        Medical_btnToggle->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_btnEdit = new QPushButton(Medical_detailContent);
        Medical_btnEdit->setObjectName("Medical_btnEdit");
        Medical_btnEdit->setGeometry(QRect(162, 253, 154, 36));
        Medical_btnEdit->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_btnDelete = new QPushButton(Medical_detailContent);
        Medical_btnDelete->setObjectName("Medical_btnDelete");
        Medical_btnDelete->setGeometry(QRect(0, 298, 316, 36));
        Medical_btnDelete->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_globalCard = new QFrame(Medical_detailPanel);
        Medical_globalCard->setObjectName("Medical_globalCard");
        Medical_globalCard->setGeometry(QRect(0, 488, 360, 215));
        Medical_panelSubTitle = new QLabel(Medical_globalCard);
        Medical_panelSubTitle->setObjectName("Medical_panelSubTitle");
        Medical_panelSubTitle->setGeometry(QRect(22, 20, 316, 26));
        Medical_bigValue = new QLabel(Medical_globalCard);
        Medical_bigValue->setObjectName("Medical_bigValue");
        Medical_bigValue->setGeometry(QRect(22, 54, 316, 38));
        Medical_globalBar = new QProgressBar(Medical_globalCard);
        Medical_globalBar->setObjectName("Medical_globalBar");
        Medical_globalBar->setGeometry(QRect(22, 100, 316, 10));
        Medical_globalBar->setMinimum(0);
        Medical_globalBar->setMaximum(100);
        Medical_globalBar->setValue(0);
        Medical_globalBar->setTextVisible(false);
        Medical_globalText = new QLabel(Medical_globalCard);
        Medical_globalText->setObjectName("Medical_globalText");
        Medical_globalText->setGeometry(QRect(22, 118, 316, 20));
        Medical_btnStatsPanel = new QPushButton(Medical_globalCard);
        Medical_btnStatsPanel->setObjectName("Medical_btnStatsPanel");
        Medical_btnStatsPanel->setGeometry(QRect(22, 148, 316, 45));
        Medical_btnStatsPanel->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        Medical_pageFooter = new QWidget(Medical_mainContent);
        Medical_pageFooter->setObjectName("Medical_pageFooter");
        Medical_pageFooter->setGeometry(QRect(0, 862, 1110, 38));
        Medical_footerLeft = new QLabel(Medical_pageFooter);
        Medical_footerLeft->setObjectName("Medical_footerLeft");
        Medical_footerLeft->setGeometry(QRect(34, 8, 300, 16));
        Medical_footerDate = new QLabel(Medical_pageFooter);
        Medical_footerDate->setObjectName("Medical_footerDate");
        Medical_footerDate->setGeometry(QRect(916, 8, 160, 16));
        Medical_footerDate->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        MedicalPage->setCentralWidget(Medical_centralwidget);
        pageStack->addWidget(MedicalPage);

        integratedLayout->addWidget(pageStack);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        Service_m_pages->setCurrentIndex(0);
        servicePlanning->setDefault(false);
        addService->setDefault(false);
        serviceStatistics->setDefault(false);
        Service_m_category->setCurrentIndex(0);
        Service_m_status->setCurrentIndex(0);
        Service_m_period->setCurrentIndex(0);
        Service_m_sort->setCurrentIndex(0);
        resetFilters->setDefault(false);
        cardsView->setDefault(false);
        tableView->setDefault(false);
        Service_m_views->setCurrentIndex(0);
        serviceFirstAdd->setDefault(false);
        Service_m_view->setDefault(false);
        Service_m_invoice->setDefault(false);
        Service_m_edit->setDefault(false);
        Service_m_advance->setDefault(false);
        Service_m_delete->setDefault(false);
        stockMovementButton->setDefault(false);
        stockExportButton->setDefault(false);
        stockStatisticsButton->setDefault(false);
        stockEditButton->setDefault(false);
        stockDeleteButton->setDefault(false);
        Stock_categoryFilter->setCurrentIndex(0);
        Stock_sortFilter->setCurrentIndex(0);
        Stock_stateFilter->setCurrentIndex(0);
        stockPurchasesButton->setDefault(false);
        Stock_alertsNavButton->setDefault(false);
        stockReplenishmentButton->setDefault(false);
        stockExpiryButton->setDefault(false);
        employeeLeavePdf->setDefault(false);
        addEmployee->setDefault(false);
        employeeStatistics->setDefault(false);
        Employees_m_comboPoste->setCurrentIndex(0);
        Employees_m_comboStatut->setCurrentIndex(0);
        Employees_m_comboTri->setCurrentIndex(0);
        editEmployee->setDefault(false);
        toggleEmployeeAvailability->setDefault(false);
        deleteEmployee->setDefault(false);
        Locaux_m_btnAdd->setDefault(false);
        Locaux_m_chipAvail->setDefault(false);
        Locaux_m_chipOcc->setDefault(false);
        Locaux_m_chipOos->setDefault(false);
        Locaux_m_chipDirty->setDefault(false);
        Locaux_m_btnAlerts->setDefault(false);
        Locaux_m_btnSmart->setDefault(false);
        Locaux_m_btnPdf->setDefault(false);
        Locaux_m_btnMap->setDefault(false);
        Locaux_m_btnStats->setDefault(false);
        Locaux_m_sortCombo->setCurrentIndex(0);
        locauxResetFilters->setDefault(false);
        Locaux_m_btnCards->setDefault(false);
        Locaux_m_btnList->setDefault(false);
        Locaux_m_views->setCurrentIndex(0);
        locauxEnlargeMap->setDefault(false);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "ANIWORLD \342\200\224 Interface int\303\251gr\303\251e", nullptr));
        ServicePage->setWindowTitle(QCoreApplication::translate("MainWindow", "Service", nullptr));
        Service_m_pages->setObjectName(QCoreApplication::translate("MainWindow", "servicePages", nullptr));
        Service_control_13->setText(QCoreApplication::translate("MainWindow", "Services", nullptr));
        Service_control_13->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "title", nullptr)));
        Service_control_14->setText(QCoreApplication::translate("MainWindow", "Prestations, suivi et organisation des soins", nullptr));
        Service_control_14->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        servicePlanning->setText(QCoreApplication::translate("MainWindow", "Planning des employ\303\251s", nullptr));
#if QT_CONFIG(tooltip)
        servicePlanning->setToolTip(QCoreApplication::translate("MainWindow", "Voir les affectations des employ\303\251s par service et par semaine.", nullptr));
#endif // QT_CONFIG(tooltip)
        addService->setText(QCoreApplication::translate("MainWindow", "Ajouter un service", nullptr));
#if QT_CONFIG(tooltip)
        addService->setToolTip(QCoreApplication::translate("MainWindow", "Ajouter une prestation \302\267 Ctrl+N", nullptr));
#endif // QT_CONFIG(tooltip)
        addService->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "primary", nullptr)));
        Service_summary_0->setText(QCoreApplication::translate("MainWindow", "0 planifi\303\251s", nullptr));
        Service_summary_1->setText(QCoreApplication::translate("MainWindow", "0 en cours", nullptr));
        Service_summary_2->setText(QCoreApplication::translate("MainWindow", "0 termin\303\251s", nullptr));
        Service_summary_3->setText(QCoreApplication::translate("MainWindow", "0 h 00 \303\240 r\303\251aliser", nullptr));
        serviceStatistics->setText(QCoreApplication::translate("MainWindow", "Voir les statistiques", nullptr));
        Service_m_notice->setObjectName(QCoreApplication::translate("MainWindow", "serviceNotice", nullptr));
        Service_m_search->setPlaceholderText(QCoreApplication::translate("MainWindow", "Rechercher par service, client ou r\303\251f\303\251rence\342\200\246", nullptr));
        Service_m_search->setObjectName(QCoreApplication::translate("MainWindow", "serviceSearch", nullptr));
        Service_m_category->setItemText(0, QCoreApplication::translate("MainWindow", "Toutes les cat\303\251gories", nullptr));
        Service_m_category->setItemText(1, QCoreApplication::translate("MainWindow", "Consultation v\303\251t\303\251rinaire", nullptr));
        Service_m_category->setItemText(2, QCoreApplication::translate("MainWindow", "Vaccination", nullptr));
        Service_m_category->setItemText(3, QCoreApplication::translate("MainWindow", "Analyses de laboratoire", nullptr));
        Service_m_category->setItemText(4, QCoreApplication::translate("MainWindow", "Imagerie m\303\251dicale", nullptr));
        Service_m_category->setItemText(5, QCoreApplication::translate("MainWindow", "Chirurgie v\303\251t\303\251rinaire", nullptr));
        Service_m_category->setItemText(6, QCoreApplication::translate("MainWindow", "Soins dentaires", nullptr));
        Service_m_category->setItemText(7, QCoreApplication::translate("MainWindow", "Soins infirmiers", nullptr));
        Service_m_category->setItemText(8, QCoreApplication::translate("MainWindow", "Suivi postop\303\251ratoire", nullptr));

        Service_m_category->setPlaceholderText(QString());
        Service_m_category->setObjectName(QCoreApplication::translate("MainWindow", "categoryFilter", nullptr));
        Service_m_status->setItemText(0, QCoreApplication::translate("MainWindow", "Tous les statuts", nullptr));
        Service_m_status->setItemText(1, QCoreApplication::translate("MainWindow", "Planifi\303\251", nullptr));
        Service_m_status->setItemText(2, QCoreApplication::translate("MainWindow", "En cours", nullptr));
        Service_m_status->setItemText(3, QCoreApplication::translate("MainWindow", "Termin\303\251", nullptr));
        Service_m_status->setItemText(4, QCoreApplication::translate("MainWindow", "Annul\303\251", nullptr));

        Service_m_status->setPlaceholderText(QString());
        Service_m_status->setObjectName(QCoreApplication::translate("MainWindow", "statusFilter", nullptr));
        Service_m_period->setItemText(0, QCoreApplication::translate("MainWindow", "Toutes les dates", nullptr));
        Service_m_period->setItemText(1, QCoreApplication::translate("MainWindow", "Aujourd'hui", nullptr));
        Service_m_period->setItemText(2, QCoreApplication::translate("MainWindow", "7 prochains jours", nullptr));
        Service_m_period->setItemText(3, QCoreApplication::translate("MainWindow", "\303\200 replanifier", nullptr));

        Service_m_period->setPlaceholderText(QString());
        Service_m_period->setObjectName(QCoreApplication::translate("MainWindow", "periodFilter", nullptr));
        Service_control_50->setText(QCoreApplication::translate("MainWindow", "Trier par", nullptr));
        Service_control_50->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_m_sort->setItemText(0, QCoreApplication::translate("MainWindow", "Plus r\303\251cents", nullptr));
        Service_m_sort->setItemText(1, QCoreApplication::translate("MainWindow", "Plus anciens", nullptr));
        Service_m_sort->setItemText(2, QCoreApplication::translate("MainWindow", "Nom : A \342\206\222 Z", nullptr));
        Service_m_sort->setItemText(3, QCoreApplication::translate("MainWindow", "Tarif croissant", nullptr));
        Service_m_sort->setItemText(4, QCoreApplication::translate("MainWindow", "Tarif d\303\251croissant", nullptr));
        Service_m_sort->setItemText(5, QCoreApplication::translate("MainWindow", "Dur\303\251e croissante", nullptr));
        Service_m_sort->setItemText(6, QCoreApplication::translate("MainWindow", "Dur\303\251e d\303\251croissante", nullptr));

        Service_m_sort->setPlaceholderText(QString());
#if QT_CONFIG(accessibility)
        Service_m_sort->setAccessibleName(QCoreApplication::translate("MainWindow", "Trier les services par date, tarif ou dur\303\251e", nullptr));
#endif // QT_CONFIG(accessibility)
        Service_m_sort->setObjectName(QCoreApplication::translate("MainWindow", "sortServices", nullptr));
        resetFilters->setText(QCoreApplication::translate("MainWindow", "R\303\251initialiser", nullptr));
        resetFilters->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "link", nullptr)));
        Service_m_count->setText(QCoreApplication::translate("MainWindow", "0 / 0 services", nullptr));
        Service_m_count->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_m_count->setObjectName(QCoreApplication::translate("MainWindow", "serviceCount", nullptr));
        cardsView->setText(QCoreApplication::translate("MainWindow", "Cartes", nullptr));
        cardsView->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "toggle", nullptr)));
        tableView->setText(QCoreApplication::translate("MainWindow", "Liste", nullptr));
        tableView->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "toggle", nullptr)));
        Service_m_table->setObjectName(QCoreApplication::translate("MainWindow", "serviceTable", nullptr));
        Service_control_96->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "card", nullptr)));
        Service_m_emptyTitle->setText(QCoreApplication::translate("MainWindow", "Votre premi\303\250re prestation", nullptr));
        Service_m_emptyTitle->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "section", nullptr)));
        Service_m_emptyText->setText(QCoreApplication::translate("MainWindow", "Ajoutez un service pour organiser vos soins.\n"
"Vos donn\303\251es seront conserv\303\251es sur cet ordinateur.", nullptr));
        Service_m_emptyText->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        serviceFirstAdd->setText(QCoreApplication::translate("MainWindow", "Ajouter un service", nullptr));
        serviceFirstAdd->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "primary", nullptr)));
        serviceDetailPanel->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "card", nullptr)));
        Service_control_102->setText(QCoreApplication::translate("MainWindow", "SERVICE S\303\211LECTIONN\303\211", nullptr));
        Service_control_102->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "eyebrow", nullptr)));
        Service_m_detailName->setText(QCoreApplication::translate("MainWindow", "S\303\251lectionnez un service", nullptr));
        Service_m_detailName->setObjectName(QCoreApplication::translate("MainWindow", "selectedServiceName", nullptr));
        Service_m_detailReference->setText(QCoreApplication::translate("MainWindow", "Les informations appara\303\256tront ici.", nullptr));
        Service_m_detailReference->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_control_110->setText(QCoreApplication::translate("MainWindow", "Cat\303\251gorie", nullptr));
        Service_control_110->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_control_111->setText(QCoreApplication::translate("MainWindow", "Date", nullptr));
        Service_control_111->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_control_112->setText(QCoreApplication::translate("MainWindow", "Horaire", nullptr));
        Service_control_112->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_control_113->setText(QCoreApplication::translate("MainWindow", "Dur\303\251e", nullptr));
        Service_control_113->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_control_114->setText(QCoreApplication::translate("MainWindow", "Responsable", nullptr));
        Service_control_114->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_control_115->setText(QCoreApplication::translate("MainWindow", "Client", nullptr));
        Service_control_115->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_control_116->setText(QCoreApplication::translate("MainWindow", "Tarif", nullptr));
        Service_control_116->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_control_117->setText(QCoreApplication::translate("MainWindow", "Facture", nullptr));
        Service_control_117->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_m_detailNotes->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_m_view->setText(QCoreApplication::translate("MainWindow", "Consulter", nullptr));
        Service_m_view->setObjectName(QCoreApplication::translate("MainWindow", "viewService", nullptr));
        Service_m_invoice->setText(QCoreApplication::translate("MainWindow", "Facture PDF", nullptr));
#if QT_CONFIG(tooltip)
        Service_m_invoice->setToolTip(QCoreApplication::translate("MainWindow", "Facture cr\303\251\303\251e automatiquement avec les informations du service.", nullptr));
#endif // QT_CONFIG(tooltip)
        Service_m_invoice->setObjectName(QCoreApplication::translate("MainWindow", "invoiceService", nullptr));
        Service_m_edit->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        Service_m_edit->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "primary", nullptr)));
        Service_m_edit->setObjectName(QCoreApplication::translate("MainWindow", "editService", nullptr));
        Service_m_advance->setText(QCoreApplication::translate("MainWindow", "D\303\251marrer le service", nullptr));
        Service_m_advance->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "success", nullptr)));
        Service_m_advance->setObjectName(QCoreApplication::translate("MainWindow", "advanceService", nullptr));
        Service_m_delete->setText(QCoreApplication::translate("MainWindow", "Supprimer", nullptr));
        Service_m_delete->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "danger", nullptr)));
        Service_m_delete->setObjectName(QCoreApplication::translate("MainWindow", "deleteService", nullptr));
        Service_m_attention->setText(QCoreApplication::translate("MainWindow", "Aucun service planifi\303\251 en retard. Les statuts et les indicateurs sont mis \303\240 jour apr\303\250s chaque modification.", nullptr));
        Service_m_attention->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_control_124->setText(QCoreApplication::translate("MainWindow", "ANIWORLD \302\267 Gestion des services", nullptr));
        Service_control_124->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        Service_control_125->setText(QCoreApplication::translate("MainWindow", "Ctrl+N  Ajouter   \302\267   Ctrl+F  Rechercher", nullptr));
        Service_control_125->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "muted", nullptr)));
        StockPage->setWindowTitle(QCoreApplication::translate("MainWindow", "Stock", nullptr));
        Stock_control_7->setText(QCoreApplication::translate("MainWindow", "\360\237\223\246  Stock", nullptr));
        Stock_control_8->setText(QCoreApplication::translate("MainWindow", "Inventaire, tra\303\247abilit\303\251 des lots et r\303\251approvisionnement", nullptr));
        stockMovementButton->setText(QCoreApplication::translate("MainWindow", "\357\274\213  Enregistrer un mouvement", nullptr));
        stockMovementButton->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "primary", nullptr)));
        stockExportButton->setText(QCoreApplication::translate("MainWindow", "Exporter PDF", nullptr));
        stockStatisticsButton->setText(QCoreApplication::translate("MainWindow", "\342\226\246  Statistiques", nullptr));
        stockEditButton->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        stockDeleteButton->setText(QCoreApplication::translate("MainWindow", "Supprimer", nullptr));
        stockDeleteButton->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "danger", nullptr)));
        Stock_searchLineEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "\342\214\225   Rechercher un article, un lot, un \303\251tat...", nullptr));
        Stock_searchLineEdit->setObjectName(QCoreApplication::translate("MainWindow", "stockSearch", nullptr));
        Stock_categoryFilter->setItemText(0, QCoreApplication::translate("MainWindow", "Toutes les cat\303\251gories", nullptr));
        Stock_categoryFilter->setItemText(1, QCoreApplication::translate("MainWindow", "Chiens", nullptr));
        Stock_categoryFilter->setItemText(2, QCoreApplication::translate("MainWindow", "Chats", nullptr));
        Stock_categoryFilter->setItemText(3, QCoreApplication::translate("MainWindow", "Oiseaux", nullptr));
        Stock_categoryFilter->setItemText(4, QCoreApplication::translate("MainWindow", "Poissons", nullptr));
        Stock_categoryFilter->setItemText(5, QCoreApplication::translate("MainWindow", "Rongeurs", nullptr));
        Stock_categoryFilter->setItemText(6, QCoreApplication::translate("MainWindow", "Mat\303\251riel Clinique", nullptr));

        Stock_categoryFilter->setPlaceholderText(QString());
        Stock_categoryFilter->setObjectName(QCoreApplication::translate("MainWindow", "stockCategoryFilter", nullptr));
        Stock_sortFilter->setItemText(0, QCoreApplication::translate("MainWindow", "Tri : par d\303\251faut", nullptr));
        Stock_sortFilter->setItemText(1, QCoreApplication::translate("MainWindow", "Quantit\303\251 : croissante", nullptr));
        Stock_sortFilter->setItemText(2, QCoreApplication::translate("MainWindow", "Quantit\303\251 : d\303\251croissante", nullptr));
        Stock_sortFilter->setItemText(3, QCoreApplication::translate("MainWindow", "\303\211ch\303\251ance : plus proche", nullptr));
        Stock_sortFilter->setItemText(4, QCoreApplication::translate("MainWindow", "\303\211ch\303\251ance : plus \303\251loign\303\251e", nullptr));

        Stock_sortFilter->setPlaceholderText(QString());
        Stock_sortFilter->setObjectName(QCoreApplication::translate("MainWindow", "stockSortFilter", nullptr));
        Stock_stateFilter->setItemText(0, QCoreApplication::translate("MainWindow", "Tous les \303\251tats", nullptr));
        Stock_stateFilter->setItemText(1, QCoreApplication::translate("MainWindow", "Disponible", nullptr));
        Stock_stateFilter->setItemText(2, QCoreApplication::translate("MainWindow", "Sous le seuil", nullptr));
        Stock_stateFilter->setItemText(3, QCoreApplication::translate("MainWindow", "\303\211ch\303\251ance proche", nullptr));
        Stock_stateFilter->setItemText(4, QCoreApplication::translate("MainWindow", "P\303\251rim\303\251", nullptr));

        Stock_stateFilter->setPlaceholderText(QString());
        Stock_stateFilter->setObjectName(QCoreApplication::translate("MainWindow", "stockStateFilter", nullptr));
        Stock_stockTable->setObjectName(QCoreApplication::translate("MainWindow", "stockTable", nullptr));
        Stock_control_70->setText(QCoreApplication::translate("MainWindow", "Gestion avanc\303\251e", nullptr));
        Stock_control_71->setText(QCoreApplication::translate("MainWindow", "Factures fournisseurs, alertes et commandes", nullptr));
        stockPurchasesButton->setText(QCoreApplication::translate("MainWindow", "\342\226\244   Factures d'achat", nullptr));
        Stock_alertsNavButton->setText(QCoreApplication::translate("MainWindow", "\342\232\240   Alertes de stock (11)", nullptr));
        Stock_alertsNavButton->setObjectName(QCoreApplication::translate("MainWindow", "stockAlertsButton", nullptr));
        stockReplenishmentButton->setText(QCoreApplication::translate("MainWindow", "\342\206\273   R\303\251approvisionnement", nullptr));
        Stock_control_75->setText(QCoreApplication::translate("MainWindow", "Gestion des articles p\303\251rim\303\251s", nullptr));
        Stock_control_76->setText(QCoreApplication::translate("MainWindow", "Consultez les lots p\303\251rim\303\251s ou \303\240 \303\251ch\303\251ance proche, puis retirez-les du disponible ou remplacez-les par un nouveau lot.", nullptr));
        stockExpiryButton->setText(QCoreApplication::translate("MainWindow", "Afficher les articles p\303\251rim\303\251s", nullptr));
        stockExpiryButton->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "primary", nullptr)));
        Stock_footerLabel->setText(QCoreApplication::translate("MainWindow", "19 / 19 articles", nullptr));
        Stock_control_77->setText(QCoreApplication::translate("MainWindow", "Stock du centre \302\267 connexion commune", nullptr));
        EmployeesPage->setWindowTitle(QCoreApplication::translate("MainWindow", "Employees", nullptr));
        Employees_control_8->setText(QCoreApplication::translate("MainWindow", "Employ\303\251s", nullptr));
        Employees_control_9->setText(QCoreApplication::translate("MainWindow", "Annuaire, disponibilit\303\251s et gestion de la charge", nullptr));
        employeeLeavePdf->setText(QCoreApplication::translate("MainWindow", "Attestation PDF", nullptr));
        addEmployee->setText(QCoreApplication::translate("MainWindow", "Ajouter un employ\303\251", nullptr));
        addEmployee->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "primary", nullptr)));
        Employees_m_lblDisponibles->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        Employees_control_15->setText(QCoreApplication::translate("MainWindow", "disponibles", nullptr));
        Employees_m_lblIndisponibles->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        Employees_control_19->setText(QCoreApplication::translate("MainWindow", "indisponibles", nullptr));
        Employees_m_lblChargeTotale->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        Employees_m_lblChargeTotale->setObjectName(QCoreApplication::translate("MainWindow", "employeeTotalHours", nullptr));
        Employees_control_23->setText(QCoreApplication::translate("MainWindow", "heures planifi\303\251es", nullptr));
        employeeStatistics->setText(QCoreApplication::translate("MainWindow", "Voir les statistiques", nullptr));
        employeeStatistics->setProperty("role", QVariant(QString()));
        Employees_m_editRecherche->setPlaceholderText(QCoreApplication::translate("MainWindow", "Rechercher par nom, ID ou poste...", nullptr));
        Employees_m_editRecherche->setObjectName(QCoreApplication::translate("MainWindow", "employeeSearch", nullptr));
        Employees_m_comboPoste->setItemText(0, QCoreApplication::translate("MainWindow", "Tous les postes", nullptr));
        Employees_m_comboPoste->setItemText(1, QCoreApplication::translate("MainWindow", "V\303\251t\303\251rinaire", nullptr));
        Employees_m_comboPoste->setItemText(2, QCoreApplication::translate("MainWindow", "Assistant(e) v\303\251t\303\251rinaire", nullptr));
        Employees_m_comboPoste->setItemText(3, QCoreApplication::translate("MainWindow", "Auxiliaire v\303\251t\303\251rinaire", nullptr));
        Employees_m_comboPoste->setItemText(4, QCoreApplication::translate("MainWindow", "Secr\303\251taire / R\303\251ceptionniste", nullptr));
        Employees_m_comboPoste->setItemText(5, QCoreApplication::translate("MainWindow", "Agent d'entretien", nullptr));
        Employees_m_comboPoste->setItemText(6, QCoreApplication::translate("MainWindow", "Aide aux soins animaliers", nullptr));

        Employees_m_comboPoste->setPlaceholderText(QString());
        Employees_m_comboPoste->setObjectName(QCoreApplication::translate("MainWindow", "employeePostFilter", nullptr));
        Employees_m_comboStatut->setItemText(0, QCoreApplication::translate("MainWindow", "Tous", nullptr));
        Employees_m_comboStatut->setItemText(1, QCoreApplication::translate("MainWindow", "Disponible", nullptr));
        Employees_m_comboStatut->setItemText(2, QCoreApplication::translate("MainWindow", "Non disponible", nullptr));

        Employees_m_comboStatut->setPlaceholderText(QString());
        Employees_m_comboStatut->setObjectName(QCoreApplication::translate("MainWindow", "employeeStatusFilter", nullptr));
        Employees_m_comboTri->setItemText(0, QCoreApplication::translate("MainWindow", "Tri par date", nullptr));
        Employees_m_comboTri->setItemText(1, QCoreApplication::translate("MainWindow", "Tri par charge", nullptr));

        Employees_m_comboTri->setPlaceholderText(QString());
        Employees_m_comboTri->setObjectName(QCoreApplication::translate("MainWindow", "employeeSort", nullptr));
        employeeDetailPanel->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "card", nullptr)));
        Employees_control_70->setText(QCoreApplication::translate("MainWindow", "EMPLOY\303\211 S\303\211LECTIONN\303\211", nullptr));
        Employees_m_detailPlaceholder->setText(QCoreApplication::translate("MainWindow", "S\303\251lectionnez un employ\303\251\n"
"pour voir ses d\303\251tails", nullptr));
        Employees_m_detailContent->setObjectName(QCoreApplication::translate("MainWindow", "detailContent", nullptr));
        Employees_m_detailNom->setObjectName(QCoreApplication::translate("MainWindow", "employeeDetailName", nullptr));
        Employees_control_82->setText(QCoreApplication::translate("MainWindow", "Poste", nullptr));
        Employees_m_detailPoste->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        Employees_control_83->setText(QCoreApplication::translate("MainWindow", "Courriel", nullptr));
        Employees_m_detailCourriel->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        Employees_control_84->setText(QCoreApplication::translate("MainWindow", "T\303\251l\303\251phone", nullptr));
        Employees_m_detailTelephone->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        Employees_control_85->setText(QCoreApplication::translate("MainWindow", "Embauche", nullptr));
        Employees_m_detailDate->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        Employees_control_86->setText(QCoreApplication::translate("MainWindow", "Charge", nullptr));
        Employees_m_detailCharge->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        Employees_control_87->setText(QCoreApplication::translate("MainWindow", "Dispo", nullptr));
        Employees_m_detailDispo->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        Employees_control_88->setText(QCoreApplication::translate("MainWindow", "Comp\303\251tences", nullptr));
        Employees_m_detailCompetences->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        editEmployee->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        editEmployee->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "primary", nullptr)));
        toggleEmployeeAvailability->setText(QCoreApplication::translate("MainWindow", "Basculer disponibilit\303\251", nullptr));
        toggleEmployeeAvailability->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "success", nullptr)));
        deleteEmployee->setText(QCoreApplication::translate("MainWindow", "Supprimer", nullptr));
        deleteEmployee->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "danger", nullptr)));
        LocauxPage->setWindowTitle(QCoreApplication::translate("MainWindow", "Locaux", nullptr));
        h1->setText(QCoreApplication::translate("MainWindow", "Locaux", nullptr));
        Locaux_m_btnAdd->setText(QCoreApplication::translate("MainWindow", "Ajouter un local", nullptr));
        Locaux_m_btnAdd->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "primary", nullptr)));
        Locaux_m_btnAdd->setObjectName(QCoreApplication::translate("MainWindow", "locauxAddButton", nullptr));
        sub->setText(QCoreApplication::translate("MainWindow", "Occupation, nettoyage et disponibilit\303\251 des espaces", nullptr));
        Locaux_m_chipAvail->setText(QCoreApplication::translate("MainWindow", "9 disponible(s)", nullptr));
        Locaux_m_chipAvail->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "green", nullptr)));
        Locaux_m_chipOcc->setText(QCoreApplication::translate("MainWindow", "4 occup\303\251(s)", nullptr));
        Locaux_m_chipOcc->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "blue", nullptr)));
        Locaux_m_chipOos->setText(QCoreApplication::translate("MainWindow", "1 hors service", nullptr));
        Locaux_m_chipOos->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "orange", nullptr)));
        Locaux_m_chipDirty->setText(QCoreApplication::translate("MainWindow", "3 \303\240 nettoyer", nullptr));
        Locaux_m_chipDirty->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "red", nullptr)));
        Locaux_m_btnAlerts->setText(QCoreApplication::translate("MainWindow", "Alertes (3)", nullptr));
        Locaux_m_btnAlerts->setProperty("alert", QVariant(QCoreApplication::translate("MainWindow", "critical", nullptr)));
        Locaux_m_btnAlerts->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "ghost", nullptr)));
        Locaux_m_btnAlerts->setObjectName(QCoreApplication::translate("MainWindow", "locauxAlertsButton", nullptr));
        Locaux_m_btnSmart->setText(QCoreApplication::translate("MainWindow", "Affectation intelligente", nullptr));
        Locaux_m_btnSmart->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "ghost", nullptr)));
        Locaux_m_btnSmart->setObjectName(QCoreApplication::translate("MainWindow", "locauxAssignButton", nullptr));
        Locaux_m_btnPdf->setText(QCoreApplication::translate("MainWindow", "Rapport nettoyage PDF", nullptr));
        Locaux_m_btnPdf->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "ghost", nullptr)));
        Locaux_m_btnPdf->setObjectName(QCoreApplication::translate("MainWindow", "locauxPdfButton", nullptr));
        Locaux_m_btnMap->setText(QCoreApplication::translate("MainWindow", "Plan de la clinique", nullptr));
        Locaux_m_btnMap->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "ghost", nullptr)));
        Locaux_m_btnMap->setObjectName(QCoreApplication::translate("MainWindow", "locauxMapButton", nullptr));
        Locaux_m_btnStats->setText(QCoreApplication::translate("MainWindow", "Voir les statistiques", nullptr));
        Locaux_m_btnStats->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "ghost", nullptr)));
        Locaux_m_btnStats->setObjectName(QCoreApplication::translate("MainWindow", "locauxStatisticsButton", nullptr));
        Locaux_m_search->setPlaceholderText(QCoreApplication::translate("MainWindow", "Recherche intelligente : \302\253 chirurgie dispo \302\273, \302\253 sale \303\251tage 1 \302\273, \302\253 plein \302\273, \302\253 alerte \302\273\342\200\246", nullptr));
        Locaux_m_search->setObjectName(QCoreApplication::translate("MainWindow", "locauxSearch", nullptr));
        Locaux_m_sortCombo->setItemText(0, QCoreApplication::translate("MainWindow", "Trier : priorit\303\251", nullptr));
        Locaux_m_sortCombo->setItemText(1, QCoreApplication::translate("MainWindow", "Trier : nom A \342\206\222 Z", nullptr));
        Locaux_m_sortCombo->setItemText(2, QCoreApplication::translate("MainWindow", "Trier : remplissage", nullptr));

        Locaux_m_sortCombo->setPlaceholderText(QString());
#if QT_CONFIG(tooltip)
        Locaux_m_sortCombo->setToolTip(QCoreApplication::translate("MainWindow", "Priorit\303\251 : alertes, puis locaux \303\240 nettoyer (blocs d'abord), puis les plus remplis.\n"
"En vue Liste, clique sur les en-t\303\252tes du tableau pour trier par colonne.", nullptr));
#endif // QT_CONFIG(tooltip)
        locauxResetFilters->setText(QCoreApplication::translate("MainWindow", "R\303\251initialiser", nullptr));
        locauxResetFilters->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "link", nullptr)));
        Locaux_m_count->setText(QCoreApplication::translate("MainWindow", "14 / 14 locaux", nullptr));
        Locaux_m_count->setObjectName(QCoreApplication::translate("MainWindow", "sub", nullptr));
        Locaux_m_btnCards->setText(QCoreApplication::translate("MainWindow", "Cartes", nullptr));
        Locaux_m_btnCards->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "ghost", nullptr)));
        Locaux_m_btnList->setText(QCoreApplication::translate("MainWindow", "Liste", nullptr));
        Locaux_m_btnList->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "ghost", nullptr)));
        Locaux_m_btnList->setObjectName(QCoreApplication::translate("MainWindow", "locauxListButton", nullptr));
        Locaux_m_queryHint->setObjectName(QCoreApplication::translate("MainWindow", "sub", nullptr));

        const bool __sortingEnabled = Locaux_m_table->isSortingEnabled();
        Locaux_m_table->setSortingEnabled(false);
        Locaux_m_table->setSortingEnabled(__sortingEnabled);

        Locaux_m_table->setObjectName(QCoreApplication::translate("MainWindow", "locauxTable", nullptr));
        Locaux_m_empty->setText(QCoreApplication::translate("MainWindow", "Aucun local ne correspond \303\240 la recherche", nullptr));
        Locaux_m_empty->setObjectName(QCoreApplication::translate("MainWindow", "sub", nullptr));
        Locaux_m_detail->setObjectName(QCoreApplication::translate("MainWindow", "panel", nullptr));
        Locaux_control_104->setText(QCoreApplication::translate("MainWindow", "PLAN EN DIRECT", nullptr));
        Locaux_control_104->setObjectName(QCoreApplication::translate("MainWindow", "kicker", nullptr));
        locauxEnlargeMap->setText(QCoreApplication::translate("MainWindow", "Agrandir", nullptr));
        locauxEnlargeMap->setProperty("role", QVariant(QCoreApplication::translate("MainWindow", "link", nullptr)));
        Locaux_control_105->setText(QCoreApplication::translate("MainWindow", "<span style='color:#4aa57a'>\342\226\240</span> Dispo &nbsp; <span style='color:#5b76c9'>\342\226\240</span> Occup\303\251 &nbsp; <span style='color:#d9972f'>\342\226\240</span> H.S. &nbsp; <span style='color:#c0504d'>\342\227\217</span> \303\200 nettoyer", nullptr));
        Locaux_control_106->setObjectName(QCoreApplication::translate("MainWindow", "panel", nullptr));
        panelTitle->setText(QCoreApplication::translate("MainWindow", "Occupation globale", nullptr));
        Locaux_m_pct->setText(QCoreApplication::translate("MainWindow", "55 %", nullptr));
        Locaux_m_pct->setObjectName(QCoreApplication::translate("MainWindow", "big", nullptr));
        Locaux_m_bar->setFormat(QCoreApplication::translate("MainWindow", "%p%", nullptr));
        Locaux_m_globalText->setText(QCoreApplication::translate("MainWindow", "33 animaux pour 60 places disponibles", nullptr));
        Locaux_m_globalText->setObjectName(QCoreApplication::translate("MainWindow", "sub", nullptr));
        MedicalPage->setWindowTitle(QCoreApplication::translate("MainWindow", "ANIWORLD \342\200\224 Gestion du centre", nullptr));
        Medical_brandName->setText(QCoreApplication::translate("MainWindow", "ANIWORLD", nullptr));
        Medical_brandTag->setText(QCoreApplication::translate("MainWindow", "THE OFFICIAL APPLICATION", nullptr));
        Medical_sectionTitle->setText(QCoreApplication::translate("MainWindow", "Centre de soins", nullptr));
        Medical_sectionSub->setText(QCoreApplication::translate("MainWindow", "Espace de gestion", nullptr));
        Medical_navLabel->setText(QCoreApplication::translate("MainWindow", "NAVIGATION", nullptr));
        Medical_btnNavDashboard->setText(QCoreApplication::translate("MainWindow", "Tableau de bord", nullptr));
        Medical_btnNavServices->setText(QCoreApplication::translate("MainWindow", "Services", nullptr));
        Medical_btnNavEmployees->setText(QCoreApplication::translate("MainWindow", "Employ\303\251s", nullptr));
        Medical_btnNavStock->setText(QCoreApplication::translate("MainWindow", "Stock", nullptr));
        Medical_btnNavPremises->setText(QCoreApplication::translate("MainWindow", "Locaux", nullptr));
        Medical_btnNavRecords->setText(QCoreApplication::translate("MainWindow", "Dossiers m\303\251dicaux", nullptr));
        Medical_sloganLabel->setText(QCoreApplication::translate("MainWindow", "Une attention \303\240 chaque patte.", nullptr));
        Medical_sloganSub->setText(QCoreApplication::translate("MainWindow", "ANIWORLD \302\267 Centre de soins", nullptr));
        Medical_crumbGestion->setText(QCoreApplication::translate("MainWindow", "Gestion", nullptr));
        Medical_crumbSep->setText(QCoreApplication::translate("MainWindow", "/", nullptr));
        Medical_crumbCurrent->setText(QCoreApplication::translate("MainWindow", "Dossiers m\303\251dicaux", nullptr));
        Medical_demoBadge->setText(QCoreApplication::translate("MainWindow", "Donn\303\251es de d\303\251monstration", nullptr));
        Medical_btnFile->setText(QCoreApplication::translate("MainWindow", "Fichier  \342\226\276", nullptr));
        Medical_roleLabel->setText(QCoreApplication::translate("MainWindow", "Responsable du centre", nullptr));
        Medical_pageTitle->setText(QCoreApplication::translate("MainWindow", "Dossiers m\303\251dicaux", nullptr));
        Medical_pageSubtitle->setText(QCoreApplication::translate("MainWindow", "Suivi de sant\303\251, consultations et vaccinations des animaux", nullptr));
        Medical_btnAdd->setText(QCoreApplication::translate("MainWindow", "+ Ajouter un dossier", nullptr));
        Medical_chipTotal->setText(QString());
        Medical_chipTotal->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "ok", nullptr)));
        Medical_chipConsultations->setText(QString());
        Medical_chipConsultations->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "info", nullptr)));
        Medical_chipUpToDate->setText(QString());
        Medical_chipUpToDate->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "ok", nullptr)));
        Medical_chipToRenew->setText(QString());
        Medical_chipToRenew->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "warn", nullptr)));
        Medical_btnStatsTop->setText(QCoreApplication::translate("MainWindow", "Statistiques", nullptr));
        Medical_searchEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Rechercher un dossier\342\200\246", nullptr));
        Medical_filterSpecies->setItemText(0, QCoreApplication::translate("MainWindow", "Toutes esp\303\250ces", nullptr));
        Medical_filterSpecies->setItemText(1, QCoreApplication::translate("MainWindow", "Chien", nullptr));
        Medical_filterSpecies->setItemText(2, QCoreApplication::translate("MainWindow", "Chat", nullptr));
        Medical_filterSpecies->setItemText(3, QCoreApplication::translate("MainWindow", "Lapin", nullptr));
        Medical_filterSpecies->setItemText(4, QCoreApplication::translate("MainWindow", "Oiseau", nullptr));

        Medical_filterVaccination->setItemText(0, QCoreApplication::translate("MainWindow", "Tous statuts", nullptr));
        Medical_filterVaccination->setItemText(1, QCoreApplication::translate("MainWindow", "\303\200 jour", nullptr));
        Medical_filterVaccination->setItemText(2, QCoreApplication::translate("MainWindow", "\303\200 renouveler", nullptr));

        Medical_sortCombo->setItemText(0, QCoreApplication::translate("MainWindow", "Tri par d\303\251faut", nullptr));
        Medical_sortCombo->setItemText(1, QCoreApplication::translate("MainWindow", "Nom (A\342\200\223Z)", nullptr));
        Medical_sortCombo->setItemText(2, QCoreApplication::translate("MainWindow", "Date r\303\251cente", nullptr));
        Medical_sortCombo->setItemText(3, QCoreApplication::translate("MainWindow", "Propri\303\251taire", nullptr));

        Medical_btnReset->setText(QCoreApplication::translate("MainWindow", "R\303\251initialiser", nullptr));
        Medical_emptyLabel->setText(QCoreApplication::translate("MainWindow", "Aucun dossier m\303\251dical ne correspond \303\240 votre recherche.", nullptr));
        Medical_eyebrow->setText(QCoreApplication::translate("MainWindow", "DOSSIER S\303\211LECTIONN\303\211", nullptr));
        Medical_placeholderText->setText(QCoreApplication::translate("MainWindow", "S\303\251lectionnez un dossier dans la liste pour afficher ses informations et actions.", nullptr));
        Medical_panelTitle->setText(QString());
        Medical_statusPill->setText(QString());
        Medical_detailLine1->setText(QString());
        Medical_detailLine2->setText(QString());
        Medical_detailLine3->setText(QString());
        Medical_btnView->setText(QCoreApplication::translate("MainWindow", "Consulter le dossier", nullptr));
        Medical_btnToggle->setText(QCoreApplication::translate("MainWindow", "Vaccin \303\240 renouveler", nullptr));
        Medical_btnEdit->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        Medical_btnDelete->setText(QCoreApplication::translate("MainWindow", "Supprimer le dossier", nullptr));
        Medical_panelSubTitle->setText(QCoreApplication::translate("MainWindow", "Suivi vaccinal global", nullptr));
        Medical_bigValue->setText(QCoreApplication::translate("MainWindow", "0 %", nullptr));
        Medical_globalText->setText(QString());
        Medical_btnStatsPanel->setText(QCoreApplication::translate("MainWindow", "Voir les statistiques", nullptr));
        Medical_footerLeft->setText(QCoreApplication::translate("MainWindow", "ANIWORLD \302\267 Gestion du centre", nullptr));
        Medical_footerDate->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_CHECK_H
