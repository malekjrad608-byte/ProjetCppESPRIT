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

#include "stockrecord.h"

class StockWidget : public QWidget {
    Q_OBJECT

public:
    explicit StockWidget(QWidget *parent = nullptr);
    ~StockWidget() override = default;

    void refresh();
    void focusSearch();
    bool exportInventoryPdf(const QString &path,QString *error=nullptr) const;
protected:
    void paintEvent(QPaintEvent *) override;

public slots:
    void openMovementDialog();
private slots:
    void exportInventory();
    void editSelectedItem();
    void deleteSelectedItem();
    void showStatistics();
    void manageExpiryItems();

private:
    void setupUi();
    void loadDemoData();
    bool loadOracle();
    bool persistStock();
    qint64 m_revision=0;
    bool m_storageReady=true;
    QVector<StockRecord> persistedItems;
    void refreshTable();
    void updateFooter();
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
    QVector<StockRecord> items;
};

#endif // STOCKWIDGET_H
