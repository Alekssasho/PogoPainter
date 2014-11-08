//
//  PPBonus.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/8/14.
//
//

#ifndef __PogoPainter__PPBonus__
#define __PogoPainter__PPBonus__

#include <vector>
#include <random>
#include <memory>

class PPPlayer;
class PPBoard;
class PPCell;

enum PPDirection
{
    Left, Up, Right, Down
};

class PPBonus
{
public:
    PPBonus(PPCell& cell)
    : mCell(cell)
    {}
    
    virtual void apply(PPPlayer& player, PPBoard& board) = 0;
    virtual void update(PPBoard& board);
    virtual ~PPBonus() {}
private:
    PPCell& mCell;
};

class PPCheckpoint : public PPBonus
{
public:
    PPCheckpoint(PPCell& cell)
    :PPBonus(cell)
    {}
    
    void apply(PPPlayer& player, PPBoard& board) override;
private:
};

class PPArrow : public PPBonus
{
    PPDirection dir;
public:
    PPArrow(PPCell& cell)
        :PPBonus(cell)
    {}

    void update(PPBoard& board) override;
    void apply(PPPlayer& player, PPBoard& board) override;
};

#define PPBoardSize 8

class PPBonusManager
{
    std::vector<PPBonus*> bonuses;
    std::vector<PPCheckpoint*> checkpoints;

    const int max_bonuses = 3;
    const int target_checkpoints = 3;
    const int steps_delay = 4;

    std::mt19937 generator;
    std::uniform_int_distribution<int> bonus_picker, position_picker;

    PPBonusManager() : bonus_picker(0, 2), position_picker(PPBoardSize - 1, PPBoardSize - 1) {}

public:
    static PPBonusManager& getInstance() {
        static PPBonusManager instance;
        return instance;
    }

    void update(PPBoard& board, std::vector<std::unique_ptr<PPPlayer>> & players);
    
};


#endif /* defined(__PogoPainter__PPBonus__) */
