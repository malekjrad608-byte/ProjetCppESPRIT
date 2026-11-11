#include "gestionemployes.h"
#include "connection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDateTime>
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QTableWidget>
#include <QStackedWidget>
#include <QTextEdit>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QPrinter>
#include <QTextDocument>
#include <QStandardPaths>
#include <QDir>
#include <QSqlError>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QScreen>
#include <QGuiApplication>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDate>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QColor>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <QSet>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTableWidgetItem>
#include <QSizePolicy>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPaintEvent>
#include <QVector>
#include <cmath>
#include <algorithm>

namespace {
// Graphique personnalisé Qt (sans dépendance supplémentaire à Qt Charts).
// Il compare la charge hebdomadaire à la disponibilité de chaque employé.
class CourbeChargeWidget : public QWidget
{
public:
    explicit CourbeChargeWidget(const QList<Employe::ChargeEmploye> &charges,
                                QWidget *parent = nullptr)
        : QWidget(parent), m_charges(charges)
    {
        setMinimumHeight(210);
        setFixedHeight(230);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setAttribute(Qt::WA_OpaquePaintEvent, false);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), Qt::white);

        if (m_charges.isEmpty()) {
            p.setPen(QColor("#7A8B8F"));
            p.drawText(rect(), Qt::AlignCenter,
                       "Aucune donnée disponible pour cette semaine.");
            return;
        }

        const QRectF plot(66.0, 43.0, qMax(120, width() - 92),
                          qMax(120, height() - 132));

        // Légende
        p.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        p.setPen(QColor("#2F8796"));
        p.setBrush(QColor("#2F8796"));
        p.drawEllipse(QPointF(78, 19), 4, 4);
        p.drawText(QRectF(89, 9, 150, 20), Qt::AlignVCenter, "Charge (h)");
        p.setPen(QColor("#E5A13A"));
        p.setBrush(QColor("#E5A13A"));
        p.drawEllipse(QPointF(210, 19), 4, 4);
        p.drawText(QRectF(221, 9, 190, 20), Qt::AlignVCenter,
                   "Disponibilité (h)");

        double maximum = 5.0;
        for (const Employe::ChargeEmploye &c : m_charges)
            maximum = qMax(maximum, qMax(c.charge, c.dispo));
        const double pas = qMax(1.0, std::ceil(maximum / 5.0));
        const double maxAxe = pas * 5.0;

        // Grille et graduations verticales.
        p.setFont(QFont("Segoe UI", 8));
        for (int i = 0; i <= 5; ++i) {
            const double y = plot.bottom() - (plot.height() * i / 5.0);
            p.setPen(QPen(QColor("#E8EFF2"), 1, Qt::DashLine));
            p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
            p.setPen(QColor("#7A8B8F"));
            const QString valeur = QString::number(pas * i, 'f', 0);
            p.drawText(QRectF(8, y - 9, 48, 18), Qt::AlignRight | Qt::AlignVCenter,
                       valeur);
        }
        p.setPen(QPen(QColor("#C8D9DE"), 1.2));
        p.drawLine(plot.bottomLeft(), plot.topLeft());
        p.drawLine(plot.bottomLeft(), plot.bottomRight());

        QVector<QPointF> pointsCharge;
        QVector<QPointF> pointsDispo;
        pointsCharge.reserve(m_charges.size());
        pointsDispo.reserve(m_charges.size());
        for (int i = 0; i < m_charges.size(); ++i) {
            const double x = m_charges.size() == 1
                ? plot.center().x()
                : plot.left() + (plot.width() * i / (m_charges.size() - 1));
            const auto yPour = [&](double valeur) {
                return plot.bottom() - (qBound(0.0, valeur, maxAxe) / maxAxe)
                                           * plot.height();
            };
            pointsCharge.append(QPointF(x, yPour(m_charges.at(i).charge)));
            pointsDispo.append(QPointF(x, yPour(m_charges.at(i).dispo)));
        }

        auto tracerCourbe = [](const QVector<QPointF> &points) {
            QPainterPath path;
            if (points.isEmpty()) return path;
            path.moveTo(points.first());
            for (int i = 1; i < points.size(); ++i) {
                const QPointF a = points.at(i - 1);
                const QPointF b = points.at(i);
                const qreal milieu = (a.x() + b.x()) / 2.0;
                path.cubicTo(QPointF(milieu, a.y()),
                             QPointF(milieu, b.y()), b);
            }
            return path;
        };

        const QPainterPath courbeCharge = tracerCourbe(pointsCharge);
        if (!pointsCharge.isEmpty()) {
            QPainterPath zone = courbeCharge;
            zone.lineTo(pointsCharge.last().x(), plot.bottom());
            zone.lineTo(pointsCharge.first().x(), plot.bottom());
            zone.closeSubpath();
            QLinearGradient remplissage(plot.topLeft(), plot.bottomLeft());
            remplissage.setColorAt(0.0, QColor(47, 135, 150, 58));
            remplissage.setColorAt(1.0, QColor(47, 135, 150, 5));
            p.fillPath(zone, remplissage);
        }

        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor("#E5A13A"), 2.8, Qt::SolidLine,
                      Qt::RoundCap, Qt::RoundJoin));
        p.drawPath(tracerCourbe(pointsDispo));
        p.setPen(QPen(QColor("#2F8796"), 3.4, Qt::SolidLine,
                      Qt::RoundCap, Qt::RoundJoin));
        p.drawPath(courbeCharge);

        // Points et valeurs de charge.
        p.setFont(QFont("Segoe UI", 8, QFont::DemiBold));
        for (int i = 0; i < pointsCharge.size(); ++i) {
            const QPointF pc = pointsCharge.at(i);
            const QPointF pd = pointsDispo.at(i);
            p.setPen(QPen(Qt::white, 1.5));
            p.setBrush(QColor("#2F8796"));
            p.drawEllipse(pc, 5.0, 5.0);
            p.setBrush(QColor("#E5A13A"));
            p.drawEllipse(pd, 4.0, 4.0);

            p.setPen(QColor("#225A64"));
            const QString valeur = QString::number(m_charges.at(i).charge, 'f', 1) + " h";
            const QRectF texte(pc.x() - 28, pc.y() - 24, 56, 17);
            p.drawText(texte, Qt::AlignCenter, valeur);
        }

        // Noms inclinés pour rester lisibles quand il y a plusieurs employés.
        p.setFont(QFont("Segoe UI", 8));
        p.setPen(QColor("#526970"));
        const int pasEtiquette = qMax(1, (m_charges.size() + 9) / 10);
        for (int i = 0; i < m_charges.size(); ++i) {
            if (i % pasEtiquette != 0 && i != m_charges.size() - 1)
                continue;
            const QPointF point = pointsCharge.at(i);
            QString nom = (m_charges.at(i).prenom + " " + m_charges.at(i).nom).trimmed();
            if (nom.isEmpty()) nom = "Employé";
            p.save();
            const double xEtiquette = (i == 0 && m_charges.size() > 1)
                ? qMax(116.0, point.x() + 2.0) : point.x() + 2.0;
            p.translate(xEtiquette, plot.bottom() + 9);
            p.rotate(-32);
            p.drawText(QRectF(-112, 0, 108, 16), Qt::AlignRight | Qt::AlignVCenter,
                       QFontMetrics(p.font()).elidedText(nom, Qt::ElideRight, 108));
            p.restore();
        }

        p.setPen(QColor("#7A8B8F"));
        p.setFont(QFont("Segoe UI", 8));
        p.drawText(QRectF(5, 0, 48, 18), Qt::AlignLeft | Qt::AlignVCenter,
                   "Heures");
    }

private:
    QList<Employe::ChargeEmploye> m_charges;
};
}

// ==================== Constructeur ====================
GestionEmployes::GestionEmployes(QWidget *parent) : QWidget(parent)
{
    construireUI();
    appliquerEtatSchema();
    if (m_schemaOk)
        chargerEmployes();
}

void GestionEmployes::actualiserStats()
{
    // Appele par MainWindow a chaque affichage de la page : on revalide l'etat
    // du schema (la connexion a pu etre retablie depuis la construction).
    appliquerEtatSchema();
    if (m_schemaOk)
        mettreAJourStats();
}

// Mode degrade du module : si le schema Oracle des Employes est non conforme,
// on affiche l'erreur ici et on desactive les boutons, sans toucher a la
// connexion Oracle (les autres modules continuent de fonctionner).
void GestionEmployes::appliquerEtatSchema()
{
    const Connection &cnx = Connection::createInstance();
    const bool ok = cnx.schemaEmployesOk();
    const bool etaitOk = m_schemaOk;
    m_schemaOk = ok;

    if (m_bandeauSchema) {
        if (!ok) {
            m_bandeauSchema->setText(
                QStringLiteral("⚠  Module Employés désactivé\n\n"
                               "La connexion Oracle fonctionne, mais la structure de la table EMPLOYER "
                               "n'est pas celle de la relation. Les autres modules "
                               "ne sont pas concernés.\n\nDétail :\n")
                + cnx.erreurSchemaEmployes()
                + QStringLiteral("\n\nVérifiez dans Oracle SQL Developer que la table EMPLOYER "
                                 "existe et contient les 17 colonnes de la relation, "
                                 "puis cliquez sur « Oracle connecté » en haut pour réessayer."));
        }
        m_bandeauSchema->setVisible(!ok);
    }

    const QString infobulle = ok
        ? QString()
        : QStringLiteral("Module Employés désactivé : schéma Oracle non conforme "
                         "(voir le message en haut de la page).");
    for (QPushButton *bouton : {m_btnNouveau, m_btnStats, m_btnEvaluation}) {
        if (!bouton) continue;
        bouton->setEnabled(ok);
        bouton->setToolTip(infobulle);
    }
    if (!ok) {
        effacerDetail();
        afficherCards(QList<Employe>());
    } else if (!etaitOk) {
        chargerEmployes();   // schema repare entre-temps : on recharge liste + stats
    }
}

// ==================== UI PRINCIPALE ====================
void GestionEmployes::construireUI()
{
    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(30, 25, 30, 25);
    main->setSpacing(18);

    main->addWidget(construireHeader());

    // Bandeau d'erreur du schema Employes (cache tant que tout va bien).
    m_bandeauSchema = new QLabel;
    m_bandeauSchema->setObjectName("bandeauSchemaEmployes");
    m_bandeauSchema->setWordWrap(true);
    m_bandeauSchema->setTextFormat(Qt::PlainText);
    m_bandeauSchema->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_bandeauSchema->setStyleSheet(
        "background-color: #FBE9E7; color: #8A2E27; border: 1px solid #E5B8B3;"
        "border-radius: 10px; padding: 14px 18px; font-size: 13px;");
    m_bandeauSchema->hide();
    main->addWidget(m_bandeauSchema);

    main->addWidget(construireStatsPills());
    main->addWidget(construireBarreRecherche());

    QHBoxLayout *corps = new QHBoxLayout;
    corps->setSpacing(20);
    corps->addWidget(construireGrid(), 3);
    corps->addWidget(construireDetailPanel(), 1);
    main->addLayout(corps, 1);
}

// ==================== HEADER ====================
QWidget *GestionEmployes::construireHeader()
{
    QWidget *w = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *titreL = new QVBoxLayout;
    titreL->setSpacing(4);

    QLabel *titre = new QLabel("👥  Employés");
    titre->setStyleSheet("font-size: 28px; font-weight: bold; color: #225A64;"
                         "background: transparent;");
    titreL->addWidget(titre);

    QLabel *sous = new QLabel("Annuaire, disponibilités et gestion de la charge");
    sous->setStyleSheet("font-size: 13px; color: #7A8B8F; background: transparent;");
    titreL->addWidget(sous);

    layout->addLayout(titreL);
    layout->addStretch();

    QPushButton *btnNouveau = new QPushButton("+  Ajouter un employé");
    m_btnNouveau = btnNouveau;
    btnNouveau->setObjectName("btnPrimary");
    btnNouveau->setMinimumHeight(44);
    btnNouveau->setMinimumWidth(210);
    btnNouveau->setCursor(Qt::PointingHandCursor);
    connect(btnNouveau, &QPushButton::clicked, this, &GestionEmployes::ajouterEmploye);
    layout->addWidget(btnNouveau);

    return w;
}

