#include "logindialog.h"
#include "loginvisuals.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QRadioButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QScreen>
#include <QScrollArea>
#include <QShowEvent>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace Palette {
const QString TealDark   = "#2B6973";
const QString Danger     = "#AF443A";
}

namespace {
constexpr auto LogoPath = ":/images/logo.png";

QPixmap logoPixmap(const QString &path, int size)
{
    const QPixmap source(path);
    if (source.isNull()) {
        return {};
    }

    const qreal ratio = qApp->devicePixelRatio();
    QPixmap result = source.scaled(qRound(size * ratio), qRound(size * ratio),
                                   Qt::KeepAspectRatio, Qt::SmoothTransformation);
    result.setDevicePixelRatio(ratio);
    return result;
}

QLabel *createLogoBadge(const char *objectName, int badgeSize, int imageSize)
{
    auto *logo = new QLabel;
    logo->setObjectName(objectName);
    logo->setAlignment(Qt::AlignCenter);
    logo->setFixedSize(badgeSize, badgeSize);
    logo->setPixmap(logoPixmap(LogoPath, imageSize));
    logo->setAccessibleName("Smart Pet Care logo");
    return logo;
}

enum class FieldIcon { User, Lock, ShowPassword, HidePassword };

QIcon fieldIcon(FieldIcon type)
{
    const qreal ratio = qApp->devicePixelRatio();
    QPixmap pixmap(qRound(20 * ratio), qRound(20 * ratio));
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor("#788F93"), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    if (type == FieldIcon::User) {
        p.drawEllipse(QPointF(10, 6), 3, 3);
        p.drawRoundedRect(QRectF(4, 12, 12, 6), 3, 3);
    } else if (type == FieldIcon::Lock) {
        p.drawRoundedRect(QRectF(4, 9, 12, 9), 2, 2);
        p.drawArc(QRectF(6, 2, 8, 12), 0, 180 * 16);
        p.drawLine(QPointF(10, 12), QPointF(10, 15));
    } else {
        QPainterPath eye;
        eye.moveTo(2, 10);
        eye.cubicTo(6, 3, 14, 3, 18, 10);
        eye.cubicTo(14, 17, 6, 17, 2, 10);
        p.drawPath(eye);
        p.drawEllipse(QPointF(10, 10), 2, 2);
        if (type == FieldIcon::HidePassword)
            p.drawLine(QPointF(3, 3), QPointF(17, 17));
    }
    return QIcon(pixmap);
}

void addPasswordToggle(QLineEdit *field)
{
    auto *toggle = field->addAction(fieldIcon(FieldIcon::ShowPassword), QLineEdit::TrailingPosition);
    toggle->setText("Show password");
    toggle->setToolTip("Show password");
    toggle->setCheckable(true);
    QObject::connect(toggle, &QAction::toggled, field, [field, toggle](bool visible) {
        field->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
        toggle->setIcon(fieldIcon(visible ? FieldIcon::HidePassword : FieldIcon::ShowPassword));
        toggle->setText(visible ? "Hide password" : "Show password");
        toggle->setToolTip(toggle->text());
    });
}

bool isValidEmail(const QString &value)
{
    static const QRegularExpression pattern(
        R"(^[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}$)",
        QRegularExpression::CaseInsensitiveOption);
    return pattern.match(value.trimmed()).hasMatch();
}

bool isValidPhone(const QString &value)
{
    QString compact = value.trimmed();
    compact.remove(QRegularExpression(R"([\s().-])"));

    static const QRegularExpression pattern(R"(^\+?[0-9]{8,15}$)");
    return pattern.match(compact).hasMatch();
}

QString maskedDestination(const QString &value, bool emailMode)
{
    const QString trimmed = value.trimmed();
    if (!emailMode) {
        QString digits = trimmed;
        digits.remove(QRegularExpression(R"(\D)"));
        return digits.size() >= 4 ? QString("phone ending in %1").arg(digits.right(4)) : trimmed;
    }

    const int at = trimmed.indexOf('@');
    if (at <= 1) {
        return trimmed;
    }

    return QString("%1***%2").arg(trimmed.left(1), trimmed.mid(at));
}
}

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Smart Pet Care Center - Login");
    setWindowIcon(QIcon(LogoPath));
    setMinimumSize(880, 620);
    const QSize available = screen()->availableGeometry().size() - QSize(32, 32);
    resize(QSize(1080, 760).boundedTo(available).expandedTo(minimumSize()));

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    buildUi();
    applyStyles();
}

