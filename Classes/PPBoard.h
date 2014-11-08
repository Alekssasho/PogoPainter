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
#include <functional>
#include <cocos2d.h>

#include "PPBonus.h"
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
    PPColor color = PPColor::Empty;
    std::unique_ptr<PPBonus> bonus;
    Sprite* sprite;
};

#define PPBoardSize 8

class PPBoard
{
public:
    Vec2 moveInDir(const Vec2& pos, PPDirection dir);
    PPCell& at(int x, int y) { return cells[x + y * PPBoardSize]; }
    std::array<PPCell, PPBoardSize * PPBoardSize> cells;

    void each(std::function<void(PPCell &)> cb);
private:
};

#endif /* defined(__PogoPainter__PPBoard__) */
