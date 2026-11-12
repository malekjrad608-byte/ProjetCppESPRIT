#ifndef FIELDVALIDATOR_H
#define FIELDVALIDATOR_H

#include <QWidget>
#include <QColor>
#include <functional>

class QLabel;
class QVariantAnimation;
class QPropertyAnimation;
class QGraphicsOpacityEffect;

// Enveloppe un champ de saisie (QLineEdit, QComboBox, QDateEdit, QSpinBox, QTextEdit...) et ajoute :
//   - une validation en temps réel (indépendante pour chaque champ),
//   - un cadre qui passe en douceur au vert (valide) ou au rouge (invalide),
//   - une icône animée  ✓ / ✕  à droite du champ,
//   - un message explicatif sous le champ quand la saisie est invalide.
// Le champ reste NEUTRE tant que l'utilisateur n'y a rien saisi (champ obligatoire vide) ou s'il est vide et facultatif.
// Le champ d'origine n'est pas modifié : le reste du code continue de l'utiliser tel quel.
class ValidatedField : public QWidget
{
    Q_OBJECT
public:
    enum State { Neutral, Valid, Invalid };
    struct Result {
        State state = Neutral;
        QString message;            // affiché sous le champ si Invalid
    };
    using Validator = std::function<Result()>;

    static Result neutral()                     { return {Neutral, QString()}; }
    static Result valid()                       { return {Valid, QString()}; }
    static Result invalid(const QString &msg)   { return {Invalid, msg}; }

    ValidatedField(QWidget *field, Validator validator, bool required, QWidget *parent = nullptr);

    QWidget *field() const { return m_field; }
    bool isRequired() const { return m_required; }
    State state() const { return m_state; }

    // Connexion pratique : toute modification du champ déclenche la validation.
    template <typename Obj, typename Sig>
    void watch(Obj *obj, Sig signal) { connect(obj, signal, this, [this]() { onInput(); }); }

    // Contrôle avant enregistrement : le champ est considéré comme "touché" (une erreur s'affiche s'il est invalide ou vide
    // alors qu'il est obligatoire). Retourne false si la valeur empêche l'enregistrement.
    bool validateNow();

    void resetTouched();                 // retour à l'état neutre (ex. race après changement d'espèce)
    void revalidate(bool animate = true);

public slots:
    void onInput();                      // l'utilisateur (ou le programme) a modifié la valeur

private:
    void applyState(State s, const QString &message, bool animate);
    void animateBorder(const QColor &to);
    void applyBorder(const QColor &c);
    void showMessage(const QString &text, bool animate);
    QString typeSelector() const;

    QWidget *m_field;
    Validator m_validator;
    bool m_required;
    bool m_touched = false;
    State m_state = Neutral;
    QString m_message;

    QLabel *m_icon;
    QLabel *m_msg;
    QGraphicsOpacityEffect *m_iconFade;
    QGraphicsOpacityEffect *m_msgFade;
    QVariantAnimation *m_colorAnim;
    QPropertyAnimation *m_iconAnim;
    QPropertyAnimation *m_msgHeightAnim;
    QPropertyAnimation *m_msgFadeAnim;
    QColor m_currentBorder;
    bool m_hasOwnBorder = false;
};

#endif // FIELDVALIDATOR_H
