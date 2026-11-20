#ifndef GROOMINGDIALOG_H
#define GROOMINGDIALOG_H

#include <QDialog>
#include <QList>
#include "groomingcut.h"

class QComboBox;
class QListWidget;
class QLabel;
class QTextBrowser;
class QPushButton;

// Choix d'une coupe pour un animal : liste filtrée par espèce / race, photo du modèle,
// photo de l'animal à côté pour comparer, déroulement étape par étape.
class GroomingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GroomingDialog(QWidget *parent = nullptr);
    void setAnimal(const QString &nom, const QString &espece, const QString &race);

private slots:
    void onSpeciesChanged();
    void refreshList();
    void onCutSelected();
    void onChoosePhoto();

private:
    void showCut(const GroomingCut &c);
    void showSpeciesPhoto();

    QLabel *m_animalLabel;
    QComboBox *m_species;
    QComboBox *m_breed;
    QListWidget *m_list;
    QLabel *m_modelPhoto;
    QLabel *m_photoHint;          // indique quand la photo propre à l'espèce / race manque
    QLabel *m_animalPhoto;
    QLabel *m_title;
    QLabel *m_info;
    QLabel *m_warning;
    QTextBrowser *m_details;
    QPushButton *m_btnPhoto;

    QList<GroomingCut> m_cuts;     // coupes affichées dans la liste (même ordre)
    QString m_animalName;
    bool m_userPhotoLoaded = false;
};

#endif // GROOMINGDIALOG_H
