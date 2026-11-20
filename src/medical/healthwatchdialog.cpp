#include "healthwatchdialog.h"
#include "smsservice.h"
#include "appointment.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextBrowser>
#include <QMessageBox>
#include <QLocale>
#include <QMap>
#include <utility>

namespace { enum Col { CNiveau, CType, CDiag, CAnimaux, CPeriode, CCount }; }

HealthWatchDialog::HealthWatchDialog(const QVector<MedicalRecord> &records, QWidget *parent)
    : QDialog(parent), m_records(records)
{
    setWindowTitle("Veille sanitaire");
    resize(1100, 640);

    m_alerts = HealthWatch::analyser(records);
    m_sms = new SmsService(this);
    connect(m_sms, &SmsService::finished, this, [this](const QString &, bool ok, bool simulated, const QString &info) {
        if (ok && !simulated) {
            ++m_smsOk;
        } else {                                   // jamais compté comme envoyé si ce n'est pas un vrai envoi
            ++m_smsFail;
            if (m_smsError.isEmpty()) m_smsError = info;
        }
        if (--m_smsPending <= 0)
            m_status->setText(QString("SMS de prévention : %1 envoyé(s), %2 échec(s).%3")
                                  .arg(m_smsOk).arg(m_smsFail)
                                  .arg(m_smsError.isEmpty() ? QString() : " Dernière erreur : " + m_smsError));
    });

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(12);

    int eleve = 0;
    for (const HealthAlert &a : std::as_const(m_alerts)) if (a.level == HealthAlert::Eleve) ++eleve;
    auto *title = new QLabel(m_alerts.isEmpty()
        ? QString::fromUtf8("✅  Aucune alerte sanitaire pour le moment")
        : QString::fromUtf8("🩺  %1 alerte(s) sanitaire(s) dont %2 de niveau élevé").arg(m_alerts.size()).arg(eleve));
    title->setObjectName("statisticsTitle");
    root->addWidget(title);

    auto *sub = new QLabel(QString("Détection automatique à partir des diagnostics : récidive d'un même animal (%1 passages ou plus en "
                                   "6 mois) et foyer possible (%2 animaux ou plus avec le même diagnostic en %3 jours). "
                                   "Aide à la décision : à valider par le vétérinaire.")
                               .arg(HealthWatch::kRecidiveMin).arg(HealthWatch::kFoyerMin).arg(HealthWatch::kFoyerJours));
    sub->setObjectName("statisticsSubtitle");
    sub->setWordWrap(true);
    root->addWidget(sub);

    m_table = new QTableWidget(m_alerts.size(), CCount);
    m_table->setObjectName("statisticsTable");
    m_table->setHorizontalHeaderLabels({"Niveau", "Type", "Diagnostic", "Animaux concernés", "Période"});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(CAnimaux, QHeaderView::Stretch);
    m_table->setMaximumHeight(220);

    const QLocale fr(QLocale::French);
    for (int i = 0; i < m_alerts.size(); ++i) {
        const HealthAlert &a = m_alerts[i];
        auto *niveau = new QTableWidgetItem(a.level == HealthAlert::Eleve ? QString::fromUtf8("🔴 Élevé") : QString::fromUtf8("🟠 Attention"));
        QFont f = niveau->font(); f.setBold(true); niveau->setFont(f);
        niveau->setForeground(a.level == HealthAlert::Eleve ? QColor("#C45458") : QColor("#B9781A"));
        m_table->setItem(i, CNiveau, niveau);
        m_table->setItem(i, CType, new QTableWidgetItem(a.type == HealthAlert::Foyer ? "Foyer possible" : "Récidive"));
        m_table->setItem(i, CDiag, new QTableWidgetItem(a.diagnostic));
        m_table->setItem(i, CAnimaux, new QTableWidgetItem(a.animaux.join(", ")));
        m_table->setItem(i, CPeriode, new QTableWidgetItem(QString("%1 → %2")
            .arg(fr.toString(a.dates.first(), "dd/MM/yyyy"), fr.toString(a.dates.last(), "dd/MM/yyyy"))));
        m_table->setRowHeight(i, 42);
    }
    root->addWidget(m_table);

    m_details = new QTextBrowser;
    root->addWidget(m_details, 1);

    m_status = new QLabel;
    m_status->setObjectName("statisticsSubtitle");
    m_status->setWordWrap(true);
    root->addWidget(m_status);

    auto *buttons = new QHBoxLayout;
    buttons->addStretch();
    auto *btnClose = new QPushButton("Fermer");
    btnClose->setObjectName("outlineButton");
    auto *btnSms = new QPushButton(QString::fromUtf8("📩  Prévenir les propriétaires (SMS)"));
    btnSms->setObjectName("primaryButton");
    buttons->addWidget(btnClose);
    buttons->addWidget(btnSms);
    root->addLayout(buttons);

    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnSms, &QPushButton::clicked, this, &HealthWatchDialog::onSendSms);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &HealthWatchDialog::onSelectionChanged);

    // le médecin a vu les alertes : elles ne réapparaissent plus en popup (sauf nouveau cas)
    HealthWatch::marquerVues(m_alerts);

    if (!m_alerts.isEmpty())
        m_table->selectRow(0);
    else
        m_details->setHtml("<p>Aucune récidive ni foyer détecté dans les dossiers récents.</p>");
}