void LoginDialog::buildUi()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(0);

    m_brandPanel = new BrandPanel;
    m_brandPanel->setMinimumWidth(340);

    auto *formPanel = new QWidget;
    formPanel->setObjectName("formPanel");

    auto *formLayout = new QVBoxLayout(formPanel);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(0);

    auto *windowControls = new QHBoxLayout;
    windowControls->setContentsMargins(24, 14, 16, 0);
    auto *motionToggle = new QPushButton("Pause motion");
    motionToggle->setObjectName("motionToggle");
    motionToggle->setCheckable(true);
    motionToggle->setCursor(Qt::PointingHandCursor);
    motionToggle->setToolTip("Pause decorative animations and transitions");
    auto *minimize = new QPushButton(QString(QChar(0x2212)));
    minimize->setObjectName("windowControl");
    minimize->setFixedSize(30, 30);
    minimize->setAccessibleName("Minimize window");
    minimize->setToolTip("Minimize");
    auto *closeButton = new QPushButton(QString(QChar(0x00D7)));
    closeButton->setObjectName("closeControl");
    closeButton->setFixedSize(30, 30);
    closeButton->setAccessibleName("Close window");
    closeButton->setToolTip("Close");
    windowControls->addWidget(motionToggle);
    windowControls->addStretch();
    windowControls->addWidget(minimize);
    windowControls->addWidget(closeButton);
    connect(motionToggle, &QPushButton::toggled, this, [this, motionToggle](bool paused) {
        setMotionEnabled(!paused);
        motionToggle->setText(paused ? "Enable motion" : "Pause motion");
    });
    connect(minimize, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    formLayout->addLayout(windowControls);

    m_pages = new QStackedWidget;
    m_pages->setObjectName("pages");
    m_pages->addWidget(createLoginPage());
    auto *recoveryScroll = new QScrollArea;
    recoveryScroll->setObjectName("recoveryScroll");
    recoveryScroll->setWidgetResizable(true);
    recoveryScroll->setFrameShape(QFrame::NoFrame);
    recoveryScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    recoveryScroll->setWidget(createResetPage());
    m_pages->addWidget(recoveryScroll);

    m_pageOpacity = new QGraphicsOpacityEffect(m_pages);
    m_pageOpacity->setOpacity(1.0);
    m_pages->setGraphicsEffect(m_pageOpacity);
    m_pageAnimation = new QPropertyAnimation(m_pageOpacity, "opacity", this);
    m_pageAnimation->setDuration(320);
    m_pageAnimation->setEasingCurve(QEasingCurve::OutCubic);

    formLayout->addWidget(m_pages, 1);

    root->addWidget(m_brandPanel, 2);
    root->addWidget(formPanel, 3);
}

