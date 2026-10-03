#include <QApplication>
#include <QFile>
#include <QStyleFactory>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include "mainwindow.h"
#include "loginwindow.h"

// ==================== Style QSS ====================
void chargerStyle()
{
    QFile file(":/style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
        file.close();
        qDebug() << "✅ Style QSS chargé";
    } else {
        qDebug() << "⚠️ Impossible de charger style.qss";
    }
}

// ==================== Base de données ====================
bool initialiserBaseDeDonnees()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("aniworld.db");

    if (!db.open()) {
        qDebug() << "❌ Erreur BD:" << db.lastError().text();
        return false;
    }

    QSqlQuery q;

    q.exec("CREATE TABLE IF NOT EXISTS employes ("
           "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
           "  nom TEXT NOT NULL,"
           "  prenom TEXT NOT NULL,"
           "  courriel TEXT,"
           "  telephone TEXT,"
           "  poste TEXT,"
           "  date_embauche TEXT,"
           "  disponible INTEGER DEFAULT 1,"
           "  charge_hebdo REAL DEFAULT 0,"
           "  dispo_hebdo REAL DEFAULT 35,"
           "  competences TEXT)");

    q.exec("SELECT COUNT(*) FROM employes");
    if (q.next() && q.value(0).toInt() == 0) {
        q.exec("INSERT INTO employes (nom, prenom, courriel, telephone, poste, "
               "date_embauche, disponible, charge_hebdo, dispo_hebdo, competences) VALUES "
               "('Ben Ali', 'Sami', 'sami@aniworld.tn', '+216 20 111 111', "
               "'Vétérinaire', '2020-01-15', 1, 30, 35, 'Soins, Chirurgie, Diagnostic'),"
               "('Trabelsi', 'Amine', 'amine@aniworld.tn', '+216 20 222 222', "
               "'Assistant(e) vétérinaire', '2021-03-10', 1, 28, 35, 'Soins, Vaccination'),"
               "('Mansouri', 'Youssef', 'youssef@aniworld.tn', '+216 20 333 333', "
               "'Auxiliaire vétérinaire', '2022-06-01', 0, 20, 35, 'Toilettage, Soins'),"
               "('Karoui', 'Lina', 'lina@aniworld.tn', '+216 20 444 444', "
               "'Secrétaire / Réceptionniste', '2019-09-20', 1, 25, 35, 'Accueil, Réservation'),"
               "('Gharbi', 'Sarra', 'sarra@aniworld.tn', '+216 20 555 555', "
               "'Agent d''entretien', '2023-02-05', 1, 22, 35, 'Nettoyage'),"
               "('Jelassi', 'Nour', 'nour@aniworld.tn', '+216 20 666 666', "
               "'Aide aux soins animaliers', '2023-07-12', 1, 24, 35, 'Soins, Alimentation')");
        qDebug() << "✅ 6 employés ajoutés";
    }

    qDebug() << "✅ BD prête";
    return true;
}

// ==================== Main ====================
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ✅ Style Fusion — bech el CSS mte3 ComboBox yekhdem
    QApplication::setStyle("Fusion");

    chargerStyle();

    LoginWindow loginWindow;
    MainWindow *mainWindow = nullptr;

    QObject::connect(&loginWindow, &LoginWindow::loginSuccessful,
                     &a, [&]() {
        if (!initialiserBaseDeDonnees()) {
            QMessageBox::critical(nullptr, "Erreur",
                                  "Impossible d'initialiser la base de données.");
            loginWindow.close();
            return;
        }

        if (!mainWindow)
            mainWindow = new MainWindow;

        loginWindow.hide();
        mainWindow->show();
        mainWindow->raise();
        mainWindow->activateWindow();
    });

    loginWindow.show();
    const int result = a.exec();
    delete mainWindow;
    return result;
}