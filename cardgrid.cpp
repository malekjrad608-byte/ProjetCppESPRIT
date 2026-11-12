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
    const int cw = RoomCard::CardWidth;
    const int ch = RoomCard::CardHeight;
    m_cols = qMax(1, width() / cw);
    for (int i = 0; i < m_cards.size(); ++i) {
        m_cards[i]->move((i % m_cols) * cw, (i / m_cols) * ch);
        m_cards[i]->show();
    }
    const int rows = (m_cards.size() + m_cols - 1) / m_cols;
    setMinimumHeight(rows * ch);
}

void CardGrid::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    const int cols = qMax(1, width() / RoomCard::CardWidth);
    if (cols != m_cols)
        relayout();
}
