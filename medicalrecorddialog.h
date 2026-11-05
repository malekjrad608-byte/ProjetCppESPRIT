#ifndef MEDICALRECORDDIALOG_H
#define MEDICALRECORDDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QMap>
#include <QVector>
#include "medicalrecord.h"
#include "appointment.h"
#include "lookupdata.h"

class QLabel;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QDateEdit;
class QTextEdit;
class QPushButton;
class QStandardItemModel;
class QDateTimeEdit;
class QCheckBox;
class QVBoxLayout;
class QTimer;
class ValidatedField;

// Liste déroulante à cases à cocher (sélection multiple) : utilisée pour les traitements.
class CheckableComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CheckableComboBox(QWidget *parent = nullptr);
    void clearItems();
    void addCheckItem(const QString &label, const QString &value, bool checked = false);
    QStringList checkedValues() const;
    void setCheckedValues(const QStringList &values);   // ajoute les valeurs inconnues

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void updateText();
    QStandardItemModel *m_model;
};

class MedicalRecordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MedicalRecordDialog(QWidget *parent = nullptr);
    // Dossiers existants : sert à retrouver un animal par sa référence,
    // à proposer les diagnostics / traitements déjà utilisés et à numéroter la référence.
    void setExistingRecords(const QVector<MedicalRecord> &records);
    void setMedicalRecord(const MedicalRecord& record);
    MedicalRecord getMedicalRecord() const;

    // Prochain rendez-vous (optionnel) + téléphone du propriétaire pour le SMS de rappel
    void setRendezVous(const RendezVous &rdv);
    bool rdvPlanifie() const;
    QDateTime rdvDate() const;
    QString telephone() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onSpeciesChanged();
    void onAnimalRefEntered();
    void refreshSuggestions();      // diagnostic intelligent
    void onApplySuggestions();

private:
    void setupUI();
    bool validateForm();
    void fillTreatments();
    void selectComboText(QComboBox *combo, const QString &text);
    QString nextAnimalRef() const;

    // Animal info widgets
    QLineEdit *m_editAnimalRef;
    QLineEdit *m_editAnimalName;
    QComboBox *m_comboSpecies;
    QComboBox *m_comboBreed;
    QSpinBox *m_spinAge;
    QComboBox *m_comboSex;
    QComboBox *m_comboOwner;

    // Medical info widgets
    QComboBox *m_comboEmployee;       // affiche le nom, enregistre l'ID employé
    QDateEdit *m_dateConsultation;
    QTextEdit *m_editReason;
    QComboBox *m_comboDiagnosis;      // éditable, propose les diagnostics déjà saisis
    CheckableComboBox *m_comboTreatment;

    // Rendez-vous
    QCheckBox *m_chkRdv = nullptr;
    QDateTimeEdit *m_dateRdv = nullptr;
    QLineEdit *m_editPhone = nullptr;

    // Health widgets
    QTextEdit *m_editVaccinations;
    QTextEdit *m_editAllergies;
    QDoubleSpinBox *m_spinWeight;
    QTextEdit *m_editObservations;
    QComboBox *m_comboVaccinationStatus;

    // Diagnostic intelligent : état de l'animal + traitements suggérés
    QLabel *m_stateLabel = nullptr;
    QWidget *m_suggestBox = nullptr;
    QVBoxLayout *m_suggestLayout = nullptr;
    QPushButton *m_btnApplySuggest = nullptr;
    QTimer *m_suggestTimer = nullptr;
    QVector<QCheckBox *> m_suggestChecks;

    // Validation dynamique : un ValidatedField par champ (indépendants les uns des autres)
    QVector<ValidatedField *> m_validated;
    ValidatedField *m_vfRace = nullptr;
    QLineEdit *m_editRecordId = nullptr;          // ID dossier (lecture seule, généré par l'application)

    QPushButton *m_btnSave;
    QPushButton *m_btnCancel;

    // Données
    QString m_recordId;                          // vide = nouveau dossier (généré par la fenêtre principale)
    bool m_editing = false;
    QMap<QString, QStringList> m_races;
    QMap<QString, MedicalRecord> m_animalsByRef; // dernier dossier connu de chaque animal
    QVector<MedicalRecord> m_existing;
    QList<LookupItem> m_stock;
};

#endif // MEDICALRECORDDIALOG_H
