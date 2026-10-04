#include "gestionnairesalles.h"
#include <QFile>
#include <QTextStream>
#include <QPrinter>
#include <QTextDocument>
#include <QDir>
#include <algorithm>

GestionnaireSalles::GestionnaireSalles() {
    initialiserCages();
    cheminSauvegarde = "salles.csv";
}

GestionnaireSalles::~GestionnaireSalles() {
    vider();
}

void GestionnaireSalles::vider() {
    for (Salle* s : salles) delete s;
    salles.clear();
}

void GestionnaireSalles::initialiserCages() {
    cagesPension.clear();
    cagesAdoption.clear();
    for (int i = 1; i <= 20; ++i) {
        cagesPension.append(Cage(i));
        cagesAdoption.append(Cage(i));
    }
}

// ==================== CRUD ====================
bool GestionnaireSalles::ajouter(Salle* s) {
    if (!s) return false;
    if (rechercherParId(s->getId())) return false;
    salles.append(s);
    synchroniserCages();
    sauvegarderAuto();
    return true;
}

bool GestionnaireSalles::supprimer(int id) {
    for (int i = 0; i < salles.size(); ++i) {
        if (salles[i]->getId() == id) {
            delete salles[i];
            salles.remove(i);
            sauvegarderAuto();
            return true;
        }
    }
    return false;
}

Salle* GestionnaireSalles::rechercherParId(int id) const {
    for (Salle* s : salles) if (s->getId() == id) return s;
    return nullptr;
}

// ==================== Recherche & Tri ====================
QVector<Salle*> GestionnaireSalles::rechercher(const QString& critere) const {
    QVector<Salle*> resultat;
    QString c = critere.toLower();
    for (Salle* s : salles) {
        if (s->getNom().toLower().contains(c) ||
            s->getType().toLower().contains(c) ||
            QString::number(s->getId()).contains(c)) {
            resultat.append(s);
        }
    }
    return resultat;
}

QVector<Salle*> GestionnaireSalles::trierParCapacite(bool ascendant) const {
    QVector<Salle*> copie = salles;
    std::sort(copie.begin(), copie.end(), [ascendant](Salle* a, Salle* b) {
        return ascendant ? a->getCapacite() < b->getCapacite()
                         : a->getCapacite() > b->getCapacite();
    });
    return copie;
}

QVector<Salle*> GestionnaireSalles::trierParOccupation(bool ascendant) const {
    QVector<Salle*> copie = salles;
    std::sort(copie.begin(), copie.end(), [ascendant](Salle* a, Salle* b) {
        return ascendant ? a->getOccupation() < b->getOccupation()
                         : a->getOccupation() > b->getOccupation();
    });
    return copie;
}

// ==================== Métier ====================
// Les salles Pension/Adoption se gèrent cage par cage (voir plus bas).
bool GestionnaireSalles::affecterAnimal(int idSalle) {
    Salle* s = rechercherParId(idSalle);
    if (!s) return false;
    if (s->getType() == "Pension" || s->getType() == "Adoption") return false;
    bool ok = s->affecterAnimal();
    if (ok) sauvegarderAuto();
    return ok;
}

bool GestionnaireSalles::libererSalle(int idSalle) {
    Salle* s = rechercherParId(idSalle);
    if (!s) return false;
    if (s->getType() == "Pension" || s->getType() == "Adoption") return false;
    s->liberer();
    sauvegarderAuto();
    return true;
}

bool GestionnaireSalles::changerEtat(int idSalle, const QString& nouvelEtat) {
    Salle* s = rechercherParId(idSalle);
    if (!s) return false;
    s->setEtat(nouvelEtat);
    sauvegarderAuto();
    return true;
}

