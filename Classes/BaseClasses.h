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

class Bonus
{
public:
    enum Type : unsigned char { Checkpoint, Arrow } type;

    Bonus(Cell& c, Type t)
        : cell(c), type(t)
    {}

    virtual ~Bonus() {}
    virtual void apply(GameState& state, int playerIndex) = 0;
    virtual void update(GameState& state) {}

    virtual unsigned char getData() const { return 0; }
    virtual void setData(unsigned char data) {};

    Cell& cell;
};

#endif
