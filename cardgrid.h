#pragma once
#include <QList>
#include <QWidget>

class RoomCard;

// Grille de cartes qui se reorganise toute seule selon la largeur disponible
class CardGrid : public QWidget
{
    Q_OBJECT
public:
    explicit CardGrid(QWidget *parent = nullptr);

    void setCards(const QList<RoomCard *> &cards);   // la grille prend possession des cartes
    const QList<RoomCard *> &cards() const { return m_cards; }

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    void clearCards();
    void relayout();

    QList<RoomCard *> m_cards;
    int m_cols = 0;
};
