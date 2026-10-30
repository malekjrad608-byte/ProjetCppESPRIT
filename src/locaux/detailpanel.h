#pragma once
#include <QFrame>
#include "local.h"

class QLabel;
class QPushButton;

// Panneau de droite : details du local selectionne + boutons d'action
class DetailPanel : public QFrame
{
    Q_OBJECT
public:
    explicit DetailPanel(QWidget *parent = nullptr);
    // nullptr = aucune selection ; alerts = textes courts des alertes de ce local
    void setLocal(const Local *l, const QStringList &alerts = QStringList());

signals:
    void assignRequested();
    void freeRequested();
    void editRequested();
    void cleanRequested();
    void deleteRequested();

private:
    QLabel *m_name;
    QLabel *m_badge;
    QLabel *m_hint;
    QLabel *m_alerts;
    QWidget *m_info;
    QWidget *m_buttons;
    QLabel *m_values[7];
    QPushButton *m_btnAssign;
    QPushButton *m_btnFree;
    QPushButton *m_btnEdit;
    QPushButton *m_btnClean;
    QPushButton *m_btnDelete;
};
