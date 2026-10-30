#pragma once

#include <QObject>
#include <memory>

class LoginWindow;
class MainWindow;

// Owns the two independent top-level windows; login styling stays local.
class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(bool demo = false, bool useSettings = true, QObject *parent = nullptr);
    ~AppController() override;
    void start();
    LoginWindow *loginWindow() const;
    MainWindow *mainWindow() const;
    bool isSignedIn() const { return m_signedIn; }

private:
    void openServices(const QString &username);
    void logout();

    bool m_demo;
    bool m_signedIn = false;
    std::unique_ptr<LoginWindow> m_login;
    std::unique_ptr<MainWindow> m_main;
};
