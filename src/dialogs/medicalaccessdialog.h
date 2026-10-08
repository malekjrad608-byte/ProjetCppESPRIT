#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QShowEvent>

class MedicalAccessDialog : public QDialog {
    Q_OBJECT
public:
    explicit MedicalAccessDialog(QWidget *parent = nullptr);
    QString getUserId() const;
signals:
    void accessGranted(const QString &userId);
private slots:
    void validateAccess();
protected:
    void showEvent(QShowEvent *event) override;
private:
    void setupUi();
    void applyStyles();
    QLineEdit *m_userIdInput;
    QLabel *m_message;
};