// ==================== STATS PILLS ====================
QWidget *GestionEmployes::construireStatsPills()
{
    QWidget *w = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto bounceIn = [](QWidget *wid, int delayMs) {
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(wid);
        wid->setGraphicsEffect(eff);
        eff->setOpacity(0);

        QTimer::singleShot(delayMs, [wid, eff]() {
            QPropertyAnimation *a1 = new QPropertyAnimation(eff, "opacity");
            a1->setDuration(400);
            a1->setStartValue(0.0);
            a1->setEndValue(1.0);
            a1->setEasingCurve(QEasingCurve::OutCubic);
            a1->start(QAbstractAnimation::DeleteWhenStopped);
        });
    };

    auto creerPill = [&](const QString &icone, const QString &bg, const QString &fg,
                         QLabel *&valeur, const QString &texte, int idx) {
        QFrame *pill = new QFrame;
        pill->setStyleSheet(QString(
                                "QFrame { background-color: %1; border-radius: 22px; border: none; }")
                                .arg(bg));
        pill->setFixedHeight(44);

        QHBoxLayout *l = new QHBoxLayout(pill);
        l->setContentsMargins(18, 5, 22, 5);
        l->setSpacing(8);

        QLabel *ico = new QLabel(icone);
        ico->setStyleSheet("font-size: 15px; background: transparent; border: none;");
        l->addWidget(ico);

        valeur = new QLabel("0");
        valeur->setStyleSheet(QString(
                                  "font-size: 16px; font-weight: bold; color: %1; "
                                  "background: transparent; border: none;").arg(fg));
        l->addWidget(valeur);

        QLabel *t = new QLabel(texte);
        t->setStyleSheet(QString(
                             "font-size: 12px; color: %1; background: transparent; border: none;")
                             .arg(fg));
        l->addWidget(t);

        bounceIn(pill, idx * 150);
        return pill;
    };

    QFrame *p1 = creerPill("✅", "#E6F4EA", "#2E7D32", m_lblDisponibles, "disponibles", 0);
    QFrame *p2 = creerPill("❌", "#FBE9E7", "#AE5751", m_lblIndisponibles, "indisponibles", 1);
    QFrame *p3 = creerPill("⏱", "#FDF3E3", "#B8741A", m_lblChargeTotale, "h de charge", 2);

    layout->addWidget(p1);
    layout->addWidget(p2);
    layout->addWidget(p3);

    layout->addStretch();

    QPushButton *btnEvaluation = new QPushButton("⭐  Performance & Évaluation");
    m_btnEvaluation = btnEvaluation;
    btnEvaluation->setObjectName("btnPrimary");
    btnEvaluation->setMinimumHeight(44);
    btnEvaluation->setMinimumWidth(220);
    btnEvaluation->setCursor(Qt::PointingHandCursor);
    connect(btnEvaluation, &QPushButton::clicked,
            this, &GestionEmployes::ouvrirEvaluationEmployes);
    layout->addWidget(btnEvaluation);

    QPushButton *btnStats = new QPushButton("📊  Voir les statistiques");
    m_btnStats = btnStats;
    btnStats->setObjectName("btnSecondary");
    btnStats->setMinimumHeight(44);
    btnStats->setCursor(Qt::PointingHandCursor);

    QGraphicsOpacityEffect *btnEff = new QGraphicsOpacityEffect(btnStats);
    btnStats->setGraphicsEffect(btnEff);
    btnEff->setOpacity(1.0);

    QPropertyAnimation *pulseBtn = new QPropertyAnimation(btnEff, "opacity");
    pulseBtn->setDuration(1800);
    pulseBtn->setStartValue(1.0);
    pulseBtn->setKeyValueAt(0.5, 0.75);
    pulseBtn->setEndValue(1.0);
    pulseBtn->setLoopCount(-1);
    pulseBtn->setEasingCurve(QEasingCurve::InOutSine);
    pulseBtn->start();

    connect(btnStats, &QPushButton::clicked, this, [this]() {
        QDialog dlg(this);
        dlg.setWindowTitle("Statistiques des employés");
        const QRect zoneEcran = QGuiApplication::primaryScreen()
                                    ? QGuiApplication::primaryScreen()->availableGeometry()
                                    : QRect(0, 0, 1280, 800);
        dlg.resize(qMin(1020, zoneEcran.width() - 70), qMin(720, zoneEcran.height() - 70));
        dlg.setMinimumSize(qMin(820, zoneEcran.width() - 30), qMin(590, zoneEcran.height() - 30));
        dlg.setSizeGripEnabled(true);
        dlg.setStyleSheet("QDialog { background-color: #F0F4F8; }");

        QVBoxLayout *mainL = new QVBoxLayout(&dlg);
        mainL->setContentsMargins(18, 16, 18, 14);
        mainL->setSpacing(8);

        QLabel *titre = new QLabel("📊  Statistiques des employés");
        titre->setStyleSheet("font-size: 22px; font-weight: bold; color: #225A64;"
                             "background: transparent;");
        mainL->addWidget(titre);

        QLabel *sous = new QLabel("Charge hebdomadaire et répartition par poste");
        sous->setStyleSheet("font-size: 13px; color: #7A8B8F; background: transparent;");
        mainL->addWidget(sous);

        QHBoxLayout *cardsL = new QHBoxLayout;
        cardsL->setSpacing(8);

        // Les requetes sont dans le modele : Employe::charges()
        const QList<Employe::ChargeEmploye> charges = Employe::charges();
        const Employe::StatsCharge stats = Employe::statistiques(charges);
        const int totalEmp = stats.total;
        const double totalCharge = stats.somme;
        const double avgCharge = stats.moyenne;
        const double maxCharge = stats.maximum;
        const double minCharge = stats.minimum;

        auto creerCard = [](const QString &icone, const QString &bg,
                            const QString &fg, const QString &titreCard,
                            const QString &valeur, const QString &sousTitre) {
            QFrame *card = new QFrame;
            card->setStyleSheet(QString(
                                    "QFrame { background-color: %1; border-radius: 14px; }").arg(bg));
            card->setMinimumHeight(76);

            QHBoxLayout *l = new QHBoxLayout(card);
            l->setContentsMargins(15, 12, 15, 12);
            l->setSpacing(12);

            QLabel *ico = new QLabel(icone);
            ico->setStyleSheet("font-size: 24px; background: transparent; border: none;");
            l->addWidget(ico);

            QVBoxLayout *textL = new QVBoxLayout;
            textL->setSpacing(2);

            QLabel *t = new QLabel(titreCard);
            t->setStyleSheet(QString(
                                 "font-size: 11px; color: %1; background: transparent;"
                                 "font-weight: 500; border: none;").arg(fg));
            textL->addWidget(t);

            QLabel *v = new QLabel(valeur);
            v->setStyleSheet(QString(
                                 "font-size: 18px; font-weight: bold; color: %1;"
                                 "background: transparent; border: none;").arg(fg));
            textL->addWidget(v);

            QLabel *s = new QLabel(sousTitre);
            s->setStyleSheet(QString(
                                 "font-size: 10px; color: %1; background: transparent; border: none;")
                                 .arg(fg));
            textL->addWidget(s);

            l->addLayout(textL);
            l->addStretch();
            return card;
        };

        cardsL->addWidget(creerCard("👥", "#E4F0F3", "#225A64",
                                    "Total", QString::number(totalEmp), "employés"));
        cardsL->addWidget(creerCard("⏱", "#FDF3E3", "#B8741A",
                                    "Charge totale", QString::number(totalCharge, 'f', 0) + " h", "par semaine"));
        cardsL->addWidget(creerCard("📈", "#E6F4EA", "#2E7D32",
                                    "Charge moyenne", QString::number(avgCharge, 'f', 1) + " h", "par employé"));
        cardsL->addWidget(creerCard("⚠️", "#FBE9E7", "#AE5751",
                                    "Charge max", QString::number(maxCharge, 'f', 0) + " h", "le plus chargé"));

        mainL->addLayout(cardsL);

        // Alertes et candidats de remplacement : aucun changement n'est fait dans Oracle.
        auto tauxCharge = [](const Employe::ChargeEmploye &e) {
            return e.dispo > 0.0 ? (e.charge / e.dispo) : (e.charge > 0.0 ? 1.0 : 0.0);
        };
        auto posteNormalise = [](QString poste) {
            poste = poste.trimmed().toLower();
            poste.replace("é", "e"); poste.replace("è", "e"); poste.replace("ê", "e");
            poste.replace("à", "a"); poste.replace("ô", "o");
            return poste;
        };
        auto candidatsRemplacants = [&](const Employe::ChargeEmploye &employe) {
            QList<Employe::ChargeEmploye> candidats;
            const QString posteCible = posteNormalise(employe.poste);
            for (const Employe::ChargeEmploye &candidat : charges) {
                if (candidat.id == employe.id || !candidat.disponible || candidat.dispo <= 0.0)
                    continue;
                if (posteNormalise(candidat.poste) != posteCible)
                    continue;
                if (candidat.charge >= candidat.dispo || tauxCharge(candidat) >= 0.90)
                    continue;
                candidats.append(candidat);
            }
            std::stable_sort(candidats.begin(), candidats.end(), [&](const auto &a, const auto &b) {
                return tauxCharge(a) < tauxCharge(b);
            });
            return candidats;
        };
        auto proposerRemplacant = [&](const Employe::ChargeEmploye &employe) {
            const QList<Employe::ChargeEmploye> candidats = candidatsRemplacants(employe);
            if (candidats.isEmpty())
                return QString("Aucun(e) %1 disponible avec une capacité restante.").arg(employe.poste);
            const Employe::ChargeEmploye &meilleur = candidats.first();
            return QString("Remplaçant proposé : %1 %2 — %3 h / %4 h (%5 %%).")
                .arg(meilleur.prenom, meilleur.nom, QString::number(meilleur.charge, 'f', 1),
                     QString::number(meilleur.dispo, 'f', 1))
                .arg(qRound(tauxCharge(meilleur) * 100.0));
        };

        // Les affectations sont conservées dans un fichier JSON local (pas de table Oracle).
        auto cheminFichierAffectations = []() {
            QString dossier = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            if (dossier.isEmpty())
                dossier = QDir::homePath() + QLatin1String("/.aniworld");
            QDir().mkpath(dossier);
            return dossier + QLatin1String("/affectations_remplacements.json");
        };
        QJsonObject affectations;
        {
            QFile fichier(cheminFichierAffectations());
            if (fichier.exists() && fichier.open(QIODevice::ReadOnly)) {
                QJsonParseError erreurJson;
                const QJsonDocument document = QJsonDocument::fromJson(fichier.readAll(), &erreurJson);
                fichier.close();
                if (erreurJson.error == QJsonParseError::NoError && document.isObject())
                    affectations = document.object().value("affectations").toObject();
            }
        }
        auto affectationActuelle = [&](const Employe::ChargeEmploye &employe) {
            const int idRemplacant = affectations.value(QString::number(employe.id))
                                         .toObject().value("remplacant_id").toInt();
            for (const Employe::ChargeEmploye &candidat : charges) {
                if (candidat.id == idRemplacant) {
                    return QString("Affecté : %1 %2 (%3 h / %4 h).")
                        .arg(candidat.prenom, candidat.nom,
                             QString::number(candidat.charge, 'f', 1),
                             QString::number(candidat.dispo, 'f', 1));
                }
            }
            return QString();
        };

        QList<Employe::ChargeEmploye> ciblesAffectation;
        int nbAbsents = 0;
        int nbSurcharges = 0;
        for (const Employe::ChargeEmploye &c : charges) {
            if (!c.disponible) {
                ++nbAbsents;
                ciblesAffectation.append(c);
            } else if (tauxCharge(c) >= 0.90) {
                ++nbSurcharges;
                ciblesAffectation.append(c);
            }
        }
        auto construireListeAlertes = [&]() {
            QStringList liste;
            for (const Employe::ChargeEmploye &c : charges) {
                const double taux = tauxCharge(c);
                const QString nom = (c.prenom + " " + c.nom).trimmed();
                if (!c.disponible) {
                    QString detail = affectationActuelle(c);
                    if (detail.isEmpty()) detail = proposerRemplacant(c);
                    liste << QString("🔴 ABSENT / INDISPONIBLE : %1 (%2).\n   %3")
                                 .arg(nom, c.poste, detail);
                } else if (taux >= 0.90) {
                    const QString niveau = taux >= 1.0 ? "CHARGE MAXIMALE" : "CHARGE PRESQUE MAXIMALE";
                    QString detail = affectationActuelle(c);
                    if (detail.isEmpty()) detail = proposerRemplacant(c);
                    liste << QString("⚠ %1 : %2 (%3) — %4 % (%5 h / %6 h).\n   %7")
                                 .arg(niveau, nom, c.poste).arg(qRound(taux * 100.0))
                                 .arg(QString::number(c.charge, 'f', 1))
                                 .arg(QString::number(c.dispo, 'f', 1))
                                 .arg(detail);
                }
            }
            if (liste.isEmpty())
                liste << "✅ Aucun employé absent ou proche de sa charge maximale.";
            return liste;
        };

        QFrame *alertFrame = new QFrame;
        alertFrame->setStyleSheet("QFrame { background: #FFF4E5; border: 1px solid #F1CF98; border-radius: 10px; }");
        QVBoxLayout *alertLayout = new QVBoxLayout(alertFrame);
        alertLayout->setContentsMargins(12, 8, 12, 8);
        QLabel *alertTitle = new QLabel(QString("🔔 Alertes au responsable  ·  %1 absence(s)  ·  %2 surcharge(s)")
                                            .arg(nbAbsents).arg(nbSurcharges));
        alertTitle->setStyleSheet("font-weight: bold; color: #8A4B08; background: transparent; border: none;");
        alertLayout->addWidget(alertTitle);
        QPlainTextEdit *alertText = new QPlainTextEdit;
        alertText->setReadOnly(true);
        alertText->setPlainText(construireListeAlertes().join("\n"));
        alertText->setFixedHeight(92);
        alertText->setStyleSheet("QPlainTextEdit { color: #6D4C22; background: transparent; border: none; padding: 0; }");
        alertLayout->addWidget(alertText);
        mainL->addWidget(alertFrame);

        QLabel *titreCourbe = new QLabel("📈  Courbe de charge et disponibilité par employé");
        titreCourbe->setStyleSheet("font-size: 16px; font-weight: bold; color: #225A64;"
                                   "background: transparent;");
        mainL->addWidget(titreCourbe);

        QFrame *graphFrame = new QFrame;
        graphFrame->setStyleSheet("QFrame { background: #FFFFFF; border: 1px solid #D7E9ED;"
                                  "border-radius: 14px; }");
        QVBoxLayout *graphLayout = new QVBoxLayout(graphFrame);
        graphLayout->setContentsMargins(8, 3, 8, 3);
        graphLayout->addWidget(new CourbeChargeWidget(charges, graphFrame));
        mainL->addWidget(graphFrame);

        QFrame *basCard = new QFrame;
        basCard->setStyleSheet("QFrame { background-color: #E4F0F3;"
                               "border-radius: 12px; border: none; }");
        QHBoxLayout *basL = new QHBoxLayout(basCard);
        basL->setContentsMargins(14, 8, 14, 8);

        QLabel *basText = new QLabel(QString(
                                         "⏱  <b>%1 heures</b> de charge au total / semaine   ·   "
                                         "📊  Moyenne : <b>%2 h</b> par employé   ·   "
                                         "🏆  Max : <b>%3 h</b>   ·   "
                                         "📉  Min : <b>%4 h</b>")
                                         .arg(QString::number(totalCharge, 'f', 0))
                                         .arg(QString::number(avgCharge, 'f', 1))
                                         .arg(QString::number(maxCharge, 'f', 0))
                                         .arg(QString::number(minCharge, 'f', 0)));
        basText->setTextFormat(Qt::RichText);
        basText->setStyleSheet("font-size: 11px; color: #225A64;"
                               "background: transparent; border: none;");
        basL->addWidget(basText);
        basL->addStretch();

        mainL->addWidget(basCard);

        // Bouton d'affectation placé en bas de la fenêtre Statistiques.
        QHBoxLayout *actionsStats = new QHBoxLayout;
        QPushButton *btnAffecter = new QPushButton("👥  Affecter un remplaçant");
        btnAffecter->setObjectName("btnPrimary");
        btnAffecter->setMinimumHeight(40);
        btnAffecter->setMinimumWidth(230);
        btnAffecter->setCursor(Qt::PointingHandCursor);
        btnAffecter->setToolTip("Choisir et enregistrer les remplaçants des employés absents ou trop chargés.");
        actionsStats->addWidget(btnAffecter);
        actionsStats->addStretch();
        QPushButton *btnFermerStats = new QPushButton("Fermer");
        btnFermerStats->setObjectName("btnSecondary");
        btnFermerStats->setMinimumHeight(40);
        btnFermerStats->setMinimumWidth(120);
        actionsStats->addWidget(btnFermerStats);
        mainL->addLayout(actionsStats);

        connect(btnFermerStats, &QPushButton::clicked, &dlg, &QDialog::accept);
        connect(btnAffecter, &QPushButton::clicked, &dlg, [&]() {
            if (ciblesAffectation.isEmpty()) {
                QMessageBox::information(&dlg, "Aucune affectation nécessaire",
                                         "Aucun employé n'est absent ou proche de sa charge maximale.");
                return;
            }

            QDialog dialogueAffectation(&dlg);
            dialogueAffectation.setWindowTitle("Affectation des remplaçants");
            dialogueAffectation.resize(820, qMin(520, zoneEcran.height() - 80));
            dialogueAffectation.setMinimumSize(650, 320);
            dialogueAffectation.setStyleSheet("QDialog { background-color: #F0F4F8; }");
            QVBoxLayout *affectL = new QVBoxLayout(&dialogueAffectation);
            affectL->setContentsMargins(18, 16, 18, 14);
            affectL->setSpacing(10);

            QLabel *explication = new QLabel(
                "Sélectionnez un remplaçant disponible du même poste. La proposition par défaut "
                "est le collègue qui a le moins de charge. Les affectations sont sauvegardées "
                "localement : aucune table Oracle n'est ajoutée et les données Oracle ne sont pas modifiées.");
            explication->setWordWrap(true);
            explication->setStyleSheet("color: #315B65; background: transparent;");
            affectL->addWidget(explication);

            QScrollArea *scrollAffect = new QScrollArea(&dialogueAffectation);
            scrollAffect->setWidgetResizable(true);
            scrollAffect->setFrameShape(QFrame::NoFrame);
            scrollAffect->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            scrollAffect->setStyleSheet("QScrollArea { background: transparent; border: none; }");
            QWidget *contenuAffect = new QWidget;
            contenuAffect->setStyleSheet("background: transparent;");
            QVBoxLayout *listeAffectL = new QVBoxLayout(contenuAffect);
            listeAffectL->setContentsMargins(2, 2, 2, 2);
            listeAffectL->setSpacing(8);
            QList<QComboBox*> choixRemplacants;
            choixRemplacants.reserve(ciblesAffectation.size());
            QSet<int> remplacantsDejaProposes;

            for (int row = 0; row < ciblesAffectation.size(); ++row) {
                const Employe::ChargeEmploye cible = ciblesAffectation.at(row);
                const QString nomCible = (cible.prenom + " " + cible.nom).trimmed();

                QFrame *ligne = new QFrame(contenuAffect);
                ligne->setStyleSheet("QFrame { background: white; border: 1px solid #D7E9ED; border-radius: 10px; }");
                QHBoxLayout *ligneL = new QHBoxLayout(ligne);
                ligneL->setContentsMargins(12, 9, 12, 9);
                ligneL->setSpacing(12);

                QVBoxLayout *infosL = new QVBoxLayout;
                infosL->setSpacing(3);
                QLabel *nomLabel = new QLabel(nomCible);
                nomLabel->setStyleSheet("font-weight: bold; color: #225A64; background: transparent; border: none;");
                infosL->addWidget(nomLabel);
                const QString motif = !cible.disponible
                    ? QString("Absent / indisponible · %1").arg(cible.poste)
                    : QString("Charge : %1 % · %2").arg(qRound(tauxCharge(cible) * 100.0)).arg(cible.poste);
                QLabel *motifLabel = new QLabel(motif);
                motifLabel->setStyleSheet("font-size: 11px; color: #788A90; background: transparent; border: none;");
                infosL->addWidget(motifLabel);
                ligneL->addLayout(infosL, 2);

                QComboBox *comboRemplacant = new QComboBox(ligne);
                comboRemplacant->setMinimumHeight(38);
                comboRemplacant->addItem("— Choisir un remplaçant —", 0);
                const QList<Employe::ChargeEmploye> candidats = candidatsRemplacants(cible);
                for (const Employe::ChargeEmploye &candidat : candidats) {
                    comboRemplacant->addItem(
                        QString("%1 %2 · %3%% · %4/%5 h")
                            .arg(candidat.prenom, candidat.nom)
                            .arg(qRound(tauxCharge(candidat) * 100.0))
                            .arg(QString::number(candidat.charge, 'f', 1))
                            .arg(QString::number(candidat.dispo, 'f', 1)),
                        candidat.id);
                }

                const int idDejaAffecte = affectations.value(QString::number(cible.id))
                                              .toObject().value("remplacant_id").toInt();
                int indexSelectionne = comboRemplacant->findData(idDejaAffecte);
                if (indexSelectionne > 0 && !remplacantsDejaProposes.contains(idDejaAffecte)) {
                    comboRemplacant->setCurrentIndex(indexSelectionne);
                    remplacantsDejaProposes.insert(idDejaAffecte);
                } else {
                    // Proposition initiale : le candidat disponible le moins chargé,
                    // en évitant de proposer la même personne pour deux absences.
                    for (int i = 1; i < comboRemplacant->count(); ++i) {
                        const int idCandidat = comboRemplacant->itemData(i).toInt();
                        if (!remplacantsDejaProposes.contains(idCandidat)) {
                            comboRemplacant->setCurrentIndex(i);
                            remplacantsDejaProposes.insert(idCandidat);
                            break;
                        }
                    }
                }
                if (candidats.isEmpty()) {
                    comboRemplacant->clear();
                    comboRemplacant->addItem("Aucun remplaçant admissible", 0);
                    comboRemplacant->setEnabled(false);
                }
                ligneL->addWidget(comboRemplacant, 3);
                listeAffectL->addWidget(ligne);
                choixRemplacants.append(comboRemplacant);
            }
            listeAffectL->addStretch();
            scrollAffect->setWidget(contenuAffect);
            affectL->addWidget(scrollAffect, 1);

            QDialogButtonBox *boutonsAffect = new QDialogButtonBox;
            QPushButton *btnEnregistrerAffect = boutonsAffect->addButton(
                "💾  Enregistrer les affectations", QDialogButtonBox::AcceptRole);
            QPushButton *btnAnnulerAffect = boutonsAffect->addButton(
                "Annuler", QDialogButtonBox::RejectRole);
            btnEnregistrerAffect->setObjectName("btnPrimary");
            btnEnregistrerAffect->setMinimumHeight(40);
            btnAnnulerAffect->setMinimumHeight(40);
            affectL->addWidget(boutonsAffect);

            connect(btnAnnulerAffect, &QPushButton::clicked,
                    &dialogueAffectation, &QDialog::reject);
            connect(btnEnregistrerAffect, &QPushButton::clicked, &dialogueAffectation, [&]() {
                QSet<int> remplacantsChoisis;
                QJsonObject nouvellesAffectations = affectations;
                for (int row = 0; row < ciblesAffectation.size(); ++row) {
                    const int idCible = ciblesAffectation.at(row).id;
                    const int idRemplacant = choixRemplacants.at(row)->currentData().toInt();
                    nouvellesAffectations.remove(QString::number(idCible));
                    if (idRemplacant <= 0)
                        continue;
                    if (remplacantsChoisis.contains(idRemplacant)) {
                        QMessageBox::warning(&dialogueAffectation, "Affectation non valide",
                                             "Un même employé ne peut pas être affecté à plusieurs remplacements en même temps.");
                        return;
                    }
                    remplacantsChoisis.insert(idRemplacant);
                    QJsonObject affectation;
                    affectation.insert("remplacant_id", idRemplacant);
                    affectation.insert("enregistre_le", QDateTime::currentDateTime().toString(Qt::ISODate));
                    nouvellesAffectations.insert(QString::number(idCible), affectation);
                }

                QJsonObject racine;
                racine.insert("version", 1);
                racine.insert("affectations", nouvellesAffectations);
                QSaveFile fichier(cheminFichierAffectations());
                if (!fichier.open(QIODevice::WriteOnly)) {
                    QMessageBox::critical(&dialogueAffectation, "Erreur d'enregistrement",
                                          "Impossible d'enregistrer les affectations : " + fichier.errorString());
                    return;
                }
                const QByteArray contenu = QJsonDocument(racine).toJson(QJsonDocument::Indented);
                if (fichier.write(contenu) != contenu.size() || !fichier.commit()) {
                    const QString erreur = fichier.errorString();
                    fichier.cancelWriting();
                    QMessageBox::critical(&dialogueAffectation, "Erreur d'enregistrement",
                                          "Impossible d'enregistrer les affectations : " + erreur);
                    return;
                }

                affectations = nouvellesAffectations;
                alertText->setPlainText(construireListeAlertes().join("\n"));
                QMessageBox::information(&dialogueAffectation, "Affectations enregistrées",
                                         "Les remplaçants ont été affectés et enregistrés localement.\n"
                                         "Aucune table ni donnée Oracle n'a été modifiée.");
                dialogueAffectation.accept();
            });

            dialogueAffectation.exec();
        });

        dlg.exec();
    });
    layout->addWidget(btnStats);

    return w;
}

