#ifndef DONATIONWINDOW_H
#define DONATIONWINDOW_H

#include <QDate>
#include <QList>
#include <QMainWindow>
#include <QString>

class QPushButton;

QT_BEGIN_NAMESPACE
namespace Ui { class DonationWindow; }
QT_END_NAMESPACE

struct DonationRecord
{
    QString id;
    QString donor;
    QDate date;
    double amount = 0.0;
    QString paymentMode;
    QString type;
    QString status;
    QString campaign;
    bool recurring = false;
    QDate nextDate;
    QString note;
    bool anomaly = false;
    QString anomalyReason;
};

class DonationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DonationWindow(QWidget *parent = nullptr);
    ~DonationWindow() override;

private slots:
    void openNewDonation();
    void showStatistics();
    void exportCsv();
    void exportPdf();
    void showRecurring();
    void showAnomalies();
    void showSelectedDonation();
    void editSelectedDonation();
    void archiveSelectedDonation();
    void deleteSelectedDonation();
    void refreshList();
    void resetFilters();

private:
    void seedDemoData();
    void updateStats();
    void recomputeAnomalies();
    void updateSelection(int index);
    void updateDetailPanel(const DonationRecord &record);
    void applyCardStyle(int index, bool selected);
    void showInfo(const QString &title, const QString &message);
    bool confirmAction(const QString &title, const QString &message);
    QString money(double amount) const;
    QString dateText(const QDate &date) const;
    int selectedIndex() const;
    QList<int> filteredIndexes() const;
    bool recordMatchesFilters(const DonationRecord &record) const;
    void ensureCardCount(int count);
    void configureCard(QPushButton *card);

    Ui::DonationWindow *ui;
    QList<DonationRecord> m_records;
    QList<QPushButton *> m_cards;
    int m_selectedIndex = -1;
};

#endif
