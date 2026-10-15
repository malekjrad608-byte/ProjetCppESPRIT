/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *sidebar;
    QLabel *logoIcon;
    QLabel *brandName;
    QLabel *brandTag;
    QFrame *sectionCard;
    QLabel *sectionTitle;
    QLabel *sectionSub;
    QLabel *navLabel;
    QPushButton *btnNavDashboard;
    QPushButton *btnNavServices;
    QPushButton *btnNavEmployees;
    QPushButton *btnNavStock;
    QPushButton *btnNavPremises;
    QPushButton *btnNavRecords;
    QLabel *sloganLabel;
    QLabel *sloganSub;
    QWidget *mainContent;
    QWidget *topBar;
    QLabel *crumbGestion;
    QLabel *crumbSep;
    QLabel *crumbCurrent;
    QLabel *demoBadge;
    QPushButton *btnFile;
    QLabel *roleLabel;
    QWidget *pageArea;
    QWidget *recordsPage;
    QLabel *pageTitle;
    QLabel *pageSubtitle;
    QPushButton *btnAdd;
    QLabel *chipTotal;
    QLabel *chipConsultations;
    QLabel *chipUpToDate;
    QLabel *chipToRenew;
    QPushButton *btnStatsTop;
    QLineEdit *searchEdit;
    QComboBox *filterSpecies;
    QComboBox *filterVaccination;
    QComboBox *sortCombo;
    QPushButton *btnReset;
    QScrollArea *cardsScroll;
    QWidget *cardsContainer;
    QLabel *emptyLabel;
    QWidget *detailPanel;
    QFrame *selectedCard;
    QLabel *eyebrow;
    QLabel *placeholderText;
    QWidget *detailContent;
    QLabel *panelTitle;
    QLabel *statusPill;
    QLabel *detailLine1;
    QLabel *detailLine2;
    QLabel *detailLine3;
    QPushButton *btnView;
    QPushButton *btnToggle;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QFrame *globalCard;
    QLabel *panelSubTitle;
    QLabel *bigValue;
    QProgressBar *globalBar;
    QLabel *globalText;
    QPushButton *btnStatsPanel;
    QWidget *pageFooter;
    QLabel *footerLeft;
    QLabel *footerDate;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1400, 900);
        MainWindow->setMinimumSize(QSize(1100, 800));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setGeometry(QRect(0, 0, 1400, 900));
        sidebar = new QWidget(centralwidget);
        sidebar->setObjectName("sidebar");
        sidebar->setGeometry(QRect(0, 0, 290, 900));
        logoIcon = new QLabel(sidebar);
        logoIcon->setObjectName("logoIcon");
        logoIcon->setGeometry(QRect(24, 26, 56, 56));
        logoIcon->setAlignment(Qt::AlignCenter);
        brandName = new QLabel(sidebar);
        brandName->setObjectName("brandName");
        brandName->setGeometry(QRect(94, 26, 180, 30));
        brandTag = new QLabel(sidebar);
        brandTag->setObjectName("brandTag");
        brandTag->setGeometry(QRect(94, 56, 180, 16));
        sectionCard = new QFrame(sidebar);
        sectionCard->setObjectName("sectionCard");
        sectionCard->setGeometry(QRect(18, 106, 254, 70));
        sectionTitle = new QLabel(sectionCard);
        sectionTitle->setObjectName("sectionTitle");
        sectionTitle->setGeometry(QRect(18, 10, 218, 27));
        sectionSub = new QLabel(sectionCard);
        sectionSub->setObjectName("sectionSub");
        sectionSub->setGeometry(QRect(18, 40, 218, 22));
        navLabel = new QLabel(sidebar);
        navLabel->setObjectName("navLabel");
        navLabel->setGeometry(QRect(18, 202, 254, 16));
        btnNavDashboard = new QPushButton(sidebar);
        btnNavDashboard->setObjectName("btnNavDashboard");
        btnNavDashboard->setGeometry(QRect(18, 226, 254, 44));
        btnNavDashboard->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/dashboard.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnNavDashboard->setIcon(icon);
        btnNavDashboard->setIconSize(QSize(20, 20));
        btnNavDashboard->setProperty("active", QVariant(false));
        btnNavServices = new QPushButton(sidebar);
        btnNavServices->setObjectName("btnNavServices");
        btnNavServices->setGeometry(QRect(18, 276, 254, 44));
        btnNavServices->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/services.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnNavServices->setIcon(icon1);
        btnNavServices->setIconSize(QSize(20, 20));
        btnNavServices->setProperty("active", QVariant(false));
        btnNavEmployees = new QPushButton(sidebar);
        btnNavEmployees->setObjectName("btnNavEmployees");
        btnNavEmployees->setGeometry(QRect(18, 326, 254, 44));
        btnNavEmployees->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/employees.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnNavEmployees->setIcon(icon2);
        btnNavEmployees->setIconSize(QSize(20, 20));
        btnNavEmployees->setProperty("active", QVariant(false));
        btnNavStock = new QPushButton(sidebar);
        btnNavStock->setObjectName("btnNavStock");
        btnNavStock->setGeometry(QRect(18, 376, 254, 44));
        btnNavStock->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/stock.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnNavStock->setIcon(icon3);
        btnNavStock->setIconSize(QSize(20, 20));
        btnNavStock->setProperty("active", QVariant(false));
        btnNavPremises = new QPushButton(sidebar);
        btnNavPremises->setObjectName("btnNavPremises");
        btnNavPremises->setGeometry(QRect(18, 426, 254, 44));
        btnNavPremises->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/premises.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnNavPremises->setIcon(icon4);
        btnNavPremises->setIconSize(QSize(20, 20));
        btnNavPremises->setProperty("active", QVariant(false));
        btnNavRecords = new QPushButton(sidebar);
        btnNavRecords->setObjectName("btnNavRecords");
        btnNavRecords->setGeometry(QRect(18, 476, 254, 44));
        btnNavRecords->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/records.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnNavRecords->setIcon(icon5);
        btnNavRecords->setIconSize(QSize(20, 20));
        btnNavRecords->setProperty("active", QVariant(true));
        sloganLabel = new QLabel(sidebar);
        sloganLabel->setObjectName("sloganLabel");
        sloganLabel->setGeometry(QRect(18, 838, 254, 18));
        sloganSub = new QLabel(sidebar);
        sloganSub->setObjectName("sloganSub");
        sloganSub->setGeometry(QRect(18, 860, 254, 18));
        mainContent = new QWidget(centralwidget);
        mainContent->setObjectName("mainContent");
        mainContent->setGeometry(QRect(290, 0, 1110, 900));
        topBar = new QWidget(mainContent);
        topBar->setObjectName("topBar");
        topBar->setGeometry(QRect(0, 0, 1110, 68));
        crumbGestion = new QLabel(topBar);
        crumbGestion->setObjectName("crumbGestion");
        crumbGestion->setGeometry(QRect(34, 22, 70, 24));
        crumbSep = new QLabel(topBar);
        crumbSep->setObjectName("crumbSep");
        crumbSep->setGeometry(QRect(108, 22, 14, 24));
        crumbCurrent = new QLabel(topBar);
        crumbCurrent->setObjectName("crumbCurrent");
        crumbCurrent->setGeometry(QRect(126, 22, 180, 24));
        demoBadge = new QLabel(topBar);
        demoBadge->setObjectName("demoBadge");
        demoBadge->setGeometry(QRect(556, 10, 210, 48));
        demoBadge->setAlignment(Qt::AlignCenter);
        btnFile = new QPushButton(topBar);
        btnFile->setObjectName("btnFile");
        btnFile->setGeometry(QRect(780, 11, 120, 45));
        btnFile->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        roleLabel = new QLabel(topBar);
        roleLabel->setObjectName("roleLabel");
        roleLabel->setGeometry(QRect(914, 22, 170, 24));
        pageArea = new QWidget(mainContent);
        pageArea->setObjectName("pageArea");
        pageArea->setGeometry(QRect(34, 96, 1042, 752));
        recordsPage = new QWidget(pageArea);
        recordsPage->setObjectName("recordsPage");
        recordsPage->setGeometry(QRect(0, 0, 656, 752));
        pageTitle = new QLabel(recordsPage);
        pageTitle->setObjectName("pageTitle");
        pageTitle->setGeometry(QRect(0, 0, 436, 44));
        pageSubtitle = new QLabel(recordsPage);
        pageSubtitle->setObjectName("pageSubtitle");
        pageSubtitle->setGeometry(QRect(0, 50, 656, 20));
        btnAdd = new QPushButton(recordsPage);
        btnAdd->setObjectName("btnAdd");
        btnAdd->setGeometry(QRect(446, 0, 210, 49));
        btnAdd->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnAdd->setProperty("big", QVariant(true));
        chipTotal = new QLabel(recordsPage);
        chipTotal->setObjectName("chipTotal");
        chipTotal->setGeometry(QRect(0, 90, 110, 39));
        chipTotal->setAlignment(Qt::AlignCenter);
        chipConsultations = new QLabel(recordsPage);
        chipConsultations->setObjectName("chipConsultations");
        chipConsultations->setGeometry(QRect(120, 90, 130, 39));
        chipConsultations->setAlignment(Qt::AlignCenter);
        chipUpToDate = new QLabel(recordsPage);
        chipUpToDate->setObjectName("chipUpToDate");
        chipUpToDate->setGeometry(QRect(260, 90, 90, 39));
        chipUpToDate->setAlignment(Qt::AlignCenter);
        chipToRenew = new QLabel(recordsPage);
        chipToRenew->setObjectName("chipToRenew");
        chipToRenew->setGeometry(QRect(360, 90, 145, 39));
        chipToRenew->setAlignment(Qt::AlignCenter);
        btnStatsTop = new QPushButton(recordsPage);
        btnStatsTop->setObjectName("btnStatsTop");
        btnStatsTop->setGeometry(QRect(526, 54, 130, 30));
        btnStatsTop->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        searchEdit = new QLineEdit(recordsPage);
        searchEdit->setObjectName("searchEdit");
        searchEdit->setGeometry(QRect(0, 147, 514, 45));
        searchEdit->setClearButtonEnabled(true);
        filterSpecies = new QComboBox(recordsPage);
        filterSpecies->addItem(QString());
        filterSpecies->addItem(QString());
        filterSpecies->addItem(QString());
        filterSpecies->addItem(QString());
        filterSpecies->addItem(QString());
        filterSpecies->setObjectName("filterSpecies");
        filterSpecies->setGeometry(QRect(0, 202, 160, 42));
        filterSpecies->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        filterVaccination = new QComboBox(recordsPage);
        filterVaccination->addItem(QString());
        filterVaccination->addItem(QString());
        filterVaccination->addItem(QString());
        filterVaccination->setObjectName("filterVaccination");
        filterVaccination->setGeometry(QRect(170, 202, 155, 42));
        filterVaccination->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        sortCombo = new QComboBox(recordsPage);
        sortCombo->addItem(QString());
        sortCombo->addItem(QString());
        sortCombo->addItem(QString());
        sortCombo->addItem(QString());
        sortCombo->setObjectName("sortCombo");
        sortCombo->setGeometry(QRect(335, 202, 155, 42));
        sortCombo->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnReset = new QPushButton(recordsPage);
        btnReset->setObjectName("btnReset");
        btnReset->setGeometry(QRect(500, 202, 156, 42));
        btnReset->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        cardsScroll = new QScrollArea(recordsPage);
        cardsScroll->setObjectName("cardsScroll");
        cardsScroll->setGeometry(QRect(0, 263, 656, 489));
        cardsScroll->setFrameShape(QFrame::NoFrame);
        cardsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        cardsScroll->setWidgetResizable(false);
        cardsContainer = new QWidget();
        cardsContainer->setObjectName("cardsContainer");
        cardsContainer->setGeometry(QRect(0, 0, 646, 489));
        emptyLabel = new QLabel(cardsContainer);
        emptyLabel->setObjectName("emptyLabel");
        emptyLabel->setGeometry(QRect(0, 0, 646, 160));
        emptyLabel->setAlignment(Qt::AlignCenter);
        cardsScroll->setWidget(cardsContainer);
        detailPanel = new QWidget(pageArea);
        detailPanel->setObjectName("detailPanel");
        detailPanel->setGeometry(QRect(682, 0, 360, 658));
        selectedCard = new QFrame(detailPanel);
        selectedCard->setObjectName("selectedCard");
        selectedCard->setGeometry(QRect(0, 0, 360, 434));
        eyebrow = new QLabel(selectedCard);
        eyebrow->setObjectName("eyebrow");
        eyebrow->setGeometry(QRect(22, 20, 316, 16));
        placeholderText = new QLabel(selectedCard);
        placeholderText->setObjectName("placeholderText");
        placeholderText->setGeometry(QRect(22, 44, 316, 60));
        placeholderText->setWordWrap(true);
        placeholderText->setVisible(false);
        detailContent = new QWidget(selectedCard);
        detailContent->setObjectName("detailContent");
        detailContent->setGeometry(QRect(22, 44, 316, 390));
        panelTitle = new QLabel(detailContent);
        panelTitle->setObjectName("panelTitle");
        panelTitle->setGeometry(QRect(0, 0, 316, 36));
        statusPill = new QLabel(detailContent);
        statusPill->setObjectName("statusPill");
        statusPill->setGeometry(QRect(0, 44, 120, 28));
        detailLine1 = new QLabel(detailContent);
        detailLine1->setObjectName("detailLine1");
        detailLine1->setGeometry(QRect(0, 78, 316, 20));
        detailLine1->setWordWrap(true);
        detailLine2 = new QLabel(detailContent);
        detailLine2->setObjectName("detailLine2");
        detailLine2->setGeometry(QRect(0, 102, 316, 36));
        detailLine2->setWordWrap(true);
        detailLine3 = new QLabel(detailContent);
        detailLine3->setObjectName("detailLine3");
        detailLine3->setGeometry(QRect(0, 142, 316, 56));
        detailLine3->setWordWrap(true);
        btnView = new QPushButton(detailContent);
        btnView->setObjectName("btnView");
        btnView->setGeometry(QRect(0, 208, 316, 36));
        btnView->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnToggle = new QPushButton(detailContent);
        btnToggle->setObjectName("btnToggle");
        btnToggle->setGeometry(QRect(0, 253, 153, 36));
        btnToggle->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnEdit = new QPushButton(detailContent);
        btnEdit->setObjectName("btnEdit");
        btnEdit->setGeometry(QRect(162, 253, 154, 36));
        btnEdit->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnDelete = new QPushButton(detailContent);
        btnDelete->setObjectName("btnDelete");
        btnDelete->setGeometry(QRect(0, 298, 316, 36));
        btnDelete->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        globalCard = new QFrame(detailPanel);
        globalCard->setObjectName("globalCard");
        globalCard->setGeometry(QRect(0, 488, 360, 215));
        panelSubTitle = new QLabel(globalCard);
        panelSubTitle->setObjectName("panelSubTitle");
        panelSubTitle->setGeometry(QRect(22, 20, 316, 26));
        bigValue = new QLabel(globalCard);
        bigValue->setObjectName("bigValue");
        bigValue->setGeometry(QRect(22, 54, 316, 38));
        globalBar = new QProgressBar(globalCard);
        globalBar->setObjectName("globalBar");
        globalBar->setGeometry(QRect(22, 100, 316, 10));
        globalBar->setMinimum(0);
        globalBar->setMaximum(100);
        globalBar->setValue(0);
        globalBar->setTextVisible(false);
        globalText = new QLabel(globalCard);
        globalText->setObjectName("globalText");
        globalText->setGeometry(QRect(22, 118, 316, 20));
        btnStatsPanel = new QPushButton(globalCard);
        btnStatsPanel->setObjectName("btnStatsPanel");
        btnStatsPanel->setGeometry(QRect(22, 148, 316, 45));
        btnStatsPanel->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        pageFooter = new QWidget(mainContent);
        pageFooter->setObjectName("pageFooter");
        pageFooter->setGeometry(QRect(0, 862, 1110, 38));
        footerLeft = new QLabel(pageFooter);
        footerLeft->setObjectName("footerLeft");
        footerLeft->setGeometry(QRect(34, 8, 300, 16));
        footerDate = new QLabel(pageFooter);
        footerDate->setObjectName("footerDate");
        footerDate->setGeometry(QRect(916, 8, 160, 16));
        footerDate->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "ANIWORLD \342\200\224 Gestion du centre", nullptr));
        brandName->setText(QCoreApplication::translate("MainWindow", "ANIWORLD", nullptr));
        brandTag->setText(QCoreApplication::translate("MainWindow", "THE OFFICIAL APPLICATION", nullptr));
        sectionTitle->setText(QCoreApplication::translate("MainWindow", "Centre de soins", nullptr));
        sectionSub->setText(QCoreApplication::translate("MainWindow", "Espace de gestion", nullptr));
        navLabel->setText(QCoreApplication::translate("MainWindow", "NAVIGATION", nullptr));
        btnNavDashboard->setText(QCoreApplication::translate("MainWindow", "Tableau de bord", nullptr));
        btnNavServices->setText(QCoreApplication::translate("MainWindow", "Services", nullptr));
        btnNavEmployees->setText(QCoreApplication::translate("MainWindow", "Employ\303\251s", nullptr));
        btnNavStock->setText(QCoreApplication::translate("MainWindow", "Stock", nullptr));
        btnNavPremises->setText(QCoreApplication::translate("MainWindow", "Locaux", nullptr));
        btnNavRecords->setText(QCoreApplication::translate("MainWindow", "Dossiers m\303\251dicaux", nullptr));
        sloganLabel->setText(QCoreApplication::translate("MainWindow", "Une attention \303\240 chaque patte.", nullptr));
        sloganSub->setText(QCoreApplication::translate("MainWindow", "ANIWORLD \302\267 Centre de soins", nullptr));
        crumbGestion->setText(QCoreApplication::translate("MainWindow", "Gestion", nullptr));
        crumbSep->setText(QCoreApplication::translate("MainWindow", "/", nullptr));
        crumbCurrent->setText(QCoreApplication::translate("MainWindow", "Dossiers m\303\251dicaux", nullptr));
        demoBadge->setText(QCoreApplication::translate("MainWindow", "Donn\303\251es de d\303\251monstration", nullptr));
        btnFile->setText(QCoreApplication::translate("MainWindow", "Fichier  \342\226\276", nullptr));
        roleLabel->setText(QCoreApplication::translate("MainWindow", "Responsable du centre", nullptr));
        pageTitle->setText(QCoreApplication::translate("MainWindow", "Dossiers m\303\251dicaux", nullptr));
        pageSubtitle->setText(QCoreApplication::translate("MainWindow", "Suivi de sant\303\251, consultations et vaccinations des animaux", nullptr));
        btnAdd->setText(QCoreApplication::translate("MainWindow", "+ Ajouter un dossier", nullptr));
        chipTotal->setText(QString());
        chipTotal->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "ok", nullptr)));
        chipConsultations->setText(QString());
        chipConsultations->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "info", nullptr)));
        chipUpToDate->setText(QString());
        chipUpToDate->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "ok", nullptr)));
        chipToRenew->setText(QString());
        chipToRenew->setProperty("tone", QVariant(QCoreApplication::translate("MainWindow", "warn", nullptr)));
        btnStatsTop->setText(QCoreApplication::translate("MainWindow", "Statistiques", nullptr));
        searchEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Rechercher un dossier\342\200\246", nullptr));
        filterSpecies->setItemText(0, QCoreApplication::translate("MainWindow", "Toutes esp\303\250ces", nullptr));
        filterSpecies->setItemText(1, QCoreApplication::translate("MainWindow", "Chien", nullptr));
        filterSpecies->setItemText(2, QCoreApplication::translate("MainWindow", "Chat", nullptr));
        filterSpecies->setItemText(3, QCoreApplication::translate("MainWindow", "Lapin", nullptr));
        filterSpecies->setItemText(4, QCoreApplication::translate("MainWindow", "Oiseau", nullptr));

        filterVaccination->setItemText(0, QCoreApplication::translate("MainWindow", "Tous statuts", nullptr));
        filterVaccination->setItemText(1, QCoreApplication::translate("MainWindow", "\303\200 jour", nullptr));
        filterVaccination->setItemText(2, QCoreApplication::translate("MainWindow", "\303\200 renouveler", nullptr));

        sortCombo->setItemText(0, QCoreApplication::translate("MainWindow", "Tri par d\303\251faut", nullptr));
        sortCombo->setItemText(1, QCoreApplication::translate("MainWindow", "Nom (A\342\200\223Z)", nullptr));
        sortCombo->setItemText(2, QCoreApplication::translate("MainWindow", "Date r\303\251cente", nullptr));
        sortCombo->setItemText(3, QCoreApplication::translate("MainWindow", "Propri\303\251taire", nullptr));

        btnReset->setText(QCoreApplication::translate("MainWindow", "R\303\251initialiser", nullptr));
        emptyLabel->setText(QCoreApplication::translate("MainWindow", "Aucun dossier m\303\251dical ne correspond \303\240 votre recherche.", nullptr));
        eyebrow->setText(QCoreApplication::translate("MainWindow", "DOSSIER S\303\211LECTIONN\303\211", nullptr));
        placeholderText->setText(QCoreApplication::translate("MainWindow", "S\303\251lectionnez un dossier dans la liste pour afficher ses informations et actions.", nullptr));
        panelTitle->setText(QString());
        statusPill->setText(QString());
        detailLine1->setText(QString());
        detailLine2->setText(QString());
        detailLine3->setText(QString());
        btnView->setText(QCoreApplication::translate("MainWindow", "Consulter le dossier", nullptr));
        btnToggle->setText(QCoreApplication::translate("MainWindow", "Vaccin \303\240 renouveler", nullptr));
        btnEdit->setText(QCoreApplication::translate("MainWindow", "Modifier", nullptr));
        btnDelete->setText(QCoreApplication::translate("MainWindow", "Supprimer le dossier", nullptr));
        panelSubTitle->setText(QCoreApplication::translate("MainWindow", "Suivi vaccinal global", nullptr));
        bigValue->setText(QCoreApplication::translate("MainWindow", "0 %", nullptr));
        globalText->setText(QString());
        btnStatsPanel->setText(QCoreApplication::translate("MainWindow", "Voir les statistiques", nullptr));
        footerLeft->setText(QCoreApplication::translate("MainWindow", "ANIWORLD \302\267 Gestion du centre", nullptr));
        footerDate->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