// ==================== PERFORMANCE & EVALUATION ====================
// Cette fenetre permet de saisir, conserver localement et modifier l'evaluation
// de chaque employe, puis de produire un PDF ou de preparer un message Gmail.
// Le schema Oracle reste strictement inchange (aucune table supplementaire).
void GestionEmployes::ouvrirEvaluationEmployes()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Performance & Évaluation des employés");
    const QRect zoneEvaluation = QGuiApplication::primaryScreen()
                                     ? QGuiApplication::primaryScreen()->availableGeometry()
                                     : QRect(0, 0, 1280, 800);
    dlg.resize(qMin(1080, zoneEvaluation.width() - 70), qMin(760, zoneEvaluation.height() - 70));
    dlg.setMinimumSize(qMin(900, zoneEvaluation.width() - 30), qMin(620, zoneEvaluation.height() - 30));
    dlg.setStyleSheet("QDialog { background-color: #F0F4F8; }");

    QVBoxLayout *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(24, 22, 24, 20);
    root->setSpacing(12);

    QLabel *titre = new QLabel("⭐  Performance & Évaluation des employés");
    titre->setStyleSheet("font-size: 22px; font-weight: bold; color: #225A64; background: transparent;");
    root->addWidget(titre);

    QLabel *sousTitre = new QLabel(
        "Sélectionnez un employé pour consulter ou remplir son évaluation professionnelle.");
    sousTitre->setWordWrap(true);
    sousTitre->setStyleSheet("font-size: 13px; color: #71858B; background: transparent;");
    root->addWidget(sousTitre);

    QStackedWidget *pages = new QStackedWidget(&dlg);
    root->addWidget(pages, 1);

    // -------------------- PAGE 1 : indicateurs + liste + recherche --------------------
    QWidget *pageListe = new QWidget;
    QVBoxLayout *listeLayout = new QVBoxLayout(pageListe);
    listeLayout->setContentsMargins(0, 4, 0, 4);
    listeLayout->setSpacing(12);

    // Reprendre les quatre indicateurs de la fenetre Statistiques et les garder
    // au-dessus de la recherche, comme dans l'interface precedente.
    const QList<Employe> employes = Employe::afficherTous();
    const QList<Employe::ChargeEmploye> charges = Employe::charges();
    const Employe::StatsCharge stats = Employe::statistiques(charges);

    QHBoxLayout *cardsLayout = new QHBoxLayout;
    cardsLayout->setSpacing(10);
    auto creerCarteIndicateur = [](const QString &icone, const QString &fond,
                                   const QString &couleur, const QString &titreCarte,
                                   const QString &valeur, const QString &sousTitre) {
        QFrame *card = new QFrame;
        card->setMinimumHeight(82);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        card->setStyleSheet(QString("QFrame { background-color: %1; border: none; border-radius: 12px; }").arg(fond));
        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(12, 9, 12, 9);
        cardLayout->setSpacing(9);
        QLabel *iconLabel = new QLabel(icone);
        iconLabel->setStyleSheet("font-size: 24px; background: transparent; border: none;");
        cardLayout->addWidget(iconLabel);
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(1);
        QLabel *titleLabel = new QLabel(titreCarte);
        titleLabel->setStyleSheet(QString("font-size: 10px; color: %1; background: transparent; border: none;").arg(couleur));
        QLabel *valueLabel = new QLabel(valeur);
        valueLabel->setStyleSheet(QString("font-size: 19px; font-weight: bold; color: %1; background: transparent; border: none;").arg(couleur));
        QLabel *subtitleLabel = new QLabel(sousTitre);
        subtitleLabel->setStyleSheet(QString("font-size: 9px; color: %1; background: transparent; border: none;").arg(couleur));
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(valueLabel);
        textLayout->addWidget(subtitleLabel);
        cardLayout->addLayout(textLayout, 1);
        return card;
    };
    cardsLayout->addWidget(creerCarteIndicateur(
        "👥", "#E4F0F3", "#225A64", "Nombre des employés",
        QString::number(employes.size()), "employés enregistrés"));
    cardsLayout->addWidget(creerCarteIndicateur(
        "⏱", "#FDF3E3", "#B8741A", "Charge totale",
        QString::number(stats.somme, 'f', 0) + " h", "par semaine"));
    cardsLayout->addWidget(creerCarteIndicateur(
        "📈", "#E6F4EA", "#2E7D32", "Charge moyenne",
        QString::number(stats.moyenne, 'f', 1) + " h", "par employé"));
    cardsLayout->addWidget(creerCarteIndicateur(
        "⚠️", "#FBE9E7", "#AE5751", "Charge max",
        QString::number(stats.maximum, 'f', 0) + " h", "le plus chargé"));
    listeLayout->addLayout(cardsLayout);

    QFrame *searchFrame = new QFrame;
    searchFrame->setStyleSheet("QFrame { background: white; border: 1px solid #D7E9ED; border-radius: 12px; }");
    QHBoxLayout *searchLayout = new QHBoxLayout(searchFrame);
    searchLayout->setContentsMargins(14, 8, 14, 8);
    QLabel *searchIcon = new QLabel("🔍");
    searchLayout->addWidget(searchIcon);
    QLineEdit *searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("Rechercher un employé par son nom ou prénom...");
    searchEdit->setMinimumHeight(38);
    searchEdit->setStyleSheet("QLineEdit { border: none; background: transparent; color: #244B56; font-size: 13px; }");
    searchLayout->addWidget(searchEdit, 1);
    listeLayout->addWidget(searchFrame);

    QLabel *listeInfo = new QLabel("Cliquez sur un employé pour ouvrir sa fiche d'évaluation.");
    listeInfo->setStyleSheet("font-size: 12px; color: #71858B; background: transparent;");
    listeLayout->addWidget(listeInfo);

    QTableWidget *employeesTable = new QTableWidget;
    employeesTable->setColumnCount(3);
    employeesTable->setHorizontalHeaderLabels({"Employé", "Poste", "Courriel"});
    employeesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    employeesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    employeesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    employeesTable->setAlternatingRowColors(true);
    employeesTable->setShowGrid(false);
    employeesTable->verticalHeader()->setVisible(false);
    employeesTable->verticalHeader()->setDefaultSectionSize(52);
    employeesTable->horizontalHeader()->setMinimumHeight(42);
    employeesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    employeesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    employeesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    employeesTable->setStyleSheet(
        "QTableWidget { background: white; border: 1px solid #D7E9ED; border-radius: 12px; "
        "color: #244B56; selection-background-color: #D9EEF1; selection-color: #174B55; }"
        "QHeaderView::section { background: #2F7F8D; color: white; padding: 11px; border: none; font-weight: bold; }");

    employeesTable->setRowCount(employes.size());
    for (int row = 0; row < employes.size(); ++row) {
        const Employe &e = employes.at(row);
        QTableWidgetItem *nom = new QTableWidgetItem((e.getPrenom() + " " + e.getNom()).trimmed());
        nom->setData(Qt::UserRole, e.getId());
        QFont nomFont = nom->font();
        nomFont.setBold(true);
        nom->setFont(nomFont);
        employeesTable->setItem(row, 0, nom);
        employeesTable->setItem(row, 1, new QTableWidgetItem(e.getPoste()));
        employeesTable->setItem(row, 2, new QTableWidgetItem(e.getCourriel()));
    }
    if (employes.isEmpty()) {
        employeesTable->setRowCount(1);
        QTableWidgetItem *empty = new QTableWidgetItem("Aucun employé enregistré.");
        empty->setTextAlignment(Qt::AlignCenter);
        employeesTable->setItem(0, 0, empty);
        employeesTable->setSpan(0, 0, 1, 3);
        employeesTable->setEnabled(false);
    }
    listeLayout->addWidget(employeesTable, 1);
    pages->addWidget(pageListe);

    // -------------------- PAGE 2 : fiche d'evaluation --------------------
    QWidget *pageFiche = new QWidget;
    QVBoxLayout *ficheLayout = new QVBoxLayout(pageFiche);
    ficheLayout->setContentsMargins(0, 4, 0, 4);
    ficheLayout->setSpacing(10);

    QPushButton *btnRetour = new QPushButton("←  Retour à la liste des employés");
    btnRetour->setObjectName("btnSecondary");
    btnRetour->setCursor(Qt::PointingHandCursor);
    btnRetour->setMinimumHeight(36);
    ficheLayout->addWidget(btnRetour, 0, Qt::AlignLeft);

    QFrame *identiteFrame = new QFrame;
    identiteFrame->setStyleSheet("QFrame { background: white; border: 1px solid #D7E9ED; border-radius: 12px; }");
    QHBoxLayout *identiteLayout = new QHBoxLayout(identiteFrame);
    identiteLayout->setContentsMargins(16, 12, 16, 12);
    QLabel *avatar = new QLabel("👤");
    avatar->setStyleSheet("font-size: 30px; background: transparent; border: none;");
    identiteLayout->addWidget(avatar);
    QVBoxLayout *identiteText = new QVBoxLayout;
    QLabel *employeeName = new QLabel;
    employeeName->setStyleSheet("font-size: 18px; font-weight: bold; color: #225A64; background: transparent; border: none;");
    QLabel *employeeInfo = new QLabel;
    employeeInfo->setStyleSheet("font-size: 12px; color: #71858B; background: transparent; border: none;");
    identiteText->addWidget(employeeName);
    identiteText->addWidget(employeeInfo);
    identiteLayout->addLayout(identiteText, 1);
    QLabel *globalScore = new QLabel("Note globale : -- / 5");
    globalScore->setAlignment(Qt::AlignCenter);
    globalScore->setMinimumWidth(160);
    globalScore->setStyleSheet("QLabel { background: #FFF5D9; color: #9A6A12; border-radius: 10px; padding: 10px; font-size: 14px; font-weight: bold; }");
    identiteLayout->addWidget(globalScore);
    ficheLayout->addWidget(identiteFrame);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    QWidget *formContent = new QWidget;
    QVBoxLayout *formLayout = new QVBoxLayout(formContent);
    formLayout->setContentsMargins(0, 0, 6, 0);
    formLayout->setSpacing(12);

    QFrame *criteriaFrame = new QFrame;
    criteriaFrame->setStyleSheet("QFrame { background: white; border: 1px solid #D7E9ED; border-radius: 12px; }");
    QVBoxLayout *criteriaLayout = new QVBoxLayout(criteriaFrame);
    criteriaLayout->setContentsMargins(16, 14, 16, 14);
    QLabel *criteriaTitle = new QLabel("⭐  Critères d'évaluation");
    criteriaTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #225A64; background: transparent; border: none;");
    criteriaLayout->addWidget(criteriaTitle);
    QLabel *criteriaHelp = new QLabel(
        "Le responsable attribue une note de 1 à 5 pour chaque critère : 1 = Insuffisant, 5 = Excellent.");
    criteriaHelp->setWordWrap(true);
    criteriaHelp->setStyleSheet("font-size: 11px; color: #71858B; background: transparent; border: none;");
    criteriaLayout->addWidget(criteriaHelp);

    const QStringList criteriaNames = {
        "Discipline & ponctualité",
        "Comportement & communication",
        "Compétences professionnelles",
        "Qualité du travail",
        "Hygiène & sécurité",
        "Responsabilité & travail en équipe"
    };
    const QStringList tooltips = {
        "Respect des horaires, ponctualité et présence.",
        "Respect des collègues, des clients et qualité de communication.",
        "Compétences techniques liées au poste occupé.",
        "Précision, sérieux et qualité des tâches réalisées.",
        "Respect des règles d'hygiène, de sécurité et des protocoles.",
        "Autonomie, respect des responsabilités et coopération avec l'équipe."
    };
    QList<QList<QToolButton*>> ratingStarButtons;
    QList<QLabel*> ratingValueLabels;
    QList<QLabel*> criterionHints;
    QList<int> ratingValues;
    const QStringList ratingNames = {"", "Insuffisant", "À améliorer", "Satisfaisant", "Très bien", "Excellent"};
    for (int i = 0; i < criteriaNames.size(); ++i) {
        QFrame *line = new QFrame;
        line->setStyleSheet("QFrame { background: #F7FAFB; border: none; border-radius: 8px; }");
        QHBoxLayout *lineLayout = new QHBoxLayout(line);
        lineLayout->setContentsMargins(10, 7, 10, 7);
        QVBoxLayout *nameLayout = new QVBoxLayout;
        QLabel *criterion = new QLabel(criteriaNames.at(i));
        criterion->setStyleSheet("font-size: 13px; font-weight: 600; color: #315B65; background: transparent;");
        QLabel *hint = new QLabel(tooltips.at(i));
        hint->setWordWrap(true);
        criterionHints.append(hint);
        hint->setStyleSheet("font-size: 10px; color: #829297; background: transparent;");
        nameLayout->addWidget(criterion);
        nameLayout->addWidget(hint);
        lineLayout->addLayout(nameLayout, 1);

        QHBoxLayout *starsLayout = new QHBoxLayout;
        starsLayout->setSpacing(1);
        QList<QToolButton*> rowStars;
        for (int star = 1; star <= 5; ++star) {
            QToolButton *starButton = new QToolButton;
            starButton->setText("★");
            starButton->setToolTip(QString::number(star) + "/5");
            starButton->setCursor(Qt::PointingHandCursor);
            starButton->setFixedSize(29, 32);
            starButton->setStyleSheet("QToolButton { color: #B8C4C8; font-size: 23px; border: none; background: transparent; padding: 0; }");
            starsLayout->addWidget(starButton);
            rowStars.append(starButton);
        }
        QWidget *starsWidget = new QWidget;
        starsWidget->setStyleSheet("background: transparent;");
        starsWidget->setLayout(starsLayout);
        lineLayout->addWidget(starsWidget, 0, Qt::AlignVCenter);

        QLabel *valueLabel = new QLabel("Non noté");
        valueLabel->setMinimumWidth(105);
        valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        valueLabel->setStyleSheet("font-size: 10px; color: #829297; background: transparent;");
        lineLayout->addWidget(valueLabel);
        criteriaLayout->addWidget(line);
        ratingStarButtons.append(rowStars);
        ratingValueLabels.append(valueLabel);
        ratingValues.append(0);
    }
    auto actualiserEtoiles = [&](int criterionIndex, int value) {
        if (criterionIndex < 0 || criterionIndex >= ratingStarButtons.size()) return;
        ratingValues[criterionIndex] = value;
        for (int j = 0; j < ratingStarButtons.at(criterionIndex).size(); ++j) {
            const bool selected = (j + 1) <= value;
            ratingStarButtons[criterionIndex].at(j)->setStyleSheet(
                selected
                    ? "QToolButton { color: #E6A52E; font-size: 23px; border: none; background: transparent; padding: 0; }"
                    : "QToolButton { color: #B8C4C8; font-size: 23px; border: none; background: transparent; padding: 0; }");
        }
        ratingValueLabels.at(criterionIndex)->setText(value > 0
            ? QString("%1/5 · %2").arg(value).arg(ratingNames.at(value))
            : "Non noté");
        ratingValueLabels.at(criterionIndex)->setStyleSheet(value > 0
            ? "font-size: 10px; color: #9A6A12; font-weight: bold; background: transparent;"
            : "font-size: 10px; color: #829297; background: transparent;");
    };
    formLayout->addWidget(criteriaFrame);

    QFrame *commentsFrame = new QFrame;
    commentsFrame->setStyleSheet("QFrame { background: white; border: 1px solid #D7E9ED; border-radius: 12px; }");
    QVBoxLayout *commentsLayout = new QVBoxLayout(commentsFrame);
    commentsLayout->setContentsMargins(16, 14, 16, 14);
    QLabel *commentsTitle = new QLabel("📝  Avis du responsable & plan d'amélioration");
    commentsTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #225A64; background: transparent; border: none;");
    commentsLayout->addWidget(commentsTitle);

    auto addTextField = [&](const QString &label, const QString &placeholder, int height) {
        QLabel *fieldLabel = new QLabel(label);
        fieldLabel->setStyleSheet("font-size: 12px; font-weight: 600; color: #315B65; background: transparent; border: none;");
        QTextEdit *field = new QTextEdit;
        field->setPlaceholderText(placeholder);
        field->setFixedHeight(height);
        field->setStyleSheet("QTextEdit { background: #F7FAFB; border: 1px solid #DCE9EC; border-radius: 8px; padding: 7px; color: #244B56; }");
        commentsLayout->addWidget(fieldLabel);
        commentsLayout->addWidget(field);
        return field;
    };
    QTextEdit *pointsForts = addTextField("Points forts", "Ex. : ponctuel, sérieux, bonne prise en charge des animaux...", 68);
    QTextEdit *axesAmelioration = addTextField("Axes d'amélioration", "Ex. : améliorer le suivi des dossiers ou la communication...", 68);
    QTextEdit *commentaires = addTextField("Commentaire général du responsable", "Écrire ici l'avis et les observations concernant cet employé...", 82);
    QTextEdit *objectifs = addTextField("Objectifs / recommandations", "Ex. : suivre une formation, améliorer une compétence, respecter un objectif...", 68);
    commentsLayout->addWidget(new QLabel("Les notes et commentaires sont sauvegardés automatiquement et peuvent être modifiés à tout moment."));
    formLayout->addWidget(commentsFrame);

    QFrame *metaFrame = new QFrame;
    metaFrame->setStyleSheet("QFrame { background: white; border: 1px solid #D7E9ED; border-radius: 12px; }");
    QFormLayout *metaLayout = new QFormLayout(metaFrame);
    metaLayout->setContentsMargins(16, 12, 16, 12);
    QDateEdit *dateEvaluation = new QDateEdit(QDate::currentDate());
    dateEvaluation->setCalendarPopup(true);
    dateEvaluation->setDisplayFormat("dd/MM/yyyy");
    dateEvaluation->setMinimumHeight(34);
    QLineEdit *evaluateur = new QLineEdit;
    evaluateur->setPlaceholderText("Nom du responsable / évaluateur");
    evaluateur->setMinimumHeight(34);
    metaLayout->addRow("Date de l'évaluation", dateEvaluation);
    metaLayout->addRow("Évaluateur", evaluateur);
    formLayout->addWidget(metaFrame);

    QLabel *reviewStatus = new QLabel("Sélectionnez les critères et saisissez les observations du responsable.");
    reviewStatus->setWordWrap(true);
    reviewStatus->setStyleSheet("QLabel { background: #E9F3F5; color: #315B65; border-radius: 8px; padding: 9px; }");
    formLayout->addWidget(reviewStatus);

    scroll->setWidget(formContent);
    ficheLayout->addWidget(scroll, 1);
    pages->addWidget(pageFiche);

    // -------------------- Barre d'actions --------------------
    QHBoxLayout *actionsLayout = new QHBoxLayout;
    actionsLayout->setSpacing(10);
    root->addLayout(actionsLayout);

    QPushButton *btnSave = new QPushButton("💾  Enregistrer");
    btnSave->setObjectName("btnPrimary");
    btnSave->setMinimumHeight(42);
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setVisible(false);
    actionsLayout->addWidget(btnSave);

    QPushButton *btnPdf = new QPushButton("📄  Exporter PDF");
    btnPdf->setObjectName("btnSecondary");
    btnPdf->setMinimumHeight(42);
    btnPdf->setCursor(Qt::PointingHandCursor);
    btnPdf->setVisible(false);
    actionsLayout->addWidget(btnPdf);

    QPushButton *btnGmail = new QPushButton("✉️  Envoyer par Gmail");
    btnGmail->setObjectName("btnPrimary");
    btnGmail->setMinimumHeight(42);
    btnGmail->setCursor(Qt::PointingHandCursor);
    btnGmail->setVisible(false);
    actionsLayout->addWidget(btnGmail);

    actionsLayout->addStretch();
    QPushButton *btnFermer = new QPushButton("Fermer");
    btnFermer->setObjectName("btnSecondary");
    btnFermer->setMinimumHeight(42);
    btnFermer->setMinimumWidth(125);
    btnFermer->setCursor(Qt::PointingHandCursor);
    actionsLayout->addWidget(btnFermer);

    Employe employeSelectionne;
    bool hasEmployee = false;
    bool loadingReview = false;
    bool reviewModified = false;

    // Les evaluations restent dans un fichier local de l'application :
    // on ne cree aucune table/colonne supplementaire dans Oracle.
    auto cheminFichierEvaluations = []() {
        QString dossier = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (dossier.isEmpty())
            dossier = QDir::homePath() + QLatin1String("/.aniworld");
        QDir().mkpath(dossier);
        return dossier + QLatin1String("/evaluations_employes.json");
    };

    auto lireEvaluations = [&](QJsonObject &racine, QString *erreur) {
        const QString chemin = cheminFichierEvaluations();
        QFile fichier(chemin);
        if (!fichier.exists()) {
            racine = QJsonObject();
            return true;
        }
        if (!fichier.open(QIODevice::ReadOnly)) {
            if (erreur) *erreur = fichier.errorString();
            return false;
        }
        const QByteArray contenu = fichier.readAll();
        fichier.close();
        QJsonParseError parseErreur;
        const QJsonDocument document = QJsonDocument::fromJson(contenu, &parseErreur);
        if (parseErreur.error != QJsonParseError::NoError || !document.isObject()) {
            if (erreur) *erreur = QString("Fichier d'évaluations illisible : %1").arg(parseErreur.errorString());
            return false;
        }
        racine = document.object();
        return true;
    };

    auto calculerNote = [&]() -> double {
        int total = 0;
        int count = 0;
        for (int value : ratingValues) {
            if (value > 0) { total += value; ++count; }
        }
        if (count == 0) return 0.0;
        return static_cast<double>(total) / count;
    };
    auto actualiserNote = [&]() {
        int count = 0;
        for (int value : ratingValues)
            if (value > 0) ++count;
        if (count == ratingValues.size())
            globalScore->setText(QString("Note globale : %1 / 5").arg(calculerNote(), 0, 'f', 2));
        else
            globalScore->setText(QString("%1 / 6 critères notés").arg(count));
    };
    auto rendreActionsDisponibles = [&]() {
        btnSave->setEnabled(hasEmployee);
        btnPdf->setEnabled(hasEmployee);
        btnGmail->setEnabled(hasEmployee);
        if (employeSelectionne.getCourriel().trimmed().isEmpty())
            btnGmail->setToolTip("Aucune adresse courriel n'est enregistrée : cliquez pour voir le message d'avertissement.");
        else
            btnGmail->setToolTip("Ouvre Gmail avec le destinataire et l'évaluation affichée préremplis.");
    };

    auto sauvegarderEvaluation = [&](bool afficherConfirmation) -> bool {
        if (!hasEmployee || employeSelectionne.getId() <= 0)
            return false;

        QJsonObject racine;
        QString erreurLecture;
        if (!lireEvaluations(racine, &erreurLecture)) {
            reviewStatus->setText("❌ Impossible de lire les évaluations : " + erreurLecture);
            if (afficherConfirmation)
                QMessageBox::critical(&dlg, "Erreur d'enregistrement", erreurLecture);
            return false;
        }

        QJsonObject toutes = racine.value("evaluations").toObject();
        QJsonObject fiche;
        QJsonArray notes;
        for (int valeur : ratingValues)
            notes.append(valeur);
        fiche.insert("notes", notes);
        fiche.insert("pointsForts", pointsForts->toPlainText());
        fiche.insert("axesAmelioration", axesAmelioration->toPlainText());
        fiche.insert("commentaires", commentaires->toPlainText());
        fiche.insert("objectifs", objectifs->toPlainText());
        fiche.insert("dateEvaluation", dateEvaluation->date().toString(Qt::ISODate));
        fiche.insert("evaluateur", evaluateur->text().trimmed());
        fiche.insert("noteGlobale", calculerNote());
        fiche.insert("updatedAt", QDateTime::currentDateTime().toString(Qt::ISODate));

        // Une seule fiche par ID_EMP : chaque sauvegarde remplace la précédente.
        toutes.insert(QString::number(employeSelectionne.getId()), fiche);
        racine.insert("version", 1);
        racine.insert("evaluations", toutes);

        QSaveFile fichier(cheminFichierEvaluations());
        if (!fichier.open(QIODevice::WriteOnly)) {
            const QString erreur = fichier.errorString();
            reviewStatus->setText("❌ Enregistrement impossible : " + erreur);
            if (afficherConfirmation)
                QMessageBox::critical(&dlg, "Erreur d'enregistrement", erreur);
            return false;
        }
        const QByteArray donnees = QJsonDocument(racine).toJson(QJsonDocument::Indented);
        if (fichier.write(donnees) != donnees.size() || !fichier.commit()) {
            const QString erreur = fichier.errorString();
            reviewStatus->setText("❌ Enregistrement impossible : " + erreur);
            if (afficherConfirmation)
                QMessageBox::critical(&dlg, "Erreur d'enregistrement", erreur);
            return false;
        }

        reviewModified = false;
        reviewStatus->setText("✅ Évaluation enregistrée. Les changements suivants remplaceront cette version.");
        if (afficherConfirmation)
            QMessageBox::information(&dlg, "Évaluation enregistrée",
                                     "L'évaluation de " + employeSelectionne.getPrenom() + " " +
                                     employeSelectionne.getNom() + " a été sauvegardée. Vous la retrouverez en rouvrant sa fiche.");
        return true;
    };

    auto marquerModifie = [&]() {
        if (loadingReview || !hasEmployee) return;
        reviewModified = true;
        actualiserNote();
        // Sauvegarde automatique à chaque changement, en plus du bouton.
        // Ainsi, fermer la fenêtre ou revenir à la liste ne fait pas perdre la note.
        sauvegarderEvaluation(false);
    };
    for (int i = 0; i < ratingStarButtons.size(); ++i) {
        for (int j = 0; j < ratingStarButtons.at(i).size(); ++j) {
            connect(ratingStarButtons.at(i).at(j), &QToolButton::clicked, &dlg, [&, i, score = j + 1]() {
                actualiserEtoiles(i, score);
                marquerModifie();
            });
        }
    }
    connect(pointsForts, &QTextEdit::textChanged, &dlg, marquerModifie);
    connect(axesAmelioration, &QTextEdit::textChanged, &dlg, marquerModifie);
    connect(commentaires, &QTextEdit::textChanged, &dlg, marquerModifie);
    connect(objectifs, &QTextEdit::textChanged, &dlg, marquerModifie);
    connect(dateEvaluation, &QDateEdit::dateChanged, &dlg, marquerModifie);
    connect(evaluateur, &QLineEdit::textChanged, &dlg, marquerModifie);

    auto construireTexteReview = [&]() {
        QString text;
        text += "ÉVALUATION PROFESSIONNELLE\n";
        text += "Employé : " + (employeSelectionne.getPrenom() + " " + employeSelectionne.getNom()).trimmed() + "\n";
        text += "Poste : " + employeSelectionne.getPoste() + "\n";
        text += "Date : " + dateEvaluation->date().toString("dd/MM/yyyy") + "\n";
        text += "Évaluateur : " + evaluateur->text().trimmed() + "\n\n";
        for (int i = 0; i < ratingValues.size(); ++i) {
            const int score = ratingValues.at(i);
            text += criteriaNames.at(i) + " : " + (score > 0 ? QString::number(score) + "/5" : "Non évalué") + "\n";
        }
        text += "\nNote globale : " + QString::number(calculerNote(), 'f', 2) + "/5\n";
        text += "\nPoints forts :\n" + pointsForts->toPlainText().trimmed() + "\n";
        text += "\nAxes d'amélioration :\n" + axesAmelioration->toPlainText().trimmed() + "\n";
        text += "\nCommentaire du responsable :\n" + commentaires->toPlainText().trimmed() + "\n";
        text += "\nObjectifs / recommandations :\n" + objectifs->toPlainText().trimmed() + "\n";
        return text;
    };

    auto construireHtmlReview = [&]() {
        QString html = "<html><head><meta charset='utf-8'><style>"
                       "body{font-family:Arial,sans-serif;color:#244B56;} h1{color:#225A64;} "
                       ".meta{color:#607D86;} table{border-collapse:collapse;width:100%;} "
                       "td,th{border:1px solid #D7E9ED;padding:9px;text-align:left;} "
                       "th{background:#2F7F8D;color:white;} .box{background:#F3F8F9;padding:10px;margin:10px 0;}"
                       "</style></head><body>";
        html += "<h1>Évaluation professionnelle</h1>";
        html += "<p class='meta'><b>Employé :</b> " + (employeSelectionne.getPrenom() + " " + employeSelectionne.getNom()).trimmed().toHtmlEscaped() +
                "<br><b>Poste :</b> " + employeSelectionne.getPoste().toHtmlEscaped() +
                "<br><b>Date :</b> " + dateEvaluation->date().toString("dd/MM/yyyy") +
                "<br><b>Évaluateur :</b> " + evaluateur->text().toHtmlEscaped() + "</p>";
        html += "<table><tr><th>Critère</th><th>Note</th></tr>";
        for (int i = 0; i < ratingValues.size(); ++i) {
            const int score = ratingValues.at(i);
            html += "<tr><td>" + criteriaNames.at(i).toHtmlEscaped() + "</td><td>" +
                    (score > 0 ? QString::number(score) + "/5" : "Non évalué") + "</td></tr>";
        }
        html += "</table><p><b>Note globale :</b> " + QString::number(calculerNote(), 'f', 2) + "/5</p>";
        auto bloc = [&](const QString &label, const QString &value) {
            return "<div class='box'><b>" + label.toHtmlEscaped() + "</b><p>" +
                   value.toHtmlEscaped().replace("\n", "<br>") + "</p></div>";
        };
        html += bloc("Points forts", pointsForts->toPlainText());
        html += bloc("Axes d'amélioration", axesAmelioration->toPlainText());
        html += bloc("Commentaire du responsable", commentaires->toPlainText());
        html += bloc("Objectifs / recommandations", objectifs->toPlainText());
        html += "</body></html>";
        return html;
    };

    auto reinitialiserFiche = [&]() {
        loadingReview = true;
        for (int i = 0; i < ratingValues.size(); ++i) actualiserEtoiles(i, 0);
        pointsForts->clear();
        axesAmelioration->clear();
        commentaires->clear();
        objectifs->clear();
        evaluateur->clear();
        dateEvaluation->setDate(QDate::currentDate());
        reviewModified = false;

        QJsonObject racine;
        QString erreurLecture;
        if (!lireEvaluations(racine, &erreurLecture)) {
            reviewStatus->setText("⚠ Impossible de charger l'évaluation sauvegardée : " + erreurLecture);
        } else {
            const QJsonObject toutes = racine.value("evaluations").toObject();
            const QJsonObject fiche = toutes.value(QString::number(employeSelectionne.getId())).toObject();
            if (!fiche.isEmpty()) {
                const QJsonArray notes = fiche.value("notes").toArray();
                for (int i = 0; i < ratingValues.size(); ++i) {
                    const int valeur = i < notes.size() ? qBound(0, notes.at(i).toInt(), 5) : 0;
                    actualiserEtoiles(i, valeur);
                }
                pointsForts->setPlainText(fiche.value("pointsForts").toString());
                axesAmelioration->setPlainText(fiche.value("axesAmelioration").toString());
                commentaires->setPlainText(fiche.value("commentaires").toString());
                objectifs->setPlainText(fiche.value("objectifs").toString());
                const QDate dateStockee = QDate::fromString(fiche.value("dateEvaluation").toString(), Qt::ISODate);
                if (dateStockee.isValid()) dateEvaluation->setDate(dateStockee);
                evaluateur->setText(fiche.value("evaluateur").toString());
                reviewStatus->setText("✅ Évaluation précédente chargée. Toute modification remplacera automatiquement cette version.");
            } else {
                reviewStatus->setText("Nouvelle fiche : saisissez les notes et commentaires. Les changements seront enregistrés automatiquement.");
            }
        }
        loadingReview = false;
        actualiserNote();
        rendreActionsDisponibles();
    };

    auto choisirEmploye = [&](int row) {
        if (row < 0 || !employeesTable->item(row, 0)) return;
        const int id = employeesTable->item(row, 0)->data(Qt::UserRole).toInt();
        if (id <= 0) return;
        if (!Employe::charger(id, employeSelectionne)) {
            QMessageBox::warning(&dlg, "Employé introuvable", "Impossible de charger les informations de cet employé.");
            return;
        }
        hasEmployee = true;
        employeeName->setText((employeSelectionne.getPrenom() + " " + employeSelectionne.getNom()).trimmed());
        employeeInfo->setText(employeSelectionne.getPoste() + "  ·  " + employeSelectionne.getCourriel());
        const QString poste = employeSelectionne.getPoste().toLower();
        QString competenceHint = "Compétences techniques et savoir-faire nécessaires pour le poste occupé.";
        if (poste.contains("vétérinaire") || poste.contains("veterinaire"))
            competenceHint = "Consultation, diagnostic, soins, suivi des dossiers médicaux et respect des protocoles vétérinaires.";
        else if (poste.contains("assistant") || poste.contains("auxiliaire"))
            competenceHint = "Préparation du matériel, aide aux soins, manipulation sécurisée des animaux et assistance au vétérinaire.";
        else if (poste.contains("réception") || poste.contains("reception") || poste.contains("secrétaire") || poste.contains("secretaire"))
            competenceHint = "Accueil des clients, gestion des rendez-vous et dossiers, communication et confidentialité.";
        else if (poste.contains("entretien"))
            competenceHint = "Nettoyage, désinfection, organisation des espaces et respect des protocoles d'hygiène.";
        criterionHints.at(2)->setText(competenceHint);
        reinitialiserFiche();
        pages->setCurrentWidget(pageFiche);
        btnSave->setVisible(true);
        btnPdf->setVisible(true);
        btnGmail->setVisible(true);
        rendreActionsDisponibles();
    };

    connect(searchEdit, &QLineEdit::textChanged, &dlg, [employeesTable](const QString &text) {
        const QString needle = text.trimmed();
        for (int row = 0; row < employeesTable->rowCount(); ++row) {
            const QString name = employeesTable->item(row, 0) ? employeesTable->item(row, 0)->text() : QString();
            employeesTable->setRowHidden(row, !name.contains(needle, Qt::CaseInsensitive));
        }
    });
    connect(employeesTable, &QTableWidget::cellClicked, &dlg,
            [&](int row, int) { choisirEmploye(row); });
    connect(btnRetour, &QPushButton::clicked, &dlg, [&]() {
        pages->setCurrentWidget(pageListe);
        btnSave->setVisible(false);
        btnPdf->setVisible(false);
        btnGmail->setVisible(false);
        btnPdf->setEnabled(false);
        btnGmail->setEnabled(false);
        employeesTable->clearSelection();
        searchEdit->setFocus();
    });

    connect(btnSave, &QPushButton::clicked, &dlg, [&]() {
        sauvegarderEvaluation(true);
    });

    connect(btnPdf, &QPushButton::clicked, &dlg, [&]() {
        if (!hasEmployee) return;
        if (reviewModified && !sauvegarderEvaluation(false)) return;
        QString defaultName = "Evaluation_" + employeSelectionne.getPrenom() + "_" + employeSelectionne.getNom() + ".pdf";
        defaultName.replace(QRegularExpression("[^A-Za-z0-9_.-]"), "_");
        const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + defaultName;
        QString path = QFileDialog::getSaveFileName(&dlg, "Exporter l'évaluation en PDF", defaultPath, "Fichier PDF (*.pdf)");
        if (path.isEmpty()) return;
        if (!path.endsWith(".pdf", Qt::CaseInsensitive)) path += ".pdf";
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(path);
        QTextDocument document;
        document.setHtml(construireHtmlReview());
        document.print(&printer);
        const QFileInfo pdfInfo(path);
        if (!pdfInfo.exists() || pdfInfo.size() <= 0) {
            QMessageBox::critical(&dlg, "Erreur PDF",
                                  "Le fichier PDF n'a pas pu être créé. Vérifiez les droits d'accès au dossier choisi.");
            return;
        }
        QMessageBox::information(&dlg, "PDF créé",
                                 "Le rapport d'évaluation a été exporté :\n" + path);
    });

    connect(btnGmail, &QPushButton::clicked, &dlg, [&]() {
        if (!hasEmployee) return;
        if (reviewModified && !sauvegarderEvaluation(false)) return;
        const QString destinataire = employeSelectionne.getCourriel().trimmed();
        if (destinataire.isEmpty()) {
            QMessageBox::warning(&dlg, "Adresse manquante", "Aucune adresse courriel n'est enregistrée pour cet employé.");
            return;
        }
        QUrl url("https://mail.google.com/mail/");
        QUrlQuery query;
        query.addQueryItem("view", "cm");
        query.addQueryItem("fs", "1");
        query.addQueryItem("to", destinataire);
        query.addQueryItem("su", "Évaluation professionnelle - " +
                           (employeSelectionne.getPrenom() + " " + employeSelectionne.getNom()).trimmed());
        query.addQueryItem("body", "Bonjour " + employeSelectionne.getPrenom() + ",\n\n" +
                           construireTexteReview() +
                           "\n\nCordialement,\nResponsable du centre\n\n" +
                           "Remarque : vous pouvez joindre le PDF exporté depuis ANIWORLD avant d'envoyer ce message.");
        url.setQuery(query);
        if (!QDesktopServices::openUrl(url)) {
            QMessageBox::warning(&dlg, "Gmail", "Impossible d'ouvrir Gmail dans le navigateur. Vérifiez la connexion Internet et l'application associée aux liens web.");
            return;
        }
        QMessageBox::information(&dlg, "Gmail ouvert",
                                 "Gmail a été ouvert avec le destinataire, le sujet et le contenu de l'évaluation préremplis.\n\nVérifiez le message puis cliquez sur Envoyer dans Gmail. Pour joindre le PDF, exportez-le avec le bouton PDF puis ajoutez-le au message.");
    });

    connect(btnFermer, &QPushButton::clicked, &dlg, &QDialog::accept);
    dlg.exec();
}

