#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include "../auth/accounts.h"

class QLineEdit;
class QPushButton;
class QLabel;
class QCheckBox;
class QRadioButton;
class QStackedWidget;
class BrandPanel;

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(bool demoMode = false, bool useSettings = true, QWidget *parent = nullptr);
    void prepareForLogin();
    void showAfterLogout();
    Accounts::Role authenticatedRole() const { return m_role; }

signals:
    void loginSucceeded(const QString &username);

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void handleLogin();
    void showResetPassword();
    void showLoginForm();
    void sendResetCode();
    void resetPassword();
    void updateResetContactMode();

private:
    void buildUi();
    QWidget *createLoginPage();
    QWidget *createResetPage();
    void applyStyles();
    void showMessage(const QString &text, bool isError);
    void showResetMessage(const QString &text, bool isError);
    void resetRecoveryState();
    void animatePage();
    void setMotionEnabled(bool enabled);

    QStackedWidget *m_pages{nullptr};
    QLineEdit   *m_username{nullptr};
    QLineEdit   *m_password{nullptr};
    QCheckBox   *m_remember{nullptr};
    QPushButton *m_loginBtn{nullptr};
    QLabel      *m_message{nullptr};
    BrandPanel *m_brandPanel{nullptr};
    bool m_motionEnabled{true};
    bool m_firstShow{true};
    bool m_demoMode{false};
    bool m_useSettings{true};
    bool m_loginPending{false};
    Accounts::Role m_role{Accounts::Role::None};

    QRadioButton *m_emailOption{nullptr};
    QRadioButton *m_phoneOption{nullptr};
    QLineEdit    *m_contactInput{nullptr};
    QLineEdit    *m_codeInput{nullptr};
    QLineEdit    *m_newPassword{nullptr};
    QLineEdit    *m_confirmPassword{nullptr};
    QPushButton  *m_sendCodeBtn{nullptr};
    QPushButton  *m_resetBtn{nullptr};
    QLabel       *m_resetMessage{nullptr};
    QString       m_resetCode;
    QString       m_resetDestination;
    QString       m_recoveredAccount;
    QString       m_recoveredPassword;

    bool        m_dragging{false};
    QPoint      m_dragStart;
};

#endif // LOGINWINDOW_H
