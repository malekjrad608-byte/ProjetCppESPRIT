#ifndef STOCKWIDGET_H
#define STOCKWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QTableView;
class QLineEdit;
class QComboBox;
class QFrame;
class QStandardItemModel;

class StockWidget : public QWidget {
    Q_OBJECT

public:
    explicit StockWidget(QWidget *parent = nullptr);
    ~StockWidget() = default;

private slots:
    void openMovementDialog();
    void prepareFefoCheckout();
    void verifyReorder();

private:
    void setupUi();
    void setupKpiCards(QHBoxLayout *layout);
    void setupActionPanels(QHBoxLayout *layout);

    QTableView *stockTable;
    QStandardItemModel *tableModel; // L-modèle salla7neh houni
    QLineEdit *searchLineEdit;
    QComboBox *categoryFilter;
};

#endif // STOCKWIDGET_H