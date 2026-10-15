#pragma once
#include <QHash>
#include <QList>
#include <QPointer>
#include <QSet>
#include <QStringList>
#include <QWidget>
#include <functional>
#include "local.h"
#include "plandialog.h"

class CardGrid;
class ClinicMap;
class DetailPanel;
class LocalRepository;
class QComboBox;
class QLabel;
class QLayout;
class QLineEdit;
class QScrollArea;
class QProgressBar;
class QPushButton;
class QStackedWidget;
class QTableWidget;
class QTimer;
class Toast;

// Page "Gestion des locaux" (a integrer dans le QStackedWidget de l'application)
class PageLocaux : public QWidget
{
    Q_OBJECT
public:
    explicit PageLocaux(LocalRepository *repo, QWidget *parent = nullptr);

private:
    // construction
    QLayout *buildHeader();
    QLayout *buildTools();
    QLayout *buildBody();
    QPushButton *makeButton(const QString &text, const QString &icon, const char *role);
    QPushButton *makeChip(const char *tone);

    // donnees affichees
    QList<Local> filteredSorted() const;
    void refresh(bool playIntro);
    void updateTable(const QList<Local> &list);
    void updateChips();
    void updateQueryHint();
    void updateGlobal();
    void setView(int index);          // 0 = cartes, 1 = liste

    // filtres
    void resetFilters();
    void toggleChip(int kw);          // pastille = raccourci de recherche

    // selection
    void onCardClicked(int id);
    void selectRoom(int id);          // selectionne meme si un filtre le cache
    void showDetail();

    // CRUD + actions metier
    void addLocal();
    void editSelected();
    void deleteSelected();
    void modifySelected(const std::function<bool(Local &)> &fn);

    // fonctionnalites avancees
    void openPlan();
    void openAlerts();
    void openSmartAssign();
    void exportPdf();

    // alertes automatiques
    void computeAlerts(bool notify);
    void applyAlerts();

    LocalRepository *m_repo;
    int m_selectedId = 0;

    // en-tete
    QPushButton *m_chipAvail;
    QPushButton *m_chipOcc;
    QPushButton *m_chipOos;
    QPushButton *m_chipDirty;
    QPushButton *m_btnAlerts;
    QPushButton *m_btnSmart;
    QPushButton *m_btnAdd;
    QPushButton *m_btnPdf;
    QPushButton *m_btnMap;
    QPushButton *m_btnStats;

    // filtres
    QLineEdit *m_search;
    QComboBox *m_sortCombo;
    QLabel *m_count;
    QLabel *m_queryHint;
    QScrollArea *m_cardScroll;
    QPushButton *m_btnCards;
    QPushButton *m_btnList;

    // corps
    QStackedWidget *m_views;
    CardGrid *m_grid;
    QTableWidget *m_table;
    QLabel *m_empty;
    DetailPanel *m_detail;
    ClinicMap *m_miniMap;
    QLabel *m_pct;
    QProgressBar *m_bar;
    QLabel *m_globalText;
    Toast *m_toast;
    QPointer<PlanDialog> m_plan;

    // tableau
    bool m_updatingTable = false;
    int m_tableSortCol = -1;
    Qt::SortOrder m_tableSortOrder = Qt::AscendingOrder;

    // alertes
    QTimer *m_alertTimer;
    QHash<int, int> m_alertLevel;
    QHash<int, QStringList> m_alertTexts;
    QSet<QString> m_knownAlertKeys;
    bool m_alertsReady = false;
};
