#pragma once
#include <QAbstractButton>
#include "../models/service.h"

class ServiceCard : public QAbstractButton {
public:
    explicit ServiceCard(const Service &service, QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *) override;
private:
    Service m_service;
};
