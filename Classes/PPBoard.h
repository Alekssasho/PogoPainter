//
//  PPBoard.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/8/14.
//
//

#ifndef __PogoPainter__PPBoard__
#define __PogoPainter__PPBoard__

#include <array>

using namespace cocos2d;

enum class PPColor {
    Red,
    Blue,
    Green,
    Yellow,
    Empty
};

class PPBonus;

class PPCell
{
public:
    int x, y;
    PPColor color;
    std::unique_ptr<PPBonus> bonus;
    Sprite* sprite;
};

class PPBoard
{
public:
    PPCell& at(int x, int y) { return cells[x + y * 8]; }
    std::array<PPCell, 8 * 8> cells;
private:
};

#endif /* defined(__PogoPainter__PPBoard__) */
