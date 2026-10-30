#include "inventoryfeatures.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QColor>
#include <QDateTime>
#include <QDateEdit>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QFile>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSettings>
#include <QVariant>
#include <QStandardPaths>
#include <QDir>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTextDocument>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPrinter>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace {
const QString kPrimary = QStringLiteral("#397F8D");
const QString kPrimaryDark = QStringLiteral("#225A64");
const QString kInk = QStringLiteral("#263B42");
const QString kMuted = QStringLiteral("#71878D");
const QString kBorder = QStringLiteral("#D8E9EC");
const QString kBackground = QStringLiteral("#EAF4F6");
const QString kLocalDataHint = QStringLiteral("Les données des nouveaux modules sont conservées localement par Qt. Aucune table Oracle supplémentaire n’est nécessaire.");

struct Supplier {
    int id = 0;
    QString name;
    QString email;
    QString phone;
    int leadDays = 3;
};

struct ParsedInvoiceLine {
    QString article;
    QString lot;
    double quantity = 0.0;
    double unitPrice = 0.0;
    QDate expiry;
};

QPushButton *featureButton(const QString &text, bool primary = false, QWidget *parent = nullptr)
{
    auto *button = new QPushButton(text, parent);
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(36);
    button->setStyleSheet(QStringLiteral(
        "QPushButton { background:%1; color:%2; border:%3; border-radius:6px; padding:8px 12px; font-weight:600; }"
        "QPushButton:hover { background:%4; }"
        "QPushButton:disabled { background:#EDF3F4; color:#9AAEB2; border:1px solid #D8E5E7; }")
        .arg(primary ? kPrimary : QStringLiteral("#FFFFFF"),
             primary ? QStringLiteral("#FFFFFF") : kPrimaryDark,
             primary ? QStringLiteral("none") : QStringLiteral("1px solid #BDD9DE"),
             primary ? kPrimaryDark : QStringLiteral("#F0F7F8")));
    return button;
}

void applyFeatureDialogStyle(QDialog &dialog)
{
    dialog.setStyleSheet(QStringLiteral(
        "QDialog { background:%1; color:%2; font-family:'Segoe UI'; }"
        "QLabel { color:%2; background:transparent; }"
        "QLineEdit, QComboBox, QSpinBox, QDateEdit { background:#FFFFFF; color:%2; border:1px solid #C9E0E5; border-radius:5px; padding:6px; min-height:22px; }"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDateEdit:focus { border:1px solid %3; }"
        "QTableWidget { background:#FFFFFF; alternate-background-color:#F8FBFC; color:%2; border:1px solid %4; gridline-color:#E2EEF0; selection-background-color:#D6ECEF; selection-color:%2; }"
        "QHeaderView::section { background:%3; color:#FFFFFF; font-weight:700; padding:8px; border:none; border-right:1px solid #2E7280; }")
        .arg(kBackground, kInk, kPrimary, kBorder));
}

QString dbError(const QSqlQuery &query)
{
    return query.lastError().text().trimmed();
}

QString localDataFile()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + QStringLiteral("/aniworld_features.ini");
}

QVector<Supplier> loadSuppliers(QString *error = nullptr)
{
    Q_UNUSED(error);
    QVector<Supplier> suppliers;
    QSettings settings(localDataFile(), QSettings::IniFormat);
    const int count = settings.beginReadArray("suppliers");
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        Supplier s;
        s.id = settings.value("id", i + 1).toInt();
        s.name = settings.value("name").toString();
        s.email = settings.value("email").toString();
        s.phone = settings.value("phone").toString();
        s.leadDays = qMax(0, settings.value("leadDays", 3).toInt());
        if (!s.name.trimmed().isEmpty()) suppliers.append(s);
    }
    settings.endArray();
    return suppliers;
}

bool saveSuppliers(const QVector<Supplier> &suppliers)
{
    QSettings settings(localDataFile(), QSettings::IniFormat);
    settings.beginWriteArray("suppliers");
    for (int i = 0; i < suppliers.size(); ++i) {
        settings.setArrayIndex(i);
        const Supplier &s = suppliers.at(i);
        settings.setValue("id", s.id);
        settings.setValue("name", s.name);
        settings.setValue("email", s.email);
        settings.setValue("phone", s.phone);
        settings.setValue("leadDays", s.leadDays);
    }
    settings.endArray();
    settings.sync();
    return settings.status() == QSettings::NoError;
}

bool addSupplier(QWidget *parent)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Ajouter un fournisseur");
    dialog.setMinimumWidth(450);
    applyFeatureDialogStyle(dialog);
    auto *root = new QVBoxLayout(&dialog);
    auto *title = new QLabel("Nouveau fournisseur");
    title->setStyleSheet(QStringLiteral("font-size:20px;font-weight:800;color:%1;").arg(kPrimaryDark));
    root->addWidget(title);
    auto *form = new QFormLayout;
    auto *nameEdit = new QLineEdit; nameEdit->setPlaceholderText("Nom de la société");
    auto *emailEdit = new QLineEdit; emailEdit->setPlaceholderText("contact@fournisseur.tn");
    auto *phoneEdit = new QLineEdit; phoneEdit->setPlaceholderText("Téléphone (facultatif)");
    auto *leadSpin = new QSpinBox; leadSpin->setRange(0, 180); leadSpin->setValue(3); leadSpin->setSuffix(" jours");
    form->addRow("Nom *", nameEdit); form->addRow("E-mail", emailEdit); form->addRow("Téléphone", phoneEdit); form->addRow("Délai de livraison", leadSpin);
    root->addLayout(form);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText("Enregistrer");
    buttons->button(QDialogButtonBox::Save)->setStyleSheet(QStringLiteral("background:%1;color:white;border:none;").arg(kPrimary));
    buttons->button(QDialogButtonBox::Cancel)->setText("Annuler"); root->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (nameEdit->text().trimmed().isEmpty()) { QMessageBox::warning(&dialog, "Champ obligatoire", "Saisissez le nom du fournisseur."); return; }
        if (!emailEdit->text().trimmed().isEmpty() && !QRegularExpression(QStringLiteral("^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$")).match(emailEdit->text().trimmed()).hasMatch()) { QMessageBox::warning(&dialog, "E-mail invalide", "Vérifiez l'adresse e-mail du fournisseur."); return; }
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return false;
    QVector<Supplier> suppliers = loadSuppliers();
    int id = 1; for (const Supplier &s : suppliers) id = qMax(id, s.id + 1);
    Supplier supplier; supplier.id = id; supplier.name = nameEdit->text().trimmed(); supplier.email = emailEdit->text().trimmed(); supplier.phone = phoneEdit->text().trimmed(); supplier.leadDays = leadSpin->value();
    suppliers.append(supplier);
    if (!saveSuppliers(suppliers)) { QMessageBox::critical(parent, "Stockage local", "Impossible d'enregistrer le fournisseur dans les données locales de Qt."); return false; }
    return true;
}

QString normalizedArticle(QString value)
{
    value = value.trimmed();
    const int end = value.indexOf(']');
    if (value.startsWith('[') && end >= 0) value = value.mid(end + 1).trimmed();
    return value;
}