// ==================== Cages ====================
bool GestionnaireSalles::occuperCagePension(int numero, const QString& nom,
                                            const QString& type,
                                            const QDate& date, int jours) {
    if (numero < 1 || numero > cagesPension.size()) return false;
    if (cagesPension[numero - 1].estOccupee()) return false;
    cagesPension[numero - 1].occuper(nom, type, date, jours);
    synchroniserCages();
    sauvegarderAuto();
    return true;
}

bool GestionnaireSalles::occuperCageAdoption(int numero, const QString& nom,
                                             const QString& type,
                                             const QDate& date, int jours) {
    if (numero < 1 || numero > cagesAdoption.size()) return false;
    if (cagesAdoption[numero - 1].estOccupee()) return false;
    cagesAdoption[numero - 1].occuper(nom, type, date, jours);
    synchroniserCages();
    sauvegarderAuto();
    return true;
}

bool GestionnaireSalles::libererCagePension(int numero) {
    if (numero < 1 || numero > cagesPension.size()) return false;
    cagesPension[numero - 1].liberer();
    synchroniserCages();
    sauvegarderAuto();
    return true;
}

bool GestionnaireSalles::libererCageAdoption(int numero) {
    if (numero < 1 || numero > cagesAdoption.size()) return false;
    cagesAdoption[numero - 1].liberer();
    synchroniserCages();
    sauvegarderAuto();
    return true;
}

int GestionnaireSalles::sejoursDepasses(bool pension) const {
    const QVector<Cage>& v = pension ? cagesPension : cagesAdoption;
    int n = 0;
    for (const Cage& c : v) if (c.sejourDepasse()) ++n;
    return n;
}

// L'occupation d'une salle Pension/Adoption = nombre de cages occupées.
void GestionnaireSalles::synchroniserCages() {
    int nbPension = 0, nbAdoption = 0;
    for (const Cage& c : cagesPension) if (c.estOccupee()) ++nbPension;
    for (const Cage& c : cagesAdoption) if (c.estOccupee()) ++nbAdoption;
    for (Salle* s : salles) {
        if (s->getType() == "Pension") s->setOccupation(nbPension);
        else if (s->getType() == "Adoption") s->setOccupation(nbAdoption);
    }
}

// ==================== Statistiques ====================
Statistiques GestionnaireSalles::calculerStatistiques() const {
    Statistiques st;
    st.totalSalles = salles.size();
    for (Salle* s : salles) {
        st.capaciteTotale += s->getCapacite();
        st.occupationTotale += s->getOccupation();
        if (s->getEtat() == "Libre") st.sallesLibres++;
        else if (s->getEtat() == "Occupée") st.sallesOccupees++;
        else st.sallesHorsService++;
        st.repartitionParType[s->getType()]++;
    }
    if (st.capaciteTotale > 0)
        st.tauxOccupation = (double)st.occupationTotale / st.capaciteTotale * 100.0;
    return st;
}

// ==================== CSV ====================
bool GestionnaireSalles::sauvegarderCSV(const QString& chemin) const {
    QFile file(chemin);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
    out << "id;nom;type;capacite;occupation;etat;extra\n";
    for (Salle* s : salles) out << s->toCSV() << "\n";

    out << "---CAGES_PENSION---\n";
    for (const Cage& c : cagesPension) out << c.toCSV() << "\n";

    out << "---CAGES_ADOPTION---\n";
    for (const Cage& c : cagesAdoption) out << c.toCSV() << "\n";

    file.close();
    return true;
}

bool GestionnaireSalles::sauvegarderAuto() const {
    return sauvegarderCSV(cheminSauvegarde);
}

