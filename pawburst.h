#pragma once
#include <QList>
#include <QPointF>
#include <QTimer>
#include <QWidget>

// Animation unique ANIWORLD : chaque clic fait s'envoler une petite patte.
// C'est un calque transparent pose au-dessus de toute la fenetre.
class PawBurst : public QWidget
{
    Q_OBJECT
public:
    explicit PawBurst(QWidget *parent);
    void spawn(const QPoint &pos);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    struct Paw {
        QPointF pos;
        qreal rot;
        qreal t;
    };
    void tick();

    QList<Paw> m_paws;
    QTimer m_timer;
};
