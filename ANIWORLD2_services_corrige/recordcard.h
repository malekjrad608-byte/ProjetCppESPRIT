#ifndef RECORDCARD_H
#define RECORDCARD_H

#include <QFrame>
#include <QList>
#include <QStringList>
#include "medicalrecord.h"
#include "healthwatch.h"

class QHBoxLayout;

// Score de risque santé (0 à 100) calculé avec des règles simples et expliquées (aide à la décision, à valider par le vétérinaire).
struct RiskResult
{
    enum Level { Faible, Moyen, Eleve };
    int score = 0;
    Level level = Faible;
    QStringList raisons;          // ex. "Vaccin à renouveler (+25)"
    QString libelle() const;      // "Faible" / "Moyen" / "Élevé"
};

class RiskScore
{
public:
    // 'alertes' = résultat de HealthWatch::analyser (récidive / foyer concernant l'animal)
    static RiskResult calculer(const MedicalRecord &r, const QList<HealthAlert> &alertes);
};

class RecordCard : public QFrame
{
    Q_OBJECT

public:
    explicit RecordCard(const MedicalRecord &record, QWidget *parent = nullptr);

    QString recordId() const { return m_recordId; }
    void setSelected(bool selected);
    void setRisk(const RiskResult &risk);       // ajoute le badge "Risque" sur la carte

signals:
    void clicked(const QString &recordId);
    void doubleClicked(const QString &recordId);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QString m_recordId;
    QHBoxLayout *m_top = nullptr;
};

#endif // RECORDCARD_H