QString prefixForCategory(const QString &category)
{
    if (category == "Chiens") return "[CHIEN] ";
    if (category == "Chats") return "[CHAT] ";
    if (category == "Oiseaux") return "[OISEAU] ";
    if (category == "Poissons") return "[POISSON] ";
    if (category == "Rongeurs") return "[HAMSTER] ";
    return "[MATÉRIEL] ";
}

QString cleanNumber(QString value)
{
    value = value.trimmed();
    value.remove(' ');
    value.replace(',', '.');
    value.remove(QRegularExpression(QStringLiteral("[^0-9.\\-]")));
    return value;
}

bool isNumberField(const QString &value)
{
    return QRegularExpression(QStringLiteral("^\\d+(?:[.,]\\d+)?$")).match(value.trimmed()).hasMatch();
}

QDate findDate(const QString &line)
{
    const QRegularExpression dateRx(QStringLiteral("(\\d{1,2}[./-]\\d{1,2}[./-]\\d{2,4}|\\d{4}-\\d{2}-\\d{2})"));
    const auto match = dateRx.match(line);
    if (!match.hasMatch()) return QDate();
    const QString value = match.captured(1);
    QDate date = QDate::fromString(value, "dd/MM/yyyy");
    if (!date.isValid()) date = QDate::fromString(value, "d/M/yyyy");
    if (!date.isValid()) date = QDate::fromString(value, "dd-MM-yyyy");
    if (!date.isValid()) date = QDate::fromString(value, "d-M-yyyy");
    if (!date.isValid()) date = QDate::fromString(value, "dd.MM.yyyy");
    if (!date.isValid()) date = QDate::fromString(value, "yyyy-MM-dd");
    if (date.isValid() && date.year() < 100) date = date.addYears(2000);
    return date;
}

QVector<ParsedInvoiceLine> parseInvoiceText(const QString &text)
{
    QVector<ParsedInvoiceLine> lines;
    const QStringList sourceLines = text.split('\n');
    const QRegularExpression splitRx(QStringLiteral("\\s{2,}"));
    for (QString line : sourceLines) {
        line = line.trimmed();
        if (line.size() < 5) continue;
        const QString lower = line.toLower();
        if (lower.contains("total") || lower.contains("tva") || lower.contains("montant") ||
            lower.contains("facture n") || lower.contains("désignation") || lower.contains("designation") ||
            lower.contains("quantité") || lower.contains("quantite") || lower.contains("prix unitaire") ||
            lower.contains("adresse") || lower.contains("matricule") || lower.contains("client")) continue;

        QString lineForColumns = line;
        const QRegularExpression dateColumnRx(QStringLiteral("\\d{1,2}[./-]\\d{1,2}[./-]\\d{2,4}|\\d{4}-\\d{2}-\\d{2}"));
        lineForColumns.remove(dateColumnRx);
        QStringList parts = lineForColumns.split(splitRx, Qt::SkipEmptyParts);
        // pdftotext usually preserves table spacing; OCR text often does not.
        if (parts.size() < 3) {
            const QRegularExpression trailingRx(QStringLiteral("^(.+?)\\s+(\\d+(?:[.,]\\d+)?)\\s+([0-9][0-9 ,.]*[0-9])\\s*$"));
            const auto m = trailingRx.match(lineForColumns.trimmed());
            if (m.hasMatch()) parts = {m.captured(1).trimmed(), m.captured(2), m.captured(3).trimmed()};
        }
        if (parts.size() < 3) continue;

        ParsedInvoiceLine parsed;
        parsed.expiry = findDate(line);
        int qtyIndex = -1;
        int priceIndex = -1;
        int totalIndex = -1;
        const int n = parts.size();
        if (n >= 4 && isNumberField(parts.at(n - 1)) && isNumberField(parts.at(n - 2)) && isNumberField(parts.at(n - 3))) {
            const double q = cleanNumber(parts.at(n - 3)).toDouble();
            const double p = cleanNumber(parts.at(n - 2)).toDouble();
            const double t = cleanNumber(parts.at(n - 1)).toDouble();
            if (q > 0.0 && std::abs(q * p - t) <= qMax(0.05, std::abs(t) * 0.03)) {
                qtyIndex = n - 3; priceIndex = n - 2; totalIndex = n - 1;
            }
        }
        if (qtyIndex < 0 && isNumberField(parts.at(n - 1)) && isNumberField(parts.at(n - 2))) {
            qtyIndex = n - 2;
            priceIndex = n - 1;
        }
        if (qtyIndex < 1 || priceIndex < 1) continue;

        parsed.quantity = cleanNumber(parts.at(qtyIndex)).toDouble();
        parsed.unitPrice = cleanNumber(parts.at(priceIndex)).toDouble();
        if (parsed.quantity <= 0.0 || parsed.unitPrice < 0.0) continue;
        parsed.article = parts.first().trimmed();
        parsed.article.remove(QRegularExpression(QStringLiteral("^\\d+[.)-]\\s*")));
        if (parsed.article.size() < 2) continue;

        const int lotLimit = qtyIndex;
        for (int i = 1; i < lotLimit; ++i) {
            if (i == priceIndex || i == totalIndex) continue;
            const QString field = parts.at(i).trimmed();
            if (findDate(field).isValid() || isNumberField(field)) continue;
            if (!parsed.lot.isEmpty()) parsed.lot += "-";
            parsed.lot += field;
        }
        if (parsed.lot.isEmpty()) parsed.lot = QStringLiteral("LOT-%1").arg(lines.size() + 1, 3, 10, QChar('0'));
        lines.append(parsed);
    }
    return lines;
}

QString extractPdfText(const QString &pdfPath, QString *error)
{
    QProcess textProcess;
    textProcess.start(QStringLiteral("pdftotext"), {QStringLiteral("-layout"), pdfPath, QStringLiteral("-")});
    if (textProcess.waitForStarted(2500) && textProcess.waitForFinished(30000) && textProcess.exitStatus() == QProcess::NormalExit && textProcess.exitCode() == 0) {
        const QString text = QString::fromLocal8Bit(textProcess.readAllStandardOutput());
        if (text.trimmed().size() > 10) return text;
    }

    // Scanned invoices: render pages with Poppler and run Tesseract OCR.
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        if (error) *error = "Impossible de créer un dossier temporaire pour l'OCR.";
        return QString();
    }
    const QString prefix = tempDir.path() + QDir::separator() + QStringLiteral("page");
    QProcess render;
    render.start(QStringLiteral("pdftoppm"), {QStringLiteral("-png"), QStringLiteral("-r"), QStringLiteral("180"), pdfPath, prefix});
    if (!render.waitForStarted(2500) || !render.waitForFinished(60000) || render.exitCode() != 0) {
        if (error) *error = "Lecture PDF directe indisponible. Installez Poppler (pdftotext/pdftoppm) et Tesseract, puis ajoutez-les au PATH. Détail : " + QString::fromLocal8Bit(render.readAllStandardError()).trimmed();
        return QString();
    }
    const QStringList images = QDir(tempDir.path()).entryList({"page-*.png"}, QDir::Files, QDir::Name);
    QString fullText;
    for (const QString &imageName : images) {
        QProcess ocr;
        ocr.start(QStringLiteral("tesseract"), {QDir(tempDir.path()).filePath(imageName), QStringLiteral("stdout"), QStringLiteral("-l"), QStringLiteral("fra+eng"), QStringLiteral("--psm"), QStringLiteral("6")});
        if (!ocr.waitForStarted(2500) || !ocr.waitForFinished(30000) || ocr.exitCode() != 0) {
            QProcess fallback;
            fallback.start(QStringLiteral("tesseract"), {QDir(tempDir.path()).filePath(imageName), QStringLiteral("stdout"), QStringLiteral("-l"), QStringLiteral("eng"), QStringLiteral("--psm"), QStringLiteral("6")});
            if (!fallback.waitForStarted(2500) || !fallback.waitForFinished(30000) || fallback.exitCode() != 0) continue;
            fullText += QString::fromLocal8Bit(fallback.readAllStandardOutput()) + "\n";
        } else {
            fullText += QString::fromLocal8Bit(ocr.readAllStandardOutput()) + "\n";
        }
    }
    if (fullText.trimmed().isEmpty() && error) *error = "L'OCR n'a pas extrait de texte du document.";
    return fullText;
}

