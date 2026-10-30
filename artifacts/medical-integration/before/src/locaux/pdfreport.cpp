#include "pdfreport.h"
#include "alerts.h"

#include <QFile>
#include <QSaveFile>
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

int rank(const Local &l)
{
    const QString p = AlertEngine::priorityText(l);
    if (p == "URGENT")  return 0;
    if (p == "Haute")   return 1;
    if (p == "Normale") return 2;
    return 3;   // en cours
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
    if (p == "Haute")
        return td(QStringLiteral("<b><font color=\"#b97712\">Haute</font></b>"), "bgcolor=\"#fff4e2\"");
    if (p == "En cours")
        return td(QStringLiteral("<font color=\"#b97712\">En cours</font>"), "bgcolor=\"#fff4e2\"");
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
    QSaveFile output(path);
    if (!output.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QStringLiteral("Impossible d'écrire dans « %1 ». Ferme le PDF s'il est déjà ouvert.").arg(path);
        return false;
    }


    const QDateTime now = QDateTime::currentDateTime();
    const QLocale fr(QLocale::French);

    QList<Local> todo, clean;
    int nDirty = 0, nProg = 0, nBusy = 0, nOos = 0;
    for (const Local &l : all) {
        if (l.cleaning == CleaningState::Clean)
            clean.append(l);
        else
            todo.append(l);
        if (l.cleaning == CleaningState::ToClean) ++nDirty;
        if (l.cleaning == CleaningState::InProgress) ++nProg;
        if (l.state() == RoomState::Occupied) ++nBusy;
        if (l.outOfService) ++nOos;
    }
    std::stable_sort(todo.begin(), todo.end(), [](const Local &a, const Local &b) {
        if (rank(a) != rank(b))
            return rank(a) < rank(b);
        if (a.dirtyMinutes() != b.dirtyMinutes())
            return a.dirtyMinutes() > b.dirtyMinutes();
        return a.floor < b.floor;
    });

    const QList<Alert> alerts = AlertEngine::evaluate(all);

    QString h;
    h += "<html><body style=\"font-family:'Segoe UI',Arial,sans-serif; font-size:10pt; color:#1d3b44;\">";

    // en-tete
    h += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\"><tr>"
         "<td width=\"80\"><img src=\"logo\" width=\"64\" height=\"64\"></td>"
         "<td><span style=\"font-size:22pt; font-weight:bold; color:#1f5560;\">ANIWORLD</span><br>"
         "<span style=\"font-size:14pt; color:#2f7482;\">Rapport de nettoyage des locaux</span></td>";
    h += QStringLiteral("<td align=\"right\"><span style=\"color:#6f8a92;\">Édité le %1<br>Seuil d'alerte : %2 min (blocs : %3 min)</span></td></tr></table>")
             .arg(fr.toString(now, "dddd d MMMM yyyy 'à' HH:mm"))
             .arg(AlertEngine::dirtyThresholdMinutes())
             .arg(qMax(1, AlertEngine::dirtyThresholdMinutes() / 2));
    h += "<hr>";

    // compteurs
    h += "<table width=\"100%\" cellspacing=\"8\" cellpadding=\"8\"><tr>";
    h += statBox(nDirty, "à nettoyer", "#fbe9e8", "#c0504d");
    h += statBox(nProg, "nettoyage en cours", "#fff4e2", "#b97712");
    h += statBox(clean.size(), "propres", "#e3f3e9", "#2e8b57");
    h += statBox(nBusy, "occupés", "#e6e9f8", "#4a63b8");
    h += statBox(nOos, "hors service", "#eef6f8", "#6f8a92");
    h += "</tr></table>";

    // alertes
    if (!alerts.isEmpty()) {
        h += "<p><b><font color=\"#1f5560\" size=\"4\">Alertes actives</font></b></p><table cellspacing=\"0\" cellpadding=\"3\">";
        for (const Alert &a : alerts) {
            const char *color = a.level == Alert::Critical ? "#c0504d" : a.level == Alert::Warning ? "#b97712" : "#4a63b8";
            const char *tag = a.level == Alert::Critical ? "CRITIQUE" : a.level == Alert::Warning ? "ATTENTION" : "INFO";
            h += QStringLiteral("<tr><td><b><font color=\"%1\">%2</font></b></td><td>%3 — <font color=\"#6f8a92\">%4</font></td></tr>")
                     .arg(QString::fromLatin1(color), QString::fromLatin1(tag), a.title.toHtmlEscaped(), a.detail.toHtmlEscaped());
        }
        h += "</table>";
    }

    // locaux a nettoyer
    h += "<p><b><font color=\"#1f5560\" size=\"4\">Locaux à nettoyer</font></b></p>";
    if (todo.isEmpty()) {
        h += "<p><font color=\"#2e8b57\">Tous les locaux sont propres.</font></p>";
    } else {
        h += "<table border=\"1\" cellspacing=\"0\" cellpadding=\"6\" width=\"100%\" style=\"border-color:#cfdde1; border-style:solid;\"><tr>";
        h += th("Priorité") + th("Local") + th("Type") + th("Étage") + th("État") + th("Sale depuis")
             + th("Nettoyage") + th("Fait") + th("Remarques / agent");
        h += "</tr>";
        for (const Local &l : todo) {
            const QString state = l.stateText() + (l.occupancy > 0 ? QStringLiteral(" (%1/%2)").arg(l.occupancy).arg(l.capacity) : QString());
            h += "<tr>";
            h += priorityCell(AlertEngine::priorityText(l));
            h += td("<b>" + l.name.toHtmlEscaped() + "</b>");
            h += td(l.type.toHtmlEscaped());
            h += td(l.floorShort());
            h += td(state.toHtmlEscaped());
            h += td(l.cleaning == CleaningState::ToClean ? AlertEngine::formatMinutes(l.dirtyMinutes()) : QStringLiteral("—"));
            h += td(l.cleaningText());
            h += td("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
            h += td("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
            h += "</tr>";
        }
        h += "</table>";
    }

    // locaux propres
    h += "<p><b><font color=\"#1f5560\" size=\"4\">Locaux propres (contrôle)</font></b></p>";
    if (clean.isEmpty()) {
        h += "<p><font color=\"#6f8a92\">Aucun.</font></p>";
    } else {
        h += "<table border=\"1\" cellspacing=\"0\" cellpadding=\"5\" width=\"100%\" style=\"border-color:#cfdde1; border-style:solid;\"><tr>";
        h += th("Local") + th("Type") + th("Étage") + th("État") + th("Dernier nettoyage");
        h += "</tr>";
        for (const Local &l : clean) {
            h += "<tr>";
            h += td(l.name.toHtmlEscaped());
            h += td(l.type.toHtmlEscaped());
            h += td(l.floorShort());
            h += td(l.stateText());
            h += td(fr.toString(l.lastCleaned, "dd MMM yyyy HH:mm"));
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
    h += "<p><font color=\"#6f8a92\" size=\"2\">Consignes : traiter d'abord les locaux URGENT (blocs, isolement, ou sales depuis plus de deux fois le seuil), "
         "puis les locaux « Haute ». Cocher « Fait » après désinfection et noter toute anomalie.</font></p>";
    h += "</body></html>";

    {
    QPdfWriter writer(&output);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Landscape);
    writer.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);
    writer.setTitle(QStringLiteral("ANIWORLD - Rapport de nettoyage"));
    writer.setCreator(QStringLiteral("ANIWORLD"));

    QTextDocument doc;
    doc.addResource(QTextDocument::ImageResource, QUrl(QStringLiteral("logo")),
                    QVariant::fromValue(QPixmap(QStringLiteral(":/brand/logo.png"))));
    doc.setHtml(h);
    doc.print(&writer);
    }
    if(!output.commit()) { if(error) *error=output.errorString(); return false; }
    return true;
}

} // namespace PdfReport
