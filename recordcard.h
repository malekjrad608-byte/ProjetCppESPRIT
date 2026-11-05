#ifndef RECORDCARD_H
#define RECORDCARD_H

#include <QFrame>
#include "medicalrecord.h"

class RecordCard : public QFrame
{
    Q_OBJECT

public:
    explicit RecordCard(const MedicalRecord &record, QWidget *parent = nullptr);

    QString recordId() const { return m_recordId; }
    void setSelected(bool selected);

signals:
    void clicked(const QString &recordId);
    void doubleClicked(const QString &recordId);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QString m_recordId;
};

#endif // RECORDCARD_H