void showSimpleTable(QWidget *parent, const QString &titleText, const QStringList &headers,
                     const QVector<QStringList> &rows, const QString &emptyText = QStringLiteral("Aucune donnée à afficher."))
{
    QDialog dialog(parent);
    dialog.setWindowTitle(titleText);
    dialog.resize(1000, 560);
    applyFeatureDialogStyle(dialog);
    auto *root = new QVBoxLayout(&dialog);
    auto *title = new QLabel(titleText);
    title->setStyleSheet(QStringLiteral("font-size:20px;font-weight:800;color:%1;").arg(kPrimaryDark));
    root->addWidget(title);
    auto *table = new QTableWidget;
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    for (const QStringList &rowValues : rows) {
        const int row = table->rowCount();
        table->insertRow(row);
        for (int col = 0; col < headers.size(); ++col)
            table->setItem(row, col, new QTableWidgetItem(col < rowValues.size() ? rowValues.at(col) : QString()));
    }
    if (rows.isEmpty()) {
        table->setRowCount(1);
        auto *empty = new QTableWidgetItem(emptyText);
        empty->setTextAlignment(Qt::AlignCenter);
        table->setItem(0, 0, empty);
        table->setSpan(0, 0, 1, headers.size());
    }
    root->addWidget(table, 1);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    buttons->button(QDialogButtonBox::Close)->setText("Fermer");
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    root->addWidget(buttons);
    dialog.exec();
}

void showSupplierManager(QWidget *parent)
{
    QString error;
    QVector<Supplier> suppliers = loadSuppliers(&error);
    if (!error.isEmpty()) {
        QMessageBox::critical(parent, "Fournisseurs", error + "\n\n" + kLocalDataHint);
        return;
    }
    QDialog dialog(parent);
    dialog.setWindowTitle("Gestion des fournisseurs");
    dialog.resize(780, 480);
    applyFeatureDialogStyle(dialog);
    auto *root = new QVBoxLayout(&dialog);
    auto *title = new QLabel("Fournisseurs");
    title->setStyleSheet(QStringLiteral("font-size:20px;font-weight:800;color:%1;").arg(kPrimaryDark));
    root->addWidget(title);
    auto *table = new QTableWidget;
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"FOURNISSEUR", "E-MAIL", "TÉLÉPHONE", "DÉLAI DE LIVRAISON"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    for (const Supplier &s : suppliers) {
        int row = table->rowCount(); table->insertRow(row);
        table->setItem(row,0,new QTableWidgetItem(s.name));
        table->setItem(row,1,new QTableWidgetItem(s.email));
        table->setItem(row,2,new QTableWidgetItem(s.phone));
        table->setItem(row,3,new QTableWidgetItem(QString::number(s.leadDays) + " jours"));
    }
    root->addWidget(table, 1);
    auto *buttonsRow = new QHBoxLayout;
    auto *add = featureButton("＋ Ajouter un fournisseur", true);
    auto *close = featureButton("Fermer");
    buttonsRow->addStretch(1); buttonsRow->addWidget(add); buttonsRow->addWidget(close);
    root->addLayout(buttonsRow);
    QObject::connect(add, &QPushButton::clicked, &dialog, [&]() {
        if (addSupplier(&dialog)) {
            dialog.accept();
            showSupplierManager(parent);
        }
    });
    QObject::connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}

void showPurchaseHistory(QWidget *parent)
{
    QSettings settings(localDataFile(), QSettings::IniFormat);
    QVector<QStringList> rows;
    const int count = settings.beginReadArray("purchaseHistory");
    for (int i = count - 1; i >= 0; --i) {
        settings.setArrayIndex(i);
        rows.append({settings.value("date").toString(), settings.value("invoice").toString(),
                     settings.value("supplier").toString(), settings.value("article").toString(),
                     settings.value("lot").toString(), settings.value("quantity").toString(),
                     settings.value("price").toString(), settings.value("previousPrice", "—").toString()});
    }
    settings.endArray();
    showSimpleTable(parent, "Historique des achats et suivi des prix",
                    {"DATE", "FACTURE", "FOURNISSEUR", "ARTICLE", "LOT", "QTÉ", "PRIX ACTUEL", "PRIX PRÉCÉDENT"}, rows,
                    "Aucune facture d'achat validée pour le moment.");
}

