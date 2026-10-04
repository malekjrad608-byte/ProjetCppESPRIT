#include "statsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QProgressBar>
#include <QPushButton>

StatsDialog::StatsDialog(const Statistiques& st, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("📊 Statistiques — ANIWORLD");
    resize(600, 550);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->setContentsMargins(20, 20, 20, 20);
    main->setSpacing(15);

    QLabel* titre = new QLabel("📊 Statistiques des salles");
    titre->setObjectName("titleLabel");
    main->addWidget(titre);

    // ===== Compteurs =====
    QGroupBox* boxCompteurs = new QGroupBox("Vue d'ensemble");
    QFormLayout* form = new QFormLayout(boxCompteurs);

    auto ajouterLigne = [&](const QString& label, const QString& valeur, const QString& couleur) {
        QLabel* lblValeur = new QLabel(valeur);
        lblValeur->setStyleSheet(QString("color:%1; font-weight:bold; font-size:14px;").arg(couleur));
        form->addRow(label, lblValeur);
    };

    ajouterLigne("Total salles :",     QString::number(st.totalSalles),     "#4A8797");
    ajouterLigne("🟢 Libres :",        QString::number(st.sallesLibres),     "#27AE60");
    ajouterLigne("🔴 Occupées :",      QString::number(st.sallesOccupees),   "#E74C3C");
    ajouterLigne("⚫ Hors service :",  QString::number(st.sallesHorsService),"#7F8C8D");
    ajouterLigne("Capacité totale :",  QString::number(st.capaciteTotale),  "#4A8797");
    ajouterLigne("Occupation :",       QString::number(st.occupationTotale),"#EA9941");

    main->addWidget(boxCompteurs);

    // ===== Taux d'occupation =====
    QGroupBox* boxTaux = new QGroupBox("Taux d'occupation global");
    QVBoxLayout* vTaux = new QVBoxLayout(boxTaux);

    QProgressBar* bar = new QProgressBar();
    bar->setValue((int)st.tauxOccupation);
    bar->setFormat(QString("%1 %").arg(st.tauxOccupation, 0, 'f', 1));
    bar->setStyleSheet(
        "QProgressBar { border: 1px solid #BCBEC0; border-radius: 6px; "
        "text-align: center; height: 30px; font-weight: bold; color: #0A0A0A; }"
        "QProgressBar::chunk { background-color: #EA9941; border-radius: 5px; }");
    vTaux->addWidget(bar);

    QLabel* note = new QLabel(
        QString("%1 animaux sur %2 places occupées")
            .arg(st.occupationTotale).arg(st.capaciteTotale));
    note->setWordWrap(true);
    note->setStyleSheet("color:#5690A2; font-style:italic; padding-top:10px;");
    vTaux->addWidget(note);

    main->addWidget(boxTaux);

    // ===== Répartition par type (barres visuelles) =====
    QGroupBox* boxType = new QGroupBox("Répartition par type");
    QVBoxLayout* vType = new QVBoxLayout(boxType);

    int maxVal = 1;
    for (auto it = st.repartitionParType.begin(); it != st.repartitionParType.end(); ++it)
        if (it.value() > maxVal) maxVal = it.value();

    for (auto it = st.repartitionParType.begin(); it != st.repartitionParType.end(); ++it) {
        QHBoxLayout* ligne = new QHBoxLayout();

        QLabel* lblNom = new QLabel(it.key());
        lblNom->setFixedWidth(120);
        lblNom->setStyleSheet("font-weight:600;");
        ligne->addWidget(lblNom);

        QProgressBar* barre = new QProgressBar();
        barre->setRange(0, maxVal);
        barre->setValue(it.value());
        barre->setFormat(QString("%1").arg(it.value()));
        barre->setStyleSheet(
            "QProgressBar { border: 1px solid #BCBEC0; border-radius: 5px; "
            "text-align: center; height: 22px; font-weight: bold; color: #0A0A0A; }"
            "QProgressBar::chunk { background-color: #4A8797; border-radius: 4px; }");
        ligne->addWidget(barre, 1);

        vType->addLayout(ligne);
    }

    main->addWidget(boxType);

    // ===== Bouton Fermer =====
    QPushButton* btnFermer = new QPushButton("Fermer");
    btnFermer->setFixedWidth(120);
    connect(btnFermer, &QPushButton::clicked, this, &QDialog::accept);

    QHBoxLayout* bas = new QHBoxLayout();
    bas->addStretch();
    bas->addWidget(btnFermer);
    main->addLayout(bas);
}