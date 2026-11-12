#include "animalpdf.h"
#include "appointment.h"
#include "vaccination.h"

#include <QPdfWriter>
#include <QPageSize>
#include <QPageLayout>
#include <QTextDocument>
#include <QLocale>
#include <QDate>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDateTime>
#include <algorithm>

namespace {

QString esc(const QString &s) { return s.toHtmlEscaped().replace("\n", "<br>"); }
QString dateFr(const QDate &d) { return d.isValid() ? QLocale(QLocale::French).toString(d, "dd/MM/yyyy") : QString("-"); }
QString dtFr(const QDateTime &d) { return d.isValid() ? QLocale(QLocale::French).toString(d, "dd/MM/yyyy 'à' HH:mm") : QString("-"); }

QString cell(const QString &s, const QString &extra = QString())
{
    return QString("<td style='padding:5px 6px; border-bottom:1px solid #D7E9ED;%1'>%2</td>").arg(extra, s.isEmpty() ? "-" : s);
}
QString head(const QStringList &cols)
{
    QString h = "<tr>";
    for (const QString &c : cols)
        h += QString("<th align='left' style='background:#176078; color:#FFFFFF; padding:6px;'>%1</th>").arg(c);
    return h + "</tr>";
}

// consultations de l'animal, de la plus récente à la plus ancienne
QVector<MedicalRecord> consultations(const QString &idAnimal, const QVector<MedicalRecord> &records)
{
    QVector<MedicalRecord> v;
    for (const MedicalRecord &r : records)
        if (r.getIdAnimal().compare(idAnimal, Qt::CaseInsensitive) == 0)
            v.append(r);
    std::sort(v.begin(), v.end(), [](const MedicalRecord &a, const MedicalRecord &b) {
        return a.getDateConsultation() > b.getDateConsultation(); });
    return v;
}

} // namespace

QString AnimalPdf::nomFichierPropose(const QString &idAnimal, const QVector<MedicalRecord> &records)
{
    const QVector<MedicalRecord> c = consultations(idAnimal, records);
    QString nom = c.isEmpty() ? idAnimal : c.first().getNomAnimal();
    nom.replace(QRegularExpression("[^\\p{L}\\p{N}_-]+"), "_");
    return QString("Fiche_%1_%2.pdf").arg(nom, idAnimal);
}

