#pragma once
#include <QDialog>
#include "local.h"

class LocalRepository;
class LocalPreview;
class ToggleSwitch;
class QButtonGroup;
class QComboBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;

// Formulaire "Ajouter un local" / "Modifier le local"
//  - controle de saisie en direct (nom libre ou deja pris, compteur de caracteres)
//  - type en boutons avec icones, etage en selecteur segmente, capacite avec - / +
//  - suggestions de specialite selon le type, interrupteur "Hors service"
//  - apercu en direct a droite (carte + petite boite 2.5D + liste des controles)
class LocalDialog : public QDialog
{
public:
    // existing == nullptr  -> ajout ; sinon modification de ce local
    LocalDialog(const Local *existing, LocalRepository *repo, QWidget *parent = nullptr);
    Local result() const { return m_result; }

protected:
    void accept() override;

private:
    Local buildLocal() const;      // local tel qu'il serait enregistre avec les valeurs du formulaire
    QString nameProblem() const;   // "" si le nom est correct
    void setCapacity(int v);
    void refreshSuggestions();
    void refresh();                // controles + apercu

    LocalRepository *m_repo;
    Local m_base;
    Local m_result;
    bool m_editing;
    bool m_triedSave = false;

    QLineEdit *m_name;
    QLabel *m_nameCount;
    QLabel *m_nameMsg;
    QButtonGroup *m_types;
    QButtonGroup *m_floors;
    QComboBox *m_service;
    int m_capacity = 1;
    QLabel *m_capValue;
    QPushButton *m_capMinus;
    QPushButton *m_capPlus;
    QLabel *m_capMsg;
    QLineEdit *m_specialty;
    QHBoxLayout *m_suggestRow;
    ToggleSwitch *m_oos;
    QLabel *m_oosMsg;
    LocalPreview *m_preview;
    QPushButton *m_save;
};