QWidget *LoginDialog::createLoginPage()
{
    auto *page = new QWidget;
    page->setObjectName("loginPage");
    auto *formLayout = new QVBoxLayout(page);
    formLayout->setContentsMargins(48, 20, 48, 30);
    formLayout->setSpacing(0);

    auto *eyebrow = new QLabel("YOUR CARE STARTS HERE");
    eyebrow->setObjectName("eyebrow");

    auto *welcome = new QLabel("Welcome back");
    welcome->setObjectName("welcome");

    auto *subtitle = new QLabel("A good day for you. A great day for them.\nSign in to your care center.");
    subtitle->setObjectName("subtitle");
    subtitle->setWordWrap(true);

    auto *userLabel = new QLabel("Username");
    userLabel->setObjectName("fieldLabel");
    m_username = new QLineEdit;
    m_username->setPlaceholderText("Enter your username");
    m_username->setObjectName("input");
    m_username->setClearButtonEnabled(true);
    m_username->setAccessibleName("Username");
    m_username->addAction(fieldIcon(FieldIcon::User), QLineEdit::LeadingPosition);
    userLabel->setBuddy(m_username);

    auto *passLabel = new QLabel("Password");
    passLabel->setObjectName("fieldLabel");
    m_password = new QLineEdit;
    m_password->setPlaceholderText("Enter your password");
    m_password->setObjectName("input");
    m_password->setEchoMode(QLineEdit::Password);
    m_password->setAccessibleName("Password");
    m_password->addAction(fieldIcon(FieldIcon::Lock), QLineEdit::LeadingPosition);
    addPasswordToggle(m_password);
    passLabel->setBuddy(m_password);

    m_remember = new QCheckBox("Remember me");

    auto *forgot = new QPushButton("Forgot password?");
    forgot->setObjectName("forgot");
    forgot->setFlat(true);
    forgot->setCursor(Qt::PointingHandCursor);

    auto *optionsRow = new QHBoxLayout;
    optionsRow->addWidget(m_remember);
    optionsRow->addStretch();
    optionsRow->addWidget(forgot);

    m_message = new QLabel;
    m_message->setObjectName("message");
    m_message->setWordWrap(true);
    m_message->hide();

    m_loginBtn = new AnimatedButton("Sign in");
    m_loginBtn->setDefault(true);

    connect(forgot, &QPushButton::clicked, this, &LoginDialog::showResetPassword);
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::handleLogin);
    connect(m_password, &QLineEdit::returnPressed, this, &LoginDialog::handleLogin);
    connect(m_username, &QLineEdit::returnPressed, this, [this]() { m_password->setFocus(); });

    formLayout->addStretch(2);
    formLayout->addWidget(eyebrow);
    formLayout->addSpacing(12);
    formLayout->addWidget(welcome);
    formLayout->addSpacing(12);
    formLayout->addWidget(subtitle);
    formLayout->addSpacing(30);
    formLayout->addWidget(userLabel);
    formLayout->addSpacing(8);
    formLayout->addWidget(m_username);
    formLayout->addSpacing(20);
    formLayout->addWidget(passLabel);
    formLayout->addSpacing(8);
    formLayout->addWidget(m_password);
    formLayout->addSpacing(14);
    formLayout->addLayout(optionsRow);
    formLayout->addSpacing(18);
    formLayout->addWidget(m_message);
    formLayout->addSpacing(10);
    formLayout->addWidget(m_loginBtn);
    formLayout->addSpacing(24);
    auto *formNote = new QLabel("A little more organized. A lot more care.");
    formNote->setObjectName("formNote");
    formNote->setAlignment(Qt::AlignCenter);
    formLayout->addWidget(formNote);
    formLayout->addStretch(3);
    auto *footer = new QLabel("SMART PET CARE CENTER  /  MADE FOR YOUR TEAM");
    footer->setObjectName("formFooter");
    footer->setAlignment(Qt::AlignCenter);
    footer->setWordWrap(true);
    formLayout->addWidget(footer);

    QWidget::setTabOrder(m_username, m_password);
    QWidget::setTabOrder(m_password, m_remember);
    QWidget::setTabOrder(m_remember, forgot);
    QWidget::setTabOrder(forgot, m_loginBtn);

    return page;
}

