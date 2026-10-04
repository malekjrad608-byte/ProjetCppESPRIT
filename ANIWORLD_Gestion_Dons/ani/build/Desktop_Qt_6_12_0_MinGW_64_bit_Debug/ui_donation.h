/********************************************************************************
** Form generated from reading UI file 'donation.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DONATION_H
#define UI_DONATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DonationWindow
{
public:
    QWidget *centralWidget;
    QHBoxLayout *outerLayout;
    QFrame *sidebar;
    QVBoxLayout *sidebarLayout;
    QHBoxLayout *brandLayout;
    QLabel *brandLogo;
    QVBoxLayout *brandTextLayout;
    QLabel *brandName;
    QLabel *brandSub;
    QSpacerItem *sidebarTopGap;
    QLabel *navTitle;
    QPushButton *btnDashboard;
    QPushButton *navServices;
    QPushButton *navEmployees;
    QPushButton *navStock;
    QPushButton *navLocaux;
    QPushButton *navMedical;
    QPushButton *navDonations;
    QSpacerItem *sidebarBottomGap;
    QLabel *sideWarning;
    QLabel *footerInfo;
    QVBoxLayout *mainLayout;
    QHBoxLayout *topMetaLayout;
    QLabel *breadcrumb;
    QSpacerItem *metaSpacer;
    QLabel *modeBadge;
    QLabel *roleLabel;
    QHBoxLayout *titleLayout;
    QVBoxLayout *headerLayout;
    QLabel *pageTitle;
    QLabel *pageSubtitle;
    QSpacerItem *titleSpacer;
    QPushButton *btnStatistics;
    QPushButton *btnNew;
    QHBoxLayout *statsLayout;
    QFrame *statCard;
    QVBoxLayout *statLayout1;
    QLabel *statCaption;
    QLabel *statTotal;
    QFrame *statCard2;
    QVBoxLayout *statLayout2;
    QLabel *statCaption2;
    QLabel *statDonors;
    QFrame *statCard3;
    QVBoxLayout *statLayout3;
    QLabel *statCaption3;
    QLabel *statAverage;
    QSpacerItem *statsSpacer;
    QFrame *filtersPanel;
    QVBoxLayout *filtersLayout;
    QHBoxLayout *filterTop;
    QLineEdit *editSearch;
    QComboBox *comboStatus;
    QPushButton *btnExportCsv;
    QPushButton *btnExportPdf;
    QHBoxLayout *filterBottom;
    QComboBox *comboType;
    QComboBox *comboSort;
    QPushButton *btnReset;
    QSpacerItem *filterSpacer;
    QLabel *countShown;
    QHBoxLayout *contentRow;
    QFrame *cardsPanel;
    QVBoxLayout *cardsPanelLayout;
    QScrollArea *cardsScroll;
    QWidget *cardsContent;
    QGridLayout *cardsGrid;
    QPushButton *card0;
    QPushButton *card1;
    QPushButton *card2;
    QPushButton *card3;
    QPushButton *card4;
    QPushButton *card5;
    QPushButton *card6;
    QPushButton *card7;
    QFrame *detailPanel;
    QVBoxLayout *detailPanelLayout;
    QLabel *detailKicker;
    QLabel *detailId;
    QHBoxLayout *badgeRow;
    QLabel *detailStatus;
    QLabel *recurringBadge;
    QLabel *anomalyBadge;
    QSpacerItem *badgeSpacer;
    QFrame *detailDataFrame;
    QVBoxLayout *detailDataLayout;
    QHBoxLayout *detailRow1;
    QLabel *detailLabel;
    QLineEdit *detailDonor;
    QHBoxLayout *detailRow2;
    QLabel *detailLabel2;
    QLineEdit *detailAmount;
    QHBoxLayout *detailRow3;
    QLabel *detailLabel3;
    QLineEdit *detailDate;
    QHBoxLayout *detailRow4;
    QLabel *detailLabel4;
    QLineEdit *detailPayment;
    QHBoxLayout *detailRow5;
    QLabel *detailLabel5;
    QLineEdit *detailType;
    QHBoxLayout *detailRow6;
    QLabel *detailLabel6;
    QLineEdit *detailCampaign;
    QHBoxLayout *actionTop;
    QPushButton *btnRecurring;
    QPushButton *btnAnomalies;
    QHBoxLayout *actionRow1;
    QPushButton *btnConsult;
    QPushButton *btnEdit;
    QHBoxLayout *actionRow2;
    QPushButton *btnArchive;
    QPushButton *btnDelete;

    void setupUi(QMainWindow *DonationWindow)
    {
        if (DonationWindow->objectName().isEmpty())
            DonationWindow->setObjectName("DonationWindow");
        DonationWindow->resize(1440, 900);
        DonationWindow->setMinimumSize(QSize(1180, 760));
        DonationWindow->setStyleSheet(QString::fromUtf8("\n"
"QMainWindow{background:#F4F7F8;}\n"
"QWidget{font-family:'Segoe UI';color:#263B42;}\n"
"QFrame#sidebar{background:#FFFFFF;border-right:1px solid #DCE6E9;}\n"
"QLabel#brandName{color:#225A64;font-size:18pt;font-weight:700;}\n"
"QLabel#brandSub{color:#728288;font-size:9pt;}\n"
"QLabel#navTitle{color:#7C8D92;font-size:9pt;font-weight:700;}\n"
"QPushButton#btnDashboard,QPushButton#navServices,QPushButton#navEmployees,QPushButton#navStock,QPushButton#navLocaux,QPushButton#navMedical{background:transparent;color:#4D6269;border:0;border-radius:7px;text-align:left;padding:10px 12px;min-height:20px;}\n"
"QPushButton#btnDashboard:hover,QPushButton#navServices:hover,QPushButton#navEmployees:hover,QPushButton#navStock:hover,QPushButton#navLocaux:hover,QPushButton#navMedical:hover{background:#F4F7F8;color:#225A64;}\n"
"QPushButton#navDonations{background:#E4F0F3;color:#225A64;border:0;border-radius:7px;text-align:left;padding:10px 12px;font-weight:700;}\n"
"QLabel#sideWarning{color:#728288;font-size:8pt;}\n"
"QLabel#b"
                        "readcrumb{color:#819197;font-size:9pt;}\n"
"QLabel#pageTitle{color:#225A64;font-size:27pt;font-weight:700;}\n"
"QLabel#pageSubtitle{color:#728288;font-size:10pt;}\n"
"QLabel#roleLabel{color:#5E747C;font-size:9pt;}\n"
"QLabel#modeBadge{background:#E6F3EC;color:#397A5C;border-radius:7px;padding:7px 12px;}\n"
"QPushButton#btnStatistics,QPushButton#btnExportCsv,QPushButton#btnExportPdf,QPushButton#btnReset{background:#FFFFFF;color:#225A64;border:1px solid #D6E2E5;border-radius:7px;padding:8px 12px;min-height:22px;}\n"
"QPushButton#btnStatistics:hover,QPushButton#btnExportCsv:hover,QPushButton#btnExportPdf:hover,QPushButton#btnReset:hover{background:#E4F0F3;}\n"
"QPushButton#btnNew{background:#225A64;color:#FFFFFF;border:0;border-radius:7px;padding:9px 16px;font-weight:700;min-height:22px;}\n"
"QPushButton#btnNew:hover{background:#397F8D;}\n"
"QFrame#statCard,QFrame#statCard2,QFrame#statCard3,QFrame#filtersPanel,QFrame#cardsPanel,QFrame#detailPanel{background:#FFFFFF;border:1px solid #DDE7E9;border-radius:10px;}\n"
""
                        "QLabel#statCaption,QLabel#statCaption2,QLabel#statCaption3{color:#788A90;font-size:9pt;}\n"
"QLabel#statTotal,QLabel#statDonors,QLabel#statAverage{color:#225A64;font-size:18pt;font-weight:700;}\n"
"QLabel#statAverage{color:#397F8D;}\n"
"QLineEdit#editSearch,QComboBox#comboStatus,QComboBox#comboType,QComboBox#comboSort{background:#FFFFFF;color:#263B42;border:1px solid #D6E2E5;border-radius:6px;padding:7px 10px;min-height:28px;}\n"
"QLineEdit#editSearch:focus,QComboBox#comboStatus:focus,QComboBox#comboType:focus,QComboBox#comboSort:focus{border:1px solid #397F8D;}\n"
"QLabel#countShown{color:#7B8D93;font-size:9pt;}\n"
"QScrollArea#cardsScroll{background:#FFFFFF;border:0;}\n"
"QWidget#cardsContent{background:#FFFFFF;}\n"
"QLabel#detailKicker{color:#7B8C92;font-size:9pt;font-weight:700;letter-spacing:1px;}\n"
"QLabel#detailId{color:#225A64;font-size:19pt;font-weight:700;}\n"
"QLabel#detailStatus{background:#FFF2D9;color:#9A6A18;border-radius:6px;padding:6px 10px;}\n"
"QLabel#recurringBadge{background:#E6F3EC;color"
                        ":#397A5C;border-radius:6px;padding:6px 10px;}\n"
"QLabel#anomalyBadge{background:#FBE9E7;color:#AE5751;border-radius:6px;padding:6px 10px;}\n"
"QLabel#detailLabel,QLabel#detailLabel2,QLabel#detailLabel3,QLabel#detailLabel4,QLabel#detailLabel5,QLabel#detailLabel6{color:#728288;font-size:8.5pt;font-weight:600;}\n"
"QLineEdit#detailDonor,QLineEdit#detailAmount,QLineEdit#detailDate,QLineEdit#detailPayment,QLineEdit#detailType,QLineEdit#detailCampaign{background:#F9FBFC;color:#263B42;border:1px solid #DCE6E9;border-radius:6px;padding:5px 9px;font-weight:600;min-height:32px;max-height:32px;}\n"
"QLabel#footerInfo{color:#8A989D;font-size:8pt;}\n"
"QPushButton#btnRecurring,QPushButton#btnAnomalies,QPushButton#btnConsult,QPushButton#btnEdit,QPushButton#btnArchive,QPushButton#btnDelete{background:#FFFFFF;color:#225A64;border:1px solid #D7E2E5;border-radius:6px;padding:7px 10px;min-height:20px;}\n"
"QPushButton#btnRecurring:hover,QPushButton#btnAnomalies:hover,QPushButton#btnConsult:hover,QPushButton#btnEdit:hover,QPushB"
                        "utton#btnArchive:hover{background:#E4F0F3;}\n"
"QPushButton#btnDelete{color:#AE5751;border-color:#E3C4C1;}\n"
"QPushButton#btnDelete:hover{background:#FBE9E7;}\n"
"QGridLayout{margin:0;}\n"
"  "));
        centralWidget = new QWidget(DonationWindow);
        centralWidget->setObjectName("centralWidget");
        outerLayout = new QHBoxLayout(centralWidget);
        outerLayout->setSpacing(0);
        outerLayout->setObjectName("outerLayout");
        outerLayout->setContentsMargins(0, 0, 0, 0);
        sidebar = new QFrame(centralWidget);
        sidebar->setObjectName("sidebar");
        sidebar->setMinimumSize(QSize(225, 0));
        sidebar->setMaximumSize(QSize(225, 16777215));
        sidebarLayout = new QVBoxLayout(sidebar);
        sidebarLayout->setSpacing(8);
        sidebarLayout->setObjectName("sidebarLayout");
        sidebarLayout->setContentsMargins(16, 18, 16, 14);
        brandLayout = new QHBoxLayout();
        brandLayout->setSpacing(8);
        brandLayout->setObjectName("brandLayout");
        brandLogo = new QLabel(sidebar);
        brandLogo->setObjectName("brandLogo");
        brandLogo->setMinimumSize(QSize(40, 40));
        brandLogo->setMaximumSize(QSize(40, 40));
        brandLogo->setPixmap(QPixmap(QString::fromUtf8(":/assets/logo_aniworld_transparent.png")));
        brandLogo->setScaledContents(true);

        brandLayout->addWidget(brandLogo);

        brandTextLayout = new QVBoxLayout();
        brandTextLayout->setSpacing(0);
        brandTextLayout->setObjectName("brandTextLayout");
        brandName = new QLabel(sidebar);
        brandName->setObjectName("brandName");

        brandTextLayout->addWidget(brandName);

        brandSub = new QLabel(sidebar);
        brandSub->setObjectName("brandSub");

        brandTextLayout->addWidget(brandSub);


        brandLayout->addLayout(brandTextLayout);


        sidebarLayout->addLayout(brandLayout);

        sidebarTopGap = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);

        sidebarLayout->addItem(sidebarTopGap);

        navTitle = new QLabel(sidebar);
        navTitle->setObjectName("navTitle");

        sidebarLayout->addWidget(navTitle);

        btnDashboard = new QPushButton(sidebar);
        btnDashboard->setObjectName("btnDashboard");

        sidebarLayout->addWidget(btnDashboard);

        navServices = new QPushButton(sidebar);
        navServices->setObjectName("navServices");

        sidebarLayout->addWidget(navServices);

        navEmployees = new QPushButton(sidebar);
        navEmployees->setObjectName("navEmployees");

        sidebarLayout->addWidget(navEmployees);

        navStock = new QPushButton(sidebar);
        navStock->setObjectName("navStock");

        sidebarLayout->addWidget(navStock);

        navLocaux = new QPushButton(sidebar);
        navLocaux->setObjectName("navLocaux");

        sidebarLayout->addWidget(navLocaux);

        navMedical = new QPushButton(sidebar);
        navMedical->setObjectName("navMedical");

        sidebarLayout->addWidget(navMedical);

        navDonations = new QPushButton(sidebar);
        navDonations->setObjectName("navDonations");

        sidebarLayout->addWidget(navDonations);

        sidebarBottomGap = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        sidebarLayout->addItem(sidebarBottomGap);

        sideWarning = new QLabel(sidebar);
        sideWarning->setObjectName("sideWarning");
        sideWarning->setWordWrap(true);

        sidebarLayout->addWidget(sideWarning);

        footerInfo = new QLabel(sidebar);
        footerInfo->setObjectName("footerInfo");

        sidebarLayout->addWidget(footerInfo);


        outerLayout->addWidget(sidebar);

        mainLayout = new QVBoxLayout();
        mainLayout->setSpacing(12);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(24, 18, 24, 12);
        topMetaLayout = new QHBoxLayout();
        topMetaLayout->setSpacing(8);
        topMetaLayout->setObjectName("topMetaLayout");
        breadcrumb = new QLabel(centralWidget);
        breadcrumb->setObjectName("breadcrumb");

        topMetaLayout->addWidget(breadcrumb);

        metaSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        topMetaLayout->addItem(metaSpacer);

        modeBadge = new QLabel(centralWidget);
        modeBadge->setObjectName("modeBadge");

        topMetaLayout->addWidget(modeBadge);

        roleLabel = new QLabel(centralWidget);
        roleLabel->setObjectName("roleLabel");

        topMetaLayout->addWidget(roleLabel);


        mainLayout->addLayout(topMetaLayout);

        titleLayout = new QHBoxLayout();
        titleLayout->setSpacing(10);
        titleLayout->setObjectName("titleLayout");
        headerLayout = new QVBoxLayout();
        headerLayout->setSpacing(2);
        headerLayout->setObjectName("headerLayout");
        pageTitle = new QLabel(centralWidget);
        pageTitle->setObjectName("pageTitle");

        headerLayout->addWidget(pageTitle);

        pageSubtitle = new QLabel(centralWidget);
        pageSubtitle->setObjectName("pageSubtitle");

        headerLayout->addWidget(pageSubtitle);


        titleLayout->addLayout(headerLayout);

        titleSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        titleLayout->addItem(titleSpacer);

        btnStatistics = new QPushButton(centralWidget);
        btnStatistics->setObjectName("btnStatistics");

        titleLayout->addWidget(btnStatistics);

        btnNew = new QPushButton(centralWidget);
        btnNew->setObjectName("btnNew");

        titleLayout->addWidget(btnNew);


        mainLayout->addLayout(titleLayout);

        statsLayout = new QHBoxLayout();
        statsLayout->setSpacing(10);
        statsLayout->setObjectName("statsLayout");
        statCard = new QFrame(centralWidget);
        statCard->setObjectName("statCard");
        statCard->setMinimumSize(QSize(240, 88));
        statLayout1 = new QVBoxLayout(statCard);
        statLayout1->setSpacing(3);
        statLayout1->setObjectName("statLayout1");
        statLayout1->setContentsMargins(16, 12, 16, 12);
        statCaption = new QLabel(statCard);
        statCaption->setObjectName("statCaption");

        statLayout1->addWidget(statCaption);

        statTotal = new QLabel(statCard);
        statTotal->setObjectName("statTotal");

        statLayout1->addWidget(statTotal);


        statsLayout->addWidget(statCard);

        statCard2 = new QFrame(centralWidget);
        statCard2->setObjectName("statCard2");
        statCard2->setMinimumSize(QSize(180, 88));
        statLayout2 = new QVBoxLayout(statCard2);
        statLayout2->setSpacing(3);
        statLayout2->setObjectName("statLayout2");
        statLayout2->setContentsMargins(16, 12, 16, 12);
        statCaption2 = new QLabel(statCard2);
        statCaption2->setObjectName("statCaption2");

        statLayout2->addWidget(statCaption2);

        statDonors = new QLabel(statCard2);
        statDonors->setObjectName("statDonors");

        statLayout2->addWidget(statDonors);


        statsLayout->addWidget(statCard2);

        statCard3 = new QFrame(centralWidget);
        statCard3->setObjectName("statCard3");
        statCard3->setMinimumSize(QSize(200, 88));
        statLayout3 = new QVBoxLayout(statCard3);
        statLayout3->setSpacing(3);
        statLayout3->setObjectName("statLayout3");
        statLayout3->setContentsMargins(16, 12, 16, 12);
        statCaption3 = new QLabel(statCard3);
        statCaption3->setObjectName("statCaption3");

        statLayout3->addWidget(statCaption3);

        statAverage = new QLabel(statCard3);
        statAverage->setObjectName("statAverage");

        statLayout3->addWidget(statAverage);


        statsLayout->addWidget(statCard3);

        statsSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        statsLayout->addItem(statsSpacer);


        mainLayout->addLayout(statsLayout);

        filtersPanel = new QFrame(centralWidget);
        filtersPanel->setObjectName("filtersPanel");
        filtersLayout = new QVBoxLayout(filtersPanel);
        filtersLayout->setSpacing(8);
        filtersLayout->setObjectName("filtersLayout");
        filtersLayout->setContentsMargins(10, 10, 10, 10);
        filterTop = new QHBoxLayout();
        filterTop->setSpacing(8);
        filterTop->setObjectName("filterTop");
        editSearch = new QLineEdit(filtersPanel);
        editSearch->setObjectName("editSearch");

        filterTop->addWidget(editSearch);

        comboStatus = new QComboBox(filtersPanel);
        comboStatus->addItem(QString());
        comboStatus->addItem(QString());
        comboStatus->addItem(QString());
        comboStatus->addItem(QString());
        comboStatus->addItem(QString());
        comboStatus->setObjectName("comboStatus");

        filterTop->addWidget(comboStatus);

        btnExportCsv = new QPushButton(filtersPanel);
        btnExportCsv->setObjectName("btnExportCsv");

        filterTop->addWidget(btnExportCsv);

        btnExportPdf = new QPushButton(filtersPanel);
        btnExportPdf->setObjectName("btnExportPdf");

        filterTop->addWidget(btnExportPdf);


        filtersLayout->addLayout(filterTop);

        filterBottom = new QHBoxLayout();
        filterBottom->setSpacing(8);
        filterBottom->setObjectName("filterBottom");
        comboType = new QComboBox(filtersPanel);
        comboType->addItem(QString());
        comboType->addItem(QString());
        comboType->addItem(QString());
        comboType->addItem(QString());
        comboType->setObjectName("comboType");

        filterBottom->addWidget(comboType);

        comboSort = new QComboBox(filtersPanel);
        comboSort->addItem(QString());
        comboSort->addItem(QString());
        comboSort->addItem(QString());
        comboSort->addItem(QString());
        comboSort->setObjectName("comboSort");

        filterBottom->addWidget(comboSort);

        btnReset = new QPushButton(filtersPanel);
        btnReset->setObjectName("btnReset");

        filterBottom->addWidget(btnReset);

        filterSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        filterBottom->addItem(filterSpacer);

        countShown = new QLabel(filtersPanel);
        countShown->setObjectName("countShown");

        filterBottom->addWidget(countShown);


        filtersLayout->addLayout(filterBottom);


        mainLayout->addWidget(filtersPanel);

        contentRow = new QHBoxLayout();
        contentRow->setSpacing(12);
        contentRow->setObjectName("contentRow");
        cardsPanel = new QFrame(centralWidget);
        cardsPanel->setObjectName("cardsPanel");
        cardsPanel->setMinimumSize(QSize(620, 430));
        cardsPanelLayout = new QVBoxLayout(cardsPanel);
        cardsPanelLayout->setSpacing(0);
        cardsPanelLayout->setObjectName("cardsPanelLayout");
        cardsPanelLayout->setContentsMargins(10, 10, 10, 10);
        cardsScroll = new QScrollArea(cardsPanel);
        cardsScroll->setObjectName("cardsScroll");
        cardsScroll->setFrameShape(QFrame::Shape::NoFrame);
        cardsScroll->setWidgetResizable(true);
        cardsContent = new QWidget();
        cardsContent->setObjectName("cardsContent");
        cardsContent->setGeometry(QRect(0, 0, 684, 478));
        cardsGrid = new QGridLayout(cardsContent);
        cardsGrid->setSpacing(10);
        cardsGrid->setObjectName("cardsGrid");
        cardsGrid->setContentsMargins(0, 0, 0, 0);
        card0 = new QPushButton(cardsContent);
        card0->setObjectName("card0");
        card0->setMinimumSize(QSize(280, 112));

        cardsGrid->addWidget(card0, 0, 0, 1, 1);

        card1 = new QPushButton(cardsContent);
        card1->setObjectName("card1");
        card1->setMinimumSize(QSize(280, 112));

        cardsGrid->addWidget(card1, 0, 1, 1, 1);

        card2 = new QPushButton(cardsContent);
        card2->setObjectName("card2");
        card2->setMinimumSize(QSize(280, 112));

        cardsGrid->addWidget(card2, 1, 0, 1, 1);

        card3 = new QPushButton(cardsContent);
        card3->setObjectName("card3");
        card3->setMinimumSize(QSize(280, 112));

        cardsGrid->addWidget(card3, 1, 1, 1, 1);

        card4 = new QPushButton(cardsContent);
        card4->setObjectName("card4");
        card4->setMinimumSize(QSize(280, 112));

        cardsGrid->addWidget(card4, 2, 0, 1, 1);

        card5 = new QPushButton(cardsContent);
        card5->setObjectName("card5");
        card5->setMinimumSize(QSize(280, 112));

        cardsGrid->addWidget(card5, 2, 1, 1, 1);

        card6 = new QPushButton(cardsContent);
        card6->setObjectName("card6");
        card6->setMinimumSize(QSize(280, 112));

        cardsGrid->addWidget(card6, 3, 0, 1, 1);

        card7 = new QPushButton(cardsContent);
        card7->setObjectName("card7");
        card7->setMinimumSize(QSize(280, 112));

        cardsGrid->addWidget(card7, 3, 1, 1, 1);

        cardsScroll->setWidget(cardsContent);

        cardsPanelLayout->addWidget(cardsScroll);


        contentRow->addWidget(cardsPanel);

        detailPanel = new QFrame(centralWidget);
        detailPanel->setObjectName("detailPanel");
        detailPanel->setMinimumSize(QSize(430, 430));
        detailPanelLayout = new QVBoxLayout(detailPanel);
        detailPanelLayout->setSpacing(8);
        detailPanelLayout->setObjectName("detailPanelLayout");
        detailPanelLayout->setContentsMargins(16, 14, 16, 14);
        detailKicker = new QLabel(detailPanel);
        detailKicker->setObjectName("detailKicker");

        detailPanelLayout->addWidget(detailKicker);

        detailId = new QLabel(detailPanel);
        detailId->setObjectName("detailId");

        detailPanelLayout->addWidget(detailId);

        badgeRow = new QHBoxLayout();
        badgeRow->setSpacing(6);
        badgeRow->setObjectName("badgeRow");
        detailStatus = new QLabel(detailPanel);
        detailStatus->setObjectName("detailStatus");

        badgeRow->addWidget(detailStatus);

        recurringBadge = new QLabel(detailPanel);
        recurringBadge->setObjectName("recurringBadge");
        recurringBadge->setVisible(false);

        badgeRow->addWidget(recurringBadge);

        anomalyBadge = new QLabel(detailPanel);
        anomalyBadge->setObjectName("anomalyBadge");
        anomalyBadge->setVisible(false);

        badgeRow->addWidget(anomalyBadge);

        badgeSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        badgeRow->addItem(badgeSpacer);


        detailPanelLayout->addLayout(badgeRow);

        detailDataFrame = new QFrame(detailPanel);
        detailDataFrame->setObjectName("detailDataFrame");
        detailDataFrame->setStyleSheet(QString::fromUtf8("QFrame#detailDataFrame{background:#FFFFFF;border:0;}"));
        detailDataLayout = new QVBoxLayout(detailDataFrame);
        detailDataLayout->setSpacing(6);
        detailDataLayout->setObjectName("detailDataLayout");
        detailDataLayout->setContentsMargins(0, 0, 0, 0);
        detailRow1 = new QHBoxLayout();
        detailRow1->setSpacing(10);
        detailRow1->setObjectName("detailRow1");
        detailLabel = new QLabel(detailDataFrame);
        detailLabel->setObjectName("detailLabel");
        detailLabel->setMinimumSize(QSize(80, 32));
        detailLabel->setMaximumSize(QSize(80, 32));
        detailLabel->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);

        detailRow1->addWidget(detailLabel);

        detailDonor = new QLineEdit(detailDataFrame);
        detailDonor->setObjectName("detailDonor");
        detailDonor->setReadOnly(true);

        detailRow1->addWidget(detailDonor);


        detailDataLayout->addLayout(detailRow1);

        detailRow2 = new QHBoxLayout();
        detailRow2->setSpacing(10);
        detailRow2->setObjectName("detailRow2");
        detailLabel2 = new QLabel(detailDataFrame);
        detailLabel2->setObjectName("detailLabel2");
        detailLabel2->setMinimumSize(QSize(80, 32));
        detailLabel2->setMaximumSize(QSize(80, 32));
        detailLabel2->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);

        detailRow2->addWidget(detailLabel2);

        detailAmount = new QLineEdit(detailDataFrame);
        detailAmount->setObjectName("detailAmount");
        detailAmount->setReadOnly(true);

        detailRow2->addWidget(detailAmount);


        detailDataLayout->addLayout(detailRow2);

        detailRow3 = new QHBoxLayout();
        detailRow3->setSpacing(10);
        detailRow3->setObjectName("detailRow3");
        detailLabel3 = new QLabel(detailDataFrame);
        detailLabel3->setObjectName("detailLabel3");
        detailLabel3->setMinimumSize(QSize(80, 32));
        detailLabel3->setMaximumSize(QSize(80, 32));
        detailLabel3->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);

        detailRow3->addWidget(detailLabel3);

        detailDate = new QLineEdit(detailDataFrame);
        detailDate->setObjectName("detailDate");
        detailDate->setReadOnly(true);

        detailRow3->addWidget(detailDate);


        detailDataLayout->addLayout(detailRow3);

        detailRow4 = new QHBoxLayout();
        detailRow4->setSpacing(10);
        detailRow4->setObjectName("detailRow4");
        detailLabel4 = new QLabel(detailDataFrame);
        detailLabel4->setObjectName("detailLabel4");
        detailLabel4->setMinimumSize(QSize(80, 32));
        detailLabel4->setMaximumSize(QSize(80, 32));
        detailLabel4->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);

        detailRow4->addWidget(detailLabel4);

        detailPayment = new QLineEdit(detailDataFrame);
        detailPayment->setObjectName("detailPayment");
        detailPayment->setReadOnly(true);

        detailRow4->addWidget(detailPayment);


        detailDataLayout->addLayout(detailRow4);

        detailRow5 = new QHBoxLayout();
        detailRow5->setSpacing(10);
        detailRow5->setObjectName("detailRow5");
        detailLabel5 = new QLabel(detailDataFrame);
        detailLabel5->setObjectName("detailLabel5");
        detailLabel5->setMinimumSize(QSize(80, 32));
        detailLabel5->setMaximumSize(QSize(80, 32));
        detailLabel5->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);

        detailRow5->addWidget(detailLabel5);

        detailType = new QLineEdit(detailDataFrame);
        detailType->setObjectName("detailType");
        detailType->setReadOnly(true);

        detailRow5->addWidget(detailType);


        detailDataLayout->addLayout(detailRow5);

        detailRow6 = new QHBoxLayout();
        detailRow6->setSpacing(10);
        detailRow6->setObjectName("detailRow6");
        detailLabel6 = new QLabel(detailDataFrame);
        detailLabel6->setObjectName("detailLabel6");
        detailLabel6->setMinimumSize(QSize(80, 32));
        detailLabel6->setMaximumSize(QSize(80, 32));
        detailLabel6->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);

        detailRow6->addWidget(detailLabel6);

        detailCampaign = new QLineEdit(detailDataFrame);
        detailCampaign->setObjectName("detailCampaign");
        detailCampaign->setReadOnly(true);

        detailRow6->addWidget(detailCampaign);


        detailDataLayout->addLayout(detailRow6);


        detailPanelLayout->addWidget(detailDataFrame);

        actionTop = new QHBoxLayout();
        actionTop->setSpacing(7);
        actionTop->setObjectName("actionTop");
        btnRecurring = new QPushButton(detailPanel);
        btnRecurring->setObjectName("btnRecurring");

        actionTop->addWidget(btnRecurring);

        btnAnomalies = new QPushButton(detailPanel);
        btnAnomalies->setObjectName("btnAnomalies");

        actionTop->addWidget(btnAnomalies);


        detailPanelLayout->addLayout(actionTop);

        actionRow1 = new QHBoxLayout();
        actionRow1->setSpacing(7);
        actionRow1->setObjectName("actionRow1");
        btnConsult = new QPushButton(detailPanel);
        btnConsult->setObjectName("btnConsult");

        actionRow1->addWidget(btnConsult);

        btnEdit = new QPushButton(detailPanel);
        btnEdit->setObjectName("btnEdit");

        actionRow1->addWidget(btnEdit);


        detailPanelLayout->addLayout(actionRow1);

        actionRow2 = new QHBoxLayout();
        actionRow2->setSpacing(7);
        actionRow2->setObjectName("actionRow2");
        btnArchive = new QPushButton(detailPanel);
        btnArchive->setObjectName("btnArchive");

        actionRow2->addWidget(btnArchive);

        btnDelete = new QPushButton(detailPanel);
        btnDelete->setObjectName("btnDelete");

        actionRow2->addWidget(btnDelete);


        detailPanelLayout->addLayout(actionRow2);


        contentRow->addWidget(detailPanel);


        mainLayout->addLayout(contentRow);


        outerLayout->addLayout(mainLayout);

        DonationWindow->setCentralWidget(centralWidget);

        retranslateUi(DonationWindow);

        QMetaObject::connectSlotsByName(DonationWindow);
    } // setupUi

    void retranslateUi(QMainWindow *DonationWindow)
    {
        DonationWindow->setWindowTitle(QCoreApplication::translate("DonationWindow", "ANIWORLD \342\200\224 Gestion des Dons", nullptr));
        brandName->setText(QCoreApplication::translate("DonationWindow", "ANIWORLD", nullptr));
        brandSub->setText(QCoreApplication::translate("DonationWindow", "Les Compagnons", nullptr));
        navTitle->setText(QCoreApplication::translate("DonationWindow", "NAVIGATION", nullptr));
        btnDashboard->setText(QCoreApplication::translate("DonationWindow", "Tableau de bord", nullptr));
        navServices->setText(QCoreApplication::translate("DonationWindow", "Services", nullptr));
        navEmployees->setText(QCoreApplication::translate("DonationWindow", "Employ\303\251s", nullptr));
        navStock->setText(QCoreApplication::translate("DonationWindow", "Stock", nullptr));
        navLocaux->setText(QCoreApplication::translate("DonationWindow", "Locaux", nullptr));
        navMedical->setText(QCoreApplication::translate("DonationWindow", "Dossiers m\303\251dicaux", nullptr));
        navDonations->setText(QCoreApplication::translate("DonationWindow", "Dons", nullptr));
        sideWarning->setText(QCoreApplication::translate("DonationWindow", "Une attention \303\240 chaque contribution.", nullptr));
        footerInfo->setText(QCoreApplication::translate("DonationWindow", "ANIWORLD \302\267 Gestion des Dons", nullptr));
        breadcrumb->setText(QCoreApplication::translate("DonationWindow", "Gestion  /  Dons", nullptr));
        modeBadge->setText(QCoreApplication::translate("DonationWindow", "Maquette \302\267 donn\303\251es fictives", nullptr));
        roleLabel->setText(QCoreApplication::translate("DonationWindow", "Responsable du centre", nullptr));
        pageTitle->setText(QCoreApplication::translate("DonationWindow", "Gestion des Dons", nullptr));
        pageSubtitle->setText(QCoreApplication::translate("DonationWindow", "Donateurs, contributions et suivi des dons", nullptr));
        btnStatistics->setText(QCoreApplication::translate("DonationWindow", "Statistiques", nullptr));
        btnNew->setText(QCoreApplication::translate("DonationWindow", "+ Nouveau don", nullptr));
        statCaption->setText(QCoreApplication::translate("DonationWindow", "Montant confirm\303\251", nullptr));
        statTotal->setText(QCoreApplication::translate("DonationWindow", "0.000 TND", nullptr));
        statCaption2->setText(QCoreApplication::translate("DonationWindow", "Dons confirm\303\251s", nullptr));
        statDonors->setText(QCoreApplication::translate("DonationWindow", "0", nullptr));
        statCaption3->setText(QCoreApplication::translate("DonationWindow", "Don moyen", nullptr));
        statAverage->setText(QCoreApplication::translate("DonationWindow", "0.000 TND", nullptr));
        editSearch->setPlaceholderText(QCoreApplication::translate("DonationWindow", "Rechercher un donateur, un ID ou une campagne...", nullptr));
        comboStatus->setItemText(0, QCoreApplication::translate("DonationWindow", "Tous les statuts", nullptr));
        comboStatus->setItemText(1, QCoreApplication::translate("DonationWindow", "Confirm\303\251", nullptr));
        comboStatus->setItemText(2, QCoreApplication::translate("DonationWindow", "En attente", nullptr));
        comboStatus->setItemText(3, QCoreApplication::translate("DonationWindow", "\303\200 v\303\251rifier", nullptr));
        comboStatus->setItemText(4, QCoreApplication::translate("DonationWindow", "Archiv\303\251", nullptr));

        btnExportCsv->setText(QCoreApplication::translate("DonationWindow", "Exporter CSV", nullptr));
        btnExportPdf->setText(QCoreApplication::translate("DonationWindow", "Exporter PDF", nullptr));
        comboType->setItemText(0, QCoreApplication::translate("DonationWindow", "Tous les types", nullptr));
        comboType->setItemText(1, QCoreApplication::translate("DonationWindow", "Ponctuel", nullptr));
        comboType->setItemText(2, QCoreApplication::translate("DonationWindow", "R\303\251current", nullptr));
        comboType->setItemText(3, QCoreApplication::translate("DonationWindow", "Campagne", nullptr));

        comboSort->setItemText(0, QCoreApplication::translate("DonationWindow", "Plus r\303\251centes", nullptr));
        comboSort->setItemText(1, QCoreApplication::translate("DonationWindow", "Date ancienne \342\206\222 r\303\251cente", nullptr));
        comboSort->setItemText(2, QCoreApplication::translate("DonationWindow", "Montant d\303\251croissant", nullptr));
        comboSort->setItemText(3, QCoreApplication::translate("DonationWindow", "Donateur A \342\206\222 Z", nullptr));

        btnReset->setText(QCoreApplication::translate("DonationWindow", "R\303\251initialiser", nullptr));
        countShown->setText(QCoreApplication::translate("DonationWindow", "0 don(s) affich\303\251(s)", nullptr));
        card0->setText(QCoreApplication::translate("DonationWindow", "DON-001   Ahmed Ben Ali\\n02/10/2026 \302\267 Ponctuel\\n150.000 TND\\nConfirm\303\251", nullptr));
        card1->setText(QCoreApplication::translate("DonationWindow", "DON-002   Soci\303\251t\303\251 Nour\\n28/09/2026 \302\267 Campagne\\n500.000 TND\\nConfirm\303\251", nullptr));
        card2->setText(QCoreApplication::translate("DonationWindow", "DON-003   Sami H.\\n25/09/2026 \302\267 Ponctuel\\n75.000 TND\\nConfirm\303\251", nullptr));
        card3->setText(QCoreApplication::translate("DonationWindow", "DON-004   Nour B.\\n21/09/2026 \302\267 R\303\251current\\n100.000 TND\\nConfirm\303\251", nullptr));
        card4->setText(QCoreApplication::translate("DonationWindow", "DON-005   Sara Gharbi\\n18/09/2026 \302\267 Ponctuel\\n250.000 TND\\nEn attente", nullptr));
        card5->setText(QCoreApplication::translate("DonationWindow", "DON-006   Clinique El Amal\\n14/09/2026 \302\267 Campagne\\n1200.000 TND\\nConfirm\303\251", nullptr));
        card6->setText(QCoreApplication::translate("DonationWindow", "DON-007   Mohamed Ali\\n10/09/2026 \302\267 Ponctuel\\n5000.000 TND\\n\303\200 v\303\251rifier", nullptr));
        card7->setText(QCoreApplication::translate("DonationWindow", "DON-008   Mohamed A. Ali\\n09/09/2026 \302\267 Ponctuel\\n80.000 TND\\nConfirm\303\251", nullptr));
        detailKicker->setText(QCoreApplication::translate("DonationWindow", "DON S\303\211LECTIONN\303\211", nullptr));
        detailId->setText(QCoreApplication::translate("DonationWindow", "DON-001", nullptr));
        detailStatus->setText(QCoreApplication::translate("DonationWindow", "Confirm\303\251", nullptr));
        recurringBadge->setText(QCoreApplication::translate("DonationWindow", "R\303\251current", nullptr));
        anomalyBadge->setText(QCoreApplication::translate("DonationWindow", "\303\200 v\303\251rifier", nullptr));
        detailLabel->setText(QCoreApplication::translate("DonationWindow", "Donateur", nullptr));
        detailDonor->setText(QCoreApplication::translate("DonationWindow", "Ahmed Ben Ali", nullptr));
        detailLabel2->setText(QCoreApplication::translate("DonationWindow", "Montant", nullptr));
        detailAmount->setText(QCoreApplication::translate("DonationWindow", "150.000 TND", nullptr));
        detailLabel3->setText(QCoreApplication::translate("DonationWindow", "Date", nullptr));
        detailDate->setText(QCoreApplication::translate("DonationWindow", "02/10/2026", nullptr));
        detailLabel4->setText(QCoreApplication::translate("DonationWindow", "Mode", nullptr));
        detailPayment->setText(QCoreApplication::translate("DonationWindow", "Carte bancaire", nullptr));
        detailLabel5->setText(QCoreApplication::translate("DonationWindow", "Type", nullptr));
        detailType->setText(QCoreApplication::translate("DonationWindow", "Ponctuel", nullptr));
        detailLabel6->setText(QCoreApplication::translate("DonationWindow", "Campagne", nullptr));
        detailCampaign->setText(QCoreApplication::translate("DonationWindow", "Soutien aux animaux", nullptr));
        btnRecurring->setText(QCoreApplication::translate("DonationWindow", "Dons r\303\251currents", nullptr));
        btnAnomalies->setText(QCoreApplication::translate("DonationWindow", "Anomalies", nullptr));
        btnConsult->setText(QCoreApplication::translate("DonationWindow", "Consulter", nullptr));
        btnEdit->setText(QCoreApplication::translate("DonationWindow", "Modifier", nullptr));
        btnArchive->setText(QCoreApplication::translate("DonationWindow", "Archiver", nullptr));
        btnDelete->setText(QCoreApplication::translate("DonationWindow", "Supprimer", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DonationWindow: public Ui_DonationWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DONATION_H
