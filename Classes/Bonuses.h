//
//  Bonuses.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#ifndef __PogoPainter__Bonuses__
#define __PogoPainter__Bonuses__

#include "BaseClasses.h"

class Checkpoint : public Bonus
{
public:
    //Inheriting constructors from Bonus
    using Bonus::Bonus;
    void apply(GameState& state, int playerIndex) override;
};

class Arrow : public Bonus
{
public:
    using Bonus::Bonus;
    void apply(GameState& state, int playerIndex) override;
    void update(GameState& state) override;
    Direction dir = Direction::Right;
};

#endif /* defined(__PogoPainter__Bonuses__) */