QWidget *LoginDialog::createResetPage()
{
    auto *page = new QWidget;
    page->setObjectName("resetPage");
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 16, 40, 24);
    layout->setSpacing(8);

    auto *titleRow = new QHBoxLayout;
    auto *titleBlock = new QVBoxLayout;
    titleBlock->setSpacing(8);

    auto *title = new QLabel("Reset password");
    title->setObjectName("resetTitle");

    auto *subtitle = new QLabel("Choose email or phone, then confirm the 6-digit security code.");
    subtitle->setObjectName("subtitle");
    subtitle->setWordWrap(true);

    titleBlock->addWidget(title);
    titleBlock->addWidget(subtitle);

    titleRow->addLayout(titleBlock);
    titleRow->addStretch();
    titleRow->addWidget(createLogoBadge("resetLogoBadge", 64, 56));

    auto *methodLabel = new QLabel("Send code by");
    methodLabel->setObjectName("fieldLabel");

    m_emailOption = new QRadioButton("Email");
    m_emailOption->setObjectName("methodOption");
    m_emailOption->setChecked(true);
    m_phoneOption = new QRadioButton("Phone");
    m_phoneOption->setObjectName("methodOption");

    auto *methodRow = new QHBoxLayout;
    methodRow->setSpacing(10);
    methodRow->addWidget(m_emailOption);
    methodRow->addWidget(m_phoneOption);

    auto *contactLabel = new QLabel("Email or phone number");
    contactLabel->setObjectName("fieldLabel");
    m_contactInput = new QLineEdit;
    m_contactInput->setObjectName("input");
    m_contactInput->setClearButtonEnabled(true);
    m_contactInput->setAccessibleName("Recovery email or phone number");
    contactLabel->setBuddy(m_contactInput);

    m_sendCodeBtn = new QPushButton("Send Code");
    m_sendCodeBtn->setObjectName("secondaryBtn");
    m_sendCodeBtn->setCursor(Qt::PointingHandCursor);

    auto *codeLabel = new QLabel("Verification code");
    codeLabel->setObjectName("fieldLabel");
    m_codeInput = new QLineEdit;
    m_codeInput->setObjectName("input");
    m_codeInput->setPlaceholderText("Enter 6-digit code");
    m_codeInput->setMaxLength(6);
    m_codeInput->setAccessibleName("Verification code");
    codeLabel->setBuddy(m_codeInput);

    auto *newPassLabel = new QLabel("New password");
    newPassLabel->setObjectName("fieldLabel");
    m_newPassword = new QLineEdit;
    m_newPassword->setObjectName("input");
    m_newPassword->setPlaceholderText("Create a new password");
    m_newPassword->setEchoMode(QLineEdit::Password);
    m_newPassword->setAccessibleName("New password");
    addPasswordToggle(m_newPassword);
    newPassLabel->setBuddy(m_newPassword);

    auto *confirmPassLabel = new QLabel("Confirm password");
    confirmPassLabel->setObjectName("fieldLabel");
    m_confirmPassword = new QLineEdit;
    m_confirmPassword->setObjectName("input");
    m_confirmPassword->setPlaceholderText("Repeat the new password");
    m_confirmPassword->setEchoMode(QLineEdit::Password);
    m_confirmPassword->setAccessibleName("Confirm password");
    addPasswordToggle(m_confirmPassword);
    confirmPassLabel->setBuddy(m_confirmPassword);

    m_resetMessage = new QLabel;
    m_resetMessage->setObjectName("message");
    m_resetMessage->setWordWrap(true);
    m_resetMessage->hide();

    m_resetBtn = new AnimatedButton("Reset password");

    auto *backBtn = new QPushButton("Back to sign in");
    backBtn->setObjectName("linkBtn");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setFlat(true);

    connect(m_emailOption, &QRadioButton::toggled, this, &LoginDialog::updateResetContactMode);
    connect(m_phoneOption, &QRadioButton::toggled, this, &LoginDialog::updateResetContactMode);
    connect(m_sendCodeBtn, &QPushButton::clicked, this, &LoginDialog::sendResetCode);
    connect(m_resetBtn, &QPushButton::clicked, this, &LoginDialog::resetPassword);
    connect(backBtn, &QPushButton::clicked, this, &LoginDialog::showLoginForm);
    connect(m_confirmPassword, &QLineEdit::returnPressed, this, &LoginDialog::resetPassword);

    layout->addLayout(titleRow);
    layout->addSpacing(6);
    layout->addWidget(methodLabel);
    layout->addLayout(methodRow);
    layout->addWidget(contactLabel);
    layout->addWidget(m_contactInput);
    layout->addWidget(m_sendCodeBtn);
    layout->addWidget(codeLabel);
    layout->addWidget(m_codeInput);
    layout->addWidget(newPassLabel);
    layout->addWidget(m_newPassword);
    layout->addWidget(confirmPassLabel);
    layout->addWidget(m_confirmPassword);
    layout->addWidget(m_resetMessage);
    layout->addWidget(m_resetBtn);
    layout->addWidget(backBtn, 0, Qt::AlignHCenter);
    layout->addStretch();

    resetRecoveryState();
    updateResetContactMode();

    return page;
}

