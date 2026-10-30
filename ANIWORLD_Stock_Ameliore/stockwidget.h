#ifndef STOCKWIDGET_H
#define STOCKWIDGET_H

#include <QDate>
#include <QString>
#include <QVector>
#include <QWidget>

class QHBoxLayout;
class QLabel;
class QPushButton;
class QTableView;
class QLineEdit;
class QComboBox;
class QStandardItemModel;

struct StockRecord {
    int id = 0; // Ajout de l'ID Oracle
    QString article;
    QString category;
    QString lot;
    int quantity = 0;
    QString unit;
    int threshold = 1;
    QDate expiry;
};

class StockWidget : public QWidget {
    Q_OBJECT

public:
    explicit StockWidget(QWidget *parent = nullptr);
    ~StockWidget() override = default;

signals:
    void logoutRequested();

private slots:
    void openMovementDialog();
    void editSelectedItem();
    void deleteSelectedItem();
    void showStatistics();
    void manageExpiryItems();
    void openPurchasesModule();
    void openAlertsModule();
    void openReplenishmentModule();

private:
    void setupUi();
    void loadDataFromDatabase(); // Déclaration de la fonction de chargement Oracle
    void refreshTable();
    void updateFooter();
    void updateAlertNavigation();
    QString statusFor(const StockRecord &item) const;
    int countBelowThreshold() const;
    int countExpiryAlerts() const;
    QVector<int> selectedSourceRows() const;

    QWidget *contentWidget = nullptr;
    QTableView *stockTable = nullptr;
    QStandardItemModel *tableModel = nullptr;
    QLineEdit *searchLineEdit = nullptr;
    QComboBox *categoryFilter = nullptr;
    QComboBox *sortFilter = nullptr;
    QComboBox *stateFilter = nullptr;
    QLabel *footerLabel = nullptr;
    QPushButton *alertsNavButton = nullptr;
    QVector<StockRecord> items;
};

#endif // STOCKWIDGET_H