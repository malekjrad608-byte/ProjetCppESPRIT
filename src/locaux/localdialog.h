#pragma once
#include <QDialog>
#include "local.h"

class LocalRepository;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;

// Formulaire "Ajouter un local" / "Modifier le local" avec controle de saisie
class LocalDialog : public QDialog
{
public:
    // existing == nullptr  -> ajout ; sinon modification de ce local
    LocalDialog(const Local *existing, LocalRepository *repo, QWidget *parent = nullptr);
    Local result() const { return m_result; }

protected:
    void accept() override;

private:
    LocalRepository *m_repo;
    Local m_base;
    Local m_result;
    bool m_editing;

    QLineEdit *m_name;
    QComboBox *m_type;
    QComboBox *m_service;
    QComboBox *m_floor;
    QComboBox *m_state=nullptr;
    QSpinBox *m_capacity;
    QSpinBox *m_occupancy = nullptr;
    QLineEdit *m_specialty;
    QCheckBox *m_oos;
    QLabel *m_nameErr;
    QLabel *m_oosErr;
};