bool importInvoice(QWidget *parent, const QVector<StockRecord> &)
{
    const QString pdfPath = QFileDialog::getOpenFileName(parent, "Importer une facture d'achat", QString(), "Factures PDF (*.pdf)");
    if (pdfPath.isEmpty()) return false;
    QString extractError;
    const QString extracted = extractPdfText(pdfPath, &extractError);
    if (extracted.trimmed().isEmpty()) {
        QMessageBox::warning(parent, "Lecture de facture impossible",
            (extractError.isEmpty() ? QStringLiteral("Aucun texte lisible n'a été trouvé.") : extractError) +
            "\n\nVous pouvez vérifier que pdftotext/pdftoppm et Tesseract sont installés.");
        return false;
    }

    QString supplierError;
    QVector<Supplier> suppliers = loadSuppliers(&supplierError);
    if (!supplierError.isEmpty()) {
        QMessageBox::critical(parent, "Fournisseurs", supplierError);
        return false;
    }
    if (suppliers.isEmpty()) {
        if (QMessageBox::question(parent, "Aucun fournisseur", "Ajoutez d'abord un fournisseur pour rattacher cette facture.\n\nVoulez-vous créer un fournisseur maintenant ?",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
            if (addSupplier(parent)) suppliers = loadSuppliers(&supplierError);
        }
        if (suppliers.isEmpty()) return false;
    }

    QRegularExpression invoiceNoRx(QStringLiteral("(?:facture|invoice|n[°o]|num[eé]ro)\\s*[:#-]?\\s*([A-Z0-9][A-Z0-9./_-]{2,})"), QRegularExpression::CaseInsensitiveOption);
    QString invoiceNumber = QFileInfo(pdfPath).completeBaseName();
    const auto noMatch = invoiceNoRx.match(extracted);
    if (noMatch.hasMatch()) invoiceNumber = noMatch.captured(1).trimmed();
    invoiceNumber = invoiceNumber.left(100);

    QDate invoiceDate = QDate::currentDate();
    const QRegularExpression dateRx(QStringLiteral("(?:date)?\\s*[: ]*([0-3]?\\d[./-][01]?\\d[./-](?:20\\d{2}|\\d{2}))"), QRegularExpression::CaseInsensitiveOption);
    const auto dateMatch = dateRx.match(extracted);
    if (dateMatch.hasMatch()) {
        const QString dateText = dateMatch.captured(1);
        QDate parsed = QDate::fromString(dateText, "dd/MM/yyyy");
        if (!parsed.isValid()) parsed = QDate::fromString(dateText, "d/M/yyyy");
        if (!parsed.isValid()) parsed = QDate::fromString(dateText, "dd-MM-yyyy");
        if (!parsed.isValid()) parsed = QDate::fromString(dateText, "dd.MM.yyyy");
        if (!parsed.isValid()) parsed = QDate::fromString(dateText, "dd/MM/yy");
        if (parsed.isValid()) invoiceDate = parsed;
    }
    QVector<ParsedInvoiceLine> parsedLines = parseInvoiceText(extracted);

    QDialog dialog(parent);
    dialog.setWindowTitle("Vérifier et valider la facture d'achat");
    dialog.resize(1000, 650);
    applyFeatureDialogStyle(dialog);
    auto *root = new QVBoxLayout(&dialog);
    auto *title = new QLabel("Import de facture — vérification avant mise à jour du stock");
    title->setStyleSheet(QStringLiteral("font-size:20px;font-weight:800;color:%1;").arg(kPrimaryDark));
    root->addWidget(title);
    auto *subtitle = new QLabel("Les valeurs extraites automatiquement restent modifiables. Vérifiez chaque article, lot, quantité, prix et date avant de valider.");
    subtitle->setWordWrap(true); subtitle->setStyleSheet(QStringLiteral("color:%1;").arg(kMuted));
    root->addWidget(subtitle);

    auto *form = new QFormLayout;
    auto *supplierBox = new QComboBox;
    for (const Supplier &s : suppliers) supplierBox->addItem(s.name + (s.email.isEmpty() ? QString() : " — " + s.email), s.id);
    auto *supplierRow = new QWidget;
    auto *supplierLayout = new QHBoxLayout(supplierRow); supplierLayout->setContentsMargins(0,0,0,0);
    auto *newSupplierButton = featureButton("＋ Fournisseur");
    supplierLayout->addWidget(supplierBox,1); supplierLayout->addWidget(newSupplierButton);
    auto *invoiceNoEdit = new QLineEdit(invoiceNumber);
    auto *invoiceDateEdit = new QDateEdit(invoiceDate); invoiceDateEdit->setCalendarPopup(true); invoiceDateEdit->setDisplayFormat("dd/MM/yyyy");
    auto *fileLabel = new QLabel(QFileInfo(pdfPath).fileName()); fileLabel->setToolTip(pdfPath); fileLabel->setWordWrap(true);
    form->addRow("Fournisseur *", supplierRow);
    form->addRow("N° de facture *", invoiceNoEdit);
    form->addRow("Date de facture", invoiceDateEdit);
    form->addRow("PDF source", fileLabel);
    root->addLayout(form);

    auto *table = new QTableWidget;
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"ARTICLE / DÉSIGNATION", "LOT", "QUANTITÉ", "PRIX UNITAIRE", "PÉREMPTION (JJ/MM/AAAA)"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setAlternatingRowColors(true);
    table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
    root->addWidget(table,1);

    auto addTableRow = [&](const ParsedInvoiceLine *line) {
        const int row = table->rowCount(); table->insertRow(row);
        const QStringList values = line
            ? QStringList{line->article, line->lot, QString::number(line->quantity, 'g', 10), QString::number(line->unitPrice, 'f', 3), line->expiry.isValid() ? line->expiry.toString("dd/MM/yyyy") : QString()}
            : QStringList{QString(), QStringLiteral("LOT-"), QStringLiteral("1"), QStringLiteral("0.000"), QString()};
        for (int col=0; col<values.size(); ++col) table->setItem(row,col,new QTableWidgetItem(values.at(col)));
    };
    for (const ParsedInvoiceLine &line : parsedLines) addTableRow(&line);
    if (parsedLines.isEmpty()) addTableRow(nullptr);

    auto *lineButtons = new QHBoxLayout;
    auto *addLineButton = featureButton("＋ Ajouter une ligne");
    auto *removeLineButton = featureButton("Retirer les lignes sélectionnées");
    auto *totalLabel = new QLabel;
    totalLabel->setStyleSheet(QStringLiteral("font-size:14px;font-weight:800;color:%1;").arg(kPrimaryDark));
    lineButtons->addWidget(addLineButton); lineButtons->addWidget(removeLineButton); lineButtons->addStretch(1); lineButtons->addWidget(totalLabel);
    root->addLayout(lineButtons);

    auto updateTotal = [&]() {
        double total=0.0;
        for(int r=0;r<table->rowCount();++r) {
            bool qOk=false,pOk=false;
            const double q=table->item(r,2)?cleanNumber(table->item(r,2)->text()).toDouble(&qOk):0.0;
            const double p=table->item(r,3)?cleanNumber(table->item(r,3)->text()).toDouble(&pOk):0.0;
            if(qOk && pOk) total += q*p;
        }
        totalLabel->setText(QString("Total estimé : %1").arg(QLocale(QLocale::French,QLocale::France).toString(total,'f',3)));
    };
    QObject::connect(table, &QTableWidget::itemChanged, &dialog, [&](QTableWidgetItem*) { updateTotal(); });
    QObject::connect(addLineButton, &QPushButton::clicked, &dialog, [&]() { addTableRow(nullptr); updateTotal(); });
    QObject::connect(removeLineButton, &QPushButton::clicked, &dialog, [&]() {
        QList<int> rows; for (const QModelIndex &idx : table->selectionModel()->selectedRows()) rows.append(idx.row());
        std::sort(rows.begin(), rows.end(), std::greater<int>());
        for (int r : rows) table->removeRow(r);
        updateTotal();
    });
    QObject::connect(newSupplierButton, &QPushButton::clicked, &dialog, [&]() {
        if (addSupplier(&dialog)) {
            QString error; const auto refreshed=loadSuppliers(&error);
            if (!error.isEmpty()) { QMessageBox::critical(&dialog,"Fournisseurs",error); return; }
            const int previous=supplierBox->currentData().toInt();
            supplierBox->clear();
            for(const Supplier &s:refreshed) supplierBox->addItem(s.name+(s.email.isEmpty()?QString():" — "+s.email),s.id);
            const int index=supplierBox->findData(previous); if(index>=0) supplierBox->setCurrentIndex(index);
            else if(supplierBox->count()>0) supplierBox->setCurrentIndex(supplierBox->count()-1);
        }
    });
    updateTotal();

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Save)->setText("Valider la facture et mettre à jour le stock");
    buttons->button(QDialogButtonBox::Save)->setStyleSheet(QStringLiteral("background:%1;color:white;border:none;padding:7px 12px;").arg(kPrimary));
    buttons->button(QDialogButtonBox::Cancel)->setText("Annuler");
    root->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (supplierBox->currentIndex()<0 || invoiceNoEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog,"Champs obligatoires","Sélectionnez un fournisseur et renseignez le numéro de facture."); return;
        }
        if (table->rowCount()==0) { QMessageBox::warning(&dialog,"Aucune ligne","Ajoutez au moins un article."); return; }
        double invoiceTotal=0.0;
        for(int r=0;r<table->rowCount();++r) {
            const QString name=table->item(r,0)?table->item(r,0)->text().trimmed():QString();
            const QString lot=table->item(r,1)?table->item(r,1)->text().trimmed():QString();
            bool qOk=false,pOk=false;
            const double qty=table->item(r,2)?cleanNumber(table->item(r,2)->text()).toDouble(&qOk):0.0;
            const double price=table->item(r,3)?cleanNumber(table->item(r,3)->text()).toDouble(&pOk):0.0;
            if(name.isEmpty() || lot.isEmpty() || !qOk || !pOk || qty<=0 || price<0 || std::abs(qty-std::round(qty))>0.0001) {
                QMessageBox::warning(&dialog,"Ligne invalide",QString("Vérifiez l'article, le lot, le prix et la quantité entière (strictement positive) à la ligne %1.").arg(r+1)); return;
            }
            const QString exp=table->item(r,4)?table->item(r,4)->text().trimmed():QString();
            if(exp.isEmpty() || (!findDate(exp).isValid() && !QDate::fromString(exp,"dd/MM/yyyy").isValid())) {
                QMessageBox::warning(&dialog,"Date de péremption requise",QString("Saisissez une date de péremption valide au format JJ/MM/AAAA pour la ligne %1.").arg(r+1)); return;
            }
            invoiceTotal += qty*price;
        }
        dialog.accept();
    });
    if (dialog.exec()!=QDialog::Accepted) return false;

    QSqlDatabase db=QSqlDatabase::database();
    if(!db.isOpen()) { QMessageBox::critical(parent,"Base de données","La connexion Oracle existante n'est pas ouverte."); return false; }
    if(!db.transaction()) { QMessageBox::critical(parent,"Base de données","Impossible de démarrer la transaction :\n"+db.lastError().text()); return false; }

    double invoiceTotal=0.0;
    for(int r=0;r<table->rowCount();++r) invoiceTotal += cleanNumber(table->item(r,2)->text()).toDouble()*cleanNumber(table->item(r,3)->text()).toDouble();
    struct SavedLine { QString name, lot, expiryText; int qty; double price; int articleId; double previousPrice; };
    QVector<SavedLine> savedLines;
    for(int r=0;r<table->rowCount();++r) {
        const QString name=table->item(r,0)->text().trimmed();
        const QString lot=table->item(r,1)->text().trimmed();
        const int qty=static_cast<int>(std::round(cleanNumber(table->item(r,2)->text()).toDouble()));
        const double price=cleanNumber(table->item(r,3)->text()).toDouble();
        const QString expiryText=table->item(r,4)->text().trimmed();
        QDate expiry=QDate::fromString(expiryText,"dd/MM/yyyy"); if(!expiry.isValid()) expiry=findDate(expiryText);
        QSqlQuery existing;
        existing.prepare("SELECT ID_ARTICLE, QUANTITE_DISP FROM C##RAWAND.STOCK WHERE UPPER(NOM_ARTICLE)=UPPER(:name) AND UPPER(LOT)=UPPER(:lot)");
        existing.bindValue(":name",name); existing.bindValue(":lot",lot);
        if(!existing.exec()) { db.rollback(); QMessageBox::critical(parent,"Mise à jour du stock",existing.lastError().text()); return false; }
        int articleId=0; int existingQty=0; const bool found=existing.next();
        if(found) { articleId=existing.value(0).toInt(); existingQty=existing.value(1).toInt(); }
        else {
            QSqlQuery maxQuery("SELECT NVL(MAX(ID_ARTICLE),0)+1 FROM C##RAWAND.STOCK");
            if(!maxQuery.exec()||!maxQuery.next()) { db.rollback(); QMessageBox::critical(parent,"Mise à jour du stock",maxQuery.lastError().text()); return false; }
            articleId=maxQuery.value(0).toInt();
            QSqlQuery insertStock;
            insertStock.prepare("INSERT INTO C##RAWAND.STOCK (ID_ARTICLE, ARTICLE, NOM_ARTICLE, LOT, QUANTITE_DISP, UNITS, SEUIL_MIN, DATE_CHEANCE) VALUES (:id,:article,:name,:lot,:qty,:unit,:seuil,TO_DATE(:expiry,'YYYY-MM-DD'))");
            insertStock.bindValue(":id",articleId); insertStock.bindValue(":article",prefixForCategory("Matériel Clinique")+name); insertStock.bindValue(":name",name); insertStock.bindValue(":lot",lot); insertStock.bindValue(":qty",qty); insertStock.bindValue(":unit",QStringLiteral("unités")); insertStock.bindValue(":seuil",1); insertStock.bindValue(":expiry",expiry.isValid()?expiry.toString("yyyy-MM-dd"):QString());
            if(!insertStock.exec()) { db.rollback(); QMessageBox::critical(parent,"Mise à jour du stock",insertStock.lastError().text()); return false; }
        }
        if(found) {
            QSqlQuery updateStock;
            if(expiry.isValid()) { updateStock.prepare("UPDATE C##RAWAND.STOCK SET QUANTITE_DISP=:qty, DATE_CHEANCE=TO_DATE(:expiry,'YYYY-MM-DD') WHERE ID_ARTICLE=:id"); updateStock.bindValue(":expiry",expiry.toString("yyyy-MM-dd")); }
            else updateStock.prepare("UPDATE C##RAWAND.STOCK SET QUANTITE_DISP=:qty WHERE ID_ARTICLE=:id");
            updateStock.bindValue(":qty",existingQty+qty); updateStock.bindValue(":id",articleId);
            if(!updateStock.exec()) { db.rollback(); QMessageBox::critical(parent,"Mise à jour du stock",updateStock.lastError().text()); return false; }
        }
        double previousPrice=-1.0;
        QSettings settings(localDataFile(),QSettings::IniFormat);
        const int historyCount=settings.beginReadArray("purchaseHistory");
        for(int i=historyCount-1;i>=0;--i) { settings.setArrayIndex(i); if(settings.value("article").toString().compare(name,Qt::CaseInsensitive)==0) { previousPrice=settings.value("price").toDouble(); break; } }
        settings.endArray();
        savedLines.append({name,lot,expiryText,qty,price,articleId,previousPrice});
    }
    if(!db.commit()) { db.rollback(); QMessageBox::critical(parent,"Mise à jour du stock","Échec de validation Oracle :\n"+db.lastError().text()); return false; }

    // The extra module data lives in Qt's local settings file, not in new Oracle tables.
    QSettings settings(localDataFile(),QSettings::IniFormat);
    int oldCount=settings.beginReadArray("purchaseHistory");
    QVector<QVariantMap> oldRows;
    for(int i=0;i<oldCount;++i) { settings.setArrayIndex(i); QVariantMap m; for(const QString &key : QStringList{"date","invoice","supplier","article","lot","quantity","price","previousPrice","source","total"}) m.insert(key,settings.value(key)); oldRows.append(m); }
    settings.endArray();
    const QString supplierName=supplierBox->currentText().section(" — ",0,0);
    for(const SavedLine &line:savedLines) {
        QVariantMap m; m.insert("date",invoiceDateEdit->date().toString("dd/MM/yyyy")); m.insert("invoice",invoiceNoEdit->text().trimmed()); m.insert("supplier",supplierName); m.insert("article",line.name); m.insert("lot",line.lot); m.insert("quantity",QString::number(line.qty)); m.insert("price",QString::number(line.price,'f',3)); m.insert("previousPrice",line.previousPrice<0?QStringLiteral("—"):QString::number(line.previousPrice,'f',3)); m.insert("source",QFileInfo(pdfPath).fileName()); m.insert("total",QString::number(invoiceTotal,'f',3)); oldRows.append(m);
    }
    settings.beginWriteArray("purchaseHistory");
    for(int i=0;i<oldRows.size();++i) { settings.setArrayIndex(i); const QVariantMap m=oldRows.at(i); for(auto it=m.cbegin();it!=m.cend();++it) settings.setValue(it.key(),it.value()); }
    settings.endArray(); settings.sync();
    QMessageBox::information(parent,"Facture validée",QString("La facture %1 a été enregistrée. Le stock, l'historique des achats et les prix ont été mis à jour.").arg(invoiceNoEdit->text().trimmed()));
    return true;
}

