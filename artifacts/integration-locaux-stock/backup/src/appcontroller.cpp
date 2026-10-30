#include "appcontroller.h"
#include "login/loginwindow.h"
#include "mainwindow.h"

AppController::AppController(bool demo, bool useSettings, QObject *parent)
    : QObject(parent), m_demo(demo), m_login(std::make_unique<LoginWindow>(demo, useSettings))
{
    connect(m_login.get(), &LoginWindow::loginSucceeded, this, &AppController::openServices);
}

AppController::~AppController() = default;

LoginWindow *AppController::loginWindow() const { return m_login.get(); }
MainWindow *AppController::mainWindow() const { return m_main.get(); }

void AppController::start()
{
    if (m_signedIn) return;
    m_login->prepareForLogin();
    m_login->show();
}

void AppController::openServices(const QString &username)
{
    if (m_signedIn) return;
    if (!m_main) {
        m_main = std::make_unique<MainWindow>(m_demo);
        connect(m_main.get(), &MainWindow::logoutRequested, this, &AppController::logout);
    }
    m_signedIn = true;
    m_main->setSession(username,m_login->authenticatedRole());
    // Keep a visible window throughout the handoff (quitOnLastWindowClosed).
    if (m_main->isMinimized()) m_main->showNormal();
    else m_main->show();
    m_login->hide();
    m_main->raise();
    m_main->activateWindow();
}

void AppController::logout()
{
    if (!m_signedIn) return;
    m_signedIn = false;
    m_login->showAfterLogout();
    m_main->hide();
    m_main->setSession({},Accounts::Role::None);
    m_login->raise();
    m_login->activateWindow();
}
