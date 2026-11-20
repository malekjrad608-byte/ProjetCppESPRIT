#pragma once
#include <QWidget>
#include <QMap>
#include "../models/service.h"
class ServiceTimetable : public QWidget {
    Q_OBJECT
public:
    struct Block {QRectF rect;int entry;int day;};
    explicit ServiceTimetable(QWidget *parent=nullptr);
    void setAppointments(QDate monday,const QList<Service::Appointment> &entries,const QMap<int,QString> &names);
    void selectEntry(int index);
    QList<Block> blocks() const;
    int firstHour() const;
    static constexpr int HourHeight=48;
signals:
    void entrySelected(int index);
    void entryActivated(int index);
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
private:
    int entryAt(QPointF point) const;
    QDate m_monday;
    QList<Service::Appointment> m_entries;
    QMap<int,QString> m_names;
    int m_selected=-1;
};
