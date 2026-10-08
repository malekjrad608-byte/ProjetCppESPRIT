#ifndef DOSSIERSPAGE_H
#define DOSSIERSPAGE_H

#include <QWidget>
#include <QVector>
#include "medicalrecord.h"

namespace Ui {
class DossiersPage;
}

class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;
class QWidget;
class QFrame;
class QScrollArea;
class QProgressBar;
class QStackedWidget;
class RecordCard;

// Page « Dossiers médicaux » (interface d'origine) intégrée dans ANIWORLD2.
class DossiersPage : public QWidget
{
    Q_OBJECT

public:
    explicit DossiersPage(QWidget *parent = nullptr);
    ~DossiersPage() override;

public slots:
    void onRefreshClicked();
    void onExportCsvClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onAddRecordClicked();
    void onViewRecordClicked();
    void onEditRecordClicked();
    void onDeleteRecordClicked();
    void onToggleVaccinationClicked();
    void onFilterChanged();
    void onResetFiltersClicked();
    void onShowStatisticsClicked();
    void onCardClicked(const QString &recordId);

private:
    Ui::DossiersPage *ui = nullptr;

    // UI construction (layout itself lives in mainwindow.ui)
    void setupUI();
    void loadStylesheet();
    void loadStaticData();

    // Data / view
    void refreshView();
    void rebuildCards();
    void relayoutCards();
    void layoutUi();
    void layoutDetailPanel();
    void updateChips();
    void updateDetailPanel();
    void updateGlobalPanel();
    void applyFiltersAndSort();
    int indexOfRecord(const QString &recordId) const;

    // Actions on a record id
    void viewRecord(const QString &recordId);
    void editRecord(const QString &recordId);
    void deleteRecord(const QString &recordId);

    static QString nextRecordNumber(const QVector<MedicalRecord> &records);

    // Header chips
    QLabel *m_chipTotal = nullptr;
    QLabel *m_chipConsultations = nullptr;
    QLabel *m_chipUpToDate = nullptr;
    QLabel *m_chipToRenew = nullptr;

    // Toolbar
    QLineEdit *m_searchEdit = nullptr;
    QComboBox *m_filterSpecies = nullptr;
    QComboBox *m_filterVaccination = nullptr;
    QComboBox *m_sortCombo = nullptr;

    // Cards grid
    QScrollArea *m_scroll = nullptr;
    QWidget *m_cardsContainer = nullptr;
    QLabel *m_emptyLabel = nullptr;
    QVector<RecordCard *> m_cards;

    // Detail panel
    QLabel *m_detailPlaceholder = nullptr;
    QWidget *m_detailContent = nullptr;
    QLabel *m_detailTitle = nullptr;
    QLabel *m_detailPill = nullptr;
    QLabel *m_detailLine1 = nullptr;
    QLabel *m_detailLine2 = nullptr;
    QLabel *m_detailLine3 = nullptr;
    QPushButton *m_btnView = nullptr;
    QPushButton *m_btnToggle = nullptr;
    QPushButton *m_btnEdit = nullptr;
    QPushButton *m_btnDelete = nullptr;

    // Global panel
    QLabel *m_globalValue = nullptr;
    QProgressBar *m_globalBar = nullptr;
    QLabel *m_globalText = nullptr;

    // Data
    QVector<MedicalRecord> m_records;
    QVector<MedicalRecord> m_filteredRecords;
    QString m_selectedId;
};

#endif // DOSSIERSPAGE_H
