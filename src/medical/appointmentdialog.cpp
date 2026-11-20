#include "appointmentdialog.h"
#include "smsservice.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QLocale>

namespace { enum Col { CAnimal, CProprio, CTel, CDate, CMotif, CStatut, CSms, CCount }; }

AppointmentDialog::AppointmentDialog(const QVector<MedicalRecord> &records, QWidget *parent)
    : QDialog(parent), m_records(records)
{
    setWindowTitle("Rendez-vous");
    resize(1100, 560);

    m_sms = new SmsService(this);
    connect(m_sms, &SmsService::finished, this,
            [this](const QString &ref, bool ok, bool simulated, const QString &info) {
        // ref = "RDV|<id>|R" (rappel) ou "RDV|<id>|M" (manqué)
        const QStringList p = ref.split('|');
        if (p.size() == 3 && ok && !simulated)
            RdvManager::marquerSms(p[1], p[2] == "M", QDate::currentDate());
        m_status->setText(ok ? (simulated ? info : QString("SMS envoyé.")) : "Échec : " + info);
        reload();
    });

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(12);

    auto *title = new QLabel("Rendez-vous des animaux");
    title->setObjectName("statisticsTitle");
    root->addWidget(title);
    auto *sub = new QLabel("Rappel SMS automatique la veille du rendez-vous, et SMS « rendez-vous manqué » le lendemain "
                           "si l'animal n'est pas venu (un seul SMS de chaque type).");
    sub->setObjectName("statisticsSubtitle");
    sub->setWordWrap(true);
    root->addWidget(sub);

    m_table = new QTableWidget(0, CCount);
    m_table->setObjectName("statisticsTable");
    m_table->setHorizontalHeaderLabels({"Animal", "Propriétaire", "Téléphone", "Date et heure", "Motif", "Statut", "SMS"});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(CSms, QHeaderView::Stretch);
    root->addWidget(m_table, 1);

    m_status = new QLabel();
    m_status->setObjectName("statisticsSubtitle");
    m_status->setWordWrap(true);
    root->addWidget(m_status);

    auto *buttons = new QHBoxLayout;
    auto *btnCame = new QPushButton("Marquer « venu »");
    btnCame->setObjectName("outlineButton");
    auto *btnCancel = new QPushButton("Annuler le RDV");
    btnCancel->setObjectName("outlineButton");
    buttons->addWidget(btnCame);
    buttons->addWidget(btnCancel);
    buttons->addStretch();
    auto *btnClose = new QPushButton("Fermer");
    btnClose->setObjectName("outlineButton");
    auto *btnSend = new QPushButton(QString::fromUtf8("📩  Envoyer SMS"));
    btnSend->setObjectName("primaryButton");
    buttons->addWidget(btnClose);
    buttons->addWidget(btnSend);
    root->addLayout(buttons);

    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnSend, &QPushButton::clicked, this, &AppointmentDialog::onSendSelected);
    connect(btnCame, &QPushButton::clicked, this, &AppointmentDialog::onMarkCame);
    connect(btnCancel, &QPushButton::clicked, this, &AppointmentDialog::onCancelRdv);

    reload();
}

void AppointmentDialog::reload()
{
    RdvManager::majStatuts(m_records);
    m_rows.clear();
    for (const RendezVous &r : RdvManager::tous(m_records))
        if (r.statut == "Prévu" || r.statut == "Manqué")
            m_rows.append(r);

    m_table->setRowCount(m_rows.size());
    const QLocale fr(QLocale::French);
    for (int i = 0; i < m_rows.size(); ++i) {
        const RendezVous &r = m_rows[i];
        QString sms = "Pas encore";
        if (SmsService::normalizePhone(r.telephone).isEmpty())
            sms = "Téléphone manquant / invalide";
        else if (r.statut == "Manqué")
            sms = r.smsManque.isValid() ? "« Manqué » envoyé le " + fr.toString(r.smsManque, "dd/MM/yyyy") : "« Manqué » à envoyer";
        else
            sms = r.smsRappel.isValid() ? "Rappel envoyé le " + fr.toString(r.smsRappel, "dd/MM/yyyy") : "Rappel prévu la veille";

        m_table->setItem(i, CAnimal, new QTableWidgetItem(QString("%1 (%2)").arg(r.nomAnimal, r.idAnimal)));
        m_table->setItem(i, CProprio, new QTableWidgetItem(r.proprietaire));
        m_table->setItem(i, CTel, new QTableWidgetItem(r.telephone));
        m_table->setItem(i, CDate, new QTableWidgetItem(fr.toString(r.dateRdv, "dd/MM/yyyy HH:mm")));
        m_table->setItem(i, CMotif, new QTableWidgetItem(r.motif));
        auto *st = new QTableWidgetItem(r.statut);
        st->setForeground(r.statut == "Manqué" ? QColor("#C45458") : QColor("#176078"));
        QFont f = st->font(); f.setBold(true); st->setFont(f);
        m_table->setItem(i, CStatut, st);
        m_table->setItem(i, CSms, new QTableWidgetItem(sms));
        m_table->setRowHeight(i, 42);
    }
}

int AppointmentDialog::currentRow() const
{
    const int row = m_table->currentRow();
    return (row >= 0 && row < m_rows.size()) ? row : -1;
}

void AppointmentDialog::onSendSelected()
{
    const int row = currentRow();
    if (row < 0) {
        QMessageBox::information(this, "SMS", "Sélectionnez d'abord un rendez-vous.");
        return;
    }
    const RendezVous &r = m_rows[row];
    const bool manque = r.statut == "Manqué";
    m_sms->send(r.telephone, manque ? SmsService::composeRdvManque(r) : SmsService::composeRdvRappel(r),
                QString("RDV|%1|%2").arg(r.idRdv, manque ? "M" : "R"));
}

void AppointmentDialog::onMarkCame()
{
    const int row = currentRow();
    if (row >= 0) {
        RdvManager::changerStatut(m_rows[row].idRdv, "Venu");
        reload();
    }
}

void AppointmentDialog::onCancelRdv()
{
    const int row = currentRow();
    if (row < 0)
        return;
    if (QMessageBox::question(this, "Confirmation", "Annuler ce rendez-vous ?") == QMessageBox::Yes) {
        RdvManager::changerStatut(m_rows[row].idRdv, "Annulé");
        reload();
    }
}
