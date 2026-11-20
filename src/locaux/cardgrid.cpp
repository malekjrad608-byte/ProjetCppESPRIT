#include "cardgrid.h"
#include "roomcard.h"

CardGrid::CardGrid(QWidget *parent) : QWidget(parent)
{
}

void CardGrid::clearCards()
{
    for (RoomCard *c : m_cards) {
        c->hide();
        c->deleteLater();
    }
    m_cards.clear();
}

void CardGrid::setCards(const QList<RoomCard *> &cards)
{
    clearCards();
    m_cards = cards;
    for (RoomCard *c : m_cards)
        c->setParent(this);
    relayout();
}

void CardGrid::relayout()
{
    m_cols = qBound(1, width() / RoomCard::CardWidth, 2);
    const int cw = width() / m_cols;
    const int ch = RoomCard::CardHeight;
    for (int i = 0; i < m_cards.size(); ++i) {
        m_cards[i]->setFixedWidth(cw);
        m_cards[i]->move((i % m_cols) * cw, (i / m_cols) * ch);
        m_cards[i]->show();
    }
    const int rows = (m_cards.size() + m_cols - 1) / m_cols;
    setMinimumHeight(rows * ch);
}

void CardGrid::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    relayout();
}