void LoginDialog::applyStyles()
{
    setStyleSheet(R"(
        QWidget { font-family: "Segoe UI", sans-serif; font-size: 13px; color: #293F45; }
        QLabel { background: transparent; }
        #formPanel {
            background: #FFFDF9;
            border-top-right-radius: 24px;
            border-bottom-right-radius: 24px;
        }
        #pages, #loginPage, #resetPage, #recoveryScroll, #recoveryScroll > QWidget > QWidget {
            background: transparent; border: none;
        }
        #brandName { color: #F7FCFB; font-size: 17px; font-weight: 700; }
        #brandTag { color: #B3D1D1; font-size: 9px; }
        #brandHeadline { color: #FFFFFF; font-size: 34px; font-weight: 600; }
        #brandDesc { color: #D1E5E3; font-size: 13px; }
        #brandFooter { color: #CEE4E2; font-size: 11px; }
        #resetLogoBadge { background: #FFFFFF; border: 1px solid #E2EBE9; border-radius: 18px; }
        #eyebrow { color: #367985; font-size: 10px; font-weight: 700; }
        #welcome { color: #263D43; font-size: 36px; font-weight: 700; }
        #resetTitle { color: #263D43; font-size: 28px; font-weight: 700; }
        #subtitle { color: #73868A; font-size: 13px; }
        #fieldLabel { color: #334D53; font-size: 12px; font-weight: 600; }
        QLineEdit#input {
            background: #F4F7F6; border: 1px solid #DDE6E3;
            border-radius: 11px; padding: 13px 12px;
            font-size: 13px; color: #293F45;
            selection-background-color: #397E89; selection-color: white;
        }
        QLineEdit#input:hover { border-color: #AAC7C6; }
        QLineEdit#input:focus { border: 1px solid #397E89; background: #FFFFFF; }
        QLineEdit#input:disabled { color: #91A4A6; background: #F1F3F1; border-color: #E4E9E6; }
        #resetPage QLineEdit#input { padding: 10px 12px; }
        QCheckBox { color: #697E82; font-size: 12px; spacing: 8px; }
        QCheckBox::indicator {
            width: 16px; height: 16px; border-radius: 5px;
            border: 1px solid #C9D8D6; background: #FFFFFF;
        }
        QCheckBox::indicator:checked {
            background: #397E89; border-color: #397E89; image: url(:/images/check.svg);
        }
        QCheckBox::indicator:hover, QCheckBox::indicator:focus { border: 1px solid #397E89; }
        QRadioButton#methodOption {
            color: #334D53; background: #F4F7F6; border: 1px solid #DDE6E3;
            border-radius: 10px; padding: 8px 12px; font-size: 12px; spacing: 8px;
        }
        QRadioButton#methodOption:checked { border-color: #397E89; background: #EAF3F1; }
        QRadioButton#methodOption:focus { border-color: #397E89; }
        QRadioButton#methodOption::indicator {
            width: 12px; height: 12px; border-radius: 7px;
            border: 1px solid #C4D6D4; background: #FFFFFF;
        }
        QRadioButton#methodOption::indicator:checked { background: #397E89; border-color: #397E89; }
        QPushButton#forgot, QPushButton#linkBtn {
            color: #2B6973; background: transparent; border: 1px solid transparent;
            border-radius: 5px; font-size: 12px; font-weight: 600; padding: 5px 2px;
        }
        QPushButton#forgot:hover, QPushButton#linkBtn:hover { color: #224C54; text-decoration: underline; }
        QPushButton#forgot:focus, QPushButton#linkBtn:focus { border: 1px solid #397E89; }
        #message { font-size: 12px; padding: 9px 12px; border-radius: 8px; }
        QPushButton#primaryBtn { border: none; background: transparent; min-height: 52px; }
        QPushButton#secondaryBtn {
            background: #EDF4F1; color: #2B6973; font-size: 13px; font-weight: 600;
            border: 1px solid #CDDEDA; border-radius: 10px; padding: 10px 16px;
        }
        QPushButton#secondaryBtn:hover { background: #E1EEEA; border-color: #397E89; }
        QPushButton#secondaryBtn:focus { border-color: #397E89; }
        QPushButton#secondaryBtn:pressed { background: #D5E8E2; }
        #formNote { color: #7A8B8C; font-size: 11px; }
        #formFooter { color: #849493; font-size: 9px; }
        QPushButton#motionToggle {
            color: #768987; background: transparent; border: 1px solid transparent;
            border-radius: 6px; font-size: 10px; padding: 5px 8px;
        }
        QPushButton#motionToggle:hover, QPushButton#motionToggle:checked { color: #2B6973; background: #EDF3EF; }
        QPushButton#motionToggle:focus { border: 1px solid #397E89; }
        QPushButton#windowControl, QPushButton#closeControl {
            background: transparent; color: #82928E; border: 1px solid transparent;
            border-radius: 8px; font-size: 19px;
        }
        QPushButton#windowControl:hover { background: #EDF3EF; color: #2B6973; }
        QPushButton#closeControl:hover { background: #F9E6E0; color: #AF443A; }
        QPushButton#windowControl:focus, QPushButton#closeControl:focus { border-color: #397E89; }
        QScrollBar:vertical { width: 7px; background: transparent; margin: 5px 0; }
        QScrollBar::handle:vertical { background: #C8D9D4; border-radius: 3px; min-height: 30px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
        QToolTip { color: #293F45; background: #FFFDF9; border: 1px solid #DDE6E3; padding: 5px; }
    )");
}
QString LoginDialog::getUsername() const
{
    return m_username ? m_username->text() : QString();
}

