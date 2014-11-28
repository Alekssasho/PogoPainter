//
//  BonusManager.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#ifndef __PogoPainter__BonusManager__
#define __PogoPainter__BonusManager__

#include "GameState.h"
#include "Bonuses.h"
#include <vector>
#include <random>
#include <memory>

using BonusPtr = std::unique_ptr<Bonus>;

class BonusManager
{
    std::vector<BonusPtr> bonuses;
    std::vector<BonusPtr> checkpoints;
    
    const int max_bonuses = 3;
    const int target_checkpoints = 3;
    const int steps_delay = 2;
    
    std::mt19937 generator;
    std::uniform_int_distribution<int> bonus_picker, position_picker;
    
    BonusManager() : bonus_picker(0, 2), position_picker(0, Board::boardSize - 1) {}
    
public:
    
    void clear() {
        bonuses.clear();
        checkpoints.clear();
    }
    
    void removeBonus(Bonus * bonus);
    
    static BonusManager& getInstance() {
        static BonusManager instance;
        return instance;
    }
    
    std::vector<BonusPtr>& getBonuses() { return bonuses; }
    
    std::vector<BonusPtr>& getCheckpoints() { return checkpoints; }
    
    void update(GameState& state);
};

#endif /* defined(__PogoPainter__BonusManager__) */