void showAlertsDialog(QWidget *parent, const QVector<StockRecord> &items)
{
    struct Alert { StockRecord item; QString kind; QString details; };
    QVector<Alert> alerts;
    const QDate today=QDate::currentDate();
    for(const StockRecord &item:items) {
        const int earlyWarning=qMax(1,item.threshold/5);
        if(item.quantity<=item.threshold+earlyWarning) {
            const QString kind=item.quantity<=item.threshold?QStringLiteral("Stock critique"):QStringLiteral("Seuil proche");
            alerts.append({item,kind,QString("Disponible : %1 %2 ; seuil minimum : %3").arg(item.quantity).arg(item.unit).arg(item.threshold)});
        }
        if(item.expiry.isValid() && item.expiry<=today.addDays(30)) {
            const int days=today.daysTo(item.expiry);
            const QString kind=days<0?QStringLiteral("Périmé"):days==0?QStringLiteral("Périme aujourd'hui"):QStringLiteral("Péremption proche");
            const QString details=days<0?QString("Périmé depuis %1 jour(s), date : %2").arg(-days).arg(item.expiry.toString("dd/MM/yyyy")):
                QString("Échéance dans %1 jour(s), date : %2").arg(days).arg(item.expiry.toString("dd/MM/yyyy"));
            alerts.append({item,kind,details});
        }
    }

    QDialog dialog(parent);
    dialog.setWindowTitle("Alertes de stock et de péremption");
    dialog.resize(960,560);
    applyFeatureDialogStyle(dialog);
    auto *root=new QVBoxLayout(&dialog);
    auto *title=new QLabel(QString("Alertes actives — %1").arg(alerts.size()));
    title->setStyleSheet(QStringLiteral("font-size:20px;font-weight:800;color:%1;").arg(kPrimaryDark)); root->addWidget(title);
    auto *sub=new QLabel("Les alertes couvrent le seuil minimum, le stock critique, les lots expirés et les lots dont la date d'échéance approche sous 30 jours.");
    sub->setWordWrap(true); sub->setStyleSheet(QStringLiteral("color:%1;").arg(kMuted)); root->addWidget(sub);
    auto *table=new QTableWidget; table->setColumnCount(5); table->setHorizontalHeaderLabels({"ALERTE","ARTICLE","LOT","STOCK","DÉTAILS"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows); table->setSelectionMode(QAbstractItemView::ExtendedSelection); table->setEditTriggers(QAbstractItemView::NoEditTriggers); table->setAlternatingRowColors(true);
    for(int i=0;i<alerts.size();++i) {
        const Alert &a=alerts.at(i); const int row=table->rowCount(); table->insertRow(row);
        QStringList values={a.kind,a.item.article,a.item.lot,QString::number(a.item.quantity)+" "+a.item.unit,a.details};
        for(int c=0;c<values.size();++c) { auto *cell=new QTableWidgetItem(values.at(c)); if(c==0) { cell->setData(Qt::UserRole,i); cell->setForeground(QColor(a.kind.contains("Périm")?"#B94A48":a.kind.contains("critique")?"#B94A48":"#C87920")); } table->setItem(row,c,cell); }
    }
    if(alerts.isEmpty()) { table->setRowCount(1); auto *empty=new QTableWidgetItem("Aucune alerte active. Le stock et les échéances sont sous contrôle."); empty->setTextAlignment(Qt::AlignCenter); table->setItem(0,0,empty); table->setSpan(0,0,1,5); }
    root->addWidget(table,1);
    auto *buttonsRow=new QHBoxLayout; auto *notify=featureButton("✉ Notifier local / fournisseur",true); auto *close=featureButton("Fermer"); buttonsRow->addStretch(1); buttonsRow->addWidget(notify); buttonsRow->addWidget(close); root->addLayout(buttonsRow);
    QObject::connect(notify,&QPushButton::clicked,&dialog,[&]() {
        QList<int> selected; for(const QModelIndex &idx:table->selectionModel()->selectedRows()) { if(table->item(idx.row(),0) && table->item(idx.row(),0)->data(Qt::UserRole).isValid()) selected.append(table->item(idx.row(),0)->data(Qt::UserRole).toInt()); }
        if(selected.isEmpty()) { QMessageBox::information(&dialog,"Sélection requise","Sélectionnez une ou plusieurs alertes à notifier."); return; }
        bool ok=false; const QString email=QInputDialog::getText(&dialog,"Notification","Adresse e-mail du dépôt/local ou du fournisseur :",QLineEdit::Normal,QString(),&ok).trimmed();
        if(!ok) return;
        if(!QRegularExpression(QStringLiteral("^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$")).match(email).hasMatch()) { QMessageBox::warning(&dialog,"E-mail invalide","Saisissez une adresse e-mail valide."); return; }
        QStringList lines;
        for(int index:selected) { if(index<0||index>=alerts.size()) continue; const Alert &a=alerts.at(index); lines<<QString("• %1 — %2 (lot %3) : %4").arg(a.kind,a.item.article,a.item.lot,a.details); }
        QUrl url(QStringLiteral("mailto:")+email); QUrlQuery query; query.addQueryItem("subject","ANIWORLD — Alerte stock / péremption"); query.addQueryItem("body",QString("Bonjour,\n\nMerci de traiter les alertes suivantes :\n\n%1\n\nCordialement,\nANIWORLD").arg(lines.join("\n"))); url.setQuery(query);
        if(!QDesktopServices::openUrl(url)) QMessageBox::warning(&dialog,"E-mail","Impossible d'ouvrir le client e-mail configuré sur cet ordinateur.");
        else QMessageBox::information(&dialog,"Notification préparée","Un nouveau message a été préparé dans votre application e-mail. Vérifiez-le puis envoyez-le.");
    });
    QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);
    dialog.exec();
}

