//
//  Interfaces.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#ifndef PogoPainter_Interfaces_h
#define PogoPainter_Interfaces_h

#include "Enums.h"

#include <math/Vec2.h>
#include <memory>

using cocos2d::Vec2;

struct Cell;
class GameState;

struct Player
{
    virtual ~Player() {}
    virtual Direction getDirection() = 0;
    
    Vec2 pos;
    Direction currentDirection;
    int points = 0;
    Color color;
};

using PlayerPtr = std::unique_ptr<Player>;

struct Bonus
{
    Bonus(Cell& c)
    : cell(c)
    {}
    
    virtual ~Bonus() {}
    virtual void apply(GameState& state, int playerIndex) = 0;
    virtual void update(GameState& state) {}
    
    Cell& cell;
};

#endif