bool GestionnaireSalles::chargerCSV(const QString& chemin) {
    QFile file(chemin);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&file);
    vider();
    initialiserCages();

    bool cagesPensionMode = false;
    bool cagesAdoptionMode = false;

    in.readLine();  // skip header
    while (!in.atEnd()) {
        QString ligne = in.readLine();

        if (ligne == "---CAGES_PENSION---") {
            cagesPensionMode = true;
            cagesAdoptionMode = false;
            continue;
        }
        if (ligne == "---CAGES_ADOPTION---") {
            cagesPensionMode = false;
            cagesAdoptionMode = true;
            continue;
        }

        if (cagesPensionMode || cagesAdoptionMode) {
            QStringList parts = ligne.split(";");
            if (parts.size() < 6) continue;
            int num = parts[0].toInt();
            if (num < 1 || num > 20) continue;
            if (parts[1] == "occupee") {
                Cage c(num);
                c.occuper(parts[2], parts[3],
                          QDate::fromString(parts[4], "yyyy-MM-dd"),
                          parts[5].toInt());
                if (cagesPensionMode) cagesPension[num - 1] = c;
                else cagesAdoption[num - 1] = c;
            }
            continue;
        }

        // Salles normales
        QStringList parts = ligne.split(";");
        if (parts.size() < 7) continue;
        int id = parts[0].toInt();
        QString nom = parts[1];
        QString type = parts[2];
        int cap = parts[3].toInt();
        int occ = parts[4].toInt();
        QString etat = parts[5];
        QString extra = parts[6];

        Salle* s = nullptr;
        if (type == "Consultation") s = new SalleConsultation(id, nom, cap, extra);
        else if (type == "Chirurgie") s = new SalleChirurgie(id, nom, cap, extra);
        else if (type == "Toilettage") s = new SalleToilettage(id, nom, cap, extra);
        else if (type == "Pension") s = new SallePension(id, nom, cap, extra);
        else if (type == "Adoption") s = new SalleAdoption(id, nom, cap, extra);
        else if (type == "Storage") s = new SalleStorage(id, nom, cap, extra);

        if (s) {
            // setOccupation puis setEtat : l'état lu dans le fichier prime
            // (l'ancienne boucle d'affectation perdait l'occupation des salles
            // "Occupée" ou "Hors service").
            s->setOccupation(occ);
            s->setEtat(etat);
            salles.append(s);
        }
    }
    file.close();
    synchroniserCages();
    return true;
}

// ==================== Export PDF ====================
bool GestionnaireSalles::exporterPDF(const QString& chemin) const {
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(chemin);

    QTextDocument doc;
    QString html = "<h1 style='color:#4A8797;'>ANIWORLD - Liste des Salles</h1>";
    html += "<table border='1' cellspacing='0' cellpadding='6' width='100%'>";
    html += "<tr style='background:#4A8797; color:white;'>"
            "<th>ID</th><th>Nom</th><th>Type</th><th>Capacite</th>"
            "<th>Occupation</th><th>Etat</th></tr>";
    for (Salle* s : salles) {
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td>"
                        "<td>%4</td><td>%5</td><td>%6</td></tr>")
                .arg(s->getId()).arg(s->getNom()).arg(s->getType())
                .arg(s->getCapacite()).arg(s->getOccupation()).arg(s->getEtat());
    }
    html += "</table>";

    auto listeCages = [&html](const QString& titre, const QVector<Cage>& cages) {
        html += QString("<h2 style='color:#4A8797;'>%1</h2><ul>").arg(titre);
        for (const Cage& c : cages) {
            if (!c.estOccupee()) continue;
            html += QString("<li>Cage %1 : %2 (%3) - %4 jours - sortie prevue le %5%6</li>")
                    .arg(c.getNumero()).arg(c.getNomAnimal())
                    .arg(c.getTypeAnimal()).arg(c.getNombreJours())
                    .arg(c.dateSortie().toString("dd/MM/yyyy"))
                    .arg(c.sejourDepasse() ? " <b style='color:#C0392B;'>(sejour depasse)</b>" : "");
        }
        html += "</ul>";
    };
    listeCages("Cages Pension", cagesPension);
    listeCages("Cages Adoption", cagesAdoption);

    doc.setHtml(html);
    doc.print(&printer);
    return true;
}
