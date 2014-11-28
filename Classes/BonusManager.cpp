//
//  BonusManager.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#include "BonusManager.h"
#include <algorithm>

#include <cocos2d.h>
using namespace cocos2d;

#include "Macros.h"

void BonusManager::update(GameState& state)
{
    static int step1 = 0;
    if (step1++ < steps_delay) {
        return;
    }
    step1 = 0;
    
    auto get_pos = [this] {
        return Vec2(position_picker(generator), position_picker(generator));
    };
    
    auto check_player = [&](const Vec2 & where) {
        bool hit = false;
        for (auto & player : state.players()) {
            hit = hit || player->pos == where;
        }
        return hit;
    };
    
    auto& board = state.board();
    
    if (target_checkpoints > checkpoints.size()) {
        auto pos = get_pos();
        bool flag = true;
        
        while (flag){
            
            while (board[pos].pBonus || check_player(pos)) {
                pos = get_pos();
            }
            
            bool isInside = false;
            for (auto& checkpoint : checkpoints) {
                if (abs(checkpoint->cell.x - pos.x) < 2 && abs(checkpoint->cell.y - pos.y) < 2){
                    isInside = true;
                    pos = get_pos();
                    break;
                }
            }
            if (!isInside){
                flag = false;
            }
        }
        checkpoints.push_back(BonusPtr(new Checkpoint(board[pos])));
        board[pos].pBonus = checkpoints.back().get();
        
        SEND_EVENT("NewBonus", board[pos].pBonus);
    }
    
    static int step2 = 0;
    if (step2++ < steps_delay) {
        return;
    }
    step2 = 0;
    
    
    if (max_bonuses > bonuses.size()) {
        auto pos = get_pos();
        while (board[pos].pBonus || check_player(pos)) {
            pos = get_pos();
        }
        
        Bonus * bonus;
        switch (bonus_picker(generator))
        {
            case 0:
            case 1:
            case 2:
                bonus = new Arrow(board[pos]);
                break;
        }
        
        bonuses.push_back(BonusPtr(bonus));
        board[pos].pBonus = bonus;
        
        SEND_EVENT("NewBonus", board[pos].pBonus);
    }
}

void BonusManager::removeBonus(Bonus * bonus)
{
    auto foundItr = std::find_if(bonuses.begin(), bonuses.end(), [bonus](const BonusPtr& ptr) {
        return bonus->cell.x  == ptr->cell.x
            && bonus->cell.y == ptr->cell.y;
    });
    
    if (foundItr != bonuses.end()) {
        bonuses.erase(foundItr);
    }
    else {
        auto foundItr = std::find_if(checkpoints.begin(), checkpoints.end(), [bonus](const BonusPtr& ptr) {
            return bonus->cell.x  == ptr->cell.x
            && bonus->cell.y == ptr->cell.y;
        });
        if (foundItr != checkpoints.end()) {
            checkpoints.erase(foundItr);
        }
    }
}