// ==================== BARRE RECHERCHE ====================
QWidget *GestionEmployes::construireBarreRecherche()
{
    QWidget *w = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    m_editRecherche = new QLineEdit;
    m_editRecherche->setObjectName("searchField");
    m_editRecherche->setPlaceholderText("🔍  Rechercher par nom, ID ou poste...");
    m_editRecherche->setMinimumHeight(46);
    connect(m_editRecherche, &QLineEdit::textChanged,
            this, &GestionEmployes::onFiltresChange);
    layout->addWidget(m_editRecherche, 3);

    m_comboPoste = new QComboBox;
    m_comboPoste->setMinimumHeight(46);
    m_comboPoste->addItems({"Tous les postes", "Vétérinaire",
                            "Assistant(e) vétérinaire", "Auxiliaire vétérinaire",
                            "Secrétaire / Réceptionniste", "Agent d'entretien",
                            "Aide aux soins animaliers"});
    connect(m_comboPoste, &QComboBox::currentTextChanged,
            this, &GestionEmployes::onFiltresChange);
    layout->addWidget(m_comboPoste, 1);

    m_comboStatut = new QComboBox;
    m_comboStatut->setMinimumHeight(46);
    m_comboStatut->addItems({"Tous", "Disponible", "Non disponible"});
    connect(m_comboStatut, &QComboBox::currentTextChanged,
            this, &GestionEmployes::onFiltresChange);
    layout->addWidget(m_comboStatut, 1);

    m_comboTri = new QComboBox;
    m_comboTri->setMinimumHeight(46);
    m_comboTri->addItems({"Tri par date", "Tri par charge"});
    connect(m_comboTri, &QComboBox::currentTextChanged,
            this, &GestionEmployes::onFiltresChange);
    layout->addWidget(m_comboTri, 1);

    return w;
}

