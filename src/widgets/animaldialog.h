#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QLabel>
#include <QShowEvent>
#include "../models/animal.h"

class AnimalDialog : public QDialog {
    Q_OBJECT
public:
    explicit AnimalDialog(QWidget *parent = nullptr);
    AnimalDialog(const Animal &animal, bool readOnly, QWidget *parent = nullptr);
    Animal getAnimal() const;
protected:
    void showEvent(QShowEvent *event) override;
private:
    void setup(const Animal &animal, bool editing, bool readOnly);
    void validateAndAccept();
    Animal m_animal;
    QLineEdit *m_name;
    QLineEdit *m_species;
    QLineEdit *m_breed;
    QComboBox *m_gender;
    QDateEdit *m_birthDate;
    QLineEdit *m_color;
    QLineEdit *m_ownerId;
    QLineEdit *m_ownerPhone;
    QLineEdit *m_ownerEmail;
    QTextEdit *m_notes;
    QLabel *m_error;
};
