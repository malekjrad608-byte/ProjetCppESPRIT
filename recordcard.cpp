#include "recordcard.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QSizePolicy>
#include <QStyle>
#include <QRegularExpression>

// Carte compacte (même présentation que la maquette) :
//   [                         ✔ Vaccins à jour ]
//   Nom de l'animal
//   Espèce · Race
//   🕒 date de consultation              Propriétaire
RecordCard::RecordCard(const MedicalRecord &record, QWidget *parent)
    : QFrame(parent), m_recordId(record.getIdDossier())
{
    setObjectName("recordCard");
    setProperty("selected", false);
    setCursor(Qt::PointingHandCursor);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 12, 16, 12);
    root->setSpacing(3);

    // Ligne du haut : badge de statut à droite
    auto *top = new QHBoxLayout();
    m_top = top;
    top->setContentsMargins(0, 0, 0, 0);
    top->addStretch();
    const bool upToDate = record.getStatutVaccination() == "À jour";
    auto *badge = new QLabel(upToDate ? QString::fromUtf8("✔  Vaccins à jour")
                                      : QString::fromUtf8("⚠  À renouveler"));
    badge->setObjectName("cardBadge");
    badge->setProperty("tone", upToDate ? "ok" : "warn");
    top->addWidget(badge);
    root->addLayout(top);

    auto *title = new QLabel(record.getNomAnimal());
    title->setObjectName("recordTitle");
    title->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    root->addWidget(title);

    auto *sub = new QLabel(QString("%1 · %2").arg(record.getEspece(), record.getRace()));
    sub->setObjectName("recordSub");
    sub->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    root->addWidget(sub);

    root->addStretch();

    // Ligne du bas : date de consultation à gauche, propriétaire à droite
    auto *bottom = new QHBoxLayout();
    bottom->setContentsMargins(0, 0, 0, 0);
    auto *date = new QLabel(QString::fromUtf8("🕒 ") + record.getDateConsultation().toString("dd/MM/yyyy"));
    date->setObjectName("recordMeta");
    bottom->addWidget(date);
    bottom->addStretch();
    auto *owner = new QLabel(record.getProprietaire());
    owner->setObjectName("recordMeta");
    owner->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    owner->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    bottom->addWidget(owner, 1);
    root->addLayout(bottom);

    for (QLabel *l : findChildren<QLabel *>())
        l->setAttribute(Qt::WA_TransparentForMouseEvents);
}

void RecordCard::setSelected(bool selected)
{
    setProperty("selected", selected);
    style()->unpolish(this);
    style()->polish(this);
}

void RecordCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked(m_recordId);
    QFrame::mousePressEvent(event);
}

void RecordCard::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit doubleClicked(m_recordId);
    QFrame::mouseDoubleClickEvent(event);
}


// ---------------------------------------------------------------- score de risque santé
QString RiskResult::libelle() const
{
    return level == Eleve ? QString::fromUtf8("Élevé") : (level == Moyen ? QString("Moyen") : QString("Faible"));
}

namespace {
QString norm(const QString &t)
{
    QString s = t.toLower();
    const QString from = QString::fromUtf8("éèêëàâäîïôöûùüç");
    const QString to = QStringLiteral("eeeeaaaiioouuuc");
    for (int i = 0; i < from.size(); ++i)
        s.replace(from[i], to[i]);
    return s;
}
}

