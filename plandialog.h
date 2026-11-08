#pragma once
#include <QDialog>

class ClinicMap;
class FlatMap;
class QStackedWidget;
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
    void setView(int index);         // 0 = vue 2.5D, 1 = vue plan

    LocalRepository *m_repo;
    ClinicMap *m_map;                // vue 2.5D
    FlatMap *m_flat;                 // vue plan (a plat)
    QStackedWidget *m_views;
    QPushButton *m_btn3d;
    QPushButton *m_btnFlat;
    QPushButton *m_rotate;
    QPushButton *m_explode;
    QLabel *m_sub;
    QLabel *m_info;
    QPushButton *m_goto;
    int m_selected = 0;
};
