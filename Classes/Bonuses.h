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
#include <unordered_map>
#include <functional>
#include <utility>

class Checkpoint : public Bonus
{
public:
    Checkpoint(Cell& c) : Bonus(c, Bonus::Type::Checkpoint) {}
    void apply(GameState& state, int playerIndex) override;
};

class Arrow : public Bonus
{
public:
    Arrow(Cell& c) : Bonus(c, Bonus::Type::Arrow) {}
    void apply(GameState& state, int playerIndex) override;
    void update(GameState& state) override;
    Direction dir = Direction::Right;

    unsigned char getData() const override { return dir; };
    void setData(unsigned char data) override { dir = static_cast<Direction>(data); }
};


const std::unordered_map<Bonus::Type, std::function<Bonus*(Cell&)>, std::hash<int>> BonusInitializers = {
    std::make_pair(Bonus::Type::Checkpoint, [](Cell & c) { return new Checkpoint(c);  }),
    std::make_pair(Bonus::Type::Arrow, [](Cell & c) { return new Arrow(c);  })
};

#endif /* defined(__PogoPainter__Bonuses__) */
