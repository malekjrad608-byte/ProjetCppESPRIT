#include "donationwindow.h"
#include "loginwindow.h"

#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("ANIWORLD - Gestion des Dons"));
    app.setOrganizationName(QStringLiteral("ANIWORLD"));

    LoginWindow login;
    DonationWindow window;

    QObject::connect(&login, &LoginWindow::loginSuccessful, [&]() {
        // Keep the PETSS login visible until its success message is shown,
        // then switch smoothly to the ANIWORLD donation dashboard.
        login.hide();

        auto *opacity = new QGraphicsOpacityEffect(&window);
        opacity->setOpacity(0.0);
        window.setGraphicsEffect(opacity);
        window.show();

        auto *fade = new QPropertyAnimation(opacity, "opacity", &window);
        fade->setDuration(500);
        fade->setStartValue(0.0);
        fade->setEndValue(1.0);
        fade->setEasingCurve(QEasingCurve::OutCubic);
        QObject::connect(fade, &QPropertyAnimation::finished, fade, &QObject::deleteLater);
        fade->start();
    });

    login.show();
    return app.exec();
}
