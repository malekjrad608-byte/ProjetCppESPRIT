#pragma once
#include <QDialog>

class LocalRepository;
class QCheckBox;
class QComboBox;
class QVBoxLayout;
class QVariantAnimation;

// Affectation intelligente : on choisit l'espece et le besoin de l'animal,
// ANIWORLD classe les locaux et propose le meilleur (avec les raisons).
class SmartAssignDialog : public QDialog
{
public:
    explicit SmartAssignDialog(LocalRepository *repo, QWidget *parent = nullptr);

    int assignedId() const { return m_assignedId; }
    QString assignedName() const { return m_assignedName; }

private:
    void rebuild();

    LocalRepository *m_repo;
    QComboBox *m_species;
    QComboBox *m_need;
    QCheckBox *m_urgent;
    QVBoxLayout *m_results;
    QVariantAnimation *m_anim = nullptr;
    int m_assignedId = 0;
    QString m_assignedName;
};