// ==================== GRID ====================
QWidget *GestionEmployes::construireGrid()
{
    m_cardsContainer = new QWidget;
    m_gridCards = new QGridLayout(m_cardsContainer);
    m_gridCards->setContentsMargins(0, 0, 0, 0);
    m_gridCards->setSpacing(18);
    m_gridCards->setAlignment(Qt::AlignTop);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(m_cardsContainer);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }"
                          "QScrollArea > QWidget > QWidget { background: transparent; }");
    return scroll;
}

// ==================== CARD ====================
QWidget *GestionEmployes::creerCard(const Employe &e)
{
    QFrame *card = new QFrame;
    bool selectionne = (e.getId() == m_idSelectionne);
    card->setObjectName(selectionne ? "cardEmployeSelected" : "cardEmploye");
    card->setCursor(Qt::PointingHandCursor);
    card->setMinimumHeight(170);
    card->setProperty("employe_id", e.getId());
    card->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(10);

    QHBoxLayout *topRow = new QHBoxLayout;
    topRow->setSpacing(6);
    topRow->addStretch();

    QLabel *badge = new QLabel(e.isDisponible() ? "✅ Disponible" : "❌ Indisponible");
    if (e.isDisponible())
        badge->setStyleSheet("font-size: 10px; color: #2E7D32; background-color: #E6F4EA;"
                             "padding: 5px 12px; border-radius: 12px; font-weight: bold;"
                             "border: none;");
    else
        badge->setStyleSheet("font-size: 10px; color: #AE5751; background-color: #FBE9E7;"
                             "padding: 5px 12px; border-radius: 12px; font-weight: bold;"
                             "border: none;");
    topRow->addWidget(badge);

    layout->addLayout(topRow);

    QLabel *nom = new QLabel(e.getPrenom() + " " + e.getNom());
    nom->setStyleSheet("font-size: 18px; font-weight: bold; color: #225A64;"
                       "background: transparent;");
    layout->addWidget(nom);

    QLabel *poste = new QLabel(e.getPoste());
    poste->setStyleSheet("font-size: 12px; color: #7A8B8F; background: transparent;");
    layout->addWidget(poste);

    layout->addStretch();

    QHBoxLayout *botRow = new QHBoxLayout;
    botRow->setSpacing(10);

    QLabel *charge = new QLabel(QString("⏱ %1 h").arg(e.getCharge(), 0, 'f', 0));
    charge->setStyleSheet("font-size: 11px; color: #263B42; font-weight: 500;"
                          "background: transparent;");
    botRow->addWidget(charge);

    botRow->addStretch();

    if (!e.getCompetences().isEmpty()) {
        QString comp = e.getCompetences();
        if (comp.length() > 22) comp = comp.left(22) + "...";
        QLabel *compLbl = new QLabel("🏷 " + comp);
        compLbl->setStyleSheet("font-size: 10px; color: #7A8B8F; background: transparent;");
        botRow->addWidget(compLbl);
    }

    layout->addLayout(botRow);

    return card;
}

