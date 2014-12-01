//
//  PPGameManager.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#include "GameManager.h"
#include "SimpleAudioEngine.h"
#include "SpecificPlayers.h"

#include <cocos2d.h>

#include <future>
#include <chrono>

#include "Macros.h"

using namespace cocos2d;

const float GameManager::tickDelay = 0.5f;

GameManager::GameManager(int t)
: mTimer(t)
{
    
}

void GameManager::init()
{
    //TODO: initialize with network here
    auto pHumanPlayer = new HumanPlayer;
    pHumanPlayer->color = Color::Red;
    pHumanPlayer->currentDirection = Direction::Up;
    pHumanPlayer->pos = Vec2(0, 0);
    mState.mPlayers.push_back(PlayerPtr(pHumanPlayer));
    
    auto pAi = new StupidAiPlayer;
    pAi->color = Color::Green;
    pAi->currentDirection = Direction::Right;
    pAi->pos = Vec2(0, 7);
    mState.mPlayers.push_back(PlayerPtr(pAi));
    
    pAi = new StupidAiPlayer;
    pAi->color = Color:: Blue;
    pAi->currentDirection = Direction::Down;
    pAi->pos = Vec2(7, 7);
    mState.mPlayers.push_back(PlayerPtr(pAi));
    
    pAi = new StupidAiPlayer;
    pAi->color = Color::Yellow;
    pAi->currentDirection = Direction::Left;
    pAi->pos = Vec2(7, 0);
    mState.mPlayers.push_back(PlayerPtr(pAi));
}

void GameManager::update(float deltaTime)
{
    int ticks = mState.ticks();
    if (ticks % 2 == 0){
        if ((mTimer - ticks) / 2 <= 10 )
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sounds/beep-08.wav");
    }
    
    //increment time
    ticks = ++mState.mTicks;
    
    BonusManager::getInstance().update(mState);
    
    auto& players = mState.players();
    auto& board = mState.board();
    
    for(unsigned int i = 0; i < players.size(); ++i) {
        auto& pl = players[i];
        auto& pBonus = board[pl->pos].pBonus;
        if(pBonus) {
            pBonus->apply(mState, i);
            
            if (pl->color == Color::Red) {
                if (dynamic_cast<Checkpoint*>(pBonus)) {
                    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sounds/checkpoint.wav");
                }
                else if (dynamic_cast<Arrow*>(pBonus)) {
                    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("Sounds/arrow.wav");
                }
            }
            
            SEND_EVENT("RemoveBonus", pBonus);
            BonusManager::getInstance().removeBonus(pBonus);
            board[pl->pos].pBonus = nullptr;
            
        }
        board[pl->pos].color = pl->color;
    }
    
    for (auto& pl : players) {
        auto dir = pl->getDirection();
        SEND_EVENT("RotatePlayer", &*pl);

        for (auto& other : players) {
            if (other != pl && board[board.moveInDir(pl->pos, pl->currentDirection)] == board[other->pos]) {
                dir = Direction::None;
                break;
            }
        }
        
        auto res = board.moveInDir(pl->pos, dir);
        if (pl->pos != res) {
            pl->pos = res;
        }
        else {
            //TODO: feedback on wall hit
        }
    }
    
    if (ticks % 2) {
        for(auto& pBonus : BonusManager::getInstance().getBonuses()) {
            pBonus->update(mState);
        }
    }
}