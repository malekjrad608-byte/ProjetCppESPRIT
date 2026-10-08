#include "medicalaccessdialog.h"
#include "../ui/design.h"
#include "../ui/motion.h"
#include "../login/loginvisuals.h"
#include "../database/databasemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

MedicalAccessDialog::MedicalAccessDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

void MedicalAccessDialog::setupUi()
{
    setObjectName("medicalAccessDialog");
    setWindowTitle("ANIWORLD — Accès Dossiers Médicaux");
    setMinimumWidth(480);
    setModal(true);

    QPalette colors = palette();
    colors.setColor(QPalette::Window, QColor("#FFFDF9"));
    setPalette(colors);
    setAutoFillBackground(true);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(48, 40, 48, 40);
    root->setSpacing(24);

    auto *head = new QHBoxLayout;
    auto *text = new QVBoxLayout;
    text->setSpacing(12);

    auto *eyebrow = new QLabel("SÉCURITÉ");
    eyebrow->setObjectName("eyebrow");
    text->addWidget(eyebrow);

    auto *welcome = new QLabel("Accès Dossiers Médicaux");
    welcome->setObjectName("welcome");
    text->addWidget(welcome);

    auto *subtitle = new QLabel("Ce module contient des informations sensibles.\nVeuillez vous identifier pour continuer.");
    subtitle->setObjectName("subtitle");
    subtitle->setWordWrap(true);
    text->addWidget(subtitle);

    head->addLayout(text);
    head->addStretch();

    auto *logo = Design::label("");
    logo->setPixmap(Design::brand("logo", QSize(60, 60)));
    head->addWidget(logo);

    root->addLayout(head);
    root->addSpacing(20);

    auto *userLabel = new QLabel("Identifiant utilisateur");
    userLabel->setObjectName("fieldLabel");

    m_userIdInput = new QLineEdit;
    m_userIdInput->setObjectName("medicalUserId");
    m_userIdInput->setPlaceholderText("Entrez votre ID utilisateur");
    m_userIdInput->setMaxLength(50);
    userLabel->setBuddy(m_userIdInput);

    root->addWidget(userLabel);
    root->addSpacing(8);
    root->addWidget(m_userIdInput);

    m_message = new QLabel;
    m_message->setObjectName("message");
    m_message->setWordWrap(true);
    m_message->hide();
    root->addSpacing(14);
    root->addWidget(m_message);

    auto *accessBtn = new LoginActionButton("Accéder");
    accessBtn->setObjectName("signInButton");
    accessBtn->setDefault(true);
    connect(accessBtn, &QPushButton::clicked, this, &MedicalAccessDialog::validateAccess);
    root->addSpacing(10);
    root->addWidget(accessBtn);

    root->addSpacing(20);

    auto *formNote = new QLabel("Entrez votre identifiant utilisateur pour accéder aux dossiers médicaux");
    formNote->setObjectName("formNote");
    formNote->setAlignment(Qt::AlignCenter);
    formNote->setWordWrap(true);
    root->addWidget(formNote);

    auto *footer = new QLabel("ANIWORLD  /  GESTION DES DOSSIERS MÉDICAUX");
    footer->setObjectName("formFooter");
    footer->setAlignment(Qt::AlignCenter);
    footer->setWordWrap(true);
    root->addWidget(footer);

    m_userIdInput->setFocus();
    connect(m_userIdInput, &QLineEdit::returnPressed, this, &MedicalAccessDialog::validateAccess);

    applyStyles();
}

void MedicalAccessDialog::validateAccess()
{
    const QString userId = m_userIdInput->text().trimmed();

    if (userId.isEmpty()) {
        m_message->setText("Veuillez entrer votre identifiant utilisateur.");
        m_message->setStyleSheet("background: #F9E6E0; color: #AF443A;");
        m_message->show();
        m_userIdInput->setFocus();
        Motion::reveal(m_message, QColor("#F3F7F8"));
        return;
    }

    // Verify user ID in database
    if (!DatabaseManager::instance().verifyUserId(userId)) {
        m_message->setText("Identifiant incorrect — accès refusé");
        m_message->setStyleSheet("background: #F9E6E0; color: #AF443A;");
        m_message->show();
        m_userIdInput->selectAll();
        m_userIdInput->setFocus();
        Motion::reveal(m_message, QColor("#F3F7F8"));
        return;
    }

    emit accessGranted(userId.toLower());
    accept();
}

QString MedicalAccessDialog::getUserId() const
{
    return m_userIdInput->text().trimmed();
}

void MedicalAccessDialog::applyStyles()
{
    setStyleSheet(R"(
        QWidget { font-family: "Segoe UI", sans-serif; font-size: 13px; color: #293F45; }
        QLabel { background: transparent; }
        QPushButton { padding: 0; }
        #eyebrow { color: #367985; font-size: 10px; font-weight: 700; }
        #welcome { color: #263D43; font-size: 36px; font-weight: 700; }
        #subtitle { color: #73868A; font-size: 13px; }
        #fieldLabel { color: #334D53; font-size: 12px; font-weight: 600; }
        QLineEdit {
            background: #F4F7F6; border: 1px solid #DDE6E3;
            border-radius: 11px; padding: 13px 12px;
            font-size: 13px; color: #293F45;
            selection-background-color: #397E89; selection-color: white;
        }
        QLineEdit:hover { border-color: #AAC7C6; }
        QLineEdit:focus { border: 1px solid #397E89; background: #FFFFFF; }
        #message { font-size: 12px; padding: 9px 12px; border-radius: 8px; }
        QPushButton#signInButton { border: none; background: transparent; min-height: 52px; }
        #formNote { color: #7A8B8C; font-size: 11px; }
        #formFooter { color: #849493; font-size: 9px; }
    )");
}

void MedicalAccessDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    Motion::reveal(this, QColor("#FFFDF9"), 200);
}
