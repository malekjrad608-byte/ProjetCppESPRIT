#include "groomingdialog.h"
#include "lookupdata.h"

#include <QComboBox>
#include <QListWidget>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QPixmap>
#include <QIcon>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSignalBlocker>
#include <utility>

GroomingDialog::GroomingDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Coupes et toilettage");
    resize(1020, 640);

    auto *root = new QVBoxLayout(this);

    auto *title = new QLabel(QString::fromUtf8("✂  Choisir une coupe"));
    title->setObjectName("cardTitle");
    root->addWidget(title);

    m_animalLabel = new QLabel();
    m_animalLabel->setObjectName("panelMeta");
    root->addWidget(m_animalLabel);

    auto *filters = new QHBoxLayout();
    m_species = new QComboBox();
    m_breed = new QComboBox();
    filters->addWidget(new QLabel("Espèce :"));
    filters->addWidget(m_species);
    filters->addWidget(new QLabel("Race :"));
    filters->addWidget(m_breed);
    filters->addStretch();
    root->addLayout(filters);

    const QMap<QString, QStringList> races = LookupData::racesParEspece();
    for (auto it = races.cbegin(); it != races.cend(); ++it)
        m_species->addItem(it.key());
    m_species->setCurrentIndex(-1);

    auto *body = new QHBoxLayout();
    m_list = new QListWidget();
    m_list->setIconSize(QSize(64, 48));
    m_list->setMinimumWidth(280);
    m_list->setMaximumWidth(320);
    body->addWidget(m_list);

    auto *right = new QVBoxLayout();
    auto *photos = new QHBoxLayout();

    auto *colModel = new QVBoxLayout();
    m_modelPhoto = new QLabel();
    m_modelPhoto->setFixedSize(300, 200);
    m_modelPhoto->setAlignment(Qt::AlignCenter);
    colModel->addWidget(new QLabel("Modèle de la coupe"));
    colModel->addWidget(m_modelPhoto);
    m_photoHint = new QLabel();
    m_photoHint->setWordWrap(true);
    m_photoHint->setMaximumWidth(300);
    m_photoHint->setStyleSheet("color:#B9781A; font-size:11px;");
    m_photoHint->hide();
    colModel->addWidget(m_photoHint);
    photos->addLayout(colModel);

    auto *colAnimal = new QVBoxLayout();
    m_animalPhoto = new QLabel("Aucune photo");
    m_animalPhoto->setFixedSize(300, 200);
    m_animalPhoto->setAlignment(Qt::AlignCenter);
    m_animalPhoto->setStyleSheet("border: 2px dashed #8FC7D2; border-radius: 12px; color: #5B7F8A;");
    m_btnPhoto = new QPushButton(QString::fromUtf8("📷  Photo de l'animal…"));
    m_btnPhoto->setObjectName("outlineButton");
    colAnimal->addWidget(new QLabel("Votre animal"));
    colAnimal->addWidget(m_animalPhoto);
    colAnimal->addWidget(m_btnPhoto);
    photos->addLayout(colAnimal);
    photos->addStretch();
    right->addLayout(photos);

    m_title = new QLabel();
    m_title->setObjectName("cardTitle");
    right->addWidget(m_title);
    m_info = new QLabel();
    m_info->setWordWrap(true);
    right->addWidget(m_info);
    m_warning = new QLabel();
    m_warning->setWordWrap(true);
    m_warning->setStyleSheet("background:#FDECEA; color:#B42318; border-radius:8px; padding:8px; font-weight:700;");
    m_warning->hide();
    right->addWidget(m_warning);
    m_details = new QTextBrowser();
    right->addWidget(m_details, 1);

    body->addLayout(right, 1);
    root->addLayout(body, 1);

    auto *bottom = new QHBoxLayout();
    bottom->addStretch();
    auto *btnClose = new QPushButton("Fermer");
    btnClose->setObjectName("outlineButton");
    bottom->addWidget(btnClose);
    root->addLayout(bottom);

    connect(m_species, &QComboBox::currentIndexChanged, this, &GroomingDialog::onSpeciesChanged);
    connect(m_breed, &QComboBox::currentIndexChanged, this, &GroomingDialog::refreshList);
    connect(m_list, &QListWidget::currentRowChanged, this, &GroomingDialog::onCutSelected);
    connect(m_btnPhoto, &QPushButton::clicked, this, &GroomingDialog::onChoosePhoto);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);

    refreshList();
}

