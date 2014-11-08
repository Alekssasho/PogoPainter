//
//  PPBoard.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/8/14.
//
//

#include "PPBoard.h"

PPBoard::PPBoard()
{
    for(int x = 0; x < PPBoardSize; ++x) {
        for(int y = 0; y < PPBoardSize; ++y) {
            this->at(x, y).x = x;
            this->at(x, y).y = y;
        }
    }
}

Vec2 PPBoard::moveInDir(const Vec2& pos, PPDirection dir)
{
    static const Vec2 directionVectors[] = {Vec2(-1, 0), Vec2(0, 1), Vec2(1, 0), Vec2(0, -1), Vec2(0, 0)};
    auto result = pos + directionVectors[dir];
    
    if(result.x < 0 || result.x >= 8 || result.y < 0 || result.y >= 8)
        return pos;
    return result;
}

void PPBoard::each(std::function<void(PPCell &)> cb)
{
    for (auto & cell : cells) {
        cb(cell);
    }
}