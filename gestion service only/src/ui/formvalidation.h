#pragma once
#include "fieldvalidator.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>

namespace FormValidation {
using Field = ValidatedField;

inline Field *bind(QWidget *input, Field::Validator rule, bool required = true) {
    auto *field = new Field(input, std::move(rule), required);
    if (auto *combo = qobject_cast<QComboBox *>(input)) {
        field->watch(combo, &QComboBox::currentIndexChanged);
        if (combo->isEditable()) field->watch(combo, &QComboBox::editTextChanged);
        QObject::connect(combo, &QComboBox::activated, field, [field] {
            if (field->state() == Field::Neutral) field->onInput();
        });
    } else if (auto *spin = qobject_cast<QAbstractSpinBox *>(input)) {
        field->watch(spin->findChild<QLineEdit *>(), &QLineEdit::textChanged);
        field->watch(spin, &QAbstractSpinBox::editingFinished);
        if (auto *integer = qobject_cast<QSpinBox *>(spin))
            field->watch(integer, &QSpinBox::valueChanged);
        else if (auto *decimal = qobject_cast<QDoubleSpinBox *>(spin))
            field->watch(decimal, &QDoubleSpinBox::valueChanged);
        else if (auto *date = qobject_cast<QDateTimeEdit *>(spin))
            field->watch(date, &QDateTimeEdit::dateTimeChanged);
    } else if (auto *line = qobject_cast<QLineEdit *>(input)) {
        field->watch(line, &QLineEdit::textChanged);
    } else if (auto *notes = qobject_cast<QTextEdit *>(input)) {
        field->watch(notes, &QTextEdit::textChanged);
    } else if (auto *button = qobject_cast<QAbstractButton *>(input)) {
        field->watch(button, &QAbstractButton::toggled);
    }
    return field;
}

inline Field *text(QLineEdit *input, int limit, bool required,
                   std::function<QString(const QString &)> problem = {}) {
    return bind(input, [=] {
        const auto value = input->text().trimmed();
        if (value.isEmpty()) return required ? Field::invalid("Ce champ est obligatoire.") : Field::neutral();
        if (value.size() > limit) return Field::invalid(QString("Maximum %1 caractères.").arg(limit));
        const auto message = problem ? problem(value) : QString();
        return message.isEmpty() ? Field::valid() : Field::invalid(message);
    }, required);
}

inline Field *choice(QComboBox *combo, bool requireId = false) {
    return bind(combo, [=] {
        const bool valid = requireId ? combo->currentData().toInt() > 0 : !combo->currentText().trimmed().isEmpty();
        return valid ? Field::valid() : Field::invalid("Choisissez une valeur.");
    });
}

inline Field *spin(QAbstractSpinBox *input, QString message) {
    return bind(input, [=] { return input->hasAcceptableInput() ? Field::valid() : Field::invalid(message); });
}

inline bool validate(QWidget *form) {
    QWidget *first = nullptr;
    for (auto *field : form->findChildren<Field *>()) {
        if (field->isEnabled() && !field->validateNow() && !first) first = field->field();
    }
    if (first) first->setFocus();
    return !first;
}
}
