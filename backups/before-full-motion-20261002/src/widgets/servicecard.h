#pragma once
#include <QAbstractButton>
#include <QVariantAnimation>
#include "../models/service.h"

class ServiceCard : public QAbstractButton {
public:
    explicit ServiceCard(const Service &service, QWidget *parent = nullptr);
protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *) override;
private:
    Service m_service;
    QVariantAnimation *m_hoverAnimation;
    qreal m_hoverProgress = 0;
    bool m_hovered = false;
};
