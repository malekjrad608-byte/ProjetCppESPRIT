#ifndef GROOMINGCUT_H
#define GROOMINGCUT_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QPixmap>
#include <QSize>

// Catalogue des coupes / soins de toilettage : nom, photo, déroulement.
// Photos : déposer un fichier  coupes/<id>.jpg  (ou .png) à côté de l'exécutable
// (ou dans le dossier du projet) ; sans photo, une image de remplacement est dessinée.
struct GroomingCut
{
    QString id;                 // nom de fichier de la photo (sans extension)
    QString nom;
    QString description;
    QString duree;
    QString difficulte;
    QString rendu;              // à quoi ressemble l'animal une fois la coupe faite
    QStringList especes;        // vide = toutes
    QStringList races;          // vide = toutes les races des espèces ci-dessus
    QStringList deconseille;    // races pour lesquelles la coupe est déconseillée
    QStringList materiel;
    QStringList etapes;
    QStringList precautions;
};

class GroomingCatalog
{
public:
    static QList<GroomingCut> tous();
    static QList<GroomingCut> pour(const QString &espece, const QString &race);
    static bool deconseillee(const GroomingCut &c, const QString &race);
    // Photo de la coupe adaptée à l'animal. Ordre de recherche (le premier fichier trouvé gagne) :
    //   coupes/<id>_<espece>_<race>.jpg  ->  coupes/<id>_<espece>.jpg  ->  coupes/<id>.jpg
    // La photo générique <id>.jpg n'est utilisée que si elle montre la bonne espèce ;
    // sinon on affiche la photo représentative de l'espèce (animal_<espece>.jpg).
    static QPixmap image(const GroomingCut &c, const QSize &size,
                         const QString &espece = QString(), const QString &race = QString());
    // false = pas de photo propre à cette espèce/race pour cette coupe (le nom du fichier à ajouter est dans 'manquant')
    static bool photoSpecifique(const GroomingCut &c, const QString &espece, const QString &race, QString *manquant = nullptr);
    static QString slug(const QString &text);      // "Bélier (Lop)" -> "belier_lop"
};

#endif // GROOMINGCUT_H
