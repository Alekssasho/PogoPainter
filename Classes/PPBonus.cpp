//
//  PPBonus.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/8/14.
//
//

#include "PPBonus.h"
#include "PPPlayer.h"

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

void PPBonusManager::update(PPBoard& board, std::vector<std::unique_ptr<PPPlayer>> & players)
{
    static int step = 0;
    if (step++ < steps_delay) {
        return;
    }

    auto get_pos = [this]() {
        return Vec2(position_picker(generator), position_picker(generator));
    };

    auto check_player = [&](const Vec2 & where) {
        bool hit = false;
        for (auto & player : players) {
            hit = hit || player->getPosition() == where;
        }
        return hit;
    };

    if (max_bonuses > bonuses.size()) {
        for (auto pos = get_pos(); !board.at(pos).bonus && !check_player(pos); pos = get_pos()) {
            PPBonus * bonus;
            switch (bonus_picker(generator))
            {
            case 0:
                bonus = new PPArrow(board.at(pos));
                break;
            case 1:
                bonus = new PPArrow(board.at(pos));
                break;
            case 2:
                bonus = new PPArrow(board.at(pos));
                break;
            }
            board.at(pos).bonus = std::unique_ptr<PPBonus>(bonus);
            bonuses.push_back(bonus);
        }
    }
    
    if (target_checkpoints > checkpoints.size()) {
        for (auto pos = get_pos(); !board.at(pos).bonus && !check_player(pos); pos = get_pos()) {
            checkpoints.push_back(new PPCheckpoint(board.at(pos)));
            board.at(pos).bonus = std::unique_ptr<PPBonus>(checkpoints.back());
        }
    }
}