//
//  Board.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#ifndef PogoPainter_Board_h
#define PogoPainter_Board_h

#include <array>

#include "Enums.h"
#include <math/Vec2.h>

using cocos2d::Vec2;

//forward declarations
class Bonus;

struct Cell
{
    int x = 0, y = 0;
    Color color = Color::Gray;
    Bonus* pBonus = nullptr;
};

inline bool operator==(const Cell& lhs, const Cell& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

struct Board
{
    static const int boardSize = 8;
    std::array<Cell, boardSize * boardSize> cells;
    
    
    Board()
    {
        for(int x = 0; x < boardSize; ++x) {
            for(int y = 0; y < boardSize; ++y) {
                auto& cell = this->operator[](Vec2(x, y));
                cell.x = x;
                cell.y = y;
            }
        }
    }
    
    Vec2 moveInDir(const Vec2& pos, Direction dir)
    {
        static const Vec2 directionVectors[] = {Vec2(-1, 0), Vec2(0, 1), Vec2(1, 0), Vec2(0, -1), Vec2(0, 0)};
        auto result = pos + directionVectors[dir];
        
        if(result.x < 0 || result.x >= 8 || result.y < 0 || result.y >= 8)
            return pos;
        return result;

    }
    
    Cell& operator[](const Vec2& pos) { return cells[pos.x + boardSize * pos.y]; }
};

#endif
