#include "pdfreport.h"

#include <QDateTime>
#include <QFile>
#include <QLocale>
#include <QMarginsF>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPixmap>
#include <QTextDocument>
#include <QUrl>
#include <algorithm>

namespace PdfReport {

namespace {

// Priorite de nettoyage : les blocs (chirurgie) et l'isolement passent d'abord
QString priorityText(const Local &l)
{
    return l.isUrgentType() ? QStringLiteral("URGENT") : QStringLiteral("Normale");
}

QString th(const QString &text)
{
    return QStringLiteral("<td bgcolor=\"#2f7482\"><b><font color=\"#ffffff\">%1</font></b></td>").arg(text);
}

QString td(const QString &text, const QString &extra = QString())
{
    return QStringLiteral("<td %1>%2</td>").arg(extra, text);
}

QString priorityCell(const QString &p)
{
    if (p == "URGENT")
        return td(QStringLiteral("<b><font color=\"#c0504d\">URGENT</font></b>"), "bgcolor=\"#fbe9e8\"");
    return td(p);
}

QString statBox(int value, const QString &label, const QString &bg, const QString &fg)
{
    return QStringLiteral("<td align=\"center\" bgcolor=\"%1\"><span style=\"font-size:18pt; font-weight:bold; color:%2;\">%3</span><br>"
                          "<span style=\"font-size:9pt; color:%2;\">%4</span></td>")
        .arg(bg, fg).arg(value).arg(label);
}

} // namespace

bool generate(const QList<Local> &all, const QString &path, QString *error)
{
    QFile probe(path);
    if (!probe.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QStringLiteral("Impossible d'écrire dans « %1 ». Ferme le PDF s'il est déjà ouvert.").arg(path);
        return false;
    }
    probe.close();

    const QDateTime now = QDateTime::currentDateTime();
    const QLocale fr(QLocale::French);

    QList<Local> todo, clean;
    int nBusy = 0, nOos = 0;
    for (const Local &l : all) {
        if (l.getEtat() == Local::ANettoyer)
            todo.append(l);
        else
            clean.append(l);
        if (l.getEtat() == Local::Occupe) ++nBusy;
        if (l.getEtat() == Local::HorsService) ++nOos;
    }
    // urgents d'abord, puis par etage
    std::stable_sort(todo.begin(), todo.end(), [](const Local &a, const Local &b) {
        if (a.isUrgentType() != b.isUrgentType())
            return a.isUrgentType();
        return a.getFloor() < b.getFloor();
    });

    QString h;
    h += "<html><body style=\"font-family:'Segoe UI',Arial,sans-serif; font-size:10pt; color:#1d3b44;\">";

    // en-tete
    h += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\"><tr>"
         "<td width=\"80\"><img src=\"logo\" width=\"64\" height=\"64\"></td>"
         "<td><span style=\"font-size:22pt; font-weight:bold; color:#1f5560;\">ANIWORLD</span><br>"
         "<span style=\"font-size:14pt; color:#2f7482;\">Rapport de nettoyage des locaux</span></td>";
    h += QStringLiteral("<td align=\"right\"><span style=\"color:#6f8a92;\">Édité le %1</span></td></tr></table>")
             .arg(fr.toString(now, "dddd d MMMM yyyy 'à' HH:mm"));
    h += "<hr>";

    // compteurs
    h += "<table width=\"100%\" cellspacing=\"8\" cellpadding=\"8\"><tr>";
    h += statBox(todo.size(), "à nettoyer", "#fbe9e8", "#c0504d");
    h += statBox(clean.size(), "propres", "#e3f3e9", "#2e8b57");
    h += statBox(nBusy, "occupés", "#e6e9f8", "#4a63b8");
    h += statBox(nOos, "hors service", "#eef6f8", "#6f8a92");
    h += "</tr></table>";

    // locaux a nettoyer
    h += "<p><b><font color=\"#1f5560\" size=\"4\">Locaux à nettoyer</font></b></p>";
    if (todo.isEmpty()) {
        h += "<p><font color=\"#2e8b57\">Tous les locaux sont propres.</font></p>";
    } else {
        h += "<table border=\"1\" cellspacing=\"0\" cellpadding=\"6\" width=\"100%\" style=\"border-color:#cfdde1; border-style:solid;\"><tr>";
        h += th("Priorité") + th("Local") + th("Type") + th("Étage") + th("Spécialité")
             + th("Fait") + th("Remarques / agent");
        h += "</tr>";
        for (const Local &l : todo) {
            h += "<tr>";
            h += priorityCell(priorityText(l));
            h += td("<b>" + l.getName().toHtmlEscaped() + "</b>");
            h += td(l.getType().toHtmlEscaped());
            h += td(l.floorShort());
            h += td(l.getSpecialty().isEmpty() ? QStringLiteral("—") : l.getSpecialty().toHtmlEscaped());
            h += td("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
            h += td("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
            h += "</tr>";
        }
        h += "</table>";
    }

    // locaux propres
    h += "<p><b><font color=\"#1f5560\" size=\"4\">Autres locaux (contrôle)</font></b></p>";
    if (clean.isEmpty()) {
        h += "<p><font color=\"#6f8a92\">Aucun.</font></p>";
    } else {
        h += "<table border=\"1\" cellspacing=\"0\" cellpadding=\"5\" width=\"100%\" style=\"border-color:#cfdde1; border-style:solid;\"><tr>";
        h += th("Local") + th("Type") + th("Étage") + th("État");
        h += "</tr>";
        for (const Local &l : clean) {
            h += "<tr>";
            h += td(l.getName().toHtmlEscaped());
            h += td(l.getType().toHtmlEscaped());
            h += td(l.floorShort());
            h += td(l.getEtat().toHtmlEscaped());
            h += "</tr>";
        }
        h += "</table>";
    }

    // signatures
    h += "<p>&nbsp;</p><table width=\"100%\" cellspacing=\"0\" cellpadding=\"6\"><tr>"
         "<td>Agent d'entretien : ______________________</td>"
         "<td>Début : ______ h ______</td>"
         "<td>Fin : ______ h ______</td>"
         "<td>Validé par : ______________________</td></tr></table>";
    h += "<p><font color=\"#6f8a92\" size=\"2\">Consignes : traiter d'abord les locaux URGENT (chirurgie et isolement), "
         "puis les autres. Cocher « Fait » après désinfection et noter toute anomalie.</font></p>";
    h += "</body></html>";

    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);
    writer.setTitle(QStringLiteral("ANIWORLD - Rapport de nettoyage"));
    writer.setCreator(QStringLiteral("ANIWORLD"));

    QTextDocument doc;
    doc.addResource(QTextDocument::ImageResource, QUrl(QStringLiteral("logo")),
                    QVariant::fromValue(QPixmap(QStringLiteral(":/images/logo.png"))));
    doc.setHtml(h);
    doc.print(&writer);
    return true;
}

} // namespace PdfReport
