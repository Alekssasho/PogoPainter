//
//  PPBonus.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/8/14.
//
//

#include "PPBonus.h"
#include "PPPlayer.h"
#include "PPBoard.h"

void PPCheckpoint::apply(PPPlayer& player, PPBoard& board)
{
    board.each([&](PPCell & cell) {
        if (cell.color == player.color) {
            cell.color = PPColor::Empty;
            ++player.points;
            //TODO: Animate points
        }
    });
}

void PPBonus::update(PPBoard& board)
{

}

void PPArrow::update(PPBoard& board)
{
    dir = (PPDirection)(((int)dir + 1) % 4);
}

void PPArrow::apply(PPPlayer& player, PPBoard& board)
{
    switch (dir)
    {
    case Left:
        for (int c = player.x; c >= 0; --c)
            board.at(c, player.y).color = player.color;
        break;
    case Up:
        for (int c = player.y; c < PPBoardSize; ++c)
            board.at(player.x, c).color = player.color;
        break;
    case Right:
        for (int c = player.x; c < PPBoardSize; ++c)
            board.at(c, player.y).color = player.color;
        break;
    case Down:
        for (int c = player.y; c >= 0; --c)
            board.at(player.x, c).color = player.color;
        break;
    default:
        break;
    }
}