#ifndef IMPORTED_MEDICAL_WIDGET_H
#define IMPORTED_MEDICAL_WIDGET_H

#include <QMainWindow>
#include <QVector>
#include "medicalrecord.h"

namespace Ui {
class MedicalWidget;
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

class MedicalWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit MedicalWidget(bool demo=false,QWidget *parent = nullptr);
    void refresh();
    void addRecord() { onAddRecordClicked(); }
    void focusSearch();
    QPushButton *fileMenuButton() const;
    ~MedicalWidget() override;

signals:

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onNavigationClicked(const QString &name);
    void onAddRecordClicked();
    void onViewRecordClicked();
    void onEditRecordClicked();
    void onDeleteRecordClicked();
    void onToggleVaccinationClicked();
    void onRefreshClicked();
    void onFilterChanged();
    void onResetFiltersClicked();
    void onShowStatisticsClicked();
    void onExportPdfClicked();
    void onCardClicked(const QString &recordId);

private:
    Ui::MedicalWidget *ui = nullptr;
    bool m_demo=false;
    void exportPdf(bool selectedOnly);
    const MedicalRecord *selected() const;
    void saveDialog(const QString &recordId);
    QPushButton *m_pdfButton=nullptr;

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

    // Sidebar
    QVector<QPushButton *> m_navButtons;

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

#endif // IMPORTED_MEDICAL_WIDGET_H