void GroomingDialog::setAnimal(const QString &nom, const QString &espece, const QString &race)
{
    m_animalName = nom;
    m_animalLabel->setText(nom.isEmpty() ? QString("Choisissez l'espèce et la race pour voir les coupes adaptées.")
                                         : QString("Animal : %1 · %2 · %3").arg(nom, espece, race));
    const int si = m_species->findText(espece);
    m_userPhotoLoaded = false;
    m_species->setCurrentIndex(si);          // remplit les races (onSpeciesChanged)
    if (si < 0 || m_species->currentText() == espece)
        showSpeciesPhoto();
    const int bi = m_breed->findText(race);
    if (bi >= 0)
        m_breed->setCurrentIndex(bi);
}

void GroomingDialog::onSpeciesChanged()
{
    QSignalBlocker block(m_breed);
    m_breed->clear();
    const QStringList races = LookupData::racesParEspece().value(m_species->currentText());
    m_breed->addItems(races);
    m_breed->setCurrentIndex(-1);
    block.unblock();
    m_userPhotoLoaded = false;
    showSpeciesPhoto();
    refreshList();
}

void GroomingDialog::showSpeciesPhoto()
{
    if (m_userPhotoLoaded) return;

    const QString species = m_species->currentText().toLower();
    QString prefix;
    QString fallback;
    if (species.contains("chien")) { prefix = "chien"; fallback = "animal_chien.jpg"; }
    else if (species.contains("chat")) { prefix = "chat"; fallback = "animal_chat.jpg"; }
    else if (species.contains("lapin")) { prefix = "lapin"; fallback = "animal_lapin.jpg"; }
    else if (species.contains("oiseau")) { prefix = "oiseau"; fallback = "animal_oiseau.jpg"; }

    if (fallback.isEmpty()) {
        m_animalPhoto->setPixmap(QPixmap());
        m_animalPhoto->setText("Choisissez une espèce");
        return;
    }

    // Chercher d'abord un portrait dédié à la race, puis la photo représentative
    // de l'espèce. Les deux sont intégrés aux ressources Qt.
    QString race = m_breed->currentText().toLower();
    race.replace("é", "e"); race.replace("è", "e"); race.replace("ê", "e");
    race.replace("à", "a"); race.replace("ï", "i"); race.replace("ô", "o");
    race.replace("û", "u"); race.replace("ü", "u");
    race.replace(" ", "_"); race.replace("-", "_");
    race.remove('('); race.remove(')'); race.remove('\'');
    QStringList candidates;
    if (!race.isEmpty()) candidates << QString(":/med/coupes/races/%1_%2.jpg").arg(prefix, race);
    candidates << QString(":/med/coupes/%1").arg(fallback);
    QPixmap photo;
    for (const QString &candidate : candidates) {
        photo.load(candidate);
        if (!photo.isNull()) break;
    }
    if (photo.isNull()) {
        const QStringList paths = {
            QCoreApplication::applicationDirPath() + "/coupes/" + fallback,
            QDir::currentPath() + "/coupes/" + fallback,
            QCoreApplication::applicationDirPath() + "/../coupes/" + fallback
        };
        for (const QString &path : paths) {
            if (QFileInfo::exists(path)) {
                photo.load(path);
                if (!photo.isNull()) break;
            }
        }
    }
    if (!photo.isNull()) {
        m_animalPhoto->setText(QString());
        m_animalPhoto->setPixmap(photo.scaled(m_animalPhoto->size() - QSize(6, 6), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        return;
    }
    m_animalPhoto->setPixmap(QPixmap());
    m_animalPhoto->setText("Aucune photo disponible");
}

void GroomingDialog::refreshList()
{
    showSpeciesPhoto();
    const int previous = m_list->currentRow();
    const QString previousId = (previous >= 0 && previous < m_cuts.size()) ? m_cuts[previous].id : QString();

    m_list->clear();
    m_cuts.clear();
    const QString species = m_species->currentIndex() >= 0 ? m_species->currentText() : QString();
    if (species.isEmpty()) {                 // aucune espèce choisie : on ne montre aucune coupe
        onCutSelected();
        return;
    }

    // seulement les coupes de cette espèce (et de cette race si elle est choisie)
    const QString race = m_breed->currentIndex() >= 0 ? m_breed->currentText() : QString();
    m_cuts = GroomingCatalog::pour(species, race);
    int select = 0;
    for (int i = 0; i < m_cuts.size(); ++i) {
        const GroomingCut &c = m_cuts[i];
        auto *item = new QListWidgetItem(QIcon(GroomingCatalog::image(c, QSize(64, 48), species, race)),
                                         GroomingCatalog::deconseillee(c, race)
                                             ? c.nom + QString::fromUtf8("  ⚠") : c.nom);
        m_list->addItem(item);
        if (c.id == previousId)
            select = i;                      // garde la coupe choisie quand on change de race
    }
    if (!m_cuts.isEmpty())
        m_list->setCurrentRow(select);
    else
        onCutSelected();
}

void GroomingDialog::onCutSelected()
{
    const int row = m_list->currentRow();
    if (row < 0 || row >= m_cuts.size()) {
        m_modelPhoto->clear();
        m_title->clear();
        m_info->clear();
        m_warning->hide();
        m_photoHint->hide();
        m_details->setHtml(m_species->currentIndex() < 0
            ? "<p>Choisissez d'abord l'<b>espèce</b> (et la race) de l'animal : seules les coupes adaptées s'afficheront.</p>"
            : "<p>Aucune coupe disponible pour cette sélection.</p>");
        return;
    }
    showCut(m_cuts[row]);
}

void GroomingDialog::showCut(const GroomingCut &c)
{
    const QString species = m_species->currentIndex() >= 0 ? m_species->currentText() : QString();
    const QString breed = m_breed->currentIndex() >= 0 ? m_breed->currentText() : QString();
    m_modelPhoto->setPixmap(GroomingCatalog::image(c, m_modelPhoto->size(), species, breed));
    QString missing;
    if (!GroomingCatalog::photoSpecifique(c, species, breed, &missing)) {
        m_photoHint->setText(QString("Photo de cette coupe pour « %1 » pas encore ajoutée (fichier : %2). "
                                     "Photo de l'espèce affichée en attendant.").arg(species, missing));
        m_photoHint->show();
    } else {
        m_photoHint->hide();
    }
    m_title->setText(c.nom);
    m_info->setText(QString("%1\nDurée : %2   ·   Difficulté : %3").arg(c.description, c.duree, c.difficulte));

    const QString race = breed;
    if (GroomingCatalog::deconseillee(c, race)) {
        m_warning->setText(QString::fromUtf8("⚠ Cette coupe est déconseillée pour la race « %1 » (voir précautions).").arg(race));
        m_warning->show();
    } else {
        m_warning->hide();
    }

    auto li = [](const QStringList &l, bool ordered) {
        QString h = ordered ? "<ol>" : "<ul>";
        for (const QString &s : l) h += "<li>" + s.toHtmlEscaped() + "</li>";
        return h + (ordered ? "</ol>" : "</ul>");
    };
    QString html;
    html += "<h3>Résultat attendu</h3><p>" + c.rendu.toHtmlEscaped() + "</p>";
    html += "<h3>Comment faire</h3>" + li(c.etapes, true);
    html += "<h3>Matériel</h3>" + li(c.materiel, false);
    html += "<h3>Précautions</h3>" + li(c.precautions, false);
    m_details->setHtml(html);
}

void GroomingDialog::onChoosePhoto()
{
    const QString path = QFileDialog::getOpenFileName(this, "Photo de l'animal", QString(),
                                                      "Images (*.png *.jpg *.jpeg *.webp)");
    if (path.isEmpty())
        return;
    const QPixmap p(path);
    if (p.isNull())
        return;
    m_userPhotoLoaded = true;
    m_animalPhoto->setText(QString());
    m_animalPhoto->setPixmap(p.scaled(m_animalPhoto->size() - QSize(6, 6), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