bool LoginDialog::isRememberChecked() const
{
    return m_remember && m_remember->isChecked();
}

void LoginDialog::handleLogin()
{
    const QString user = m_username->text().trimmed();
    const QString pass = m_password->text();

    if (user.isEmpty() || pass.isEmpty()) {
        showMessage("Please enter both your username and password.", true);
        return;
    }

    if (user.compare("rawand.k", Qt::CaseSensitive) == 0 && pass == "rawand123") {
        accept();
    } else {
        showMessage("Invalid username or password. Please try again.", true);
    }
}

void LoginDialog::showResetPassword()
{
    resetRecoveryState();
    m_pages->setCurrentIndex(1);
    animatePage();
    m_contactInput->setFocus();
}

void LoginDialog::showLoginForm()
{
    m_pages->setCurrentIndex(0);
    animatePage();
    m_password->setFocus();
}

void LoginDialog::sendResetCode()
{
    const bool emailMode = m_emailOption->isChecked();
    const QString destination = m_contactInput->text().trimmed();

    if (destination.isEmpty()) {
        showResetMessage("Enter your email address or phone number first.", true);
        return;
    }

    if (emailMode && !isValidEmail(destination)) {
        showResetMessage("Please enter a valid email address.", true);
        return;
    }

    if (!emailMode && !isValidPhone(destination)) {
        showResetMessage("Please enter a valid phone number with 8 to 15 digits.", true);
        return;
    }

    m_resetDestination = destination;
    m_resetCode = QString::number(QRandomGenerator::global()->bounded(100000, 1000000));

    m_codeInput->setEnabled(true);
    m_newPassword->setEnabled(true);
    m_confirmPassword->setEnabled(true);
    m_resetBtn->setEnabled(true);
    m_sendCodeBtn->setText("Send New Code");
    m_codeInput->clear();
    m_newPassword->clear();
    m_confirmPassword->clear();

    const QString masked = maskedDestination(destination, emailMode);
    showResetMessage(
        QString("Security code sent to %1. Demo delivery code: %2")
            .arg(masked, m_resetCode),
        false);
    m_codeInput->setFocus();
}

void LoginDialog::resetPassword()
{
    if (m_resetCode.isEmpty()) {
        showResetMessage("Send a security code before resetting your password.", true);
        return;
    }

    if (m_codeInput->text().trimmed() != m_resetCode) {
        showResetMessage("The security code is not correct. Please try again.", true);
        return;
    }

    const QString newPassword = m_newPassword->text();
    const QString confirmPassword = m_confirmPassword->text();

    if (newPassword.length() < 6) {
        showResetMessage("New password must be at least 6 characters long.", true);
        return;
    }

    if (newPassword != confirmPassword) {
        showResetMessage("The two password fields do not match.", true);
        return;
    }

    m_recoveredAccount = m_resetDestination;
    m_recoveredPassword = newPassword;
    showResetMessage("Password updated. Returning to sign in...", false);

    QTimer::singleShot(900, this, [this]() {
        m_username->setText(m_recoveredAccount);
        m_password->clear();
        showLoginForm();
        showMessage("Password updated. Sign in with your new password.", false);
    });
}

