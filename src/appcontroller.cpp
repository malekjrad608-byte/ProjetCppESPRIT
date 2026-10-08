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

    // Login screen disabled: open the main application directly.
    if (!m_main) {
        m_main = std::make_unique<MainWindow>(m_demo);
        connect(m_main.get(), &MainWindow::logoutRequested, this, &AppController::logout);
    }

    m_signedIn = true;
    m_main->setSignedInUser({});
    m_main->show();
    m_main->raise();
    m_main->activateWindow();
}

void AppController::openServices(const QString &username)
{
    if (m_signedIn) return;
    if (!m_main) {
        m_main = std::make_unique<MainWindow>(m_demo);
        connect(m_main.get(), &MainWindow::logoutRequested, this, &AppController::logout);
    }
    m_signedIn = true;
    m_main->setSignedInUser(username);
    m_main->navigate(1);
    // Keep a visible window throughout the handoff (quitOnLastWindowClosed).
    if (m_main->isMinimized()) m_main->showNormal();
    else m_main->show();
    m_login->hide();
    m_main->raise();
    m_main->activateWindow();
}

void AppController::logout()
{
    // Login is disabled for this version, so keep the main window open.
    if (!m_main) return;
    m_main->setSignedInUser({});
    m_main->show();
    m_main->raise();
    m_main->activateWindow();
}