void showSmartReplenishmentDialog(QWidget *parent, const QVector<StockRecord> &items)
{
    QString supplierError; QVector<Supplier> suppliers=loadSuppliers(&supplierError);
    if(!supplierError.isEmpty()) { QMessageBox::critical(parent,"Réapprovisionnement",supplierError+"\n\n"+kLocalDataHint); return; }
    if(suppliers.isEmpty()) { QMessageBox::information(parent,"Fournisseur requis","Ajoutez d'abord un fournisseur dans le module Factures d'achat."); return; }
    QDialog dialog(parent); dialog.setWindowTitle("Réapprovisionnement intelligent"); dialog.resize(1100,650); applyFeatureDialogStyle(dialog);
    auto *root=new QVBoxLayout(&dialog); auto *title=new QLabel("Réapprovisionnement intelligent"); title->setStyleSheet(QStringLiteral("font-size:20px;font-weight:800;color:%1;").arg(kPrimaryDark)); root->addWidget(title);
    auto *subtitle=new QLabel("La recommandation estime la consommation moyenne à partir des sorties enregistrées, du délai de livraison et du stock de sécurité. Vous pouvez ajuster les quantités avant de générer le bon de commande PDF."); subtitle->setWordWrap(true); subtitle->setStyleSheet(QStringLiteral("color:%1;").arg(kMuted)); root->addWidget(subtitle);
    auto *form=new QFormLayout; auto *supplierBox=new QComboBox; for(const Supplier &s:suppliers) supplierBox->addItem(s.name,s.id);
    auto *periodSpin=new QSpinBox; periodSpin->setRange(7,365); periodSpin->setValue(30); periodSpin->setSuffix(" jours d'historique");
    auto *coverageSpin=new QSpinBox; coverageSpin->setRange(1,180); coverageSpin->setValue(14); coverageSpin->setSuffix(" jours");
    auto *safetySpin=new QSpinBox; safetySpin->setRange(0,90); safetySpin->setValue(7); safetySpin->setSuffix(" jours");
    auto *leadLabel=new QLabel; leadLabel->setStyleSheet(QStringLiteral("font-weight:700;color:%1;").arg(kPrimaryDark));
    auto supplierLead=[&]() { int id=supplierBox->currentData().toInt(); for(const Supplier &s:suppliers) if(s.id==id) return s.leadDays; return 3; };
    auto updateLead=[&]() { leadLabel->setText(QString::number(supplierLead())+" jour(s)"); };
    form->addRow("Fournisseur *",supplierBox); form->addRow("Période d'analyse",periodSpin); form->addRow("Couverture cible",coverageSpin); form->addRow("Stock de sécurité",safetySpin); form->addRow("Délai fournisseur",leadLabel); root->addLayout(form);
    auto *table=new QTableWidget; table->setColumnCount(7); table->setHorizontalHeaderLabels({"COMMANDER","ARTICLE / LOT","STOCK ACTUEL","CONSOMMATION / JOUR","DÉLAI","STOCK SÉCURITÉ","QTÉ À COMMANDER"}); table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); table->verticalHeader()->setVisible(false); table->setSelectionBehavior(QAbstractItemView::SelectRows); table->setAlternatingRowColors(true); root->addWidget(table,1);
    auto *hint=new QLabel; hint->setWordWrap(true); hint->setStyleSheet(QStringLiteral("color:%1;").arg(kMuted)); root->addWidget(hint);

    auto recalculate=[&]() {
        table->setRowCount(0); bool movementAvailable=true;
        const int period=periodSpin->value(); const int lead=supplierLead(); const int coverage=coverageSpin->value(); const int safeDays=safetySpin->value();
        for(const StockRecord &item:items) {
            double perDay=0.0;
            QSettings movementSettings(localDataFile(), QSettings::IniFormat);
            const int movementCount = movementSettings.beginReadArray("stockMovements");
            const QDate cutoff = QDate::currentDate().addDays(-period);
            bool hasRecentOutput = false;
            for (int i = 0; i < movementCount; ++i) {
                movementSettings.setArrayIndex(i);
                if (movementSettings.value("articleId").toInt() != item.id || movementSettings.value("type").toString() != "SORTIE") continue;
                const QDate movementDate = QDate::fromString(movementSettings.value("date").toString(), Qt::ISODate);
                if (movementDate.isValid() && movementDate >= cutoff && movementDate <= QDate::currentDate()) {
                    perDay += movementSettings.value("quantity").toDouble();
                    hasRecentOutput = true;
                }
            }
            movementSettings.endArray();
            if (!hasRecentOutput) movementAvailable = false;
            perDay /= period;
            const int safety=qMax(item.threshold,static_cast<int>(std::ceil(perDay*safeDays)));
            const int target=qMax(item.threshold*2,static_cast<int>(std::ceil(perDay*(lead+coverage)))+safety);
            const int order=qMax(0,target-item.quantity);
            const int row=table->rowCount(); table->insertRow(row);
            auto *check=new QTableWidgetItem; check->setFlags((check->flags()|Qt::ItemIsUserCheckable)&~Qt::ItemIsEditable); check->setCheckState(order>0?Qt::Checked:Qt::Unchecked); check->setData(Qt::UserRole,item.id); table->setItem(row,0,check);
            auto *article=new QTableWidgetItem(item.article+" — "+item.lot); article->setData(Qt::UserRole,item.id); article->setFlags(article->flags()&~Qt::ItemIsEditable); table->setItem(row,1,article);
            QStringList rest={QString::number(item.quantity)+" "+item.unit,QString::number(perDay,'f',2),QString::number(lead)+" j",QString::number(safety),QString::number(order)};
            for(int c=0;c<rest.size();++c) { auto *cell=new QTableWidgetItem(rest.at(c)); if(c<rest.size()-1) cell->setFlags(cell->flags()&~Qt::ItemIsEditable); table->setItem(row,c+2,cell); }
        }
        hint->setText(movementAvailable
            ? "Calcul : consommation moyenne locale × (délai + couverture) + stock de sécurité. Les sorties validées dans Qt sont utilisées pour estimer la consommation."
            : "Calcul : consommation moyenne locale × (délai + couverture) + stock de sécurité. Sans historique de sorties, la consommation est estimée à zéro et le seuil minimum reste le garde-fou.");
    };
    updateLead(); recalculate();
    QObject::connect(supplierBox,qOverload<int>(&QComboBox::currentIndexChanged),&dialog,[&](int){ updateLead(); recalculate(); });
    QObject::connect(periodSpin,qOverload<int>(&QSpinBox::valueChanged),&dialog,[&](int){ recalculate(); });
    QObject::connect(coverageSpin,qOverload<int>(&QSpinBox::valueChanged),&dialog,[&](int){ recalculate(); });
    QObject::connect(safetySpin,qOverload<int>(&QSpinBox::valueChanged),&dialog,[&](int){ recalculate(); });

    auto *buttonsRow=new QHBoxLayout; auto *recalcButton=featureButton("↻ Recalculer"); auto *pdfButton=featureButton("Générer le bon de commande PDF",true); auto *close=featureButton("Fermer"); buttonsRow->addWidget(recalcButton); buttonsRow->addStretch(1); buttonsRow->addWidget(pdfButton); buttonsRow->addWidget(close); root->addLayout(buttonsRow);
    QObject::connect(recalcButton,&QPushButton::clicked,&dialog,[&](){ recalculate(); });
    QObject::connect(pdfButton,&QPushButton::clicked,&dialog,[&]() {
        QVector<QStringList> orderLines; double totalUnits=0.0;
        for(int row=0;row<table->rowCount();++row) {
            if(!table->item(row,0)||table->item(row,0)->checkState()!=Qt::Checked) continue;
            bool ok=false; const double qty=table->item(row,6)?cleanNumber(table->item(row,6)->text()).toDouble(&ok):0.0;
            if(!ok||qty<=0) continue;
            const QString itemLabel=table->item(row,1)?table->item(row,1)->text():QString();
            orderLines.append({itemLabel,QString::number(static_cast<int>(std::ceil(qty))),table->item(row,4)?table->item(row,4)->text():QString()}); totalUnits+=qty;
        }
        if(orderLines.isEmpty()) { QMessageBox::information(&dialog,"Aucune quantité","Cochez les articles à commander et indiquez une quantité supérieure à zéro."); return; }
        QString supplierName=supplierBox->currentText(); QString supplierEmail;
        for(const Supplier &s:suppliers) if(s.id==supplierBox->currentData().toInt()) { supplierName=s.name; supplierEmail=s.email; break; }
        const QString defaultName=QString("Bon_commande_%1.pdf").arg(QDate::currentDate().toString("yyyyMMdd"));
        const QString path=QFileDialog::getSaveFileName(&dialog,"Enregistrer le bon de commande",QDir::home().filePath(defaultName),"Document PDF (*.pdf)");
        if(path.isEmpty()) return;
        QString outputPath=path; if(!outputPath.endsWith(".pdf",Qt::CaseInsensitive)) outputPath+=".pdf";
        QString html=QStringLiteral("<html><head><meta charset='utf-8'><style>body{font-family:Arial;color:#263B42;font-size:10pt}h1{color:#225A64;font-size:20pt} .banner{background:#397F8D;color:#fff;padding:12px}table{width:100%;border-collapse:collapse;margin-top:20px}th{background:#397F8D;color:white;padding:8px;text-align:left}td{border:1px solid #D8E9EC;padding:8px}tr:nth-child(even){background:#F3F8F9}.muted{color:#71878D}</style></head><body>");
        html+=QStringLiteral("<div class='banner'><h1 style='color:white'>ANIWORLD — BON DE COMMANDE</h1></div><p><b>Fournisseur :</b> %1<br><b>E-mail :</b> %2<br><b>Date :</b> %3<br><b>Référence :</b> BC-%4</p>").arg(supplierName.toHtmlEscaped(),supplierEmail.toHtmlEscaped(),QDate::currentDate().toString("dd/MM/yyyy"),QDateTime::currentDateTime().toString("yyyyMMdd-hhmm"));
        html+=QStringLiteral("<table><thead><tr><th>Article / lot</th><th>Quantité commandée</th><th>Délai estimé</th></tr></thead><tbody>");
        for(const QStringList &line:orderLines) html+=QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(line.value(0).toHtmlEscaped(),line.value(1).toHtmlEscaped(),line.value(2).toHtmlEscaped());
        html+=QStringLiteral("</tbody></table><p class='muted'>Merci de confirmer la disponibilité et la date de livraison avant expédition.</p><p><b>Total unités demandées :</b> %1</p></body></html>").arg(QString::number(static_cast<int>(totalUnits)));
        QPrinter printer(QPrinter::HighResolution); printer.setOutputFormat(QPrinter::PdfFormat); printer.setOutputFileName(outputPath); printer.setPageSize(QPageSize(QPageSize::A4)); printer.setPageMargins(QMarginsF(14,14,14,14),QPageLayout::Millimeter);
        QTextDocument document; document.setHtml(html); document.print(&printer);
        if(!QFileInfo::exists(outputPath)||QFileInfo(outputPath).size()==0) { QMessageBox::critical(&dialog,"PDF","Le bon de commande n'a pas pu être généré."); return; }
        QMessageBox::information(&dialog,"Bon de commande prêt",QString("Le PDF a été créé :\n%1").arg(outputPath));
        QDesktopServices::openUrl(QUrl::fromLocalFile(outputPath));
    });
    QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);
    dialog.exec();
}
} // namespace