RiskResult RiskScore::calculer(const MedicalRecord &r, const QList<HealthAlert> &alertes)
{
    RiskResult res;
    auto ajoute = [&res](int pts, const QString &raison) {
        res.score += pts;
        res.raisons << QString("%1 (+%2)").arg(raison).arg(pts);
    };

    // 1) âge selon l'espèce
    const QString esp = norm(r.getEspece());
    const int age = r.getAge();
    const int seuilSenior = esp.contains("lapin") ? 6 : (esp.contains("chien") || esp.contains("chat") ? 8 : 10);
    if (age >= seuilSenior + 3)
        ajoute(25, QString::fromUtf8("Animal très âgé (%1 ans)").arg(age));
    else if (age >= seuilSenior)
        ajoute(15, QString::fromUtf8("Animal âgé (%1 ans)").arg(age));
    else if (age == 0)
        ajoute(10, QString::fromUtf8("Très jeune animal"));

    // 2) vaccination
    if (r.getStatutVaccination() == QString::fromUtf8("À renouveler"))
        ajoute(25, QString::fromUtf8("Vaccin à renouveler"));
    if (r.getVaccinations().trimmed().isEmpty())
        ajoute(10, QString::fromUtf8("Aucun vaccin renseigné"));

    // 3) allergies
    if (!r.getAllergies().trimmed().isEmpty() && !norm(r.getAllergies()).contains("aucune"))
        ajoute(10, QString::fromUtf8("Allergie connue"));

    // 4) diagnostic
    const QString d = norm(r.getDiagnostic());
    static const QStringList graves = { "parvo", "leptospir", "cancer", "tumeur", "insuffisance", "diabet", "pneumon",
                                        "typhus", "leucose", "carre", "rage", "grave", "septic" };
    static const QStringList routine = { "controle", "routine", "bonne sante", "bilan", "vaccin", "aucun", "normal" };
    bool grave = false, routineDiag = d.trimmed().isEmpty();
    for (const QString &g : graves) if (d.contains(g)) grave = true;
    for (const QString &g : routine) if (d.contains(g)) routineDiag = true;
    if (grave)
        ajoute(25, QString::fromUtf8("Diagnostic sérieux"));
    else if (!routineDiag)
        ajoute(10, QString::fromUtf8("Pathologie en cours"));

    // 5) poids inhabituel pour l'espèce (0 = non renseigné : ignoré)
    const double p = r.getPoids();
    if (p > 0) {
        if ((esp.contains("chat") && (p > 7.5 || p < 1.5)) || (esp.contains("lapin") && (p > 3.5 || p < 0.8))
            || (esp.contains("chien") && (p > 50 || p < 1.0)))
            ajoute(10, QString::fromUtf8("Poids inhabituel (%1 kg)").arg(p, 0, 'f', 1));
    }

    // 6) veille sanitaire : récidive ou foyer
    const QString id = r.getIdAnimal().trimmed().toUpper();
    for (const HealthAlert &a : alertes) {
        bool concerne = false;
        for (const QString &x : a.idAnimaux)
            if (x.trimmed().toUpper() == id) concerne = true;
        if (!concerne)
            continue;
        if (a.type == HealthAlert::Recidive)
            ajoute(20, QString::fromUtf8("Récidive : %1").arg(a.diagnostic));
        else
            ajoute(15, QString::fromUtf8("Foyer possible : %1").arg(a.diagnostic));
    }

    res.score = qMin(100, res.score);
    res.level = res.score >= 50 ? RiskResult::Eleve : (res.score >= 25 ? RiskResult::Moyen : RiskResult::Faible);
    if (res.raisons.isEmpty())
        res.raisons << QString::fromUtf8("Aucun facteur de risque détecté");
    return res;
}

void RecordCard::setRisk(const RiskResult &risk)
{
    if (!m_top)
        return;
    const bool high = risk.level == RiskResult::Eleve, mid = risk.level == RiskResult::Moyen;
    auto *lab = new QLabel(QString::fromUtf8("🩺 Risque %1 · %2").arg(risk.libelle()).arg(risk.score));
    lab->setStyleSheet(QString("QLabel { color: %1; background: %2; border-radius: 8px; padding: 2px 8px; font-weight: bold; }")
                           .arg(high ? "#B42318" : (mid ? "#B54708" : "#1E7B46"),
                                high ? "#FDECEA" : (mid ? "#FFF6E5" : "#EAF7EF")));
    lab->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_top->insertWidget(0, lab);
    setToolTip(QString::fromUtf8("Score de risque santé : %1 / 100 (%2)\n• %3\n\nAide à la décision, à valider par le vétérinaire.")
                   .arg(risk.score).arg(risk.libelle(), risk.raisons.join("\n• ")));
}