bool AnimalPdf::exporter(const QString &idAnimal, const QVector<MedicalRecord> &records, const QString &path)
{
    const QVector<MedicalRecord> cons = consultations(idAnimal, records);
    if (cons.isEmpty())
        return false;
    const MedicalRecord &last = cons.first();            // dossier le plus récent = infos à jour
    const QDate today = QDate::currentDate();

    // ---- résumé
    const int nb = cons.size();
    const QDate premiere = cons.last().getDateConsultation();
    const QDate derniere = last.getDateConsultation();

    RendezVous prochain;
    const bool aRdv = RdvManager::prochain(idAnimal, records, prochain);
    const QList<RendezVous> rdvs = RdvManager::pourAnimal(idAnimal, records);
    int manques = 0;
    for (const RendezVous &r : rdvs) if (r.statut == "Manqué") ++manques;

    QString tel = RdvManager::dernierTelephone(idAnimal, records);

    // ---- HTML
    QString html;
    html += "<html><body style='font-family:Segoe UI, Arial; font-size:10pt; color:#1F3A44;'>";
    html += "<table width='100%'><tr><td><span style='font-size:20pt; font-weight:bold; color:#176078;'>ANIWORLD</span><br>"
            "<span style='color:#5F7E88;'>Fiche de l'animal et historique des consultations</span></td>";
    html += QString("<td align='right' style='color:#5F7E88;'>Édité le %1</td></tr></table><hr>").arg(dateFr(today));

    html += QString("<h2 style='color:#176078; margin-bottom:2px;'>%1 <span style='font-size:11pt; color:#5F7E88;'>(%2)</span></h2>")
                .arg(esc(last.getNomAnimal()), esc(idAnimal));

    html += "<table width='100%' cellspacing='0'>";
    html += QString("<tr>%1%2%3</tr>")
                .arg(cell("<b>Espèce :</b> " + esc(last.getEspece())),
                     cell("<b>Race :</b> " + esc(last.getRace())),
                     cell("<b>Sexe :</b> " + esc(last.getSexe())));
    html += QString("<tr>%1%2%3</tr>")
                .arg(cell(QString("<b>Âge :</b> %1 an(s)").arg(last.getAge())),
                     cell(QString("<b>Poids :</b> %1 kg").arg(last.getPoids(), 0, 'f', 1)),
                     cell("<b>Propriétaire :</b> " + esc(last.getProprietaire())));
    html += QString("<tr>%1%2%3</tr>")
                .arg(cell("<b>Téléphone :</b> " + esc(tel)),
                     cell("<b>Allergies :</b> " + esc(last.getAllergies())),
                     cell("<b>Vaccinations :</b> " + esc(last.getStatutVaccination())));
    html += "</table><br>";

    // ---- cartes résumé
    auto box = [](const QString &titre, const QString &valeur, const QString &couleur) {
        return QString("<td align='center' style='background:%3; padding:10px; border:1px solid #D7E9ED;'>"
                       "<span style='color:#5F7E88;'>%1</span><br><span style='font-size:15pt; font-weight:bold; color:#176078;'>%2</span></td>")
                   .arg(titre, valeur, couleur);
    };
    html += "<table width='100%' cellspacing='6'><tr>";
    html += box("Consultations", QString::number(nb), "#E3F0F3");
    html += box("Première visite", dateFr(premiere), "#E8F5EC");
    html += box("Dernière visite", dateFr(derniere), "#FFF3DE");
    html += box("Prochain RDV", aRdv ? dtFr(prochain.dateRdv) : QString("Aucun"), aRdv ? "#FDE9E7" : "#F2F6F8");
    html += "</tr></table>";
    if (manques > 0)
        html += QString("<p style='color:#C45458;'><b>%1 rendez-vous manqué(s)</b></p>").arg(manques);

    // ---- historique des consultations
    html += "<h3 style='color:#176078;'>Historique des consultations</h3>";
    html += "<table width='100%' cellspacing='0'>" + head({"Date", "Motif", "Diagnostic", "Traitement", "Poids"});
    for (const MedicalRecord &r : cons)
        html += "<tr>" + cell(dateFr(r.getDateConsultation()), " white-space:nowrap;") + cell(esc(r.getMotif()))
                + cell(esc(r.getDiagnostic())) + cell(esc(r.getTraitement()))
                + cell(QString("%1 kg").arg(r.getPoids(), 0, 'f', 1)) + "</tr>";
    html += "</table>";

    // ---- observations du dernier dossier
    if (!last.getObservations().trimmed().isEmpty())
        html += "<h3 style='color:#176078;'>Observations (dernière consultation)</h3><p>" + esc(last.getObservations()) + "</p>";

    // ---- vaccinations
    QList<Vaccination> vacs;
    for (const Vaccination &v : VaccinationManager::derniers(records))
        if (v.idAnimal.compare(idAnimal, Qt::CaseInsensitive) == 0)
            vacs.append(v);
    if (!vacs.isEmpty()) {
        std::sort(vacs.begin(), vacs.end(), [](const Vaccination &a, const Vaccination &b) { return a.dateRappel < b.dateRappel; });
        html += "<h3 style='color:#176078;'>Vaccinations et rappels</h3>";
        html += "<table width='100%' cellspacing='0'>" + head({"Vaccin", "Dernière date", "Prochain rappel", "État"});
        for (const Vaccination &v : vacs) {
            const Vaccination::Etat e = v.etat();
            const QString etat = e == Vaccination::EnRetard ? QString("<span style='color:#C45458;'><b>En retard (%1 j)</b></span>").arg(-v.joursRestants())
                               : e == Vaccination::Bientot ? QString("<span style='color:#B9781A;'><b>À renouveler</b></span>")
                                                           : QString("<span style='color:#21834E;'><b>À jour</b></span>");
            html += "<tr>" + cell(esc(v.nomVaccin)) + cell(dateFr(v.dateVaccin)) + cell(dateFr(v.dateRappel)) + cell(etat) + "</tr>";
        }
        html += "</table>";
    }

    // ---- rendez-vous
    if (!rdvs.isEmpty()) {
        html += "<h3 style='color:#176078;'>Rendez-vous</h3>";
        html += "<table width='100%' cellspacing='0'>" + head({"Date et heure", "Motif", "Statut"});
        for (int i = rdvs.size() - 1; i >= 0; --i) {
            const RendezVous &r = rdvs[i];
            const QString col = r.statut == "Manqué" ? "#C45458" : (r.statut == "Venu" ? "#21834E" : "#176078");
            html += "<tr>" + cell(dtFr(r.dateRdv)) + cell(esc(r.motif))
                    + cell(QString("<span style='color:%1;'><b>%2</b></span>").arg(col, esc(r.statut))) + "</tr>";
        }
        html += "</table>";
    }

    html += "</body></html>";

    // ---- écriture PDF (A4, marges 15 mm)
    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    writer.setResolution(120);
    writer.setTitle("Fiche " + last.getNomAnimal());

    QTextDocument doc;
    doc.setHtml(html);
    doc.setPageSize(QSizeF(writer.width(), writer.height()));
    doc.print(&writer);
    return QFileInfo(path).exists();
}