void HealthWatchDialog::onSelectionChanged()
{
    const int row = m_table->currentRow();
    if (row < 0 || row >= m_alerts.size())
        return;
    const HealthAlert &a = m_alerts[row];
    const QLocale fr(QLocale::French);

    QString dates;
    for (const QDate &d : a.dates)
        dates += fr.toString(d, "dd/MM/yyyy") + " · ";
    dates.chop(3);

    QString html = QString("<h3>%1</h3>").arg(a.resume.toHtmlEscaped());
    html += "<p><b>Diagnostic :</b> " + a.diagnostic.toHtmlEscaped() + "<br>";
    html += "<b>Espèce(s) :</b> " + a.especes.join(", ").toHtmlEscaped() + "<br>";
    html += "<b>Dates des consultations :</b> " + dates + "</p>";
    html += "<p><b>Animaux concernés :</b></p><ul>";
    for (const QString &n : a.animaux)
        html += "<li>" + n.toHtmlEscaped() + "</li>";
    html += "</ul><h3>Pistes à valider</h3><p>" + a.conseil.toHtmlEscaped() + "</p>";
    m_details->setHtml(html);
}

void HealthWatchDialog::onSendSms()
{
    const int row = m_table->currentRow();
    if (row < 0 || row >= m_alerts.size()) {
        QMessageBox::information(this, "SMS", "Sélectionnez d'abord une alerte.");
        return;
    }
    const HealthAlert &a = m_alerts[row];

    // Pas de service SMS configuré : on n'envoie rien et on ne prétend pas avoir envoyé.
    if (!m_sms->isConfigured()) {
        m_status->setText("Aucun SMS envoyé : service SMS non configuré.");
        QMessageBox::warning(this, "SMS non configuré",
            "Aucun service SMS n'est configuré : aucun SMS n'a été envoyé.\n\n"
            "Définissez les variables d'environnement ANIWORLD_SMS_SID, ANIWORLD_SMS_TOKEN et "
            "ANIWORLD_SMS_FROM (compte Twilio), puis relancez l'application.");
        return;
    }

    // Un SMS par numéro de propriétaire : plusieurs animaux d'un même foyer sont regroupés.
    struct Envoi { QString tel, texte, ref; };
    QMap<QString, QStringList> animauxParTelephone;
    int sansNumero = 0;
    for (int i = 0; i < a.idAnimaux.size(); ++i) {
        const QString tel = SmsService::normalizePhone(RdvManager::dernierTelephone(a.idAnimaux[i], m_records));
        if (tel.isEmpty()) { ++sansNumero; continue; }
        QString nom = a.animaux[i];
        const int parenthese = nom.lastIndexOf(" (");
        if (parenthese >= 0) nom = nom.left(parenthese);
        if (!animauxParTelephone[tel].contains(nom))
            animauxParTelephone[tel].append(nom);
    }
    QList<Envoi> envois;
    for (auto it = animauxParTelephone.cbegin(); it != animauxParTelephone.cend(); ++it) {
        const QString noms = it.value().join(", ");
        const QString texte = a.type == HealthAlert::Foyer
            ? QString("ANIWORLD : plusieurs cas de %1 ont ete signales a la clinique. Surveillez : %2 (isolement des malades, hygiene) "
                      "et contactez-nous au moindre symptome.").arg(a.diagnostic, noms)
            : QString("ANIWORLD : %1 a deja ete soigne plusieurs fois pour %2. Merci de prendre rendez-vous pour un controle "
                      "approfondi.").arg(noms, a.diagnostic);
        envois.append({ it.key(), texte, QString("VEILLE|%1").arg(it.key()) });
    }
    if (envois.isEmpty()) {
        QMessageBox::warning(this, "SMS", "Aucun numéro de téléphone connu pour les animaux concernés.\n"
                                          "Le numéro est récupéré depuis un rendez-vous ou une vaccination enregistrée pour l'animal.");
        return;
    }
    if (QMessageBox::question(this, "Confirmation",
            QString("Envoyer %1 SMS de prévention ?%2").arg(envois.size())
                .arg(sansNumero ? QString("\n(%1 animal(aux) sans numéro seront ignorés)").arg(sansNumero) : QString())) != QMessageBox::Yes)
        return;

    m_smsOk = m_smsFail = 0;
    m_smsError.clear();
    m_smsPending = envois.size();
    for (const Envoi &e : std::as_const(envois))
        m_sms->send(e.tel, e.texte, e.ref);
}