bool InventoryFeatures::showPurchases(QWidget *parent, const QVector<StockRecord> &items)
{
    bool stockChanged=false;
    QDialog dialog(parent); dialog.setWindowTitle("Factures d'achat et fournisseurs"); dialog.setMinimumWidth(520); applyFeatureDialogStyle(dialog);
    auto *root=new QVBoxLayout(&dialog); auto *title=new QLabel("Achats et fournisseurs"); title->setStyleSheet(QStringLiteral("font-size:22px;font-weight:800;color:%1;").arg(kPrimaryDark)); root->addWidget(title);
    auto *sub=new QLabel("Importez une facture PDF, contrôlez les lignes extraites, puis suivez l'historique et l'évolution des prix d'achat."); sub->setWordWrap(true); sub->setStyleSheet(QStringLiteral("color:%1;").arg(kMuted)); root->addWidget(sub);
    auto *import=featureButton("⇧ Importer une facture PDF",true); auto *history=featureButton("▤ Historique des achats / prix"); auto *suppliersButton=featureButton("♙ Gérer les fournisseurs"); auto *close=featureButton("Fermer");
    root->addSpacing(10); root->addWidget(import); root->addWidget(history); root->addWidget(suppliersButton); root->addWidget(close);
    QObject::connect(import,&QPushButton::clicked,&dialog,[&]() { dialog.accept(); stockChanged=importInvoice(parent,items); });
    QObject::connect(history,&QPushButton::clicked,&dialog,[&]() { dialog.accept(); showPurchaseHistory(parent); });
    QObject::connect(suppliersButton,&QPushButton::clicked,&dialog,[&]() { dialog.accept(); showSupplierManager(parent); });
    QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);
    dialog.exec();
    return stockChanged;
}

void InventoryFeatures::showAlerts(QWidget *parent, const QVector<StockRecord> &items)
{
    showAlertsDialog(parent,items);
}

void InventoryFeatures::showSmartReplenishment(QWidget *parent, const QVector<StockRecord> &items)
{
    showSmartReplenishmentDialog(parent,items);
}
