#ifndef MEDICALWINDOW_H
#define MEDICALWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QStringList>
#include "medicalrecord.h"

namespace Ui {
class MedicalWindow;
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
class SmsService;
class MedicalRecordDialog;
class QTimer;

class MedicalWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MedicalWindow(QWidget *parent = nullptr);
    ~MedicalWindow() override;

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
    void onExportCsvClicked();
    void onExportPdfClicked();       // fiche PDF de l'animal sélectionné
    void onRdvClicked();             // fenêtre des rendez-vous
    void checkAppointments();
    void onWatchClicked();           // veille sanitaire        // SMS automatiques (rappel la veille / rendez-vous manqué)
    void onCardClicked(const QString &recordId);
    void onRemindersClicked();     // DM-I1 : rappels vaccinaux
    void onLogoutClicked();        // bouton « Se déconnecter » de la barre du haut

private:
    Ui::MedicalWindow *ui = nullptr;

    // UI construction (layout itself lives in mainwindow.ui)
    void setupUI();
    void loadStylesheet();
    void loadStaticData();
    void syncVaccinationStatus();   // statut À jour / À renouveler d'après les rappels
    void showStartupReminders();
    void updateReminderButton();
    void updateRdvButton(int pending);
    void updateWatchButton();
    void checkHealthWatch(bool popup);   // alertes automatiques pour le médecin
    void exportAnimalPdf(const QString &idAnimal);
    void applyRdv(const MedicalRecordDialog &dialog, const MedicalRecord &record);
    void applyVaccin(const MedicalRecordDialog &dialog, const MedicalRecord &record);
    void actualiser();   // Refresh : relance Mtmp.afficher() (comme l'atelier)

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

    // Barre du haut et en-tête de page
    QPushButton *m_btnLogout = nullptr;
    QLabel *m_titleIcon = nullptr;

    // Panneau « Dossier sélectionné » : lignes libellé / valeur
    QVector<QLabel *> m_detailKeys;
    QVector<QLabel *> m_detailVals;
    QStringList m_detailValues;
    QFrame *m_detailSep = nullptr;

    // Boutons Veille et Rappels
    QPushButton *m_btnReminders = nullptr;
    QPushButton *m_btnWatch = nullptr;
    int m_rdvPending = 0;            // SMS de rendez-vous en attente (compté dans le badge Rappels)
    QTimer *m_rdvTimer = nullptr;
    SmsService *m_sms = nullptr;

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
    bool m_db = false;   // true si les données viennent de la base

    // Attribut de la classe MedicalRecord (comme "Etudiant Etmp;" dans l'atelier)
    // pour pouvoir faire appel aux méthodes afficher() et supprimer()
    MedicalRecord Mtmp;
};

#endif // MEDICALWINDOW_H
