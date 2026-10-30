#pragma once
#include <QDialog>

class ClinicMap;
class LocalRepository;
class QLabel;
class QPushButton;

// Grand plan de la clinique. Fenetre NON modale : on peut ajouter / supprimer
// un local dans la page et voir le plan se modifier en direct.
class PlanDialog : public QDialog
{
    Q_OBJECT
public:
    PlanDialog(LocalRepository *repo, QWidget *parent = nullptr);
    void setSelected(int id);

signals:
    void selectRequested(int id);

private:
    void updateInfo();

    LocalRepository *m_repo;
    ClinicMap *m_map;
    QLabel *m_info;
    QPushButton *m_goto;
    int m_selected = 0;
};
