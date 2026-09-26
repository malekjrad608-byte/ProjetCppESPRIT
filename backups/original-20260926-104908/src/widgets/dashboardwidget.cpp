#include "dashboardwidget.h"
#include "../database/databasemanager.h"
#include <QDate>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

DashboardWidget::DashboardWidget(QWidget *parent) : QWidget(parent) {
    setupUi();
    refresh();
}

void DashboardWidget::setupUi() {
    setStyleSheet(R"(
        QWidget { background: #1e1e2e; color: #cdd6f4; }
        QFrame#heroCard, QFrame#infoCard { background: #181825; border: 1px solid #313244; border-radius: 14px; }
        QFrame#statCard { background: #181825; border: 1px solid #313244; border-radius: 12px; }
    )");

    auto *root = new QVBoxLayout(this);
    root->setSpacing(18);
    root->setContentsMargins(36, 30, 36, 30);

    auto *hero = new QFrame(this);
    hero->setObjectName("heroCard");
    auto *heroLayout = new QHBoxLayout(hero);
    heroLayout->setContentsMargins(24, 20, 24, 20);

    auto *heroText = new QVBoxLayout;
    auto *title = new QLabel("Bonjour, bienvenue 👋", hero);
    QFont titleFont = title->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet("color: #89b4fa;");
    heroText->addWidget(title);

    auto *subtitle = new QLabel("Pilotez vos services et votre équipe depuis un seul espace.", hero);
    subtitle->setStyleSheet("color: #a6adc8; font-size: 14px;");
    heroText->addWidget(subtitle);
    heroLayout->addLayout(heroText);
    heroLayout->addStretch();

    auto *date = new QLabel(QDate::currentDate().toString("dddd  •  dd MMMM yyyy"), hero);
    date->setStyleSheet("color: #6c7086; font-size: 13px; padding: 8px 0;");
    date->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    heroLayout->addWidget(date);
    root->addWidget(hero);

    auto *sectionTitle = new QLabel("Vue d'ensemble", this);
    QFont sectionFont = sectionTitle->font();
    sectionFont.setPointSize(16);
    sectionFont.setBold(true);
    sectionTitle->setFont(sectionFont);
    sectionTitle->setStyleSheet("color: #cdd6f4;");
    root->addWidget(sectionTitle);

    auto *grid = new QGridLayout;
    grid->setSpacing(14);
    for (int column = 0; column < 4; ++column)
        grid->setColumnStretch(column, 1);

    auto makeCard = [&](const QString &label, const QString &accent, const QString &icon) {
        auto *card = new QFrame(this);
        card->setObjectName("statCard");
        card->setStyleSheet(QString(R"(
            QFrame#statCard { background: #181825; border: 1px solid #313244; border-top: 3px solid %1; border-radius: 12px; }
        )").arg(accent));

        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(18, 16, 18, 16);
        cardLayout->setSpacing(8);

        auto *top = new QHBoxLayout;
        auto *labelWidget = new QLabel(label, card);
        labelWidget->setStyleSheet("color: #9399b2; font-size: 11px; font-weight: bold; letter-spacing: 1px;");
        top->addWidget(labelWidget);
        top->addStretch();
        auto *iconWidget = new QLabel(icon, card);
        iconWidget->setStyleSheet(QString("color: %1; font-size: 18px;").arg(accent));
        top->addWidget(iconWidget);
        cardLayout->addLayout(top);

        auto *value = new QLabel("0", card);
        value->setStyleSheet(QString("color: %1; font-size: 34px; font-weight: bold;").arg(accent));
        cardLayout->addWidget(value);
        return qMakePair(card, value);
    };

    auto total = makeCard("SERVICES AU TOTAL", "#89b4fa", "▣");
    auto progress = makeCard("EN COURS", "#f9e2af", "◷");
    auto done = makeCard("TERMINÉS", "#a6e3a1", "✓");
    auto employees = makeCard("EMPLOYÉS", "#cba6f7", "♙");
    m_totalServices = total.second;
    m_inProgress = progress.second;
    m_done = done.second;
    m_employees = employees.second;
    grid->addWidget(total.first, 0, 0);
    grid->addWidget(progress.first, 0, 1);
    grid->addWidget(done.first, 0, 2);
    grid->addWidget(employees.first, 0, 3);
    root->addLayout(grid);

    auto *infoCard = new QFrame(this);
    infoCard->setObjectName("infoCard");
    auto *infoLayout = new QVBoxLayout(infoCard);
    infoLayout->setContentsMargins(22, 20, 22, 20);
    auto *infoTitle = new QLabel("Tout est prêt pour commencer", infoCard);
    QFont infoFont = infoTitle->font();
    infoFont.setPointSize(17);
    infoFont.setBold(true);
    infoTitle->setFont(infoFont);
    infoTitle->setStyleSheet("color: #cdd6f4;");
    infoLayout->addWidget(infoTitle);

    auto *infoText = new QLabel(
        "Utilisez le menu à gauche pour gérer vos services, votre équipe et vos plannings.\n"
        "Les indicateurs se mettent à jour automatiquement après chaque modification.",
        infoCard);
    infoText->setStyleSheet("color: #8187a1; font-size: 13px; line-height: 1.5;");
    infoText->setWordWrap(true);
    infoLayout->addWidget(infoText);
    root->addWidget(infoCard);
    root->addStretch();
}

void DashboardWidget::refresh() {
    auto &db = DatabaseManager::instance();
    const auto services = db.getAllServices();
    const auto employees = db.getAllEmployees();

    int inProgress = 0;
    int done = 0;
    for (const auto &service : services) {
        if (service.status == "En cours") ++inProgress;
        if (service.status == "Termine" || service.status == "Terminé") ++done;
    }

    m_totalServices->setText(QString::number(services.size()));
    m_inProgress->setText(QString::number(inProgress));
    m_done->setText(QString::number(done));
    m_employees->setText(QString::number(employees.size()));
}