// ==================== DETAIL PANEL ====================
QWidget *GestionEmployes::construireDetailPanel()
{
    QFrame *card = new QFrame;
    card->setObjectName("card");
    card->setMinimumWidth(360);
    card->setMaximumWidth(420);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QLabel *titre = new QLabel("EMPLOYÉ SÉLECTIONNÉ");
    titre->setStyleSheet("font-size: 11px; font-weight: bold; color: #7A8B8F;"
                         "letter-spacing: 1px; padding: 22px 25px 8px 25px;"
                         "background: transparent;");
    layout->addWidget(titre);

    m_detailPlaceholder = new QLabel("👈\n\nSélectionnez un employé\npour voir ses détails");
    m_detailPlaceholder->setAlignment(Qt::AlignCenter);
    m_detailPlaceholder->setStyleSheet("font-size: 13px; color: #7A8B8F; padding: 80px 20px;"
                                       "background: transparent;");
    layout->addWidget(m_detailPlaceholder);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    m_detailContent = new QWidget;
    m_detailContent->setObjectName("detailContent");
    m_detailContent->setStyleSheet("QWidget#detailContent { background: transparent; }");

    QVBoxLayout *contentL = new QVBoxLayout(m_detailContent);
    contentL->setContentsMargins(25, 5, 25, 25);
    contentL->setSpacing(10);

    m_detailNom = new QLabel;
    m_detailNom->setStyleSheet("font-size: 22px; font-weight: bold; color: #225A64;"
                               "background: transparent;");
    m_detailNom->setWordWrap(true);
    contentL->addWidget(m_detailNom);

    m_detailId = new QLabel;
    m_detailId->setStyleSheet("font-size: 12px; color: #7A8B8F; background: transparent;");
    contentL->addWidget(m_detailId);

    m_detailStatut = new QLabel;
    m_detailStatut->setAlignment(Qt::AlignCenter);
    contentL->addWidget(m_detailStatut);

    contentL->addSpacing(8);

    QFrame *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color: #E4F0F3; max-height: 1px; border: none;");
    contentL->addWidget(sep);

    contentL->addSpacing(6);

    auto creerRow = [](const QString &label, QLabel *&valeur) {
        QHBoxLayout *row = new QHBoxLayout;
        row->setSpacing(8);
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 12px; color: #7A8B8F; background: transparent;");
        lbl->setFixedWidth(95);
        row->addWidget(lbl);
        valeur = new QLabel("-");
        valeur->setStyleSheet("font-size: 12px; color: #263B42; font-weight: 500;"
                              "background: transparent;");
        valeur->setWordWrap(true);
        row->addWidget(valeur, 1);
        return row;
    };

    contentL->addLayout(creerRow("CIN", m_detailCin));
    contentL->addLayout(creerRow("Naissance", m_detailNaissance));
    contentL->addLayout(creerRow("Adresse", m_detailAdresse));
    contentL->addLayout(creerRow("Téléphone", m_detailTel));
    contentL->addLayout(creerRow("Courriel", m_detailCourriel));
    contentL->addLayout(creerRow("Poste", m_detailPoste));
    contentL->addLayout(creerRow("Service", m_detailService));
    contentL->addLayout(creerRow("Contrat", m_detailContrat));
    contentL->addLayout(creerRow("Embauche", m_detailDate));
    contentL->addLayout(creerRow("Salaire", m_detailSalaire));
    contentL->addLayout(creerRow("Charge", m_detailCharge));
    contentL->addLayout(creerRow("Repos", m_detailDispo));
    contentL->addLayout(creerRow("Compétences", m_detailCompetences));

    contentL->addSpacing(20);

    QPushButton *btnModifier = new QPushButton("✏️  Modifier");
    btnModifier->setObjectName("btnPrimary");
    btnModifier->setMinimumHeight(46);
    btnModifier->setCursor(Qt::PointingHandCursor);
    connect(btnModifier, &QPushButton::clicked, this, &GestionEmployes::modifierEmploye);
    contentL->addWidget(btnModifier);

    QPushButton *btnBasculer = new QPushButton("🔄  Basculer disponibilité");
    btnBasculer->setObjectName("btnSuccess");
    btnBasculer->setMinimumHeight(46);
    btnBasculer->setCursor(Qt::PointingHandCursor);
    connect(btnBasculer, &QPushButton::clicked, this, &GestionEmployes::basculerDisponibilite);
    contentL->addWidget(btnBasculer);

    QPushButton *btnSupprimer = new QPushButton("🗑️  Supprimer");
    btnSupprimer->setObjectName("btnDanger");
    btnSupprimer->setMinimumHeight(46);
    btnSupprimer->setCursor(Qt::PointingHandCursor);
    connect(btnSupprimer, &QPushButton::clicked, this, &GestionEmployes::supprimerEmploye);
    contentL->addWidget(btnSupprimer);

    contentL->addStretch();

    scroll->setWidget(m_detailContent);
    layout->addWidget(scroll, 1);

    m_detailContent->hide();

    return card;
}

