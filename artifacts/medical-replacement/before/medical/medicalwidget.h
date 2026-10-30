#pragma once
#include <QWidget>
#include "medicalrecord.h"
#include <QVector>
class QLabel;
class QLineEdit;
class QComboBox;
class QScrollArea;
class QPushButton;
class RecordCard;
class MedicalWidget : public QWidget {
    Q_OBJECT
public:
    explicit MedicalWidget(bool demo=false,QWidget *parent=nullptr);
    void refresh();
    void addRecord();
    void focusSearch();
protected:
    bool eventFilter(QObject *watched,QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    void filter();
    void arrangeCards();
    void select(const QString &id);
    void updateDetails();
    void editRecord(bool create);
    void viewRecord();
    void removeRecord();
    void toggleVaccination();
    void statistics();
    void exportPdf(bool selectedOnly);
    const MedicalRecord *selected() const;
    bool m_demo;
    QVector<MedicalRecord> m_records,m_filtered;
    QVector<RecordCard *> m_cards;
    QString m_selectedId;
    QLineEdit *m_search;
    QComboBox *m_species,*m_vaccination,*m_sort;
    QLabel *m_total,*m_upToDate,*m_renew,*m_counter,*m_error,*m_empty,*m_placeholder,*m_title,*m_details,*m_badge;
    QScrollArea *m_scroll;
    QWidget *m_cardSurface,*m_detailContent;
    QPushButton *m_toggle,*m_export;
};
