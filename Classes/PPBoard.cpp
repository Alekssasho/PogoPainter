//
//  PPBoard.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/8/14.
//
//

#include "PPBoard.h"

Vec2 PPBoard::moveInDir(const Vec2& pos, PPDirection dir)
{
    static const Vec2 directionVectors[] = {Vec2(-1, 0), Vec2(0, 1), Vec2(1, 0), Vec2(0, -1)};
    auto result = pos + directionVectors[dir];
    
    if(result.x < 0 || result.x >= 8 || result.y < 0 || result.y >= 8)
        return pos;
    return result;
}