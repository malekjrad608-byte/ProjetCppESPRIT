#include "groomingcut.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QLinearGradient>
#include <QFont>
#include <utility>

QList<GroomingCut> GroomingCatalog::tous()
{
    static const QList<GroomingCut> catalogue = {
        { "teddy_bear", "Teddy bear",
          "Poil taillé court et égal (2 à 3 cm) sur tout le corps, tête arrondie et visage soigné : look « peluche ».",
          "1 h 30 – 2 h", "Moyenne",
          "Corps uniforme et doux, tête ronde et fournie, oreilles et museau arrondis aux ciseaux.",
          { "Chien" }, { "Caniche", "Croisé", "Autre" }, {},
          { "Tondeuse avec peigne 6–9 mm", "Ciseaux droits et courbes", "Peigne métallique", "Brosse carde" },
          { "Brosser et démêler complètement, puis bain et séchage.",
            "Tondre le corps dans le sens du poil avec le peigne choisi.",
            "Égaliser aux ciseaux : pattes en « colonnes », dos et flancs.",
            "Arrondir la tête : couper le dessus du crâne, les oreilles et le museau en rond.",
            "Dégager les yeux et nettoyer le tour des yeux et des babines.",
            "Contrôle final : symétrie, aucune zone irritée." },
          { "Ne jamais tondre un poil emmêlé : démêler avant.", "Vérifier l'absence de peau irritée ou de plaie." } },

        { "caniche_moderne", "Caniche moderne",
          "Tête ronde en « pompon », corps court, pattes en manchettes et queue en pompon.",
          "2 h – 2 h 30", "Difficile",
          "Silhouette élégante du caniche : tête bouffante, corps ras, pattes et queue plus fournies.",
          { "Chien" }, { "Caniche" }, {},
          { "Tondeuse + lames 10 et 4F", "Ciseaux droits, courbes, boules", "Peigne, brosse carde" },
          { "Brossage complet, bain, séchage au séchoir en brossant.",
            "Tonte du museau, des pieds et de la base de la queue (lame fine).",
            "Tonte du corps au peigne choisi.",
            "Mise en forme aux ciseaux de la tête (pompon), des manchettes et du pompon de queue.",
            "Finitions et contrôle de symétrie." },
          { "Lame fine : risque de brûlure de la peau si elle chauffe, vérifier souvent.", "Toilettage réservé à un professionnel formé." } },

        { "coupe_ete", "Coupe d'été (courte)",
          "Poil raccourci uniformément à 1–2 cm pour supporter la chaleur, sans raser à blanc.",
          "1 h – 1 h 30", "Facile",
          "Animal plus léger, poil court régulier, jamais rasé à blanc.",
          { "Chien" }, {}, { "Husky", "Berger Allemand", "Golden Retriever", "Labrador" },
          { "Tondeuse avec peigne 9–12 mm", "Brosse, peigne" },
          { "Brossage et bain, séchage complet.",
            "Tonte dans le sens du poil avec un peigne long (jamais à blanc).",
            "Ciseaux pour les pattes, le tour des oreilles et la queue.",
            "Contrôle : peau non irritée, longueur régulière." },
          { "DÉCONSEILLÉE aux chiens à double poil (Husky, Berger, Golden, Labrador) : le sous-poil protège de la chaleur et de la brûlure du soleil. Préférer le déshabillage.",
            "Prévoir de la crème solaire pour peau rasée courte." } },

        { "deshabillage", "Déshabillage du sous-poil",
          "Retrait du sous-poil mort par brossage et bain, sans couper le poil : idéal pour les chiens à double poil.",
          "1 h – 1 h 30", "Facile",
          "Poil allégé et aéré, volume réduit, sans coupe visible.",
          { "Chien" }, { "Husky", "Berger Allemand", "Golden Retriever", "Labrador", "Beagle" }, {},
          { "Cardeuse / peigne à sous-poil", "Shampoing adapté", "Séchoir à haute pression" },
          { "Brossage préalable pour enlever les nœuds.",
            "Bain avec shampoing + démêlant, laisser poser.",
            "Séchage à haute pression pour décoller le sous-poil mort.",
            "Passage du peigne à sous-poil par petites sections, dans le sens du poil.",
            "Finition : brossage doux, contrôle de la peau." },
          { "Ne jamais tirer fort sur le peigne : risque d'irriter la peau.", "À répéter 2 fois par an environ (mues)." } },

        { "franges_golden", "Nettoyage des franges",
          "Coupe naturelle : on garde le poil mais on nettoie aux ciseaux les franges des pattes, du ventre et de la queue.",
          "45 min – 1 h", "Facile",
          "Silhouette naturelle et propre, franges nettes sans coupe courte.",
          { "Chien" }, { "Golden Retriever", "Labrador", "Berger Allemand" }, {},
          { "Ciseaux courbes et à effiler", "Peigne, brosse" },
          { "Brosser tout le corps.",
            "Égaliser les franges des pattes avant et arrière.",
            "Raccourcir les poils entre les coussinets.",
            "Nettoyer le tour de la queue et le ventre.",
            "Contrôle final." },
          { "Ne pas couper le poil du dos : il protège de la chaleur." } },

        { "hygienique", "Coupe hygiénique",
          "Coupe localisée du poil autour des yeux, des oreilles, du ventre, des coussinets et de la zone anale.",
          "20 – 30 min", "Facile",
          "Aucun changement de silhouette ; zones sensibles propres et dégagées.",
          { "Chien", "Chat", "Lapin" }, {}, {},
          { "Petite tondeuse silencieuse", "Ciseaux à bouts ronds" },
          { "Installer l'animal calmement, le tenir fermement mais sans stress.",
            "Dégager le tour des yeux aux ciseaux à bouts ronds.",
            "Raccourcir le poil sous le ventre et autour de la zone anale.",
            "Couper le poil entre les coussinets.",
            "Récompense et contrôle." },
          { "Ciseaux à bouts ronds uniquement.", "Animal agité ou agressif : voir le vétérinaire." } },

        { "lion", "Coupe lion",
          "Corps tondu court, crinière, bout de queue et bas des pattes gardés : coupe estivale pour chats à poil long.",
          "1 h – 1 h 30", "Difficile",
          "Allure de petit lion : crinière fournie, corps ras, pompon au bout de la queue.",
          { "Chat" }, { "Persan", "Maine Coon", "Angora", "Européen", "Croisé" }, { "Sphynx" },
          { "Tondeuse silencieuse", "Peigne, ciseaux" },
          { "Démêler complètement, retirer les nœuds avant toute tonte.",
            "Tondre le corps dans le sens du poil, de la nuque vers la queue.",
            "Laisser la crinière autour de la tête et du cou.",
            "Laisser un pompon au bout de la queue et des « bottines » aux pattes.",
            "Contrôle de la peau et récompense." },
          { "Souvent faite sous contrôle vétérinaire (stress, sédation éventuelle).", "Ne jamais tondre un chat qui se débat : risque de blessure." } },

        { "demelage", "Démêlage et dénouage",
          "Retrait des nœuds et du feutrage, avec brossage régulier : indispensable pour les chats et chiens à poil long.",
          "30 min – 1 h", "Moyenne",
          "Poil lisse et brillant, sans nœuds ni feutre.",
          { "Chat", "Chien", "Lapin" }, { "Persan", "Maine Coon", "Angora", "Caniche", "Croisé", "Autre" }, {},
          { "Peigne à dents larges puis fines", "Démêlant en spray", "Coupe-nœuds" },
          { "Vaporiser un démêlant sur le nœud.",
            "Dénouer du bout du poil vers la racine, mèche par mèche.",
            "Nœud serré : le couper en lamelles avec un coupe-nœuds, jamais aux ciseaux contre la peau.",
            "Brosser tout le corps.",
            "Prévoir un brossage tous les 1 à 2 jours." },
          { "Les nœuds contre la peau peuvent cacher une plaie ou une irritation.", "Tirer sur le feutre fait mal : y aller doucement." } },

        { "tonte_angora", "Tonte du lapin angora",
          "Tonte ou brossage profond du poil du lapin angora, plusieurs fois par an pour éviter les boules de poils.",
          "45 min – 1 h", "Moyenne",
          "Poil court et régulier, lapin plus léger.",
          { "Lapin" }, { "Angora" }, {},
          { "Ciseaux à bouts ronds ou tondeuse douce", "Brosse douce" },
          { "Brosser pour retirer le poil mort.",
            "Poser le lapin sur une surface antidérapante, le maintenir sans le retourner sur le dos.",
            "Couper par petites mèches, loin de la peau.",
            "Contrôler la peau (fine, facile à couper).",
            "Récompense." },
          { "Ne jamais retourner un lapin sur le dos (immobilité tonique = stress).", "Peau très fine : ciseaux à bouts ronds obligatoires." } },

        { "griffes", "Taille des griffes",
          "Raccourcir les griffes pour éviter les blessures et les problèmes d'appui.",
          "10 – 15 min", "Facile",
          "Griffes courtes, sans toucher le sol en marchant.",
          {}, {}, {},
          { "Coupe-griffes adapté à l'espèce", "Poudre hémostatique" },
          { "Tenir la patte fermement, sans serrer.",
            "Repérer la partie vivante (rose) de la griffe.",
            "Couper uniquement le bout, 2 mm avant la partie vivante.",
            "Limer les bords si besoin.",
            "En cas de saignement : poudre hémostatique." },
          { "Griffes noires : couper très peu à la fois.", "Toujours avoir de la poudre hémostatique à portée de main." } },

        { "bec_ailes", "Entretien du bec et des ailes",
          "Raccourcir les rémiges ou limer le bec : acte vétérinaire pour oiseaux de compagnie.",
          "20 – 30 min", "Difficile",
          "Bec bien aligné ; ailes taillées pour limiter les chutes et les fuites.",
          { "Oiseau" }, {}, {},
          { "Ciseaux de précision", "Lime / pierre à bec", "Serviette de contention" },
          { "Contention douce dans une serviette, la tête dégagée.",
            "Repérer les plumes à couper (sans sang, sans canal vasculaire).",
            "Couper les rémiges sur les deux ailes de manière symétrique.",
            "Limer le bec si trop long (jamais couper une partie vivante).",
            "Remettre l'oiseau au calme." },
          { "Plumes en croissance (« en sang ») : ne pas couper.", "Acte réservé au vétérinaire." } },
    };
    return catalogue;
}

