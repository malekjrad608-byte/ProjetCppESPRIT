#pragma once
#include <QDialog>
#include <QList>
#include "local.h"

class QLabel;
class QSpinBox;
class QVBoxLayout;

// Liste des alertes automatiques (local sale trop longtemps, local sature, ...)
class AlertsDialog : public QDialog
{
public:
    AlertsDialog(const QList<Local> &locaux, QWidget *parent = nullptr);

    // si l'utilisateur a clique sur "Voir le local" : l'id du local, sinon 0
    int selectedLocalId() const { return m_selected; }

private:
    void rebuild();

    QList<Local> m_locaux;
    QVBoxLayout *m_list;
    QLabel *m_summary;
    QSpinBox *m_threshold;
    int m_selected = 0;
};