void LoginDialog::updateResetContactMode()
{
    if (!m_contactInput) {
        return;
    }

    if (m_emailOption->isChecked()) {
        m_contactInput->setPlaceholderText("name@example.com");
    } else {
        m_contactInput->setPlaceholderText("+216 12 345 678");
    }

    m_contactInput->clear();
    resetRecoveryState();
}

void LoginDialog::showMessage(const QString &text, bool isError)
{
    m_message->setText(QString("%1: %2").arg(isError ? "Error" : "Success", text));

    const QString color = isError ? Palette::Danger : Palette::TealDark;
    const QString bg = isError ? "#FCEBEA" : "#E7F7F4";
    m_message->setStyleSheet(
        QString("color:%1; background:%2; padding:8px 10px; "
                "border-radius:8px; font-size:12.5px;").arg(color, bg));
    m_message->show();
}

void LoginDialog::showResetMessage(const QString &text, bool isError)
{
    m_resetMessage->setText(QString("%1: %2").arg(isError ? "Error" : "Sent", text));

    const QString color = isError ? Palette::Danger : Palette::TealDark;
    const QString bg = isError ? "#FCEBEA" : "#E7F7F4";
    m_resetMessage->setStyleSheet(
        QString("color:%1; background:%2; padding:8px 10px; "
                "border-radius:8px; font-size:12.5px;").arg(color, bg));
    m_resetMessage->show();
}

void LoginDialog::resetRecoveryState()
{
    if (!m_codeInput || !m_newPassword || !m_confirmPassword || !m_resetBtn || !m_sendCodeBtn) {
        return;
    }

    m_resetCode.clear();
    m_resetDestination.clear();
    m_codeInput->clear();
    m_newPassword->clear();
    m_confirmPassword->clear();
    m_codeInput->setEnabled(false);
    m_newPassword->setEnabled(false);
    m_confirmPassword->setEnabled(false);
    m_resetBtn->setEnabled(false);
    m_sendCodeBtn->setText("Send Code");

    if (m_resetMessage) {
        m_resetMessage->hide();
    }
}

void LoginDialog::animatePage()
{
    m_pageAnimation->stop();
    if (!m_motionEnabled) {
        m_pageOpacity->setOpacity(1.0);
        return;
    }
    m_pageAnimation->setStartValue(0.0);
    m_pageAnimation->setEndValue(1.0);
    m_pageAnimation->start();
}

void LoginDialog::setMotionEnabled(bool enabled)
{
    m_motionEnabled = enabled;
    m_brandPanel->setMotionEnabled(enabled);
    static_cast<AnimatedButton *>(m_loginBtn)->setMotionEnabled(enabled);
    static_cast<AnimatedButton *>(m_resetBtn)->setMotionEnabled(enabled);
    if (!enabled) {
        m_pageAnimation->stop();
        m_pageOpacity->setOpacity(1.0);
    }
}

void LoginDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (m_firstShow) {
        m_firstShow = false;
        animatePage();
        m_username->setFocus();
    }
}

void LoginDialog::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    const QRectF card = QRectF(rect()).adjusted(16, 16, -16, -16);
    for (int spread = 12; spread > 0; --spread) {
        painter.setBrush(QColor(20, 48, 52, 2));
        painter.drawRoundedRect(card.adjusted(-spread, -spread + 3, spread, spread + 3),
                                24 + spread, 24 + spread);
    }
    painter.setBrush(QColor("#FFFDF9"));
    painter.drawRoundedRect(card, 24, 24);
}

void LoginDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->pos().y() < 60) {
        m_dragging = true;
        m_dragStart = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void LoginDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragStart);
        event->accept();
    }
}

void LoginDialog::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QDialog::mouseReleaseEvent(event);
}
