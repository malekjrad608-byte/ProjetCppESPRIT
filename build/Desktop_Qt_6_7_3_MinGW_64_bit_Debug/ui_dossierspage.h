/********************************************************************************
** Form generated from reading UI file 'dossierspage.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DOSSIERSPAGE_H
#define UI_DOSSIERSPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DossiersPage
{
public:
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

    void setupUi(QWidget *DossiersPage)
    {
        if (DossiersPage->objectName().isEmpty())
            DossiersPage->setObjectName("DossiersPage");
        DossiersPage->resize(1100, 800);
        pageArea = new QWidget(DossiersPage);
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
        chipTotal->setGeometry(QRect(0, 90, 130, 39));
        chipTotal->setAlignment(Qt::AlignCenter);
        chipConsultations = new QLabel(recordsPage);
        chipConsultations->setObjectName("chipConsultations");
        chipConsultations->setGeometry(QRect(140, 90, 160, 39));
        chipConsultations->setAlignment(Qt::AlignCenter);
        chipUpToDate = new QLabel(recordsPage);
        chipUpToDate->setObjectName("chipUpToDate");
        chipUpToDate->setGeometry(QRect(310, 90, 110, 39));
        chipUpToDate->setAlignment(Qt::AlignCenter);
        chipToRenew = new QLabel(recordsPage);
        chipToRenew->setObjectName("chipToRenew");
        chipToRenew->setGeometry(QRect(430, 90, 150, 39));
        chipToRenew->setAlignment(Qt::AlignCenter);
        btnStatsTop = new QPushButton(recordsPage);
        btnStatsTop->setObjectName("btnStatsTop");
        btnStatsTop->setGeometry(QRect(526, 90, 130, 45));
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
        filterSpecies->setGeometry(QRect(0, 202, 175, 45));
        filterSpecies->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        filterVaccination = new QComboBox(recordsPage);
        filterVaccination->addItem(QString());
        filterVaccination->addItem(QString());
        filterVaccination->addItem(QString());
        filterVaccination->setObjectName("filterVaccination");
        filterVaccination->setGeometry(QRect(187, 202, 160, 45));
        filterVaccination->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        sortCombo = new QComboBox(recordsPage);
        sortCombo->addItem(QString());
        sortCombo->addItem(QString());
        sortCombo->addItem(QString());
        sortCombo->addItem(QString());
        sortCombo->setObjectName("sortCombo");
        sortCombo->setGeometry(QRect(359, 202, 170, 45));
        sortCombo->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        btnReset = new QPushButton(recordsPage);
        btnReset->setObjectName("btnReset");
        btnReset->setGeometry(QRect(541, 207, 110, 34));
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

        retranslateUi(DossiersPage);

        QMetaObject::connectSlotsByName(DossiersPage);
    } // setupUi

    void retranslateUi(QWidget *DossiersPage)
    {
        pageTitle->setText(QCoreApplication::translate("DossiersPage", "Dossiers m\303\251dicaux", nullptr));
        pageSubtitle->setText(QCoreApplication::translate("DossiersPage", "Suivi de sant\303\251, consultations et vaccinations des animaux", nullptr));
        btnAdd->setText(QCoreApplication::translate("DossiersPage", "+ Ajouter un dossier", nullptr));
        chipTotal->setText(QString());
        chipTotal->setProperty("tone", QVariant(QCoreApplication::translate("DossiersPage", "ok", nullptr)));
        chipConsultations->setText(QString());
        chipConsultations->setProperty("tone", QVariant(QCoreApplication::translate("DossiersPage", "info", nullptr)));
        chipUpToDate->setText(QString());
        chipUpToDate->setProperty("tone", QVariant(QCoreApplication::translate("DossiersPage", "ok", nullptr)));
        chipToRenew->setText(QString());
        chipToRenew->setProperty("tone", QVariant(QCoreApplication::translate("DossiersPage", "warn", nullptr)));
        btnStatsTop->setText(QCoreApplication::translate("DossiersPage", "Statistiques", nullptr));
        searchEdit->setPlaceholderText(QCoreApplication::translate("DossiersPage", "Rechercher un dossier\342\200\246", nullptr));
        filterSpecies->setItemText(0, QCoreApplication::translate("DossiersPage", "Toutes esp\303\250ces", nullptr));
        filterSpecies->setItemText(1, QCoreApplication::translate("DossiersPage", "Chien", nullptr));
        filterSpecies->setItemText(2, QCoreApplication::translate("DossiersPage", "Chat", nullptr));
        filterSpecies->setItemText(3, QCoreApplication::translate("DossiersPage", "Lapin", nullptr));
        filterSpecies->setItemText(4, QCoreApplication::translate("DossiersPage", "Oiseau", nullptr));

        filterVaccination->setItemText(0, QCoreApplication::translate("DossiersPage", "Tous statuts", nullptr));
        filterVaccination->setItemText(1, QCoreApplication::translate("DossiersPage", "\303\200 jour", nullptr));
        filterVaccination->setItemText(2, QCoreApplication::translate("DossiersPage", "\303\200 renouveler", nullptr));

        sortCombo->setItemText(0, QCoreApplication::translate("DossiersPage", "Tri par d\303\251faut", nullptr));
        sortCombo->setItemText(1, QCoreApplication::translate("DossiersPage", "Nom (A\342\200\223Z)", nullptr));
        sortCombo->setItemText(2, QCoreApplication::translate("DossiersPage", "Date r\303\251cente", nullptr));
        sortCombo->setItemText(3, QCoreApplication::translate("DossiersPage", "Propri\303\251taire", nullptr));

        btnReset->setText(QCoreApplication::translate("DossiersPage", "R\303\251initialiser", nullptr));
        emptyLabel->setText(QCoreApplication::translate("DossiersPage", "Aucun dossier m\303\251dical ne correspond \303\240 votre recherche.", nullptr));
        eyebrow->setText(QCoreApplication::translate("DossiersPage", "DOSSIER S\303\211LECTIONN\303\211", nullptr));
        placeholderText->setText(QCoreApplication::translate("DossiersPage", "S\303\251lectionnez un dossier dans la liste pour afficher ses informations et actions.", nullptr));
        panelTitle->setText(QString());
        statusPill->setText(QString());
        detailLine1->setText(QString());
        detailLine2->setText(QString());
        detailLine3->setText(QString());
        btnView->setText(QCoreApplication::translate("DossiersPage", "Consulter le dossier", nullptr));
        btnToggle->setText(QCoreApplication::translate("DossiersPage", "Vaccin \303\240 renouveler", nullptr));
        btnEdit->setText(QCoreApplication::translate("DossiersPage", "Modifier", nullptr));
        btnDelete->setText(QCoreApplication::translate("DossiersPage", "Supprimer le dossier", nullptr));
        panelSubTitle->setText(QCoreApplication::translate("DossiersPage", "Suivi vaccinal global", nullptr));
        bigValue->setText(QCoreApplication::translate("DossiersPage", "0 %", nullptr));
        globalText->setText(QString());
        btnStatsPanel->setText(QCoreApplication::translate("DossiersPage", "Voir les statistiques", nullptr));
        (void)DossiersPage;
    } // retranslateUi

};

namespace Ui {
    class DossiersPage: public Ui_DossiersPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DOSSIERSPAGE_H