// ==================== CHARGEMENT ====================
void GestionEmployes::chargerEmployes(const QString &filtre, const QString &tri,
                                      const QString &poste, const QString &statut)
{
    // Schema Employes non conforme : on ne lance aucune requete (filtres inclus).
    if (!m_schemaOk)
        return;

    // Aucune requete ici : tout passe par le modele Employe (architecture Modele-Vue)
    const QList<Employe> liste = Employe::rechercher(filtre, tri, poste, statut);

    afficherCards(liste);
    mettreAJourStats();
}

// ==================== AFFICHER CARDS ====================
void GestionEmployes::afficherCards(const QList<Employe> &liste)
{
    QLayoutItem *item;
    while ((item = m_gridCards->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    int row = 0, col = 0;
    const int COLS = 2;

    for (const Employe &e : liste) {
        QWidget *card = creerCard(e);
        m_gridCards->addWidget(card, row, col);

        col++;
        if (col >= COLS) { col = 0; row++; }
    }
}

// ==================== AFFICHER DETAIL ====================
void GestionEmployes::afficherDetail(const Employe &e)
{
    m_detailPlaceholder->hide();
    m_detailContent->show();

    m_detailNom->setText(e.getPrenom() + " " + e.getNom());
    m_detailId->setText(QString("ID_EMP #%1").arg(e.getId()));

    if (e.isDisponible()) {
        m_detailStatut->setText("✅  Disponible");
        m_detailStatut->setStyleSheet(
            "font-size: 12px; color: #2E7D32; background-color: #E6F4EA;"
            "padding: 8px; border-radius: 8px; font-weight: bold; border: none;");
    } else {
        m_detailStatut->setText("❌  Non disponible");
        m_detailStatut->setStyleSheet(
            "font-size: 12px; color: #AE5751; background-color: #FBE9E7;"
            "padding: 8px; border-radius: 8px; font-weight: bold; border: none;");
    }

    m_detailCin->setText(e.getCin().isEmpty() ? "-" : e.getCin());
    m_detailNaissance->setText(e.getDateNaiss().isValid()
                                   ? e.getDateNaiss().toString("dd/MM/yyyy") : "-");
    m_detailAdresse->setText(e.getAdresse().isEmpty() ? "-" : e.getAdresse());
    m_detailTel->setText(e.getTel().isEmpty() ? "-" : e.getTel());
    m_detailCourriel->setText(e.getCourriel().isEmpty() ? "-" : e.getCourriel());
    m_detailPoste->setText(e.getPoste());
    m_detailService->setText(e.getIdServ() > 0 ? Employe::libelleService(e.getIdServ()) : "-");
    m_detailContrat->setText(e.getTypeContrat().isEmpty() ? "-" : e.getTypeContrat());
    m_detailDate->setText(e.getDateEmbauche().isValid()
                              ? e.getDateEmbauche().toString("dd/MM/yyyy") : "-");
    m_detailSalaire->setText(QString("%1 DT").arg(e.getSalaire(), 0, 'f', 2));

    double taux = e.tauxCharge() * 100;
    m_detailCharge->setText(QString("%1 h (%2%)")
                                .arg(e.getCharge(), 0, 'f', 0).arg(qRound(taux)));
    const QString reposTexte = e.getDispoReposText().trimmed();
    bool reposNumerique = false;
    reposTexte.toDouble(&reposNumerique);
    m_detailDispo->setText(reposTexte.isEmpty() ? "Non renseigné"
                                                : (reposNumerique ? reposTexte + " h" : reposTexte));
    m_detailCompetences->setText(e.getCompetences().isEmpty() ? "-" : e.getCompetences());
}

void GestionEmployes::effacerDetail()
{
    m_idSelectionne = -1;
    m_detailContent->hide();
    m_detailPlaceholder->show();
}

// ==================== STATS ====================
void GestionEmployes::mettreAJourStats()
{
    m_lblDisponibles->setText(QString::number(Employe::compterParDisponibilite(true)));
    m_lblIndisponibles->setText(QString::number(Employe::compterParDisponibilite(false)));

    const double total = Employe::chargeTotaleDisponibles();

    m_lblChargeTotale->setText(QString::number(qRound(total)));
}

// ==================== FILTRES ====================
void GestionEmployes::onFiltresChange()
{
    QString filtre = m_editRecherche->text().trimmed();
    QString poste = m_comboPoste->currentText();
    QString statut = m_comboStatut->currentText();
    const QString tri = (m_comboTri->currentIndex() == 1) ? "charge" : "date";
    chargerEmployes(filtre, tri, poste, statut);
}

// ==================== EVENT FILTER ====================
bool GestionEmployes::eventFilter(QObject *obj, QEvent *event)
{
    QWidget *card = qobject_cast<QWidget*>(obj);
    if (!card || card->property("employe_id").toInt() <= 0)
        return QWidget::eventFilter(obj, event);

    if (event->type() == QEvent::Enter) {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
        shadow->setBlurRadius(0);
        shadow->setColor(QColor(57, 127, 141, 150));
        shadow->setOffset(0, 6);
        card->setGraphicsEffect(shadow);

        QPropertyAnimation *anim = new QPropertyAnimation(shadow, "blurRadius");
        anim->setDuration(300);
        anim->setStartValue(0.0);
        anim->setEndValue(30.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start(QPropertyAnimation::DeleteWhenStopped);
        return true;
    }

    if (event->type() == QEvent::Leave) {
        QGraphicsDropShadowEffect *shadow =
            qobject_cast<QGraphicsDropShadowEffect*>(card->graphicsEffect());
        if (shadow) {
            QPropertyAnimation *anim = new QPropertyAnimation(shadow, "blurRadius");
            anim->setDuration(300);
            anim->setStartValue(shadow->blurRadius());
            anim->setEndValue(0.0);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, &QPropertyAnimation::finished, [card]() {
                card->setGraphicsEffect(nullptr);
            });
            anim->start(QPropertyAnimation::DeleteWhenStopped);
        }
        return true;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        int id = card->property("employe_id").toInt();
        if (id > 0) {
            m_idSelectionne = id;
            onFiltresChange();

            Employe emp;
            if (Employe::charger(id, emp))
                afficherDetail(emp);
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

// ==================== DIALOG FORMULAIRE ====================
bool GestionEmployes::ouvrirDialog(Employe &e, bool modification)
{
    QDialog dlg(this);
    dlg.setWindowTitle(modification ? "Modifier l'employé" : "Nouvel employé");
    const QRect zoneFormulaire = QGuiApplication::primaryScreen()
                                     ? QGuiApplication::primaryScreen()->availableGeometry()
                                     : QRect(0, 0, 1280, 800);
    dlg.resize(qMin(1040, zoneFormulaire.width() - 60), qMin(700, zoneFormulaire.height() - 70));
    dlg.setMinimumSize(qMin(900, zoneFormulaire.width() - 30), qMin(610, zoneFormulaire.height() - 30));
    dlg.setStyleSheet("QDialog { background-color: #F0F4F8; }");

    QVBoxLayout *mainL = new QVBoxLayout(&dlg);
    mainL->setContentsMargins(24, 20, 24, 18);
    mainL->setSpacing(12);

    QLabel *titre = new QLabel(modification ? "✏️  Modifier l'employé" : "➕  Nouvel employé");
    titre->setStyleSheet("font-size: 20px; font-weight: bold; color: #225A64;"
                         "background: transparent;");
    mainL->addWidget(titre);

    QGridLayout *form = new QGridLayout;
    form->setContentsMargins(0, 4, 0, 4);
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(12);
    form->setColumnMinimumWidth(0, 118);
    form->setColumnMinimumWidth(2, 138);
    form->setColumnStretch(1, 1);
    form->setColumnStretch(3, 1);
    auto ajouterChamp = [&](int ligne, int colonne, const QString &libelle, QWidget *champ) {
        QLabel *label = new QLabel(libelle);
        label->setMinimumWidth(colonne == 0 ? 112 : 132);
        label->setStyleSheet("font-size: 13px; color: #244B56; background: transparent;");
        form->addWidget(label, ligne, colonne, Qt::AlignVCenter);
        form->addWidget(champ, ligne, colonne + 1);
    };

    QLineEdit *editNom = new QLineEdit(e.getNom());
    editNom->setMinimumHeight(46);
    editNom->setPlaceholderText("Nom *");
    ajouterChamp(0, 0, "Nom *", editNom);

    QLineEdit *editPrenom = new QLineEdit(e.getPrenom());
    editPrenom->setMinimumHeight(46);
    editPrenom->setPlaceholderText("Prénom *");
    ajouterChamp(1, 0, "Prénom *", editPrenom);

    QLineEdit *editCin = new QLineEdit(e.getCin());
    editCin->setMinimumHeight(46);
    editCin->setMaxLength(8);
    editCin->setPlaceholderText("8 chiffres *");
    editCin->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9]{0,8}"), editCin));
    ajouterChamp(2, 0, "CIN *", editCin);

    // Une date sentinelle permet d'afficher un vrai état vide au lieu de
    // préremplir silencieusement une date que l'utilisateur n'a pas choisie.
    const QDate dateNonRenseignee(1900, 1, 1);
    QDateEdit *dateNaiss = new QDateEdit;
    dateNaiss->setMinimumHeight(46);
    dateNaiss->setCalendarPopup(true);
    dateNaiss->setDisplayFormat("dd/MM/yyyy");
    dateNaiss->setMinimumDate(dateNonRenseignee);
    dateNaiss->setMaximumDate(QDate::currentDate());
    dateNaiss->setSpecialValueText("Choisir une date *");
    dateNaiss->setDate(e.getDateNaiss().isValid() ? e.getDateNaiss() : dateNonRenseignee);
    ajouterChamp(3, 0, "Date de naissance *", dateNaiss);

    QLineEdit *editAdresse = new QLineEdit(e.getAdresse());
    editAdresse->setMinimumHeight(46);
    editAdresse->setMaxLength(250);
    ajouterChamp(4, 0, "Adresse", editAdresse);

    QLineEdit *editTel = new QLineEdit(e.getTel());
    editTel->setMinimumHeight(46);
    editTel->setPlaceholderText("+216 20 000 000");
    ajouterChamp(5, 0, modification ? "Tél" : "Tél *", editTel);

    QLineEdit *editCourriel = new QLineEdit(e.getCourriel());
    editCourriel->setMinimumHeight(46);
    editCourriel->setPlaceholderText("nom@domaine.tn");
    ajouterChamp(6, 0, modification ? "Courriel" : "Courriel *", editCourriel);

    QComboBox *comboPoste = new QComboBox;
    comboPoste->setMinimumHeight(46);
    comboPoste->addItem("— Choisir un poste —", QString());
    comboPoste->addItems({"Vétérinaire", "Assistant(e) vétérinaire",
                          "Auxiliaire vétérinaire", "Secrétaire / Réceptionniste",
                          "Agent d'entretien", "Aide aux soins animaliers"});
    if (!e.getPoste().isEmpty()) {
        if (comboPoste->findText(e.getPoste()) < 0)
            comboPoste->addItem(e.getPoste());
        comboPoste->setCurrentText(e.getPoste());
    } else {
        comboPoste->setCurrentIndex(0);
    }
    ajouterChamp(0, 2, "Poste *", comboPoste);

    // Service facultatif : vide par défaut. Les libellés visibles sont les
    // catégories de SERVICE ; seuls les services ayant un ID_SERV existant
    // peuvent être sélectionnés et enregistrés comme clé étrangère.
    QComboBox *comboService = new QComboBox;
    comboService->setMinimumHeight(46);
    comboService->addItem("— Aucun service —", 0);
    const QList<Employe::ServiceInfo> services = Employe::listerServices();
    for (const Employe::ServiceInfo &sv : services) {
        const int index = comboService->count();
        comboService->addItem(sv.libelle, sv.id);
        if (sv.id <= 0) {
            if (QStandardItemModel *model = qobject_cast<QStandardItemModel*>(comboService->model())) {
                if (QStandardItem *item = model->item(index)) {
                    item->setEnabled(false);
                    item->setToolTip("Cette catégorie n'a pas encore de ligne correspondante dans SERVICE.");
                }
            }
        }
    }
    if (e.getIdServ() > 0) {
        int idx = comboService->findData(e.getIdServ());
        if (idx < 0) {
            comboService->addItem(QString("Service #%1").arg(e.getIdServ()), e.getIdServ());
            idx = comboService->count() - 1;
        }
        comboService->setCurrentIndex(idx);
    } else {
        comboService->setCurrentIndex(0);
    }
    ajouterChamp(6, 2, "Service", comboService);

    QComboBox *comboContrat = new QComboBox;
    comboContrat->setMinimumHeight(46);
    comboContrat->addItem("— Choisir un type de contrat —", QString());
    comboContrat->addItems({"CDI", "CDD", "CIVP", "Stage", "Contractuel"});
    if (!e.getTypeContrat().isEmpty()) {
        if (comboContrat->findText(e.getTypeContrat()) < 0)
            comboContrat->addItem(e.getTypeContrat());
        comboContrat->setCurrentText(e.getTypeContrat());
    } else {
        comboContrat->setCurrentIndex(0);
    }
    ajouterChamp(2, 2, "Type de contrat *", comboContrat);

    QDateEdit *dateEmb = new QDateEdit;
    dateEmb->setMinimumHeight(46);
    dateEmb->setCalendarPopup(true);
    dateEmb->setDisplayFormat("dd/MM/yyyy");
    dateEmb->setMinimumDate(dateNonRenseignee);
    dateEmb->setMaximumDate(QDate::currentDate());
    dateEmb->setSpecialValueText("Choisir une date *");
    dateEmb->setDate(e.getDateEmbauche().isValid() ? e.getDateEmbauche() : dateNonRenseignee);
    ajouterChamp(1, 2, "Date d'embauche *", dateEmb);

    QLineEdit *editSalaire = new QLineEdit(e.getSalaire() > 0
                                               ? QString::number(e.getSalaire(), 'f', 0) : QString());
    editSalaire->setMinimumHeight(46);
    editSalaire->setPlaceholderText("Ex. : 1500");
    editSalaire->setValidator(new QRegularExpressionValidator(
        QRegularExpression("[0-9]{0,7}([.,][0-9]{0,2})?"), editSalaire));
    ajouterChamp(3, 2, modification ? "Salaire (DT)" : "Salaire (DT) *", editSalaire);

    QDoubleSpinBox *spinCharge = new QDoubleSpinBox;
    spinCharge->setMinimumHeight(46);
    spinCharge->setRange(0, 60);
    spinCharge->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    spinCharge->setSingleStep(1.0);
    spinCharge->setAccelerated(true);
    spinCharge->setSuffix(" h");
    spinCharge->setValue(e.getCharge());
    ajouterChamp(4, 2, "Charge (h / semaine)", spinCharge);

    QLineEdit *editRepos = new QLineEdit(e.getDispoReposText());
    editRepos->setMinimumHeight(46);
    editRepos->setMaxLength(20);
    editRepos->setPlaceholderText("Ex. : Dimanche");
    ajouterChamp(5, 2, "Repos", editRepos);

    QLineEdit *editComp = new QLineEdit(e.getCompetences());
    editComp->setMinimumHeight(46);
    editComp->setPlaceholderText("Soins, Toilettage...");
    ajouterChamp(7, 0, "Compétences", editComp);

    QCheckBox *checkDispo = new QCheckBox("Disponible");
    checkDispo->setTristate(false);
    checkDispo->setChecked(modification ? e.isDisponible() : true);
    form->addWidget(checkDispo, 7, 3, Qt::AlignLeft | Qt::AlignVCenter);

    // Le formulaire est long : il defile dans la fenetre au lieu de depasser l'ecran.
    QWidget *formContent = new QWidget;
    formContent->setStyleSheet("background: transparent;");
    formContent->setLayout(form);
    QScrollArea *formScroll = new QScrollArea;
    formScroll->setWidgetResizable(true);
    formScroll->setFrameShape(QFrame::NoFrame);
    formScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    formScroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    formScroll->setWidget(formContent);
    formScroll->setMinimumHeight(470);
    mainL->addWidget(formScroll, 1);
    mainL->addSpacing(4);

    QDialogButtonBox *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btns->button(QDialogButtonBox::Ok)->setText(modification ? "Modifier" : "Ajouter");
    btns->button(QDialogButtonBox::Ok)->setMinimumHeight(46);
    btns->button(QDialogButtonBox::Ok)->setMinimumWidth(120);
    btns->button(QDialogButtonBox::Cancel)->setMinimumHeight(46);
    btns->button(QDialogButtonBox::Cancel)->setMinimumWidth(120);
    // Construit un Employe a partir des champs du formulaire (id conserve)
    auto lireFormulaire = [&]() {
        Employe saisie = e;
        saisie.setNom(editNom->text().trimmed());
        saisie.setPrenom(editPrenom->text().trimmed());
        saisie.setCin(editCin->text().trimmed());
        saisie.setDateNaiss(dateNaiss->date() == dateNonRenseignee ? QDate() : dateNaiss->date());
        saisie.setAdresse(editAdresse->text().trimmed());
        saisie.setTel(editTel->text().trimmed());
        saisie.setCourriel(editCourriel->text().trimmed());
        saisie.setPoste(comboPoste->currentIndex() <= 0 ? QString() : comboPoste->currentText());
        saisie.setIdServ(comboService->currentData().toInt());
        saisie.setTypeContrat(comboContrat->currentIndex() <= 0 ? QString() : comboContrat->currentText());
        saisie.setDateEmbauche(dateEmb->date() == dateNonRenseignee ? QDate() : dateEmb->date());
        QString salaireTexte = editSalaire->text().trimmed();
        salaireTexte.replace(',', '.');
        saisie.setSalaire(salaireTexte.isEmpty()
                              ? (modification ? e.getSalaire() : -1.0)
                              : salaireTexte.toDouble());
        saisie.setCharge(spinCharge->value());
        saisie.setDispoRepos(editRepos->text().trimmed());
        saisie.setDisponible(checkDispo->isChecked());
        saisie.setCompetences(editComp->text().trimmed());
        return saisie;
    };

    // Controle de saisie (C++) : le dialogue reste ouvert tant que la saisie est invalide
    connect(btns, &QDialogButtonBox::accepted, &dlg, [&]() {
        const QString erreurSaisie = lireFormulaire().validerSaisie(!modification, !modification);
        if (!erreurSaisie.isEmpty()) {
            QMessageBox::warning(&dlg, "Validation", erreurSaisie);
            return;
        }
        dlg.accept();
    });
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    mainL->addWidget(btns);

    if (dlg.exec() != QDialog::Accepted) return false;

    // La requete (INSERT / UPDATE prepare) est dans le modele Employe
    Employe saisie = lireFormulaire();
    QString erreur;
    const bool ok = modification ? saisie.modifier(&erreur) : saisie.ajouter(&erreur);
    if (!ok) {
        QMessageBox::warning(this, "Erreur", "❌ " + erreur);
        return false;
    }

    e = saisie;
    return true;
}

// ==================== CRUD ====================
void GestionEmployes::ajouterEmploye()
{
    Employe vide;
    vide.setDispoRepos(QString());
    vide.setTypeContrat(QString());
    vide.setDateEmbauche(QDate());

    if (ouvrirDialog(vide, false)) {
        QMessageBox::information(this, "Succès", "✅ Employé ajouté");
        onFiltresChange();
    }
}

void GestionEmployes::modifierEmploye()
{
    if (m_idSelectionne <= 0) {
        QMessageBox::warning(this, "Attention", "Sélectionnez un employé dans le grid");
        return;
    }

    Employe e;
    if (!Employe::charger(m_idSelectionne, e)) return;

    if (ouvrirDialog(e, true)) {
        QMessageBox::information(this, "Succès", "✅ Employé modifié");
        onFiltresChange();
        afficherDetail(e);
    }
}

void GestionEmployes::supprimerEmploye()
{
    if (m_idSelectionne <= 0) {
        QMessageBox::warning(this, "Attention", "Sélectionnez un employé dans le grid");
        return;
    }

    QString nom;
    {
        Employe e;
        if (Employe::charger(m_idSelectionne, e))
            nom = e.getPrenom() + " " + e.getNom();
    }

    if (QMessageBox::question(this, "Confirmation",
                              "Supprimer " + nom + " ?") != QMessageBox::Yes)
        return;

    QString erreur;
    if (!Employe::supprimer(m_idSelectionne, &erreur)) {
        QMessageBox::warning(this, "Erreur", "❌ " + erreur);
        return;
    }

    QMessageBox::information(this, "Succès", "✅ Employé supprimé");
    effacerDetail();
    onFiltresChange();
}

// ==================== BASCULER ====================
void GestionEmployes::basculerDisponibilite()
{
    if (m_idSelectionne <= 0) {
        QMessageBox::warning(this, "Attention", "Sélectionnez un employé dans le grid");
        return;
    }

    bool nouveauStatut = false;
    QString erreur;
    if (Employe::basculerDisponibilite(m_idSelectionne, &nouveauStatut, &erreur)) {
        QMessageBox::information(this, "Succès",
                                 QString("✅ Statut changé : %1").arg(nouveauStatut ? "Disponible" : "Non disponible"));
        onFiltresChange();
    } else {
        QMessageBox::warning(this, "Erreur", "❌ " + erreur);
    }
}

// ==================== EMP-11 ====================
void GestionEmployes::verifierAdequation()
{
    bool ok;
    QString comp = QInputDialog::getText(this,
                                         "EMP-11 · Adéquation des compétences",
                                         "Compétence requise (ex: Soins, Toilettage, Accueil) :",
                                         QLineEdit::Normal, "", &ok);

    if (!ok || comp.trimmed().isEmpty()) return;

    QStringList resultats;
    const QList<Employe> trouves = Employe::rechercherParCompetence(comp);
    for (const Employe &e : trouves) {
        const QString nom = e.getNom() + " " + e.getPrenom();
        const double dispoH = e.getDispoRepos();
        const double taux = dispoH > 0 ? (e.getCharge() / dispoH * 100.0) : 0;
        resultats << QString("• %1 — %2 (%3% de charge)")
                         .arg(nom, e.getPoste()).arg(qRound(taux));
    }

    if (resultats.isEmpty()) {
        QMessageBox::information(this, "EMP-11 · Résultat",
                                 "❌ Aucun employé disponible ne possède la compétence « " + comp + " ».");
    } else {
        QMessageBox::information(this, "EMP-11 · Profils proposés",
                                 "✅ Employés habilités pour « " + comp + " » :\n\n" + resultats.join("\n"));
    }
}

// ==================== EMP-12 ====================
void GestionEmployes::verifierEquilibrage()
{
    QStringList surcharges, dispo;
    const QList<Employe> liste = Employe::listerDisponiblesParCharge(true);
    for (const Employe &e : liste) {
        const QString nom = e.getNom() + " " + e.getPrenom();
        const double charge = e.getCharge();
        const double d = e.getDispoRepos();
        const double taux = d > 0 ? charge / d : 0.0;

        if (taux > 0.8)
            surcharges << QString("⚠️  %1 — %2% (%3 h / %4 h)")
                              .arg(nom).arg(qRound(taux * 100)).arg(charge).arg(d);
        else if (taux < 0.5)
            dispo << QString("💡  %1 — %2% (%3 h / %4 h)")
                         .arg(nom).arg(qRound(taux * 100)).arg(charge).arg(d);
    }

    QString msg;
    msg += surcharges.isEmpty() ? "✅ Aucun employé en surcharge.\n\n"
                                : "🔴 Surcharge (>80%) :\n" + surcharges.join("\n") + "\n\n";
    if (!dispo.isEmpty())
        msg += "🟢 Moins chargés (<50%) — candidats réaffectation :\n" + dispo.join("\n");

    QMessageBox::information(this, "EMP-12 · Équilibrage", msg);
}