bool GroomingCatalog::deconseillee(const GroomingCut &c, const QString &race)
{
    for (const QString &r : c.deconseille)
        if (r.compare(race, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}

QList<GroomingCut> GroomingCatalog::pour(const QString &espece, const QString &race)
{
    QList<GroomingCut> out;
    for (const GroomingCut &c : tous()) {
        if (!c.especes.isEmpty() && !espece.isEmpty() && !c.especes.contains(espece, Qt::CaseInsensitive))
            continue;
        const bool racesOk = c.races.isEmpty() || race.isEmpty()
                             || c.races.contains(race, Qt::CaseInsensitive) || deconseillee(c, race);
        if (!racesOk)
            continue;
        out.append(c);
    }
    return out;
}

QString GroomingCatalog::slug(const QString &text)
{
    QString s = text.toLower();
    const QString from = QString::fromUtf8("éèêëàâäîïôöûùüç");
    const QString to = QStringLiteral("eeeeaaaiioouuuc");
    for (int k = 0; k < from.size(); ++k)
        s.replace(from[k], to[k]);
    s.replace(' ', '_');
    s.replace('-', '_');
    s.remove('(');
    s.remove(')');
    s.remove('\'');
    s.remove(QChar(0x2019));
    return s;
}

namespace {

// Espèce montrée par la photo générique <id>.jpg des coupes communes à plusieurs espèces.
// (vide = la coupe est propre à une seule espèce : la photo générique convient)
QString especeDeLaPhotoGenerique(const GroomingCut &c)
{
    if (c.especes.size() == 1)          // coupe propre à une seule espèce : la photo générique convient
        return QString();
    if (c.id == "hygienique") return "Chien";
    if (c.id == "demelage")   return "Chat";
    if (c.id == "griffes")    return "Chien";
    return QString();
}

QStringList dossiersPhotos()
{
    return { QCoreApplication::applicationDirPath() + "/coupes", QDir::currentPath() + "/coupes",
             QCoreApplication::applicationDirPath() + "/../coupes" };
}

// cherche un fichier <nom>.<ext> sur le disque puis dans les ressources ; renvoie une pixmap nulle sinon
QPixmap chargerPhoto(const QString &nom)
{
    for (const QString &d : dossiersPhotos())
        for (const char *ext : { "jpg", "jpeg", "png", "webp" }) {
            const QString path = QString("%1/%2.%3").arg(d, nom, ext);
            if (QFileInfo::exists(path)) {
                QPixmap p(path);
                if (!p.isNull())
                    return p;
            }
        }
    for (const QString &ext : { QStringLiteral("jpg"), QStringLiteral("jpeg"), QStringLiteral("png"), QStringLiteral("webp") }) {
        QPixmap res(QStringLiteral(":/med/coupes/") + nom + QStringLiteral(".") + ext);
        if (!res.isNull())
            return res;
    }
    return QPixmap();
}

} // namespace

bool GroomingCatalog::photoSpecifique(const GroomingCut &c, const QString &espece, const QString &race, QString *manquant)
{
    if (espece.isEmpty())
        return true;
    const QString es = slug(espece);
    if (!race.isEmpty() && !chargerPhoto(c.id + "_" + es + "_" + slug(race)).isNull())
        return true;
    if (!chargerPhoto(c.id + "_" + es).isNull())
        return true;
    const QString generique = especeDeLaPhotoGenerique(c);
    if (generique.isEmpty() || generique.compare(espece, Qt::CaseInsensitive) == 0)
        return !chargerPhoto(c.id).isNull();
    if (manquant)
        *manquant = QString("coupes/%1_%2.jpg").arg(c.id, es);
    return false;
}

QPixmap GroomingCatalog::image(const GroomingCut &c, const QSize &size, const QString &espece, const QString &race)
{
    // 1) photo propre à la race, puis à l'espèce, puis photo générique si elle montre la bonne espèce
    QStringList candidats;
    if (!espece.isEmpty()) {
        if (!race.isEmpty())
            candidats << c.id + "_" + slug(espece) + "_" + slug(race);
        candidats << c.id + "_" + slug(espece);
    }
    const QString generique = especeDeLaPhotoGenerique(c);
    if (espece.isEmpty() || generique.isEmpty() || generique.compare(espece, Qt::CaseInsensitive) == 0)
        candidats << c.id;
    else
        candidats << "animal_" + slug(espece);        // pas de photo de cette coupe pour cette espèce : photo de l'espèce
    for (const QString &nom : std::as_const(candidats)) {
        const QPixmap p = chargerPhoto(nom);
        if (!p.isNull())
            return p.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // 3) image de remplacement dessinée
    QPixmap pm(size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient g(0, 0, size.width(), size.height());
    g.setColorAt(0, QColor("#E8F4F6"));
    g.setColorAt(1, QColor("#BFE0E8"));
    p.setBrush(g);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(QRectF(0, 0, size.width(), size.height()), 16, 16);
    QPixmap paw(":/med/icons/paw.svg");
    if (!paw.isNull()) {
        const int s = qMin(size.width(), size.height()) / 3;
        p.drawPixmap((size.width() - s) / 2, size.height() / 5, paw.scaled(s, s, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    QFont f = p.font();
    f.setBold(true);
    f.setPointSize(13);
    p.setFont(f);
    p.setPen(QColor("#174B5A"));
    p.drawText(QRectF(10, size.height() * 0.62, size.width() - 20, size.height() * 0.2),
               Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, c.nom);
    f.setBold(false);
    f.setPointSize(9);
    p.setFont(f);
    p.drawText(QRectF(10, size.height() * 0.82, size.width() - 20, size.height() * 0.15),
               Qt::AlignHCenter | Qt::AlignTop, QString("(ajouter coupes/%1.jpg)").arg(c.id));
    return pm;
}
