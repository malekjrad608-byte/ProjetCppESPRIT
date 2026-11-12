#pragma once
#include <QMainWindow>

class LocalRepository;
class PawBurst;

// Fenetre de test : sidebar + header + page Locaux.
// Pour l'integration, seule PageLocaux est a recuperer.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    QWidget *buildSidebar();
    QWidget *buildHeader();
    QWidget *buildFooter();

    LocalRepository *m_repo;
    PawBurst *m_paws = nullptr;
};
