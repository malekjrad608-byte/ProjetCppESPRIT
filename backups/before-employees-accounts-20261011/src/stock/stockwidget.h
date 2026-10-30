#ifndef STOCKWIDGET_H
#define STOCKWIDGET_H

#include <QWidget>
#include <QList>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QTableView;
class QLineEdit;
class QComboBox;
class QFrame;
class QStandardItemModel;
class StockFilterModel;

class StockWidget : public QWidget {
    Q_OBJECT

public:
    explicit StockWidget(QWidget *parent = nullptr);
    ~StockWidget() = default;
    void refresh();
    void focusSearch();
    bool exportInventoryPdf(const QString &path, QString *error=nullptr) const;

public slots:
    void openMovementDialog();
protected:
    void paintEvent(QPaintEvent *event) override;
private slots:
    void prepareFefoCheckout();
    void verifyReorder();
    void exportInventory();

private:
    void setupUi();
    void setupKpiCards(QHBoxLayout *layout);
    void setupActionPanels(QHBoxLayout *layout);
    void showMovementDialog(int row, bool checkout);
    void updateResultCount();

    QTableView *stockTable;
    QStandardItemModel *tableModel;
    StockFilterModel *filterModel;
    QLineEdit *searchLineEdit;
    QComboBox *categoryFilter;
    QList<QLabel *> kpiValues;
    QLabel *fefoText;
    QLabel *reorderText;
    QLabel *resultCount;
    QLabel *notice;
    QPushButton *fefoButton;
    QPushButton *reorderButton;
    int fefoRow = -1;
    int reorderRow = -1;
};

#endif // STOCKWIDGET_H
